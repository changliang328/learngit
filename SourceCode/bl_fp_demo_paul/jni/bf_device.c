
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include "bf_log.h"
#include <cutils/log.h>

#include "bf_device.h"

#include <dirent.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
const char* dev_name = "/dev/blestech_fp";//"/dev/fingerprint";
#define FP_IOC_MAGIC     'f'  //define magic number
int file_descriptor_ = -1;
//define commands
#if 1
#define  FP_IOC_CMD_ENABLE_IRQ   _IO(FP_IOC_MAGIC, 1)
#define  FP_IOC_CMD_DISABLE_IRQ   _IO(FP_IOC_MAGIC, 2)
#define  FP_IOC_CMD_SET_NAVIGATION_EVENT   _IO(FP_IOC_MAGIC, 3)
#define  FP_IOC_CMD_GET_IRQ_STATUS  _IO(FP_IOC_MAGIC, 4)
#define  FP_IOC_CMD_SET_WAKELOCK_STATUS  _IO(FP_IOC_MAGIC, 5)
#define  FP_IOC_CMD_SEND_SENSORID      _IO(FP_IOC_MAGIC, 6)
/*DTS2016120706211 shehong.wt swx382985 20161231 begin*/
#define  FP_IOC_CMD_SEND_SENSOR_MODE      _IO(FP_IOC_MAGIC, 7)
/*DTS2016120706211 shehong.wt swx382985 20161231 end*/
#define  FP_IOC_CMD_ENABLE_SPI_CLK      _IO(FP_IOC_MAGIC, 10)
#define  FP_IOC_CMD_DISABLE_SPI_CLK     _IO(FP_IOC_MAGIC, 11)
#else
#define  FP_IOC_CMD_ENABLE_IRQ   _IO(FP_IOC_MAGIC, 3)
#define  FP_IOC_CMD_DISABLE_IRQ   _IO(FP_IOC_MAGIC, 4)
#define  FP_IOC_CMD_SET_NAVIGATION_EVENT   _IO(FP_IOC_MAGIC, 14)
#define  FP_IOC_CMD_GET_IRQ_STATUS  _IO(FP_IOC_MAGIC, 4)//##
#define  FP_IOC_CMD_SET_WAKELOCK_STATUS  _IO(FP_IOC_MAGIC, 5)//##
#define  FP_IOC_CMD_SEND_SENSORID      _IO(FP_IOC_MAGIC, 13)
/*DTS2016120706211 shehong.wt swx382985 20161231 begin*/
#define  FP_IOC_CMD_SEND_SENSOR_MODE      _IO(FP_IOC_MAGIC, 7)
/*DTS2016120706211 shehong.wt swx382985 20161231 end*/
#endif
//extern uint16_t g_sensor_id;
/*
struct bf_ree_device {
	int devfd;
    int sysfs_fd;
    int cancel_fds[2];
};
*/
/* < DTS2015112804817 c00299109 20151204 begin */
static void check_fail_reason()
{
    struct stat buf;
    const char* path = "/sys/bl_fingerprint_sysfs/irq";

    memset((void*)&buf, 0, sizeof(struct stat));
    if (0 > stat(dev_name, &buf))
    {
        LOGE("stat %s failed: %d", dev_name, errno);
    }
    else
    {
        LOGE("%s is %d:%d", dev_name, buf.st_uid, buf.st_gid);
    }

    memset((void*)&buf, 0, sizeof(struct stat));
    if (0 > stat(path, &buf))
    {
        LOGE("stat %s failed: %d", path, errno);
    }
    else
    {
        LOGE("%s is %d:%d", path, buf.st_uid, buf.st_gid);
    }
}
 /* DTS2015112804817 c00299109 20151204 end > */

int32_t fingerprint_open(void)
{

    if (file_descriptor_ < 0)
    {
        file_descriptor_ = open(dev_name, O_RDWR);
    }


    if (file_descriptor_ < 0)
    {
/* < DTS2015112804817 c00299109 20151204 begin */
        check_fail_reason();
/* DTS2015112804817 c00299109 20151204 end > */
        LOGE("bf fingerprint_open failed \n");
        return -1;
    }

    return file_descriptor_;
}


int32_t fingerprint_close(void)
{
    close(file_descriptor_);
    file_descriptor_ = -1;
    return 0;
}


int32_t enable_Irq_ioctl(void)
{
    //LOGI("bf enable_Irq_ioctl to call ioctl.\n");

    int ret = ioctl(file_descriptor_, FP_IOC_CMD_ENABLE_IRQ, NULL);
    if ( ret < 0)
    {
        LOGE("bf enable_Irq_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }

    return ret;
}


int32_t disable_Irq_ioctl(void)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_DISABLE_IRQ, NULL);
    if (ret < 0)
    {
        LOGE("bf disable_Irq_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }

    return ret;
}

int32_t enable_spi_ioctl(void)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_ENABLE_SPI_CLK, NULL);
    if ( ret < 0)
    {
        LOGE("bf enable_spi_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}
int32_t disable_spi_ioctl(void)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_DISABLE_SPI_CLK, NULL);
    if (ret < 0)
    {
        LOGE("bf disable_spi_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t set_navigation_event_ioctl(uint32_t value)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_SET_NAVIGATION_EVENT, &value);
    if (ret < 0)
    {
        LOGE("bf set_navigation_event_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }

    return ret;
}


int32_t get_irq_status_ioctl(void)
{

    uint32_t value=0;
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_GET_IRQ_STATUS, &value);
    if (ret < 0)
    {
        LOGE("bf get_irq_status_ioctl failed errno = %d, ret = %d\n", errno, ret);
        return ret;
    }

    return value;
}


int32_t set_wake_lock_status_ioctl(uint32_t value)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_SET_WAKELOCK_STATUS, &value);
    if ( ret < 0)
    {
        LOGE("bf set_wake_lock_status to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}


int32_t send_sensor_id_ioctl(uint32_t value)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_SEND_SENSORID, &value);
    if ( ret < 0)
    {
        LOGE("bf send_sensor_id_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}
/*DTS2016120706211 shehong.wt swx382985 20161231 begin*/
int32_t send_sensor_mode_ioctl(char *sensor_mode)
{
    int ret = ioctl(file_descriptor_, FP_IOC_CMD_SEND_SENSOR_MODE, sensor_mode);
    if ( ret < 0)
    {
        LOGE("bf send_sensor_mode_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}
/*DTS2016120706211 shehong.wt swx382985 20161231 end*/
static int dev_phys_path_by_attr(const char *attr_val,
                                 const char *base, char *path, int path_max);
static int sysfs_node_write(bf_ree_device_t* irq_device, const char* name,
                                const char* value);


void bf_ree_device_destroy(bf_ree_device_t* device)
{
    if (!device)
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return;
    }
    
    if (device->sysfs_fd != -1)
    {
        close(device->sysfs_fd);
        device->sysfs_fd = -1;
    }
    
    if (device->cancel_fds[0] != -1)
    {
        close(device->cancel_fds[0]);
        device->cancel_fds[0] = -1;
    }
    
    if (device->cancel_fds[1] != -1)
    {
        close(device->cancel_fds[1]);
        device->cancel_fds[1] = -1;
    }

    free(device);
    device = NULL;
}


bf_ree_device_t* bf_ree_device_new()
{
    bf_ree_device_t* device = malloc(sizeof(bf_ree_device_t));
    if (!device)
    {
        errno = ENOMEM;
        goto err;
    }
    device->devfd = -1;
    device->sysfs_fd = -1;
    device->cancel_fds[0] = -1;
    device->cancel_fds[1] = -1;
    char path[PATH_MAX] = {0};

	device->devfd = open(dev_name, O_RDWR);
	if (device->devfd < 0)
	{
        LOGE("%s Error can not open dev %s", __func__,dev_name);
	    errno = ENOENT;
        goto err;
	}

    if (!dev_phys_path_by_attr("bl_fingerprint_sysfs", "/sys",
                              path, PATH_MAX))
    {
        LOGE("%s Error didn't find phys path device", __func__);
        errno = ENOENT;
        goto err;
    }

    device->sysfs_fd = open(path, O_RDONLY);
    if (device->sysfs_fd == -1)
    {
        LOGE("%s open %s failed %i\n", __func__, path, errno);
        goto err;
    }
    if (pipe(device->cancel_fds))
    {
        goto err;
    }
    return device;
err:
    bf_ree_device_destroy(device);
    return NULL;
}

int bf_ree_device_wait_irq_paul_timeout(bf_ree_device_t* device, int timeout)
{
    int irq_fd = -1;
    int status = 0;
    int irq_count = 0;

    if( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    for(;;)
    {
        irq_fd = device->devfd;

        if (irq_fd == -1) 
        {
            status = -errno;
            LOGE("bf open irq error %d ",status);
            goto out;
        }

        struct pollfd pfd[2] = {{-1,0,0},{-1,0,0}};
        pfd[0].fd = irq_fd;
        pfd[0].events = POLLERR | POLLPRI;
        pfd[1].fd = device->cancel_fds[0];
        pfd[1].events = POLLIN;
        status = poll(pfd, 2, timeout);


        if (status == -1)
        {
            status = -errno;
            LOGE("bf poll irq error");
            goto out;
        }else if (status == 0)
        {
        	status = -ETIME;
            LOGE("bf poll irq timeout");
            goto out;
        }
        else if (pfd[1].revents)
        {
            status = -EINTR;
            LOGD("bf poll irq interupt");
            goto out;
        }else if (pfd[0].revents)
        {
            LOGD("bf poll irq status=%d",pfd[0].revents);
            status = pfd[0].revents;
            goto out;
        }
        
        irq_count++;
        //LOGE("irq count is %d", irq_count);
        if (irq_count >= 5)
        {
            LOGE("reach max unexpected irq count, goto clean");
            status = -1;
            goto out;
        }

    }
out:
    //LOGE("irq count is %d", irq_count);  //add this for flexlint warning:Last value assigned to variable 'irq_count' (defined at line 249) not used
    return status;
}
int bf_ree_device_wait_irq_paul(bf_ree_device_t* device)
{
	return bf_ree_device_wait_irq_paul_timeout(device, -1);
}

int bf_ree_device_wait_irq(bf_ree_device_t* device)
{
    int irq_fd = -1;
    int status = 0;
    int irq_count = 0;

    if( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }

    for(;;)
    {
#ifdef FP_M_AUTO_TEST
        irq_fd = openat(device->sysfs_fd, "result", O_RDONLY | O_NONBLOCK);
#else
        irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
#endif
        if (irq_fd == -1) 
        {
            status = -errno;
            LOGE("bf open irq error %d ",status);
            goto out;
        }
#ifdef FP_M_AUTO_TEST
        int value = 0;
        char finger_num[4] = {'0'};
        status = read(irq_fd, finger_num, sizeof(finger_num));
        if (status < 0)
        {
            status = -errno;
            LOGE("bf read irq error %d",status);
            goto out;
        }
        value = atoi(finger_num);
        if ((value >= IMAGE_VALUE_MIN && value <= IMAGE_VALUE_MAX) || (value >= EVENT_VALUE_MIN && value <= EVENT_VALUE_MAX))
        {
            status = value;
            sysfs_node_write(device, "result", "0");
            goto out;
        }
#else
        char value = 0;
        status = read(irq_fd, &value, sizeof(value));
        if (status < 0)
        {
            status = -errno;
            LOGE("bf read irq error %d",status);
            goto out;
        }
        else if (status == 0)
        {
            status = -ENOSYS;
            goto out;
        }
        if (value == '1')
        {
            status = 0;
            goto out;
        }
#endif
        struct pollfd pfd[2] = {{-1,0,0},{-1,0,0}};
        pfd[0].fd = irq_fd;
        pfd[0].events = POLLERR | POLLPRI;
        pfd[1].fd = device->cancel_fds[0];
        pfd[1].events = POLLIN;
        status = poll(pfd, 2, -1);
        sysfs_node_write(device, "irq", "1");
        if (status == -1)
        {
            status = -errno;
            LOGE("bf poll irq error");
            goto out;
        }
        else if (pfd[1].revents)
        {
            status = -EINTR;
            LOGD("bf poll irq interupt");
            goto out;
        }
        irq_count++;
        LOGE("irq count is %d", irq_count);
        if (irq_count >= 5)
        {
            LOGE("reach max unexpected irq count, goto clean");
            status = 0;
            goto out;
        }
        close(irq_fd);
    }
out:
    if (irq_fd != -1)
    {
        close(irq_fd);
    }
    LOGE("irq count is %d", irq_count);  //add this for flexlint warning:Last value assigned to variable 'irq_count' (defined at line 249) not used
    return status;
}

int bf_ree_read_readimage_flag(bf_ree_device_t* device, char* value)
{
    int status = 0;
    int readimage_fd = -1;
    if(NULL == value)
    {
        LOGE("%s return, reason: value is NULL", __func__);
        return -EINVAL;
    }
    readimage_fd = openat(device->sysfs_fd, "read_image_flag", O_RDONLY);
    if (readimage_fd < 0)
    {
        LOGE("%s openat failed %i\n", __func__, errno);
        status = -EIO;
        goto out;
    }
    status = read(readimage_fd, value, (strlen(value) + 1));
    if (status < 0)
    {
        LOGE("%s read failed %i\n", __func__, errno);
        goto out;
    }
out:
    if (readimage_fd >= 0)
    {
        close(readimage_fd);
    }
    return status;
}

int bf_ree_write_readimage_flag(bf_ree_device_t* device)
{
    int flag_fd = -1;
    int status = 0;

    if( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
    flag_fd = openat(device->sysfs_fd, "read_image_flag", O_RDONLY | O_NONBLOCK);
    if (flag_fd == -1) 
    {
        status = -errno;
        LOGE("bf open read_image_flag error %d ",status);
        goto out;
    }
   status = sysfs_node_write(device, "read_image_flag", "1");
out:
    if (flag_fd != -1)
    {
        close(flag_fd);
    }
    return status;
}
int bf_ree_device_set_cancel(bf_ree_device_t* device)
{
    uint8_t byte = 1;
	LOGE("%s+++",__func__);
    if( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
    
    if (write(device->cancel_fds[1], &byte, sizeof(byte)) != sizeof(byte))
    {
        LOGE("%s write failed %i\n", __func__, errno);
        return -EIO;
    }
    return 0;
}


int bf_ree_device_clear_cancel(bf_ree_device_t* device)
{
    uint8_t byte = 0;
	LOGE("%s+++",__func__);
    if( NULL == device )
    {
        LOGE("%s return, reason: device is NULL", __func__);
        return -EINVAL;
    }
    
    if (read(device->cancel_fds[0], &byte, sizeof(byte)) < 0)
    {
        LOGE("%s read failed %i\n", __func__, errno);
        return -EIO;
    }
    return 0;
}
static int dev_phys_path_by_attr(const char *attr_val,
                                 const char *base, char *path, int path_max) {
    int rc = 0;
    int found = 0;
    DIR *dir = NULL;
    struct dirent *item = NULL;

    LOGD("%s", __func__);

    if( NULL == attr_val || NULL == base || NULL == path )
    {
        LOGE("%s return, reason: attr_val or base or path is NULL", __func__);
        return -EINVAL;
    }
    
    dir = opendir(base);
    if (!dir)
    {
        LOGE("Unable to open '%s'", base);
        return -1;
    }

    while (NULL != (item = readdir(dir)))
    {
        if (item->d_type != DT_DIR && item->d_type != DT_LNK)
            continue;

        if (item->d_name[0] == '.')
            continue;

        found = (strstr(item->d_name, attr_val) != NULL);

        if (found)
        {
            rc = snprintf(path, path_max, "%s/%s",
                          base, item->d_name);
            if (rc >= path_max)
            {
                LOGE("Entry name truncated '%s'", path);
                closedir(dir);
                return -1;
            }
            LOGD("dir='%s' found on  path '%s'",
                       item->d_name, path);
            break;
        }
    }
    
    closedir(dir);
    return found;
}


static int sysfs_node_write(bf_ree_device_t* irq_device, const char* name,
                                const char* value)
{
    int status = 0;

    if( NULL == irq_device || NULL == name || NULL == value )
    {
        LOGE("%s return, reason: irq_device or name or value is NULL", __func__);
        return -EINVAL;
    }

    int fd = openat(irq_device->sysfs_fd, name, O_WRONLY);
    if (fd == -1)
    {
        LOGE("%s openat failed %i\n", __func__, errno);
        status = -EIO;
        goto out;
    }

    int size = strlen(value);
    if (write(fd, value, size) != size)
    {
        LOGE("%s write failed %i\n", __func__, errno);
        status = -ENOSYS;
        goto out;
    }
out:
    if (fd != -1)
    {
        close(fd);
    }
    return status;
}
int nav_node_write(bf_ree_device_t* device, const char* value)
{
    int status = 0;
    if(NULL == value )
    {
        LOGE("%s return, reason: value is NULL", __func__);
        return -EINVAL;
    }
    status = sysfs_node_write(device, "nav", value);
    if(status < 0)
    {
        LOGE("sysfs_node_write failed %d\n",status);
    }
    return status;
}
int nav_node_read(bf_ree_device_t* device, char* value)
{
    int status = 0;
    int nav_fd = -1;
    if(NULL == value)
    {
        LOGE("%s return, reason: value is NULL", __func__);
        return -EINVAL;
    }
    nav_fd = openat(device->sysfs_fd, "nav", O_RDONLY);
    if (nav_fd < 0)
    {
        LOGE("%s openat failed %i\n", __func__, errno);
        status = -EIO;
        goto out;
    }
    status = read(nav_fd, value, (strlen(value) + 1));
    if (status < 0)
    {
        LOGE("%s read failed %i\n", __func__, errno);
        goto out;
    }
out:
    if (nav_fd >= 0)
    {
        close(nav_fd);
    }
    return status;
}
