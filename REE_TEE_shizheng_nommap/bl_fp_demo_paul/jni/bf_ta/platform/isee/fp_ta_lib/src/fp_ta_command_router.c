#include <stdlib.h>
#include "fp_ta_command_router.h"
#include <ut_fp_ta.h>
#include <contrib/libuTfp/ut_fp_def.h>
#include <contrib/libuTfp/wechat_fp_api.h>

#include "fp_spi.h"
#include "bf_core.h"
#include "bf_log.h"


uint32_t   fp_ta_invoke_cmd_entry_point(uint32_t *buffer, uint32_t *buff_len);


typedef struct 
{
  uint32_t head1;
  uint32_t head2;
  uint32_t head3;
  uint32_t len;
}beanpod_param_t;

void init_main(){

}

uint32_t fp_ta_invoke_cmd_entry_point(uint32_t *buffer, uint32_t *buff_len){

	BF_LOG("cmd=%d,len=%d",*buffer, *buff_len);

	return bf_core_handler(buffer, buff_len);
}

TEE_Result ut_fp_invokeCommand(void *param, void * data, uint32_t param_length) {
	uint32_t ret = 0;
	beanpod_param_t *bcmd = (beanpod_param_t*)param;
	if ((NULL == data) || (NULL == param)) {
		BF_LOG("fp_ta_invoke_cmd_entry_point invalid parameters");
		ret =  -1;
	} else if (0 == bcmd->head1 && 0 == bcmd->head2 && 0 == bcmd->head3) { // fp cmd from fp ca
		ret = fp_ta_invoke_cmd_entry_point((uint32_t *)data, &(bcmd->len));
	} 

	return ret;
}

