
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
#include "bf_types.h"

const char* dev_name = BF_FP_DEV_NAME;//"/dev/fingerprint";


int file_descriptor_ = -1;
//define commands
#define BF_IOCTL_MAGIC_NO			0xFB
#define BF_IOCTL_INIT		            _IO(BF_IOCTL_MAGIC_NO,   0)
#define BF_IOCTL_EXIT					 _IO(BF_IOCTL_MAGIC_NO,  1)
#define BF_IOCTL_KEY_EVENT	       	    _IOW(BF_IOCTL_MAGIC_NO,  2,struct bf_key_event)
#define BF_IOCTL_CLEAR_POLLFLAG             _IOW(BF_IOCTL_MAGIC_NO,  3,uint32_t)

#define BF_IOCTL_ENABLE_INTERRUPT           _IOW(BF_IOCTL_MAGIC_NO,  4, uint32_t)
#define BF_IOCTL_DISABLE_INTERRUPT          _IOW(BF_IOCTL_MAGIC_NO,  5, uint32_t)

#define BF_IOCTL_ENABLE_POWER               _IOW(BF_IOCTL_MAGIC_NO,  6,uint32_t)
#define BF_IOCTL_DISABLE_POWER              _IOW(BF_IOCTL_MAGIC_NO,  7,uint32_t)

#define BF_IOCTL_ENABLE_SPI_CLOCK           _IOW(BF_IOCTL_MAGIC_NO,  8,uint32_t)
#define BF_IOCTL_DISABLE_SPI_CLOCK          _IOW(BF_IOCTL_MAGIC_NO,  9,uint32_t)

#define BF_IOC_RESET			    		_IO(BF_IOCTL_MAGIC_NO,   10)
#define BF_IOCTL_SET_RST_GPIOVALUE            _IOW(BF_IOCTL_MAGIC_NO,  11,uint32_t)
#define BF_IOCTL_GET_RST_GPIOVALUE            _IOW(BF_IOCTL_MAGIC_NO,  12,uint32_t)
#define BF_IOCTL_GET_IRQ_GPIOVALUE             _IOW(BF_IOCTL_MAGIC_NO,  13,uint32_t)

#define BF_IOCTL_SET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  14,uint32_t)
#define BF_IOCTL_GET_WORK_STATE             _IOW(BF_IOCTL_MAGIC_NO,  15,uint32_t)
#define BF_IOCTL_SET_WORK_STATE_NOT_NOTIFY  _IOW(BF_IOCTL_MAGIC_NO,  16,uint32_t)


int bf_io_init(void)
{
	int ret = -1;
	LOGD("<%s:%d>++", __func__, __LINE__);
	ret = ioctl(file_descriptor_, BF_IOCTL_INIT);
	if ( ret < 0)
    {
        LOGE("bf_io_init failed:%s, ret = %d\n", strerror(errno), ret);
    }
	return ret;
}

int bf_io_exit(void)
{
	int ret = -1;
	ret = ioctl(file_descriptor_, BF_IOCTL_EXIT);
	return ret;
}


/**
 * bf_key_event_ioctl  
 * @return: 1--success    0--evenValue=-1    -1:ioctl fail  
 */    
static int navi_key_type(int eventValue)
{
	int ntype;
	switch(eventValue)
	{
		case EVENT_UP:
		case EVENT_DOWN:
		case EVENT_LEFT:
		case EVENT_RIGHT:
			ntype = 2;
			break;
		case EVENT_LOST:
			ntype = 0;
			break;
		default:
			ntype = 1;
			break;
	}
	return ntype;
}

static int navi_key_code(int eventValue)
{
	int ncode;
	switch(eventValue)
	{
		case EVENT_UP:
			ncode = eventValue;
			break;
		case EVENT_DOWN:
			ncode = eventValue;
			break;
		case EVENT_LEFT:
			ncode = eventValue;
			break;
		case EVENT_RIGHT:
			ncode = eventValue;
			break;
		case EVENT_HOLD:
			ncode = eventValue;
			break;
		case EVENT_CLICK:
			ncode = eventValue;
			break;
		case EVENT_DCLICK:
			ncode = eventValue;
			break;
		default:
			ncode = 0;
			break;
	}
	return ncode;
}

int bf_key_event_ioctl(navigation_info *navi_info)
{
   int ret=0;
   struct bf_key_event key;
   BF_LOG("%d", navi_info->eventValue);
   if( (navi_info->eventValue) > 0 )
   {	
		key.code = navi_key_code(navi_info->eventValue);
		key.value = navi_key_type(navi_info->eventValue);
		if(ioctl(file_descriptor_,BF_IOCTL_KEY_EVENT,(unsigned long)&key) < 0)
		{
		   LOGE("<%s> Failed to send key event",__FUNCTION__);
		   ret=-1;
		} 
		else
		{
			ret=1;
			LOGE("<%s>send key event=%d success\n",__FUNCTION__,key.code);
		}
		navi_info->downValue = navi_info->eventValue;
   }else if(EVENT_LOST == navi_info->eventValue)//up
   {
		if(1 == navi_key_type(navi_info->downValue))
		{
	   		key.code = navi_key_code(navi_info->downValue);
			key.value = 0;
			if(ioctl(file_descriptor_,BF_IOCTL_KEY_EVENT,(unsigned long)&key) < 0)
			{
			   LOGE("<%s> Failed to send key event",__FUNCTION__);
			   ret=-1;
			} 
			else
			{
				ret=1;
				LOGE("<%s>send key event=%d success\n",__FUNCTION__,key.code);
			}
			navi_info->downValue = 0;
		}
   }
   return ret;   
}


static void check_fail_reason()
{
    struct stat buf;
    const char* path = dev_name;

    memset((void*)&buf, 0, sizeof(struct stat));
    if (0 > stat(dev_name, &buf))
    {
        LOGE("stat %s failed: %d", dev_name, errno);
    }
    else
    {
        LOGE("%s is %ld:%ld", dev_name, buf.st_uid, buf.st_gid);
    }

    memset((void*)&buf, 0, sizeof(struct stat));
    if (0 > stat(path, &buf))
    {
        LOGE("stat %s failed: %d", path, errno);
    }
    else
    {
        LOGE("%s is %lu:%lu", path, buf.st_uid, buf.st_gid);
    }
}

int32_t fingerprint_open(void)
{

    if (file_descriptor_ < 0)
    {
        file_descriptor_ = open(dev_name, O_RDWR);
    }


    if (file_descriptor_ < 0)
    {
        check_fail_reason();
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
    int ret = ioctl(file_descriptor_, BF_IOCTL_ENABLE_INTERRUPT, NULL);
    if ( ret < 0)
    {
        LOGE("bf enable_Irq_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }

    return ret;
}


int32_t disable_Irq_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_DISABLE_INTERRUPT, NULL);
    if (ret < 0)
    {
        LOGE("bf disable_Irq_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }

    return ret;
}

int32_t enable_spi_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_ENABLE_SPI_CLOCK, NULL);
    if ( ret < 0)
    {
        LOGE("bf enable_spi_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}
int32_t disable_spi_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_DISABLE_SPI_CLOCK, NULL);
    if (ret < 0)
    {
        LOGE("bf disable_spi_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t enable_power_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_ENABLE_POWER, NULL);
    if (ret < 0)
    {
        LOGE("bf enable_power_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}
int32_t disable_power_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_DISABLE_POWER, NULL);
    if (ret < 0)
    {
        LOGE("bf disable_power_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t do_hwreset_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOC_RESET, NULL);
    if (ret < 0)
    {
        LOGE("bf do_hwreset_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t clear_pollflag_ioctl(void)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_CLEAR_POLLFLAG, NULL);
    if (ret < 0)
    {
        LOGE("clear_pollflag_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t set_reset_gpio_high_ioctl(void)
{
	int value = 1;
    int ret = ioctl(file_descriptor_, BF_IOCTL_SET_RST_GPIOVALUE, &value);
    if (ret < 0)
    {
        LOGE("bf reset_gpio_high_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t set_reset_gpio_low_ioctl(void)
{
	int value = 0;
    int ret = ioctl(file_descriptor_, BF_IOCTL_SET_RST_GPIOVALUE, &value);
    if (ret < 0)
    {
        LOGE("bf reset_gpio_low_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t get_reset_gpio_value_ioctl(void)
{
	int value = 0;
    int ret = ioctl(file_descriptor_, BF_IOCTL_GET_RST_GPIOVALUE, &value);
    if (ret < 0)
    {
        LOGE("bf reset_gpio_low_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
        return ret;
    }
    return value;
}

int32_t get_irq_gpio_value_ioctl(void)
{
	int value = 0;
    int ret = ioctl(file_descriptor_, BF_IOCTL_GET_IRQ_GPIOVALUE, &value);
    if (ret < 0)
    {
        LOGE("bf reset_gpio_low_ioctl to call ioctl failed errno = %d, ret = %d\n", errno, ret);
        return ret;
    }
    return value;
}

int32_t set_dev_mmi_workstate(uint32_t value)
{
    int ret = ioctl(file_descriptor_, BF_IOCTL_SET_WORK_STATE_NOT_NOTIFY, &value);
    if (ret < 0)
    {
        LOGE("bf set_workstate_mmi to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

int32_t write_image_to_driver(char *buf,int size)
{
    int ret = write(file_descriptor_, buf, size);
    if (ret < 0)
    {
        LOGE("bf set_workstate_mmi to call ioctl failed errno = %d, ret = %d\n", errno, ret);
    }
    return ret;
}

void bf_ree_device_destroy(bf_ree_device_t* device)
{
	
	if(file_descriptor_ != -1)
	{	
		close(file_descriptor_);
		file_descriptor_ = -1;
	}
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

	file_descriptor_ = device->devfd = open(dev_name, O_RDWR);
	if (device->devfd < 0)
	{
        LOGE("%s Error can not open dev %s", __func__,dev_name);
	    errno = ENOENT;
        goto err1;
	}

    if (pipe(device->cancel_fds))
    {
        goto err2;
    }
    return device;
	
err2:
	close(file_descriptor_);
	file_descriptor_ = -1;
err1:
	free(device);
err:
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
            //LOGD("bf poll irq status=%d",pfd[0].revents);
            if(pfd[0].revents == POLLERR)
            	status = -EINVAL;
            else
            	status = 0;
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
    return status;
}
int bf_ree_device_wait_irq_paul(bf_ree_device_t* device)
{
	return bf_ree_device_wait_irq_paul_timeout(device, -1);
}

static int cancelflg = 0;

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
    cancelflg = 1;
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
    if(cancelflg == 1)
    {
    	cancelflg = 0;
		if (read(device->cancel_fds[0], &byte, sizeof(byte)) < 0)
		{
		    LOGE("%s read failed %i\n", __func__, errno);
		    return -EIO;
		}
    }
    return 0;
}

