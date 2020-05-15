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
 
#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__
	
#include "gm_type.h"
#include "error_code.h"
#include "system_state.h"
#include "config_service.h"
	
typedef enum
{
	PERIPHERAL_TYPE_NONE = 0,

	//重优油杆传感器YW-201
	PERIPHERAL_TYPE_YW201,
	//重优载重传感器ZZH-201
	PERIPHERAL_TYPE_ZZH201,
	//常润油杆传感器CR6061
	PERIPHERAL_TYPE_CR6061,

	PERIPHERAL_TYPE_MAX
}GM_PERIPHERAL_TYPE;


/**
 * Function:   1.创建peripheral模块
 * Description:创建peripheral模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   使用前必须调用,否则调用其它接口返回失败错误码
 */
GM_ERRCODE peripheral_create(void);

/**
 * Function:   2.销毁peripheral模块
 * Description:销毁peripheral模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:		 
 */
GM_ERRCODE peripheral_destroy(void);

/**
 * Function:   3.peripheral模块定时处理入口
 * Description:peripheral模块定时处理入口
 * Input:		无
 * Output:		无
 * Return:		GM_SUCCESS——成功；其它错误码——失败
 * Others:		1秒调用1次
 */
GM_ERRCODE peripheral_timer_proc(void);

/**
 * Function:   4.peripheral模块收到串口数据处理函数
 * Description:peripheral模块收到串口数据处理函数
 * Input:		无
 * Output:		无
 * Return:		GM_SUCCESS——成功；其它错误码——失败
 * Others:		1秒调用1次
 */
GM_ERRCODE peripheral_uart_on_receive(char* p_data,U16 data_len);

#endif /*__PERIPHERAL_H__*/
	

