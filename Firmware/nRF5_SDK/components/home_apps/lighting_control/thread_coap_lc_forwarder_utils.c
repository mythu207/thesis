/**	Do Ngoc Nhuan, 09/03/2019
 *	OpenThread CoAP server-client utilities for forwarding BLE message duty in Lighting Control Application.
 *	Note: CoAP client combine with BLE Forwarder Service to perform forwarding BLE message to CoAP server on other nodes.
 */
 
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(THREAD_COAP_LC_FORWARDER_UTILS)
#include "thread_coap_lc_forwarder_utils.h"

#define NRF_LOG_MODULE_NAME thread_coap
#if THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_DEBUG_COLOR
#else // THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // THREAD_COAP_LC_FORWARDER_UTILS_CONFIG_LOG_ENABLED
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

APP_TIMER_DEF(m_thread_discover_timer_id);
APP_TIMER_DEF(m_random_response_timer_id);
static bool thread_discover_on_progress;
static uint16_t	discovered_count;

static otCoapResource lightResource;
static otCoapResource forwarderResource;

static const char * lightUriPath = "light";
static const char * forwarderUriPath = "forwarder";

static otIp6Address forwarderAddress, olderDestinationAddress, destinationAddrress, discoverAddress;

static uint8_t receivedMessage[THREAD_COAP_MAX_LENGTH_OF_MESSAGE];
static uint16_t receivedMessageLength;

//static bool messageFromCoap;

static const otIp6Address m_unspecified_ipv6 =
{
    .mFields =
    {
        .m8 = {0}
    }
};

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void lc_thread_coap_light_response_default_callback(uint8_t * p_response_message, uint16_t length);
static void random_response_timeout_handler(void * p_context);
static void thread_discover_timeout_handler(void * p_context);
void thread_coap_unicast_light_request_send(const uint8_t *	peer_addr_string,
											const uint8_t *	message_to_send,
											int16_t 		length);
											
static void thread_coap_unicast_forwarder_request_send(	const uint8_t *	message_to_send,
														const uint16_t 	length);
												
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static lc_thread_coap_response_callback_t m_registered_callback = lc_thread_coap_light_response_default_callback;
														
														
static void thread_coap_handler_default(void                * p_context,
										otCoapHeader        * p_header,
										otMessage           * p_message,
										const otMessageInfo * p_message_info)
{
    (void)p_context;
    (void)p_header;
    (void)p_message;
    (void)p_message_info;

    NRF_LOG_INFO("Received CoAP message that does not match any request or resource\r\n");
}

void thread_coap_acknowlegdement_send( void                * p_context,
									   otCoapHeader        * p_request_header,
									   const otMessageInfo * p_message_info)
{
    otError      error = OT_ERROR_NO_BUFS;
    otCoapHeader header;
    otMessage  * p_response;
    do
    {
        otCoapHeaderInit(&header, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_VALID);
        otCoapHeaderSetMessageId(&header, otCoapHeaderGetMessageId(p_request_header));
        otCoapHeaderSetToken(&header,
                             otCoapHeaderGetToken(p_request_header),
                             otCoapHeaderGetTokenLength(p_request_header));

        p_response = otCoapNewMessage(p_context, &header);
        if (p_response == NULL)
        {
            break;
        }

        error = otCoapSendResponse(p_context, p_response, p_message_info);

    } while (false);

    if ((error != OT_ERROR_NONE) && (p_response != NULL))
    {
        otMessageFree(p_response);
    }
}

static void thread_coap_light_request_handler(	void 				* p_context, 
												otCoapHeader 		* p_header, 
												otMessage 			* p_message, 
												const otMessageInfo * p_message_info)
{
	do	{
		if (otCoapHeaderGetType(p_header) != OT_COAP_TYPE_NON_CONFIRMABLE &&
			otCoapHeaderGetType(p_header) != OT_COAP_TYPE_CONFIRMABLE)
		{
			break;
		}
		
		if (otCoapHeaderGetCode(p_header) != OT_COAP_CODE_GET &&
			otCoapHeaderGetCode(p_header) != OT_COAP_CODE_PUT &&
			otCoapHeaderGetCode(p_header) != OT_COAP_CODE_POST)
		{
			break;
		}

		receivedMessageLength = otMessageGetLength(p_message)-otMessageGetOffset(p_message);
		otMessageRead(p_message, 
					  otMessageGetOffset(p_message), 
					  &receivedMessage, 
					  receivedMessageLength);
//		NRF_LOG_INFO("receivedMessage 1: %s", (uint32_t)receivedMessage);
//		NRF_LOG_INFO("receivedMessageLength 1: %d", receivedMessageLength);

		if (otCoapHeaderGetCode(p_header) == OT_COAP_CODE_POST &&
			receivedMessageLength == 0)
		{	
			uint8_t random;
			ret_code_t err_code = nrf_drv_rng_rand(&random, 1);
			ASSERT(err_code == NRF_SUCCESS);
			
			memcpy(&discoverAddress, &p_message_info->mPeerAddr, sizeof(discoverAddress));
			
			err_code = app_timer_start(m_random_response_timer_id, APP_TIMER_TICKS((random+1)*MAX_RANDOM_RESPONSE_TIMEOUT/256), NULL);
			ASSERT(err_code == NRF_SUCCESS);
			break;
		}

		memcpy(&forwarderAddress, &p_message_info->mPeerAddr, sizeof(forwarderAddress)); //why source Address is Peer Address?? (07/03/2019, DNN)
		
		if (otCoapHeaderGetCode(p_header) == OT_COAP_CODE_GET)	//Unpeer message
		{
			memcpy(&forwarderAddress, &m_unspecified_ipv6, sizeof(forwarderAddress));
		}

		if (otCoapHeaderGetType(p_header) == OT_COAP_TYPE_CONFIRMABLE)
		{
			thread_coap_acknowlegdement_send(p_context, p_header, p_message_info);
		}
		
		if (otCoapHeaderGetCode(p_header) == OT_COAP_CODE_PUT)
		{
			recognize_lc_control_command(receivedMessage, receivedMessageLength);
		}
	} while (false);
}

static void thread_coap_forwarder_request_handler(	void 				* p_context, 
													otCoapHeader 		* p_header, 
													otMessage 			* p_message, 
													const otMessageInfo * p_message_info)
{
	do	{
		if (otCoapHeaderGetType(p_header) != OT_COAP_TYPE_NON_CONFIRMABLE &&
			otCoapHeaderGetType(p_header) != OT_COAP_TYPE_CONFIRMABLE)
		{
			break;
		}
		
		if (otCoapHeaderGetCode(p_header) != OT_COAP_CODE_PUT &&
			otCoapHeaderGetCode(p_header) != OT_COAP_CODE_POST)
		{
			break;
		}
		
		receivedMessageLength = otMessageGetLength(p_message)-otMessageGetOffset(p_message);
		otMessageRead(p_message, 
					  otMessageGetOffset(p_message), 
					  &receivedMessage, 
					  receivedMessageLength);
		
//		NRF_LOG_INFO("receivedMessage 2: %s", (uint32_t)receivedMessage);
//		NRF_LOG_INFO("receivedMessageLength 2: %d", receivedMessageLength);

		if (otCoapHeaderGetCode(p_header) == OT_COAP_CODE_POST &&
			receivedMessageLength > 0)	
		{
			if (!thread_discover_on_progress)	{
				break;
			}
			
			otIp6Address discoveredAddr;
			memcpy(&discoveredAddr, receivedMessage, receivedMessageLength);
			
			if (otIp6IsAddressUnspecified(&discoveredAddr))	{
				break;
			}
			
			discovered_count++;
		}

		if (otCoapHeaderGetType(p_header) == OT_COAP_TYPE_CONFIRMABLE)
		{
			thread_coap_acknowlegdement_send(p_context, p_header, p_message_info);
		}
		
		m_registered_callback(receivedMessage, receivedMessageLength);
		
	} while (false);
}

static void lighting_control_event_callback(lighting_control_info_t * p_lighting_control_info)
{
	uint8_t * textMessage = (uint8_t *)nrf_malloc(THREAD_COAP_MAX_LENGTH_OF_MESSAGE*sizeof(uint8_t));
	memset(textMessage, NULL, sizeof(textMessage));
	
	make_msg_lc_light_info(p_lighting_control_info, textMessage);
	
	thread_coap_unicast_forwarder_request_send(	textMessage, 
												strlen((char *)textMessage));
	
	nrf_free(textMessage);
}

static void lc_string_handling_event_callback(uint8_t * p_msg_to_send, uint8_t length)
{
	thread_coap_unicast_forwarder_request_send(	p_msg_to_send, 
												length);
}

static void lc_thread_coap_light_response_default_callback(uint8_t * p_response_message, uint16_t length)
{
	NRF_LOG_INFO("p_response_message: %s", (uint32_t)p_response_message);
	NRF_LOG_INFO("length: %d", length);
}

void thread_coap_for_lighting_control_forwarder_init(void)
{
	otError error = otCoapStart(thread_ot_instance_get(), OT_DEFAULT_COAP_PORT);
	ASSERT(error == OT_ERROR_NONE);
	
	otCoapSetDefaultHandler(thread_ot_instance_get(), thread_coap_handler_default, NULL);
	
	lightResource.mContext	= thread_ot_instance_get();
	lightResource.mHandler 	= thread_coap_light_request_handler;
	lightResource.mNext		= NULL;
	lightResource.mUriPath	= lightUriPath;
	
	forwarderResource.mContext	= thread_ot_instance_get();
	forwarderResource.mHandler	= thread_coap_forwarder_request_handler;
	forwarderResource.mNext		= NULL;
	forwarderResource.mUriPath	= forwarderUriPath;
	
	error = otCoapAddResource(thread_ot_instance_get(), &lightResource);
	ASSERT(error == OT_ERROR_NONE);
	error = otCoapAddResource(thread_ot_instance_get(), &forwarderResource);
	ASSERT(error == OT_ERROR_NONE);
	
	ret_code_t err_code;
	
	err_code = lighting_control_callback_register(lighting_control_event_callback);
	ASSERT(err_code == OT_ERROR_NONE);
	err_code = lc_string_handling_callback_register(lc_string_handling_event_callback);
	ASSERT(err_code == OT_ERROR_NONE);
	
	err_code = app_timer_create(&m_thread_discover_timer_id, 
											APP_TIMER_MODE_SINGLE_SHOT, 
											thread_discover_timeout_handler);
	ASSERT(err_code == NRF_SUCCESS);
	
	err_code = app_timer_create(&m_random_response_timer_id, 
											APP_TIMER_MODE_SINGLE_SHOT, 
											random_response_timeout_handler);
	ASSERT(err_code == NRF_SUCCESS);
	
	err_code = nrf_drv_rng_init(NULL);
    ASSERT(err_code == NRF_SUCCESS || err_code == NRF_ERROR_MODULE_ALREADY_INITIALIZED);
}

void thread_coap_lighting_control_forwarder_callback_set(lc_thread_coap_response_callback_t callback)
{
	m_registered_callback = callback;
}

static void thread_coap_light_response_handler(	void                * p_context,
												otCoapHeader        * p_header,
												otMessage           * p_message,
												const otMessageInfo * p_message_info,
												otError               result)
{
	(void)p_context;
    (void)p_header;

    if (result != OT_ERROR_NONE)
    {
		memcpy(&destinationAddrress, &m_unspecified_ipv6, sizeof(destinationAddrress));
    }
}

static void thread_coap_forwarder_response_handler(	void                * p_context,
													otCoapHeader        * p_header,
													otMessage           * p_message,
													const otMessageInfo * p_message_info,
													otError               result)
{
	(void)p_context;
    (void)p_header;

    if (result != OT_ERROR_NONE)
    {
		memcpy(&forwarderAddress, &m_unspecified_ipv6, sizeof(forwarderAddress));
    }
}

void thread_coap_unicast_unpeer_light_request_send(bool older_address)
{
    otError       	error = OT_ERROR_NONE;
	otInstance 	  * p_instance;
    otMessage     *	p_message;
    otMessageInfo 	message_info;
    otCoapHeader  	header;
	otIp6Address  * peer_address;
	
	p_instance = thread_ot_instance_get();
	if (older_address)	{
		peer_address = &destinationAddrress;
	} else	{
		peer_address = &olderDestinationAddress;
	}
	do
    {	
		ASSERT(error == OT_ERROR_NONE);
        if (otIp6IsAddressEqual(peer_address, &m_unspecified_ipv6))
        {
            NRF_LOG_INFO("Failed to send the CoAP Request to the Unspecified IPv6 Address\r\n");
            break;
        }

		otCoapHeaderInit(&header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
		otCoapHeaderGenerateToken(&header, 2);
		UNUSED_VARIABLE(otCoapHeaderAppendUriPathOptions(&header, lightUriPath));
		p_message = otCoapNewMessage(p_instance, &header);
		if (p_message == NULL)
		{
			NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
			break;
		}
		
        memset(&message_info, 0, sizeof(message_info));
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
        memcpy(&message_info.mPeerAddr, peer_address, sizeof(otIp6Address));

        error = otCoapSendRequest(p_instance,
                                  p_message,
                                  &message_info,
                                  NULL,
                                  p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

void thread_coap_unicast_light_request_send(const uint8_t *	peer_addr_string,
											const uint8_t *	message_to_send,
											int16_t 		length)
{
    otError       error = OT_ERROR_NONE;
	otInstance  * p_instance;
    otMessage   * p_message;
    otMessageInfo message_info;
    otCoapHeader  header;
	
	p_instance = thread_ot_instance_get();
    do
    {
		memcpy(&olderDestinationAddress, &destinationAddrress, sizeof(olderDestinationAddress));
		
		error = otIp6AddressFromString((char *)peer_addr_string, &destinationAddrress);
		if (error != OT_ERROR_NONE)	{
			NRF_LOG_INFO("Destination address is not IPv6 address\r\n");
			break;
		}
		
        if (otIp6IsAddressEqual(&destinationAddrress, &m_unspecified_ipv6))
        {
            NRF_LOG_INFO("Failed to send the CoAP Request to the Unspecified IPv6 Address\r\n");
            break;
        }

		otCoapHeaderInit(&header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
		otCoapHeaderGenerateToken(&header, 2);
		UNUSED_VARIABLE(otCoapHeaderAppendUriPathOptions(&header, lightUriPath));
		UNUSED_VARIABLE(otCoapHeaderSetPayloadMarker(&header));
		p_message = otCoapNewMessage(p_instance, &header);
		if (p_message == NULL)
		{
			NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
			break;
		}
		error = otMessageAppend(p_message, message_to_send, length);
		if (error != OT_ERROR_NONE)
		{
			break;
		}

        memset(&message_info, 0, sizeof(message_info));
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
        memcpy(&message_info.mPeerAddr, &destinationAddrress, sizeof(destinationAddrress));

        error = otCoapSendRequest(p_instance,
                                  p_message,
                                  &message_info,
                                  thread_coap_light_response_handler,
                                  p_instance);
    } while (false);

	if (!otIp6IsAddressEqual(&destinationAddrress, &olderDestinationAddress))
	{
		thread_coap_unicast_unpeer_light_request_send(true);	//unpeer to older Node.
	}
	
    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

static void thread_coap_unicast_forwarder_request_send(	const uint8_t *	message_to_send,
														const uint16_t 	length)
{
    otError       error = OT_ERROR_NONE;
	otInstance	* p_instance;
    otMessage   * p_message;
    otMessageInfo message_info;
    otCoapHeader  header;
	
	p_instance = thread_ot_instance_get();
    do
    {	
        if (otIp6IsAddressEqual(&forwarderAddress, &m_unspecified_ipv6))
        {
            NRF_LOG_INFO("Failed to send the CoAP Request to the Unspecified IPv6 Address\r\n");
            break;
        }

        otCoapHeaderInit(&header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
        otCoapHeaderGenerateToken(&header, 2);
        UNUSED_VARIABLE(otCoapHeaderAppendUriPathOptions(&header, forwarderUriPath));
        UNUSED_VARIABLE(otCoapHeaderSetPayloadMarker(&header));

        p_message = otCoapNewMessage(p_instance, &header);
        if (p_message == NULL)
        {
            NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
            break;
        }

        error = otMessageAppend(p_message, message_to_send, length);
        if (error != OT_ERROR_NONE)
        {
            break;
        }

        memset(&message_info, 0, sizeof(message_info));
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
        memcpy(&message_info.mPeerAddr, &forwarderAddress, sizeof(forwarderAddress));

        error = otCoapSendRequest(p_instance,
                                  p_message,
                                  &message_info,
                                  thread_coap_forwarder_response_handler,
                                  p_instance);
    } while (false);

    if (error != OT_ERROR_NONE && p_message != NULL)
    {
        NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
        otMessageFree(p_message);
    }
}
static void thread_coap_ucast_discover_response_send(void)	/*Send multicast POST message with mesh local address in payload to Forwarder Resource of Discovering Node. */ 
{
	otError      error = OT_ERROR_NO_BUFS;
	otInstance * p_instance;
    otCoapHeader header;
    otMessage  * p_response;
	otMessageInfo message_info;
	
	p_instance = thread_ot_instance_get();
	do	{
		otCoapHeaderInit(&header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);
		UNUSED_VARIABLE(otCoapHeaderAppendUriPathOptions(&header, forwarderUriPath));
		UNUSED_VARIABLE(otCoapHeaderSetPayloadMarker(&header));
		
		p_response = otCoapNewMessage(p_instance, &header);
        if (p_response == NULL)
        {
            NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
            break;
        }
		
        error = otMessageAppend(
            p_response, otThreadGetMeshLocalEid(p_instance), sizeof(otIp6Address));
        if (error != OT_ERROR_NONE)
        {
            break;
        }
		
		memset(&message_info, 0, sizeof(message_info));
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
		memcpy(&message_info.mPeerAddr, &discoverAddress, sizeof(message_info.mPeerAddr));
		
		error = otCoapSendRequest(p_instance, p_response, &message_info, NULL, NULL);
	} while (false);
	
	if (error != OT_ERROR_NONE && p_response != NULL)
    {
        NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
        otMessageFree(p_response);
    }
}
static void thread_coap_mcast_discover_request_send(void)	/*Send multicast POST message with empty payload to Light Resource of nodes in network. */ 
{
	otError       error = OT_ERROR_NONE;
	otInstance  * p_instance;
    otMessage   * p_message;
    otMessageInfo message_info;
    otCoapHeader  header;
	
	p_instance = thread_ot_instance_get();
	do	{
		otCoapHeaderInit(&header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);
        UNUSED_VARIABLE(otCoapHeaderAppendUriPathOptions(&header, lightUriPath));
		
        p_message = otCoapNewMessage(p_instance, &header);
        if (p_message == NULL)
        {
            NRF_LOG_ERROR("Failed to allocate message for CoAP Request\r\n");
            break;
        }
		
		memset(&message_info, 0, sizeof(message_info));
        message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
        message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
        UNUSED_VARIABLE(otIp6AddressFromString("FF03::1", &message_info.mPeerAddr));
		
		error = otCoapSendRequest(p_instance, p_message, &message_info, NULL, NULL);
	} while (false);
	
	if (error != OT_ERROR_NONE && p_message != NULL)
    {
        NRF_LOG_ERROR("Failed to send CoAP Request: %d\r\n", error);
        otMessageFree(p_message);
    }
}

static void random_response_timeout_handler(void * p_context)
{
	thread_coap_ucast_discover_response_send();
}

static void thread_discover_timeout_handler(void * p_context)
{
	thread_discover_on_progress = false;
	/*Send to BLE device msg with number of nodes that are discovered. */

	m_registered_callback((uint8_t *)&discovered_count, sizeof(discovered_count));
}

void thread_coap_discover_start(void)
{
	thread_discover_on_progress = true;
	discovered_count			= 0;
	ret_code_t err_code = app_timer_start(	m_thread_discover_timer_id, 
											APP_TIMER_TICKS(THREAD_COAP_DISCOVER_TIMEOUT),
											NULL);
	ASSERT(err_code == NRF_SUCCESS);
	thread_coap_mcast_discover_request_send();
}

#endif // NRF_MODULE_ENABLED(THREAD_COAP_LC_FORWARDER_UTILS)
