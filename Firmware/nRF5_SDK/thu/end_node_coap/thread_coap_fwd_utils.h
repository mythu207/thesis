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
#define MAX_RANDOM_RESPONSE_TIMEOUT             990	

/**@brief Thread CoAP utils configuration structure. */
typedef struct
{
    bool forwarder_enabled;                /**< Indicates if CoAP Server should be enabled. */
    bool end_node_enabled;                /**< Indicates if CoAP Client should be enabled. */
} thread_coap_fwd_utils_configuration_t;


/**@brief Enumeration describing light commands. */
typedef enum
{
    THREAD_COAP_UTILS_LIGHT_CMD_OFF = '0',
    THREAD_COAP_UTILS_LIGHT_CMD_ON = '1',
    THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE = '2'
} thread_coap_utils_light_command_t;


/**@brief Type definition of the function used to handle light resource change.
 */
typedef void (*thread_coap_utils_light_command_handler_t)(thread_coap_utils_light_command_t light_state);


void thread_coap_discover_start(void);
void thread_coap_fwd_utils_init(const thread_coap_fwd_utils_configuration_t * p_config);
//void thread_coap_led_request_send();
//void thread_coap_led_request_send();

#endif