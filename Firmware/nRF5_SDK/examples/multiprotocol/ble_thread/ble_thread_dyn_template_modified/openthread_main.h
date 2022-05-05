#ifndef __OPENTHREAD_MAIN_H__
#define __OPENTHREAD_MAIN_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "mqttsn_client.h"
#include "thread_utils.h"
#if BSP_BLE_THREAD_DYN
#include "bsp_thread.h"
#endif // BSP_BLE_THREAD_DYN

#include <openthread/diag.h>
#include <openthread/coap.h>
#include <openthread/cli.h>
#include <openthread/thread_ftd.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/platform-softdevice.h>

#include "thread_coap_lc_forwarder_utils.h"
#include "lighting_control.h"
#include "lc_string_handling.h"

#define MAIN_OT_SEARCH_GATEWAY_TIMEOUT 						5                                            /**< MQTT-SN Gateway discovery procedure timeout in [s]. */
#define MAIN_OT_MAX_MESSAGE_FOR_SENDING_LENGTH		MEMORY_MANAGER_SMALL_BLOCK_SIZE


typedef enum mqttsn_connect_steps_t
{
	STARTING_CONNECT_PROCESS = 0,
	SEARCHING_GATEWAY_STEP = 0,
	CONNECTING_STEP,
	REGISTERING_TOPIC_1_STEP,
	REGISTERING_TOPIC_2_STEP,
	SUBSCRIBING_TOPIC_1_STEP,
	CONNECTED_SUCCESSFULLY,
} mqttsn_connect_steps_t;

extern mqttsn_client_t      m_client;    
extern uint16_t             m_msg_id;
extern mqttsn_connect_steps_t connect_to_gateway_step;
extern mqttsn_topic_t       m_topic_1;
extern mqttsn_topic_t       m_topic_2;

void mqttsn_lc_disconnect_to_gateway(void);
void mqttsn_lc_connect_to_gateway(mqttsn_connect_steps_t step);
void mqttsn_for_lighting_control_init(void);

void thread_instance_init(void);

static void mqttsn_init(void);
static void connect_opt_init(void);
static void gateway_info_callback(mqttsn_event_t * p_event);
static void connected_callback(void);
static void disconnected_callback(void);
static void regack_callback(mqttsn_event_t * p_event);
static void subscribed_callback(void);
static void timeout_callback(mqttsn_event_t * p_event);
static void searchgw_timeout_callback(mqttsn_event_t * p_event);
static void mqttsn_evt_handler(mqttsn_client_t * p_client, mqttsn_event_t * p_event);

static void lighting_control_evt_callback(lighting_control_info_t * p_lighting_control_info);
static void lighting_control_auto_cfg(void);
static void recognize_rcv_command(mqttsn_event_publish_t * rcv_data);

#endif // __OPENTHREAD_MAIN_H__
