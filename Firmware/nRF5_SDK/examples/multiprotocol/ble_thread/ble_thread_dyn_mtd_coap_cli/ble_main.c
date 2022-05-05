/**	Do Ngoc Nhuan, 04/03/2019
 */
 
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(MAIN_BLE)
#include "ble_main.h"

#define NRF_LOG_MODULE_NAME main_ble
#if MAIN_BLE_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       MAIN_BLE_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  MAIN_BLE_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR MAIN_BLE_CONFIG_DEBUG_COLOR
#else // MAIN_BLE_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // MAIN_BLE_CONFIG_LOG_ENABLED
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();


BLE_LCS_DEF(m_lcs, NRF_SDH_BLE_TOTAL_LINK_COUNT);
BLE_LFS_DEF(m_lfs, NRF_SDH_BLE_TOTAL_LINK_COUNT);
NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */
ble_advertising_t * p_m_advertising = &m_advertising;
bool erase_bonds;
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;
// Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_LCS_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};
void advertising_start(bool erase_bonds);

/***************************************************************************************************
 * @section Handle lighting control application
 **************************************************************************************************/

void ble_lc_string_event_handler(uint8_t * msg_to_send, uint8_t length)
{
	ret_code_t err_code;
	uint16_t length16 = length;
	
	if (m_conn_handle != BLE_CONN_HANDLE_INVALID)	{
		err_code = ble_lcs_data_send(&m_lcs, msg_to_send, &length16, m_conn_handle);
		if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)	{
			NRF_LOG_ERROR("BLE LCS Service: error happen in sending message. Error code: 0x%x\r\n", err_code);
		}
	}
}

void ble_lighting_control_event_handler(lighting_control_info_t * p_lighting_control_info)
{
	ret_code_t err_code;
	uint8_t * message = (uint8_t *)nrf_malloc(MAIN_BLE_MAX_STRING_TO_SEND_LENGTH*sizeof(uint8_t));
	uint16_t message_lenght;
	
	memset(message, NULL, MAIN_BLE_MAX_STRING_TO_SEND_LENGTH*sizeof(uint8_t));
	
	make_msg_lc_light_info((lighting_control_info_t *)p_lighting_control_info, message);
	message_lenght = strlen((char *)message);
	
	if (m_conn_handle != BLE_CONN_HANDLE_INVALID)	{
		err_code = ble_lcs_data_send(&m_lcs, message, &message_lenght, m_conn_handle);
		if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)	{
			NRF_LOG_ERROR("BLE LCS Service: error happen in sending message. Error code: 0x%x\r\n", err_code);
		}
	}
	
	nrf_free(message);
}

static void lcs_data_handler(ble_lcs_evt_t * p_evt)
{
	if (p_evt->type == BLE_LCS_EVT_RX_DATA)	{
		recognize_lc_control_command(p_evt->params.write_data.p_data, p_evt->params.write_data.length);
	}
}

static void lfs_data_handler(ble_lfs_evt_t * p_evt)
{
	if (p_evt->type == BLE_LFS_EVT_RX_DATA)	{
		uint8_t * received_message = (uint8_t *) nrf_malloc((p_evt->params.write_data.length+1)*sizeof(uint8_t));
		char * token;
		char * saveptr;
		
		memset(received_message, 0, (p_evt->params.write_data.length+1)*sizeof(uint8_t));
		strncpy((char *)received_message, (char *)p_evt->params.write_data.p_data, p_evt->params.write_data.length);
		
		if (strcmp((char *)received_message, "discover")!=0)	{
			token = strtok_r((char *)received_message, "_", &saveptr);		//payload format: <ip6 address>_<data to coap server>
			thread_coap_unicast_light_request_send((uint8_t *)token, (uint8_t *)saveptr, strlen(saveptr));
		} else	{
			thread_coap_discover_start();
		}
		nrf_free(received_message);
	}
}

void thread_coap_lighting_control_forwarder_callback(uint8_t * p_response_message, uint16_t length)
{
	ret_code_t err_code;
	if (m_conn_handle != BLE_CONN_HANDLE_INVALID)	{
		err_code = ble_lfs_data_send(&m_lfs, p_response_message, &length, m_conn_handle);
		if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)	{
			NRF_LOG_ERROR("BLE LFS Service: error happen in sending message. Error code: 0x%x\r\n", err_code);
		}
	} else	{
		thread_coap_unicast_unpeer_light_request_send(false);
	};
}

/***************************************************************************************************
 * @section BLE System-on-Chip control
 **************************************************************************************************/
/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(false);
            break;

        default:
            break;
    }
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing services that will be used by the application.
 */
void services_init(void)
{
	uint32_t        err_code;
	ble_lcs_init_t	lcs_init;
	ble_lfs_init_t	lfs_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initizlize LCS.
    memset(&lcs_init, 0, sizeof(lcs_init));
	lcs_init.data_handler = lcs_data_handler;
	err_code = ble_lcs_init(&m_lcs, &lcs_init);
	APP_ERROR_CHECK(err_code);
	
	// Initizlize LFS.
    memset(&lfs_init, 0, sizeof(lfs_init));
	lfs_init.data_handler = lfs_data_handler;
	err_code = ble_lfs_init(&m_lfs, &lfs_init);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
		

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.");
			m_conn_handle = BLE_CONN_HANDLE_INVALID;
		
            // LED indication will be changed when advertising starts.
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
	
	init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
	init.srdata.uuids_complete.p_uuids = m_adv_uuids;
	
    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for starting advertising.
 */
void advertising_start(bool x_erase_bonds)
{
    if (x_erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}

void ble_for_lighting_control_init(void)
{
//	ret_code_t err_code;
	ble_stack_init();
    gap_params_init();
    gatt_init();
	
	services_init();
	advertising_init();

    conn_params_init();
    peer_manager_init();
	
	lighting_control_callback_register(ble_lighting_control_event_handler);
	lc_string_handling_callback_register(ble_lc_string_event_handler);
	thread_coap_lighting_control_forwarder_callback_set(thread_coap_lighting_control_forwarder_callback);
}

#endif // NRF_MODULE_ENABLED(MAIN_BLE)
