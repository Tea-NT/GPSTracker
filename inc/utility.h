#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <time.h>
#include "gm_type.h"
#include "error_code.h"
#include "gm_time.h"
#include "time.h"
#include "stdint.h"

#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080
#define BIT8    0x0100
#define BIT9    0x0200
#define BIT10   0x0400
#define BIT11   0x0800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000

#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000



#define GET_BIT0(val)   (((val) & BIT0)?1:0)
#define GET_BIT1(val)   (((val) & BIT1)?1:0)
#define GET_BIT2(val)   (((val) & BIT2)?1:0)
#define GET_BIT3(val)   (((val) & BIT3)?1:0)
#define GET_BIT4(val)   (((val) & BIT4)?1:0)
#define GET_BIT5(val)   (((val) & BIT5)?1:0)
#define GET_BIT6(val)   (((val) & BIT6)?1:0)
#define GET_BIT7(val)   (((val) & BIT7)?1:0)
#define GET_BIT8(val)   (((val) & BIT8)?1:0)
#define GET_BIT9(val)   (((val) & BIT9)?1:0)
#define GET_BIT10(val)  (((val) & BIT10)?1:0)
#define GET_BIT11(val)  (((val) & BIT11)?1:0)
#define GET_BIT12(val)  (((val) & BIT12)?1:0)
#define GET_BIT13(val)  (((val) & BIT13)?1:0)
#define GET_BIT14(val)  (((val) & BIT14)?1:0)
#define GET_BIT15(val)  (((val) & BIT15)?1:0)

#define GET_BIT16(val)  (((val) & BIT16)?1:0)
#define GET_BIT17(val)  (((val) & BIT17)?1:0)
#define GET_BIT18(val)  (((val) & BIT18)?1:0)
#define GET_BIT19(val)  (((val) & BIT19)?1:0)
#define GET_BIT20(val)  (((val) & BIT20)?1:0)
#define GET_BIT21(val)  (((val) & BIT21)?1:0)
#define GET_BIT22(val)  (((val) & BIT22)?1:0)
#define GET_BIT23(val)  (((val) & BIT23)?1:0)
#define GET_BIT24(val)  (((val) & BIT24)?1:0)
#define GET_BIT25(val)  (((val) & BIT25)?1:0)
#define GET_BIT26(val)  (((val) & BIT26)?1:0)
#define GET_BIT27(val)  (((val) & BIT27)?1:0)
#define GET_BIT28(val)  (((val) & BIT28)?1:0)
#define GET_BIT29(val)  (((val) & BIT29)?1:0)
#define GET_BIT30(val)  (((val) & BIT30)?1:0)
#define GET_BIT31(val)  (((val) & BIT31)?1:0)


#define SET_BIT0(val)   val = val|BIT0
#define SET_BIT1(val)   val = val|BIT1
#define SET_BIT2(val)   val = val|BIT2
#define SET_BIT3(val)   val = val|BIT3
#define SET_BIT4(val)   val = val|BIT4
#define SET_BIT5(val)   val = val|BIT5
#define SET_BIT6(val)   val = val|BIT6
#define SET_BIT7(val)   val = val|BIT7
#define SET_BIT8(val)   val = val|BIT8
#define SET_BIT9(val)   val = val|BIT9
#define SET_BIT10(val)  val = val|BIT10
#define SET_BIT11(val)  val = val|BIT11
#define SET_BIT12(val)  val = val|BIT12
#define SET_BIT13(val)  val = val|BIT13
#define SET_BIT14(val)  val = val|BIT14
#define SET_BIT15(val)  val = val|BIT15

#define SET_BIT16(val)  val = val|BIT16
#define SET_BIT17(val)  val = val|BIT17
#define SET_BIT18(val)  val = val|BIT18
#define SET_BIT19(val)  val = val|BIT19
#define SET_BIT20(val)  val = val|BIT20
#define SET_BIT21(val)  val = val|BIT21
#define SET_BIT22(val)  val = val|BIT22
#define SET_BIT23(val)  val = val|BIT23
#define SET_BIT24(val)  val = val|BIT24
#define SET_BIT25(val)  val = val|BIT25
#define SET_BIT26(val)  val = val|BIT26
#define SET_BIT27(val)  val = val|BIT27
#define SET_BIT28(val)  val = val|BIT28
#define SET_BIT29(val)  val = val|BIT29
#define SET_BIT30(val)  val = val|BIT30
#define SET_BIT31(val)  val = val|BIT31



#define CLR_BIT0(val)   val = val&(~BIT0)
#define CLR_BIT1(val)   val = val&(~BIT1)
#define CLR_BIT2(val)   val = val&(~BIT2)
#define CLR_BIT3(val)   val = val&(~BIT3)
#define CLR_BIT4(val)   val = val&(~BIT4)
#define CLR_BIT5(val)   val = val&(~BIT5)
#define CLR_BIT6(val)   val = val&(~BIT6)
#define CLR_BIT7(val)   val = val&(~BIT7)
#define CLR_BIT8(val)   val = val&(~BIT8)
#define CLR_BIT9(val)   val = val&(~BIT9)
#define CLR_BIT10(val)  val = val&(~BIT10)
#define CLR_BIT11(val)  val = val&(~BIT11)
#define CLR_BIT12(val)  val = val&(~BIT12)
#define CLR_BIT13(val)  val = val&(~BIT13)
#define CLR_BIT14(val)  val = val&(~BIT14)
#define CLR_BIT15(val)  val = val&(~BIT15)

#define CLR_BIT16(val)  val = val&(~BIT16)
#define CLR_BIT17(val)  val = val&(~BIT17)
#define CLR_BIT18(val)  val = val&(~BIT18)
#define CLR_BIT19(val)  val = val&(~BIT19)
#define CLR_BIT20(val)  val = val&(~BIT20)
#define CLR_BIT21(val)  val = val&(~BIT21)
#define CLR_BIT22(val)  val = val&(~BIT22)
#define CLR_BIT23(val)  val = val&(~BIT23)
#define CLR_BIT24(val)  val = val&(~BIT24)
#define CLR_BIT25(val)  val = val&(~BIT25)
#define CLR_BIT26(val)  val = val&(~BIT26)
#define CLR_BIT27(val)  val = val&(~BIT27)
#define CLR_BIT28(val)  val = val&(~BIT28)
#define CLR_BIT29(val)  val = val&(~BIT29)
#define CLR_BIT30(val)  val = val&(~BIT30)
#define CLR_BIT31(val)  val = val&(~BIT31)

#define BHIGH_BYTE(arg)        (*((u8 *)(&arg) + 1))
#define BLOW_BYTE(arg)        (*(u8 *)(&arg))
#define WHIGH_WORD(arg)      (*((u16 *)(&arg) + 1))
#define WLOW_WORD(arg)      (*(u16 *)(&arg))
#define MERGEBCD(a, b)     (((a)<<4)+((b)&0x0f))
#define UPPER_BYTE(n)      (((n) >> 8 ) & 0xff)
#define LOWER_BYTE(n)      (((n) & 0xff))
#define MKWORD(B1, B0)     ((((u16)B1)<<8) + ((u16)B0))
#define MKDWORD(B3, B2, B1, B0)  ((((u32)B3)<<24) + (((u32)B2)<<16) + (((u32)B1)<<8) + ((u32)B0))
#define BCD2HEX(n)              ((((n)>>4)*10) + ((n)&0x0f))  // 0x13 = 10+3
#define HEX2BCD(n)              (((((n)/10)%10)<<4)  +  ((n)%10))
#define BCD_HIGH(A)         (((A) >> 4 ) & 0x0f)
#define BCD_LOW(A)           ((A) & 0x0f)
#define IS_DNS_CHAR(c)  (((c)!=' ') && ((c)!='!') && ((c)!='$') && ((c)!='&') && ((c)!='?'))
#define BASE_YEAR_2000     2000


typedef enum 
{
	GM_CHANGE_FALSE = -1,
	GM_NO_CHANGE = 0,
	GM_CHANGE_TRUE = 1
}GM_CHANGE_ENUM;

typedef struct
{
	bool state;
	//报警状态保持时�?�?
	U32 true_state_hold_seconds;
	
    //正常状态保持时�?�?
	U32 false_state_hold_seconds;

}StateRecord,*PStateRecord;


//GPS的时间起点是1980.1.6 �?970年到1980.1.6的秒�?
#define SECONDS_FROM_UTC_TO_GPS_START     (315936000+8*3600)

/**
 * Function:   创建util模块
 * Description:创建util模块
 * Input:	   �?
 * Output:	   �?
 * Return:	   GM_SUCCESS——成功；其它错误码——失�?
 * Others:	   使用前必须调�?否则调用其它接口返回失败错误�?
 */
GM_ERRCODE util_create(void);

/**
 * Function:   销毁util模块
 * Description:销毁util模块
 * Input:	   �?
 * Output:	   �?
 * Return:	   GM_SUCCESS——成功；其它错误码——失�?
 * Others:	   
 */
GM_ERRCODE util_destroy(void);


/**
 * Function:   util模块定时处理入口
 * Description:uart模块定时处理入口
 * Input:	   �?
 * Output:	   �?
 * Return:	   GM_SUCCESS——成功；其它错误码——失�?
 * Others:	   
 */
GM_ERRCODE util_timer_proc(void);


/**
 * Function:   
 * Description:change character hex number to real number
 * Input:	   �?
 * Output:	   �?
 * Return:	   �?
 * Others:	   �?
 */
u8 util_chr(u8 x);


/**
 * Function:   
 * Description: change number to character hex number.
 * Input:	   �?
 * Output:	   �?
 * Return:	   �?
 * Others:	   if x is not number, it will be changed as unrecognize_char, normally is 'x' or '0'
 */
u8 util_asc(u8 x, u8 unrecognize_char);


/**
 * Function:   
 * Description: remove a char from pdata
 * Input:	   �?
 * Output:	   �?
 * Return:	   �?
 * Others:	   �?
 */
u16 util_remove_char(u8 *pdata, u16 len,char c);

char util_to_upper(char c);

char util_to_lower(char c);

/**
 * Function:   
 * Description:change pdata string to upper case
 * Input:	   �?
 * Output:	   �?
 * Return:	   �?
 * Others:	   �?
 */
void util_string_upper(u8 *pdata, u16 len);


/**
 * Function:   
 * Description: change pdata string to lower case
 * Input:	   �?
 * Output:	   �?
 * Return:	   �?
 * Others:	   �?
 */
void util_string_lower(u8 *pdata, u16 len);


/**
 * Function:   
 * Description:check whether pdns is valid dns
 * Input:	   pdns  len
 * Output:	   �?
 * Return:	   vaild dns return 1, else retun 0
 * Others:	   �?
 */
u8 util_is_valid_dns(const u8 *pdns, u16 len);

/**
 * Function:   
 * Description:check whether ip is valid ip
 * Input:	   ip    len
 * Output:	   �?
 * Return:	   vaild dns return 1, else retun 0
 * Others:	   �?
 */
u8 util_is_valid_ip(const u8 *ip, u16 len);

/**
 * Function:   
 * Description:check whether ip is internal IP
 * Input:	   ip    len
 * Output:	   �?
 * Return:	   internal IP return true, else retun false
 * Others:	   �?
 */
bool util_is_internal_ip(const u8 *ip, u16 len);

/**
 * Function:   
 * Description: get current time to pdata(bcd format) and time(ST_Time format) based on zone.
 * Input:	   �?
 * Output:	   �?
 * Return:	   success.
 * Others:	   �?
 */
GM_ERRCODE util_get_current_local_time(u8* pdata, ST_Time* time, u8 zone);


/**
 * Function:   
 * Description: change seconds to bcd time.
 * Input:	   sec_time UTC format
 * Output:	   �?
 * Return:	   �?
 * Others:	   �?
 */
void util_utc_sec_to_bcdtime_base2000(time_t sec_time, u8 *bcd, u8 zone);

/**
 * Function:   判断是否是闰�?
 * Description:
 * Input:	   year:�?
 * Output:	   
 * Return:	   true——是闰年；false——不是闰�?
 * Others:	   
 */
bool util_is_leap_year(U16 year);

/**
 * Function:   时间戳转换成数据结构
 * Description:替换time系统函数
 * Input:	   p_time:指向时间戳的指针
 * Output:	   �?
 * Return:	   时间数据结构
 * Others:	   
 */
struct tm util_gmtime(time_t t);

/**
 * Function:   时间数据结构转换成时间戳
 * Description:替换time系统函数
 * Input:	   t:指向时间数据结构的指�?
 * Output:	   �?
 * Return:	   时间�?
 * Others:	   
 */
time_t util_mktime(struct tm* t);

/**
 * Function:   时间�?转换�?时间数据结构
 * Description:替换time系统函数
 * Input:	   srctime:指向时间戳的指针
 * Output:	   �?
 * Return:	   时间数据结构
 * Others:	   
 */
struct tm * util_localtime(const time_t *srctime);

/**
 * Function:   把MTK时间转换为GPS时间
 * Description:
 * Input:	   st_time:MTK时间；leap_sencond——闰秒�?
 * Output:	   p_tow:周内时间（秒）；p_wn:周数（从1980�?�?日开始）
 * Return:	   �?
 * Others:	   GPS时间格式,用周数和周内时表�?GPS的时间起点是1980.1.6
			   GPS时间没有闰秒修正,是连续的时间,而常规时间是经过闰秒修正�?
			   2016年的闰秒修正值是17�?
 */
void util_mtktime_to_gpstime(const ST_Time st_time, const U8 leap_sencond, double* p_tow, U16* p_wn);


/**
 * Function:   mtktime 转换�?tm
 * Description:
 * Input:	   �?
 * Output:	   �?
 * Return:	   Linux时间结构�?
 * Others:	   
 */
void util_mtktime_to_tm(const ST_Time *mtk_t, struct tm *tm_t);

/**
 * Function:   tm 转换�?mtktime 
 * Description:
 * Input:	   �?
 * Output:	   �?
 * Return:	   Linux时间结构�?
 * Others:	   
 */
void util_tm_to_mtktime(const struct tm *tm_t, ST_Time *mtk_t);

/**
 * Function:   获取UTC时间
 * Description:
 * Input:	   �?
 * Output:	   
 * Return:	   Linux时间结构�?
 * Others:	   
 */
time_t util_get_utc_time(void);


/**
 * Function:   获取开机以来的秒数
 * Description:
 * Input:	   �?
 * Output:	   
 * Return:	   开机以来的秒数
 * Others:	   每隔229�?小时54�?6秒会翻转�?
 */
u32 util_clock(void);


/************************************************************************
 * Function :
 * Description :
 *      Unicode符号范围         UTF-8编码方式
 *         十六进制               二进�?
 *      0000 0000-0000 007F | 0xxxxxxx
 *      0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *      0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *      0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

 *      utf8Str -- UTF8 source string
 *      utf8StrLen  -- max utf8 length
 *      unStr  ---  Unicode Str dest
 *      unMaxLen --- Unicode max length
 *      return : ----  实际转化的长�?

 *      把UTF-8转成双字节的UNICODE
 *      E6 96 AD(汉字"�?的UTF-8编码) ---> 0x65AD (即汉�?�?的UNICODE)

 *  Parameter :
 *  Return :
 *  Author:
 *  Date:
************************************************************************/
u16 util_utf8_to_unicode(const u8 *msg, u16 msg_len, u16 *ucs2_msg, u16 max_len);


/************************************************************************
 * Function :
 * Description :
 *   把双字节的UNICODE转成单字节的UNICODE
 *   0x1234 ---> 0x12 , 0x34

 * Parameter :
 * Return :
 * Author:
 * Date:
************************************************************************/
u16 util_ucs2_u16_to_u8(const u16 *input, u16 len, u8 *output);


/************************************************************************
 * Function :
 * Description :
 *   把双字节的UNICODE 字节反过�?
 *   0x1234 ---> 0x3412

 * Parameter :
 * Return :
 * Author:
 * Date:
************************************************************************/
u16 util_ucs2_byte_revert(u8 *pdata, u16 len);


/************************************************************************
 * Function :
 * Description :
 *   long转成ascii字符�?
 *   withZero=1 0x0123456->"0123456"    包括0
 *   withZero=0 0x0123456->"123456"    不包�?
 * Parameter :
 * Return :
 * Author:
 * Date:
************************************************************************/
u8 util_long_to_asc(u32 value, u8 *buffer, u8 with_zero);


/************************************************************************
    Function :
    Description : delete a file
    Parameter : 
    Return : 
    Author:  
    Date: 
************************************************************************/
s32 util_delete_file(const u16 *file);


/************************************************************************
    Function :
    Description : 
        change ucs2 to ascii
    Parameter : 
    Return : 
    Author: 
    Date:  
************************************************************************/
u16 util_ucs2_to_ascii(const u8 *w_in, u8 *a_out, u16 len);

/************************************************************************
 * Function:   将海里转换为公里
 * Description:
 * Input:	   �?
 * Output:	   
 * Return:	   Linux时间结构�?
 * Others:	   
 ************************************************************************/
float util_mile_to_km(const float miles);

/************************************************************************
 * Function:   判断字符是否为数�?
 * Description:
 * Input:	   c:字符
 * Output:	   
 * Return:	   true——是；false——否
 * Others:	   
 ************************************************************************/
bool util_isdigit(const char c);


/************************************************************************
 * Function:   判断字符是否为可打印字符
 * Description:
 * Input:	   c:字符
 * Output:	   
 * Return:	   true——是；false——否
 * Others:	   
 ************************************************************************/
bool util_isprint(const char c);

/************************************************************************
 * Function:   �?0进制数字符串转换成长整型
 * Description:https://baike.baidu.com/item/strtol/6134558
 * Input:	   cp:字符串；
 * Output:	   endptr:是一个传出参�?函数返回时指向后面未被识别的第一个字�?
 * Return:	   true——是；false——否
 * Others:	   
 ************************************************************************/
S32 util_strtol(const char* cp,char** endp);

u32 util_hexstrtoul(const char* cp);


/**
 * Function:   检查状态发生了什么变�?
 * Description:当前状态与历史状态不�?并且保持时间足够认为状态发生了变化并记录为新的历史状�?
 * Input:      current_state:当前状态；p_record:历史状态记录；true_hold_time_threshold:true状态保持时间阈值；false_hold_time_threshold:false状态保持时间阈�?
 * Output:     p_record:历史状态记�?
 * Return:     GM_CHANGE_TRUE——变为true状态；GM_NO_CHANGE——不变；GM_CHANGE_FALSE——变为false状�?
 * Others:      
 */	
GM_CHANGE_ENUM util_check_state_change(bool current_state, PStateRecord p_record, U16 true_hold_time_threshold, U16 false_hold_time_threshold);

/**********************************************************
GSM PDU 7bit字符串解码，�?bit PDU字符串转为ASCII字符�?
"E8329BFD4697D9EC37"----->"hellohello"
pDst：解码后的ASCII字符串目标指�?
pSrc：解码前的PDU 7bit字符串指�?
返回：解码后的ASCII字符串长�?
***********************************************************/
int util_pdu_7bit_decoding(char *pDst,char *pSrc);

/**********************************************************
GSM PDU 7bit编码，由ASCII字符串转为编码后�?bit PDU字符�?
"hellohello"----->"E8329BFD4697D9EC37"
pDst：编码后的目标指�?
pSrc：编码前的ASCII字符串指�?
返回：编码后的字符数据长�?
***********************************************************/
int util_pdu_7bit_encoding(unsigned char *pDst,char *pSrc);


int util_memmem(char * haystack, int haystacklen, char * needle, int needlelen);

#endif

