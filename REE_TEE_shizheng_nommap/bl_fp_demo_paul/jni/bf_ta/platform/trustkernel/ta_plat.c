#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <tee_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_api_types.h>
#include <tee_ta_api.h>
#include "bf_log.h"
#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"


int32_t tz_fd_open(const char * pathname, int flags);
int32_t tz_fd_lseek(int fildes, int32_t offset, int whence);
int32_t tz_fd_read(int filedes, void *buf, size_t nbytes);


#define HMAC_SHA256_SIZE	(32)
#define HMAC_SHA256_KEY_SIZE	(256)

#define FP_TA_ERROR_PARAMETER (22)
#define FP_TA_ERROR_IO_NO_FILE (4)
#define FP_TA_ERROR_IO (3)
bf_tee_platform_t *gpbf_plat = NULL;
static  TEE_SPIConfig spi_chip_config = {
	.setuptime = 10,
	.holdtime = 10,
	.high_time = 7,
	.low_time =  7,
	.cs_idletime = 20, //10,
	//.ulthgh_thrsh = 0,

	.cpol = 0,
	.cpha = 0,

	.rx_mlsb = 1,
	.tx_mlsb = 1,

	.tx_endian = 0,
	.rx_endian = 0,

	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};



static int32_t tz_spi_open(void)
{
	return SUCCESS;
}

static int32_t tz_spi_close(void)
{
	return SUCCESS;
}

static int32_t tz_spi_setup(uint32_t speed)
{
	switch(speed)
	{
		case 1:
			spi_chip_config.high_time = 50;
			spi_chip_config.low_time = 50;
			break;
		case 2:
			spi_chip_config.high_time = 25;
			spi_chip_config.low_time = 25;
			break;
		case 3:
			spi_chip_config.high_time = 17;
			spi_chip_config.low_time = 17;
			break;
		case 4: 
			spi_chip_config.high_time = 13;
			spi_chip_config.low_time = 13;
			break;
		case 5: 
			spi_chip_config.high_time = 10;
			spi_chip_config.low_time = 10;
			break;
		case 6: 
			spi_chip_config.high_time = 8;
			spi_chip_config.low_time = 8;
			break;
		default:
			spi_chip_config.high_time = 50;
			spi_chip_config.low_time = 50;
			break;
	}

	return SUCCESS;
}

int32_t tz_fd_open(const char * pathname, int flags)
{
	TEE_Result result = -1;
	TEE_ObjectHandle handle = NULL;
	BF_LOG("+++++");
	result = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,(void *) pathname, strlen(pathname),
			TEE_DATA_FLAG_ACCESS_READ|TEE_DATA_FLAG_ACCESS_WRITE, &handle);
	if (result != TEE_SUCCESS) {
		if(result == TEE_ERROR_ITEM_NOT_FOUND) {
			BF_LOG("No PersistentObject");
		}
		BF_LOG("TEE_OpenPersistentObject failed: 0x%08x", result);             
		return -1;   
	}
	flags=result;
	return flags;
}

int32_t tz_fd_lseek(int fildes, int32_t offset, int whence)
{
	BF_LOG("+++++");
	TEE_SeekObjectData( (TEE_ObjectHandle)fildes,offset, whence);
	return 0;
}

int32_t tz_fd_read(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return 0;
}

int32_t tz_fd_write(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return 0;
}


static int32_t tz_spi_init(void)
{
	int rc;
	if ((rc = TEE_SPISetup2(0, &spi_chip_config)) != TEE_SUCCESS) {
		BF_LOG("inside %s TEE_SPISetup2(..) fail, rc = %d. \n", __func__, rc);
		rc = -1;
		return rc;
	}
	return 0;
}

static int32_t tz_spi_write_read(uint8_t* tx, uint32_t tx_bytes, uint8_t* rx, uint32_t rx_bytes)

{
	TEE_Result r;
	tz_spi_init();
	uint32_t nxferbytes = (tx_bytes > rx_bytes)?tx_bytes:rx_bytes;
	
	if ((r = TEE_SPIWriteRead(0, tx, nxferbytes)) != TEE_SUCCESS) {
		BF_LOG("inside %s TEE_SPIWriteRead(..) fail, r = %d. \n", __func__, r);
		r = -1;
		goto exit;
	}

exit:
	return r;
}

static int32_t tz_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size) {
    TEE_Result r;
    TEE_ObjectHandle h;
    uint32_t read_count;
    uint32_t flags;


    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META;

	r = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, (void *) path, strlen(path),flags, &h);

    if (r != TEE_SUCCESS) {
        BF_LOG("TEE_OpenPersistentObject failed with 0x%08x\n", r);
        return FAIL;
    }

    if (offset != 0) {
        if ((r = TEE_SeekObjectData(h, offset, TEE_DATA_SEEK_SET)) != TEE_SUCCESS) {
            BF_LOG("TEE_SeekDataObject failed with 0x%x\n", r);
            return FAIL;
        }
    }

    r = TEE_ReadObjectData(h, buf, size, &read_count);

    if (r != TEE_SUCCESS) {
        BF_LOG("TEE_ReadObjectData failed with 0x%08x\n", r);
    }

    TEE_CloseObject(h);

    if (read_count != size) {
        BF_LOG("Requested size for reading %s does not match read bytes %d",
                path, size);
        return FAIL;
	}

    return  size;
}

static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size) {
    TEE_Result r;
    TEE_ObjectHandle h;
    uint32_t flags;

    BF_LOG("path %s buf %p size %u\n", path, buf, size);
 
    if (path == NULL) {
        BF_LOG("Path is NULL\n");
        return FAIL;
    }

    if (buf == NULL && size) {
        BF_LOG("Write buffer is NULL while size = %u\n", size);
        return FAIL;
    }

    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META;

    r = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            (void *) path, strlen(path),
            flags, &h);

    if (r != TEE_SUCCESS) {
        if (r != TEE_ERROR_ITEM_NOT_FOUND) {
            BF_LOG("TEE_OpenPersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }

        BF_LOG("TEE_CreatePersistentObject path %s\n", path);

        if ((r = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                        (void *) path, strlen(path), flags,
                        TEE_HANDLE_NULL, NULL, 0, &h)) != TEE_SUCCESS) {
            BF_LOG("TEE_CreatePersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }
    }

    if (offset != 0) {
        if ((r = TEE_SeekObjectData(h, offset, TEE_DATA_SEEK_SET)) != TEE_SUCCESS) {
            BF_LOG("TEE_SeekDataObject failed with 0x%x\n", r);
            return FAIL;
        }
    }

    if (buf) {

        r = TEE_WriteObjectData(h, buf, size);

        if (r != TEE_SUCCESS) {
            BF_LOG("TEE_WriteObjectData failed with 0x%08x\n", r);
        }
    }

    TEE_CloseObject(h);
    return  size;
}


static void* tz_malloc(uint64_t size) {
	return TEE_Malloc(size, 0);
}
static void tz_free(void* ptr) {
	TEE_Free(ptr);
}

static void *tz_memcpy(void *dest, void *src, uint32_t n){
	TEE_MemMove(dest,src,n);
	return dest;
}
static void *tz_memset(void *s, uint32_t c, uint32_t n){
	TEE_MemFill(s,c,n);
	return s;
}

static int32_t tz_memcmp(const void *buffer1, const void *buffer2, uint32_t size)
{
	return TEE_MemCompare(buffer1, buffer2, size);
}

static void tz_msleep(uint32_t ms)
{
	TEE_Wait(ms);
}

/* generate random of `data_size` bytes and store in `data` */
static int32_t tz_get_random(uint8_t *data, uint32_t data_size)
{
	if (data_size == 0)
		return (int32_t) TEE_ERROR_BAD_PARAMETERS;
	TEE_GenerateRandom(data, data_size);
	return (int32_t) TEE_SUCCESS;
}


static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{
	TEE_Result res;
	if (key == NULL) {
		BF_LOG("NULL key\n");
		return -1;
	}
	if ((res = TEE_GetBootSeed(key, (HMAC_SHA256_KEY_SIZE >> 3))) != TEE_SUCCESS) {
		BF_LOG("TEE_GetBootSeed failed with 0x%08x\n", res);
		return -1;
	}
	return 0;
}

static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key,uint32_t key_len,uint8_t *hmac)
{
	TEE_Result r = 0;
	TEE_OperationHandle handle;
	TEE_ObjectHandle key_handle;
	TEE_Attribute secret_value;

	uint32_t hmac_size = HMAC_SHA256_SIZE;

	(void)key_len;
	if ((r = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, HMAC_SHA256_KEY_SIZE, &key_handle))  != TEE_SUCCESS)
	{
		BF_LOG("TEE_AllocateTransientObject failed with %08x\n", r);
		return FAIL;
	}

	TEE_InitRefAttribute(&secret_value, TEE_ATTR_SECRET_VALUE, (void *) key, (HMAC_SHA256_KEY_SIZE >> 3));

	if ((r = TEE_PopulateTransientObject(key_handle,&secret_value, 1)) != TEE_SUCCESS)
	{
		BF_LOG("TEE_PopulateTransientObject failed with 0x%08x\n", r);
		goto error_free_obj;
	}

	if ((r = TEE_AllocateOperation(&handle,TEE_ALG_HMAC_SHA256, TEE_MODE_MAC, 256)) != TEE_SUCCESS)
	{
		BF_LOG("TEE_AllocateOperation failed with %08x\n", r);

		goto error_free_obj;
	}

	if ((r = TEE_SetOperationKey(handle, key_handle)) != TEE_SUCCESS)
	{
		BF_LOG("TEE_SetOperationKey failed with %08x\n", r);
		goto error_free_operation;
	}

	TEE_MACInit(handle, NULL, 0);

	if ((r = TEE_MACComputeFinal(handle, (void *) data, data_size, (void *) hmac, &hmac_size))!= TEE_SUCCESS)
	{
		BF_LOG("TEE_MACComputeFinal failed with %08x\n", r);
	}

error_free_operation:
	TEE_FreeOperation(handle);

error_free_obj:

	TEE_FreeTransientObject(key_handle);

	return r;
}


static uint32_t convert_x(uint32_t x)
{
	return (x >> 24) | ((x >> 8) & 0xff00U) | ((x << 8) & 0xff0000U) | (x << 24);
}

static uint64_t htobe64(uint64_t x)
{
	return ((convert_x(x) + 0ULL) << 32) | convert_x(x >> 32);
}

static uint64_t tz_get_time(void)
{
	TEE_Time time;
	uint64_t time_stamp;
	TEE_GetSystemTime(&time);
	time_stamp = (uint64_t) time.seconds * 1000 + time.millis;
	return time_stamp;//htobe64(time_stamp);      
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
	.plat_fd_lseek = tz_fd_lseek,
	//.plat_fd_open = tz_fd_open,
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
