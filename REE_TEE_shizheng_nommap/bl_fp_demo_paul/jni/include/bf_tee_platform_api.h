#ifndef __BF_PLATFORM_API_H__
#define __BF_PLATFORM_API_H__
#include "bf_types.h"

#define SUCCESS	(0)
#define FAIL	(-1)

typedef struct {
	int devfd;
	int dbfd;
}bf_tee_platform_t;

bf_tee_platform_t *bf_tee_platform_init(void);

//SPI TEE API
int bf_tee_spi_open(void);
int bf_tee_spi_close(void);
int bf_tee_spi_setup(uint32_t speed);
int bf_tee_spi_init(uint32_t speed);
int bf_tee_spi_read_write(uint8_t *tx, size_t tx_bytes, uint8_t *rx, size_t rx_bytes);
//FS TEE API
int bf_tee_fd_open(const char * pathname, int flags);
int bf_tee_fd_lseek(int fildes, int32_t offset, int whence);
int bf_tee_fd_read(int filedes, void *buf, size_t nbytes);
int bf_tee_fd_write(int filedes, void *buf, size_t nbytes);

int bf_tee_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size);
int bf_tee_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size);
//memory TEE API
void*  bf_tee_malloc(uint64_t size);
void bf_tee_free(void* ptr);
void* bf_tee_memcpy(void *dest, void *src, uint32_t n);
void* bf_tee_memset(void *s, uint32_t c, uint32_t n);
int32_t bf_tee_memcmp(const void *buffer1, const void *buffer2, uint32_t size);
//time hamc key
int32_t bf_tee_msleep(uint32_t ms);
int32_t bf_tee_get_random(uint8_t *data, uint32_t data_size);
int32_t bf_tee_hmac_key(uint8_t* key, uint32_t key_len);
int32_t bf_tee_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key,  uint32_t key_len, uint8_t *hmac);
uint64_t bf_tee_get_time(void);
int32_t bf_tee_set_key(uint8_t *key_buf, uint32_t key_len);

//SPI TEE OS
//file system TEE OS read write
/*
int bf_tee_fs_open(char* path);
int bf_tee_fs_read(int fd, void *buf, size_t count);
int bf_tee_fs_write(int fd, void *buf, size_t count);
int bf_tee_fs_lseek(int fd, off_t offset, int whence);
int bf_tee_fs_close(int fd);
*/

#endif
