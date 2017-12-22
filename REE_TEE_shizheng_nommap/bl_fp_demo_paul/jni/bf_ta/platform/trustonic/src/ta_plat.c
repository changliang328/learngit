#include "bf_log.h"
#include "drtlspi_api.h"
#include "bf_tee_plat_func.h"


#define HMAC_SHA256_SIZE	(32)
#define HMAC_SHA_256_KEY_SIZE   (32)
#define HMAC_SHA256_KEY_SIZE	(256)


static uint8_t password_key_[HMAC_SHA_256_KEY_SIZE];   
static int32_t spi_dev_fd = -1;
bf_tee_platform_t *gpbf_plat = NULL;

static struct mt_chip_conf  spi_chip_config = {
    .setuptime = 7,
    .holdtime = 7,

    .high_time = 50,
    .low_time = 50,
    .cs_idletime = 3,
    .ulthgh_thrsh = 0,
    .cpol = SPI_CPOL_0, //0
    .cpha = SPI_CPHA_0, //0
    .rx_mlsb = SPI_MSB,  //1
    .tx_mlsb = SPI_MSB,  //1
    .tx_endian = SPI_LENDIAN, //0
    .rx_endian = SPI_LENDIAN, //0
    .com_mod = FIFO_TRANSFER, //0
    .pause = PAUSE_MODE_DISABLE, //0
    .finish_intr = FINISH_INTR_EN, //1
    .deassert = DEASSERT_DISABLE, //0
    .ulthigh = ULTRA_HIGH_DISABLE, //0
    .tckdly = TICK_DLY0, //0
};


void tlApiLogPrintf(const char *fmt, ...)
{
    return;
}



static int32_t tz_spi_open(void)
{ 
    return TEE_SUCCESS;
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

    default:
        spi_chip_config.high_time = 50;
        spi_chip_config.low_time = 50;
        break;
    }

    return SUCCESS;
}

static int32_t tz_spi_write_read(uint8_t* tx, uint32_t tx_bytes, uint8_t* rx, uint32_t rx_bytes) 
{
    TEE_Result r = 0;
    int rc = -1;
    unsigned char *rx_buf = NULL;

    uint32_t total_bytes = tx_bytes + rx_bytes;

    rx_buf = TEE_Malloc(total_bytes, 0);
    if (NULL == rx_buf) 
    {
        BF_LOG("TEE_Malloc(..) fail\n");
        rc = FAIL;
        goto exit;
    }
    TEE_MemFill(rx_buf, 0, total_bytes);

    if (total_bytes > 32)
    {
        spi_chip_config.com_mod = DMA_TRANSFER;
    }
    else
    {
        spi_chip_config.com_mod = FIFO_TRANSFER;
    }    

    tz_spi_open();

    if ((r = drSpiSend(tx, rx_buf, total_bytes, &spi_chip_config, 1)) != TEE_SUCCESS)
    {
        BF_LOG("drSpiSend(..) fail, rc = %d\n", r);
        rc = FAIL;
        goto exit;
    }
    rc = SUCCESS;
    
    if (rx != NULL && rx_bytes != 0)
    {
        TEE_MemMove(rx, rx_buf + tx_bytes, rx_bytes);
    }
    
exit:
    tz_spi_close();

    if (NULL != rx_buf)
    {
        TEE_Free(rx_buf);
        rx_buf = NULL;
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
        BF_LOG("Requested size for reading %s does not match read bytes %d", path, size);
        return SUCCESS;
    }

    return SUCCESS;
}

static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size)
{
    TEE_Result r = 0;
    TEE_ObjectHandle h;
    uint32_t flags = 0;

    BF_LOG("tz_fs_write path: %s buf %p size %u\n", path, buf, size);

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
    return TEE_MemCompare((void *)buffer1, (void *)buffer2, size);
}

static void tz_msleep(uint32_t ms) 
{
    TEE_Wait(ms);
}

/* generate random of `data_size` bytes and store in `data` */
static int32_t tz_get_random(uint8_t *data, uint32_t data_size)
{
    if (data == NULL && data_size)
    {
        BF_LOG("bad parameters\n");      
        return (int)TEE_ERROR_BAD_PARAMETERS;
    }

    TEE_GenerateRandom(data, data_size);
    return (int)TEE_SUCCESS;
}


static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{

    if (key == NULL && key_len) 
    {
        BF_LOG("bad parameters\n");
        return (int)TEE_ERROR_BAD_PARAMETERS;
    }


    if (key_len != HMAC_SHA_256_KEY_SIZE) 
    {
        BF_LOG("key_len not match! \n");
        return FAIL;
    }

    return get_hmac_key(key, key_len);
}

static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint32_t key_len, uint8_t *hmac)
{
    TEE_Result r = 0;
    TEE_OperationHandle handle;
    TEE_ObjectHandle key_handle;
    TEE_Attribute secret_value;

    uint32_t hmac_size = HMAC_SHA256_SIZE;

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

    if ((r = TEE_MACComputeFinal(handle, (void *) data, (size_t)data_size, (void *) hmac, (size_t*)&hmac_size))
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
/*
    TEE_Time time;
    uint64_t time_stamp = 0;
    TEE_GetSystemTime(&time);

    time_stamp = (uint64_t) time.seconds * 1000 + time.millis;

    return time_stamp;
*/
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
