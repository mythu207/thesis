#include "thread_coap_fwd_utils.h"

#include "app_timer.h"
#include "bsp_thread.h"
#include "nrf_assert.h"
#include "nrf_log.h"
#include "sdk_config.h"
#include "thread_utils.h"
//#include "ble_main.h"
#include "nrf_drv_rng.h"

#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/platform/alarm-milli.h>

#define THREAD_COAP_DISCOVER_TIMEOUT           10000

thread_coap_fwd_utils_configuration_t   m_config;

static bool thread_discover_in_progress = false;
uint16_t discovered_count;

static uint8_t receivedMessage[THREAD_COAP_MAX_LENGTH_OF_MESSAGE];
static uint16_t receivedMessageLength;

APP_TIMER_DEF(m_fwd_discover_timer_id);
APP_TIMER_DEF(m_random_response_timer_id);

static thread_coap_utils_light_command_handler_t m_light_command_handler;

static void discovery_request_handler(void *, otMessage *, const otMessageInfo *);
static void discovery_response_handler(void *, otMessage *, const otMessageInfo *);
static void light_request_handler(void *, otMessage *, const otMessageInfo *);

otIp6Address  forwarderAddress, endNodeAddress;

static otCoapResource m_fwd_discovery_resource =
{
  .mUriPath = "disc_server",
  .mHandler = discovery_response_handler,
  .mContext = NULL,
  .mNext    = NULL,
};

static otCoapResource m_end_discovery_resource =
{
  .mUriPath = "disc_client",
  .mHandler = discovery_request_handler,
  .mContext = NULL,
  .mNext    = NULL,
};

static otCoapResource m_light_resource =
{
  .mUriPath = "light",
  .mHandler = light_request_handler,
  .mContext = NULL,
  .mNext    = NULL,
};


static void coap_default_handler(void                * p_context,
                                 otMessage           * p_message,
                                 const otMessageInfo * p_message_info)
{
    UNUSED_PARAMETER(p_context);
    UNUSED_PARAMETER(p_message);
    UNUSED_PARAMETER(p_message_info);

    NRF_LOG_INFO("Received CoAP message that does not match any request or resource\r\n");
}

void thread_coap_discover_timeout_handler(void * p_context)
{
  thread_discover_in_progress = false;
  NRF_LOG_INFO("Print address");
  //thread_coap_fwd_callback((uint8_t *)&discovered_count, sizeof(discovered_count));
}

void random_response_timer_timeout_handler(void * p_context)
{ 
   thread_coap_fwd_utils_ucast_discover_response_send();

}


void thread_coap_fwd_utils_mcast_discover_request_send(void)	
{
	NRF_LOG_INFO("Thu: thread_coap_fwd_utils_mcast_discover_request_send()");
        
        otError       error = OT_ERROR_NONE;
        otMessage   * p_request;
        otMessageInfo message_info;
        const char * p_scope = NULL;
        otInstance * p_instance = thread_ot_instance_get();
	
	do
        {
 
            p_request = otCoapNewMessage(p_instance, NULL);
            if (p_request == NULL)
            {
                NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
                break;
            }
            
            otCoapMessageInit(p_request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);
            
            error = otCoapMessageAppendUriPathOptions(p_request, "disc_client");
            ASSERT(error == OT_ERROR_NONE);
            
            p_scope = "ff03::1";

            memset(&message_info, 0, sizeof(message_info));
            message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
            
            error = otIp6AddressFromString(p_scope, &message_info.mPeerAddr);
            ASSERT(error == OT_ERROR_NONE);

            NRF_LOG_INFO("Thu: thread_coap_fwd_utils_mcast_discover_request_send(): Sending Request");
            
            error = otCoapSendRequest(p_instance, p_request, &message_info, NULL, NULL);
        } while (false);
	
    	if (error != OT_ERROR_NONE && p_request != NULL)
        {
            NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
            otMessageFree(p_request);
        }
}

static void discovery_request_handler(void                * p_context,
                                      otMessage           * p_message,
                                      const otMessageInfo * p_message_info)
{
  NRF_LOG_INFO("Thu: discovery_request_handler()");
  
  otMessageInfo message_info;

  do
  {
      if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE &&
          otCoapMessageGetCode(p_message) != OT_COAP_CODE_POST)
      {
  	break;
      }
      message_info = *p_message_info;
      
      memcpy(&forwarderAddress, &p_message_info->mPeerAddr, sizeof(otIp6Address));
      
      uint8_t random;
      random = rand() % 10;
      
      ret_code_t err_code = app_timer_start(m_random_response_timer_id, APP_TIMER_TICKS((random+1)*MAX_RANDOM_RESPONSE_TIMEOUT/256), NULL);
      ASSERT(err_code == NRF_SUCCESS);  
  } while (false);
}

static void discovery_response_handler(void               * p_context,
                                      otMessage           * p_message,
                                      const otMessageInfo * p_message_info)
{
    NRF_LOG_INFO("Thu: discovery_response_handler");
    UNUSED_PARAMETER(p_context);

    if(thread_discover_in_progress)
    {
      discovered_count++;
      UNUSED_RETURN_VALUE(otMessageRead(p_message,
                                      otMessageGetOffset(p_message),
                                      receivedMessage,
                                      sizeof(otIp6Address)));
      //thread_coap_fwd_callback(receivedMessage, sizeof(otIp6Address));
    }
    
    
}

static void light_request_handler(void                * p_context,
                                      otMessage           * p_message,
                                      const otMessageInfo * p_message_info)
{
  NRF_LOG_INFO("Thu: light_request_handler()");
  uint8_t command;
  
  do
  {
    if (otCoapMessageGetType(p_message) != OT_COAP_TYPE_CONFIRMABLE &&
        otCoapMessageGetType(p_message) != OT_COAP_TYPE_NON_CONFIRMABLE)
        {
            break;
        }

        if (otCoapMessageGetCode(p_message) != OT_COAP_CODE_PUT)
        {
            break;
        }

        if (otMessageRead(p_message, otMessageGetOffset(p_message), &command, 1) != 1)
        {
            NRF_LOG_INFO("light handler - missing command\r\n");
        }

        m_light_command_handler(THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE);

  }while(false);
}
void thread_coap_fwd_utils_ucast_discover_response_send(void)
{
        NRF_LOG_INFO("Thu: thread_coap_fwd_utils_ucast_discover_response_send");
        otError       error = OT_ERROR_NONE;
        otMessage   * p_response;
        otMessageInfo message_info;
        otInstance * p_instance = thread_ot_instance_get();
	
	do
        {
 
            p_response = otCoapNewMessage(p_instance, NULL);
            if (p_response == NULL)
            {
                NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
                break;
            }
            
            otCoapMessageInit(p_response, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);
            otCoapMessageGenerateToken(p_response, 2);

            error = otCoapMessageAppendUriPathOptions(p_response, "disc_server");
            ASSERT(error == OT_ERROR_NONE);

            error = otCoapMessageSetPayloadMarker(p_response);
            ASSERT(error == OT_ERROR_NONE);
            
            memset(&message_info, 0, sizeof(message_info));
            message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
            memcpy(&message_info.mPeerAddr, &forwarderAddress, sizeof(otIp6Address));
            

            error = otMessageAppend(p_response, otThreadGetMeshLocalEid(p_instance), sizeof(otIp6Address));
            if (error != OT_ERROR_NONE)
            {
                break;
            }
    
            NRF_LOG_INFO("Thu: thread_coap_fwd_utils_ucast_discover_response_send(): Sending Request");
            
            error = otCoapSendRequest(p_instance, p_response, &message_info, NULL, NULL);
        } while (false);
	
    	if (error != OT_ERROR_NONE && p_response != NULL)
        {
            NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
            otMessageFree(p_response);
        }
	

  
}

static void light_changed_default(thread_coap_utils_light_command_t light_command)
{
    NRF_LOG_INFO("Thu: light_changed_default()"); 
    switch (light_command)
    {
        case THREAD_COAP_UTILS_LIGHT_CMD_ON:
            NRF_LOG_INFO("Led on");
            LEDS_ON(BSP_LED_3_MASK);
            break;

        case THREAD_COAP_UTILS_LIGHT_CMD_OFF:
            LEDS_OFF(BSP_LED_3_MASK);
            break;

        case THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE:
            LEDS_INVERT(BSP_LED_3_MASK);
            LEDS_INVERT(BSP_LED_4_MASK);
            break;

        default:
            break;
    }
}

void thread_coap_fwd_utils_init(const thread_coap_fwd_utils_configuration_t * p_config)
{
    otInstance * p_instance = thread_ot_instance_get();
    
    otError error = otCoapStart(p_instance, OT_DEFAULT_COAP_PORT);
    ASSERT(error == OT_ERROR_NONE);
    
    otCoapSetDefaultHandler(p_instance, coap_default_handler, NULL);
    
    m_config = *p_config;
    
    if(m_config.forwarder_enabled)
    {
      NRF_LOG_INFO("Thu: forwarder node");
      
      m_fwd_discovery_resource.mContext = p_instance;
      m_light_resource.mContext = p_instance;

      uint32_t retval = app_timer_create(&m_fwd_discover_timer_id,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        thread_coap_discover_timeout_handler);

      ASSERT(retval == NRF_SUCCESS);

      error = otCoapAddResource(p_instance, &m_fwd_discovery_resource);
      ASSERT(error == OT_ERROR_NONE);
      
      error = otCoapAddResource(p_instance, &m_light_resource);
      ASSERT(error == OT_ERROR_NONE);
    }

    else if(m_config.end_node_enabled)
    {
      NRF_LOG_INFO("Thu: end node");
      
      m_end_discovery_resource.mContext = p_instance;
      m_light_resource.mContext = p_instance;
      
      m_light_command_handler = light_changed_default;

      uint32_t retval = app_timer_create(&m_random_response_timer_id,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        random_response_timer_timeout_handler);
      ASSERT(retval == NRF_SUCCESS);

      error = otCoapAddResource(p_instance, &m_end_discovery_resource);
      ASSERT(error == OT_ERROR_NONE);
      
      error = otCoapAddResource(p_instance, &m_light_resource);
      ASSERT(error == OT_ERROR_NONE);

    }
}

void thread_coap_discover_start(void)
{
  NRF_LOG_INFO("discovering");
  thread_discover_in_progress = true;
  discovered_count = 0;
  ret_code_t err_code = app_timer_start(m_fwd_discover_timer_id,
                                        APP_TIMER_TICKS(THREAD_COAP_DISCOVER_TIMEOUT),
                                        NULL);
  ASSERT(err_code == NRF_SUCCESS);
  thread_coap_fwd_utils_mcast_discover_request_send();
}



