/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        command.h
 * Author:           李耀轩       
 * Version:          1.0
 * Date:             2019-12-10
 * Description:      
 * Others:      
 * Function List:    
    1. 创建at_command模块
    2. 销毁at_command模块
    3. at_command模块定时处理入口
 * History: 
    1. Date:         2019-12-10
       Author:       李耀轩
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include <stdarg.h>
#include <string.h>
#include "at_command.h"
#include "uart.h"
#include "gm_memory.h"
#include "log_service.h"
#include "gm_system.h"
#include "gm_stdlib.h"
#include "utility.h"
#include "gm_timer.h"
#include "system_state.h"
#include "gm_network.h"
#include "gm_gprs.h"
#include "gm_sms.h"
#include "hard_ware.h"
#include "gprs.h"
#include "gps_service.h"

#define AT_CATCH_MAX_NUM 100
#define AT_SEND_MAX_CNT 5
#define RECV_MAX_AT_LEN 2048

#define MAX_TYPE_LEN 20

typedef struct
{
	char cmd[MAX_TYPE_LEN];
	u16 index;
}AtCommandRspStruct;


AtCommandRspStruct s_at_list[AT_CMD_MAX] = 
{
	{"",AT_NONE},//AT
	{"RDY",AT_RDY},
	{"AT",AT_AT},//AT
	{"ATE1",AT_ATE},
	{"AT+CPIN",AT_CPIN},//AT+CPIN?
	{"AT+CSQ",AT_CSQ},//AT+CSQ
	{"AT+CREG",AT_CREG},
	{"AT+CGREG",AT_CGREG},
	{"AT+GMR",AT_GMR},
	{"AT+CIMI",AT_CIMI},
	{"AT+QCCID",AT_QCCID},
	{"AT+QENG",AT_QENG},
	{"AT+COPS",AT_COPS},
	{"ATA",AT_ATA},
	{"ATD",AT_ATD},
	{"ATH",AT_ATH},
	{"AT+QICSGP",AT_QICSGP},
	{"AT+QIACT",AT_QIACT},
	{"AT+QIDEACT",AT_QIDEACT},
	{"AT+QIOPEN",AT_QIOPEN},
	{"AT+QICLOSE",AT_QICLOSE},
	{"AT+QISTATE",AT_QISTATE},
	{"AT+QISEND",AT_QISEND},
	{"AT+QISENDEX",AT_QISENDEX},
	{"AT+QIRD",AT_QIRD},
	{"AT+QIDNSGIP",AT_QIDNSGIP},
	{"+QIURC",AT_QIURC},
	{"+QIOPEN",AT_QIOPENURC},
	{"AT+QCFG",AT_QCFG},
	{"+CMTI",AT_SMSURC},
	{"AT+CMGR",AT_CMGR},
	{"AT+CMGD",AT_CMGD},
	{"AT+CMGF",AT_CMGF},
	{"AT+CMGS",AT_CMGS},
	{"AT+QURCCFG",AT_QURCCFG},
	{"AT+USER",AT_USER},
	{"ATS0",AT_AUTO_ANSWER},
};




typedef struct
{
	char *data;
	u16 data_len;
	char *data2;
	u16 data2_len;
	SocketIndexEnum access_id;
	AtCommandListEnum index;
	AtCommandTypeEnum type;
	u32 delay_time;
	bool need_reboot;
}AtCommandPara;


typedef struct
{
	bool sim_valid;
	PsFuncPtr sim_call_back;
	u8 csq_value;
	bool creg;
	bool cgreg;
	char verno[50];
	PsFuncPtr imsi_call_back;
	PsFuncPtr iccid_call_back;
	LbsCallBackFunc lbs_call_back;
	PsFuncPtr apn_call_back;
	PsFuncPtr qiact_call_back;
	PsFuncPtr local_ip_call_back;
	PsFuncPtr host_ip_call_back;
	PsFuncPtr sock_notify_call_back;
	SocketRecvCallBack sock_recv_call_back;
	PsFuncPtr new_sms_call_back;
	PsFuncPtr send_sms_call_back;
}AtCommandResult;


typedef struct
{
	bool inited;
	bool is_test_at;
	u8 status;
	bool wait_ask;
	AtCommandPara *command_que;
	u32 read_idx;
	u32 write_idx;
	u32 clock_ms;
	u32 clock_sec;
	u8 fail_count;
	AtCommandResult result;
	char *recv_buff;
	u16 recv_len;
	FifoType *rcv_fifo;
	AtCommandThird third_cmd;
}Type_AT;

Type_AT s_at_command;


typedef enum
{
	RSP_OK,
	RSP_SEND_OK,
	RSP_CME_ERROR,
	RSP_ERROR,
	RSP_RING,
	RSP_CONNECT,
	RSP_NOCARRIER,
	RSP_BUSY,
	RSP_NOANSWER,
	RSP_SOCKETSEND,
	RSP_URC,
	RSP_QIOPEN,
	RSP_QIRD,
	RSP_NEWSMS,
	RSP_QIND,
	RSP_RDY,

	RSP_MAX
}AtCommandRspEnum;

typedef struct 
{
	char rsp_string[MAX_TYPE_LEN];
	u8 index;
}AtCommandResultStruct;

AtCommandResultStruct s_at_rsp[RSP_MAX] = 
{
	{"\r\nSEND OK",RSP_SEND_OK},
	{"\r\nOK",RSP_OK},
	{"+CME ERROR",RSP_CME_ERROR},
	{"ERROR",RSP_ERROR},
	{"RING",RSP_RING},
	{"CONNECT",RSP_CONNECT},
	{"NO CARRIER",RSP_NOCARRIER},
	{"BUSY",RSP_BUSY},
	{"NO ANSWER",RSP_NOANSWER},
	{">",RSP_SOCKETSEND},
	{"+QIURC",RSP_URC},
	{"+QIOPEN",RSP_QIOPEN},
	{"+QIRD",RSP_QIRD},
	{"+CMTI",RSP_NEWSMS},
	{"+QIND",RSP_QIND},
	{"RDY",RSP_RDY}
};

typedef enum
{
	AT_COMMAND_READY,
	AT_COMMAND_INIT,
	AT_COMMAND_WORK,
	AT_COMMAND_FAIL,
	AT_STATUS_MAX
}AtCommandStatus;

#define AT_STATUS_STRING_MAX_LEN 16
static u8 *find_at_command_write_string_by_index(AtCommandListEnum index);
static GM_ERRCODE at_command_transfer_status(AtCommandStatus new_status);
static void at_command_release(AtCommandPara *one);
const char * at_command_status_string(u8 statu);
static GM_ERRCODE at_command_peek_one(AtCommandPara *one, bool from_head);
static GM_ERRCODE at_command_commit_peek(bool from_head,bool write_log);
static void at_command_write_to_uart(AtCommandPara *one);
static GM_ERRCODE at_command_init_proc(void);
static GM_ERRCODE at_command_work_proc(void);
static bool at_command_scan(const char* p_command,bool keep_space, const char* p_format, ...);
static void at_command_parse_pdu_message(u8 *pdu_msg, u16 pdu_len);






const char s_at_command_status_string[AT_STATUS_MAX][AT_STATUS_STRING_MAX_LEN] = 
{
	"AT_COMMAND_READY",
    "AT_COMMAND_INIT",
    "AT_COMMAND_WORK",
    "AT_COMMAND_FAIL",
};


static u8 *find_at_command_write_string_by_index(AtCommandListEnum index)
{
	u16 idx = 0;

	for(idx=0; idx<AT_CMD_MAX; idx++)
	{
		if (s_at_list[idx].index == index)
		{
			return (u8 *)&s_at_list[idx].cmd;
		}
	}

	return (u8 *)&s_at_list[0].cmd;
}

AtCommandListEnum find_at_command_by_string(char *string)
{
	u16 idx = 0;

	for(idx=0; idx<AT_CMD_MAX; idx++)
	{
		if (0 == GM_memcmp(s_at_list[idx].cmd, string, GM_strlen(string)))
		{
			return (AtCommandListEnum)s_at_list[idx].index;
		}
	}

	return AT_CMD_MAX;
}


static const char * at_command_status_string(u8 statu)
{
    return s_at_command_status_string[statu];
}


static GM_ERRCODE at_command_transfer_status(AtCommandStatus new_status)
{
	u8 old_status = (u8)s_at_command.status;
	GM_ERRCODE ret = GM_PARAM_ERROR;
	switch(s_at_command.status)
	{
		case AT_COMMAND_READY:
			switch(new_status)
			{
				case AT_COMMAND_READY:
					break;
				case AT_COMMAND_INIT:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_WORK:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_FAIL:
					ret = GM_SUCCESS;
					break;
				default:
					break;
			}
			break;
		case AT_COMMAND_INIT:
			switch(new_status)
			{
				case AT_COMMAND_READY:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_INIT:
					break;
				case AT_COMMAND_WORK:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_FAIL:
					break;
				default:
					break;
			}
			break;
		case AT_COMMAND_WORK:
			switch(new_status)
			{
				case AT_COMMAND_READY:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_INIT:
					break;
				case AT_COMMAND_WORK:
					break;
				case AT_COMMAND_FAIL:
					ret = GM_SUCCESS;
					break;
				default:
					break;
			}
			break;
		case AT_COMMAND_FAIL:
			switch(new_status)
			{
				case AT_COMMAND_READY:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_INIT:
					ret = GM_SUCCESS;
					break;
				case AT_COMMAND_WORK:
					break;
				case AT_COMMAND_FAIL:
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if(GM_SUCCESS == ret)
	{
		s_at_command.status = new_status;
		LOG(INFO,"clock(%d) at_command_transfer_status from %s to %s success", util_clock(), at_command_status_string(old_status),at_command_status_string(new_status));
	}
	else
	{
		LOG(ERROR,"clock(%d) at_command_transfer_status assert(from %s to %s) failed", util_clock(), at_command_status_string(old_status),at_command_status_string(new_status));
	}

	return ret;

}

bool at_command_sim_valid(PsFuncPtr handler_func_ptr)
{
	at_command_insert_one(AT_CPIN, COMMAND_READ, NULL, 0, NULL, 0, 300);
	s_at_command.result.sim_call_back = handler_func_ptr;
	return s_at_command.result.sim_valid;
}

u8 at_command_csq_value(void)
{
	at_command_insert_one(AT_CSQ, COMMAND_EXECUTION, NULL, 0, NULL, 0, 300);
	return s_at_command.result.csq_value;
}

bool at_command_creg_register(void)
{
	at_command_insert_one(AT_CREG, COMMAND_READ, NULL, 0, NULL, 0, 300);
	return s_at_command.result.creg;
}

bool at_command_cgreg_register(void)
{
	static u32 interval_clock = 0;
	
	if (util_clock() - interval_clock >= 1)
	{	
		//没有注册到网络时，每10ms查询一次，太频繁
		at_command_insert_one(AT_CGREG, COMMAND_READ, NULL, 0, NULL, 0, 300);
		interval_clock = util_clock();
	}
	return s_at_command.result.cgreg;
}

GM_ERRCODE at_command_get_lbs(LbsCallBackFunc handler_func_ptr)
{
	//gm_cell_info_struct
	at_command_insert_one(AT_QENG, COMMAND_WRITE, "\"servingcell\"", GM_strlen("\"servingcell\""), NULL, 0,300);
	at_command_insert_one(AT_QENG, COMMAND_WRITE, "\"neighbourcell\"", GM_strlen("\"neighbourcell\""), NULL, 0, 300);
	s_at_command.result.lbs_call_back = handler_func_ptr;
	return GM_SUCCESS;
}

GM_ERRCODE at_command_get_imsi(PsFuncPtr handler_func_ptr)
{
	s_at_command.result.imsi_call_back = handler_func_ptr;
	return at_command_insert_one(AT_CIMI, COMMAND_EXECUTION, NULL, 0, NULL, 0, 300);
}

GM_ERRCODE at_command_get_iccid(PsFuncPtr handler_func_ptr)
{
	s_at_command.result.iccid_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QCCID, COMMAND_EXECUTION, NULL, 0, NULL, 0, 300);
}

GM_ERRCODE at_command_set_apn(ST_GprsConfig apnConfig, PsFuncPtr handler_func_ptr)
{
	u8 apn_buff[200] = {0};
	u8 len = 0;

	len = GM_snprintf((char*)&apn_buff, 199, "1,1,\"%.100s\",\"%.32s\",\"%.32s\",%d", apnConfig.apnName, apnConfig.apnUserId, apnConfig.apnPasswd, apnConfig.authtype);
	s_at_command.result.apn_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QICSGP, COMMAND_WRITE, apn_buff, len, NULL, 0, 300);
}

GM_ERRCODE at_command_attach_network(PsFuncPtr handler_func_ptr)
{
	u8 attach = '1';

	//最大等待150秒
	at_command_unattach_network(NULL);
	s_at_command.result.qiact_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QIACT, COMMAND_WRITE, &attach, 1, NULL, 0, 150*TIM_GEN_1SECOND);
}

GM_ERRCODE at_command_unattach_network(PsFuncPtr handler_func_ptr)
{
	u8 unattach = '1';
	s_at_command.result.qiact_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QIDEACT, COMMAND_WRITE, &unattach, 1, NULL, 0, 40*TIM_GEN_1SECOND);
}

GM_ERRCODE at_command_get_local_ip(PsFuncPtr handler_func_ptr)
{
	s_at_command.result.local_ip_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QIACT, COMMAND_READ, NULL, 0, NULL, 0, 300);
}

GM_ERRCODE at_command_get_host_ip(u8 *host_name, PsFuncPtr handler_func_ptr)
{
	u8 host[100] = {0};
	u8 len = 0;

	if (host_name == NULL)
	{
		return GM_PARAM_ERROR;
	}
	len = GM_snprintf((char*)&host, 100, "1,\"%s\"", host_name);
	s_at_command.result.host_ip_call_back = handler_func_ptr;
	return at_command_insert_one(AT_QIDNSGIP, COMMAND_WRITE, host, len, NULL, 0, 60*TIM_GEN_1SECOND);
}

GM_ERRCODE at_command_open_connect(SocketType *socket, PsFuncPtr handler_func_ptr, SocketRecvCallBack recv_func_ptr)
{
	u8 open_buf[100] = {0};
	u16 len = 0;

	len = GM_snprintf((char*)&open_buf, 100, "1,%d,%.5s,\"%d.%d.%d.%d\",%d", socket->access_id, (socket->type == STREAM_TYPE_STREAM) ? "\"TCP\"" : "\"UDP\""
															 , socket->ip[0], socket->ip[1], socket->ip[2], socket->ip[3], socket->port);
	s_at_command.result.sock_notify_call_back = handler_func_ptr;
	s_at_command.result.sock_recv_call_back = recv_func_ptr;
	
	return at_command_insert_one(AT_QIOPEN, COMMAND_WRITE, open_buf, len, NULL, 0, 150*TIM_GEN_1SECOND);
}

GM_ERRCODE at_command_close_connect(SocketIndexEnum index)
{
	u8 write_buf[10] = {0};
	u8 len = 0;

	len = GM_snprintf((char*)&write_buf, 10, "%d", index);
	return at_command_insert_one(AT_QICLOSE, COMMAND_WRITE, write_buf, len, NULL, 0, 10*TIM_GEN_1SECOND);
}


GM_ERRCODE at_command_socket_send(SocketIndexEnum access_id, u8 *data, u16 data_len)
{
	u8 write_buf[10] = {0};
	u8 len = 0;
	GM_ERRCODE ret;
		
	len = GM_snprintf((char*)&write_buf, 10, "%d,%d", access_id, data_len);
	ret = at_command_insert_one(AT_QISEND, COMMAND_WRITE, write_buf, len, data, data_len, 300);
	return ret;
}


GM_ERRCODE at_command_sock_recvive(SocketIndexEnum access_id)
{
	u8 write_buf[20] = {0};
	u8 len = 0;

	len = GM_snprintf((char*)&write_buf, 20, "%d,%d", access_id, MAX_SOCKET_RECV_MSG_LEN);
	at_command_insert_one(AT_QIRD, COMMAND_WRITE, write_buf, len, NULL, 0, 500);
	return GM_SUCCESS;
}

GM_ERRCODE at_command_read_new_message(u8 message_index)
{
	u8 write_buf[10] = {0};
	u8 len = 0;
	len = GM_snprintf((char*)&write_buf, 10, "%d", message_index);
	at_command_insert_one(AT_CMGR, COMMAND_WRITE, write_buf, len, NULL, 0, 300);
	return GM_SUCCESS;
}


GM_ERRCODE at_command_deal_message(u8 message_index)
{
	u8 write_buf[10] = {0};
	u8 len = 0;
	len = GM_snprintf((char*)&write_buf, 10, "%d", message_index);
	at_command_insert_one(AT_CMGD, COMMAND_WRITE, write_buf, len, NULL, 0, 300);
	return GM_SUCCESS;
}


GM_ERRCODE at_command_sms_register(PsFuncPtr handler_func_ptr)
{
	s_at_command.result.new_sms_call_back = handler_func_ptr;
	return GM_SUCCESS;
}


GM_ERRCODE replace_pdu_message(u8 *pdu_message, u16 message_len)
{
	u16 idx = 0;
	u8 *p_des = NULL;

	if (message_len%2)
	{
		return GM_PARAM_ERROR;
	}
	
	p_des = GM_MemoryAlloc(message_len+2);
	if (!p_des)
	{
		return GM_MEM_NOT_ENOUGH;
	}
	GM_memset(p_des, 0, (message_len+2));
	
	for(idx=0; idx<message_len; idx+=2)
	{
		p_des[idx] = pdu_message[idx+1];
		p_des[idx+1] = pdu_message[idx];
	}
	
	GM_memset(pdu_message, 0, message_len);
	GM_memcpy(pdu_message, p_des, message_len);
	GM_MemoryFree(p_des);
	p_des = NULL;
	
	return GM_SUCCESS;
}


GM_ERRCODE at_command_sms_send(char*number, char*content, u16 contentlen, ENUM_SMSAL_DCS doc_type, PsFuncPtr handler_func_ptr)
{
	char value_u8;
	char write_number[30] = {0};
	u8 number_len = 0;
	char write_buf[601] = {0};
	u16 buf_len = 0;
	u8 message_cnt = 0;
	u8 idx = 0;
	u16 message_len = 0;
	
	u16 index = 0;
	u8 *pdu_message = NULL;
	u16 pdu_message_len = 0;
	bool is_group = false;

	s_at_command.result.send_sms_call_back = handler_func_ptr;
	
	if (doc_type == GM_DEFAULT_DCS)
	{
		value_u8 = '1';//设置为TEXT模式
		number_len = GM_snprintf(write_number, 30,"\"%s\"", number);
		at_command_insert_one(AT_CMGF, COMMAND_WRITE, (u8 *)&value_u8,1, NULL, 0, 300);
		message_cnt = (contentlen+160) / 160;
		for(idx=0; idx<message_cnt; idx++)
		{
			GM_memset(write_buf, 0, sizeof(write_buf));
			buf_len = GM_snprintf(write_buf, 160,"%s", &content[160*idx]);
			at_command_insert_one(AT_CMGS, COMMAND_WRITE, (u8 *)write_number,number_len, (u8 *)write_buf, buf_len, 120000);
		}
	}
	else
	{
		//TXT模式发送长短信会变成乱码，暂时采用分开发送模式
		value_u8 = '0';//设置为PDU模式
		at_command_insert_one(AT_CMGF, COMMAND_WRITE, (u8 *)&value_u8,1, NULL, 0, 300);
		pdu_message = GM_MemoryAlloc(1500);
		if (!pdu_message)return GM_MEM_NOT_ENOUGH;
		GM_memset(pdu_message, 0x00, 1500);
		value_u8 = 1;
		if (doc_type == GM_DEFAULT_DCS)
		{
			pdu_message_len = util_pdu_7bit_encoding(pdu_message, content);
			pdu_message_len /= 2;
		}
		else
		{
			GM_memcpy(pdu_message, content, contentlen);
			pdu_message_len = contentlen;
		}
		value_u8 = (pdu_message_len>140) ? ((pdu_message_len + 134) / 134) : 1;
		
		if (value_u8>1)
		{
			is_group = true;
		}
		
		for (idx=0; idx<value_u8; idx++)
		{
			GM_memset(write_buf, 0x00, sizeof(write_buf));
			//00 短信中心号码长度，默认不写
			//11 文件头字节
			//00 信息参考号
			buf_len = GM_snprintf(write_buf, 600, "00%d100", is_group ? 5 : 1);
			//根据长度判断是国内号码还是国际号码
			number_len = GM_strlen(number);
			if (number_len == 0x0D || number_len == 5)
			{
				if (number[0] == '8' && number[1] == '6')
				{
					buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "%02X91", GM_strlen(number));
				}
				else if (number[0] == '1' && number[1] == '0' && number[2] == '6') //物联网卡
				{
					buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "%02XA0", GM_strlen(number));
				}
				else
				{
					buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "%02X", GM_strlen(number));
				}
			}
			else
			{
				buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "%02X81", GM_strlen(number));
			}
			
			//接收短信的号码
			if (number_len % 2)number_len++;
			for(index=0; index<number_len; index+=2)
			{
				if (number[index+1] == '\0')
				{
					write_buf[buf_len+index] = 'F';
				}
				else
				{
					write_buf[buf_len+index] = number[index+1];
				}
				write_buf[buf_len+index+1] = number[index];
			}
			buf_len += number_len;
			//00 协议标识，点对点 
			//doc_type 短信类型
			//FF有效期
			//短消息长度
			message_len = (is_group)? (pdu_message_len>134 ? 134 : pdu_message_len) : pdu_message_len;
			pdu_message_len -= message_len;
			if (doc_type == GM_DEFAULT_DCS)
			{
				buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "00%02XFF%02X", doc_type, is_group ? ((message_len+6)*8/7) :(contentlen*2));
			}
			else
			{
				buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "00%02XFF%02X", doc_type, message_len+ (is_group?6:0));
			}
			
			if (is_group)
			{
				//分包消息05 长度 00 表示分包 03分包长度 分包标识，总包数，当前包数
				buf_len += GM_snprintf(&write_buf[buf_len], 600-buf_len, "050003%02X%02X%02X", (contentlen*2) & 0xFF, value_u8, idx+1);
			}
			
			if (doc_type == GM_DEFAULT_DCS)
			{
				GM_memcpy(&write_buf[buf_len], &pdu_message[134*2*idx], message_len*2);
				buf_len += message_len*2;
			}
			else
			{
				if (doc_type == GM_UCS2_DCS)
				{
					replace_pdu_message(&pdu_message[134*idx], message_len);
				}
				//短消息内容
				for(index=0; index<message_len; index++)
				{
					write_buf[buf_len++] = util_asc((pdu_message[134*idx+index] >> 4) & 0x0F, '0');
					write_buf[buf_len++] = util_asc(pdu_message[134*idx+index] & 0x0F, '0');
				}
			}
			
			number_len = GM_snprintf(write_number, 29, "%d", buf_len/2-1);
			
			at_command_insert_one(AT_CMGS, COMMAND_WRITE, (u8 *)&write_number,number_len, (u8 *)&write_buf, buf_len, 120000);
		}
		GM_MemoryFree(pdu_message);
		pdu_message = NULL;
	}
	
	return GM_SUCCESS;
}


GM_ERRCODE at_command_send_by_third(AtCommandThird *third_cmd, AtCommandTypeEnum type, u8 *data, u16 len)
{
	if (third_cmd == NULL)
	{
		GM_memset(&s_at_command.third_cmd, 0x00, sizeof(s_at_command.third_cmd));
		return GM_PARAM_ERROR;
	}
	if (GM_strlen(s_at_command.third_cmd.cmd_str) > 0)
	{
		return GM_WILL_DELAY_EXEC;
	}
	s_at_command.third_cmd.from = third_cmd->from;
	if (GM_strlen(third_cmd->sms_msg.asciiNum) > 0)
	{
		GM_memcpy(&s_at_command.third_cmd.sms_msg, &third_cmd->sms_msg, sizeof(s_at_command.third_cmd.sms_msg));
	}
	GM_memcpy(s_at_command.third_cmd.cmd_str, third_cmd->cmd_str, GM_strlen(third_cmd->cmd_str));
	return at_command_insert_one(AT_USER, type, data, len, NULL, 0, 300);
}


GM_ERRCODE at_command_create(void)
{
	uart_open_port(GM_UART_AT, BAUD_RATE_HIGH, 0);
	s_at_command.inited = true;
	s_at_command.wait_ask = false;
	s_at_command.command_que = NULL;
	s_at_command.read_idx = 0;
	s_at_command.write_idx = 0;
	s_at_command.clock_ms = 0;
	s_at_command.clock_sec = util_clock();
	s_at_command.fail_count = 0;
	s_at_command.status = AT_COMMAND_INIT;
	s_at_command.command_que = GM_MemoryAlloc(AT_CATCH_MAX_NUM * sizeof(AtCommandPara) + 1);
	s_at_command.recv_buff = GM_MemoryAlloc(RECV_MAX_AT_LEN);
	GM_memset(s_at_command.recv_buff, 0x00, (RECV_MAX_AT_LEN));
	s_at_command.recv_len = 0;
	hard_ware_ec20_reset(true);
	GM_memset(&s_at_command.third_cmd, 0, sizeof(s_at_command.third_cmd));
	if (!hard_ware_is_at_command())
	{
		s_at_command.is_test_at = true;
	}
	else
	{
		s_at_command.is_test_at = false;
	}
	return GM_SUCCESS;
}


GM_ERRCODE at_command_destroy(void)
{
	AtCommandPara one;
	
	s_at_command.inited = false;
	s_at_command.wait_ask = false;
	while(GM_SUCCESS == at_command_peek_one(&one, true))
	{
		at_command_commit_peek(true, true);
	}
	GM_MemoryFree(s_at_command.command_que);
	s_at_command.command_que = NULL;
	GM_MemoryFree(s_at_command.recv_buff);
	s_at_command.recv_buff = NULL;
	uart_close_port(GM_UART_AT);
	at_command_transfer_status(AT_COMMAND_READY);
	return GM_SUCCESS;
}


GM_ERRCODE at_command_timer_proc(void)
{
	switch(s_at_command.status)
	{
		case AT_COMMAND_READY:
			break;
		
		case AT_COMMAND_INIT:
			at_command_init_proc();
			break;

		case AT_COMMAND_WORK:
			at_command_work_proc();
			break;

		case AT_COMMAND_FAIL:
			if (util_clock() - s_at_command.clock_sec >= 1)
			{
				s_at_command.clock_sec = util_clock();
				at_command_transfer_status(AT_COMMAND_INIT);
			}
			break;
	}
	return GM_SUCCESS;
}


GM_ERRCODE at_command_set_nwscanmode(u8 mode)
{
	u8 data[100] = {0};

	GM_snprintf((char*)data, 99, "\"nwscanmode\",%d,1", mode);
	return at_command_insert_one(AT_QCFG, COMMAND_WRITE, data,GM_strlen((char*)data), NULL, 0,1000);
}


static GM_ERRCODE at_command_init_proc(void)
{
	u8 value_u8;
	char urccfg[20] = {"\"urcport\",\"uart1\""};
	if (!s_at_command.inited)
	{
		return GM_SYSTEM_ERROR;
	}


	hard_ware_ec20_reset(false);
	if (util_clock() - s_at_command.clock_sec < 4)
	{
		hard_ware_ec20_power_key(true);
	}
	else
	{
		s_at_command.clock_sec = util_clock();
		at_command_insert_one(AT_AT, COMMAND_EXECUTION, NULL,0, NULL, 0,1000);
		at_command_insert_one(AT_ATE, COMMAND_EXECUTION, NULL,0, NULL, 0,1000);
		at_command_insert_one(AT_GMR, COMMAND_EXECUTION, NULL,0, NULL, 0,1000);
		config_service_get(CFG_4G_NWSCAN,TYPE_BYTE,&value_u8,sizeof(value_u8));
		at_command_set_nwscanmode(value_u8);
		at_command_insert_one(AT_QURCCFG, COMMAND_WRITE, (u8 *)urccfg,GM_strlen(urccfg), NULL, 0,1000);
		value_u8 = '0';
		at_command_insert_one(AT_CMGF, COMMAND_WRITE, &value_u8,1, NULL, 0,1000);
		value_u8 = '3';
		at_command_insert_one(AT_AUTO_ANSWER, COMMAND_WRITE, &value_u8,1, NULL, 0,1000);
		sms_create();
		at_command_transfer_status(AT_COMMAND_WORK);
	}
	
	return GM_SUCCESS;
}


static GM_ERRCODE at_command_work_proc(void)
{ 
	AtCommandPara one;
	SocketType * socket;
	
	if (!s_at_command.inited)
	{
		return GM_SUCCESS;
	}

	at_command_recv();
	s_at_command.clock_sec = util_clock();
	s_at_command.clock_ms += 10;
	
	if (s_at_command.clock_ms < 300)
	{
		return GM_WILL_DELAY_EXEC;
	}

	if (s_at_command.is_test_at)
	{
		if (util_clock() >= 60)
		{
			at_command_destroy();
			return GM_SUCCESS;
		}
		uart_write(GM_UART_AT, "AT\r\n", GM_strlen("AT\r\n"));
		s_at_command.clock_ms = 0;
		return GM_SUCCESS;
	}
	
	while(GM_SUCCESS == at_command_peek_one(&one, true))
	{
		if (s_at_command.fail_count >= AT_SEND_MAX_CNT)
		{
			at_command_commit_peek(true, true);
			s_at_command.fail_count = 0;
		}
		else if (s_at_command.fail_count == 0 || s_at_command.clock_ms >= one.delay_time)
		{
			if (one.index == AT_QISEND || one.index == AT_QISENDEX || one.index == AT_QIRD)
			{
				socket = get_socket_by_accessid(util_chr(one.data[0]));
				if (socket->id < 0)
				{
					at_command_commit_peek(true, true);
					s_at_command.fail_count = 0;
					break;
				}
			}
			s_at_command.fail_count++;
			s_at_command.clock_ms = 0;
			at_command_write_to_uart(&one);
			break;
		}
		else
		{
			break;
		}
	}

	return GM_SUCCESS;
}


static void at_command_write_to_uart(AtCommandPara *one)
{
	u8 *cmd_buff = NULL;
	u16 cmd_len = 0;

	if (!one->data2 || !one->data)
	{
		at_command_commit_peek(true, true);
		return;
	}

	cmd_buff = GM_MemoryAlloc(one->data_len + MAX_TYPE_LEN);
	if (cmd_buff == NULL)
	{
		return;
	}
	GM_memset(cmd_buff, 0x00, (one->data_len + MAX_TYPE_LEN));

	if (AT_USER == one->index)
	{
		cmd_len += GM_snprintf((char*)&cmd_buff[cmd_len], (one->data_len + MAX_TYPE_LEN), "%s", s_at_command.third_cmd.cmd_str);
	}
	else
	{
		cmd_len += GM_snprintf((char*)&cmd_buff[cmd_len], (one->data_len + MAX_TYPE_LEN), "%s", find_at_command_write_string_by_index(one->index));
	}
	switch (one->type)
	{
		case COMMAND_WRITE:
			cmd_buff[cmd_len++] = '=';
			GM_memcpy(&cmd_buff[cmd_len], one->data, one->data_len);
			cmd_len += one->data_len;
			break;
		case COMMAND_READ:
			cmd_buff[cmd_len++] = '?';
			break;
		case COMMAND_TEST:
			cmd_len += GM_snprintf((char*)&cmd_buff[cmd_len], (one->data_len + MAX_TYPE_LEN-cmd_len), "=?");
			break;
		default://COMMAND_EXECUTION
			break;
	}
	//cmd_len += GM_snprintf((char*)&cmd_buff[cmd_len], (one->data_len + MAX_TYPE_LEN-cmd_len), "\r\n");
	cmd_buff[cmd_len++] = '\r';
	cmd_buff[cmd_len++] = '\n';
	if (AT_USER == one->index)
	{
		uart_write(GM_UART_DEBUG, cmd_buff, cmd_len);
	}
	uart_write(GM_UART_AT, cmd_buff, cmd_len);
	GM_MemoryFree(cmd_buff);
	cmd_buff = NULL;
}


GM_ERRCODE at_command_insert_one(AtCommandListEnum index, AtCommandTypeEnum type, u8 *data, u16 len, u8 *data2, u16 data2_len, u32 delay_time)
{
    int write_point = 0;
    int write_idx;

	if (!s_at_command.inited)
	{
		return GM_SYSTEM_ERROR;
	}
	
    write_idx = write_point = s_at_command.write_idx;

    write_point++;
    if(write_point >= AT_CATCH_MAX_NUM)
    {
        write_point -= (int)AT_CATCH_MAX_NUM;
    }

    if (write_point == s_at_command.read_idx)
    {
    	return GM_MEM_NOT_ENOUGH;
    }
    
	s_at_command.command_que[write_idx].data = (char *)GM_MemoryAlloc(len+1);
    if(NULL == s_at_command.command_que[write_idx].data)
    {
        return GM_SYSTEM_ERROR;
    }
    
    GM_memset(s_at_command.command_que[write_idx].data, 0, (len+1));
	s_at_command.command_que[write_idx].data_len = len;
	GM_memcpy(s_at_command.command_que[write_idx].data, data, len);

	//SOCKET 数据
	s_at_command.command_que[write_idx].data2 = (char *)GM_MemoryAlloc(data2_len+1);
    if(NULL == s_at_command.command_que[write_idx].data2)
    {
        return GM_SYSTEM_ERROR;
    }
    GM_memset(s_at_command.command_que[write_idx].data2, 0, (data2_len+1));
	s_at_command.command_que[write_idx].data2_len = data2_len;
	GM_memcpy(s_at_command.command_que[write_idx].data2, data2, data2_len);
    
    s_at_command.command_que[write_idx].delay_time = delay_time;
    s_at_command.command_que[write_idx].index = index;
    s_at_command.command_que[write_idx].type = type;

    s_at_command.write_idx = write_point;

    return GM_SUCCESS;
}


static GM_ERRCODE at_command_peek_one(AtCommandPara *one, bool from_head)
{
    int read_point = 0;
    int write_point = 0;
    int idx;

    if(s_at_command.command_que == NULL)
    {
    	LOG(ERROR,"at_command_peek_one command_que null");
        return GM_EMPTY_BUF;
    }

    read_point = s_at_command.read_idx;
    write_point = s_at_command.write_idx;
    
    if(read_point == write_point)
    {
        return GM_EMPTY_BUF;
    }
    
    idx = from_head ? read_point : write_point;
    *one = s_at_command.command_que[idx];
    
    return GM_SUCCESS;
}


static GM_ERRCODE at_command_commit_peek(bool from_head,bool write_log)
{
    int read_point = 0;
    int write_point = 0;
    int idx = 0;
	int len = 0;
    
    read_point = s_at_command.read_idx;
    write_point = s_at_command.write_idx;

    if(write_point == read_point)
    {
        return GM_EMPTY_BUF;
    }

    if(from_head)
    {
        //删除头部一个元素
        idx = read_point;
        len = s_at_command.command_que[idx].data_len;
        at_command_release(&s_at_command.command_que[idx]);
        read_point ++;
        if(read_point >= AT_CATCH_MAX_NUM)
        {
            read_point -= (int)AT_CATCH_MAX_NUM;
        }
        s_at_command.read_idx = read_point;
    }
    else
    {
        write_point--;
        if(write_point < 0)
        {
            write_point += (int)AT_CATCH_MAX_NUM;
        }
        idx = write_point;
        len = s_at_command.command_que[idx].data_len;
        at_command_release(&s_at_command.command_que[idx]);
        s_at_command.write_idx = write_point;
    }

    if(write_log)
    {
        LOG(DEBUG,"clock(%d) at_command_commit_peek(r:%d,w:%d,len:%d, from_head:%d).", util_clock(),
            s_at_command.read_idx,s_at_command.write_idx,len,from_head);
    }
    return GM_SUCCESS;
}




static void at_command_release(AtCommandPara *one)
{
    if(one)
    {
    	if (one->data)
    	{
	        GM_MemoryFree(one->data);
	        one->data = NULL;
	        one->data_len = 0;
        }
        if (one->data2)
    	{
	        GM_MemoryFree(one->data2);
	        one->data2 = NULL;
	        one->data2_len = 0;
        }
    }
}


static bool at_command_isfield(char c) 
{
    return (c != ',' && c != '\"' && c != '\r' && c != '\n' && c != '=' && c != ':');
}



/**
 * Function:   从语句中提取变量
 * Description:
 * Input:	   p_sentence:语句；format——格式；
 * Output:	   ...——变量
 * Return:	   true——成功；false——失败
 * Others:	   类似于scanf的处理方式,支持下面几种格式:
               c - single character (char*)
               d - direction, returned as 1/-1, default 0 (S32*)
               f - fractional, returned as value + scale (S32*, S32*)
               i - decimal, default zero (S32*)
               s - string (char*)
               t - talker identifier and type (char*)
               T - date/time stamp (S32*, S32*, S32*)              
 */         
static bool at_command_scan(const char* p_command,bool keep_space, const char* p_format, ...)
{
    bool para_num = false;
    bool optional = false;
	const char* p_field = p_command;
	char type = 0;
	S32 value_32 = 0;
	char* p_buf = NULL;
	U8 index = 0;
	
    va_list ap;
    va_start(ap, p_format);

    while (*p_format) 
	{
        type = *p_format++;

        if (type == ';') 
		{
            // 后面所有的域都是可选的
            optional = true;
            continue;
        }

        if (!p_field && !optional) 
		{
            goto parse_error;
        }

        index = 0;

        switch (type) 
		{
			case 'c': 
			{ 
				char value = 0;
                if (p_field && util_isprint((U8)*p_field) && at_command_isfield(*p_field) && *p_field != ' ')
                {
                    value = *p_field;
					para_num++;
                }
                *va_arg(ap, char*) = value;
                index = 1;
            } 
			break;
			
			// Integer value, default 0 (S32).
            case 'i': 
			{ 
                value_32 = 0;

                if (p_field && util_isdigit(*p_field)) 
				{	
                    char *endptr;
                    value_32 = util_strtol(p_field, &endptr);
                    if (util_isprint((U8)*endptr) && at_command_isfield(*endptr) && *endptr != ' ')
                    {
                        goto parse_error;
                    }
					para_num++;
					index = endptr-p_field;
                }
                *va_arg(ap, S32*) = value_32;
				
            } 
			break;

			// String value (char *).
            case 's': 
			{ 
                p_buf = va_arg(ap, char*);

                if (p_field && at_command_isfield(*p_field) && *p_field != ' ' || *p_field == '\"')
				{
                    while (at_command_isfield(*p_field) || *p_field == '\"')
                    {
                    	if (*p_field == '\"' || (*p_field == ' ' && !keep_space))
                    	{
                    		p_field++;
                    	}
                    	else
                    	{
                    		*p_buf++ = *p_field++;
                    	}
                    }
					para_num++;
                }
                *p_buf = '\0';
            } 
			break;

			case 't': 
			{ 
                // This p_field is always mandatory.
                if (!p_field)
                {
                    goto parse_error;
                }

                while(at_command_isfield(p_field[index]) && index < MAX_TYPE_LEN)
                {
             		index++;
                }

                p_buf = va_arg(ap, char*);
                GM_memcpy(p_buf, p_field, index);
                para_num++;
                p_buf[index] = '\0';
            }
			break;

            default:
			{ 
                goto parse_error;
            }
        }

		p_field +=index;
    	while (*p_field == ',' || *p_field == '\"' || *p_field == '\r' || *p_field == '\n' || *p_field == ':' || *p_field == ' ')
    	{
        	p_field++;
    	}
    	
    	/* Make sure there is a p_field there. */ 
    	if (*p_field == '\0') 
		{ 
        	p_field = NULL;
    	} 
    }

parse_error:
    va_end(ap);
    return para_num;
}

u16 at_command_get_item(char* p_src, u16 src_len, char* p_dest, u8 index, char item_flag)
{
	char* p_str = p_src;
	u16 src_idx = 0;
	u16 len = src_len;
	u8 item_index = 0;
	u16 dest_len = 0;

	if (index > 0)
	{
		for (src_idx=0; src_idx<len; src_idx++)
		{
			if (*p_str == item_flag)
			{
				item_index++;
			}
			p_str++;
			if (item_index == index)
			{
				break;
			}
		}
	}

	if (item_index < index)
	{
		//没有找到
		return 0;
	}

	len -= (p_str-p_src);
	
	for (src_idx=0; src_idx<len; src_idx++)
	{
		
		if (*p_str != item_flag)
		{
			*p_dest = *p_str;
			dest_len++;
			p_str++;
			p_dest++;
		}
		else
		{			
			*p_dest = '\0';
			break;
		}
	}

	return dest_len;
}


AtCommandListEnum at_command_sentence_id(const char* p_sentence, const U16 len, u16 *start_len)
{
	char type[MAX_TYPE_LEN] = {0};
	u16 index = 0;
	AtCommandPara one;
    
    if (!at_command_scan(p_sentence, false, "t", type))
    {
    	LOG(WARN ,"clock(%d) at_command_sentence_id error type(%s)", util_clock(), type);
        return AT_CMD_MAX;
    }

	if (GM_strlen(s_at_command.third_cmd.cmd_str) &&  0 == GM_memcmp(type, s_at_command.third_cmd.cmd_str,GM_strlen(s_at_command.third_cmd.cmd_str)))
	{
		index = AT_USER;
	}
	else
	{
		for(index=AT_CMD_MAX; index>0; --index)
		{
			if (GM_strlen(s_at_list[index].cmd) > 0 && GM_memcmp(type, s_at_list[index].cmd, GM_strlen(s_at_list[index].cmd)) == 0)
			{
				break;
			}
		}
	}


	if (index == AT_NONE)
	{
		return AT_NONE;
	}

	at_command_peek_one(&one, true);
	if (s_at_list[index].index == AT_QIURC || s_at_list[index].index == AT_QIOPENURC || s_at_list[index].index == AT_SMSURC)
	{
		*start_len = 0;
	}
	else if ((s_at_list[index].index == AT_QISEND || s_at_list[index].index == AT_CMGS) && p_sentence[len-2] == '>')
	{
		*start_len += GM_strlen(s_at_list[one.index].cmd);
		*start_len += (1+one.data_len);
		*start_len += 3;
	}
	else
	{
		*start_len += (index == AT_USER) ? GM_strlen(s_at_command.third_cmd.cmd_str) : GM_strlen(s_at_list[one.index].cmd);
		switch(one.type)
		{
			case COMMAND_WRITE:
				*start_len += (1+one.data_len);
				break;
			case COMMAND_READ:
				*start_len += 1;
				break;
			case COMMAND_TEST:
				*start_len += 2;
				break;
			default://COMMAND_EXECUTION
				break;
		}
		
		*start_len += one.data2_len + 3;
	}

    return (AtCommandListEnum)s_at_list[index].index;
}

static u16 at_command_remove_carriage_return_in_head(u8 *data, u16 len)
{
	u8 *dest_data = NULL;
	u16 dest_len = 0;
	u16 index = 0;

	dest_data = GM_MemoryAlloc(len + 1);
	if (!dest_data)return len;
	for (index=0; index<len;index++)
	{
		if (data[index]!='\r' && data[index]!='\n')
		{
			GM_memcpy(dest_data, &data[index], len-index);
			dest_len = len-index;
			break;
		}
	}

	GM_memset(data, 0x00, len);
	GM_memcpy(data, dest_data, dest_len);
	GM_MemoryFree(dest_data);
	dest_data = NULL;
	return dest_len;
}

void at_command_recv(void)
{
	GM_ERRCODE ret;
	u32 fifo_len = 0;
	u16 sentence_len;
	u16 idx = 0;
	int jval = -1;
	char *p_check_rsp = NULL;
	u16 remove_len = 0;
	static bool is_pop = true;
	
	s_at_command.rcv_fifo = get_uart_recv_fifo(GM_UART_AT);
	if (!s_at_command.rcv_fifo)
	{
		return;
	}

	//先读取所有数据
	GM_memset(s_at_command.recv_buff, 0x00, RECV_MAX_AT_LEN);
	fifo_len = fifo_get_msg_length(s_at_command.rcv_fifo);
	if (fifo_len == 0)
	{
		return;
	}
	fifo_len = (fifo_len > RECV_MAX_AT_LEN) ? RECV_MAX_AT_LEN : fifo_len;
	ret = fifo_peek(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, fifo_len);
	if (GM_SUCCESS != ret)
	{
		return;
	}

	//测试AT指令响应
	if (s_at_command.is_test_at)
	{
		if (GM_strstr(s_at_command.recv_buff, "OK\r\n")
		   || GM_strstr(s_at_command.recv_buff, "RDY\r\n"))
		{
			fifo_pop_len(s_at_command.rcv_fifo, fifo_len);
			s_at_command.is_test_at = false;
			config_service_set_device(DEVICE_GS08,false);
	        config_service_save_to_local();
			gprs_destroy();
		}
		return;
	}

	//pop掉不合法数据
	remove_len = at_command_remove_carriage_return_in_head((u8 *)s_at_command.recv_buff, fifo_len);
	fifo_pop_len(s_at_command.rcv_fifo, fifo_len-remove_len);
	fifo_len = remove_len;

	
	
	//检查完整性截取数据长度
	//比较前N个字符,得出URC数据
	for(idx=0; idx<RSP_MAX; idx++)
	{
		if (0 == GM_strncmp(s_at_command.recv_buff, s_at_rsp[idx].rsp_string, GM_strlen(s_at_rsp[idx].rsp_string)))
		{
			p_check_rsp = s_at_command.recv_buff;
			break;
		}
	}

	//获取rsp数据
	if (idx>= RSP_MAX)
	{
		for(idx=0; idx<RSP_MAX; idx++)
		{
			jval = util_memmem(s_at_command.recv_buff, fifo_len, s_at_rsp[idx].rsp_string, GM_strlen(s_at_rsp[idx].rsp_string));
			if (jval >= 0)
			{
				p_check_rsp = &s_at_command.recv_buff[jval];
				if (idx == RSP_SOCKETSEND)
				{
					if (p_check_rsp[0] != '>' || p_check_rsp[1] != ' ' || p_check_rsp[2] != '\0')
					{
						continue;
					}
				}
				else if (idx != RSP_CME_ERROR && idx < RSP_SOCKETSEND)
				{
					if (p_check_rsp[GM_strlen(s_at_rsp[idx].rsp_string)] != '\r' || p_check_rsp[GM_strlen(s_at_rsp[idx].rsp_string)+1] != '\n')
					{
						continue;
					}
				}
				break;
			}
		}
	}

	if (idx >= RSP_MAX)
	{
		sentence_len = fifo_get_msg_length(s_at_command.rcv_fifo);
		ret = fifo_peek(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, sentence_len);
		is_pop = false;
		
		jval = util_memmem(s_at_command.recv_buff, fifo_len, "OK\r\n", GM_strlen("OK\r\n"));
		if (jval >= 0)
		{
			s_at_command.recv_len = jval + GM_strlen("OK\r\n");
			is_pop = true;
		}
		if (!is_pop)
		{
			return;
		}
		sentence_len = s_at_command.recv_len;
	}
	else
	{
		GM_memset(s_at_command.recv_buff, 0, RECV_MAX_AT_LEN);
		fifo_len = fifo_get_msg_length(s_at_command.rcv_fifo);
		if (idx < RSP_SOCKETSEND)
		{
			//数据+结尾标志+\r\n
			sentence_len = p_check_rsp - s_at_command.recv_buff + GM_strlen(s_at_rsp[idx].rsp_string) + 2;
			sentence_len = (sentence_len>fifo_len) ? fifo_len : sentence_len;
			ret = fifo_peek(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, sentence_len);
			s_at_command.recv_len = sentence_len;
			if (RSP_CME_ERROR == idx)
			{
				fifo_pop_len(s_at_command.rcv_fifo, sentence_len);
				sentence_len = RECV_MAX_AT_LEN;
				ret = fifo_peek_until(s_at_command.rcv_fifo, (U8*)&s_at_command.recv_buff[s_at_command.recv_len], &sentence_len, '\n');
				s_at_command.recv_len += sentence_len;
			}
			is_pop = true;
		}
		else if (idx == RSP_SOCKETSEND)
		{
			//需要等待数据发送完成，第一次收到'>'时发送数据，这时不能pop
			if (!is_pop)
			{
				return;
			}
			//数据+结尾标志+' '
			sentence_len = p_check_rsp - s_at_command.recv_buff + GM_strlen(s_at_rsp[idx].rsp_string) + 1;
			sentence_len = (sentence_len>fifo_len) ? fifo_len : sentence_len;
			ret = fifo_peek(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, sentence_len);
			s_at_command.recv_len = sentence_len;
			is_pop = false;
		}
		else if (idx == RSP_QIRD)
		{
			//读取数据的时候，有HEX字段，暂时先全部读取
			sentence_len = fifo_get_msg_length(s_at_command.rcv_fifo);
			ret = fifo_peek(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, sentence_len);
			is_pop = false;
			
			jval = util_memmem(s_at_command.recv_buff, fifo_len, "\r\nOK\r\n", GM_strlen("\r\nOK\r\n"));
			if (jval >= 0)
			{
				s_at_command.recv_len = jval + GM_strlen("\r\nOK\r\n");
				is_pop = true;
			}
			if (!is_pop)
			{
				return;
			}
			sentence_len = s_at_command.recv_len;
		}
		else
		{
			//URC一直到换行
			sentence_len = RECV_MAX_AT_LEN;
			ret = fifo_peek_until(s_at_command.rcv_fifo, (U8*)s_at_command.recv_buff, &sentence_len, '\n');
			s_at_command.recv_len = sentence_len;
			is_pop = true;
		}
	}

	if (GM_SUCCESS != ret)
	{
		sentence_len = fifo_get_msg_length(s_at_command.rcv_fifo);
		fifo_pop_len(s_at_command.rcv_fifo, sentence_len);
		return;
	}
	
	
	if (is_pop)
	{
		fifo_pop_len(s_at_command.rcv_fifo, sentence_len);
	}
	
	at_command_on_receive_data(s_at_command.recv_buff, s_at_command.recv_len, NULL);
}


GM_ERRCODE at_command_on_receive_data(char* p_cmd, u16 cmd_len, char* p_rsp)
{
	char para_num = 0;
	AtCommandListEnum command_id = AT_NONE;
	AtCommandPara one;
	u16 start_len = 0;
	u8 value_u8 = 0;
	u8 value_u8_1 = 0;
	u8 value_u8_2 = 0;
	u32 value_32 = 0;
	char *p_str = p_cmd;
	u16 str_len = cmd_len;
	char string_value[30] = {0};
	char string_value_1[30] = {0};
	char string_value_2[30] = {0};
	
	if (p_cmd == NULL || cmd_len == 0)
	{
		return GM_SYSTEM_ERROR;
	}

	command_id = at_command_sentence_id(p_str, str_len, &start_len);
	if (cmd_len > start_len)
	{
		p_str += start_len;
		str_len = cmd_len-start_len;
	}

	if (p_str == NULL)
	{
		return GM_SYSTEM_ERROR;
	}

	at_command_peek_one(&one, true);
	switch(command_id)
	{
		case AT_RDY:
			break;
			
		case AT_USER:
			if (s_at_command.third_cmd.from == COMMAND_SMS)
			{
				sms_send(p_str, str_len, s_at_command.third_cmd.sms_msg.asciiNum, GM_DEFAULT_DCS);
			}
			else if (s_at_command.third_cmd.from == COMMAND_GPRS)
			{
				gps_service_after_receive_remote_msg((u8 *)p_str, str_len);
			}
			else
			{
				uart_write(GM_UART_DEBUG, (U8*)p_str, str_len);
			}
			GM_memset(&s_at_command.third_cmd, 0x00, sizeof(s_at_command.third_cmd));
			break;
			
		case AT_AT:
			at_command_scan(p_str, false, "s", string_value);
			break;

		case AT_ATE:
			at_command_scan(p_str, false, "s", string_value);
			break;

		case AT_CPIN:
			/*
			+CPIN: READY
			
			OK
			*/
			at_command_scan(p_str, false, "ss", string_value, string_value_1);
			if (!GM_strcmp(string_value_1, "READY"))
			{
				s_at_command.result.sim_valid = true;
			}
			else
			{
				s_at_command.result.sim_valid = false;
			}
			s_at_command.result.sim_call_back((void*)&s_at_command.result.sim_valid);
			break;

		case AT_CSQ:
			{
				at_command_scan(p_str, false, "si", string_value, &value_u8);
				s_at_command.result.csq_value = value_u8;
			}
			break;

		case AT_CREG:
			if (one.type == COMMAND_READ)
			{
				at_command_scan(p_str, false, "sii;ss", string_value, &value_u8, &value_u8_1, string_value_1, string_value_2);
				if (value_u8_1 == 1 || value_u8_1 == 5)
				{
					s_at_command.result.creg = true;
				}
				else
				{
					s_at_command.result.creg = false;
				}
			}
			break;

		case AT_CGREG:
			at_command_scan(p_str, false, "sii", string_value, &value_u8, &value_u8_1);
			if (value_u8_1 == 1 || value_u8_1 == 5)
			{
				s_at_command.result.cgreg = true;
			}
			else
			{
				s_at_command.result.cgreg = false;
			}
			break;

		case AT_GMR:
			/*
			AT+GMR
			EC20CEHDLGR06A02M1G

			OK
			*/
			GM_memset(s_at_command.result.verno, 0x00, sizeof(s_at_command.result.verno));
			at_command_scan(p_str, false, "s", s_at_command.result.verno);
			break;

		case AT_CIMI:
			/*
			AT+CIMI
			460046163005420

			OK
			*/
			at_command_scan(p_str, false, "s", string_value);
			if (s_at_command.result.imsi_call_back)
			{
				s_at_command.result.imsi_call_back(string_value);
			}
			break;

		case AT_QCCID:
			/*
			AT+QCCID
			+QCCID: 89860441191890955420

			OK
			*/
			at_command_scan(p_str, false, "ss", string_value, string_value_1);
			if (s_at_command.result.iccid_call_back)
			{
				s_at_command.result.iccid_call_back(string_value_1);
			}
			break;

		case AT_QENG:
			{
				//4G
				/*
				+QENG: "servingcell","NOCONN","LTE","TDD",460,00,C412C83,401,38950,40,5,5,247C,-90,-7,-62,24,37

				OK
				*/
				/*
				+QENG: "neighbourcell intra","LTE",38950,401,-7,-90,-62,-20,37,6,22,6,46
				+QENG: "neighbourcell inter","LTE",37900,-,-,-,-,-,-,0,22,6
				+QENG: "neighbourcell inter","LTE",38098,-,-,-,-,-,-,0,22,6

				OK
				*/
				//2G
				/*
				AT+QENG="servingcell"
				AT+QENG="servingcell"
				+QENG: "servingcell","NOCONN","GSM",460,00,247C,1327,35,82,-,-52,255,255,0,53,179,1,-,-,-,-,-,-,-,-,-,"-"

				OK

				AT+QENG="neighbourcell"
				AT+QENG="neighbourcell"
				+QENG: "neighbourcell","GSM",460,00,24A5,1241,33,33,-83,22,146,0,0
				+QENG: "neighbourcell","GSM",460,00,2495,11C8,60,614,-87,12,128,0,0
				+QENG: "neighbourcell","GSM",460,00,247C,1326,58,16,-92,13,139,0,0
				+QENG: "neighbourcell","GSM",460,00,24A5,FCB,3,616,-90,9,125,0,0
				+QENG: "neighbourcell","GSM",460,00,2495,14B7,24,622,-93,6,124,0,0

				OK
				*/
				//SEARCH
				char qeng_data[500] = {0};
				u16 qeng_len = 0;
				bool is_recv_nbr_cell = false;
				bool is_lte = false;
				static gm_cell_info_struct cell_info;
				gm_lte_cell_info_struct lte_cell_info;
				
				GM_memset(qeng_data, 0x00, sizeof(qeng_data));
				qeng_len = at_command_get_item(p_str, str_len, qeng_data, 0, '\n');
				if (qeng_len<10)
				{
					if (!GM_strcmp(one.data, "\"neighbourcell\""))
					{
						if (s_at_command.result.lbs_call_back)
						{
							s_at_command.result.lbs_call_back(true, NULL);
						}
					}
					break;
				}
				p_str += (qeng_len+1);
				str_len -= (qeng_len+1);
				do
				{
					at_command_scan(qeng_data,true, "sss", string_value, string_value_1, string_value_2);
					if (!GM_strcmp(string_value_2, "SEARCH"))
					{
						if (s_at_command.result.lbs_call_back)
						{
							s_at_command.result.lbs_call_back(true, NULL);
						}
						break;
					}
					else if (!GM_strcmp(string_value_1, "servingcell"))
					{
						GM_memset(&cell_info, 0, sizeof(gm_cell_info_struct));
						GM_memset(&lte_cell_info, 0, sizeof(gm_lte_cell_info_struct));
						GM_memset(string_value, 0x00, sizeof(string_value));
						if (!at_command_get_item(qeng_data, qeng_len, string_value, 2, ','))
						{
							break;
						}
						if (!GM_strcmp(string_value, "\"GSM\""))
						{
							cell_info.nbr_cell_num = 0;
							if (at_command_get_item(qeng_data, qeng_len, string_value, 3, ','))
							{
								cell_info.serv_info.mcc = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 4, ','))
							{
								cell_info.serv_info.mnc = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 5, ','))
							{
								cell_info.serv_info.lac = util_hexstrtoul(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 6, ','))
							{
								cell_info.serv_info.ci = util_hexstrtoul(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 7, ','))
							{
								cell_info.serv_info.bsic = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 8, ','))
							{
								cell_info.serv_info.arfcn = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 10, ','))
							{
								cell_info.serv_info.rxlev = GM_atoi(string_value) + 110;
							}
						}
						else if (!GM_strcmp(string_value, "\"LTE\""))
						{
							is_lte = true;
							if (at_command_get_item(qeng_data, qeng_len, string_value, 3, ','))
							{
								lte_cell_info.is_tdd = GM_strcmp(string_value, "\"TDD\"") ? false : true;
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 4, ','))
							{
								lte_cell_info.mcc = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 5, ','))
							{
								lte_cell_info.mnc = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 6, ','))
							{
								lte_cell_info.ci = util_hexstrtoul(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 8, ','))
							{
								lte_cell_info.earfcn = GM_atoi(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 12, ','))
							{
								lte_cell_info.lac = util_hexstrtoul(string_value);
							}
							if (at_command_get_item(qeng_data, qeng_len, string_value, 15, ','))
							{
								lte_cell_info.srxlev = GM_atoi(string_value)+110;
							}
						}
					}
					else if (!GM_strcmp(string_value_1, "neighbourcell"))
					{
						if (is_lte)break;
						GM_memset(string_value, 0x00, sizeof(string_value));
						if (!at_command_get_item(qeng_data, qeng_len, string_value, 1, ','))
						{
							break;
						}
						if (GM_strcmp(string_value, "\"GSM\""))
						{
							break;
						}
						is_recv_nbr_cell = true;
						if (at_command_get_item(qeng_data, qeng_len, string_value, 2, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].mcc = GM_atoi(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 3, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].mnc = GM_atoi(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 4, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].lac = util_hexstrtoul(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 5, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].ci = util_hexstrtoul(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 6, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].bsic = GM_atoi(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 7, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].arfcn = GM_atoi(string_value);
						}
						if (at_command_get_item(qeng_data, qeng_len, string_value, 8, ','))
						{
							cell_info.nbr_cell_info[cell_info.nbr_cell_num].rxlev = GM_atoi(string_value) + 110;
						}
						cell_info.nbr_cell_num++;
					}
					else
					{
						break;
					}
					qeng_len = at_command_get_item(p_str, str_len, qeng_data, 0, '\n');
					p_str += (qeng_len+1);
					str_len -= (qeng_len+1);
				}while(qeng_len>10);
				
				if (is_recv_nbr_cell)
				{
					if (s_at_command.result.lbs_call_back)
					{
						s_at_command.result.lbs_call_back(false, (void*)&cell_info);
					}
				}
				else if (is_lte)
				{
					is_lte = false;
					if (s_at_command.result.lbs_call_back)
					{
						s_at_command.result.lbs_call_back(true, (void*)&lte_cell_info);
					}
				}
			}
			break;

		case AT_QICSGP:
			{
				bool is_apn_ok = false;
				at_command_scan(p_str,false, "s", string_value);
				if (!GM_strcmp(string_value, "OK"))
				{
					is_apn_ok = true;
				}
				if (s_at_command.result.apn_call_back)
				{
					s_at_command.result.apn_call_back((void *)&is_apn_ok);
				}
			}
			break;

		case AT_QIDEACT:
			{
				bool is_deact_ok = false;
				at_command_scan(p_str,false, "s", string_value);
				if (!GM_strcmp(string_value, "OK"))
				{
					is_deact_ok = true;
					s_at_command.result.qiact_call_back((void *)&is_deact_ok);
				}
			}
			break;

		case AT_QIACT:
			//激活
			if (one.type == COMMAND_WRITE)
			{
				bool is_success = false;
				at_command_scan(p_str,false, "s", string_value);
				if (!GM_strcmp(string_value, "OK"))
				{
					is_success = true;
				}
				if (s_at_command.result.qiact_call_back)
				{
					s_at_command.result.qiact_call_back((void *)&is_success);
				}
			}
			//查询本地IP
			else if (one.type == COMMAND_READ)
			{
				u8 local_ip[4] = {0};
				at_command_scan(p_str,false, "siiis", string_value, &value_u8, &value_u8_1, &value_u8_2, string_value_1);
				if (GM_strlen(string_value_1) > 0)
				{
					GM_ConvertIpAddr((u8 *)string_value_1, (u8 *)&local_ip);
				}
				if (s_at_command.result.local_ip_call_back)
				{
					s_at_command.result.local_ip_call_back(NULL);
				}
			}
			break;

		case AT_QIDNSGIP:
			//para_num = at_command_scan(p_str,false, "s", string_value);
			break;

		case AT_QIURC:
			{
				static bool is_dnsip = false;

				at_command_scan(p_str,true, "ss", string_value, string_value_1);
				if (!GM_strcmp(string_value_1, "dnsgip"))
				{
					u8 host_ip[4] = {0};
					//+QIURC: "dnsgip",0,1,170
					//+QIURC: "dnsgip","47.106.251.151"
					if (p_str[17] != '\"')
					{
						at_command_scan(&p_str[17],false, "iii", &value_u8, &value_u8_1, &value_32);
						is_dnsip = true;
					}
					else
					{
						at_command_scan(&p_str[18],false, "s", string_value_2);
						//只需要第一个IP
						if (is_dnsip)
						{
							if (GM_strlen(string_value_2) > 0)
							{
								GM_ConvertIpAddr((u8 *)string_value_2, (u8 *)&host_ip);
							}
							//LOG(DEBUG, "host_ip_call_back:(%s)(%d.%d.%d.%d)", string_value_2, host_ip[0], host_ip[1], host_ip[2], host_ip[3]);
							if (s_at_command.result.host_ip_call_back)
							{
								s_at_command.result.host_ip_call_back((void *)host_ip);
							}
							is_dnsip = false;
						}
					}
				}
				//+QIURC: "recv",0
				else if (!GM_strcmp(string_value_1, "recv"))
				{
					gm_soc_notify_at_command_struct notify_msg;
					notify_msg.access_id = (SocketIndexEnum)util_chr(p_str[15]);
					notify_msg.detail_cause = 0;
					notify_msg.event_type = GM_SOC_READ;
					notify_msg.result = KAL_TRUE;
					if (s_at_command.result.sock_notify_call_back)
					{
						s_at_command.result.sock_notify_call_back((void *)&notify_msg);
					}
				}
				//+QIURC: "pdpdeact",1
				else if (!GM_strcmp(string_value_1, "pdpdeact"))
				{
					bool is_success = false;
					if (s_at_command.result.qiact_call_back)
					{
						s_at_command.result.qiact_call_back((void *)&is_success);
					}
				}
				//+QIURC: "closed",1
				else if (!GM_strcmp(string_value_1, "closed"))
				{
					gm_soc_notify_at_command_struct notify_msg;
					notify_msg.access_id = (SocketIndexEnum)util_chr(p_str[17]);
					notify_msg.detail_cause = 0;
					notify_msg.event_type = GM_SOC_CLOSE;
					notify_msg.result = KAL_TRUE;
					if (s_at_command.result.sock_notify_call_back)
					{
						s_at_command.result.sock_notify_call_back((void *)&notify_msg);
					}
				}
			}
			break;

		case AT_QICLOSE:
			{
				gm_soc_notify_at_command_struct notify_msg;
				at_command_scan(p_str,false, "s", string_value);
				if (!GM_strcmp(string_value, "OK"))
				{
					notify_msg.result = KAL_TRUE;
					notify_msg.detail_cause = 0;
				}
				else
				{
					notify_msg.result = KAL_FALSE;
					notify_msg.detail_cause = -1;
				}
				notify_msg.access_id = (SocketIndexEnum)util_chr(one.data[0]);
				notify_msg.event_type = GM_SOC_CLOSE;
				if (s_at_command.result.sock_notify_call_back)
				{
					s_at_command.result.sock_notify_call_back((void *)&notify_msg);
				}
			}
			break;
			
		case AT_QIOPEN:
			//AT+QIOPEN=1,0,"TCP","54.222.189.233",8821
			//OK
			//at_command_scan(p_str,false, "s", string_value);
			break;
			
		case AT_QIOPENURC:
			{
				//+QIOPEN: 0,563
				//+QIOPEN: 4,0
				gm_soc_notify_at_command_struct notify_msg;
				at_command_scan(p_str,false, "sii", string_value, &value_u8, &value_32);
				notify_msg.access_id = (SocketIndexEnum)(value_u8);
				notify_msg.detail_cause = value_32;
				notify_msg.result = value_32 ? KAL_FALSE : KAL_TRUE;
				notify_msg.event_type = GM_SOC_CONNECT;
				if (s_at_command.result.sock_notify_call_back)
				{
					s_at_command.result.sock_notify_call_back((void *)&notify_msg);
				}
			}
			break;

		case AT_QISEND:
		case AT_QISENDEX:
			{
				static bool is_qisend = false;
				gm_soc_notify_at_command_struct notify_msg;

				if (*p_str == '>')
				{
					uart_write(GM_UART_AT, (u8 *)one.data2, one.data2_len);
					is_qisend = true;
					return GM_WILL_DELAY_EXEC;
				}
				else
				{
					if (is_qisend)
					{
						p_str += 4;
						notify_msg.msg_len = one.data2_len;
					}
					else
					{
						notify_msg.msg_len = (one.data_len-4)/2;
					}
					is_qisend = false;
					para_num = at_command_scan(p_str,true, "s", string_value);
					if (para_num)
					{
						if (!GM_strcmp(string_value, "SEND OK"))
						{
							notify_msg.access_id = (SocketIndexEnum)util_chr(one.data[0]);
							notify_msg.result = KAL_TRUE;
							notify_msg.detail_cause = 0;
							notify_msg.event_type = GM_SOC_WRITE;
							
						}
						else if (!GM_strcmp(p_str, "SEND FAIL") || !GM_strcmp(p_str, "ERROR"))
						{
							notify_msg.access_id = (SocketIndexEnum)util_chr(one.data[0]);
							notify_msg.result = KAL_FALSE;
							notify_msg.detail_cause = -1;
							notify_msg.event_type = GM_SOC_WRITE;
						}
						else
						{
							break;
						}
						
						if (s_at_command.result.sock_notify_call_back)
						{
							s_at_command.result.sock_notify_call_back((void *)&notify_msg);
						}
					}
					else
					{
					}
				}
			}
			break;
		case AT_QIRD:
			{
				/*
				+QIRD: 10
				<GM*ACK*0>

				OK
				*/
				u8 *recv_data = NULL;
				s32 result = 0;
				char *carriage_first = NULL;
				
				if (2 != at_command_scan(p_str,true, "si", string_value, &value_32))
				{
					break;
				}
				
				if (value_32 > 0)
				{
					carriage_first = GM_strstr(p_str, "\r\n");
					if (!carriage_first)break;
					recv_data = GM_MemoryAlloc(MAX_SOCKET_RECV_MSG_LEN+1);
					if (!recv_data)
					{
						carriage_first = NULL;
						break;
					}
					GM_memset(recv_data, 0, (MAX_SOCKET_RECV_MSG_LEN+1));
					if (carriage_first[2+value_32] != '\r' && carriage_first[3+value_32] != '\n')
					{
						//LOG(WARN, "clock(%d) at command sock recv data(%X,%X) error!",carriage_first[2+value_32], carriage_first[3+value_32]);
						result = -1;
					}
					
					GM_memcpy(recv_data, &carriage_first[2], value_32);
					if (s_at_command.result.sock_recv_call_back)
					{
						s_at_command.result.sock_recv_call_back(result, (SocketIndexEnum)util_chr(one.data[0]), value_32, recv_data);
					}
					
					GM_MemoryFree(recv_data);
					recv_data = NULL;
					carriage_first = NULL;
				}
				else
				{
					LOG(ERROR, "at command recv sock data null.");
				}
			}
			break;
		case AT_QURCCFG:
			break;
		case AT_SMSURC:
			{
				if (3 == at_command_scan(p_str,true, "ssi", string_value, string_value_1, &value_u8))
				{
					value_u8_1 = '0';//设置为PDU模式
					at_command_insert_one(AT_CMGF, COMMAND_WRITE, &value_u8_1,1, NULL, 0, 300);
					at_command_read_new_message(value_u8);
					at_command_deal_message(value_u8);
				}
			}
			break;
		case AT_CMGR:
			{
				/*
				AT+CMGR=5
				+CMGR: 0,,26
				0891683108703505F0040D91688174556187F500000210306124452306F2FA18AD5603

				OK
				*/
				char *p_sms = NULL;
				u8 sms_buff[300] = {0};
				p_sms = GM_strstr(p_str, "\n");
				if (!p_sms)break;
				p_str = p_sms+1;
				p_sms = GM_strstr(p_str, "\r");
				if (!p_sms)break;
				GM_memcpy(sms_buff, p_str, p_sms-p_str);
				at_command_parse_pdu_message(sms_buff, p_sms-p_str);
			}
			break;
		case AT_CMGD:
			//LOG(DEBUG, "at command delete sms index(%d)", util_chr(one.data[0]));
			break;
		case AT_CMGS:
			{
				static bool is_cmgs = false;
				bool is_send_ok = false;
				if (*p_str == '>')
				{
					u8 send_head = 0x1A;
					uart_write(GM_UART_AT, (u8 *)one.data2, one.data2_len);
					uart_write(GM_UART_AT, &send_head, 1);
					is_cmgs = true;
					return GM_WILL_DELAY_EXEC;
				}
				else
				{
					if (is_cmgs)
					{
						if (GM_strstr(p_str, "OK"))
						{
							is_send_ok = true;
						}
						s_at_command.result.send_sms_call_back((void *)&is_send_ok);
					}
					else
					{
					}
					is_cmgs = false;
				}
			}
			break;
		default:
			//LOG(ERROR, "default:%s",p_str);
			break;
	}

	//清除当前发送AT指令
	if (one.index == s_at_list[command_id].index)
	{
		at_command_commit_peek(true, true);
		s_at_command.fail_count = 0;
		s_at_command.clock_ms = 0;
	}

	LOG(DEBUG, "%s",s_at_command.recv_buff);
	p_str = NULL;

	return GM_SUCCESS;
}


static void at_command_parse_pdu_message(u8 *pdu_msg, u16 pdu_len)
{
	/*
	0891683108703505F0040D91688174556187F50000 02103061244523 06F2FA18AD5603
	//分包
	0891683108703505F0440D91688174556187F50000 02103051311123 A0 05 0003D50201 62
	B219AD66BBE172B0586D46ABD96EB81C2C269BD16AB61B2E078BD566B49AED86CBC162B2
	190D67BBE172B0986C46ABD96EB81C2C269BD16AB61B2E078BC966B49AED86CBC162B219
	AD66BBE172B0986C46ABD96EB81C2C269BD170B61B2E078BC96A34DB0D47CBD16C311C2D
	47B3D170B71A2D47C3D56EB3980D57B3E16EB41A0E57B3E16E
	
	0891683108703505F0440D91688174556187F50000 02103051312123 37 05 0003D50202 6A
	381CCE86C3E16A351CEE56ABE172B65A8D76C3E16CB5986C66CBE170375A0D87CBE56C33598D76C3E100
	*/
	//0891683108703505F0 04 0D91 688174556187F5 0000 02108061551223 07 737A985E9F8F00
	//0891683108100065F9 24 0DA0 014698194000F6 0000 02207141058423 07536A905A9D8E00
	//0891683108100065F9 24 0DA0 014698194000F6 0000 02204261228223 09 E41EC88C79A16101
	u16 idx = 0;
	u8 index = 0;
	u8 value_u8 = 0;
	u16 messageLen = 0;
	char src_message[300] = {0};
	u8 src_message_len = 0;
	static u8 recv_pack = 0;
	static gm_sms_new_msg_struct new_sms;
	u8 total_pack = 0;
	u8 cur_pack = 0;

	if (recv_pack == 0)
	{
		GM_memset(&new_sms, 0x00, sizeof(gm_sms_new_msg_struct));
	}
	//短信中心号码长度
	value_u8 = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
	idx += (2*value_u8+2);
	
	//短信源号码长度
	value_u8 = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
	if (value_u8 == 0x0D && pdu_msg[idx] == '9' && pdu_msg[idx+1] == '1')
	{
		idx+=4;//91
		value_u8 -= 2;
	}
	else
	{
		idx+=2;//81 or A0
	}
	if (value_u8 % 2)value_u8++;
	//8174556187F5
	for(index=0; index<value_u8; index+=2)
	{
		new_sms.asciiNum[index] = pdu_msg[idx+1];
		if (pdu_msg[idx] != 'F')
		{
			new_sms.asciiNum[index+1] = pdu_msg[idx];
		}
		idx+=2;
	}
	//LOG(INFO, "at_command_parse_pdu_message asciiNum(%s)", new_sms.asciiNum);
	idx+=2;//协议标识(TP-PID) 是普通GSM类型，点到点方式
	new_sms.smstype = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
	idx+=14;//时间02103051311123

	//消息长度 //A0
	src_message_len = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
	//分包
	//05(分包消息长度)00(表示长消息)03(剩余长度)D5(短信唯一标识)02(总包数)01(当前包数)
	if (5 == ((util_chr(pdu_msg[idx]) << 4) | util_chr(pdu_msg[idx+1]))
		&& 0 == ((util_chr(pdu_msg[idx+2]) << 4) | util_chr(pdu_msg[idx+3]))
		&& 3 == ((util_chr(pdu_msg[idx+4]) << 4) | util_chr(pdu_msg[idx+5])))
	{
		LOG(INFO, "at_command_parse_pdu_message isdepart");
		idx += 8;
		//当前包实际长度，总长度-分包消息总长度-分包消息内容长度
		new_sms.messageLen += (src_message_len-6);
		total_pack = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
		cur_pack = (util_chr(pdu_msg[idx++]) << 4) | util_chr(pdu_msg[idx++]);
		
		recv_pack++;
		messageLen = pdu_len-idx;
		GM_memcpy(src_message, &pdu_msg[idx], messageLen);
		if (!new_sms.content)
		{
			new_sms.content = GM_MemoryAlloc(160*total_pack*2+2);
			if (!new_sms.content)
			{
				GM_memset(&new_sms, 0, sizeof(gm_sms_new_msg_struct));
				recv_pack = 0;
				return;
			}
			GM_memset(new_sms.content, 0, (160*total_pack*2+2));
		}

	    //GM_DEFAULT_DCS 格式每包最多153字节(160-6)
		if (new_sms.smstype == GM_DEFAULT_DCS)
		{
			if (!util_pdu_7bit_decoding(&new_sms.content[154*(cur_pack-1)], src_message))
			{
				//转换失败
				GM_MemoryFree(new_sms.content);
				new_sms.content = NULL;
				GM_memset(&new_sms, 0, sizeof(gm_sms_new_msg_struct));
				recv_pack = 0;
				return;
			}
		}
		else
		{
			//非GM_DEFAULT_DCS 格式每包最多134字节(140-6)
			GM_memcpy(&new_sms.content[134*(cur_pack-1)*2], src_message, messageLen);
		}
		
		if (recv_pack != total_pack)
		{
			//返回等待下一包
			return;
		}
		
	}
	else//不需分包
	{
		new_sms.messageLen = src_message_len;
		messageLen = pdu_len-idx;
		GM_memcpy(src_message, &pdu_msg[idx], messageLen);
		if (new_sms.content)
		{
			GM_MemoryFree(new_sms.content);
			new_sms.content = NULL;
		}
		new_sms.content = GM_MemoryAlloc(src_message_len*2+2);
		if (!new_sms.content)
		{
			GM_memset(&new_sms, 0, sizeof(gm_sms_new_msg_struct));
			return;
		}
		GM_memset(new_sms.content, 0, (src_message_len*2+2));
		if (new_sms.smstype == GM_DEFAULT_DCS)
		{
			if (!util_pdu_7bit_decoding(new_sms.content, src_message))
			{
				//转换失败
				GM_MemoryFree(new_sms.content);
				new_sms.content = NULL;
				GM_memset(&new_sms, 0, sizeof(gm_sms_new_msg_struct));
				recv_pack = 0;
				return;
			}
		}
		else
		{
			GM_memcpy(new_sms.content, src_message, messageLen);
		}
	}

	//LOG(INFO, "newmsg new_sms.smstype(%02X) asciiNum(%s) messageLen(%d) content(%s)", new_sms.smstype, new_sms.asciiNum, new_sms.messageLen, new_sms.content);
	if (s_at_command.result.new_sms_call_back)
	{
		s_at_command.result.new_sms_call_back((void*)&new_sms);
	}
	
	if (new_sms.content)
	{
		GM_MemoryFree(new_sms.content);
		new_sms.content = NULL;
	}
	GM_memset(&new_sms, 0, sizeof(gm_sms_new_msg_struct));
	recv_pack = 0;
}


