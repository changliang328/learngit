#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

/* on cryptography */
int get_random(uint8_t *data, uint32_t data_size);
int get_hmac_key(uint8_t* key);
int hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint8_t *hmac);

/* on file operations */
int fs_read(char *path, void *buf, uint32_t *size);
int fs_write(char *path, void *buf, uint32_t size);

/* on memory operations*/
void *tk_malloc(uint32_t size);
void tk_free(void *p);
void *tk_memcpy(void *dest, void *src, int n);
void *tk_memset(void *s, int c, int n);

/* on SPI operations */
int spi_init(void);
int spi_transfer(char *buf, int size);

/* on helper */
void mdelay(int ms);
uint64_t get_time(void);

#endif
