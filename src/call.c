#include "call.h"
#include "gm_record.h"
#include "log_service.h"
#include "utility.h"
#include "gsm.h"
#include "gm_callback.h"

static GM_ERRCODE gm_call_status_call_back(void* call_status);


static GM_ERRCODE gm_call_status_call_back(void* call_status)
{
    int* status =(int*) call_status;
        
    switch(*status)
    {
        case 0:
            //phone calling
            break;
        case 1:
            //call OK
            break;
        case 2:
            //call hang 
            break;
    }

    return GM_SUCCESS;
}



static GM_ERRCODE gm_call_receive_call_back(void* evt)
{
    u8* phone_number =(u8*) evt;

    LOG(INFO, "clock(%d) gm_call_receive_call_back RING:%.20s", util_clock(), phone_number);
	
    GM_CallAnswer((PsFuncPtr)gm_call_status_call_back);
	GM_Set_Mic_Volume(2,6);
    return GM_SUCCESS;
}



GM_ERRCODE gm_make_phone_call(const u8 *dist_number)
{
    if (NULL == dist_number)
    {
        return GM_PARAM_ERROR;
    }

    if ((GSM_CREG_REGISTER_LOCAL != gsm_get_creg_state()) && (GSM_CREG_REGISTER_ROAM != gsm_get_creg_state()))
    {
        return GM_ERROR_STATUS;
    }
    
    GM_MakeCall((u8 *)dist_number, (PsFuncPtr)gm_call_status_call_back);

    return GM_SUCCESS;
}



GM_ERRCODE call_create(void)
{
    GM_RegisterCallBack(GM_CB_CALL_RECEIVE, (u32)gm_call_receive_call_back);
    
    return GM_SUCCESS;
}


GM_ERRCODE call_destroy(void)
{
    GM_RegisterCallBack(GM_CB_CALL_RECEIVE, 0);
    
    return GM_SUCCESS;
}


