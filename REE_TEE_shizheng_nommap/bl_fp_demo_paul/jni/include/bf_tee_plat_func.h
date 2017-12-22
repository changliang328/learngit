#ifndef __BF_TEE_PLATFORM_FUNC_H__
#define __BF_TEE_PLATFORM_FUNC_H__
#include "bf_tee_platform_api.h"
struct plat_func{
    //.plat_spi_open = tz_spi_open,
    int32_t (*plat_spi_open)(void);
    //.plat_spi_close = tz_spi_close,
    int32_t (*plat_spi_close)(void);
    //plat_spi_init = tz_spi_init,
    int32_t (*plat_spi_setup)(uint32_t speed);    
    //.plat_spi_write_read = tz_spi_write_read,
    int32_t (*plat_spi_write_read)(uint8_t* in, uint32_t in_size, uint8_t* out, uint32_t out_size);
	int32_t (*plat_fd_open)(const char * pathname, int flags);
	int32_t (*plat_fd_lseek)(int fildes, int32_t offset, int whence);
	int32_t (*plat_fd_read)(int filedes, void *buf, size_t nbytes);
	int32_t (*plat_fd_write)(int filedes, void *buf, size_t nbytes);
    //.plat_fs_read = tz_fs_read,
    int32_t (*plat_fs_read)(const char* path, void* buf, uint32_t offset, uint32_t size);
    //.plat_fs_write = tz_fs_write,
    int32_t (*plat_fs_write)(const char* path, void* buf, uint32_t offset, uint32_t size);
    //.plat_malloc = tz_malloc,
    void* (*plat_malloc)(uint64_t size);
    //.plat_free = tz_free,
    void (*plat_free)(void* ptr);
    //.plat_memcpy = tz_memcpy,
    void* (*plat_memcpy)(void *dest, void *src, uint32_t n);
    //.plat_memset = tz_memset,
    void* (*plat_memset)(void *s, uint32_t c, uint32_t n);
    //.plat_memcmp = tz_memcmp,
    int32_t (*plat_memcmp)(const void *buffer1, const void *buffer2, uint32_t size);
    //.plat_msleep = tz_msleep,
    void (*plat_msleep)(uint32_t ms);
    //.plat_get_random = tz_get_random,
    int32_t (*plat_get_random)(uint8_t *data, uint32_t data_size);
    //.plat_hmac_key = tz_get_hmac_key,
    int32_t (*plat_hmac_key)(uint8_t* key, uint32_t key_len);
    //.plat_hmac_sha256 = tz_hmac_sha256,
    int32_t (*plat_hmac_sha256)(const uint8_t *data, uint32_t data_size, const uint8_t *key,  uint32_t key_len, uint8_t *hmac);
    //.plat_get_time = tz_get_time,
    uint64_t (*plat_get_time)(void);
	//.plat_set_key = tz_set_key,
    int32_t (*plat_set_key)(uint8_t *key_buf, uint32_t key_len);
};
struct plat_func *bf_get_plat_func(bf_tee_platform_t *pbf_plat);
#endif
