/**	Do Ngoc Nhuan, 24/02/2019
 *	Module for handling string data receive from user and preparing string data to send to user.
 */
#ifndef LC_STRING_HANDLING_H
#define LC_STRING_HANDLING_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "mem_manager.h"
#include "lighting_control.h"

#define MAXIMUM_NUMBER_OF_CALLBACK			3
#define LC_MAX_STRING_LENGTH				36  //length of "set_mode_auto_<uint32_t_max>_<uint32_t_max><NULL>"
#define MAX_NUM_OF_WORD_IN_CMD				10
#define LENGTH_CODE_ARRAY					10
#define LC_THIS_IS_NUMBER					1
#define LC_GET_WORD							2
#define LC_SET_WORD							3
#define LC_MODE_WORD						4
#define LC_STATE_WORD						5
#define LC_ON_WORD							6
#define LC_OFF_WORD							7
#define LC_AUTO_WORD						8
#define LC_MANUAL_WORD						9
#define LC_NOTHING							0

typedef void (* lc_string_handling_callback_t)(uint8_t * p_msg_to_send, uint8_t length);

void convert_int_to_string(uint32_t num, uint8_t * str);

/**@brief Function for making ASCII message that will be used to publish on topic 2 (information topic).
 *
 * @param[in]    p_lighting_control_info   		Pointer to infomation of lighting device.
 * @param[in]    str_msg   			 			Pointer to memory area that store result message.
 */
void make_msg_lc_light_info(const lighting_control_info_t * p_lighting_control_info, uint8_t * str_msg);
void recognize_lc_control_command(const uint8_t * p_cmd, uint8_t cmd_length);
ret_code_t lc_string_handling_callback_register(lc_string_handling_callback_t callback);
void lc_string_handling_init(void);
#endif // LC_STRING_HANDLING_H
