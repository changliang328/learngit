#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>

//#include "fp_type_define.h"
#include "bf_log.h"
#include <pthread.h>  

static int fpDeviceHandle = 0;
static uint8_t g_cmd_buffer[512 * 1024] = {0};

#define UT_FP_ENTRY_IOCTL_DEVICE "/dev/teei_fp"
#define UT_FP_ENTRY_IOCTL_MAGIC_NO _IO(0x5A777E,0x2)


typedef struct {
    uint32_t head1;
    uint32_t head2;
    uint32_t head3;
    uint32_t len;
} fp_cmd_header_t;

int ree_adapter_init_tzapp(void)
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

	return ret;
}


void ree_adapter_destroy_tzapp(void)
{
	if (fpDeviceHandle != 0) 
	{
		close(fpDeviceHandle);
		fpDeviceHandle = 0;
	}

}


int ree_ca_invoke_command_from_user(void *cmd, int len)
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

	header->len = len;
	memcpy(buffer + sizeof(fp_cmd_header_t), (uint8_t*)cmd, len);

	//InvokeCommand
	if (fpDeviceHandle != 0)
	{			BF_LOG("++++");
		ret = ioctl(fpDeviceHandle, UT_FP_ENTRY_IOCTL_MAGIC_NO, buffer);
		if (ret == 0) 
		{
			BF_LOG("++++");
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

