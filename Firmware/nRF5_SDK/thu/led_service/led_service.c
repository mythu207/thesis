#include "sdk_common.h"
#include "ble_Srv_common.h"
#include "led_service.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"



/* This code belongs in ble_cus.c*/

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{ 
    
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_CONNECTED;

    p_cus->evt_handler(p_cus, &evt);
  
   
    
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    NRF_LOG_INFO("THU: RUN THIS?");
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Check if the handle passed with the event matches the Custom Value Characteristic handle.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        NRF_LOG_INFO("THU: VALUE %X", *(p_evt_write->data));
        nrf_gpio_pin_toggle(LED_2); 
    }
    // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    if ((p_evt_write->handle == p_cus->custom_value_handles.cccd_handle)
        && (p_evt_write->len == 2)
       )
    {
        // CCCD written, call application event handler
        if (p_cus->evt_handler != NULL)
        {
            ble_cus_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_CUS_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_cus->evt_handler(p_cus, &evt);
        }
    }
}
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    /* This code belongs in custom_value_char_add() in ble_cus.c*/

    memset(&char_md, 0, sizeof(char_md));

    //READ, WRITE, NO Notify
    char_md.char_props.read   = 1; 
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;

    /* This code belongs in custom_value_char_add() in ble_cus.c*/

    memset(&attr_md, 0, sizeof(attr_md));
    
    
    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    /* This code belongs in custom_value_char_add() in ble_cus.c*/

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;
    

    /* This code belongs in custom_value_char_add() in ble_cus.c*/

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);

    /* This code belongs in custom_value_char_add() in ble_cus.c*/

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                                   &attr_char_value,
                                                   &p_cus->custom_value_handles);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        return NRF_SUCCESS;

}

void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_cus, p_ble_evt);
            break;
        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init){
    
    //Checking to make sure non of input pointers are NULL
    if (p_cus == NULL || p_cus_init == NULL){
        return NRF_ERROR_NULL;
    }

    //If they are non-NULL -> create two variables: err_code and ble_uuid
    uint32_t    err_code;
    ble_uuid_t  ble_uuid;

    //Initialize service structure --> set it to invalid and only become valid when in a connection
    p_cus->evt_handler      = p_cus_init->evt_handler;
    p_cus->conn_handle      = BLE_CONN_HANDLE_INVALID;


    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    //Add the custom service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid,  &p_cus->service_handle);
    VERIFY_SUCCESS(err_code);

    err_code = custom_value_char_add(p_cus, p_cus_init);
    VERIFY_SUCCESS(err_code);
    
    return err_code;
}

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value){
    if (p_cus == NULL)
    {
        return NRF_ERROR_NULL;
    }
    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = &custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                        p_cus->custom_value_handles.value_handle,
                                        &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}






