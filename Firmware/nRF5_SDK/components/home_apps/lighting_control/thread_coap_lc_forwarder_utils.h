/**	Do Ngoc Nhuan, 09/03/2019
 *	OpenThread CoAP server-client utilities for forwarding BLE message duty in Lighting Control Application.
 *	Note: CoAP client combine with BLE Forwarder Service to perform forwarding BLE message to CoAP server on other nodes.
 */
#ifndef THREAD_COAP_LC_FORWARDER_UTILS_H__
#define THREAD_COAP_LC_FORWARDER_UTILS_H__

#include <stdbool.h>
#include "app_timer.h"
#include "app_error.h"
#include "nrf_assert.h"
#include "sdk_config.h"
#include "nrf_drv_rng.h"

#include "thread_utils.h"
#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/platform/alarm-milli.h>

#include "lighting_control.h"
#include "lc_string_handling.h"

#define THREAD_COAP_MAX_LENGTH_OF_MESSAGE	32
#define THREAD_COAP_DISCOVER_TIMEOUT			10000	//miliseconds
#define MAX_RANDOM_RESPONSE_TIMEOUT				9900	//miliseconds
typedef void (* lc_thread_coap_response_callback_t)(uint8_t * p_response_message, uint16_t length);

void thread_coap_for_lighting_control_forwarder_init(void);
void thread_coap_lighting_control_forwarder_callback_set(lc_thread_coap_response_callback_t callback);
void thread_coap_unicast_light_request_send(const uint8_t *	peer_addr_string,
											const uint8_t *	message_to_send,
											int16_t 		length);
void thread_coap_unicast_unpeer_light_request_send(bool older_address);
void thread_coap_discover_start(void);

#endif /* THREAD_COAP_LC_FORWARDER_UTILS_H__ */
