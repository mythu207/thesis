
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ble_main.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "thread_utils.h"
#include "thread_coap_fwd_utils.h"
#include "mqttsn_client.h"

#include <openthread/coap.h>
#include <openthread/cli.h>
#include <openthread/thread_ftd.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/platform-softdevice.h>

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define SCHED_QUEUE_SIZE                64                                      /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE         /**< Maximum app_scheduler event size. */

#define LED_ON_REQUEST         1                                            /**< LED ON command. */
#define LED_OFF_REQUEST        0                                            /**< LED OFF command. */
#define SEARCH_GATEWAY_TIMEOUT 5                                            /**< MQTT-SN Gateway discovery procedure timeout in [s]. */

#define SCHED_EVENT_DATA_SIZE  APP_TIMER_SCHED_EVENT_DATA_SIZE              /**< Maximum app_scheduler event size. */

static mqttsn_client_t      m_client;                                       /**< An MQTT-SN client instance. */
static mqttsn_remote_t      m_gateway_addr;                                 /**< A gateway address. */
static uint8_t              m_gateway_id;                                   /**< A gateway ID. */
static mqttsn_connect_opt_t m_connect_opt;                                  /**< Connect options for the MQTT-SN client. */
static bool                 m_subscribed       = 0;                         /**< Current subscription state. */
static uint16_t             m_msg_id           = 0;                         /**< Message ID thrown with MQTTSN_EVENT_TIMEOUT. */
static char                 m_client_id[]      = "nRF52840_subscriber";     /**< The MQTT-SN Client's ID. */
static char                 m_topic_name[]     = "nRF52840_resources/led3"; /**< Name of the topic corresponding to subscriber's BSP_LED_2. */
static mqttsn_topic_t       m_topic            =                            /**< Topic corresponding to subscriber's BSP_LED_2. */
{
    .p_topic_name = (unsigned char *)m_topic_name,
    .topic_id     = 0,
};
/***************************************************************************************************
 * @section MQTT-SN
 **************************************************************************************************/

 /**@brief Turns the MQTT-SN network indication LED on.
 *
 * @details This LED is on when an MQTT-SN client is in connected or awake state.
 */
static void light_on(void)
{
    LEDS_ON(BSP_LED_3_MASK);
}


/**@brief Turns the MQTT-SN network indication LED off.
 *
 * @details This LED is on when an MQTT-SN client is in disconnected or asleep state.
 */
static void light_off(void)
{
    LEDS_OFF(BSP_LED_3_MASK);
}


/**@brief Toggles BSP_LED_2 based on received LED command. */
static void led_update(uint8_t * p_data)
{
    //if (*p_data == LED_ON_REQUEST)
    //{
    //    LEDS_TOGGLE(BSP_LED_2_MASK);
    //}
    //else if (*p_data == LED_OFF_REQUEST)
    //{
    //    LEDS_OFF(BSP_LED_2_MASK);
    //}
    if (*p_data != NULL)
    {
      LEDS_INVERT(BSP_LED_2_MASK);
    }

    return;
}

/**@brief Initializes MQTT-SN client's connection options. */
static void connect_opt_init(void)
{
    m_connect_opt.alive_duration = MQTTSN_DEFAULT_ALIVE_DURATION,
    m_connect_opt.clean_session  = MQTTSN_DEFAULT_CLEAN_SESSION_FLAG,
    m_connect_opt.will_flag      = MQTTSN_DEFAULT_WILL_FLAG,
    m_connect_opt.client_id_len  = strlen(m_client_id),

    memcpy(m_connect_opt.p_client_id,  (unsigned char *)m_client_id,  m_connect_opt.client_id_len);
}


/**@brief Processes GWINFO message from a gateway.
 *
 * @details This function updates MQTT-SN Gateway information.
 *
 * @param[in]    p_event  Pointer to MQTT-SN event.
 */
static void gateway_info_callback(mqttsn_event_t * p_event)
{
    m_gateway_addr  = *(p_event->event_data.connected.p_gateway_addr);
    m_gateway_id    = p_event->event_data.connected.gateway_id;
}


/**@brief Processes CONNACK message from a gateway.
 *
 * @details This function launches the topic registration procedure if necessary.
 */
static void connected_callback(void)
{
    light_on();

    uint32_t err_code = mqttsn_client_topic_register(&m_client,
                                                     m_topic.p_topic_name,
                                                     strlen(m_topic_name),
                                                     &m_msg_id);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("REGISTER message could not be sent. Error code: 0x%x\r\n", err_code);
    }
}


/**@brief Processes DISCONNECT message from a gateway. */
static void disconnected_callback(void)
{
    light_off();
}


/**@brief Processes REGACK message from a gateway.
 *
 * @param[in] p_event Pointer to MQTT-SN event.
 */
static void regack_callback(mqttsn_event_t * p_event)
{
    m_topic.topic_id = p_event->event_data.registered.packet.topic.topic_id;
    NRF_LOG_INFO("MQTT-SN event: Topic has been registered with ID: %d.\r\n",
                 p_event->event_data.registered.packet.topic.topic_id);
}


/**@brief Processes data published by a broker.
 *
 * @details This function processes LED command.
 */
static void received_callback(mqttsn_event_t * p_event)
{
    //if (p_event->event_data.published.packet.topic.topic_id == m_topic.topic_id)
    //{
    NRF_LOG_INFO("MQTT-SN event: Content to subscribed topic received.\r\n");
    led_update(p_event->event_data.published.p_payload);
    //}
    //else
    //{
    //    NRF_LOG_INFO("MQTT-SN event: Content to unsubscribed topic received. Dropping packet.\r\n");
    //}
}


/**@brief Processes retransmission limit reached event. */
static void timeout_callback(mqttsn_event_t * p_event)
{
    NRF_LOG_INFO("MQTT-SN event: Timed-out message: %d. Message ID: %d.\r\n",
                  p_event->event_data.error.msg_type,
                  p_event->event_data.error.msg_id);
}


/**@brief Processes results of gateway discovery procedure. */
static void searchgw_timeout_callback(mqttsn_event_t * p_event)
{
    NRF_LOG_INFO("MQTT-SN event: Gateway discovery result: 0x%x.\r\n", p_event->event_data.discovery);
}


/**@brief Function for handling MQTT-SN events. */
void mqttsn_evt_handler(mqttsn_client_t * p_client, mqttsn_event_t * p_event)
{
    switch(p_event->event_id)
    {
        case MQTTSN_EVENT_GATEWAY_FOUND:
            NRF_LOG_INFO("MQTT-SN event: Client has found an active gateway.\r\n");
            gateway_info_callback(p_event);
            break;

        case MQTTSN_EVENT_CONNECTED:
            NRF_LOG_INFO("MQTT-SN event: Client connected.\r\n");
            connected_callback();
            break;

        case MQTTSN_EVENT_DISCONNECT_PERMIT:
            NRF_LOG_INFO("MQTT-SN event: Client disconnected.\r\n");
            disconnected_callback();
            break;

        case MQTTSN_EVENT_REGISTERED:
            NRF_LOG_INFO("MQTT-SN event: Client registered topic.\r\n");
            regack_callback(p_event);
            break;

        case MQTTSN_EVENT_SUBSCRIBED:
            NRF_LOG_INFO("MQTT-SN event: Client subscribed to topic.\r\n");
            break;

        case MQTTSN_EVENT_UNSUBSCRIBED:
            NRF_LOG_INFO("MQTT-SN event: Client unsubscribed to topic.\r\n");
            break;

        case MQTTSN_EVENT_RECEIVED:
            NRF_LOG_INFO("MQTT-SN event: Client received content.\r\n");
            received_callback(p_event);
            break;

        case MQTTSN_EVENT_TIMEOUT:
            NRF_LOG_INFO("MQTT-SN event: Retransmission retries limit has been reached.\r\n");
            timeout_callback(p_event);
            break;

        case MQTTSN_EVENT_SEARCHGW_TIMEOUT:
            NRF_LOG_INFO("MQTT-SN event: Gateway discovery procedure has finished.\r\n");
            searchgw_timeout_callback(p_event);
            break;

        default:
            break;
    }
}
/***************************************************************************************************
 * @section Buttons
 **************************************************************************************************/

static void subscribe(void)
{
    uint8_t  topic_name_len = strlen(m_topic_name);
    uint32_t err_code       = NRF_SUCCESS;

    if (m_subscribed)
    {
        err_code = mqttsn_client_unsubscribe(&m_client, m_topic.p_topic_name, topic_name_len, &m_msg_id);
        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_ERROR("UNSUBSCRIBE message could not be sent.\r\n");
        }
        else
        {
            m_subscribed = false;
        }
    }
    else
    {
        err_code = mqttsn_client_subscribe(&m_client, m_topic.p_topic_name, topic_name_len, &m_msg_id);
        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_ERROR("SUBSCRIBE message could not be sent.\r\n");
        }
        else
        {
            m_subscribed = true;
        }
    }
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    if (otThreadGetDeviceRole(thread_ot_instance_get()) < OT_DEVICE_ROLE_CHILD )
    {
        (void)event;
        return;
    }

    switch (event)
    {
        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0
        
        case BSP_EVENT_KEY_1:
        {
            uint32_t err_code = mqttsn_client_search_gateway(&m_client, SEARCH_GATEWAY_TIMEOUT);
            if (err_code != NRF_SUCCESS)
            {
                NRF_LOG_ERROR("SEARCH GATEWAY message could not be sent. Error: 0x%x\r\n", err_code);
            }
            break;
        }
       
        case BSP_EVENT_KEY_2:
        {
            uint32_t err_code;

            if (mqttsn_client_state_get(&m_client) == MQTTSN_CLIENT_CONNECTED)
            {
                err_code = mqttsn_client_disconnect(&m_client);
                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG_ERROR("DISCONNECT message could not be sent. Error: 0x%x\r\n", err_code);
                }
            }
            else
            {
                err_code = mqttsn_client_connect(&m_client, &m_gateway_addr, m_gateway_id, &m_connect_opt);
                if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG_ERROR("CONNECT message could not be sent. Error: 0x%x\r\n", err_code);
                }
            }
            break;
        }

        case BSP_EVENT_KEY_3:
        {
            subscribe();
            break;
        }

        default:
            break;
    }
}
/***************************************************************************************************
 * @section State
 **************************************************************************************************/
/**@brief Function that is called when the device Thread state changes.
 */
static void thread_state_changed_callback(uint32_t flags, void * p_context)
{
    NRF_LOG_INFO("State changed! Flags: 0x%08x Current role: %d\r\n",
                 flags, otThreadGetDeviceRole(p_context));
}
/***************************************************************************************************
 * @section Initialization
 **************************************************************************************************/

/**@brief Function for initializing the MQTTSN client.
 */
static void mqttsn_init(void)
{
    uint32_t err_code = mqttsn_client_init(&m_client,
                                           MQTTSN_DEFAULT_CLIENT_PORT,
                                           mqttsn_evt_handler,
                                           thread_ot_instance_get());
    APP_ERROR_CHECK(err_code);

    connect_opt_init();
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void application_timers_start(void)
{
}

/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

/**@brief Function for initializing the Thread stack.
 */
static void thread_instance_init(void)
{
    thread_configuration_t  thread_configuration =
    {
        .radio_mode        = THREAD_RADIO_MODE_RX_ON_WHEN_IDLE,
        .autocommissioning = true,
        .autostart_disable = false,
    };

    thread_init(&thread_configuration);
    thread_cli_init();
    thread_state_changed_callback_set(thread_state_changed_callback);
}


/**@brief Function for initializing scheduler module.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

static void thread_coap_fwd_init(void)
{
  const thread_coap_fwd_utils_configuration_t thread_coap_configuration_t = 
  {
    .forwarder_enabled = true,
    .end_node_enabled  = false
  };
  
  thread_coap_fwd_utils_init(&thread_coap_configuration_t);
}



/***************************************************************************************************
 * @section Main
 **************************************************************************************************/

/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;
    
    // Initialize.
    log_init();
    scheduler_init();
    buttons_leds_init(&erase_bonds);
    ble_main_init();

    // Start execution.
    NRF_LOG_INFO("Forwarder let's go!");
    application_timers_start();
    
    // Init Thread stack.
    thread_instance_init();
    thread_coap_fwd_init();
    mqttsn_init();

    advertising_start(erase_bonds);

    // Enter main loop.
    while (true)
    {
        thread_process();
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false)
        {
            thread_sleep();
        }
    }
}


/**
 * @}
 */
