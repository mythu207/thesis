/*	Ho Chi Minh, 08/03/2019
 *	Writer: Do Ngoc Nhuan
 *	BLE LC Forwarder Service that combine with Lighting Control Thread Coap (lc_thread_coap) to perform forwarding BLE message in a Thread network.
 */
 
#ifndef BLE_FORWARDER_SERVICE_H__
#define BLE_FORWARDER_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

/**@brief   Macro for defining a ble_lfs instance.
 *
 * @param     _name            Name of the instance.
 * @param[in] _lfs_max_clients Maximum number of lfs clients connected at a time.
 * @hideinitializer
 */
#define BLE_LFS_DEF(_name, _lfs_max_clients)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),  \
                             (_lfs_max_clients),                  \
                             sizeof(ble_lfs_client_context_t));   \
    static ble_lfs_t _name =                                      \
    {                                                             \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage) \
    };                                                            \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                           \
                         BLE_LFS_BLE_OBSERVER_PRIO,               \
                         ble_lfs_on_ble_evt,                      \
                         &_name)

//Defining 16-bit service and 128-bit base UUIDs
#define LFS_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID
#define BLE_LFS_SERVICE_UUID                0xFEED // Just a random, but recognizable value

//Defining 16-bit characteristic UUIDs
#define BLE_LFS_SND_MSG_CHARACTERISTC_UUID         	0x000C // Send BLE Message characteristic
#define BLE_LFS_RCV_MSG_CHARACTERISTC_UUID          0x000D // Receive BLE Message characteristic

#ifndef OPCODE_LENGTH
#define OPCODE_LENGTH        1
#endif

#ifndef HANDLE_LENGTH
#define HANDLE_LENGTH        2
#endif

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the BLE LC Forwarder Service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
	#define BLE_LFS_MAX_DATA_LEN 			(NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_LFS_MAX_DATA_LEN 			(BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

#define BLE_LFS_MAX_NOTIFY_CHAR_LEN       BLE_LFS_MAX_DATA_LEN /**< Maximum length of the Send BLE Message Characteristic (in bytes). */
#define BLE_LFS_MAX_WRITE_CHAR_LEN        BLE_LFS_MAX_DATA_LEN /**< Maximum length of the Receive BLE Message Characteristic (in bytes). */


/**@brief   BLE LC Forwarder Service event types. */
typedef enum
{
    BLE_LFS_EVT_RX_DATA,      /**< Data received. */
    BLE_LFS_EVT_TX_RDY,       /**< Service is ready to accept new data to be transmitted. */
    BLE_LFS_EVT_COMM_STARTED, /**< Notification has been enabled. */
    BLE_LFS_EVT_COMM_STOPPED, /**< Notification has been disabled. */
} ble_lfs_evt_type_t;
	
/* Forward declaration of the ble_lfs_t type. */
typedef struct ble_lfs_s ble_lfs_t;

/**@brief   BLE LC Forwarder Service @ref BLE_LFS_EVT_WRITE_DATA event data.
 *
 * @details This structure is passed to an event when @ref BLE_LFS_EVT_WRITE_DATA occurs.
 */
typedef struct
{
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} ble_lfs_evt_write_data_t;

/**@brief BLE LC Forwarder Service client context structure.
 *
 * @details This structure contains state context related to hosts.
 */
typedef struct
{
    bool is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the Send BLE Message characteristic.*/
} ble_lfs_client_context_t;

/**@brief   BLE LC Forwarder Service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct
{
    ble_lfs_evt_type_t         type;        /**< Event type. */
    ble_lfs_t                * p_lfs;       /**< A pointer to the instance. */
    uint16_t                   conn_handle; /**< Connection handle. */
    ble_lfs_client_context_t * p_link_ctx;  /**< A pointer to the link context. */
    union
    {
        ble_lfs_evt_write_data_t  write_data; /**< @ref BLE_LFS_EVT_WRITE_DATA event data. */
    } params;
} ble_lfs_evt_t;

/**@brief BLE LC Forwarder Service event handler type. */
typedef void (* ble_lfs_data_handler_t) (ble_lfs_evt_t * p_evt);

/**@brief   LC Forwarder Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * 			must fill this structure and pass it to the service using the @ref ble_lfs_init
 *          function.
 */
typedef struct
{
    ble_lfs_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_lfs_init_t;

/**@brief   BLE LC Forwarder Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_lfs_s
{
    uint8_t                         uuid_type;          	/**< UUID type for BLE LC Forwarder Service Base UUID. */
    uint16_t                        service_handle;     	/**< Handle of BLE LC Forwarder Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t       	fwd_snd_char_handles; 	/**< Handles related to the Send BLE Message characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        fwd_rcv_char_handles;   /**< Handles related to the Receive BLE Message characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t * const p_link_ctx_storage; 	/**< Pointer to link context storage with handles of all current connections and its context. */
    ble_lfs_data_handler_t          data_handler;       	/**< Event handler to be called for handling received data. */
};

ret_code_t ble_lfs_init(ble_lfs_t * p_lfs, ble_lfs_init_t const * p_lfs_init);
void ble_lfs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
ret_code_t ble_lfs_data_send(	ble_lfs_t * p_lfs,
							   uint8_t   * p_data,
							   uint16_t  * p_length,
							   uint16_t    conn_handle);

#endif  /* _ BLE_FORWARDER_SERVICE_H__ */
