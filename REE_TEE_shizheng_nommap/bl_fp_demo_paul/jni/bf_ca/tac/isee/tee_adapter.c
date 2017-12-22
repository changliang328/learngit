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

	if (fpDeviceHandle == 0)
		fpDeviceHandle = open(UT_FP_ENTRY_IOCTL_DEVICE, O_RDWR);

	if (fpDeviceHandle <= 0)
	{
		ret = -1;
		fpDeviceHandle = 0;
		LOGE("open %s failed! for %s",UT_FP_ENTRY_IOCTL_DEVICE,strerror(errno));
	}
	else
	{
		ret = 0;
		LOGD("open %s success!",UT_FP_ENTRY_IOCTL_DEVICE);
	}

	pthread_mutex_init(&tee_mtx, NULL);

	return ret;
}


void tee_adapter_destroy_tzapp(void)
{
	if (fpDeviceHandle != 0) 
	{
		close(fpDeviceHandle);
		fpDeviceHandle = 0;
	}

	pthread_mutex_destroy(&tee_mtx);
}


static int fp_ca_invoke_command_from_user(void *cmd, int len, uint8_t cmdType)
{
	uint8_t* buffer = NULL;
	int  ret = -1;
	fp_cmd_header_t* header = NULL;

	if (NULL == cmd || len == 0)
	{
		LOGE("bad parameter! cmd is NULL or len is 0");
		return (-EINVAL);
	}

	buffer = (uint8_t*)g_cmd_buffer;
	header = (fp_cmd_header_t*)buffer;

	memset(buffer, 0, len + sizeof(fp_cmd_header_t));

	switch(cmdType)
	{
		case TEST_CA_TO_TA:
			break;

		case TEST_WECHAT_GET_IDENTIFY:
			header->head1 = FP_USER_OPERATION_ID;
			header->head2 = 150;
			break;

		case TEST_IFAA_GET_IDS:
			header->head1 = FP_USER_OPERATION_ID;
			header->head2 = 130;
			break;

		case TEST_IFAA_GET_LAST_IDENTIFY:
			header->head1 = FP_OPERATION_ID;
			header->head2 = 132;
			break;

		default:
			LOGD("valid command");
			break;
	} 

	header->len = len;
	memcpy(buffer + sizeof(fp_cmd_header_t), (uint8_t*)cmd, len);

	//InvokeCommand
	if (fpDeviceHandle != 0)
	{
		ret = ioctl(fpDeviceHandle, UT_FP_ENTRY_IOCTL_MAGIC_NO, buffer);
		if (ret == 0) 
		{
			memcpy(cmd, buffer + sizeof(fp_cmd_header_t), *(uint32_t*)(buffer+12));
		} 
		else 
		{
			ret = -1;
			LOGE("ioctl failed! for %s",strerror(errno));
		}

	}
	return ret;	
}

int tee_adapter_invoke_command(char* buf, unsigned int len)
{
	int ret = -1;

	if (buf == NULL || len == 0)
	{
		LOGE("bad parameter! buf is NULL or len is 0");
		return -1;
	}

	pthread_mutex_lock(&tee_mtx);
	ret = fp_ca_invoke_command_from_user(buf, len, TEST_CA_TO_TA);
	pthread_mutex_unlock(&tee_mtx);
	if (ret != 0) 
	{
		LOGE("fp_ca_invoke_command_from_user failed!");
		return -2;	
	}

	return 0;
}


int ts_ca_get_key(void *key_data, uint32_t *key_len)
{
	LOGD("ts_ca_get_key no complete!");
	(void)key_data;
	(void)key_len;
	
	return 0;
}

