#include "bf_custom.h"
#include "bf_types.h"
#include "bf_log.h"

#include "bf_tee_platform_api.h"
#include "bf_tee_plat_func.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
bf_tee_platform_t *gpbf_plat = NULL;
static const char *dev_name = BF_FP_DEV_NAME;
static uint8_t *g_hmac_key="abcdefghijklmnopqrstuvwxyz1234567890";
static uint32_t g_hmac_key_len = 32;
static int32_t tz_spi_open(void)
{
	int devfd = 0;
	devfd = open(dev_name, O_RDWR);//fingerprint_open();
	if(devfd < 0)
	{
		BF_LOG("ERROR:%d",errno);
		return devfd;	
	}
	gpbf_plat->devfd = devfd;
    return SUCCESS;
}

static int32_t tz_spi_close(void)
{
	close(gpbf_plat->devfd);//fingerprint_close();
    return SUCCESS;
}

static int32_t tz_spi_setup(uint32_t speed)
{

    return SUCCESS;
}

int32_t tz_fd_open(const char * pathname, int flags)
{
	BF_LOG("+++++");
	return open(pathname, O_RDWR | O_CREAT | O_SYNC, S_IRWXG|S_IRWXU);
}

int32_t tz_fd_lseek(int fildes, int32_t offset, int whence)
{
	BF_LOG("+++++");
	return lseek(fildes, offset, whence);
}

int32_t tz_fd_read(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return read(filedes, buf, nbytes);
}

int32_t tz_fd_write(int filedes, void *buf, size_t nbytes)
{
	BF_LOG("+++++");
	return write(filedes, buf, nbytes);
}

static int32_t tz_spi_write_read(uint8_t* tx, uint32_t tx_bytes, uint8_t* rx, uint32_t rx_bytes)
{
	int ret = 0;
	if(rx == NULL)
		ret = write(gpbf_plat->devfd, tx, tx_bytes);
	else
		ret = read(gpbf_plat->devfd, rx, rx_bytes);
	
    return ret;
}

static int32_t tz_fs_read(const char* path, void* buf, uint32_t offset, uint32_t size) {
    int32_t rc = -1;
    uint32_t read_count = 0;
    int fd = -1;

    if (buf == NULL && size) 
    {
        BF_LOG("bad parameters\n");
        return -1;
    }

    fd = open(path, O_RDWR | O_CREAT | O_SYNC, S_IRWXG|S_IRWXU);
    if (fd < 0) 
    {
        BF_LOG("open(..) fail. fd = %d\n", fd);
        return -1;
    }
 
    if (offset != 0) 
    {
        rc = lseek(fd, offset, 0);
        if (rc < 0) 
        {
            BF_LOG("lseek(..) fail. rc = %d\n", rc);
            close(fd);
            return -1;
        }
    }
    
    read_count = read(fd, (char*)buf, (int)size);
    if (read_count != size)
    {
        BF_LOG("Request size for readind %s does not match read bytes %d\n", path, size );  
    }      

    close(fd);

    return read_count;
}

static int32_t tz_fs_write(const char* path, void* buf, uint32_t offset, uint32_t size) {

    int32_t rc = -1;
    uint32_t write_count = 0;
    int fd = -1;

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

    fd = open(path, O_RDWR | O_CREAT | O_SYNC, S_IRWXG|S_IRWXU);
    if (fd < 0) 
    {
        BF_LOG("open(..) fail. fd = %d\n", fd);
        return -1;
    }

    if (offset != 0) 
    {
        rc = lseek(fd, offset, 0);
        if (rc < 0) 
        {
            BF_LOG("lseek(..) fail. rc = %d\n", rc);
            close(fd);
            return -1;
        }
    }

    write_count = write(fd, (char*)buf, (int)size);

    if (write_count != size)
    {
        BF_LOG("Request size for readind %s does not match read bytes %d\n", path, size );
    }    

    close(fd);
 
    return write_count;
}

static void tz_msleep(uint32_t ms)
{
    usleep(ms * 1000);
}

/* generate random of `data_size` bytes and store in `data` */
static int32_t tz_get_random(uint8_t *data, uint32_t data_size)
{
    int32_t r = 0;

    return r;
}

static void bl_hexdump(const unsigned char *buf, const int num)
{
	int i;
	int pos = 0;
	char printbuf[1024] = {0};
	for(i = 0; i < num; i++) {
		pos += sprintf(printbuf + pos,"%02X ", buf[i]);
		if ((i+1)%8 == 0)
			pos += sprintf(printbuf + pos,"\n");
	}
	BF_LOG("printbuf=%s\n",printbuf);
	return;
}

static int32_t tz_get_hmac_key(uint8_t* key, uint32_t key_len)
{
	memcpy(key, g_hmac_key, key_len);
	key[key_len - 1] = '\0';
	BF_LOG("key=%s", key);
	//bl_hexdump(key, key_len);
	/*
	uint32_t *cmd = (uint32_t *)key;
	cmd[0] = 0xc1;
	ree_adapter_init_tzapp();
	ree_ca_invoke_command_from_user(key , key_len);
	ree_adapter_destroy_tzapp();
	BF_LOG("key=%s", key);
	bl_hexdump(key, key_len);
	*/
	
    return 0;
}

static int32_t tz_hmac_sha256(const uint8_t *data, uint32_t data_size, const uint8_t *key, uint32_t key_len, uint8_t *hmac)
{
    int32_t ret = 0;
	//BF_LOG("key=%s", key);
    hmac_sha256(key, key_len, data, data_size, hmac, 32);
    //bl_hexdump(hmac, 32);
    return ret;
}
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
	uint64_t timestamp = 0;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	timestamp = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	
    return timestamp;
}


static int32_t tz_set_key(uint8_t *key_buf, uint32_t key_len)
{
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
    .plat_malloc = malloc,
    .plat_free = free,
    .plat_memcpy = memcpy,
    .plat_memset = memset,
    .plat_memcmp = memcmp,
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
