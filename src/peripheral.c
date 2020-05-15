/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        peripheral.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2020-04-17
 * Description:      外设管理
 * Others:           需要永久存储的状态,放到system_state模块
 * Function List:    
    1. 创建peripheral模块
    2. 销毁peripheral模块
    3. 定时处理入口
    
 * History: 
    1. Date:         2020-04-17
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#include <gm_stdlib.h>
#include <gm_system.h>
#include "peripheral.h"
#include "uart.h"
#include "config_service.h"
#include "utility.h"
#include "log_service.h"
#include "applied_math.h"
#include "protocol.h"

//包头	命令    ID号	校验	  包尾
//4字节	2字节	4字节	2字节	2字节


//HBTQ    D0    1111  0003       ABAD 
//包头	  命令	ID号	  报警标志位	数据	校验	        包尾
//4字节	2字节	4字节	      2字节	    4字节	2字节	2字节


//HBTQ    D01    000    0          0              ABAA    63
//包头      命令     ID号    报警标志位 空重载标志位              数据     校验     包尾
//4字节     3字节    3字节    1字节         1字节           4字节    2字节    2字节


#define YW201_HAED_LEN 4
#define YW201_CMD_LEN 2
#define YW201_ID_LEN 4
#define YW201_ALARM_LEN 2
#define YW201_DATA_LEN 4
#define YW201_CHECKSUM_LEN 2
#define YW201_TAIL_LEN 2
#define YW201_HEAD "HBTQ"
#define YW201_DATA_CMD "D0"
#define YW201_ID "0000"
#define YW201_TAIL "\r\n"
#define YW201_DATA_MAX 0xFFF
#define OIL_KEY "oil"
#define OIL_PERCENT_NAME "oil_percent"

#define ZZH201_HAED_LEN 4
#define ZZH201_CMD_LEN 3
#define ZZH201_ID_LEN 3
#define ZZH201_ALARM_LEN 1
#define ZZH201_Empty_HEAVY_FLAG_LEN 1
#define ZZH201_DATA_LEN 4
#define ZZH201_CHECKSUM_LEN 2
#define ZZH201_TAIL_LEN 2
#define ZZH201_HEAD "HBTQ"
#define ZZH201_DATA_CMD "D01"
#define ZZH201_ID "000"
#define ZZH201_TAIL "\r\n"
#define ZZH201_DATA_MAX 0xFFF
#define WEIGHT_KEY "weight"
#define WEIGHT_PERCENT_NAME "weight_percent"


//CR6061油杆协议定义

//发送
#define CR6061_REQUEST_HEAD_LEN 2
#define CR6061_CMD_LEN 2
#define CR6061_REQUEST_CHECKSUM_LEN 2
#define CR6061_REQUEST_HEAD "$!"
#define CR6061_CMD "RY"


//接受
#define CR6061_RECV_HEAD_LEN 1
#define CR6061_FLAG_LEN 3
#define CR6061_RETURN_VALUE_LEN 6
#define CR6061_RECV_CHECKSUM_LEN 2
#define CR6061_RECV_HEAD "*"
#define CR6061_FLAG "CFV"
#define CR6061_RETURN_VALUE "00FA32"
#define CR6061_DATA_MAX 0xFFFF


//共有
#define CR6061_ID_LEN 2
#define CR6061_TAIL_LEN 2
#define CR6061_ID "01"
#define CR6061_TAIL "\r\n"



typedef struct
{
	GM_PERIPHERAL_TYPE peripheral_type;
}PeripheralPara;

static PeripheralPara s_peripheral_para;

GM_ERRCODE peripheral_create(void)
{
	s_peripheral_para.peripheral_type = PERIPHERAL_TYPE_NONE;
	return GM_SUCCESS;
}
 
GM_ERRCODE peripheral_destroy(void)
{
	return GM_SUCCESS;
}

static void yw201_request_data(void)
{
	char check_sum_str[YW201_CHECKSUM_LEN + 1] = {0};
	char request_oil_cmd[YW201_HAED_LEN + YW201_CMD_LEN + YW201_ID_LEN + YW201_CHECKSUM_LEN + YW201_TAIL_LEN + 1] = {0};
	GM_strcat(request_oil_cmd,YW201_HEAD);
	GM_strcat(request_oil_cmd,YW201_DATA_CMD);
	GM_strcat(request_oil_cmd,YW201_ID);
	GM_snprintf(check_sum_str,sizeof(check_sum_str),"%02X",applied_math_8bit_checksum((U8*)request_oil_cmd, GM_strlen(request_oil_cmd)));
	GM_strcat(request_oil_cmd,check_sum_str);
	GM_strcat(request_oil_cmd,YW201_TAIL);
	
	uart_write(GM_UART_DEBUG,(U8*)request_oil_cmd,GM_strlen((char *)request_oil_cmd));
}
static void cr6061_request_data(void)
{
	char check_sum_str[CR6061_REQUEST_CHECKSUM_LEN + 1] = {0};
	char request_oil_cmd[CR6061_REQUEST_HEAD_LEN + CR6061_CMD_LEN + CR6061_ID_LEN + CR6061_REQUEST_CHECKSUM_LEN + CR6061_TAIL_LEN + 1] = {0};
	GM_strcat(request_oil_cmd,CR6061_REQUEST_HEAD);
	GM_strcat(request_oil_cmd,CR6061_CMD);
	GM_strcat(request_oil_cmd,CR6061_ID);
	GM_snprintf(check_sum_str, sizeof(check_sum_str), "%02X" ,applied_math_8bit_checksum((U8*)request_oil_cmd, GM_strlen(request_oil_cmd)));
	GM_strcat(request_oil_cmd, check_sum_str);
	GM_strcat(request_oil_cmd,CR6061_TAIL);
	
	uart_write(GM_UART_DEBUG,(U8*)request_oil_cmd,GM_strlen((char *)request_oil_cmd));
}

static void zzh201_request_data(void)
{
	char check_sum_str[ZZH201_CHECKSUM_LEN + 1] = {0};
	char request_weight_cmd[ZZH201_HAED_LEN + ZZH201_CMD_LEN + ZZH201_ID_LEN + ZZH201_CHECKSUM_LEN + ZZH201_TAIL_LEN + 1] = {0};
	GM_strcat(request_weight_cmd,ZZH201_HEAD);
	GM_strcat(request_weight_cmd,ZZH201_DATA_CMD);
	GM_strcat(request_weight_cmd, ZZH201_ID);
	GM_snprintf(check_sum_str,sizeof(check_sum_str),"%02X",applied_math_8bit_checksum((U8*)request_weight_cmd, GM_strlen(request_weight_cmd)));
	GM_strcat(request_weight_cmd,check_sum_str);
	GM_strcat(request_weight_cmd,ZZH201_TAIL);
	uart_write(GM_UART_DEBUG,(U8*)request_weight_cmd,GM_strlen((char *)request_weight_cmd));	
} 

static void yw201_recv_data(char* p_raw_data,U16 data_len)
{
	char* p_head = p_raw_data;
	char* p_cmd = p_head + YW201_HAED_LEN;
	char* p_id = p_cmd + YW201_CMD_LEN;
	char* p_alarm = p_id + YW201_ID_LEN;
	char* p_data = p_alarm + YW201_ALARM_LEN;	

	if (GM_strstr(p_raw_data, YW201_HEAD) && GM_strstr(p_cmd, YW201_DATA_CMD) )
	{
		char upload_data_str[100] = {0};
		U16 oil_data = 0;
		JsonObject* p_root  = json_create();
		JsonObject* p_oil_data = json_add_object(p_root,OIL_KEY);

		p_data[YW201_DATA_LEN] = '\0';

		oil_data = util_hexstrtoul(p_data);
		GM_snprintf(upload_data_str, sizeof(upload_data_str), "%.1f", oil_data*100.0/YW201_DATA_MAX);
		json_add_string(p_oil_data, OIL_PERCENT_NAME, upload_data_str);
		
		json_print_to_buffer(p_root, upload_data_str, 100 - 1);
		
		protocol_send_transprent_msg(get_socket_by_accessid(SOCKET_INDEX_MAIN), upload_data_str);
		//log_service_upload(INFO, p_root);
		json_destroy(p_root);
	}


}


static void cr6061_recv_data(char* p_raw_data,U16 data_len)
{
	char* p_head = p_raw_data;
	char* p_flag = p_head + CR6061_RECV_HEAD_LEN;
	char* p_id  = p_flag + CR6061_FLAG_LEN;
	char* p_recv_return_value = p_id + CR6061_ID_LEN;

	if(GM_strstr(p_raw_data, CR6061_RECV_HEAD) && GM_strstr(p_flag,CR6061_FLAG))
	{
		char upload_data_str[100] = {0};
		U16 oil_data = 0;
		JsonObject* p_root = json_create();
		JsonObject* p_oil_data = json_add_object(p_root,OIL_KEY);

		p_recv_return_value[CR6061_RETURN_VALUE_LEN + 1] = '\0';

		oil_data = util_hexstrtoul(p_recv_return_value);
		GM_snprintf(upload_data_str, sizeof(upload_data_str), "%.1f", oil_data*100.0/CR6061_DATA_MAX);
		json_add_string(p_oil_data, OIL_PERCENT_NAME, upload_data_str);
		
		json_print_to_buffer(p_root, upload_data_str, 100 - 1);
		
		protocol_send_transprent_msg(get_socket_by_accessid(SOCKET_INDEX_MAIN), upload_data_str);
		//log_service_upload(INFO, p_root);
		json_destroy(p_root);
	}
}

static void zzh201_recv_data(char* p_raw_data,U16 data_len)
{
		
	char* p_head = p_raw_data;
	char* p_cmd = p_head + ZZH201_HAED_LEN;
	char* p_id = p_cmd + ZZH201_CMD_LEN;
	char* p_alarm = p_id + ZZH201_ID_LEN;
	char* p_empty_heavy_flag = p_alarm + ZZH201_ALARM_LEN;
	char* p_data = p_empty_heavy_flag + ZZH201_Empty_HEAVY_FLAG_LEN;
	
	if (GM_strstr(p_raw_data, ZZH201_HEAD) && GM_strstr(p_cmd, ZZH201_DATA_CMD) )
	{
		char upload_data_str[100] = {0};
		U16 weight_data = 0;
		JsonObject* p_root  = json_create();
		JsonObject* p_weight_data = json_add_object(p_root,WEIGHT_KEY);

		p_data[ZZH201_DATA_LEN] = '\0';

		weight_data = util_hexstrtoul(p_data);
		GM_snprintf(upload_data_str, sizeof(upload_data_str), "%.1f", weight_data*100.0/ZZH201_DATA_MAX);
		json_add_string(p_weight_data, WEIGHT_PERCENT_NAME, upload_data_str);
		
		json_print_to_buffer(p_root, upload_data_str, 100 - 1);

		//谷米透传协议发送到平台
		protocol_send_transprent_msg(get_socket_by_accessid(SOCKET_INDEX_MAIN), upload_data_str);
		json_destroy(p_root);
	}

}

GM_ERRCODE peripheral_timer_proc(void)
{	
	U8 peripheral_type = 0;
	config_service_get(CFG_PERIPHERAL_TYPE, TYPE_BYTE, &peripheral_type, sizeof(peripheral_type));
	s_peripheral_para.peripheral_type = (GM_PERIPHERAL_TYPE)peripheral_type;

	if(PERIPHERAL_TYPE_NONE == s_peripheral_para.peripheral_type)
	{
		if(BAUD_RATE_HIGH != uart_get_baud(GM_UART_DEBUG))
		{
			LOG(INFO,"reopen debug port with %d",BAUD_RATE_HIGH);
			uart_close_port(GM_UART_DEBUG);
			GM_SysMsdelay(10);
			uart_open_port(GM_UART_DEBUG, BAUD_RATE_HIGH, 0);
		}
		log_service_enable_print(TRUE);
		return GM_SUCCESS;
	}

	log_service_enable_print(FALSE);

	if(BAUD_RATE_LOW != uart_get_baud(GM_UART_DEBUG))
	{
		LOG(INFO,"reopen debug port with %d",BAUD_RATE_HIGH);
		uart_close_port(GM_UART_DEBUG);
		GM_SysMsdelay(10);
		uart_open_port(GM_UART_DEBUG, BAUD_RATE_LOW, 0);
	}

	if(0 != (util_clock() % 10))
	{
		return GM_SUCCESS;
	}

	switch (s_peripheral_para.peripheral_type)
	{
		case PERIPHERAL_TYPE_YW201:
		{
			yw201_request_data();
			break;
		}
		case PERIPHERAL_TYPE_ZZH201:
		{
			zzh201_request_data();
			break;
		}
		case PERIPHERAL_TYPE_CR6061:
		{
			cr6061_request_data();
			break;
		}
		default:
		{
			break;
		}
	}
	
	return GM_SUCCESS;
}

GM_ERRCODE peripheral_uart_on_receive(char* p_data,U16 data_len)
{
	switch (s_peripheral_para.peripheral_type)
	{
		case PERIPHERAL_TYPE_YW201:
		{
			yw201_recv_data(p_data,data_len);
			break;
		}
		case PERIPHERAL_TYPE_ZZH201:
		{
			zzh201_recv_data(p_data,data_len);
			break;
		}
		case PERIPHERAL_TYPE_CR6061:
		{
			cr6061_recv_data(p_data,data_len);
			break;
		}
		default:
		{
			break;
		}
	}
	return GM_SUCCESS;
}


