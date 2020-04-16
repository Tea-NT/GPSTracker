/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        recorder.h
 * Author:           李耀轩       
 * Version:          1.0
 * Date:             2019-09-20
 * Description:      
 * Others:      
 * Function List:    

 * History: 
    1. Date:         2019-09-20
       Author:       李耀轩
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#ifndef __RECORDER_H_
#define __RECORDER_H_

#include "gm_type.h"
#include "error_code.h"

typedef enum
{
	RES_SUCCESS = 0x00, //开启成功 or 关闭成功
	RES_START_RECD = 0x01,  //已开始录音
	RES_STOP_FAIL = 0x01,  //关闭录音失败
	RES_START_FAIL = 0x03, //开启录音失败
	RES_IDLE = 0x03, //终端未录音

	RES_END
}RecoderResStateEnum;


typedef struct
{
	u8 *data;
	u8 time[6];
	u8 total_pack;
	u8 cur_pack;
	u16 pack_len;
	u16 uploadtimeout;
}RecoderSendRegStruct;



/**
 * Function:   获取当前录音状态
 * Description:获取当前录音状态
 * Input:	   无
 * Output:	   无
 * Return:	   当前录音状态
 * Others:	   
 */
u8 get_recorder_ctrl_state(void);


/**
 * Function:   发送录音文件响应入口
 * Description:发送录音文件响应入口
 * Input:	   响应信息，包括响应包数及录音文件时间
 * Output:	   无
 * Return:	   无
 * Others:	   
 */
void recorder_file_send_ack(u8 *ack_msg);



/**
 * Function:   获取接收录音命令响应状态
 * Description:获取接收录音命令响应状态
 * Input:	   无
 * Output:	   无
 * Return:	   RecoderResStateEnum
 * Others:	   
 */
RecoderResStateEnum get_recorder_response_state(void);


/**
 * Function:   关闭录音命令
 * Description:关闭录音命令
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE stop_record(bool upload_service);



/**
 * Function:   开启录音命令
 * Description:开启录音命令
 * Input:	   upload_service 是否上传服务器
 *			   true 上传 false 不上传
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE start_record(bool upload_service);


 /**
  * Function:	创建recorder模块
  * Description:创建recorder模块
  * Input:		upload_service 是否上传服务器
  *             true 上传 false 不上传
  * Output: 	无
  * Return: 	GM_SUCCESS——成功；其它错误码——失败
  * Others: 	使用前必须调用
  */
 GM_ERRCODE recorder_create(bool upload_service);
 
 
 /**
  * Function:	销毁recorder模块
  * Description:销毁recorder模块
  * Input:		无
  * Output: 	无
  * Return: 	GM_SUCCESS——成功；其它错误码——失败
  * Others: 	
  */
 GM_ERRCODE recorder_destroy(void);
 
 
 /**
  * Function:	recorder模块定时处理入口
  * Description:recorder模块定时处理入口
  * Input:		无
  * Output: 	无
  * Return: 	GM_SUCCESS——成功；其它错误码——失败
  * Others: 	
  */
 GM_ERRCODE recorder_timer_proc(void);


 /**
  * Function:	删除保存的录音文件
  * Description:删除保存的录音文件
  * Input:		无
  * Output: 	无
  * Return: 	无
  * Others: 	
  */
 void delete_recorder_file(void);

 #endif /*__RECODER_H_*/

