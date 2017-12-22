#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <tee_api.h>
#include <tee_api_types.h>
#include <tee_internal_api_extensions.h>
#include <rsee_spi.h>
#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"
#include "bf_log.h"


#define HMAC_SHA256_SIZE	(32)
#define HMAC_SHA256_KEY_SIZE	(256)
//key master  param
#define TA_KEYMASTER_UUID { 0xb657ba17, 0xb3b3, 0x47f5, \
			{ 0xb3, 0x89, 0x6b, 0x53, 0xf3, 0x0c, 0x0b, 0xaf} }
#define ACCESS_KM_GETKEY_CMD	100
#define HMAC_SHA_256_KEY_SIZE  32

static uint8_t password_key_[HMAC_SHA_256_KEY_SIZE];   
static int32_t spi_dev_fd = -1;
bf_tee_platform_t *gpbf_plat = NULL;

static rsee_spi_config_t chip_config = {
    .setuptime = 7,
    .holdtime = 7,
    .high_time = 50,
    .low_time = 50,
    .cs_idletime = 3,
    .ulthgh_thrsh = 0,
    .cpol = RSEE_SPI_CPOL_0,
    .cpha = RSEE_SPI_CPHA_0,

    .rx_mlsb = RSEE_SPI_MSB,  //1
    .tx_mlsb = RSEE_SPI_MSB,  //1

    .tx_endian = RSEE_SPI_LENDIAN, //0
    .rx_endian = RSEE_SPI_LENDIAN, //0

    .com_mod = RSEE_SPI_FIFO_TRANSFER, //0
    .pause = RSEE_SPI_PAUSE_MODE_DISABLE, //0
    .finish_intr = RSEE_SPI_FINISH_INTR_EN, //1
    .deassert = RSEE_SPI_DEASSERT_DISABLE, //0
    .ulthigh = RSEE_SPI_ULTRA_HIGH_DISABLE, //0
    .tckdly = RSEE_SPI_TICK_DLY0, //0
};

static int32_t tz_spi_open(void)
{
	if (spi_dev_fd < 0) {
		spi_dev_fd = rsee_spi_open("spi0");

		if (spi_dev_fd < 0) 
		{
			BF_LOG("spi open fail, rc = %d\n", spi_dev_fd);
			return FAIL;
		}
    }
	else
	{
		BF_LOG("spi has beed opened! no need to open again! spi_dev_fd = %d\n", spi_dev_fd);
	}
    return TEE_SUCCESS;
}

static int32_t tz_spi_close(void)
{
    int rc = -1;

	if (spi_dev_fd > 0)
	{
		rc = rsee_spi_close(spi_dev_fd);
		if (rc != 0)
		{
			BF_LOG("spi close fail, rc = %d\n", rc);
			return FAIL;
		}
			spi_dev_fd = -1;
	}
	else
	{
		BF_LOG("spi is not opened! no need to close! spi_dev_fd = %d\n", spi_dev_fd);
    }

    return TEE_SUCCESS;
}

static int32_t tz_spi_setup(uint32_t speed)
{
    switch(speed)
    {
    case 1:
        chip_config.high_time = 50;
        chip_config.low_time = 50;
        break;

    default:
        chip_config.high_time = 50;
        chip_config.low_time = 50;
        break;
    }

    return SUCCESS;
}


static int32_t tz_spi_write_read(uint8_t* in, uint32_t in_size, uint8_t* out, uint32_t out_size) 
{
    TEE_Result r = 0;
    int rc = -1;
    unsigned char *buf = NULL;

    buf = TEE_Malloc(in_size + out_size, 0);
    if (NULL == buf)
    {
        BF_LOG("TEE_Malloc(..) fail\n");
        rc = FAIL;
        goto exit;
    }
    TEE_MemFill(buf, 0, in_size + out_size);
    TEE_MemMove(buf, in, in_size);
    
    if (in_size + out_size > 32)
    {
        chip_config.com_mod = RSEE_SPI_DMA_TRANSFER;
    }
    else
    {
        chip_config.com_mod = RSEE_SPI_FIFO_TRANSFER;
    }


    if ((r = rsee_spi_full_duplex_transfer(spi_dev_fd, &chip_config,  buf, in_size + out_size, buf, in_size + out_size)) != TEE_SUCCESS)
    {
        BF_LOG("rsee_spi_full_duplex(..) fail, rc = %d\n", rc);
        rc = FAIL;
        goto exit;
    }
    rc = SUCCESS;
    
    if (out != NULL && out_size != 0)
    {
        TEE_MemMove(out, buf + in_size, out_size);
    }
    
exit:

    if (NULL != buf)
    {
        TEE_Free(buf);
        buf = NULL;
    }

    return rc;
}

static int32_t tz_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size)
{
    TEE_Result r = 0;
    TEE_ObjectHandle h;
    uint32_t read_count = 0;
    uint32_t flags = 0;


    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META;

    r = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            (void *) path, strlen(path),
            flags, &h);

    if (r != TEE_SUCCESS)
    {
        if (r != TEE_ERROR_ITEM_NOT_FOUND)
        {
            BF_LOG("TEE_OpenPersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }

        BF_LOG("TEE_CreatePersistentObject path %s\n", path);

        if ((r = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                        (void *) path, strlen(path), flags,
                        TEE_HANDLE_NULL, NULL, 0, &h)) != TEE_SUCCESS)
        {
            BF_LOG("TEE_CreatePersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }
    }

    if (offset != 0)
    {
        if ((r = TEE_SeekObjectData(h, offset, TEE_DATA_SEEK_SET)) != TEE_SUCCESS)
        {
            BF_LOG("TEE_SeekDataObject failed with 0x%x\n", r);
            return FAIL;
        }
    }

    r = TEE_ReadObjectData(h, buf, size, &read_count);

    if (r != TEE_SUCCESS)
    {
        BF_LOG("TEE_ReadObjectData failed with 0x%08x\n", r);
    }

    TEE_CloseObject(h);

    if (read_count != size)
    {
        BF_LOG("Requested size for reading %s does not match read bytes %d",
                path, size);
        return SUCCESS;
    }

    return SUCCESS;
}

static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size)
{
    TEE_Result r = 0;
    TEE_ObjectHandle h;
    uint32_t flags = -1;

    BF_LOG("path %s buf %p size %u\n", path, buf, size);

    if (path == NULL)
    {
        BF_LOG("Path is NULL\n");
        return FAIL;
    }

    if (buf == NULL && size)
    {
        BF_LOG("Write buffer is NULL while size = %u\n", size);
        return FAIL;
    }

    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META;

    r = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
            (void *) path, strlen(path),
            flags, &h);

    if (r != TEE_SUCCESS)
    {
        if (r != TEE_ERROR_ITEM_NOT_FOUND)
        {
            BF_LOG("TEE_OpenPersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }

        BF_LOG("TEE_CreatePersistentObject path %s\n", path);

        if ((r = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                        (void *) path, strlen(path), flags,
                        TEE_HANDLE_NULL, NULL, 0, &h)) != TEE_SUCCESS)
        {
            BF_LOG("TEE_CreatePersistentObject failed with 0x%08x\n", r);
            return FAIL;
        }
    }

    if (offset != 0)
    {
        if ((r = TEE_SeekObjectData(h, offset, TEE_DATA_SEEK_SET)) != TEE_SUCCESS)
        {
            BF_LOG("TEE_SeekDataObject failed with 0x%x\n", r);
            return FAIL;
        }
    }

    if (buf)
    {

        r = TEE_WriteObjectData(h, buf, size);

        if (r != TEE_SUCCESS)
        {
            BF_LOG("TEE_WriteObjectData failed with 0x%08x\n", r);
        }
    }

    TEE_CloseObject(h);

    return SUCCESS;
}

static void* tz_malloc(uint64_t size)
{
    return TEE_Malloc(size, 0);
}

static void tz_free(void* ptr)
{
    TEE_Free(ptr);
}

static void *tz_memcpy(void *dest, void *src, uint32_t n)
{
    TEE_MemMove(dest,src,n);
    return dest;
}

static void *tz_memset(void *s, uint32_t c, uint32_t n)
{
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
    if (NULL == data && data_size != 0)
    {
        BF_LOG("bad params\n");
        return (int)TEE_ERROR_BAD_PARAMETERS;
    }

    TEE_GenerateRandom(data, data_size);
    return (int) TEE_SUCCESS;
}

    
static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{
    TEE_Result res = 0;

    TEE_UUID uuid = TA_KEYMASTER_UUID;
    uint32_t paramTypes = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE );

    TEE_TASessionHandle session;

    uint32_t err_origin = 0;

    TEE_Param paramsta[4];
    TEE_MemFill(paramsta,0,4 * sizeof(TEE_Param));

    if (key == NULL && key_len)
    {
        BF_LOG("bad params\n");
        return FAIL;
    }

    if (key_len != HMAC_SHA_256_KEY_SIZE)
    {
        BF_LOG("key_len not match! \n");
        return FAIL;
    }

    //TEE_OpenTASession
    res = TEE_OpenTASession(&uuid, 0, paramTypes, paramsta, &session, &err_origin);
    if (res != TEE_SUCCESS)
    {
	BF_LOG("TEE_OpenTASession failed res  = 0x%x\n", res);
	return res;
    }

    TEE_MemFill(paramsta,0,4 * sizeof(TEE_Param));

    paramsta[0].memref.buffer = password_key_;
    paramsta[0].memref.size = HMAC_SHA_256_KEY_SIZE;

    paramTypes = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE );

    //TEE_InvokeTACommand
    res = TEE_InvokeTACommand(session, 0, ACCESS_KM_GETKEY_CMD,paramTypes,paramsta,&err_origin);
    if (res != TEE_SUCCESS)
    {
	BF_LOG("TEE_InvokeTACommand failed res = 0x%x\n", res);
	return res;
    }
    else
    {
	TEE_MemMove(key, password_key_, HMAC_SHA_256_KEY_SIZE);	
	//Dump_Buf(password_key_,HMAC_SHA_256_KEY_SIZE);
	TEE_CloseTASession(session);
	return 0;
    }
}

static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint32_t key_len, uint8_t *hmac)
{
    TEE_Result r = 0;
    TEE_OperationHandle handle;
    TEE_ObjectHandle key_handle;
    TEE_Attribute secret_value;

    uint32_t hmac_size = HMAC_SHA256_SIZE;
	(void)key_len;
	
    if ((r = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, HMAC_SHA256_KEY_SIZE, &key_handle))
          != TEE_SUCCESS)
    {
        BF_LOG("TEE_AllocateTransientObject failed with %08x\n", r);
        return FAIL;
    }

    TEE_InitRefAttribute(&secret_value, TEE_ATTR_SECRET_VALUE, (void *) key, (HMAC_SHA256_KEY_SIZE >> 3));

    if ((r = TEE_PopulateTransientObject(key_handle,
         &secret_value, 1)) != TEE_SUCCESS)
    {
        BF_LOG("TEE_PopulateTransientObject failed with 0x%08x\n", r);
        goto error_free_obj;
    }

    if ((r = TEE_AllocateOperation(&handle,
        TEE_ALG_HMAC_SHA256, TEE_MODE_MAC, 256)) != TEE_SUCCESS)
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

    if ((r = TEE_MACComputeFinal(handle, (void *) data, data_size, (void *) hmac, &hmac_size))
          != TEE_SUCCESS)
    {
        BF_LOG("TEE_MACComputeFinal failed with %08x\n", r);
    }

error_free_operation:

    TEE_FreeOperation(handle);

error_free_obj:

    TEE_FreeTransientObject(key_handle);

return r;
}

static uint64_t tz_get_time(void)
{
    TEE_Time time;
    uint64_t time_stamp = 0;

    TEE_GetSystemTime(&time);

    time_stamp = (uint64_t) time.seconds * 1000 + time.millis;

    return time_stamp;
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
//    .plat_fd_open = tz_fd_open,
//    .plat_fd_lseek = tz_fd_lseek,
//    .plat_fd_read = tz_fd_read,
//    .plat_fd_write = tz_fd_write,
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
