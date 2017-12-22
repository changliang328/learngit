#include <tee_client_api.h>

#include "bf_log.h"
#include <pthread.h>  

#define TEE_NORMAL_CMD 9527

static pthread_mutex_t tee_mtx;
static TEEC_Context context;
static TEEC_Session session;


int tee_adapter_init_tzapp(void)
{
	TEEC_Result ret = -1;

	TEEC_UUID uuid = { 0xfdc4725d, 0x89e4, 0x7829, \
	{ 0x0f, 0xc3, 0xf3, 0x73, 0x90, 0x87, 0xb8, 0xa2 } };

	CF_ENTRY();

	ret = TEEC_InitializeContext(NULL,&context);
	if (ret != TEEC_SUCCESS) 
	{
		LOGE("TEEC_InitializeContext failed! ret = 0x%x",ret);
		return -2;
	}

	ret = TEEC_OpenSession(
			&context,
			&session,
			&uuid,
			TEEC_LOGIN_PUBLIC,
			NULL,
			NULL,
			NULL);
	if (ret != TEEC_SUCCESS)
	{
		LOGE("TEEC_OpenSession failed!  = 0x%x",ret);
		TEEC_FinalizeContext(&context);
		return -2;
	}

	pthread_mutex_init(&tee_mtx, NULL);

	CF_EXIT();
	return 0;
}


void tee_adapter_destroy_tzapp(void)
{
	TEEC_FinalizeContext(&context);
	TEEC_CloseSession(&session);
	pthread_mutex_destroy(&tee_mtx);

}


int tee_adapter_invoke_command(char* buf, unsigned int len)
{
	TEEC_Result ret = -1;
	TEEC_Operation op;

	CF_ENTRY();

	if (buf == NULL || len == 0)
	{
		LOGE("bad parameter");
		return -2;
	}

	op.params[0].tmpref.buffer = buf;
	op.params[0].tmpref.size = len;
	op.started = 1;
	op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_MEMREF_TEMP_INOUT,
			TEEC_NONE,
			TEEC_NONE,
			TEEC_NONE);

	pthread_mutex_lock(&tee_mtx);
	ret = TEEC_InvokeCommand(&session,TEE_NORMAL_CMD, &op, NULL);
	pthread_mutex_unlock(&tee_mtx);

	if (ret != TEEC_SUCCESS) 
	{
		LOGE("TEEC_InvokeCommand failed! ret = 0x%x",ret);
		return -2;
	}

	CF_EXIT();

	return ret;
}


int ts_ca_get_key(void *key_data, uint32_t *key_len)
{
	LOGD("ts_ca_get_key no complete!");
	(void)key_data;
	(void)key_len;
	
	return 0;
}

