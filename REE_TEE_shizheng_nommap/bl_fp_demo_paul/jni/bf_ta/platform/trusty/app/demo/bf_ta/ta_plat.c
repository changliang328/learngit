

#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"
#include "bf_log.h"

#include <openssl/hmac.h>
#include <lib/keymaster/keymaster.h>
#include <malloc.h>
#include <lib/rng/trusty_rng.h>
#include <sprd_pal_fp_default.h>
#include <io_device_def.h>
#include <errno.h>
#include <lib/storage/storage.h>

bf_tee_platform_t *gpbf_plat = NULL;
struct WRITE_THEN_READ_STR wr;

static int32_t tz_spi_open(void)
{
	//trusty should be empty here
	return SUCCESS;
}



static int32_t tz_spi_close(void)
{
	//trusty should be empty here
	return SUCCESS;
}



static int32_t tz_spi_setup(uint32_t speed)
{
	switch(speed)
	{
		case 1:
			wr.max_speed_hz =6000000;
			break;
		default:
			wr.max_speed_hz =3000000;
			break;
	}

	return SUCCESS;
}




static int32_t tz_spi_write_read(uint8_t* in, uint32_t in_size, uint8_t* out, uint32_t out_size) 
{
	int ret;
	wr.max_speed_hz =3000000;
	wr.chip_select =0;
	wr.mode=0;
	wr.bits_per_word=8;
	wr.number=0;
	wr.len=(in_size+out_size);
	wr.rxbuf=out;
	wr.txbuf=in;
	wr.debug=1;
	//BF_LOG("%s +%d [%s]  before ioctl\n",__FILE__,__LINE__,__func__);
	ret =ioctl(IO_DEVICE_FP,SPI_WRITE_AND_READ,&wr);
	//BF_LOG("%s +%d [%s]  after ioctl\n",__FILE__,__LINE__,__func__);
}

static int32_t tz_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size)
{
	storage_session_t session;
	int retval = 0;
	int rc = -1;
	char file_name[50]={};
	file_handle_t handle;
	if(path==NULL){
		return FAIL;
	}
	for(int i=0;*(path+i)!='\0';i++)
	{
		file_name[i]=*(path+i);
	}
	BF_LOG("%s[%s]  start read file :  %s \n",__FILE__,__func__,file_name);
	rc = storage_open_session(&session,STORAGE_CLIENT_TD_PORT);
	if(rc < 0){
		BF_LOG("%s +%d [%s] Error fail to open storage session\n",__FILE__,__LINE__,__func__);
		return FAIL;
	}
	retval = storage_open_file(session,&handle,file_name,STORAGE_FILE_OPEN_CREATE,0);
	if (retval < 0) {
		BF_LOG("%s +%d [%s] storage_open_file failed! \n",__FILE__,__LINE__,__func__);
		goto exit2;
	} else {	
		retval = storage_read(handle,offset,buf,size*sizeof(uint32_t));
	}

	goto exit1;
exit1:
	storage_close_file(handle);
	storage_end_transaction(session,true);
	storage_close_session(session);

	if(rc<0){
		BF_LOG("%d [%s] read for file failed!\n", __LINE__,__func__);
	}
	return SUCCESS;
exit2:
	storage_close_session(session);
	return FAIL;
}




static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size)
{

	storage_session_t session;
	int retval = 0;
	int rc = 0;
	file_handle_t handle;

	rc=storage_open_session(&session,STORAGE_CLIENT_TD_PORT);
	if(rc<0){
		BF_LOG("%s +%d [%s] storage_open_session failed!  \n",__FILE__,__LINE__,__func__);
		return FAIL;
	}
	retval = storage_open_file(session,&handle,path,STORAGE_FILE_OPEN_CREATE,0);
	if (retval < 0) {
		BF_LOG("%s +%d [%s] storage_open_file failed! \n",__FILE__,__LINE__,__func__);
		goto exit2;
	}
	rc=storage_write(handle,offset,buf,size*sizeof(char), STORAGE_OP_COMPLETE);

	goto exit1;

exit1:
	storage_close_file(handle);
	storage_end_transaction(session,true);
	storage_close_session(session);
	if(rc<0){
		BF_LOG("%d [%s]storage_write fail!\n", __LINE__,__func__);
	}
	return SUCCESS;
exit2:
	storage_close_session(session);
	BF_LOG("%d [%s] storage_open_session fail!\n", __LINE__,__func__);
	return FAIL;

}



static void* tz_malloc(uint64_t size)
{
	return malloc(size);
}

static void tz_free(void* ptr)
{
	free(ptr);
}

static void *tz_memcpy(void *dest, void *src, uint32_t n)
{
	memcpy(dest,src,n);
	return dest;
}

static void *tz_memset(void *s, uint32_t c, uint32_t n)
{
	memset(s,c,n);
	return s;
}

static int32_t tz_memcmp(const void *buffer1, const void *buffer2, uint32_t size)
{
	return memcmp(buffer1, buffer2, size);
}


static void tz_msleep(uint32_t ms)
{
	nanosleep(0,0,ms*1000*1000);
}


static int32_t tz_get_random(uint8_t *data, uint32_t data_size)
{	
	trusty_rng_secure_rand(data,data_size);
	return 0;
}


static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{
	long rc=keymaster_open();
	if(rc<0)
		return false;
	keymaster_session_t session=(keymaster_session_t)rc;
	rc=keymaster_get_auth_token_key(session,key,&key_len);
	keymaster_close(session);
	return 0;
}


static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint32_t key_len, uint8_t *hmac)
{
	uint32_t  hmac_length =0;
	HMAC(EVP_sha256(),key,key_len,data,data_size,hmac,&hmac_length);
	BF_LOG("hmac_length =%d \n",hmac_length); 
	return 0;
}

static uint64_t tz_get_time(void)
{
	int rc;
	int64_t million_seconds=0;
	rc=gettime(0,0,&million_seconds);
	if (rc != 0) {
		BF_LOG("%s +%d [%s] get system time failed!\n",__FILE__,__LINE__,__func__);
		return -1; 
	}
	return htobe64(million_seconds/1000/1000);
}


static int32_t tz_set_key(uint8_t *key_buf, uint32_t key_len)
{
	(void)key_buf;
	(void)key_len;

	return 0;
}


struct plat_func g_plat_func = {
	.plat_spi_open = tz_spi_open,
	.plat_spi_close = tz_spi_close,
	.plat_spi_setup = tz_spi_setup,
	.plat_spi_write_read = tz_spi_write_read,
	//.plat_fd_open = tz_fd_open,
	//.plat_fd_lseek = tz_fd_lseek,
	//.plat_fd_read = tz_fd_read,
	//.plat_fd_write = tz_fd_write,
	.plat_fs_read = tz_fs_read,
	.plat_fs_write = tz_fs_write,
	.plat_malloc = tz_malloc,
	.plat_free = tz_free,
	.plat_memcpy = tz_memcpy,
	.plat_memset = tz_memset,
	.plat_memcmp = tz_memcmp,
	.plat_msleep = tz_msleep,
	.plat_get_random = tz_get_random,
	.plat_hmac_key = tz_get_hmac_key,
	.plat_hmac_sha256 = tz_hmac_sha256,
	.plat_get_time = tz_get_time,
	.plat_set_key = tz_set_key,
};



struct plat_func *bf_get_plat_func(bf_tee_platform_t *pbf_plat)
{
	gpbf_plat = pbf_plat;
	return &g_plat_func;
}

