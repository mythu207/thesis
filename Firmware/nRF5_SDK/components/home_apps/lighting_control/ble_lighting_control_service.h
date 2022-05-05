/*	Ho Chi Minh, 26/02/2019
 *	Writer: Do Ngoc Nhuan
 *	Lighting Control Service.
 */
#ifndef BLE_LIGHTING_CONTROL_SERVICE_H__
#define BLE_LIGHTING_CONTROL_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

/**@brief   Macro for defining a ble_lcs instance.
 *
 * @param     _name            Name of the instance.
 * @param[in] _lcs_max_clients Maximum number of lcs clients connected at a time.
 * @hideinitializer
 */
#define BLE_LCS_DEF(_name, _lcs_max_clients)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),  \
                             (_lcs_max_clients),                  \
                             sizeof(ble_lcs_client_context_t));   \
    static ble_lcs_t _name =                                      \
    {                                                             \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage) \
    };                                                            \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                           \
                         BLE_LCS_BLE_OBSERVER_PRIO,               \
                         ble_lcs_on_ble_evt,                      \
                         &_name)

//Defining 16-bit service and 128-bit base UUIDs
#define LCS_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID
#define BLE_LCS_SERVICE_UUID                0xF00D // Just a random, but recognizable value

//Defining 16-bit characteristic UUIDs
#define BLE_LCS_SEND_INFO_CHARACTERISTC_UUID         0x000A // Send Infomation Characteristic
#define BLE_LCS_RCV_CMD_CHARACTERISTC_UUID          0x000B // Receive Lighting Control (LC) Characteristic

#ifndef OPCODE_LENGTH
#define OPCODE_LENGTH        1
#endif

#ifndef HANDLE_LENGTH
#define HANDLE_LENGTH        2
#endif

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the BLE Lighting Control Service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_LCS_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_LCS_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

#define BLE_LCS_MAX_NOTIFY_CHAR_LEN       BLE_LCS_MAX_DATA_LEN /**< Maximum length of the Send Infomation Characteristic (in bytes). */
#define BLE_LCS_MAX_WRITE_CHAR_LEN        BLE_LCS_MAX_DATA_LEN /**< Maximum length of the Receive LC Command Characteristic (in bytes). */

/**@brief   Lighting Control Service event types. */
typedef enum
{
    BLE_LCS_EVT_RX_DATA,      /**< Data received. */
    BLE_LCS_EVT_TX_RDY,       /**< Service is ready to accept new data to be transmitted. */
    BLE_LCS_EVT_COMM_STARTED, /**< Notification has been enabled. */
    BLE_LCS_EVT_COMM_STOPPED, /**< Notification has been disabled. */
} ble_lcs_evt_type_t;
	
/* Forward declaration of the ble_lcs_t type. */
typedef struct ble_lcs_s ble_lcs_t;

/**@brief   Lighting Control Service @ref BLE_LCS_EVT_WRITE_DATA event data.
 *
 * @details This structure is passed to an event when @ref BLE_LCS_EVT_WRITE_DATA occurs.
 */
typedef struct
{
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} ble_lcs_evt_write_data_t;

/**@brief Lighting Control Service client context structure.
 *
 * @details This structure contains state context related to hosts.
 */
typedef struct
{
    bool is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the Send Infomation Characteristic.*/
} ble_lcs_client_context_t;

/**@brief   Lighting Control Service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct
{
    ble_lcs_evt_type_t         type;        /**< Event type. */
    ble_lcs_t                * p_lcs;       /**< A pointer to the instance. */
    uint16_t                   conn_handle; /**< Connection handle. */
    ble_lcs_client_context_t * p_link_ctx;  /**< A pointer to the link context. */
    union
    {
        ble_lcs_evt_write_data_t  write_data; /**< @ref BLE_LCS_EVT_WRITE_DATA event data. */
    } params;
} ble_lcs_evt_t;

/**@briefLighting Control Service event handler type. */
typedef void (* ble_lcs_data_handler_t) (ble_lcs_evt_t * p_evt);

/**@brief   Lighting Control Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * 			must fill this structure and pass it to the service using the @ref ble_lcs_init
 *          function.
 */
typedef struct
{
    ble_lcs_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_lcs_init_t;

/**@brief   Lighting Control Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_lcs_s
{
    uint8_t                         uuid_type;          	/**< UUID type for Lighting Control Service Base UUID. */
    uint16_t                        service_handle;     	/**< Handle of Lighting Control Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t       	send_info_char_handles; /**< Handles related to the Send Infomation characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        rcv_cmd_char_handles;   /**< Handles related to the Receive LC Command characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t * const p_link_ctx_storage; 	/**< Pointer to link context storage with handles of all current connections and its context. */
    ble_lcs_data_handler_t          data_handler;       	/**< Event handler to be called for handling received data. */
};

uint32_t ble_lcs_init(ble_lcs_t * p_lcs, ble_lcs_init_t const * p_lcs_init);
void ble_lcs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_lcs_data_send(ble_lcs_t * p_lcs,
                           uint8_t   * p_data,
                           uint16_t  * p_length,
                           uint16_t    conn_handle);

void ble_lcs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_lcs_init(ble_lcs_t * p_lcs, ble_lcs_init_t const * p_lcs_init);
uint32_t ble_lcs_data_send(ble_lcs_t * p_lcs,
                           uint8_t   * p_data,
                           uint16_t  * p_length,
                           uint16_t    conn_handle);

#endif  /* _ BLE_LIGHTING_CONTROL_SERVICE_H__ */
