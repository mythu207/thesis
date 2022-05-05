#include <stdint.h>
#include <string.h>
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"


#if NRF_MODULE_ENABLED(BLE_FWD_SERVICE)
#include "fwd_service.h"

#define NRF_LOG_MODULE_NAME ble_fwd_service
#if BLE_FWD_SERVICE_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       BLE_FWD_SERVICE_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  BLE_FWD_SERVICE_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR BLE_FWD_SERVICE_CONFIG_DEBUG_COLOR
#else // BLE_OUR_SERVICE_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // BLE_OUR_SERVICE_CONFIG_LOG_ENABLED
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

// FROM_SERVICE_TUTORIAL: Defining 16-bit service and 128-bit base UUIDs

#define BLE_FWD_MAX_TX_CHAR_LEN       BLE_FWD_MAX_DATA_LEN /**< Maximum length of the Send BLE Message Characteristic (in bytes). */
#define BLE_FWD_MAX_RX_CHAR_LEN       BLE_FWD_MAX_DATA_LEN /**< Maximum length of the Receive BLE Message Characteristic (in bytes). */



static void on_connect(ble_fwd_t * p_fwd, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_fwd_evt_t              evt;
    ble_gatts_value_t          gatts_val;
    uint8_t                    cccd_value[2];
    ble_fwd_client_context_t * p_client = NULL;

    err_code = blcm_link_ctx_get(p_fwd->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_fwd->tx_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_fwd->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_fwd_evt_t));
        evt.type        = BLE_FWD_EVT_COMM_STARTED;
        evt.p_fwd       = p_fwd;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_fwd->data_handler(&evt);
    }

}



static void on_write(ble_fwd_t * p_fwd, ble_evt_t const * p_ble_evt)
{
    ret_code_t                    err_code;
    ble_fwd_evt_t                 evt;
    ble_fwd_client_context_t    * p_client;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_fwd->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_fwd_evt_t));
    evt.p_fwd       = p_fwd;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_fwd->tx_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_FWD_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_FWD_EVT_COMM_STOPPED;
            }

            if (p_fwd->data_handler != NULL)
            {
                p_fwd->data_handler(&evt);
            }

        }
    }
    else if ((p_evt_write->handle == p_fwd->rx_handles.value_handle) &&
             (p_fwd->data_handler != NULL))
    {
        evt.type                  = BLE_FWD_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_fwd->data_handler(&evt);
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }
}



/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_fwd_t * p_fwd, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_fwd_evt_t              evt;
    ble_fwd_client_context_t * p_client;

    err_code = blcm_link_ctx_get(p_fwd->p_link_ctx_storage,
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
        memset(&evt, 0, sizeof(ble_fwd_evt_t));
        evt.type        = BLE_FWD_EVT_TX_RDY;
        evt.p_fwd       = p_fwd;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_fwd->data_handler(&evt);
    }
}




void ble_fwd_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_fwd_t * p_fwd = (ble_fwd_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_fwd, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_fwd, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_fwd, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
	
}


uint32_t ble_fwd_init(ble_fwd_t * p_fwd, ble_fwd_init_t const * p_fwd_init)
{
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         fwd_base_uuid = BLE_UUID_FWD_BASE;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_fwd);
    VERIFY_PARAM_NOT_NULL(p_fwd_init);

    // Initialize the service structure.
    p_fwd->data_handler = p_fwd_init->data_handler;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&fwd_base_uuid, &p_fwd->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_fwd->uuid_type;
    ble_uuid.uuid = BLE_UUID_FWD_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_fwd->service_handle);
    /**@snippet [Adding proprietary Service p_fwd the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the RX Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_FWD_RX_CHAR;
    add_char_params.uuid_type                = p_fwd->uuid_type;
    add_char_params.max_len                  = BLE_FWD_MAX_RX_CHAR_LEN;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = true;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_fwd->service_handle, &add_char_params, &p_fwd->rx_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the TX Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_FWD_TX_CHAR;
    add_char_params.uuid_type         = p_fwd->uuid_type;
    add_char_params.max_len           = BLE_FWD_MAX_TX_CHAR_LEN;
    add_char_params.init_len          = sizeof(uint8_t);
    add_char_params.is_var_len        = true;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_fwd->service_handle, &add_char_params, &p_fwd->tx_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}



// ALREADY_DONE_FOR_YOU: Function to be called when updating characteristic value
ret_code_t ble_fwd_data_send(ble_fwd_t * p_fwd,
                             uint8_t   * p_data,
                             uint16_t  * p_length,
                             uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_fwd_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_fwd);

    err_code = blcm_link_ctx_get(p_fwd->p_link_ctx_storage, conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (*p_length > BLE_FWD_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_fwd->tx_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = p_length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

#endif