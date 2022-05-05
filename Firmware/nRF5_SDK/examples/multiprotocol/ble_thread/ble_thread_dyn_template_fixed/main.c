/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file for switched BLE and Thread.
 *
 * This file contains a template for creating a new application. It has the code necessary to wake up
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection was created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 *
 * Additionally when device operates in a Thread mode, application may utilize OpenThread CLI feature.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


//#include "nrf_pwr_mgmt.h"

//BLE
#include "ble_main.h"
//BLE


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "thread_utils.h"
#include "thread_coap_fwd_utils.h"

#include <openthread/coap.h>
#include <openthread/cli.h>
#include <openthread/thread_ftd.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/platform-softdevice.h>





#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_QUEUE_SIZE                64                                      /**< Maximum number of events in the scheduler queue. */
#define SCHED_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE         /**< Maximum app_scheduler event size. */


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
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       ret_code_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */

       //app_timer_start(m_our_char_timer_id, OUR_CHAR_TIMER_INTERVAL, NULL);


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
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0

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
    NRF_LOG_INFO("BLE Thread template example started.");
    application_timers_start();
    
    // Init Thread stack.
    thread_instance_init();
    thread_coap_fwd_init();

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
