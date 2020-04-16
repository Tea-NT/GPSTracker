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

#ifndef __AT_COMMAND_H_
#define __AT_COMMAND_H_
#include "error_code.h"
#include "gm_type.h"
#include "gm_gprs.h"
#include "socket.h"
#include "gsm.h"
#include "sms.h"
#include "command.h"

typedef void (*SocketRecvCallBack) (s32 result, SocketIndexEnum access_id, u16 recv_len, u8 *recv_data);

typedef enum
{
	AT_NONE,
	AT_RDY,
	AT_AT,
	AT_ATE,
	AT_CPIN,
	AT_CSQ,
	AT_CREG,
	AT_CGREG,
	AT_GMR,
	AT_CIMI,
	AT_QCCID,
	AT_QENG,
	AT_COPS,
	AT_ATA,
	AT_ATD,
	AT_ATH,
	//GPRS
	AT_QICSGP,
	AT_QIACT,
	AT_QIDEACT,
	AT_QIOPEN,
	AT_QICLOSE,
	AT_QISTATE,
	AT_QISEND,
	AT_QISENDEX,
	AT_QIRD,
	AT_QIDNSGIP,
	AT_QIURC,
	AT_QIOPENURC,
	AT_QCFG,
	AT_SMSURC,
	AT_CMGR,
	AT_CMGD,
	AT_CMGF,
	AT_CMGS,
	AT_QURCCFG,
	AT_USER,
	AT_AUTO_ANSWER,

	AT_CMD_MAX,
}AtCommandListEnum;


typedef enum
{
	COMMAND_TEST,
	COMMAND_WRITE,
	COMMAND_READ,
	COMMAND_EXECUTION,

	AT_MAX,
}AtCommandTypeEnum;

typedef struct
{
	char cmd_str[30];
	CommandReceiveFromEnum from;
	gm_sms_new_msg_struct sms_msg;
}AtCommandThird;



/**
 * Function:   检查SIM卡状态
 * Description:检查SIM卡状态
 * Input:	   无
 * Output:	   无
 * Return:	   true 有SIM卡，false 无SIM卡
 * Others:	   
 */
bool at_command_sim_valid(PsFuncPtr handler_func_ptr);


u8 at_command_csq_value(void);


bool at_command_creg_register(void);

bool at_command_cgreg_register(void);

GM_ERRCODE at_command_get_imsi(PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_get_iccid(PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_get_lbs(LbsCallBackFunc handler_func_ptr);

GM_ERRCODE at_command_set_apn(ST_GprsConfig apnConfig, PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_attach_network(PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_unattach_network(PsFuncPtr handler_func_ptr);


GM_ERRCODE at_command_get_local_ip(PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_get_host_ip(u8 *host_name, PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_open_connect(SocketType *socket, PsFuncPtr handler_func_ptr, SocketRecvCallBack recv_func_ptr);

GM_ERRCODE at_command_close_connect(SocketIndexEnum index);

GM_ERRCODE at_command_socket_send(SocketIndexEnum access_id, u8 *data, u16 data_len);

GM_ERRCODE at_command_sock_recvive(SocketIndexEnum access_id);

GM_ERRCODE at_command_read_new_message(u8 message_index);

GM_ERRCODE at_command_deal_message(u8 message_index);

GM_ERRCODE at_command_sms_register(PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_sms_send(char*number, char*content, u16 contentlen, ENUM_SMSAL_DCS doc_type, PsFuncPtr handler_func_ptr);

GM_ERRCODE at_command_send_by_third(AtCommandThird *third_cmd, AtCommandTypeEnum type, u8 *data, u16 len);



/**
 * Function:   创建at_command模块
 * Description:创建at_command模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   使用前必须调用,否则调用其它接口返回失败错误码
 */
GM_ERRCODE at_command_create(void);

/**
 * Function:   销毁at_command模块
 * Description:销毁at_command模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE at_command_destroy(void);


/**
 * Function:   at_command模块定时处理入口
 * Description:at_command模块定时处理入口
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   1秒钟调用1次
 */
GM_ERRCODE at_command_timer_proc(void);



/**
 * Function:   AT指令入口函数
 * Description:AT指令入口函数
 * Input:      p_cmd[in]: 输入数据指针
 *             src_len[in]: 输入数据长度
 * Output:	   p_rsp[out]:输出数据指针
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:
 */
GM_ERRCODE at_command_insert_one(AtCommandListEnum index, AtCommandTypeEnum type, u8 *data, u16 len, u8 *data2, u16 data2_len, u32 delay_time);


/**
 * Function:   AT指令入口函数
 * Description:AT指令入口函数
 * Input:      p_cmd[in]: 输入数据指针
 *             src_len[in]: 输入数据长度
 * Output:	   p_rsp[out]:输出数据指针
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:
 */
GM_ERRCODE at_command_on_receive_data(char* p_cmd, u16 cmd_len, char* p_rsp);


GM_ERRCODE at_command_set_nwscanmode(u8 mode);

void at_command_recv(void);

AtCommandListEnum find_at_command_by_string(char *string);



#endif /*__AT_COMMAND_H_*/

