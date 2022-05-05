#include "ble_main.h"
#include "openthread_main.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_QUEUE_SIZE                32                                      /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE         /**< Maximum app_scheduler event size. */

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
}

/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       ret_code_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */

}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated when button is pressed.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

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
                err_code = ble_advertising_restart_without_whitelist(p_m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0
        case BSP_EVENT_KEY_0:
        {
			mqttsn_lc_connect_to_gateway(STARTING_CONNECT_PROCESS);
        }
			break;
        case BSP_EVENT_KEY_1:
        {
            mqttsn_lc_disconnect_to_gateway();
        }
			break;
        case BSP_EVENT_KEY_3:
        {
			if (lc_auto_mode_is_on())	{
				lc_set_off_auto_mode();
				NRF_LOG_INFO("Lighting is in manual mode.");
			} else	{
				lc_set_on_auto_mode();
				NRF_LOG_INFO("Lighting is in automatic mode.");
			}
            break;
        }

        default:
            break;
    }
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

#if BSP_BLE_THREAD_DYN
static void thread_bsp_init(void)
{
	ret_code_t err_code;
	
    err_code = bsp_thread_init(thread_ot_instance_get());
    APP_ERROR_CHECK(err_code);
}
#endif // BSP_BLE_THREAD_DYN

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing scheduler module.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/***************************************************************************************************
 * @section Main
 **************************************************************************************************/

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    scheduler_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
	
	lighting_control_init();
	ble_for_lighting_control_init();
	
	// Init Thread stack.
	thread_instance_init();
#if BSP_BLE_THREAD_DYN
	thread_bsp_init();
#endif // BSP_BLE_THREAD_DYN
	mqttsn_for_lighting_control_init();
	thread_coap_for_lighting_control_forwarder_init();
	
    // Start execution.
    NRF_LOG_INFO("BLE-Thread Dynamic Lighting Control Application started.");
    application_timers_start();
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
