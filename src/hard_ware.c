/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        hard_ware.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-03-01
 * Description:      简单硬件（开关）读写操作封装,包括读取ADC的数值
 * Others:      
 * Function List:    
    1. 创建hard_ware模块
    2. 销毁hard_ware模块
    3. 定时处理入口
    4. 获取ACC输入状态
    5. 设置GPS LED状态
    6. 设置GSM LED状态
    7. 设置断油电IO状态
    8. 获取供电电源电压值
    9. 设置内置电池充电状态（GS05）
 * History: 
    1. Date:         2019-03-01
       Author:       王志华
       Modification: 创建初始版本
    2. Date: 		 
	   Author:		 
	   Modification: 

 */

#include <gm_time.h>
#include <gm_timer.h>
#include <gm_gpio.h>
#include <gm_adc.h>
#include <gm_power.h>
#include <gm_memory.h>
#include <gm_system.h>
#include <gm_stdlib.h>
#include "g_sensor.h"
#include "hard_ware.h"
#include "gps.h"
#include "uart.h"
#include "system_state.h"
#include "log_service.h"
#include "config_service.h"
#include "utility.h"
#include "gps_service.h"
#include "applied_math.h"
#include "auto_test.h"
#include "gsm.h"
#include "gps_service.h"
#include <stdlib.h>
#include <math.h>
#include "circular_queue.h"
#include "peripheral.h"

//输入:电池是否充满
#define GM_GPS_CHARGE_LEVLE_GPIO_PIN GM_GPIO4

//输出:充电控制
#define GM_GPS_CHARGE_CTROL_GPIO_PIN GM_GPIO5

//输出:GPS开关
#define GM_GPS_POWER_CTROL_GPIO_PIN  GM_GPIO19

//输出:断油电
#define GM_RELAY_CTROL_GPIO_PIN  GM_GPIO25

//输出:看门狗
#define GM_WDT_CTROL_GPIO_PIN  GM_GPIO28

//输入:ACC高低
#define GM_ACC_LEVEL_GPIO_PIN  GM_GPIO20

//输出:POWER LED控制
#define GM_POWER_LED_GPIO_PIN GM_GPIO2

//输出:EC20 POWER_KEY
#define GM_EC20_POWER_KEY_GPIO_PIN GM_GPIO6

//输出:EC20 RESET
#define GM_EC20_RESET_GPIO_PIN GM_GPIO7

//输入:检测GS03是否为高电压版本
#define GM_POWER_90V_GPIO_PIN GM_GPIO14

//输入:电源电压,ADC,通道ID
#define GM_POWER_VOLTAGE_ADC_CHANNEL 4

//输入:VCDT,ADC,通道ID
#define GM_VCDT_VOLTAGE_ADC_CHANNEL 2

//输入:温度检测,ADC,通道ID
#define GM_BATON_ADC_CHANNEL 3

#define VOLTAGE_BUFF_LEN 50

//计算电压平均值的次数
#define GM_POWER_VOLTAGE_CALC_NUM 10

#define GM_HIGH_VOLTAGE_ALARM_THRESHHOLD_90V 96

#define GM_HIGH_VOLTAGE_ALARM_THRESHHOLD_36V 36

#define GM_BATTERY_HIGHEST_VOLTAGE 7.0

#define GM_TEMP_MAX_CNT 146


//1200mA时 W1用电池电量曲线 
float s_1_2ah_battary_level[21] = {4.09f,4.02f,3.95f,3.90f,3.85f,3.80f,3.75f,3.71f,3.67f,3.64f,3.61f,3.60f,3.58f,3.56f,3.55f,3.53f,3.50f,3.48f,3.45f,3.40f,0.0f};

//3000mA时 W5用电池电量曲线
float s_3ah_battary_level[21] = {4.06f,4.02f,4.01f,3.99f,3.97f,3.94f,3.91f,3.88f,3.86f,3.84f,3.82f,3.80f,3.78f,3.75f,3.69f,3.63f,3.58f,3.54f,3.48f,3.43f,0.0f};

//6000mA时 W7用电池电量曲线
float s_6ah_battary_level[21] = {4.07f,4.01f,3.95f,3.89f,3.84f,3.78f,3.73f,3.69f,3.65f,3.62f,3.59f,3.58f,3.56f,3.55f,3.53f,3.51f,3.48f,3.46f,3.43f,3.40f,0.0f};

//12000mA时 W12用电池电量曲线 10600
float s_12ah_battary_level[21] = {3.90f,3.863f,3.851f,3.847f,3.843f,3.838f,3.829f,3.809f,3.765f,3.714f,3.695f,3.688f,3.679f,3.666f,3.651f,3.630f,3.598f,3.547f,3.472f,3.403f,0.0f};

//18000mA时 W18用电池电量曲线 15545
float s_18ah_battary_level[21] = {3.864f,3.827f,3.814f,3.809f,3.806f,3.802f,3.795f,3.780f,3.741f,3.686f,3.658f,3.652f,3.645f,3.635f,3.621f,3.604f,3.578f,3.537f,3.475f,3.40f,0.0f};


typedef struct
{
	float adc_value;
	float adc_diff;
	s8 temperature;
}HardWareTemp;


static HardWareTemp s_temperature[TEMP_MAX][GM_TEMP_MAX_CNT] = 
{
	{//MF52                  
	{952.85,   2.06 , -40},
	{948.89,   2.06  , -39},
	{944.77,   2.13  , -38},
	{940.50,   2.21  , -37},
	{936.07,   2.30  , -36},
	{931.48,   2.38  , -35},
	{926.72,   2.47  , -34},
	{921.79,   2.55  , -33},
	{916.69,   2.64  , -32},
	{911.41,   2.72  , -31},
	{905.96,   2.82  , -30},
	{900.33,   2.91  , -29},
	{894.51,   3.00  , -28},
	{888.51,   3.09  , -27},
	{882.32,   3.19  , -26},
	{875.95,   3.29  , -25},
	{869.38,   3.37  , -24},
	{862.64,   3.47  , -23},
	{855.70,   3.57  , -22},
	{848.57,   3.66  , -21},
	{841.26,   3.75  , -20},
	{833.76,   3.84  , -19},
	{826.08,   3.93  , -18},
	{818.22,   4.02  , -17},
	{810.18,   4.10  , -16},
	{801.97,   4.19  , -15},
	{793.59,   4.28  , -14},
	{785.03,   4.35  , -13},
	{776.32,   4.44  , -12},
	{767.45,   4.51  , -11},
	{758.43,   4.58  , -10},
	{749.26,   4.65  , -9 },
	{739.96,   4.72  , -8 },
	{730.52,   4.78  , -7 },
	{720.96,   4.85  , -6 },
	{711.27,   4.89  , -5 },
	{701.48,   4.94  , -4 },
	{691.60,   5.00  , -3 },
	{681.61,   5.04  , -2 },
	{671.54,   5.04  , -1 },
	{662.05,   5.43  , 0  },
	{651.19,   5.43  , 1  },
	{640.92,   5.15  , 2  },
	{630.62,   5.19  , 3  },
	{620.25,   5.19  , 4  },
	{609.88,   5.20  , 5  },
	{599.47,   5.21  , 6  },
	{589.06,   5.21  , 7  },
	{578.64,   5.21  , 8  },
	{568.23,   5.20  , 9  },
	{558.23,   5.38  , 10 },
	{547.47,   5.38  , 11 },
	{537.14,   5.17  , 12 },
	{526.85,   5.14  , 13 },
	{516.60,   5.13  , 14 },
	{506.40,   5.10  , 15 },
	{496.30,   5.05  , 16 },
	{486.24,   5.03  , 17 },
	{476.27,   4.99  , 18 },
	{466.38,   4.94  , 19 },
	{456.60,   4.89  , 20 },
	{446.90,   4.85  , 21 },
	{437.29,   4.80  , 22 },
	{427.82,   4.74  , 23 },
	{418.44,   4.69  , 24 },
	{409.20,   4.62  , 25 },
	{400.06,   4.57  , 26 },
	{391.05,   4.51  , 27 },
	{382.18,   4.44  , 28 },
	{373.42,   4.38  , 29 },
	{364.81,   4.31  , 30 },
	{356.35,   4.23  , 31 },
	{348.02,   4.17  , 32 },
	{339.82,   4.10  , 33 },
	{331.75,   4.04  , 34 },
	{323.85,   3.95  , 35 },
	{316.09,   3.88  , 36 },
	{308.48,   3.80  , 37 },
	{301.02,   3.73  , 38 },
	{293.67,   3.67  , 39 },
	{286.50,   3.59  , 40 },
	{279.47,   3.51  , 41 },
	{272.60,   3.44  , 42 },
	{265.86,   3.37  , 43 },
	{259.23,   3.32  , 44 },
	{252.78,   3.23  , 45 },
	{246.47,   3.16  , 46 },
	{240.29,   3.09  , 47 },
	{234.26,   3.02  , 48 },
	{228.38,   2.97  , 49 },
	{222.45,   2.97  , 50 },
	{216.98,   2.75  , 51 },
	{211.48,   2.75  , 52 },
	{206.12,   2.68  , 53 },
	{200.87,   2.63  , 54 },
	{195.78,   2.55  , 55 },
	{190.80,   2.49  , 56 },
	{185.94,   2.43  , 57 },
	{181.21,   2.36  , 58 },
	{176.57,   2.32  , 59 },
	{172.06,   2.26  , 60 },
	{167.70,   2.18  , 61 },
	{163.38,   2.16  , 62 },
	{159.22,   2.08  , 63 },
	{155.17,   2.03  , 64 },
	{151.22,   1.97  , 65 },
	{147.34,   1.94  , 66 },
	{143.58,   1.88  , 67 },
	{139.94,   1.82  , 68 },
	{136.36,   1.79  , 69 },
	{132.92,   1.72  , 70 },
	{129.55,   1.68  , 71 },
	{126.26,   1.65  , 72 },
	{123.05,   1.61  , 73 },
	{119.93,   1.56  , 74 },
	{116.89,   1.52  , 75 },
	{113.94,   1.48  , 76 },
	{111.07,   1.44  , 77 },
	{108.30,   1.39  , 78 },
	{105.57,   1.37  , 79 },
	{102.93,   1.32  , 80 },
	{100.38,   1.28  , 81 },
	{97.88 ,   1.25  , 82 },
	{95.42 ,   1.23  , 83 },
	{93.06 ,   1.18  , 84 },
	{90.80 ,   1.14  , 85 },
	{88.53 ,   1.14  , 86 },
	{86.36 ,   1.09  , 87 },
	{84.24 ,   1.06  , 88 },
	{82.17 ,   1.04  , 89 },
	{80.14 ,   1.02  , 90 },
	{78.23 ,   0.97  , 91 },
	{76.30 ,   0.97  , 92 },
	{74.49 ,   0.91  , 93 },
	{72.67 ,   0.91  , 94 },
	{70.90 ,   0.88  , 95 },
	{69.24 ,   0.83  , 96 },
	{67.58 ,   0.83  , 97 },
	{65.97 ,   0.81  , 98 },
	{64.42 ,   0.77  , 99 },
	{62.92 ,   0.76  , 100},
	{61.41 ,   0.76  , 101},
	{59.96 ,   0.72  , 102},
	{58.57 ,   0.70  , 103},
	{57.24 ,   0.67  , 104},
	{55.90 ,   0.67  , 105},
	},
};


typedef struct
{
	bool inited;
	//设备型号
	ConfigDeviceTypeEnum dev_type;

	//电源电压
	float power_voltage;

	//外部电池电量百分比（根据供电电压计算）
	float extern_battery_percent;

	//电池电压
	float battery_voltage;
	u8 battery_voltage_percent;
	StateRecord battery_voltage_percent_record;

	bool battery_is_charging;

	bool battery_is_full;

	//电池充电电流
	float battery_charge_current;

	S32 battery_charge_time;
	
	U16 timer_ms;

	StateRecord power_off_alarm_record;
	
	StateRecord low_voltage_alarm_record;

	StateRecord high_voltage_alarm_record;

	StateRecord power_range_record;
	
	StateRecord acc_record;
	bool acc_line_is_valid;
	U32 acc_low_but_run_seconds;

	float baton_adc;
	StateRecord baton_record;
	CircularQueue baton_adc_queue;
	StateRecord sos_alarm_record;
	StateRecord temperature_record;
	s8 baton_temperature;

	StateRecord battery_is_full_record;

	bool gps_led_is_on;
	bool gsm_led_is_on;
	bool power_led_is_on;

	float vcdt_voltage;
	CircularQueue battery_voltage_queue;
	CircularQueue vcdt_voltage_queue;
	
}HardWare, *PHardWare;

static HardWare s_hard_ware;

static GM_ERRCODE init_gpio(void);

static void hard_ware_read_voltage(void);

static U8 hard_ware_calc_extern_voltage_grade(float voltage);

static float hard_ware_calc_extern_battery_percent(float voltage, U8 voltage_grade);

static void hard_ware_check_power_range(void);

static void hard_ware_check_high_voltage_alarm(void);

static void hard_ware_check_power_off_alarm(void);

static void hard_ware_check_battery(void);

static void hard_ware_check_acc(void);

static void hard_ware_set_auto_defence(void);

static void hard_ware_reboot_atonce(void);

static bool hard_ware_has_acc_line(const ConfigDeviceTypeEnum dev_type);

static void hard_ware_check_vcdt_voltage(void);

static float hard_ware_smooth_voltage_avg(CircularQueue *voltage_que, float last_voltage);

static bool hard_ware_is_charge_voltage_ready(void);

static float hard_ware_get_charge_current(bool charging);

static void hard_ware_check_baton(void);

static void hard_ware_check_sos_alarm(void);

static void hard_ware_check_baton_temperature(void);

static float hard_ware_find_adc_by_temperature(s8 temperature);



/**
 * Function:   创建hard_ware模块
 * Description:创建hard_ware模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   管脚(参看电路图及管脚功能表)
 */
GM_ERRCODE hard_ware_create(void)
{
	GM_ERRCODE ret = GM_SUCCESS;
	u8 key_feature;
	bool is_90v_power;

	if (s_hard_ware.inited)
	{
		return GM_SUCCESS;
	}

	ret = init_gpio();
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to init GPIO!");
	}
	
	s_hard_ware.inited = false;
	s_hard_ware.dev_type = DEVICE_NONE;
	s_hard_ware.power_voltage = 0.0;
	s_hard_ware.extern_battery_percent = 0.0;
	s_hard_ware.battery_voltage_percent = 0;
	s_hard_ware.battery_is_charging = false;
	s_hard_ware.battery_is_full = false;
	s_hard_ware.battery_charge_current = 0;
	s_hard_ware.battery_charge_time = -1;
	s_hard_ware.timer_ms = 0;
	s_hard_ware.gps_led_is_on = false;
	s_hard_ware.gsm_led_is_on = false;
	s_hard_ware.power_led_is_on = false;

	LOG(DEBUG, "hard_ware_create");

	s_hard_ware.power_off_alarm_record.state = system_state_get_power_off_alarm();
	s_hard_ware.power_off_alarm_record.true_state_hold_seconds = 0;
	s_hard_ware.power_off_alarm_record.false_state_hold_seconds = 0;

	s_hard_ware.low_voltage_alarm_record.state = system_state_get_battery_low_voltage_alarm();
	s_hard_ware.low_voltage_alarm_record.true_state_hold_seconds = 0;
	s_hard_ware.low_voltage_alarm_record.false_state_hold_seconds = 0;

	s_hard_ware.high_voltage_alarm_record.state = system_state_get_high_voltage_alarm();
	s_hard_ware.high_voltage_alarm_record.true_state_hold_seconds = 0;
	s_hard_ware.high_voltage_alarm_record.false_state_hold_seconds = 0;

	config_service_get(CFG_IS_90V_POWER, TYPE_BOOL, &is_90v_power, sizeof(bool));
	s_hard_ware.power_range_record.state = is_90v_power ? false:true;
	s_hard_ware.power_range_record.true_state_hold_seconds = 0;
	s_hard_ware.power_range_record.false_state_hold_seconds = 0;

	s_hard_ware.acc_record.state = false;
	s_hard_ware.acc_record.true_state_hold_seconds = 0;
	s_hard_ware.acc_record.false_state_hold_seconds = 0;
	s_hard_ware.acc_line_is_valid = true;
	s_hard_ware.acc_low_but_run_seconds = 0;

	s_hard_ware.baton_record.state = false;
	s_hard_ware.baton_record.true_state_hold_seconds = 0;
	s_hard_ware.baton_record.false_state_hold_seconds = 0;
	s_hard_ware.baton_temperature = 0;

	s_hard_ware.temperature_record.state = false;
	s_hard_ware.temperature_record.true_state_hold_seconds = 0;
	s_hard_ware.temperature_record.false_state_hold_seconds = 0;

	config_service_get(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
	s_hard_ware.sos_alarm_record.state = (key_feature == KEY_SOS) ? false : true;
	s_hard_ware.sos_alarm_record.true_state_hold_seconds = 0;
	s_hard_ware.sos_alarm_record.false_state_hold_seconds = 0;

	s_hard_ware.battery_is_full_record.state = true;
	s_hard_ware.battery_is_full_record.true_state_hold_seconds = 0;
	s_hard_ware.battery_is_full_record.false_state_hold_seconds = 0;

	s_hard_ware.inited = true;
	s_hard_ware.vcdt_voltage = 0.0;
	circular_queue_create(&s_hard_ware.battery_voltage_queue, VOLTAGE_BUFF_LEN, GM_QUEUE_TYPE_FLOAT);
	circular_queue_create(&s_hard_ware.vcdt_voltage_queue, VOLTAGE_BUFF_LEN, GM_QUEUE_TYPE_FLOAT);
	circular_queue_create(&s_hard_ware.baton_adc_queue, VOLTAGE_BUFF_LEN, GM_QUEUE_TYPE_INT);
	
	return GM_SUCCESS;
}

static GM_ERRCODE init_gpio(void)
{
	S32 ret = GM_SUCCESS;
	ret = GM_GpioInit(GM_GPS_CHARGE_LEVLE_GPIO_PIN, PINDIRECTION_IN, PINLEVEL_LOW, PINPULLSEL_DISABLE);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_GPS_CHARGE_LEVLE_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_GPS_CHARGE_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_DISABLE);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_GPS_CHARGE_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_GPS_POWER_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_DISABLE);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_GPS_POWER_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_RELAY_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_RELAY_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_WDT_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_DISABLE);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_WDT_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_ACC_LEVEL_GPIO_PIN, PINDIRECTION_IN, PINLEVEL_HIGH, PINPULLSEL_PULLUP);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_ACC_LEVEL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_POWER_LED_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);
	if (GM_SUCCESS != ret)
	{
		LOG(FATAL,"Failed to init GM_POWER_LED_GPIO_PIN!");
	   	return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_EC20_POWER_KEY_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_EC20_POWER_KEY_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_EC20_RESET_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLUP);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_EC20_RESET_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	ret = GM_GpioInit(GM_POWER_90V_GPIO_PIN, PINDIRECTION_IN, PINLEVEL_LOW, PINPULLSEL_PULLUP);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_POWER_90V_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	return GM_SUCCESS;
}


GM_ERRCODE reinit_relay_gpio(void)
{
	S32 ret = GM_SUCCESS;
	
	ret = GM_GpioInit(GM_RELAY_CTROL_GPIO_PIN, PINDIRECTION_OUT, PINLEVEL_LOW, PINPULLSEL_PULLDOWN);
	if (GM_SUCCESS != ret)
	{
	   LOG(FATAL,"Failed to init GM_RELAY_CTROL_GPIO_PIN!");
	   return GM_HARD_WARE_ERROR;
	}

	return (GM_ERRCODE)ret;
}


/**
 * Function:   销毁hard_ware模块
 * Description:销毁hard_ware模块
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_destroy(void)
{
	s_hard_ware.inited = false;
	circular_queue_destroy(&s_hard_ware.battery_voltage_queue, GM_QUEUE_TYPE_FLOAT);
	circular_queue_destroy(&s_hard_ware.vcdt_voltage_queue, GM_QUEUE_TYPE_FLOAT);
	circular_queue_destroy(&s_hard_ware.baton_adc_queue, GM_QUEUE_TYPE_INT);
	return GM_SUCCESS;
}

GM_ERRCODE hard_ware_timer_proc(void)
{
	U8 extern_battery_voltage_grade = 0;
    u16 device_type;

	
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (GM_SYSTEM_STATE_WORK == system_state_get_work_state())
	{
		s_hard_ware.timer_ms += TIM_GEN_10MS;
	}
	else
	{
		s_hard_ware.timer_ms += TIM_GEN_1SECOND;
	}
    
	//100ms读一次数据
	if (s_hard_ware.timer_ms % TIM_GEN_100MS == 0)
	{
		hard_ware_read_voltage();
	}

	//1秒以内不做其它处理
	if (s_hard_ware.timer_ms < TIM_GEN_1SECOND)
	{
		return GM_SUCCESS;
	}

	config_service_get(CFG_DEVICETYPE, TYPE_SHORT, &device_type , sizeof(device_type));
	
    s_hard_ware.dev_type = (ConfigDeviceTypeEnum)device_type;

	if (hard_ware_has_acc_line(s_hard_ware.dev_type) || config_service_is_test_mode())
	{
		hard_ware_check_acc();
	}
	else
	{
		system_state_set_acc_is_line_mode(false);
	}

	//后续所有设备都加上温度检测 //add by lyx 20200326
	hard_ware_check_baton();

	//设备类型未知的时候也要能充电
	if (hard_ware_has_battery() || DEVICE_NONE == s_hard_ware.dev_type)
	{
		hard_ware_check_vcdt_voltage();
		hard_ware_check_battery();
	}

	if (!hard_ware_is_device_w())
	{
		hard_ware_check_power_range();
		
		//检查电源电压过高报警
		hard_ware_check_high_voltage_alarm();

		//检查断电报警
		hard_ware_check_power_off_alarm();

		//计算外部电压等级并保存到系统状态中
		extern_battery_voltage_grade = hard_ware_calc_extern_voltage_grade(s_hard_ware.power_voltage);
		system_state_set_extern_battery_voltage_grade(extern_battery_voltage_grade);

		//计算外部电池百分比（根据电源电压估算）
		s_hard_ware.extern_battery_percent = hard_ware_calc_extern_battery_percent(s_hard_ware.power_voltage, extern_battery_voltage_grade);
	}

	//毫秒计时器清零
	s_hard_ware.timer_ms = 0;
	return GM_SUCCESS;
}

static ConfigDeviceTypeEnum four_line_devtypes[] = 
{
    DEVICE_GS03A,
	DEVICE_AS03A,
    DEVICE_GS07A,
    DEVICE_AS07A,
    DEVICE_GS03H,
    DEVICE_GS05A, 
    DEVICE_GS05H,
    DEVICE_GM06E,
    DEVICE_GS06,
    DEVICE_GS08,
    DEVICE_GS03D,
    DEVICE_GS05D,
};
	
static bool hard_ware_has_acc_line(const ConfigDeviceTypeEnum dev_type)
{
	U8 index = 0;
	for (index = 0; index < sizeof(four_line_devtypes)/sizeof(ConfigDeviceTypeEnum); ++index)
	{
		if (dev_type == four_line_devtypes[index])
		{
			return true;
		}
	}

	return false;
}


bool hard_ware_has_battery(void)
{
	switch(s_hard_ware.dev_type)
 	{
 		case DEVICE_NONE:
 		case DEVICE_GS03A: //4线单sensor 有电池
	    case DEVICE_AS03A: //4线单sensor 有电池
	    case DEVICE_GS03F: //2线单sensor 有电池
	    case DEVICE_AS03F: //2线单sensor 有电池
	    case DEVICE_GS07B: //2线单sensor 有电池90V
	    case DEVICE_AS07B: //2线单sensor 有电池90V
	    case DEVICE_GS03H: //4线双sensor,同03A
	    case DEVICE_GS05A: //4线单sensor 90V,有电池
	    case DEVICE_GS05F: //2线单sensor 90V,有电池
	    case DEVICE_GS05H: //4线双sensor 90V,有电池
	    case DEVICE_GM06E: //同GS03A,客户定制
		case DEVICE_GS06:
	    case DEVICE_W1://无线，录音，无WIFI
	    case DEVICE_W3://无线，录音，WIFI
	    case DEVICE_W7://无线，录音，WIFI
	    case DEVICE_W12://无线，录音，WIFI
	    case DEVICE_W18://无线，录音，WIFI
	    case DEVICE_B5:
	    case DEVICE_B7:
	    case DEVICE_GS08:
		case DEVICE_GS03D:
		case DEVICE_GS05D:	
			return true;

		default:
			return false;
 	}
}

bool hard_ware_has_sos(void)
{
	bool has_sos;
	
	switch(s_hard_ware.dev_type)
	{
		case DEVICE_GS06:
		case DEVICE_GS08:
			has_sos = true;
			break;
		default:
			has_sos = false;
			break;
	}
	
	return has_sos;
}


bool hard_ware_has_vcdt_voltage(void)
{
	switch(s_hard_ware.dev_type)
 	{
 		case DEVICE_W1:
	    case DEVICE_W3:
	    case DEVICE_W7:
	    case DEVICE_W12:
	    case DEVICE_W18:
 		case DEVICE_B5:
 		case DEVICE_B7:
 		case DEVICE_GS06:
 		case DEVICE_GS08:
			return true;

		default:
			return false;
 	}
}




bool hard_ware_is_device_w(void)
{
	switch(s_hard_ware.dev_type)
 	{
 		case DEVICE_W1:
		case DEVICE_W3:
		case DEVICE_W7:
		case DEVICE_W12:
		case DEVICE_W18:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_is_at_command(void)
{
	u16 dev_type;
	
	config_service_get(CFG_DEVICETYPE, TYPE_SHORT, &dev_type, sizeof(U16));
	switch(dev_type)
	{
		case DEVICE_GS08:
			return true;

		default:
			return false;
	}
}


bool hard_ware_is_device_05(void)
{
	switch(s_hard_ware.dev_type)
 	{
 		case DEVICE_GS05A:
		case DEVICE_GS05B:
		case DEVICE_GS05F:
		case DEVICE_GS05I:
		case DEVICE_GS05H:
		case DEVICE_GS05C:
		case DEVICE_GS05D:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_is_device_obd(void)
{
	switch(s_hard_ware.dev_type)
 	{
 		case DEVICE_B1:
 		case DEVICE_B3:
 		case DEVICE_B5:
 		case DEVICE_B7:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_device_has_wifi(void)
{
	switch(s_hard_ware.dev_type)
 	{
		case DEVICE_W3:
		case DEVICE_W7:
		case DEVICE_W12:
		case DEVICE_W18:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_device_has_recorder(void)
{
	switch(s_hard_ware.dev_type)
 	{
		case DEVICE_W1:
		case DEVICE_W3:
		case DEVICE_W7:
		case DEVICE_W12:
		case DEVICE_W18:
		case DEVICE_B3:
		case DEVICE_B7:
		case DEVICE_GS06:
		case DEVICE_GS08:
			return true;

		default:
			return false;
 	}
}




bool hard_ware_device_has_relay(void)
{
	switch(s_hard_ware.dev_type)
 	{
		case DEVICE_GS03A:
		case DEVICE_AS03A:
		case DEVICE_GS03H:
		case DEVICE_GS05A:
		case DEVICE_GS05H:
		case DEVICE_GM06E:
		case DEVICE_GS07A:
		case DEVICE_AS07A:
		case DEVICE_GS06:
		case DEVICE_GS08:
		case DEVICE_GS03D:
		case DEVICE_GS05D:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_sensor_same_side_as_gps_antenna(void)
{
	switch(s_hard_ware.dev_type)
 	{
		case DEVICE_GS08:
			return true;

		default:
			return false;
 	}
}


bool hard_ware_device_unlock_ip(void)
{
	u16 device_type;
	
	config_service_get(CFG_DEVICETYPE, TYPE_SHORT, &device_type , sizeof(device_type));
	switch(device_type)
 	{
		case DEVICE_GS03C:
		case DEVICE_GS03D:
		case DEVICE_GS05C:
		case DEVICE_GS05D:
			return true;

		default:
			return false;
 	}
}


static bool hard_ware_is_charge_voltage_ready(void)
{				
	if (!s_hard_ware.inited)
	{
		return false;
	}

	if (hard_ware_has_battery() == false)
	{
		return false;
	}

	if (hard_ware_has_vcdt_voltage())
	{
		if (s_hard_ware.vcdt_voltage <= 3.0f)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		if (s_hard_ware.power_voltage < 7.0f)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}


static float hard_ware_get_charge_current(bool charging)
{	
	float charge_current = 0.0;
	
	if (!s_hard_ware.inited)
	{
		return charge_current;
	}

	if (hard_ware_has_battery() == false)
	{
		return charge_current;
	}
	
	if (charging == false)
	{
		charge_current = 0.0;
	}
	else
	{
		switch(s_hard_ware.dev_type)
		{
			case DEVICE_W1:
				charge_current = 0.5;
				break;
			case DEVICE_W3:
			case DEVICE_W7:
				charge_current = 1.0;
				break;
			case DEVICE_W12:
			case DEVICE_W18:
				charge_current = 1.75;
				break;
			case DEVICE_GS06:
			case DEVICE_GS08:
				charge_current = 0.2;
				break;
			default:
				if (hard_ware_is_device_05())
				{
					charge_current = 0.06;
				}
				else if (hard_ware_is_device_obd())
				{
					charge_current = 0.08;
				}
				else
				{
					charge_current = 0.05;
				}
				break;
		}
	}
	return charge_current;
}






GM_ERRCODE hard_ware_get_acc_level(bool* p_state)
{
	if (false == s_hard_ware.inited || NULL == p_state)
	{
		return GM_NOT_INIT;
	}
	if(system_state_get_acc_is_line_mode())
	{
		*p_state = s_hard_ware.acc_record.state;
		LOG(DEBUG,"ACC from line is %d",*p_state);
	}
	else
	{
		*p_state = (GM_SYSTEM_STATE_WORK == system_state_get_work_state());
		LOG(DEBUG,"ACC from sensor is %d",*p_state);
	}
	return GM_SUCCESS;
}

GM_ERRCODE hard_ware_get_acc_line_level(bool* p_state)
{
	if (false == s_hard_ware.inited || NULL == p_state)
	{
		return GM_NOT_INIT;
	}
	if(system_state_get_acc_is_line_mode())
	{
		*p_state = s_hard_ware.acc_record.state;
		LOG(DEBUG,"ACC from line is %d",*p_state);
	}
	else
	{
		*p_state = false;
		LOG(DEBUG,"ACC line is invalid,return false");
	}
	return GM_SUCCESS;
}


s8 hard_ware_get_temperature(void)
{
	if (false == s_hard_ware.inited)
	{
		return 25;
	}

	return s_hard_ware.baton_temperature;
}



/**
 * Function:   设置GPS LED状态
 * Description:
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_gps_led(bool is_on)
{
	S32 ret = 0;


	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (s_hard_ware.gps_led_is_on == is_on)
	{
		return GM_SUCCESS;
	}
	else
	{
		s_hard_ware.gps_led_is_on = is_on;
			
		ret = GM_IsinkBacklightCtrl(is_on,is_on);
		if (0 == ret)
		{
			return GM_SUCCESS;
		}
		else
		{
			LOG(ERROR,"Failed to GM_IsinkBacklightCtrl,ret=%d!",ret);
			return GM_HARD_WARE_ERROR;
		}
	}
}

/**
 * Function:   设置GSM LED状态
 * Description:
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_gsm_led(bool is_on)
{
	S32 ret = 0;
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (s_hard_ware.gsm_led_is_on == is_on)
	{
		return GM_SUCCESS;
	}
	else
	{
		s_hard_ware.gsm_led_is_on = is_on;

		ret = GM_KpledLevelCtrl(is_on,is_on);
		if (0 == ret)
		{
			return GM_SUCCESS;
		}
		else
		{
			LOG(ERROR,"Failed to GM_KpledLevelCtrl,ret=%d!",ret);
			return GM_HARD_WARE_ERROR;
		}
	}
}


/**
 * Function:   设置POWER LED状态
 * Description:
 * Input:	   state:true——亮灯；false——灭灯
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_power_led(bool is_on)
{
	S32 ret = 0;
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (s_hard_ware.power_led_is_on == is_on)
	{
		return GM_SUCCESS;
	}
	else
	{
		s_hard_ware.power_led_is_on = is_on;
		if (hard_ware_is_at_command())
		{
			ret = GM_GpioSetLevel(GM_POWER_LED_GPIO_PIN, (Enum_PinLevel)(s_hard_ware.power_led_is_on));
		}
		else
		{
			ret = GM_GpioSetLevel(GM_POWER_LED_GPIO_PIN, (Enum_PinLevel)(!s_hard_ware.power_led_is_on));
		}
		if (0 == ret)
		{
			return GM_SUCCESS;
		}
		else
		{
			LOG(ERROR,"Failed to GM_GpioSetLevel,ret=%d!",ret);
			return GM_HARD_WARE_ERROR;
		}
	}
}


/**
 * Function:   设置断油电IO状态
 * Description:
 * Input:	   state:true——断油电；false——恢复油电
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_relay(bool state)
{
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}
	GM_GpioSetLevel(GM_RELAY_CTROL_GPIO_PIN,(Enum_PinLevel)state);
	return GM_SUCCESS;
}

/**
 * Function:   设置看门狗IO电平
 * Description:
 * Input:	   state:true——高电平；false——低电平
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_set_watchdog(bool state)
{
	if (false == s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}
	//总是返回0,不需要检查是否执行成功
	GM_GpioSetLevel(GM_WDT_CTROL_GPIO_PIN, (Enum_PinLevel)state);
	return GM_SUCCESS;
}


/**
 * Function:   获取供电电源电压值
 * Description:单位伏特（v）
 * Input:	   无
 * Output:	   p_voltage:电压值指针
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_get_power_voltage(float* p_voltage)
{
	if (false == s_hard_ware.inited || NULL == p_voltage)
	{
		return GM_NOT_INIT;
	}
	*p_voltage = s_hard_ware.power_voltage;
	return GM_SUCCESS;
}

GM_ERRCODE hard_ware_get_internal_battery_voltage(float* p_voltage)
{
	if (false == s_hard_ware.inited || NULL == p_voltage)
	{
		LOG(ERROR,"Has not inited hardware!");
		return GM_NOT_INIT;
	}
	*p_voltage = s_hard_ware.battery_voltage;
	return GM_SUCCESS;
}

bool hard_ware_battery_is_charging(void)
{
	return s_hard_ware.battery_is_charging;
}

bool hard_ware_battery_is_full(void)
{
	return s_hard_ware.battery_is_full;
}


u8 hard_ware_get_internal_battery(float battery)
{
	ConfigDeviceTypeEnum dev_type_id = DEVICE_MAX;
	float *battary_level = NULL;
	u8 level = 0;
	config_service_get(CFG_DEVICETYPE, TYPE_SHORT, &dev_type_id, sizeof(U16));

	switch(dev_type_id)
	{
		case DEVICE_W1:
			battary_level = s_1_2ah_battary_level;
			break;
		case DEVICE_W3:
			battary_level = s_3ah_battary_level;
			break;
		case DEVICE_W12:
			battary_level = s_12ah_battary_level;
			break;
		case DEVICE_W18:
			battary_level = s_18ah_battary_level;
			break;
		default:
			battary_level = s_6ah_battary_level;
			break;
	}
	
	for(level = 0; level < 20 ; level++)
	{
		if(battery >= battary_level[level])
		{
			 break; 
		}
	}

	return level;
}

GM_ERRCODE hard_ware_calu_internal_battery_percent(U8* p_percent)
{				
	if (!s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (NULL == p_percent)
	{
		return GM_PARAM_ERROR;
	}

	//W系列电池曲线
	if (hard_ware_is_device_w())
	{
		u8 percent = 0;

		percent = hard_ware_get_internal_battery(s_hard_ware.battery_voltage);
		//LOG(INFO, "s_hard_ware.battery_voltage:percent:%d,%.3f", percent, s_hard_ware.battery_voltage);
		if(percent >= 20)
		{
			*p_percent = 1;
		}
		else if (percent == 0)
		{
			if (s_hard_ware.battery_voltage >= 4.15)
			{
			    *p_percent = 100;
			}
			else
			{
				*p_percent = 99;
			}
		}
		else
		{
			*p_percent = 100 - 5*percent;
		}
	}
	else
	{
		if (s_hard_ware.battery_voltage >= 4.15)
		{
		    *p_percent = 100;
		}
		else if (s_hard_ware.battery_voltage <= 3.50)
		{
		    *p_percent = 1;
		}
		else
		{
		    *p_percent = applied_math_round((s_hard_ware.battery_voltage - 3.50) * 100 / 0.65);
		}
	}
	
	return GM_SUCCESS;

}



void hard_ware_internal_battery_percent(void)
{
	u8 battery_percent = 0;
	u8 false_state_hold_seconds = 0;
	u8 true_state_hold_seconds = 0;
	
	hard_ware_calu_internal_battery_percent(&battery_percent);
	if (s_hard_ware.battery_voltage_percent == 0)
	{	//初值
		s_hard_ware.battery_voltage_percent = battery_percent;
		GM_memset(&s_hard_ware.battery_voltage_percent_record, 0, sizeof(StateRecord));	
	}
	
	if (s_hard_ware.battery_voltage_percent > battery_percent)
	{
		s_hard_ware.battery_voltage_percent_record.false_state_hold_seconds++;
		s_hard_ware.battery_voltage_percent_record.true_state_hold_seconds = 0;
	}
	else if (hard_ware_is_charge_voltage_ready())
	{   //充电器插入才允许电压升高
		s_hard_ware.battery_voltage_percent_record.false_state_hold_seconds = 0;
		s_hard_ware.battery_voltage_percent_record.true_state_hold_seconds++;
	}
	else
	{		
		s_hard_ware.battery_voltage_percent_record.false_state_hold_seconds = 0;
		s_hard_ware.battery_voltage_percent_record.true_state_hold_seconds = 0;
	}

	//W系列电池电量每5%变化，时间稍微长点
	false_state_hold_seconds = (hard_ware_is_device_w()) ? 180 : 36;
	true_state_hold_seconds = (hard_ware_is_device_w()) ? 60 : 12;
	if (s_hard_ware.battery_voltage_percent_record.false_state_hold_seconds > false_state_hold_seconds || s_hard_ware.battery_voltage_percent_record.true_state_hold_seconds > true_state_hold_seconds)
	{
		s_hard_ware.battery_voltage_percent = battery_percent;
		s_hard_ware.battery_voltage_percent_record.false_state_hold_seconds = 0;
		s_hard_ware.battery_voltage_percent_record.true_state_hold_seconds = 0;
		//gps_service_heart_atonce();
	}
}



GM_ERRCODE hard_ware_get_extern_battery_percent(U8* p_percent)
{
	if (false == s_hard_ware.inited || NULL == p_percent)
	{
		return GM_NOT_INIT;
	}
	*p_percent = applied_math_round(s_hard_ware.extern_battery_percent);
	return GM_SUCCESS;
}

u8 hard_ware_get_internal_battery_percent(void)
{
	return s_hard_ware.battery_voltage_percent;
}

GM_ERRCODE hard_ware_get_internal_battery_level(U8* p_level)
{
	if (!s_hard_ware.inited)
	{
		return GM_NOT_INIT;
	}

	if (NULL == p_level)
	{
		return GM_PARAM_ERROR;
	}
	
	if (s_hard_ware.battery_voltage <= 3.45f)
    {
        *p_level = 0;
    }
    else if (s_hard_ware.battery_voltage <= 3.5f)
    {
        *p_level = 1;
    }
    else if (s_hard_ware.battery_voltage <= 3.6f)
    {
        *p_level = 2;
    }
    else if (s_hard_ware.battery_voltage <= 3.8f)
    {
        *p_level = 3;
    }
    else if (s_hard_ware.battery_voltage <= 3.95f)
    {
        *p_level = 4;
    }
    else if (s_hard_ware.battery_voltage <= 4.1f)
    {
        *p_level = 5;
    }
    else
    {
        *p_level = 6;
    }
	return GM_SUCCESS;
}


static float hard_ware_smooth_voltage_avg(CircularQueue *voltage_que, float last_voltage)
{
	u8 index;
	float voltage_avg = 0;
	float last_n_voltage = 0;
	
	circular_queue_en_queue_f(voltage_que,last_voltage);
	//取前20次平均值
	for(index = 0;index < 20;index++)
	{
		if (false == circular_queue_get_by_index_f(voltage_que,index,&last_n_voltage))
		{
			break;
		}
		voltage_avg = (voltage_avg*index + last_n_voltage)/(index + 1);
		last_n_voltage = 0;
	}
	
	return voltage_avg;
}



static void hard_ware_read_voltage(void)
{
    u32 realtime_voltage = 0;
	float power_voltage = 0;


	if (hard_ware_is_device_w())
    {
    	s_hard_ware.power_voltage =  0;
    	return;
    }
    
    if(0xFFFF == GM_AdcRead(GM_POWER_VOLTAGE_ADC_CHANNEL,&realtime_voltage))
    {
    	LOG(ERROR,"Failed to GM_AdcRead!");
    	return;
    }
	
    power_voltage = realtime_voltage * 145.6 /1024.0;
	
	//补偿二极管压降
    if (power_voltage > 0.5)
    {
        power_voltage += (0.7 + 0.35);   
    } 
	if(power_voltage < 7)
	{
		s_hard_ware.power_voltage =  0;
	}
	else
	{
		if (s_hard_ware.power_voltage > 7)
		{
			s_hard_ware.power_voltage = applied_math_lowpass_filter(s_hard_ware.power_voltage, power_voltage, 0.5);
		}
		else
		{			
    		s_hard_ware.power_voltage = power_voltage;
		}
	}
}

//电压等级标准
static U8 voltage_grade_standard[]={1,12,24,36,48,60,72,84,96};

/**
 * Function:   判断电压等级
 * Description:
 * Input:      voltage:电压
 * Output:     
 * Return:     电压等级:0——未知,其它情况返回对应12,24,36,48,60,72,84,96
 * Others:      
 */
static U8 hard_ware_calc_extern_voltage_grade(float voltage)
{
	U8 index = 0;
	
	U8 array_size = sizeof(voltage_grade_standard)/sizeof(U8);
	
	if (voltage > voltage_grade_standard[array_size -1])
	{
		return voltage_grade_standard[array_size -1];
	}
	for (index = 0; index < array_size - 1; ++index)
	{
		if (voltage > voltage_grade_standard[index] + 6 && voltage <= voltage_grade_standard[index + 1] + 6)
		{
			return voltage_grade_standard[index + 1];
		}
	}
	return 0;
}

//根据电压值和电压等级计算电量百分比,返回[0,100.0]
static float hard_ware_calc_extern_battery_percent(float voltage, U8 voltage_grade)
{
	float extern_battery_percent = 0;
	
	switch(voltage_grade)
	{
	case 0:
		extern_battery_percent = 0;
		break;

	case 12:
		if (voltage >= 12)
		{
			extern_battery_percent = 100;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 24:
		if (voltage >= 24)
		{
			extern_battery_percent = 100;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 36:
		if (voltage > 33)
		{
			extern_battery_percent = (voltage - 33) * 100 /9;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 48:
		if (voltage > 42)
		{
			extern_battery_percent = (voltage - 42) * 100 /14;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 60:
		if (voltage > 56)
		{
			extern_battery_percent = (voltage - 56) * 100 /14;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 72:
		if (voltage > 70)
		{
			extern_battery_percent = (voltage - 70) * 100 /14;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	case 84:
		if (voltage > 83)
		{
			extern_battery_percent = (voltage - 83) * 100 /13;
		}
		else
		{
			extern_battery_percent = 0;
		}
		break;

	default:
		extern_battery_percent = 100;
		break;
	}
	
	extern_battery_percent = (extern_battery_percent > 100) ? 100 : extern_battery_percent;
	
	return extern_battery_percent;

}

static void hard_ware_check_power_range(void)
{
	GM_CHANGE_ENUM power_range_change = GM_NO_CHANGE;
	bool is_90v_power = false;
	U16 voltage_alarm_hold_time_sec = 5;
	JsonObject* p_log_root = NULL;
	bool gpio_level = false;

	if (hard_ware_is_device_05())
	{
		return;
	}
	
	gpio_level = GM_GpioGetLevel(GM_POWER_90V_GPIO_PIN);
	config_service_get(CFG_POWER_CHECK_TIME, TYPE_SHORT, &voltage_alarm_hold_time_sec, sizeof(voltage_alarm_hold_time_sec));
	power_range_change	= util_check_state_change(gpio_level, &s_hard_ware.power_range_record,voltage_alarm_hold_time_sec,voltage_alarm_hold_time_sec);
	if (GM_CHANGE_TRUE == power_range_change)
	{
		is_90v_power = false;
		config_service_set(CFG_IS_90V_POWER, TYPE_BOOL, &is_90v_power, sizeof(bool));
		config_service_save_to_local();
	}
	else if (GM_CHANGE_FALSE == power_range_change)
	{
		is_90v_power = true;
		config_service_set(CFG_IS_90V_POWER, TYPE_BOOL, &is_90v_power, sizeof(bool));
		config_service_save_to_local();
	}
	else
	{
		return;
	}

	p_log_root = json_create();
	json_add_string(p_log_root, "event", "power_range_change");
	json_add_string(p_log_root, "voltage", is_90v_power ? "range[9V-90V]" : "range[9V-36V]");
	log_service_upload(INFO,p_log_root);
}

static void hard_ware_check_high_voltage_alarm(void)
{
	GM_ERRCODE ret = GM_SUCCESS;
	U16 voltage_alarm_hold_time_sec = 5;
	bool is_90v_power = false;
	U8 high_voltage_alarm_threshhold = GM_HIGH_VOLTAGE_ALARM_THRESHHOLD_36V;
	JsonObject* p_log_root = NULL;
	GM_CHANGE_ENUM alarm_state_change = GM_NO_CHANGE;
	AlarmInfo alarm_info;
	
	ret = config_service_get(CFG_POWER_CHECK_TIME, TYPE_SHORT, &voltage_alarm_hold_time_sec, sizeof(voltage_alarm_hold_time_sec));
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to config_service_get,ret=%d",ret);
		return;
	}
	
	ret = config_service_get(CFG_IS_90V_POWER, TYPE_BOOL, &is_90v_power, sizeof(bool));
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to config_service_get,ret=%d",ret);
		return;
	}

	high_voltage_alarm_threshhold = is_90v_power ? GM_HIGH_VOLTAGE_ALARM_THRESHHOLD_90V : GM_HIGH_VOLTAGE_ALARM_THRESHHOLD_36V;
	alarm_state_change  = util_check_state_change(s_hard_ware.power_voltage > high_voltage_alarm_threshhold, &s_hard_ware.high_voltage_alarm_record,voltage_alarm_hold_time_sec,voltage_alarm_hold_time_sec);

	if (GM_CHANGE_TRUE == alarm_state_change)
	{
		//报电压过高报警
		alarm_info.type = ALARM_POWER_HIGH;
		alarm_info.info = (u16)s_hard_ware.power_voltage;
		ret = gps_service_push_alarm(&alarm_info);
		if (GM_SUCCESS != ret)
		{
			LOG(ERROR, "Failed to gps_service_push_alarm(ALARM_POWER_HIGH),ret=%d", ret);
		}
		
		system_state_set_high_voltage_alarm(true);

		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("high_voltage_alarm"));
		json_add_double(p_log_root, "voltage", s_hard_ware.power_voltage);
		log_service_upload(FATAL,p_log_root);
	}
    else if(GM_CHANGE_FALSE == alarm_state_change)
    {
    	
    	system_state_set_high_voltage_alarm(false);
		
		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("high_voltage_alarm recover"));
		json_add_double(p_log_root, "voltage", s_hard_ware.power_voltage);
		log_service_upload(FATAL,p_log_root);
    }
	else
	{
	}
		
}

static void hard_ware_check_power_off_alarm(void)
{
	GM_ERRCODE ret = GM_SUCCESS;
	U16 voltage_alarm_hold_time_sec = 0;
	JsonObject* p_log_root = NULL;
	GM_CHANGE_ENUM alarm_state_change = GM_NO_CHANGE;
	AlarmInfo alarm_info;
	U8 power_off_alarm_disable = 0;
	
	config_service_get(CFG_POWER_CHECK_TIME, TYPE_SHORT, &voltage_alarm_hold_time_sec, sizeof(voltage_alarm_hold_time_sec));
	config_service_get(CFG_CUTOFFALM_DISABLE, TYPE_BYTE, &power_off_alarm_disable, sizeof(power_off_alarm_disable));

	if (power_off_alarm_disable)
	{
		return;
	}
	
	alarm_state_change	= util_check_state_change(s_hard_ware.power_voltage < GM_BATTERY_HIGHEST_VOLTAGE ,&s_hard_ware.power_off_alarm_record,voltage_alarm_hold_time_sec,10);

    if (GM_CHANGE_TRUE == alarm_state_change)
	{
		//报断电报警
		alarm_info.type = ALARM_POWER_OFF;
		alarm_info.info = (u16)s_hard_ware.power_voltage;
		ret = gps_service_push_alarm(&alarm_info);
		if (GM_SUCCESS != ret)
		{
			LOG(ERROR, "Failed to gps_service_push_alarm(ALARM_POWER_OFF),ret=%d", ret);
		}
		
		system_state_set_power_off_alarm(true);

		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("power_off_alarm"));
		json_add_double(p_log_root, "voltage", s_hard_ware.power_voltage);
		log_service_upload(DEBUG,p_log_root);
	}
	else if(GM_CHANGE_FALSE == alarm_state_change)
    {
    	system_state_set_power_off_alarm(false);
	
		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("power_off_alarm recover"));
		json_add_double(p_log_root, "voltage", s_hard_ware.power_voltage);
		log_service_upload(DEBUG,p_log_root);
		
    }
	else
	{
		//do nothing
	}
		
}


static void hard_ware_check_vcdt_voltage(void)
{
	u32 realtime_voltage = 0;
	float vcdt_voltage = 0;
				
	if (!s_hard_ware.inited)
	{
		return;
	}
	
	if (!hard_ware_has_vcdt_voltage() && !config_service_is_test_mode())
	{
		return;
	}
	
	if(0xFFFF == GM_AdcRead(GM_VCDT_VOLTAGE_ADC_CHANNEL,&realtime_voltage))
	{
		LOG(ERROR,"Failed to GM_AdcRead!");
		return;
	}
	
	vcdt_voltage = (realtime_voltage * 2.8f * 369.0f / 39.0f) / 1023.0f;

	s_hard_ware.vcdt_voltage = hard_ware_smooth_voltage_avg(&s_hard_ware.vcdt_voltage_queue, vcdt_voltage);
	//LOG(DEBUG, "clock(%d), hard_ware_check_vcdt_voltage vcdt_voltage(%f)", util_clock(),s_hard_ware.vcdt_voltage);
}



static void hard_ware_check_battery(void)
{
	GM_ERRCODE ret = GM_SUCCESS;
	U16 voltage_alarm_hold_time_sec = 0;
	JsonObject* p_log_root = NULL;
    gm_chr_status_struct charge_status = {0};
	bool app_battery_mgr = true;
	bool current_alarm_state = false;
	GM_CHANGE_ENUM alarm_state_change = GM_NO_CHANGE;
	GM_CHANGE_ENUM battery_is_full_change = GM_NO_CHANGE;
	AlarmInfo alarm_info;
	bool voltage_alarm_disable = false;

	U16 min_charge_time = 0;

	ret = config_service_get(CFG_POWER_CHECK_TIME, TYPE_SHORT, &voltage_alarm_hold_time_sec, sizeof(voltage_alarm_hold_time_sec));
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to config_service_get(CFG_POWER_CHECK_TIME),ret=%d",ret);
		return;
	}

	ret = config_service_get(CFG_APP_BATTERT_MGR, TYPE_BOOL, &app_battery_mgr, sizeof(app_battery_mgr));
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to config_service_get(CFG_APP_BATTERT_MGR),ret=%d", ret);
		return;
	}

	config_service_get(CFG_POWER_CHARGE_MIN_TIME, TYPE_SHORT, &min_charge_time, sizeof(min_charge_time));
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to config_service_get(CFG_APP_BATTERT_MGR),ret=%d", ret);
		return;
	}

	//GS05系列没有使用MTK内部充电管理,这些数据对05无效
	ret = (GM_ERRCODE)GM_GetChrStatus((U8* )&charge_status);
	if (GM_SUCCESS != ret)
	{
		LOG(ERROR,"Failed to GM_GetChrStatus(),ret=%d", ret);
		return;
	}
	
	s_hard_ware.battery_voltage = hard_ware_smooth_voltage_avg(&s_hard_ware.battery_voltage_queue,charge_status.VBAT/1000000.0);
	s_hard_ware.battery_is_full = GM_GpioGetLevel(GM_GPS_CHARGE_LEVLE_GPIO_PIN);
	hard_ware_internal_battery_percent();

	//自动检测充电类型，只要充电IC有控制在充电，就说明在APP控制
	battery_is_full_change = util_check_state_change(s_hard_ware.battery_is_full, &s_hard_ware.battery_is_full_record, voltage_alarm_hold_time_sec,voltage_alarm_hold_time_sec);
	if (GM_CHANGE_FALSE == battery_is_full_change)
	{
		if (!app_battery_mgr)
		{
			app_battery_mgr = true;
			config_service_set(CFG_APP_BATTERT_MGR, TYPE_BOOL, &app_battery_mgr, sizeof(app_battery_mgr));
			config_service_save_to_local();
		}
	}
	LOG(DEBUG,"battery voltage:%f,charge_current:%f,is_charging:%d,is_full:%d, app_battery_mgr:%d",s_hard_ware.battery_voltage,s_hard_ware.battery_charge_current,s_hard_ware.battery_is_charging,s_hard_ware.battery_is_full, app_battery_mgr);

	//即使型号未知也一直充电
	GM_GpioSetLevel(GM_GPS_CHARGE_CTROL_GPIO_PIN,PINLEVEL_LOW);
	if (hard_ware_has_vcdt_voltage())
	{
		if (false == hard_ware_is_charge_voltage_ready())
		{
			s_hard_ware.battery_is_charging = false;
			s_hard_ware.battery_charge_current = hard_ware_get_charge_current(false);
		}
		else
		{
			s_hard_ware.battery_is_charging = true;
			s_hard_ware.battery_charge_current = hard_ware_get_charge_current(true);
		}
	}
	//GS05系列/W系列,应用层控制
	else if (app_battery_mgr)
	{
		if (GM_CHANGE_TRUE == battery_is_full_change || false == hard_ware_is_charge_voltage_ready())
		{
			s_hard_ware.battery_is_charging = false;
		    s_hard_ware.battery_charge_current = hard_ware_get_charge_current(false);
		    LOG(DEBUG,"is full");
		}
		else if(GM_CHANGE_FALSE == battery_is_full_change)
		{
			s_hard_ware.battery_is_charging = true;
		    s_hard_ware.battery_charge_current = hard_ware_get_charge_current(true);
		    LOG(DEBUG,"is not full.");		
		}
		else
		{
			LOG(DEBUG,"Dont change charge state.");
		}
	}
	else
	{
		LOG(DEBUG,"MTK charge.");
		s_hard_ware.battery_charge_current = charge_status.ICHARGE > 0 ? charge_status.ICHARGE/1000000.0 : 0.0;
		s_hard_ware.battery_is_charging = (charge_status.chr_plug == CHARGER_PLUG_IN);
	}

	system_state_set_has_started_charge(s_hard_ware.battery_is_charging);
	config_service_get(CFG_LOWBATTALM_DISABLE, TYPE_BYTE, &voltage_alarm_disable, sizeof(voltage_alarm_disable));
	
	//正在充电或者报警关闭
	if (s_hard_ware.battery_is_charging || voltage_alarm_disable)
	{
		return;
	}
	
    current_alarm_state = s_hard_ware.low_voltage_alarm_record.state;
	if (s_hard_ware.battery_voltage_percent >= 20)
	{
		current_alarm_state = false;
	}
	else if (s_hard_ware.battery_voltage_percent <= 15)
	{
		current_alarm_state = true;
	}
	else
	{
	}
	
	alarm_state_change	= util_check_state_change(current_alarm_state,&s_hard_ware.low_voltage_alarm_record,voltage_alarm_hold_time_sec,min_charge_time);
	if (GM_CHANGE_TRUE == alarm_state_change)
	{
		//报低电压报警
		alarm_info.type = ALARM_BATTERY_LOW;
		alarm_info.info = (u16)s_hard_ware.battery_voltage;
		ret = gps_service_push_alarm(&alarm_info);
		if (GM_SUCCESS != ret)
		{
			LOG(ERROR, "Failed to gps_service_push_alarm(ALARM_BATTERY_LOW),ret=%d", ret);
		}
		system_state_set_battery_low_voltage_alarm(true);
		

		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("low_voltage_alarm"));
		log_service_upload(DEBUG,p_log_root);
	}
	else if(GM_CHANGE_FALSE == alarm_state_change)
    {
    	system_state_set_battery_low_voltage_alarm(false);
	
		//上传日志
		p_log_root = json_create();
		json_add_string(p_log_root, "event", ("low_voltage_alarm recover"));
		json_add_double(p_log_root, "voltage", s_hard_ware.battery_voltage);
		log_service_upload(DEBUG,p_log_root);
    }
	else
	{
		//do nothing
	}
	
	return;
}


static float hard_ware_smooth_adc_avg(CircularQueue *adc_que, s32 last_adc)
{
	u8 index;
	float adc_avg = 0.0f;
	s32 last_n_adc = 0;
	
	circular_queue_en_queue_i(adc_que,last_adc);
	//取前20次平均值
	for(index = 0;index < 5;index++)
	{
		if (false == circular_queue_get_by_index_i(adc_que,index,&last_n_adc))
		{
			break;
		}
		adc_avg = (adc_avg*index + last_n_adc)/(index + 1);
		last_n_adc = 0;
	}
	
	return adc_avg;
}



static float hard_ware_find_adc_by_temperature(s8 temperature)
{
	u8 temp_type;
	u16 idx;
	
	config_service_get(CFG_TEMP_SENSOR, TYPE_BYTE, &temp_type, sizeof(temp_type));
	for (idx=0; idx<GM_TEMP_MAX_CNT; idx++)
	{
		if (temperature == s_temperature[temp_type][idx].temperature)
		{
			break;
		}
	}

	if (idx >= GM_TEMP_MAX_CNT)
	{
		return 0.0f;
	}

	return s_temperature[temp_type][idx].adc_value;
}




static void hard_ware_check_baton(void)
{
	u32 realtime_voltage = 0;
	
    if(0xFFFF == GM_AdcRead(GM_BATON_ADC_CHANNEL,&realtime_voltage))
    {
    	LOG(ERROR,"Failed to GM_AdcRead!");
    	return;
    }

    s_hard_ware.baton_adc = hard_ware_smooth_adc_avg(&s_hard_ware.baton_adc_queue, realtime_voltage);
	LOG(DEBUG, "clock(%d) hard_ware_check_baton GM_BATON_ADC_CHANNEL(%.2f-%d)", util_clock(), s_hard_ware.baton_adc, realtime_voltage);
	auto_test_key_value(realtime_voltage);
	if (hard_ware_has_sos())
	{
		hard_ware_check_sos_alarm();
	}
	hard_ware_check_baton_temperature();
}


static void hard_ware_check_sos_alarm(void)
{
	GM_CHANGE_ENUM change = GM_NO_CHANGE;
	bool baton_level = true;
	JsonObject* p_log_root = NULL;
	AlarmInfo alarm_info;
	GM_ERRCODE ret;
	u8 key_feature;

	if (util_clock() < 10)
	{
		return;
	}
	
	if (s_hard_ware.baton_adc <= 50)
	{
		baton_level = false;
	}

	config_service_get(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
	change = util_check_state_change(baton_level,&s_hard_ware.sos_alarm_record,5,5);
	if (GM_CHANGE_FALSE == change)
	{
		if (KEY_SOS != key_feature)
		{
			key_feature = KEY_SOS;
			config_service_set(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
			config_service_save_to_local();
			p_log_root = json_create();
			json_add_string(p_log_root, "event", ("baton_change_to_check_sos_alarm"));
			json_add_double(p_log_root, "adc", s_hard_ware.baton_adc);
			log_service_upload(DEBUG,p_log_root);
		}
		
		if (!system_state_get_sos_alarm())
		{
			//报SOS报警
			GM_memset(&alarm_info, 0x00, sizeof(AlarmInfo));
			alarm_info.type = ALARM_SOS;
			ret = gps_service_push_alarm(&alarm_info);
			if (GM_SUCCESS != ret)
			{
				LOG(ERROR, "Failed to gps_service_push_alarm(ALARM_SOS),ret=%d", ret);
			}

			system_state_set_sos_alarm(true);

			//上传日志
			p_log_root = json_create();
			json_add_string(p_log_root, "event", ("sos_alarm"));
			json_add_double(p_log_root, "adc", s_hard_ware.baton_adc);
			log_service_upload(FATAL,p_log_root);
		}
	}
	else if (GM_CHANGE_TRUE == change && KEY_SOS == key_feature)
	{
		if (system_state_get_sos_alarm() && s_hard_ware.baton_adc >= 986.46) //2.7V-->986.4642857142857
		{
			system_state_set_sos_alarm(false);
	
			//上传日志
			p_log_root = json_create();
			json_add_string(p_log_root, "event", ("sos_alarm recover"));
			json_add_double(p_log_root, "adc", s_hard_ware.baton_adc);
			log_service_upload(DEBUG,p_log_root);
		}
	}

	LOG(DEBUG, "hard_ware_check_sos_alarm sos_alarm_record(%d) baton_level(%d) change(%d)", s_hard_ware.sos_alarm_record.state, baton_level, change);
}



static void hard_ware_check_baton_temperature(void)
{
	GM_CHANGE_ENUM change = GM_NO_CHANGE;
	bool baton_level = false;
	JsonObject* p_log_root = NULL;
	u16 idx;
	u8 temp_type;
	u8 key_feature;

	config_service_get(CFG_TEMP_SENSOR, TYPE_BYTE, &temp_type, sizeof(temp_type));
	config_service_get(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
	
	//用了劫警按钮后再用回温度传感器,或者第一次使用，持续一分钟
	if (key_feature != KEY_TEMP)
	{
		if (s_hard_ware.baton_adc >= hard_ware_find_adc_by_temperature(60) && s_hard_ware.baton_adc <= hard_ware_find_adc_by_temperature(-30))
		{
			baton_level = true;
		}
		change = util_check_state_change(baton_level,&s_hard_ware.baton_record,60,60);
		if (GM_CHANGE_TRUE == change)
		{
			key_feature = KEY_TEMP;
			config_service_set(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
			config_service_save_to_local();
			s_hard_ware.baton_record.state = false;
			s_hard_ware.baton_record.true_state_hold_seconds = 0;
			s_hard_ware.baton_record.false_state_hold_seconds = 0;
			//设置当前温度
			for (idx=0; idx<GM_TEMP_MAX_CNT; idx++)
			{
				if (fabs(s_hard_ware.baton_adc - s_temperature[temp_type][idx].adc_value) <= s_temperature[temp_type][idx].adc_diff)
				{
					s_hard_ware.baton_temperature = s_temperature[temp_type][idx].temperature;
				}
			}
			if (system_state_get_sos_alarm())
			{
				system_state_set_sos_alarm(false);
			}
			//上传日志
			p_log_root = json_create();
			json_add_string(p_log_root, "event", ("baton_change_to_check_temperature"));
			json_add_double(p_log_root, "adc", s_hard_ware.baton_adc);
			log_service_upload(DEBUG,p_log_root);
		}
	}

	if (key_feature != KEY_TEMP)
	{
		return;
	}

	  
	for (idx=0; idx<GM_TEMP_MAX_CNT; idx++)
	{
		if (fabs(s_hard_ware.baton_adc - s_temperature[temp_type][idx].adc_value) <= s_temperature[temp_type][idx].adc_diff)
		{
			break;
		}
	}

	if (idx >= GM_TEMP_MAX_CNT)
	{
		//连续10次非法，认为温度传感器拔出
		change = util_check_state_change(true,&s_hard_ware.baton_record,10,10);
		if (GM_CHANGE_TRUE == change)
		{
			key_feature = KEY_NONE;
			config_service_set(CFG_KEY_FEATURE, TYPE_BYTE, &key_feature, sizeof(u8));
			config_service_save_to_local();
			s_hard_ware.sos_alarm_record.state = true;
			s_hard_ware.baton_record.state = false;
			s_hard_ware.baton_record.true_state_hold_seconds = 0;
			s_hard_ware.baton_record.false_state_hold_seconds = 0;
			//上传日志
			p_log_root = json_create();
			json_add_string(p_log_root, "event", ("baton_temperature_change_to_unknow"));
			json_add_double(p_log_root, "adc", s_hard_ware.baton_adc);
			log_service_upload(DEBUG,p_log_root);
		}
		LOG(DEBUG, "clock(%d) hard_ware_check_baton_temperature idx(%d), adc(%.2f)", util_clock(), idx, s_hard_ware.baton_adc);
		return;
	}
	else
	{
		s_hard_ware.baton_record.state = false;
		s_hard_ware.baton_record.true_state_hold_seconds = 0;
		s_hard_ware.baton_record.false_state_hold_seconds = 0;
	}
	
	if (s_hard_ware.baton_temperature != s_temperature[temp_type][idx].temperature)
	{
		baton_level = true;
	}
	
	change = util_check_state_change(baton_level,&s_hard_ware.temperature_record,10,10);
	LOG(DEBUG, "clock(%d) hard_ware_check_baton_temperature change:%d, idx(%d), temperature(%d/%d)", util_clock(), change, idx, s_hard_ware.baton_temperature, s_temperature[temp_type][idx].temperature);
	if (GM_CHANGE_TRUE == change)
	{
		s_hard_ware.baton_temperature = s_temperature[temp_type][idx].temperature;
		s_hard_ware.temperature_record.state = false;
		s_hard_ware.temperature_record.true_state_hold_seconds = 0;
		s_hard_ware.temperature_record.false_state_hold_seconds = 0;
		//拔掉温度传感器1秒大约降3度，5-20度触发，防止立即上传无效温度
		if (abs(s_hard_ware.baton_temperature - gps_service_get_last_temperature()) >= 5
			&& abs(s_hard_ware.baton_temperature - gps_service_get_last_temperature()) < 20)
		{
			gps_service_temperature_atonce();
		}
	}
}


static void hard_ware_check_acc(void)
{	
	//硬件设计上，高电平是没有接ACC，低电平是接了ACC,所以这里要取反
	bool acc_level = !GM_GpioGetLevel(GM_ACC_LEVEL_GPIO_PIN);
	GM_CHANGE_ENUM change = GM_NO_CHANGE;
	U16 auto_defence_delay = 0;
	bool is_defence_by_hand = false;
	
	config_service_get(CFG_IS_MANUAL_DEFENCE,TYPE_BOOL, &is_defence_by_hand, sizeof(is_defence_by_hand));
	config_service_get(CFG_AUTO_DEFENCE_DELAY, TYPE_SHORT, &auto_defence_delay, sizeof(auto_defence_delay));
	
	change = util_check_state_change(acc_level,&s_hard_ware.acc_record,5,5);
	if (GM_CHANGE_TRUE == change)
	{
		auto_test_acc_on();
		if(!is_defence_by_hand)
		{
			GM_StopTimer(GM_TIMER_HARDWARE_AUTODEFENCE);
			system_state_set_defence(false);
		}
	}
	else if(GM_CHANGE_FALSE == change)
	{
		auto_test_acc_off();
		if(!is_defence_by_hand)
		{
			GM_StartTimer(GM_TIMER_HARDWARE_AUTODEFENCE, auto_defence_delay, hard_ware_set_auto_defence);
		}
	}
	else
	{
		//do nothing
	}

	if (s_hard_ware.acc_record.state == false && system_state_get_vehicle_state() == VEHICLE_STATE_RUN)
	{
		s_hard_ware.acc_low_but_run_seconds++;
	}

	if(s_hard_ware.acc_record.state)
	{
		s_hard_ware.acc_low_but_run_seconds = 0;
	}

	//连续24小时ACC高            ————>接常电
	//连续1分钟运动状态ACC低————>没接ACC线
	if (s_hard_ware.acc_record.true_state_hold_seconds > SECONDS_PER_DAY || s_hard_ware.acc_low_but_run_seconds > SECONDS_PER_HOUR)
	{
		//ACC无效，取消自动设防
		system_state_set_acc_is_line_mode(false);
		if(!is_defence_by_hand)
		{
			GM_StopTimer(GM_TIMER_HARDWARE_AUTODEFENCE);
			system_state_set_defence(false);
		}
	}
	else
	{
		system_state_set_acc_is_line_mode(true);
	}
	
	//如果ACC状态发生变化，立即触发一次心跳
	if (change != GM_NO_CHANGE)
	{
		gps_service_heart_atonce();
	}
}

static void hard_ware_set_auto_defence(void)
{
	system_state_set_defence(true);
}

static void hard_ware_reboot_atonce(void)
{
    gps_service_save_to_history_file();
	if (hard_ware_is_at_command())
	{
		hard_ware_ec20_reset(true);
	}
	if (0 == GM_SystemReboot())
	{
		LOG(FATAL,"Failed to GM_SystemReboot!");
	}
}

/**
 * Function:   重启
 * Description:重启整个系统软件
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_reboot(const BootReason reason,U16 delay_seconds)
{
	//快速重复调用这个函数启动定时器会取消之前设置的定时器，导致无法重启，因此要用个静态变量记录下来
	static bool has_reboot = false;
	system_state_set_boot_reason(reason);

	if (0 == delay_seconds)
	{
		hard_ware_reboot_atonce();
		has_reboot = true;
	}
	else
	{
		if(!has_reboot)
		{
			GM_StartTimer(GM_TIMER_HARDWARE_REBOOT,delay_seconds*TIM_GEN_1SECOND,hard_ware_reboot_atonce);
			has_reboot = true;
		}
	}
	return GM_SUCCESS;
}

/**
 * Function:   休眠
 * Description:使系统休眠
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_sleep(void)
{
	GM_PERIPHERAL_TYPE peripheral_type;
	config_service_get(CFG_PERIPHERAL_TYPE, TYPE_BYTE, &peripheral_type, sizeof(peripheral_type));

	//如果是外接传感器，不关串口1，不休眠，不上报日志
	if (PERIPHERAL_TYPE_NONE == peripheral_type)
	{
		return GM_SUCCESS;
	}
	
	//如果通过AT指令连接模组，关串口1，不休眠，不上报日志
	uart_close_port(GM_UART_DEBUG);
	if(hard_ware_is_at_command())
	{
		return GM_SUCCESS;
	}
	
	//否则，关串口1，休眠，上报日志
	if (0 != GM_SleepEnable())
	{
		LOG(ERROR,"Failed to GM_SleepEnable.");
		return GM_HARD_WARE_ERROR;
	}
	else
	{
		//上传日志
		JsonObject* p_log_root = json_create();
		json_add_string(p_log_root, "event", ("sleep"));
		json_add_int(p_log_root, "csq", gsm_get_csq());
		log_service_upload(INFO,p_log_root);
		return GM_SUCCESS;
	}
	
}

GM_ERRCODE hard_ware_close_gps(void)
{
	GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_LOW);
	return GM_SUCCESS;
}


/**
 * Function:   唤醒
 * Description:使系统唤醒
 * Input:	   无
 * Output:	   无
 * Return:	   GM_SUCCESS——成功；其它错误码——失败
 * Others:	   
 */
GM_ERRCODE hard_ware_awake(void)
{
	if (0 == GM_SleepDisable())
	{
		LOG(ERROR,"Failed to hard_ware_awake.");
		return GM_HARD_WARE_ERROR;
	}
	else
	{
		//上传日志
		JsonObject* p_log_root = json_create();
		json_add_string(p_log_root, "event", ("awake"));
		json_add_int(p_log_root, "csq", gsm_get_csq());
		json_add_int(p_log_root, "run time", system_state_get_start_time());
		log_service_upload(INFO,p_log_root);
		return GM_SUCCESS;
	}
}

GM_ERRCODE hard_ware_open_gps(void)
{
	GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_HIGH);
    GM_SysMsdelay(10);
    GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_HIGH);
    GM_SysMsdelay(10);
    GM_GpioSetLevel(GM_GPS_POWER_CTROL_GPIO_PIN, PINLEVEL_HIGH);
	return GM_SUCCESS;
}


GM_ERRCODE hard_ware_ec20_power_key(bool is_on)
{
	if (is_on)
	{
		GM_GpioSetLevel(GM_EC20_POWER_KEY_GPIO_PIN, PINLEVEL_HIGH);
	}
	else
	{
		GM_GpioSetLevel(GM_EC20_POWER_KEY_GPIO_PIN, PINLEVEL_LOW);
	}
	return GM_SUCCESS;
}

GM_ERRCODE hard_ware_ec20_reset(bool is_on)
{
	if (is_on)
	{
		GM_GpioSetLevel(GM_EC20_RESET_GPIO_PIN, PINLEVEL_HIGH);
	}
	else
	{
		GM_GpioSetLevel(GM_EC20_RESET_GPIO_PIN, PINLEVEL_LOW);
	}
	return GM_SUCCESS;
}



