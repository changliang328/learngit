#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <contrib/libtsfs/f_fs.h>
#include <contrib/libtsfs/f_file_cipher.h>
#include <contrib/lib_tvm_time/lib_tvm_time.h>
#include <contrib/lib_tvm_crypto/lib_tvm_crypto.h>
#include <contrib/libuTkeymaster/hw_auth_token.h>
#include <contrib/libuTkeymaster/km_auth_api.h>
#include <contrib/libmatrix_s/libmatrix_s.h>
#include "bf_log.h"
#include "fp_spi.h"
#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"


#define HMAC_SHA256_SIZE	(32)
#define FP_TA_ERROR_PARAMETER (22)
#define FP_TA_ERROR_IO_NO_FILE (4)
#define FP_TA_ERROR_IO (3)
bf_tee_platform_t *gpbf_plat = NULL;
static struct mt_chip_conf spi_chip_config = {
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

	.com_mod = FIFO_TRANSFER,
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

int32_t tz_fd_open(const char * pathname, int  flags)
{
	BF_LOG("+++++");
	flags=fc_open(pathname, FO_RDWR | FO_CREAT);
	return flags;
}

int32_t tz_fd_lseek(int fildes, int32_t offset, int whence)
{
	BF_LOG("+++++");
	return fc_lseek(fildes, offset, whence);
}

int32_t tz_fd_read(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return fc_read(filedes, buf, nbytes);
}

int32_t tz_fd_write(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return fc_write(filedes, buf, nbytes);
}

static int32_t tz_spi_write_read(uint8_t* tx, uint32_t tx_bytes, uint8_t* rx, uint32_t rx_bytes)
{
	uint32_t tlRet;
	uint32_t total_bytes = tx_bytes + rx_bytes;

	if (total_bytes >= 32)
		spi_chip_config.com_mod = DMA_TRANSFER;
	else
		spi_chip_config.com_mod = FIFO_TRANSFER;

	tlRet = drSpiSend(tx, rx, tx_bytes, rx_bytes, &spi_chip_config, 1);
	
	return tlRet;
}

static int32_t tz_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size) {
  int read_bytes = 0;
  int retval = 0;
  int fd = -1;

  if (NULL == buf)
  {
    printf("%s: Read buffer is NULL", __func__);
    return -FP_TA_ERROR_PARAMETER;
  }

  fd = fc_open(path, FO_RDONLY);
  if (fd < 0)
  {
			printf("%s: isee_sfs_open failed for %s with error %d",
				__func__, path, fd);
			return -FP_TA_ERROR_IO_NO_FILE;
  }
	fc_lseek(fd , offset, SEEK_SET);
  retval = fc_read(fd, (char *) buf, (int) size);
  if (0 > retval)
  {
    printf("%s: isee_sfs_read failed for %s with error %d",
        __func__, path, retval);
    return -FP_TA_ERROR_IO;
  }
  else if(size != (uint32_t)retval)
  {
    printf("%s: requested size for reading %s does not match read bytes %d %d",
        __func__, path, size, retval);
 //   return -FP_TA_ERROR_IO;
  }

  read_bytes = retval;

  retval = fc_close(fd);
  if (0 != retval)
  {
    printf("%s: isee_fs_close failed for %s with error %d",
        __func__, path, retval);
    return -FP_TA_ERROR_IO;
  }

  BF_LOG("%s: Read %d bytes", __func__, read_bytes);

  return read_bytes;
}

static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size) {

  int written_bytes = 0;

  int retval = 0;
  int fd = -1;

  if (NULL == buf)
  {
    printf("%s: Write buffer is NULL", __func__);
    return -FP_TA_ERROR_PARAMETER;
  }

  fd = fc_open(path, FO_RDWR | FO_CREAT);
  if (fd < 0)
  {
    printf("%s: isee_sfs_open failed for %s with error %d",
        __func__, path, fd);
    return -FP_TA_ERROR_IO;
  }

	fc_lseek(fd , offset, SEEK_SET);
  retval = fc_write(fd, (const char *) buf, (int) size);
  if (0 > retval)
  {
    printf("%s: isee_sfs_write failed for %s with error %d",
         __func__, path, retval);
    return -FP_TA_ERROR_IO;
  }
  else if(size != (uint32_t)retval)
  {
    printf("%s: requested size for writing %s does not match written bytes %d %d",
        __func__, path, size, retval);
   // return -FP_TA_ERROR_IO;
  }
  written_bytes = retval;

  retval = fc_close(fd);
  if (0 != retval)
  {
    printf("%s: isee_fs_close failed for %s with error %d",
        __func__, path, retval);
    return -FP_TA_ERROR_IO;
  }


  BF_LOG("%s: Wrote %d bytes", __func__, written_bytes);

  return written_bytes;
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
    ut_msleep(ms);
}

/* generate random of `data_size` bytes and store in `data` */
static int32_t tz_get_random(uint8_t *data, uint32_t data_size)
{
  int liret = 0;

  BF_LOG("tvm_tls_open is doing");
  liret = tvm_tls_open("random_client");
  if(liret != 0)
  {
    BF_LOG("Error:%d. Did not open tvm tls device.", liret);
    return -1;
  }

  BF_LOG("TVM_RAND_Generate is doing");
  TVM_RAND_Generate(data, data_size);

  BF_LOG("tvm_tls_close is doing");
  liret = tvm_tls_close();
  if(liret != 0)
  {
    BF_LOG("Error:%d. Did not close tvm tls device.", liret);
    return -1;
  }
    //*(random) = random_temp++;
  BF_LOG("fido_sec_generate_random end");

  return liret;
}


static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{
  int status;
  status = get_hmac_key(key, &key_len);
  if (status != 0) {
    BF_LOG("fp get hmac key has failed with code: %d", status);
    return -1;
  } 
  else {
    BF_LOG("fp get hmac key success");
  }  
  return 0;
}

#if 1
static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint32_t key_len, uint8_t *hmac)
{
  key=key;
  key_len=key_len;
  int status;
  unsigned char sha256_hmac[32] = { 0 };
  unsigned long out_hmacKeyLen;
  uint8_t* hmac_key;
  int hmac_key_size;

  hmac_key = malloc(UT_HMAC_KEY_MAX_LENGTH);
  if (hmac_key == 0) {
    BF_LOG("%s :Memeroy alloc failed.", __func__);
    return -1;
  }
  status = get_hmac_key(hmac_key, &hmac_key_size);
  if (status != 0) {
    BF_LOG("fp get hmac key has failed with code: %d", status);
    free(hmac_key);
    return -1;
  } else {
    BF_LOG("fp get hmac key success");
  }
  status = psHmacSha2(hmac_key, hmac_key_size, data, data_size, hmac, sha256_hmac,
      &out_hmacKeyLen, SHA256_HASH_SIZE);
  if (status <= 0) {
    BF_LOG("%s failed to create HMAC, error %d", __func__, status);
    free(hmac_key);
    return -1;
  }
  free(hmac_key);
  return 0;
}


#else 

uint32_t tz_hmac_sha256(const uint8_t* data, uint32_t size_data,
		const uint8_t* key, uint32_t size_key,
		uint8_t* hmac){
	(void)key;
	(void)size_key;
	int ret = 0 ;
	ut_pf_cp_context_t *ctx = NULL;
	ut_size_t maclen = SHA256_SIZE;
	/* open operation */
	ret = ut_pf_cp_open(&ctx, UT_PF_CP_CLS_MC, UT_PF_CP_ACT_MC_HMAC_SHA256);
	if ( ret < 0 ) {
		LOGE("call ut_pf_cp_open failed !!! ret = %d", ret);
		return ret;
	}
	/* start mac operaton */
	ret = ut_pf_cp_mc_start(ctx, key, size_key, NULL, 0);
	if ( ret < 0 ) {
		ut_pf_cp_close(ctx);
		LOGE("call ut_pf_cp_mc_starts failed !!! ret = %d", ret);
		return ret;
	}
	/* update mac operaton */
	ret = ut_pf_cp_mc_update(ctx, data,  size_data);
	if ( ret < 0 ) {
		ut_pf_cp_close(ctx);
		LOGE("call ut_pf_cp_mc_update failed !!! ret = %d", ret);
		return ret;
	}
	/* finish mac operaton */
	ret = ut_pf_cp_mc_finish(ctx, hmac, &maclen);
	if ( ret < 0 ) {
		ut_pf_cp_close(ctx);
		LOGE("call ut_pf_cp_mc_finish failed !!! ret = %d", ret);
		return ret;
	}
	/* close operation */
	ret = ut_pf_cp_close(ctx);
	if ( ret < 0 ) {
		LOGE("call ut_pf_cp_close failed !!! ret = %d", ret);
		return ret;
	}
	return ret;
}
#endif


/*
static uint32_t convert_x(uint32_t x)
{
	return (x >> 24) | ((x >> 8) & 0xff00U) | ((x << 8) & 0xff0000U) | (x << 24);
}

static uint64_t htobe64(uint64_t x)
{
	return ((convert_x(x) + 0ULL) << 32) | convert_x(x >> 32);
}*/

static uint64_t tz_get_time(void)
{
        uint32_t high = 0;
        uint32_t low = 0;
        uint32_t time_result;
        uint64_t now;
        TIME_Time time;
        time_result = GetSystemTime(&time);
        if(time_result == 0){
                high = time.seconds;
                low = time.mills;
        }
    now  = high;
    now = now * 1000;
    now = now + low;
    return now;
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
	.plat_fd_open = tz_fd_open,
	.plat_fd_lseek = tz_fd_lseek,
	.plat_fd_read = tz_fd_read,
	.plat_fd_write = tz_fd_write,
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
