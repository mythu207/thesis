#ifndef FWD_SERVICE_H__
#define FWD_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_UUID_FWD_BASE              {{0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E}} /**< Used vendor specific UUID. */
#define BLE_UUID_FWD_SERVICE           0x0001 // Just a random, but recognizable value

// ALREADY_DONE_FOR_YOU: Defining 16-bit characteristic UUID
#define BLE_UUID_FWD_TX_CHAR          0x0004 // Just a random, but recognizable value
#define BLE_UUID_FWD_RX_CHAR          0x0002





#define BLE_FWD_DEF(_name, _fwd_max_clients)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),  \
                             (_fwd_max_clients),                  \
                             sizeof(ble_fwd_client_context_t));   \
    static ble_fwd_t _name =                                      \
    {                                                             \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage) \
    };                                                            \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                           \
                         BLE_FWD_BLE_OBSERVER_PRIO,               \
                         ble_fwd_on_ble_evt,                      \
                         &_name)

#define OPCODE_LENGTH        1
#define HANDLE_LENGTH        2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_FWD_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_FWD_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

typedef enum
{
    BLE_FWD_EVT_RX_DATA,      /**< Data received. */
    BLE_FWD_EVT_TX_RDY,       /**< Service is ready to accept new data to be transmitted. */
    BLE_FWD_EVT_COMM_STARTED, /**< Notification has been enabled. */
    BLE_FWD_EVT_COMM_STOPPED, /**< Notification has been disabled. */
} ble_fwd_evt_type_t;


typedef struct  ble_fwd_s    ble_fwd_t;


typedef struct
{
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} ble_fwd_evt_rx_data_t;

typedef struct
{
    bool is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} ble_fwd_client_context_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_fwd_evt_type_t          type;                                  /**< Type of event. */
    ble_fwd_t                 * p_fwd;
    uint16_t                    conn_handle;
    ble_fwd_client_context_t  * p_link_ctx;
    union
    {
        ble_fwd_evt_rx_data_t rx_data; /**< @ref BLE_LFS_EVT_WRITE_DATA event data. */
    } params;
} ble_fwd_evt_t;

typedef void (*ble_fwd_data_handler_t) (ble_fwd_evt_t * p_evt);


typedef struct
{
    ble_fwd_data_handler_t         data_handler;
   
} ble_fwd_init_t;

struct ble_fwd_s
{
    uint8_t                         uuid_type;          /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                        service_handle;     /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        tx_handles;         /**< Handles related to the TX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        rx_handles;         /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t * const p_link_ctx_storage; /**< Pointer to link context storage with handles of all current connections and its context. */
    ble_fwd_data_handler_t          data_handler;       /**< Event handler to be called for handling received data. */

};



/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_our_service       Our Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_fwd_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */
uint32_t ble_fwd_init(ble_fwd_t * p_fwd, ble_fwd_init_t const * p_fwd_init);

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever our timer_timeout_handler triggers
 *
 * @param[in]   p_our_service                     Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
ret_code_t ble_fwd_data_send(ble_fwd_t * p_fwd,
                             uint8_t   * p_data,
                             uint16_t  * p_length,
                             uint16_t    conn_handle);
#endif  /* _ OUR_SERVICE_H__ */