#include "bf_custom.h"
#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"

struct plat_func *gpMethods;
static bf_tee_platform_t gbf_tee_platform;
bf_tee_platform_t *bf_tee_platform_init(void)
{
	gpMethods = bf_get_plat_func(&gbf_tee_platform);
	bf_tee_spi_open();
	return &gbf_tee_platform;
}

//SPI TEE OS 
int bf_tee_spi_open()
{
	return gpMethods->plat_spi_open();
}

int bf_tee_spi_close()
{
	return gpMethods->plat_spi_close();
}

int bf_tee_spi_setup(uint32_t speed)
{
	return gpMethods->plat_spi_setup(speed);
}

int bf_tee_spi_init(uint32_t speed)
{
	return 0;
}

int bf_tee_fd_open(const char * pathname, int flags)
{
	int ret = 0;
	if(gpMethods->plat_fd_open)
		ret = gpMethods->plat_fd_open(pathname, flags);
	else
		ret = 0;
	return ret;
}

int bf_tee_fd_lseek(int fildes, int32_t offset, int whence)
{
	return gpMethods->plat_fd_lseek(fildes, offset, whence);
}

int bf_tee_fd_read(int filedes, void *buf, size_t nbytes)
{
	int ret = 0;
	if(gpMethods->plat_fd_read)
		ret = gpMethods->plat_fd_read(filedes, buf, nbytes);
	return ret;
}

int bf_tee_fd_write(int filedes, void *buf, size_t nbytes)
{
	int ret = 0;
	if(gpMethods->plat_fd_write)
		ret = gpMethods->plat_fd_write(filedes, buf, nbytes);
	return ret;
}

int bf_tee_spi_read_write(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes)
{
	return gpMethods->plat_spi_write_read(tx, tx_bytes, rx, rx_bytes);
}

int bf_tee_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size)
{
	return gpMethods->plat_fs_read(path, buf, offset, size);
}

int bf_tee_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size)
{
	return gpMethods->plat_fs_write(path, buf, offset, size);
}

void*  bf_tee_malloc(uint64_t size)
{
	return gpMethods->plat_malloc(size);
}

void bf_tee_free(void* ptr)
{
	return gpMethods->plat_free(ptr);
}

void* bf_tee_memcpy(void *dest, void *src, uint32_t n)
{
	return gpMethods->plat_memcpy(dest, src, n);
}

void* bf_tee_memset(void *s, uint32_t c, uint32_t n)
{
	return gpMethods->plat_memset(s, c, n);
}

int32_t bf_tee_memcmp(const void *buffer1, const void *buffer2, uint32_t size)
{
	return gpMethods->plat_memcmp(buffer1, buffer2, size);
}

int32_t bf_tee_msleep(uint32_t ms)
{
	gpMethods->plat_msleep(ms);
	return 0;
}

int32_t bf_tee_get_random(uint8_t *data, uint32_t data_size)
{
	return gpMethods->plat_get_random(data, data_size);
}

int32_t bf_tee_hmac_key(uint8_t* key, uint32_t key_len)
{
	return gpMethods->plat_hmac_key(key, key_len);
}

int32_t bf_tee_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key,  uint32_t key_len, uint8_t *hmac)
{
	return gpMethods->plat_hmac_sha256(data, data_size, key, key_len, hmac);
}

uint64_t bf_tee_get_time()
{
	return gpMethods->plat_get_time();
}

int32_t bf_tee_set_key(uint8_t *key_buf, uint32_t key_len)
{
	return gpMethods->plat_set_key(key_buf, key_len);
}

