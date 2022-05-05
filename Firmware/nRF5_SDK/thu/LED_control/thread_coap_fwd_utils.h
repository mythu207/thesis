#ifndef THREAD_COAP_FWD_UTILS_H__
#define THREAD_COAP_FWD_UTILS_H__

#include <stdbool.h>
#include "thread_utils.h"
#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/platform/alarm-milli.h>

#define THREAD_COAP_MAX_LENGTH_OF_MESSAGE	32
#define MAX_RANDOM_RESPONSE_TIMEOUT             9900	

/**@brief Thread CoAP utils configuration structure. */
typedef struct
{
    bool forwarder_enabled;                /**< Indicates if CoAP Server should be enabled. */
    bool end_node_enabled;                /**< Indicates if CoAP Client should be enabled. */
} thread_coap_fwd_utils_configuration_t;

void thread_coap_discover_start(void);
void thread_coap_fwd_utils_init(const thread_coap_fwd_utils_configuration_t * p_config);
//void thread_coap_led_request_send();
//void thread_coap_led_request_send();

#endif