#ifndef bf_DEVICE_H
#define bf_DEVICE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdlib.h>
#include <stdbool.h>
#include "bf_types.h"

#ifdef FP_M_AUTO_TEST
#define IMAGE_VALUE_MIN 1
#define IMAGE_VALUE_MAX 100
#define EVENT_VALUE_MIN 101
#define EVENT_VALUE_MAX 107
#define IMAGE_FOR_EVENT 101
#endif

#define SILEAD_6185_ID  0x6185
#define SILEAD_6163_ID  0x6163
#define GOODIX_3288_ID  0x3288
#define GOODIX_3268_ID  0x3268

struct bf_ree_device {
	int devfd;
    int sysfs_fd;
    int cancel_fds[2];
};

typedef struct bf_ree_device bf_ree_device_t;
int32_t fingerprint_open(void);

int32_t fingerprint_close(void);

int32_t enable_Irq_ioctl(void);

int32_t disable_Irq_ioctl(void);

int32_t enable_power_ioctl(void);
int32_t disable_power_ioctl(void);
int32_t do_hwreset_ioctl(void);

void bf_ree_device_destroy(bf_ree_device_t* device);
bf_ree_device_t* bf_ree_device_new();

int bf_ree_device_wait_irq_paul_timeout(bf_ree_device_t* device, int timeout);
int bf_ree_device_set_cancel(bf_ree_device_t* device);
int bf_ree_device_clear_cancel(bf_ree_device_t* device);
int bf_key_event_ioctl(navigation_info *navi_info);
int bf_io_init(void);
int bf_io_exit(void);
int32_t clear_pollflag_ioctl(void);
int bf_ree_device_wait_irq_paul(bf_ree_device_t* device);
int32_t disable_spi_ioctl(void);


#ifdef __cplusplus
}
#endif
#endif
