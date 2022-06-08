#include "ble_main.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "led_service.h"

#include "thread_coap_fwd_utils.h"
#include "fwd_service.h"
#include "nrf_log.h"

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
NRF_BLE_GATT_DEF(m_gatt);
NRF_BLE_QWR_DEF(m_qwr);
BLE_LED_SERVICE_DEF(m_led_service);                                                                                               
//BLE_ADVERTISING_DEF(m_advertising);                                            
//ble_advertising_t * p_m_advertising = &m_advertising;

extern uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

//ble_fwd_t  m_fwd_service;
int32_t last_temp = 0;


#define LIGHTBULB_LED                   BSP_BOARD_LED_1

BLE_FWD_DEF(m_fwd, NRF_SDH_BLE_TOTAL_LINK_COUNT);
APP_TIMER_DEF(m_fwd_timer_id);
APP_TIMER_DEF(m_notification_timer_id);

// YOUR_JOB: Use UUIDs for service(s) used in your application.

static uint8_t m_custom_value = 0;

#define OUR_CHAR_TIMER_INTERVAL     APP_TIMER_TICKS(1000) // 1000 ms intervals
#define NOTIFICATION_INTERVAL       APP_TIMER_TICKS(1000)


static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_FWD_SERVICE, FWD_SERVICE_UUID_TYPE},
};


// BLE_WRITE:
/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] characteristic1_value     value that was received from the phone
 */
// called from our_services.c from on_write();
// Make a note of the arguments that are passed to this handler, we will use that later on
static void fwd_data_handler(ble_fwd_evt_t * p_evt)
{
  NRF_LOG_INFO("Thu:  fwd_data_handler()");
  ret_code_t  err_code;
  if (p_evt->type == BLE_FWD_EVT_RX_DATA)	
  {
    uint8_t * received_message = (uint8_t *) nrf_malloc((p_evt->params.rx_data.length+1)*sizeof(uint8_t));
    char * token;
    char * saveptr;
  
    memset(received_message, 0, (p_evt->params.rx_data.length+1)*sizeof(uint8_t));
    strncpy((char *)received_message, (char *)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
  
    //If the receive payload message (not discover)
    if (strcmp((char *)received_message, "discover") != 0)	
    {
      NRF_LOG_INFO("thread_coap_unicast_light_request_send()");
      token = strtok_r((char *)received_message, "_",&saveptr); 
      thread_coap_unicast_light_request_send((uint8_t *)token, (uint8_t *)saveptr, strlen(saveptr));
    } 
    //If receive discover message
    else	
    {
       NRF_LOG_INFO("thread_coap_discover_start()");
       thread_coap_discover_start();
    }
    nrf_free(received_message);
  }     
}
// Add other handlers here...
/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_led_service  Instance of LED Service to which the write applies.
 * @param[in] led_state      Written/desired state of the LED.
 */
static void led_write_handler(uint16_t conn_handle, ble_led_service_t * p_led_service, uint8_t led_state)
{
    if (led_state)
    {
        bsp_board_led_on(LIGHTBULB_LED);
        NRF_LOG_INFO("Received LED ON!");
    }
    else
    {
        bsp_board_led_off(LIGHTBULB_LED);
        NRF_LOG_INFO("Received LED OFF!");
    }
}
void advertising_start(bool erase_bonds);


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

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

//Thu added:

static void timer_timeout_handler(void * p_context)
{
  ble_fwd_evt_rx_data_t temperature;
  sd_temp_get(temperature.p_data);
  temperature.length = 4;
  NRF_LOG_INFO("Sending: %d %x", *(temperature.p_data), *(temperature.p_data));
  ble_fwd_data_send(&m_fwd, &temperature, &temperature.length, m_conn_handle);

  //nrf_gpio_pin_toggle(LED_3);
}


static void notification_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    ret_code_t err_code;
    
    // Increment the value of m_custom_value before nortifing it.
    m_custom_value++;
    NRF_LOG_INFO("Thu: Value to update: %d", m_custom_value);
    
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
       ret_code_t err_code;
       err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
       APP_ERROR_CHECK(err_code); */
      
      err_code = app_timer_create(&m_notification_timer_id, APP_TIMER_MODE_REPEATED, notification_timeout_handler);
      err_code = app_timer_create(&m_fwd_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
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

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

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


/**@brief Function for handling the YYY Service events.
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service,
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. ", p_evt->params.char_xx.value.p_str);
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
   
  
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};
    ble_fwd_init_t     fwd_init;
    ble_led_service_init_t led_init;

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);
   
     // BLE_WRITE: Initialize Our Service module.
    memset(&fwd_init, 0, sizeof(fwd_init));

    fwd_init.data_handler = fwd_data_handler;

    err_code = ble_fwd_init(&m_fwd, &fwd_init);
    APP_ERROR_CHECK(err_code);

    // 1. Initialize the LED service
    led_init.led_write_handler = led_write_handler;
    err_code = ble_led_service_init(&m_led_service, &led_init);
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
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

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

/**@brief Function for handling SOC events.
 *
 * @param[in]   sys_evt     SoC stack event.
 * @param[in]   p_context   Unused.
 */
static void soc_evt_handler(uint32_t sys_evt, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    otSysSoftdeviceSocEvtHandler(sys_evt);
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

    // Register a handler for SOC events.
    NRF_SDH_SOC_OBSERVER(m_soc_observer, NRF_SDH_SOC_STACK_OBSERVER_PRIO, soc_evt_handler, NULL);
    
    //NRF_SDH_BLE_OBSERVER(m_our_service_observer, APP_BLE_OBSERVER_PRIO, ble_fwd_service_on_ble_evt, (void*) &m_fwd);
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
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));
    /* Thu added this
    ble_advdata_manuf_data_t             manuf_data;
    uint8_t data[]                       = "SomeData!";
    manuf_data.company_identifier        = 0x0059;
    manuf_data.data.p_data               = data;
    manuf_data.data.size                 = sizeof(data);
    init.advdata.p_manuf_specific_data   = &manuf_data;
    

    ble_advdata_manuf_data_t             manuf_data_response;
    uint8_t data2[]                       = "SomeDa222222";
    manuf_data_response.company_identifier = "0x0059";
    manuf_data_response.data.p_data       = data2;
    manuf_data_response.data.size         = sizeof(data2);
    init.srdata.name_type                 = BLE_ADVDATA_NO_NAME;
    init.srdata.p_manuf_specific_data     = &manuf_data_response;
    */

    //init.advdata.p_tx_power_level        = 100;
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME; 
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED evetnt
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}

void thread_coap_fwd_callback(uint8_t * p_response_message, uint16_t length)
{
    ret_code_t err_code;
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)	
    {
      if(p_response_message != NULL)
        {
          if(*p_response_message == 0)
          {
            NRF_LOG_ERROR("No end nodes were found");
          }
    
          NRF_LOG_INFO("Address: %x",*p_response_message);
          err_code = ble_fwd_data_send(&m_fwd, p_response_message, &length, m_conn_handle);
          
          if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)	
            {
              NRF_LOG_ERROR("BLE LFS Service: error happen in sending message. Error code: 0x%x\r\n", err_code);
            }
        }

      else
      {
        NRF_LOG_ERROR("BLE LFS Service: Sending failed");
      }       
    }
     
    else	
    {
        NRF_LOG_INFO("BLE connection is invalid");
    };

}

/**@brief Function for starting advertising.
 */
void ble_main_init(void)
{
    timers_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    peer_manager_init();
    
}
#endif