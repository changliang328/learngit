#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>

#include "fp_type_define.h"
#include "bf_log.h"
#include <pthread.h>  

static int fpDeviceHandle = 0;
static uint8_t g_cmd_buffer[512 * 1024] = {0};

#define UT_FP_ENTRY_IOCTL_DEVICE "/dev/teei_fp"
#define UT_FP_ENTRY_IOCTL_MAGIC_NO _IO(0x5A777E,0x2)

static pthread_mutex_t tee_mtx;

enum{
	TEST_CA_TO_TA = 0x20,
	TEST_IFAA_GET_IDS,
	TEST_IFAA_GET_LAST_IDENTIFY,
	TEST_WECHAT_GET_IDENTIFY
};


int tee_adapter_init_tzapp(void)
{
	int ret = -1;

	ret = pthread_mutex_init(&tee_mtx, NULL);

	return ret;
}


void tee_adapter_destroy_tzapp(void)
{
	pthread_mutex_destroy(&tee_mtx);
}


static int fp_ca_invoke_command_from_user(void *cmd, int len, uint8_t cmdType)
{
	int ret = 0;
	uint8_t *buffer = (uint8_t*) g_cmd_buffer;
	fp_cmd_header_t *header = (fp_cmd_header_t *) buffer;

	memset(buffer, 0, len + 16);

	header->len = len;
	if (cmd != NULL) {
		memcpy(buffer + sizeof(fp_cmd_header_t), (uint8_t*) cmd, len);
	}
	
	bf_core_handler(buffer + sizeof(fp_cmd_header_t), buffer + 12);
	if (ret == 0) 
	{
		memcpy(cmd, buffer + sizeof(fp_cmd_header_t), *(uint32_t*)(buffer+12));
	} 
	else 
	{
		LOGE("ioctl failed! for errno :%d\n",ret);
	}

	return 0;
}

int tee_adapter_invoke_command(char* buf, unsigned int len)
{
	int ret = -1;

	pthread_mutex_lock(&tee_mtx);
	ret = fp_ca_invoke_command_from_user(buf, len, TEST_CA_TO_TA);
	pthread_mutex_unlock(&tee_mtx);
	if (ret != 0) 
	{
		LOGE("fp_ca_invoke_command_from_user failed!");
		return -2;	
	}

	return ret;
}


int ts_ca_get_key(void *key_data, uint32_t *key_len)
{
	LOGD("ts_ca_get_key no complete!");
	(void)key_data;
	(void)key_len;
	
	return 0;
}

