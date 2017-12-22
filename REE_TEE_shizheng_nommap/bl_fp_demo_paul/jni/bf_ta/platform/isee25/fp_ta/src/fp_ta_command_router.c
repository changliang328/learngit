#include <stdlib.h>
#include "fp_ta_command_router.h"
#include <stdint.h>
#include <ut_sys_type.h>
#include <sys/stat.h>
#include "bf_core.h"
#include "bf_log.h"


//ut_int32_t ut_pf_fp_invoke_command(void *param, void * data ,uint32_t data_length);
//uint32_t fp_ta_invoke_cmd_entry_point(uint32_t *buffer, uint32_t *buff_len);


typedef struct 
{
  uint32_t head1;
  uint32_t head2;
  uint32_t head3;
  uint32_t len;
}beanpod_param_t;

typedef enum {
    	UT_FP_CMD_PRESSURE_TEST = 100,
    	/* add by fido cmd*/
	UT_FIDO_CMD_GET_UVT = 110,
	UT_FIDO_CMD_GET_LAST_WARP_KEY,
	/* add by microtrust ali pay cmd*/
	UT_ALI_PAY_CMD_FP_GET_IDS = 130,
	UT_ALI_PAY_CMD_FP_GET_NAME,
	UT_ALI_PAY_CMD_FP_GET_IDENTIFY_RESULT,
	UT_ALI_PAY_CMD_FP_GET_VERSION,
	UT_ALI_PAY_CMD_FP_ID_EQUATOR,
        /* add by microtrust wechat pay cmd*/
	UT_WECHAT_PAY_CMD_FP_GET_IDENTIFY_RESULT = 150,
	UT_WECHAT_PAY_CMD_FP_GET_SENSOR_NAME,
	UT_WECHAT_PAY_CMD_FP_GET_SENSOR_VERSION,
} ut_fp_cmd_t;

uint32_t fp_ta_invoke_cmd_entry_point(uint32_t *buffer, uint32_t *buff_len){

	BF_LOG("cmd=%d,len=%d",*buffer, *buff_len);

	return bf_core_handler(buffer, buff_len);
}



ut_int32_t ut_pf_fp_invoke_command(void *param, void * data ,uint32_t data_length)
{
	//BF_LOG("PARMMETER IN: data = 0x%08x  data_length = %d\n", *(uint32_t *)data, data_length);
	uint32_t ret = FP_SUCCESS;
	(void)data_length;

	beanpod_param_t *bcmd = (beanpod_param_t*)param;

	if ((NULL == data) || (NULL == param)) {
		//BF_LOG("fp_ta_invoke_cmd_entry_point invalid parameters\n");
		ret =  FP_ERROR_BAD_PARAMS;
	} else if (0 == bcmd->head1 && 0 == bcmd->head2 && 0 == bcmd->head3) { // fp cmd from fp ca
		ret = fp_ta_invoke_cmd_entry_point((uint32_t *)data, &(bcmd->len));
	} 
	else if ((0x01 == bcmd->head1) && (UT_ALI_PAY_CMD_FP_GET_IDENTIFY_RESULT == bcmd->head2)) { // alipay_sec_fp_get_identify_result
		ret = 0;//ifaa_fp_ta_get_last_identify_id((uint8_t *)data, &(bcmd->len));
	} 
	else if ((0x02 == bcmd->head1) && (UT_ALI_PAY_CMD_FP_GET_IDS == bcmd->head2)) { // alipay_sec_fp_get_ids
		ret = 0;//ifaa_fp_ta_get_id_list((uint8_t *)data, &(bcmd->len));
	} 
	else if ((0x02 == bcmd->head1) && (UT_WECHAT_PAY_CMD_FP_GET_IDENTIFY_RESULT == bcmd->head2)) { // wechat interface
		ret = 0;//wechat_fp_ta_get_last_identify_id((uint8_t *)data, &(bcmd->len));
	}
	
	return ret;
}
