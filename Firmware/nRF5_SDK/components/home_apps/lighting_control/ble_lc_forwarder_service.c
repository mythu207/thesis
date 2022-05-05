/*	Ho Chi Minh, 08/03/2019
 *	Writer: Do Ngoc Nhuan
 *	BLE LC Forwarder Service that combine with BLE LC Forwarder Thread Coap (lc_thread_coap) to perform forwarding BLE message in a Thread network.
 */
 
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_LFS)
#include "ble_lc_forwarder_service.h"

#define NRF_LOG_MODULE_NAME ble_lfs
#if BLE_LFS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       BLE_LFS_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  BLE_LFS_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BLE_LFS_CONFIG_DEBUG_COLOR
#else // BLE_LFS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // BLE_LFS_CONFIG_LOG_ENABLED
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_lfs     BLE LC Forwarder Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */

static void on_connect(ble_lfs_t * p_lfs, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_lfs_evt_t              evt;
    ble_gatts_value_t          gatts_val;
    uint8_t                    cccd_value[2];
    ble_lfs_client_context_t * p_client = NULL;

    err_code = blcm_link_ctx_get(p_lfs->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the (Write) characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_lfs->fwd_snd_char_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_lfs->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_lfs_evt_t));
        evt.type        = BLE_LFS_EVT_COMM_STARTED;
        evt.p_lfs       = p_lfs;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_lfs->data_handler(&evt);
    }
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_lfs     BLE LC Forwarder Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_lfs_t * p_lfs, ble_evt_t const * p_ble_evt)
{
	ret_code_t                    err_code;
    ble_lfs_evt_t                 evt;
    ble_lfs_client_context_t    * p_client;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
    err_code = blcm_link_ctx_get(p_lfs->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }
    memset(&evt, 0, sizeof(ble_lfs_evt_t));
    evt.p_lfs       = p_lfs;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_lfs->fwd_snd_char_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_LFS_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_LFS_EVT_COMM_STOPPED;
            }

            if (p_lfs->data_handler != NULL)
            {
                p_lfs->data_handler(&evt);
            }

        }
    }
    else if ((p_evt_write->handle == p_lfs->fwd_rcv_char_handles.value_handle) &&
             (p_lfs->data_handler != NULL))
    {
        evt.type                 	 = BLE_LFS_EVT_RX_DATA;
        evt.params.write_data.p_data = p_evt_write->data;
        evt.params.write_data.length = p_evt_write->len;

        p_lfs->data_handler(&evt);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_lfs     BLE LC Forwarder Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_lfs_t * p_lfs, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_lfs_evt_t              evt;
    ble_lfs_client_context_t * p_client;

    err_code = blcm_link_ctx_get(p_lfs->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }

    if (p_client->is_notification_enabled)
    {
        memset(&evt, 0, sizeof(ble_lfs_evt_t));
        evt.type        = BLE_LFS_EVT_TX_RDY;
        evt.p_lfs       = p_lfs;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_lfs->data_handler(&evt);
    }
}

/**@brief Function for adding Send BLE Message characteristic.
 *
 * @param[in] p_lfs     BLE LC Forwarder Service structure.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static ret_code_t forwarder_send_message_characteristic_add(ble_lfs_t * p_lfs)
{
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_lfs->uuid_type;
    ble_uuid.uuid = BLE_LFS_SND_MSG_CHARACTERISTC_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;	//chiều dài dữ liệu thay đổi được. VD: temperature chiều dài cố định 4 byte, string chiều dài vô định.

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_LFS_MAX_NOTIFY_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_lfs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_lfs->fwd_snd_char_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}

/**@brief Function for adding Receive BLE Message characteristic.
 *
 * @param[in] p_lfs     BLE LC Forwarder Service structure.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
static ret_code_t forwarder_receive_message_characteristic_add(ble_lfs_t * p_lfs)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_lfs->uuid_type;
    ble_uuid.uuid = BLE_LFS_RCV_MSG_CHARACTERISTC_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_LFS_MAX_WRITE_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_lfs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_lfs->fwd_rcv_char_handles);
}

void ble_lfs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_lfs_t * p_lfs = (ble_lfs_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_lfs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_lfs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_lfs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

ret_code_t ble_lfs_init(ble_lfs_t * p_lfs, ble_lfs_init_t const * p_lfs_init)
{
    ret_code_t    err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t lfs_base_uuid = LFS_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_lfs);
	VERIFY_PARAM_NOT_NULL(p_lfs_init);
	
    // Initialize the service structure.
    p_lfs->data_handler = p_lfs_init->data_handler;
	
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&lfs_base_uuid, &p_lfs->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_lfs->uuid_type;
    ble_uuid.uuid = BLE_LFS_SERVICE_UUID;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_lfs->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the Receive BLE Message characteristic.
    err_code = forwarder_receive_message_characteristic_add(p_lfs);
    VERIFY_SUCCESS(err_code);

    // Add the Send BLE Message characteristic.
    err_code = forwarder_send_message_characteristic_add(p_lfs);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

ret_code_t ble_lfs_data_send(ble_lfs_t * p_lfs,
							 uint8_t   * p_data,
							 uint16_t  * p_length,
							 uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_lfs_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_lfs);

    err_code = blcm_link_ctx_get(p_lfs->p_link_ctx_storage, conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_LFS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_lfs->fwd_snd_char_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

#endif // NRF_MODULE_ENABLED(BLE_LFS)
