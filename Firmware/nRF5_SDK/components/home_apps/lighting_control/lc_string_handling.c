/**	Do Ngoc Nhuan, 24/02/2019
 *	Module for handling string data receive from user and preparing string data to send to user.
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(LC_STRING_HANDLING)
#include "lc_string_handling.h"

#define NRF_LOG_MODULE_NAME lc_string
#if LC_STRING_HANDLING_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       LC_STRING_HANDLING_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  LC_STRING_HANDLING_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR LC_STRING_HANDLING_CONFIG_DEBUG_COLOR
#else // LC_STRING_HANDLING_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // LC_STRING_HANDLING_CONFIG_LOG_ENABLED
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();


static const char *code_arr[LENGTH_CODE_ARRAY] = {	"", /*0*/
													"", /*1*/
													"get", /*2*/
													"set", /*3*/
													"mode", /*4*/
													"state", /*5*/
													"on", /*6*/
													"off", /*7*/
													"auto", /*8*/
													"manual", /*9*/
																};
static lc_string_handling_callback_t registered_callback[MAXIMUM_NUMBER_OF_CALLBACK];
static uint8_t m_string[LC_MAX_STRING_LENGTH];
static int8_t cmd_arr[MAX_NUM_OF_WORD_IN_CMD];
static int32_t cmd_val_arr[MAX_NUM_OF_WORD_IN_CMD];

static bool lc_string_find_word(const char * p_word, int8_t * p_result)
{
	int8_t i;
	
	for (i = 2; i < LENGTH_CODE_ARRAY; i++)	{
		if (strcmp(p_word, code_arr[i])==0)	{
			*p_result = i;
			return true;
		}
	}
	return false;
}

static bool lc_string_convert_to_uint(const char * p_num, int8_t * p_char_result, int32_t * p_num_result)
{
	uint32_t result = 0;
	uint8_t i;
	
	for (i=0; i<strlen(p_num); i++)	{
		if ((*(p_num+i)>='0')&&(*(p_num+i)<='9'))	{
			result *= 10;
			result += (*(p_num+i)-0x30);
		} else {
			return false;
		}
	}
	*p_num_result = result;
	*p_char_result = 1;
	return true;
}

static bool lc_encrypt_command( const uint8_t * p_command,
								uint8_t cmdStrLen, 
								int8_t * encrypted_char_cmd,
								int32_t * encrypted_num_cmd)
{
	char * array_command;
	char * saveptr;
	char * token;
	int8_t k = 0;
	
	array_command = (char *) m_string;
	//initialized array_command for this process.
	memset(array_command, 0, LC_MAX_STRING_LENGTH*sizeof(char));
	//copy received payload to another array before handling. 
	strncpy(array_command, (char *)p_command, cmdStrLen);

	if (cmdStrLen != 0)	{
		do	{
			do	{
				token = strtok_r(array_command, "_", &saveptr);
				if ((*token>='a')&&(*token<='z'))	{
					
					if (!lc_string_find_word(token, (encrypted_char_cmd+k))) break;
					
				} else if ((*token>='0')&&(*token<='9'))	{
					
					if (!lc_string_convert_to_uint(token, (encrypted_char_cmd+k), (encrypted_num_cmd+k))) break;
				
				} else if (*token=='-')	{
					
					if (!lc_string_convert_to_uint(++token, (encrypted_char_cmd+k), (encrypted_num_cmd+k))) break;
					
					*(encrypted_num_cmd+k) = -(*(encrypted_num_cmd+k));
				}

				k++;
				array_command = saveptr;

			} while (strlen(array_command)!=0);
			return true;
		} while (false);
	}

	NRF_LOG_ERROR("Can't recognize received command!");
	return false;
}

void convert_int_to_string(uint32_t num, uint8_t * str)	
{
	uint8_t c, lenStr = 0;
	
	do	{
		c = (num%10)+48;
		num /= 10;
		*(str+lenStr) = c;
		lenStr++;
	}while (num != 0);
	
	*(str+lenStr) = NULL; //NULL: end of string
	
	for (uint8_t i=0;i<lenStr/2;i++)	{
		c = *(str+i);
		*(str+i) = *(str+(lenStr-1)-i);
		*(str+(lenStr-1)-i) = c;
	}
}

void make_msg_lc_light_info(const lighting_control_info_t * p_lighting_control_info, uint8_t * str_msg)
{
	convert_int_to_string(p_lighting_control_info->lux, str_msg);
	
	if (p_lighting_control_info->state == LIGHT_ON)	{
		strcat((char *)str_msg, "_on");
	} else {
		strcat((char *)str_msg, "_off");
	}
}

static void calling_to_string_hangling_calbacks(uint8_t * p_msg, uint8_t length)
{
	for (uint8_t i=0; i<MAXIMUM_NUMBER_OF_CALLBACK; i++)	{
		if (registered_callback[i]!=NULL)	{
			registered_callback[i](p_msg, length);
		} else {
			break;
		}
	}
}

static void make_msg_lc_mode(void)
{
	uint8_t * msg_to_send = m_string;
	
	memset(msg_to_send, 0, LC_MAX_STRING_LENGTH*sizeof(msg_to_send));
	
	if (lc_auto_mode_is_on())	{
		strcat((char *)msg_to_send, "auto");
	} else {
		strcat((char *)msg_to_send, "manual");
	}
	
	calling_to_string_hangling_calbacks(msg_to_send, strlen((char *)msg_to_send));
}

static void make_msg_lc_auto_mode_cfg(void)
{
	uint8_t * msg_to_send = m_string;
	const auto_mode_config_t * auto_mode_config;
	char * p_string	= (char *)nrf_malloc(10*sizeof(char));
		
	auto_mode_config = lc_auto_mode_get_cfg();
	
	memset(msg_to_send, 0, LC_MAX_STRING_LENGTH*sizeof(msg_to_send));
	if (auto_mode_config->set_as_default)	{
		strcat((char *)msg_to_send, "default");
	} else	{//message format: <interval>_<threshold>
		memset(p_string, 0, 10*sizeof(char));
		convert_int_to_string(auto_mode_config->interval, (uint8_t *)p_string);
		strcat((char *)msg_to_send, p_string);
		
		strcat((char *)msg_to_send, "_");
		
		memset(p_string, 0, 10*sizeof(char));
		convert_int_to_string(auto_mode_config->threshold, (uint8_t *)p_string);
		strcat((char *)msg_to_send, p_string);
	}
	
	nrf_free(p_string);
	
	calling_to_string_hangling_calbacks(msg_to_send, strlen((char *)msg_to_send));
}

static void getting_lighting_control(int8_t * encrypted_char_cmd)
{
	switch (*encrypted_char_cmd)	{
		
		case LC_NOTHING: /*get*/
			lc_start_get_info();
			break;
		
		case LC_MODE_WORD: /*get_mode*/
			make_msg_lc_mode();
			break;
		
		case LC_AUTO_WORD: /*get_auto*/
			make_msg_lc_auto_mode_cfg();
			break;
		
		default:
			break;
	}
}
static void setting_lighting_control(int8_t * encrypted_char_cmd, int32_t * encrypted_num_cmd)
{
	switch (*encrypted_char_cmd)	{
		case LC_MODE_WORD: /*set_mode...*/
			if (*(encrypted_char_cmd+1) == LC_AUTO_WORD)/*set_mode_auto*/	{
				
				auto_mode_config_t auto_mode_config;
				memset(&auto_mode_config, 0, sizeof(auto_mode_config));
				
				if ((*(encrypted_char_cmd+2) == LC_THIS_IS_NUMBER)	
					&&(*(encrypted_char_cmd+3) == LC_THIS_IS_NUMBER))	{
						
					auto_mode_config.interval = *(encrypted_num_cmd+2);
					auto_mode_config.threshold = *(encrypted_num_cmd+3);
				
				} else {
					auto_mode_config.set_as_default = true;
				}
				
				if (!lc_auto_mode_is_on()) {
					lc_set_off_auto_mode();
				}
				
				lc_auto_mode_set_cfg(&auto_mode_config);
				lc_set_on_auto_mode();
				
				if (auto_mode_config.set_as_default)	{
					NRF_LOG_INFO("Lighting is in default automatic mode.");
				} else	{
					NRF_LOG_INFO("Lighting is in automatic mode with %d (ms) interval and %d (lux) threshold.",
									auto_mode_config.interval, auto_mode_config.threshold);
				}
				
			} else if (*(encrypted_char_cmd+1) == LC_MANUAL_WORD)/*set_mode_manual*/	{
				
				if (lc_auto_mode_is_on()) {
					lc_set_off_auto_mode();
				}
				NRF_LOG_INFO("Lighting is in manual mode.");
				
			} else {
				NRF_LOG_ERROR("Setting command is out of type.");
			}
			break;
		case LC_STATE_WORD: /*set_state...*/
			if (!lc_auto_mode_is_on())	{
				if (*(encrypted_char_cmd+1) == LC_ON_WORD)/*set_state_on*/	{
					lc_light_on();
				} else if (*(encrypted_char_cmd+1) == LC_OFF_WORD)/*set_state_off*/	{
					lc_light_off();
				} else	{
					NRF_LOG_ERROR("Setting command is out of type.");
				}
			}
			break;
		default:
			break;
	}
}

void recognize_lc_control_command(const uint8_t * p_cmd, uint8_t cmd_length)
{
	int8_t	* encrypted_char_cmd = cmd_arr;
	int32_t	* encrypted_num_cmd  = cmd_val_arr;
	
	memset(encrypted_char_cmd, 0, MAX_NUM_OF_WORD_IN_CMD*sizeof(encrypted_char_cmd));
	memset(encrypted_num_cmd, 0, MAX_NUM_OF_WORD_IN_CMD*sizeof(encrypted_num_cmd));
	
	lc_encrypt_command(p_cmd, cmd_length, encrypted_char_cmd, encrypted_num_cmd);
	
//	uint32_t i=0;
//	for (i = 0; i<6; i++)	{
//		NRF_LOG_INFO("encrypted_char_cmd[%d] = %d", i, encrypted_num_cmd[i]);
//	}
//	NRF_LOG_FLUSH();
	switch (encrypted_char_cmd[0])	{
		case LC_GET_WORD: /*get...*/
			getting_lighting_control(&encrypted_char_cmd[1]);
			break;
		case LC_SET_WORD: /*set...*/
			setting_lighting_control(&encrypted_char_cmd[1], &encrypted_num_cmd[1]);
			break;
		default:
			break;
	}
}

ret_code_t lc_string_handling_callback_register(lc_string_handling_callback_t callback)
{
	for (uint8_t i=0; i<MAXIMUM_NUMBER_OF_CALLBACK; i++)	{
		if (registered_callback[i]==NULL)	{
			registered_callback[i] = callback;
			return NRF_SUCCESS;
		}
	}
	return NRF_ERROR_INTERNAL;
}

void lc_string_handling_init(void)
{
	ret_code_t err_code = nrf_mem_init();
	APP_ERROR_CHECK(err_code);
}
#endif // NRF_MODULE_ENABLED(LC_STRING_HANDLING)
