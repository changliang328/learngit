/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/spi/spidev.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/miscdevice.h>


#include <linux/signal.h>
#include <linux/ctype.h>
#include <linux/wakelock.h>
#include <linux/kobject.h>
#include <linux/poll.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#else
#include <linux/notifier.h>
#endif

#include <net/sock.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/of_gpio.h>



/* MTK header */
#include <mach/irqs.h>
#include <mach/emi_mpu.h>
#include <mach/mt_clkmgr.h>
#include <mt_chip.h>

#include "bf_tee_spi.h"
#define USE_SPI1_4GB_TEST 0
//platform select
#define PLATFORM_MTK	0
#define PLATFORM_QCOM	1
#define PLATFORM_SPRD	2
#define BL_PLATFORM 	PLATFORM_MTK
#define BUF_SIZE (11*1024)

#define BF_IOCTL_MAGIC_NO			0xFB
#define BF_IOCTL_INIT		            _IO(BF_IOCTL_MAGIC_NO,   0)
#define BF_IOCTL_CAPTURE_MODE	            _IOW(BF_IOCTL_MAGIC_NO,  1, uint32_t)
#define BF_IOCTL_INTERRUPT_MODE		    _IOW(BF_IOCTL_MAGIC_NO,  2, uint32_t)
#define BF_IOCTL_CONTRAST_ADJUST            _IOW(BF_IOCTL_MAGIC_NO,  3, uint32_t)
#define BF_IOCTL_POWERDOWN_MODE	            _IO (BF_IOCTL_MAGIC_NO,  4)
#define BF_IOCTL_ENABLE_INTERRUPT           _IOW(BF_IOCTL_MAGIC_NO,  8, uint32_t)
#define BF_IOCTL_DISABLE_INTERRUPT          _IOW(BF_IOCTL_MAGIC_NO,  5, uint32_t)
#define BF_IOCTL_MULTIFUNCTIONAL_KEYCODE    _IOW(BF_IOCTL_MAGIC_NO,  6, uint32_t)
#define BF_IOCTL_TEST_MODE	                _IOWR(BF_IOCTL_MAGIC_NO, 7, uint32_t)
#define BF_IOCTL_GET_ID	                    _IOWR(BF_IOCTL_MAGIC_NO, 9, uint32_t)
#define BF_IOCTL_INIT_ARGS	                _IOWR(BF_IOCTL_MAGIC_NO, 11,uint32_t)
#define BF_IOCTL_GAIN_ADJUST                _IOWR(BF_IOCTL_MAGIC_NO, 12,uint32_t)
#define BF_IOCTL_ENABLE_POWER               _IOW(BF_IOCTL_MAGIC_NO,  13,uint32_t)
#define BF_IOCTL_DISABLE_POWER              _IOW(BF_IOCTL_MAGIC_NO,  14,uint32_t)
#define BF_IOCTL_ENABLE_SPI_CLOCK           _IOW(BF_IOCTL_MAGIC_NO,  15,uint32_t)
#define BF_IOCTL_DISABLE_SPI_CLOCK          _IOW(BF_IOCTL_MAGIC_NO,  16,uint32_t)
#define BF_IOCTL_KEY_EVENT	       	    _IOW(BF_IOCTL_MAGIC_NO,  17,struct bf_key_event)
#define BF_IOCTL_NAV_MODE                   _IOW(BF_IOCTL_MAGIC_NO,  18,uint32_t)
#define BF_IOCTL_INIT_REG                   _IOW(BF_IOCTL_MAGIC_NO,  19,uint32_t)
#define BF_IOCTL_IMG_AREA                   _IOW(BF_IOCTL_MAGIC_NO,  20,uint32_t)
#define BF_IOC_RESET						_IO(BF_IOCTL_MAGIC_NO,   21)
#define BF_IOCTL_CLEAR_POLLFLAG             _IOW(BF_IOCTL_MAGIC_NO,  22,uint32_t)
#define BF_IOCTL_SET_GPIO_RESET_HIGH             _IOW(BF_IOCTL_MAGIC_NO,  23,uint32_t)
#define BF_IOCTL_SET_GPIO_RESET_LOW             _IOW(BF_IOCTL_MAGIC_NO,  24,uint32_t)

#define BL229X_IOCTL_MAGIC_NO			0xFC
#define BL229X_GET_ID	                _IOWR(BL229X_IOCTL_MAGIC_NO, 9, u32)
#define BL229X_ENBACKLIGHT           	_IOW(BL229X_IOCTL_MAGIC_NO, 13, u32)
#define BL229X_ISBACKLIGHT           	_IOWR(BL229X_IOCTL_MAGIC_NO, 14, u32)
#define FB_EVENT_NOTIFIER
#define NEED_INPUT_EVENT_OPT
int g_bl229x_enbacklight = 1;
static int display_blank_flag = -1;
EXPORT_SYMBOL(g_bl229x_enbacklight);
extern wait_queue_head_t waiter;
#ifdef NEED_NETLINK_OPT
/* for netlink use */
static int g_pid;
#endif
static struct bf_device *g_bf_dev=NULL;
static struct kobject *bl_kobj = NULL;
static int g_poll_mask = 0;
#if (BL_PLATFORM == PLATFORM_MTK)
static struct mt_chip_conf spi_init_conf = {

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
#endif

/**
 * sysf node to check the interrupt status of the sensor, the interrupt
 * handler should perform sysf_notify to allow userland to poll the node.
 */
static ssize_t irq_get(struct device* device,
                       struct device_attribute* attribute,
                       char* buffer)
{
	struct bf_device *bf_dev = g_bf_dev;
    int irq = gpio_get_value(bf_dev->irq_gpio);

    //if ((1 == irq) && (fp_LCD_POWEROFF == atomic_read(&fingerprint->state)))
    {
        //adreno_force_waking_gpu();
    }

    return scnprintf(buffer, PAGE_SIZE, "%i\n", irq);
}

/**
 * writing to the irq node will just drop a printk message
 * and return success, used for latency measurement.
 */
static ssize_t irq_ack(struct device* device,
                       struct device_attribute* attribute,
                       const char* buffer, size_t count)
{
    //struct fp_data* fingerprint = dev_get_drvdata(device);
    BF_LOG("[%s]buffer=%s.\n", __func__, buffer);
    return count;
}

static DEVICE_ATTR(irq, S_IRUSR | S_IWUSR, irq_get, irq_ack);

#ifdef NEED_INPUT_EVENT_OPT
static int bf_create_inputdev(struct bf_device *bf_dev)
{
	bf_dev->fp_inputdev = input_allocate_device();
	if (!bf_dev->fp_inputdev) {
		BF_LOG("bf_dev->fp_inputdev create faile!\n");
		return -ENOMEM;
	}
	__set_bit(502,bf_dev->fp_inputdev->keybit);	//EVENT_HOLD
	__set_bit(601,bf_dev->fp_inputdev->keybit);	//EVENT_CLICK
	__set_bit(501,bf_dev->fp_inputdev->keybit);	//EVENT_DCLICK 
	__set_bit(511,bf_dev->fp_inputdev->keybit);	//EVENT_UP
	__set_bit(512,bf_dev->fp_inputdev->keybit);	//EVENT_DOWN
	__set_bit(513,bf_dev->fp_inputdev->keybit);	//EVENT_LEFT
	__set_bit(514,bf_dev->fp_inputdev->keybit);	//EVENT_RIGHT
	__set_bit(EV_KEY,bf_dev->fp_inputdev->evbit);
	__set_bit(KEY_F10,bf_dev->fp_inputdev->keybit);		//68
	__set_bit(KEY_F11,bf_dev->fp_inputdev->keybit);		//88
	__set_bit(KEY_F12,bf_dev->fp_inputdev->keybit);		//88
	__set_bit(KEY_CAMERA,bf_dev->fp_inputdev->keybit);	//212
	__set_bit(KEY_POWER,bf_dev->fp_inputdev->keybit);	//116
	__set_bit(KEY_PHONE,bf_dev->fp_inputdev->keybit);  //call 169
	__set_bit(KEY_BACK,bf_dev->fp_inputdev->keybit);  //call 158
	__set_bit(KEY_HOMEPAGE,bf_dev->fp_inputdev->keybit);  //call 158
	__set_bit(KEY_MENU,bf_dev->fp_inputdev->keybit);  //call 158
	__set_bit(KEY_F1,bf_dev->fp_inputdev->keybit);	//69
	__set_bit(KEY_F2,bf_dev->fp_inputdev->keybit);	//60
	__set_bit(KEY_F3,bf_dev->fp_inputdev->keybit);	//61
	__set_bit(KEY_F4,bf_dev->fp_inputdev->keybit);	//62
	__set_bit(KEY_F5,bf_dev->fp_inputdev->keybit);	//63
	__set_bit(KEY_F6,bf_dev->fp_inputdev->keybit);	//64
	__set_bit(KEY_F7,bf_dev->fp_inputdev->keybit);	//65
	__set_bit(KEY_F8,bf_dev->fp_inputdev->keybit);	//66
	__set_bit(KEY_F9,bf_dev->fp_inputdev->keybit);	//67
	__set_bit(KEY_UP,bf_dev->fp_inputdev->keybit);	//103
	__set_bit(KEY_DOWN,bf_dev->fp_inputdev->keybit);	//108
	__set_bit(KEY_LEFT,bf_dev->fp_inputdev->keybit);	//105
	__set_bit(KEY_RIGHT,bf_dev->fp_inputdev->keybit);	//106
	bf_dev->fp_inputdev->id.bustype = BUS_HOST;
	bf_dev->fp_inputdev->name = "blestech_inputdev";
	if (input_register_device(bf_dev->fp_inputdev)) {
		BF_LOG("%s, register inputdev failed\n", __func__);
		input_free_device(bf_dev->fp_inputdev);
		return -ENOMEM;
	}
	BF_LOG("blestech_fp add blestech_inputdev secussed.");
	return 0;
}
#endif
/*----------------------------------------------------------------------------*/
static int bf_hw_power(struct bf_device *bf_dev, bool enable)
{
#ifdef NEED_OPT_POWER_ON
#ifdef CONFIG_OF
	if (enable) {
		pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_high);
		//pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_1v8_high);
	} 
	else {
		pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_low);
		//pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_1v8_low);
	}
#else
	if (enable) {
		gpio_direction_output (bf_dev->power_en_gpio, 1);
	} 
	else {
		gpio_direction_output (bf_dev->power_en_gpio, 0);
	}
#endif
#endif
	return 0;
}

static int bf_hw_reset(struct bf_device *bf_dev)
{
#ifdef CONFIG_OF
	pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_low);
	mdelay(50);
	pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_high);
#else
	gpio_direction_output (bf_dev->reset_gpio, 0);
	mdelay(50);
	gpio_direction_output (bf_dev->reset_gpio, 1);
#endif
	return 0;
}

static int bf_reset_gpio_value(struct bf_device *bf_dev,int value)
{
#ifdef CONFIG_OF
	if (value) {
		pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_high);
	} 
	else {
		pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_low);
	}
#else
	if (value) {
		gpio_direction_output (bf_dev->reset_gpio, 1);
	} 
	else {
		gpio_direction_output (bf_dev->reset_gpio, 0);
	}
#endif
	return 0;
}

static void bf_enable_irq(struct bf_device *bf_dev)
{
	if (1 == bf_dev->irq_count) {
		BF_LOG("irq already enabled\n");
	} else {
		enable_irq(bf_dev->irq_num);
		bf_dev->irq_count = 1;
		BF_LOG(" enable interrupt!\n");
	}
}

static void bf_disable_irq(struct bf_device *bf_dev)
{
	if (0 == bf_dev->irq_count) {
		BF_LOG(" irq already disabled\n");
	} else {
		disable_irq(bf_dev->irq_num);
		bf_dev->irq_count = 0;
		BF_LOG(" disable interrupt!\n");
	}
}


static void bf_spi_clk_enable(struct bf_device *bf_dev, u8 onoff)
{
#ifdef CONFIG_MTK_CLKMGR
	if (onoff)
		enable_clock(MT_CG_PERI_SPI0, "spi");
	else
		disable_clock(MT_CG_PERI_SPI0, "spi");

#else
	/* changed after MT6797 platform */
	struct mt_spi_t *ms = NULL;
	static int count = 0;

	ms = spi_master_get_devdata(bf_dev->spi->master);

	if (onoff && (count == 0)) {
//		mt_spi_enable_clk(ms);    // FOR MT6797
		clk_enable(ms->clk_main); // FOR MT6755/MT6750
		count = 1;
	} else if ((count > 0) && (onoff == 0)) {
//		mt_spi_disable_clk(ms);    // FOR MT6797
		clk_disable(ms->clk_main); // FOR MT6755/MT6750
		count = 0;
	}
#endif
}


/* -------------------------------------------------------------------- */
/* fingerprint chip hardware configuration								           */
/* -------------------------------------------------------------------- */


static int bf_get_gpio_info_from_dts(struct bf_device *bf_dev)
{
#ifdef CONFIG_OF
	int ret;

	struct device_node *node = NULL;
	struct platform_device *pdev = NULL;


	node = of_find_compatible_node (NULL, NULL, "mediatek,betterlife-fp");
	if (node) {
		pdev = of_find_device_by_node (node);
		if (pdev) {
			bf_dev->pinctrl_gpios = devm_pinctrl_get (&pdev->dev);
			if (IS_ERR (bf_dev->pinctrl_gpios)) {
				ret = PTR_ERR (bf_dev->pinctrl_gpios);
				BF_LOG( "can't find fingerprint pinctrl");
				return ret;
			}
		} else {
			BF_LOG( "platform device is null");
		}
	} else {
		BF_LOG( "device node is null");
	}

	/* it's normal that get "default" will failed */
	bf_dev->pins_default = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "default");
	if (IS_ERR (bf_dev->pins_default)) {
		ret = PTR_ERR (bf_dev->pins_default);
		BF_LOG( "can't find fingerprint pinctrl default");
		/* return ret; */
	}
	bf_dev->pins_reset_high = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "reset_high");
	if (IS_ERR (bf_dev->pins_reset_high)) {
		ret = PTR_ERR (bf_dev->pins_reset_high);
		BF_LOG( "can't find fingerprint pinctrl pins_reset_high");
		return ret;
	}
	bf_dev->pins_reset_low = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "reset_low");
	if (IS_ERR (bf_dev->pins_reset_low)) {
		ret = PTR_ERR (bf_dev->pins_reset_low);
		BF_LOG( "can't find fingerprint pinctrl reset_low");
		return ret;
	}
	bf_dev->pins_fp_interrupt = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "fp_interrupt");
	if (IS_ERR (bf_dev->pins_fp_interrupt)) {
		ret = PTR_ERR (bf_dev->pins_fp_interrupt);
		BF_LOG( "can't find fingerprint pinctrl fp_interrupt");
		return ret;
	}

#ifdef NEED_OPT_POWER_ON
	bf_dev->pins_power_high = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "power_high");
	if (IS_ERR (bf_dev->pins_power_high)) {
		ret = PTR_ERR (bf_dev->pins_power_high);
		BF_LOG ("can't find fingerprint pinctrl power_high");
		return ret;
	}
	bf_dev->pins_power_low = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "power_low");
	if (IS_ERR (bf_dev->pins_power_low)) {
		ret = PTR_ERR (bf_dev->pins_power_low);
		BF_LOG ("can't find fingerprint pinctrl power_low");
		return ret;
	}
	bf_dev->pins_power_1v8_high = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "power_1v8_high");
	if (IS_ERR (bf_dev->pins_power_1v8_high)) {
		ret = PTR_ERR (bf_dev->pins_power_1v8_high);
		BF_LOG ("can't find fingerprint pinctrl pins_power_1v8_high");
		return ret;
	}
	bf_dev->pins_power_1v8_low = pinctrl_lookup_state (bf_dev->pinctrl_gpios, "power_1v8_low");
	if (IS_ERR (bf_dev->pins_power_1v8_low)) {
		ret = PTR_ERR (bf_dev->pins_power_1v8_low);
		BF_LOG ("can't find fingerprint pinctrl power_1v8_low");
		return ret;
	}
#endif
	BF_LOG( "get pinctrl success!");

	bf_dev->reset_gpio = of_get_named_gpio(node, "reset-gpio", 0);
	bf_dev->irq_gpio =  of_get_named_gpio(node, "int_gpio", 0);

	bf_dev->irq_num = irq_of_parse_and_map(node, 0);
#ifdef NEED_OPT_POWER_ON
	bf_dev->power_en_gpio =  of_get_named_gpio(node, "power-gpio", 0);
	bf_dev->power1v8_en_gpio =  of_get_named_gpio(node, "power-1v8-gpio", 0);
#endif


#endif
	return 0;

}

#ifdef NEED_NETLINK_OPT
/* -------------------------------------------------------------------- */
/* netlink functions                 */
/* -------------------------------------------------------------------- */
void bf_send_netlink_msg(struct bf_device *bf_dev, const int command)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	int ret;
	char data_buffer[2];

	BF_LOG("enter, send command %d",command);
	memset(data_buffer,0,2);
	data_buffer[0] = (char)command;
	if (NULL == bf_dev->netlink_socket) {
		BF_LOG("invalid socket");
		return;
	}

	if (0 == g_pid) {
		BF_LOG("invalid native process pid");
		return;
	}

	/*alloc data buffer for sending to native*/
	skb = alloc_skb(MAX_NL_MSG_LEN, GFP_ATOMIC);
	if (skb == NULL) {
		return;
	}

	nlh = nlmsg_put(skb, 0, 0, 0, MAX_NL_MSG_LEN, 0);
	if (!nlh) {
		BF_LOG("nlmsg_put failed");
		kfree_skb(skb);
		return;
	}

	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	*(char *)NLMSG_DATA(nlh) = command;
	*((char *)NLMSG_DATA(nlh)+1) = 0; 
	ret = netlink_unicast(bf_dev->netlink_socket, skb, g_pid, MSG_DONTWAIT);
	if (ret < 0) {
		BF_LOG("send failed");
		return;
	}

	BF_LOG("send done, data length is %d",ret);
	return ;
}

static void bf_recv_netlink_msg(struct sk_buff *__skb)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	char str[128];


	skb = skb_get(__skb);
	if (skb == NULL) {
		BF_LOG("skb_get return NULL");
		return;
	}

	if (skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(skb);
		//memcpy(str, NLMSG_DATA(nlh), sizeof(str));
		g_pid = nlh->nlmsg_pid;
		BF_LOG("pid: %d, msg: %s",g_pid, str);

	} else {
		BF_LOG("not enough data length");
	}

	kfree_skb(__skb);

}


static int bf_close_netlink(struct bf_device *bf_dev)
{
	if (bf_dev->netlink_socket != NULL) {
		netlink_kernel_release(bf_dev->netlink_socket);
		bf_dev->netlink_socket = NULL;
		return 0;
	}

	BF_LOG("no netlink socket yet");
	return -1;
}


static int bf_init_netlink(struct bf_device *bf_dev)
{
	struct netlink_kernel_cfg cfg;

	memset(&cfg, 0, sizeof(struct netlink_kernel_cfg));
	cfg.input = bf_recv_netlink_msg;

	bf_dev->netlink_socket = netlink_kernel_create(&init_net, NETLINK_BF, &cfg);
	if (bf_dev->netlink_socket == NULL) {
		BF_LOG("netlink create failed");
		return -1;
	}
	BF_LOG("netlink create success");
	return 0;
}
#endif

static irqreturn_t bf_eint_handler(int irq, void *data)
{
	struct bf_device *bf_dev = (struct bf_device *)data;
#ifdef NEED_NETLINK_OPT
	BF_LOG("++++irq_handler netlink send+++++");
	bf_send_netlink_msg(bf_dev, BF_NETLINK_CMD_IRQ);
#endif
	g_poll_mask = POLLERR;
	wake_up_interruptible(&bf_dev->intwait);
    //sysfs_notify(bl_kobj, NULL, dev_attr_irq.attr.name);
	bf_dev->sig_count++;
	BF_LOG("-----irq_handler netlink bf_dev->sig_count=%d-----",bf_dev->sig_count);
	return IRQ_HANDLED;
}



/* -------------------------------------------------------------------- */
/* file operation function                                                                                */
/* -------------------------------------------------------------------- */
static long bf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct bf_device *bf_dev = NULL;
	struct bf_key_event *key = NULL;
	int error = 0;
	unsigned long missing;

	bf_dev = (struct bf_device *)filp->private_data;

	/*
	if (_IOC_TYPE(cmd) != BF_IOCTL_MAGIC_NO){
		BF_LOG("Not blestech fingerprint cmd.");
		return -EINVAL;
	}
	 Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 
	if (_IOC_DIR(cmd) & _IOC_READ)
		error = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));

	if (error == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		error = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (error){
		BF_LOG("Not blestech fingerprint cmd direction.");
		return -EINVAL;
	}
	*/

	switch (cmd) {
		case BF_IOCTL_INIT:
			BF_LOG("BF_IOCTL_INIT: chip reset command\n");
			break;
		case BF_IOC_RESET:
			BF_LOG("BF_IOC_RESET: chip reset command\n");
			bf_hw_reset(bf_dev);
			break;

		case BF_IOCTL_ENABLE_INTERRUPT:
			BF_LOG("BF_IOCTL_ENABLE_INTERRUPT:  command\n");
			bf_enable_irq(bf_dev);
			break;

		case BF_IOCTL_DISABLE_INTERRUPT:
			BF_LOG("BF_IOCTL_DISABLE_INTERRUPT:  command\n");
			bf_disable_irq(bf_dev);
			break;

		case BF_IOCTL_ENABLE_SPI_CLOCK:
			BF_LOG("BF_IOCTL_ENABLE_SPI_CLOCK:  command\n");
			bf_spi_clk_enable(bf_dev, 1);
			break;

		case BF_IOCTL_DISABLE_SPI_CLOCK:
			BF_LOG("BF_IOCTL_DISABLE_SPI_CLOCK:  command\n");
			bf_spi_clk_enable(bf_dev, 0);
			break;

		case BF_IOCTL_ENABLE_POWER:
			BF_LOG("BF_IOCTL_ENABLE_POWER:  command\n");
			bf_hw_power(bf_dev,1);
			break;

		case BF_IOCTL_DISABLE_POWER:
			BF_LOG("BF_IOCTL_DISABLE_POWER:  command\n");
			bf_hw_power(bf_dev,0);
			break;
			
		case BF_IOCTL_KEY_EVENT:
#ifdef NEED_INPUT_EVENT_OPT
			BF_LOG("BF_IOCTL_INPUT_KEY:  command\n");
			/*malloc for *key*/
			key = kzalloc(sizeof(*key),GFP_KERNEL);
			if(NULL == key){
				BF_LOG("musktest:Failed to allocate mem for key event\n");
				error = -ENOMEM;
				break;
			}
			
			/*get key data from user space*/
			missing = copy_from_user(key,(u8 __user *)arg,sizeof(*key));
			if(missing != 0){
				BF_LOG("musktest:Failed to copy key from user space\n");
				error = -EFAULT;
				break;
			}
			
			/*report a whole key (keydown+keyup)*/
			if(key->value == 2){
				BF_LOG("musktest:a key event reported,key->code = %d,key->value = %d\n",key->code,key->value);
				//__set_bit(key->code,bf_dev->fp_inputdev->keybit);
				input_report_key(bf_dev->fp_inputdev,key->code,1);
				input_sync(bf_dev->fp_inputdev);
				input_report_key(bf_dev->fp_inputdev,key->code,0);
				input_sync(bf_dev->fp_inputdev);
				
			/*report a single key (keydown/keyup)*/
			}else{  
				BF_LOG("musktest:a key event reported,key->code = %d,key->value = %d\n",key->code,key->value);
				//__set_bit(key->code,bf_dev->fp_inputdev->keybit);
				input_report_key(bf_dev->fp_inputdev,key->code,key->value);
				input_sync(bf_dev->fp_inputdev);
			}
#endif
			break;
		case BF_IOCTL_CLEAR_POLLFLAG:
			g_poll_mask = 0;
			break;
		case BF_IOCTL_SET_GPIO_RESET_HIGH:			
			bf_reset_gpio_value(bf_dev, 1);
			break;
		case BF_IOCTL_SET_GPIO_RESET_LOW:			
			bf_reset_gpio_value(bf_dev, 0);
			break;
		case BL229X_GET_ID:
			chipid = 0x5183;//(*bl229x_read_chipid)(bl229x);
			if (copy_to_user((void __user*)arg,&chipid,sizeof(u32)*1) != 0 ){
			   error = -EFAULT;
			}
			break;
	case BL229X_ENBACKLIGHT:
		BF_LOG("BL229X_ENBACKLIGHT arg:%d\n", (int)arg);
		g_bl229x_enbacklight = (int)arg;
		wake_up_interruptible(&waiter);
		break;
	case BL229X_ISBACKLIGHT:
		BF_LOG("BL229X_ISBACKLIGHT\n");
		if (copy_to_user((void __user*)arg,&g_bl229x_enbacklight,sizeof(u32)*1) != 0 ){
		   error = -EFAULT;
		}
		break;
			
		default:
			BF_LOG("Supportn't this command(%x)\n",cmd);
			break;
	}

	return error;

}
#ifdef CONFIG_COMPAT
static long bf_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;

	retval = bf_ioctl(filp, cmd, arg);

	return retval;
}
#endif

/*----------------------------------------------------------------------------*/
static int bf_open(struct inode *inode, struct file *filp)
{
	struct bf_device *bf_dev = g_bf_dev;
	int status = 0;

	filp->private_data = bf_dev;
	BF_LOG( " Success to open device.");

	return status;
}

/*----------------------------------------------------------------------------*/
#if USE_SPI1_4GB_TEST
#include "mt_spi_hal.h"
#endif
/*----------------------------------------------------------------------------*/
int mtspi_set_dma_en(int mode)
{
#if (BL_PLATFORM == PLATFORM_MTK)
	struct mt_chip_conf* spi_par;
	spi_par = &g_bf_dev->mtk_spi_config;
	if (!spi_par) {
		return -1;
	}
	if (1 == mode) {
		if (spi_par->com_mod == DMA_TRANSFER) {
			return 0;
		}
		spi_par->com_mod = DMA_TRANSFER;
	} else {
		if (spi_par->com_mod == FIFO_TRANSFER) {
			return 0;
		}
		spi_par->com_mod = FIFO_TRANSFER;
	}

	spi_setup(g_bf_dev->spi);
#endif
	return 0;
}

#if USE_SPI1_4GB_TEST
static char *spi_tx_local_buf;
static char *spi_rx_local_buf;
static int spi_setup_xfer(struct bf_device *bf_dev)
{
	if (NULL == spi_tx_local_buf) {
		bf_dev->tx_buf = spi_tx_local_buf = (char *)ioremap_nocache(SpiDmaBufTx_pa, 0x4000);
		if (!spi_tx_local_buf) {
			BF_LOG("SPI Failed to dma_alloc_coherent()\n");
			return -ENOMEM;
		}
	}
	if (NULL == spi_rx_local_buf) {
		bf_dev->rx_buf = spi_rx_local_buf = (char *)ioremap_nocache(SpiDmaBufRx_pa, 0x4000);
		if (!spi_rx_local_buf) {
			BF_LOG("SPI Failed to dma_alloc_coherent()\n");
			return -ENOMEM;
		}
	}
	return 0;
}
#endif
int bl_spi_send_cmd(struct bf_device *bf_dev,u8 *tx,u8 *rx,u16 spilen)
{
    int ret=0;
    struct spi_message msg;
    struct spi_transfer xfer = {
        .tx_buf = tx,
        .rx_buf = rx,
        .len = spilen,
#if USE_SPI1_4GB_TEST
		.tx_dma = SpiDmaBufTx_pa,
		.rx_dma = SpiDmaBufRx_pa,
#endif
    };
	//hexdump(tx,spilen);
	if(spilen >= 8)
		mtspi_set_dma_en(1);
		
    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);
    ret= spi_sync(bf_dev->spi,&msg);
    
	if(spilen >= 8)
		mtspi_set_dma_en(0);

    return ret;
}
static ssize_t bf_write(struct file *file, const char *buff, size_t count, loff_t *ppos)
{
	u8 *tx_buf;
	tx_buf = g_bf_dev->tx_buf;
    BF_LOG("  count=%zu++\n",count);
    
	if(copy_from_user(tx_buf,buff,count)){
		return -EFAULT;
	}

	bl_spi_send_cmd(g_bf_dev, tx_buf, NULL, count);

    BF_LOG("  --\n");
    return 0;
}

static ssize_t bf_read(struct file *filp, char  *buff, size_t count, loff_t *ppos)
{
	u8 *rx_buf;
	u8 *tx_buf;
	tx_buf = g_bf_dev->tx_buf;
	rx_buf = g_bf_dev->rx_buf;
    BF_LOG("  count=%zu++\n",count);
    
	if(copy_from_user(tx_buf,buff,count)){
		return -EFAULT;
	}

	bl_spi_send_cmd(g_bf_dev, tx_buf, rx_buf, count);

	if(copy_to_user(buff,rx_buf,count)){
		return -EFAULT; 
	}
    BF_LOG("  --\n");

	return count;
}

static int bf_release(struct inode *inode, struct file *file)
{
	int status = 0 ;

	return status;
}

static unsigned int bf_poll(struct file *file, poll_table *wait)
{
	struct bf_device *bf_dev = file->private_data;
	unsigned int mask;

	poll_wait(file, &bf_dev->intwait, wait);

	if(gpio_get_value(bf_dev->irq_gpio) == 1)
		mask = POLLPRI;
	else
		mask = g_poll_mask;
	g_poll_mask = 0;
	return mask;
}

static const struct file_operations bf_fops = {
	.owner = THIS_MODULE,
	.open  = bf_open,
	.write = bf_write,
	.read  = bf_read,
	.poll  = bf_poll,
	.release = bf_release,
	.unlocked_ioctl = bf_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = bf_compat_ioctl,
#endif
};

static struct miscdevice bf_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bl229x",
	.fops = &bf_fops,
};

static int bf_remove(struct spi_device *spi)
{
	struct bf_device *bf_dev = g_bf_dev;

	/* make sure ops on existing fds can abort cleanly */
	if (bf_dev->irq_num) {
		free_irq(bf_dev->irq_num, bf_dev);
		bf_dev->irq_count = 0;
		bf_dev->irq_num= 0;
	}

#ifdef NEED_NETLINK_OPT
	bf_close_netlink(bf_dev);
#endif	

	bf_hw_power(bf_dev,0);
	bf_spi_clk_enable(bf_dev, 0);

	spi_set_drvdata(spi, NULL);
	bf_dev->spi = NULL;

	kfree(bf_dev);

	return 0;
}


static u8 spicmd_rsp[256];//for debug spi transfer
/* -------------------------------------------------------------------- */
static ssize_t bl_show_spicmd(struct device *ddri,struct device_attribute *attr,char *buf)
{
	int count = 0;
	int buflen = 0;
	int param_len = spicmd_rsp[0];
	for(count = 0; count < param_len; count++)
	{
		buflen += sprintf(buf + buflen, "rsp[%d]=%x\n", count, spicmd_rsp[count]);
	}

	return buflen;
}

static ssize_t bl_store_spicmd(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	char *next;
	int count=0;
	u8 param[256];
	int param_len = 0;

	param_len = simple_strtoul(buf, &next, 16);
	BF_LOG("\n spicmd len=%d\n ",param_len);
	while(*next == '.')
	{
		param[count] = simple_strtoul(next+1, &next, 16);
		BF_LOG("\nsky test param[%d]=%x \n ",count,param[count]);
		++count;
	}
	bl_spi_send_cmd(g_bf_dev, param, spicmd_rsp, count);

	for(count = 0; count < param_len; count++)
	{
		BF_LOG("rsp[%d]=%x\n", count, spicmd_rsp[count]);
	}
	spicmd_rsp[0] = param_len;
	return size;
}
static DEVICE_ATTR(spicmd, 0664, bl_show_spicmd, bl_store_spicmd);


/* -------------------------------------------------------------------- */
/**
 * sysf node to check the screen status,
 * handler should perform sysf_notify to allow userland to poll the node.
 */
static ssize_t btl_show_screenState(struct device* device,
                       struct device_attribute* attribute,
                       char* buffer)
{
    int screenstate =  display_blank_flag;

    return scnprintf(buffer, PAGE_SIZE, "%d", screenstate);
}

/**
 * writing to the screenstate node will just drop a printk message
 * and return success, used for latency measurement.
 */
static ssize_t btl_store_screenState(struct device* device,
                       struct device_attribute* attribute,
                       const char* buffer, size_t count)
{
    BF_LOG("[%s]buffer=%s.\n", __func__, buffer);
    return count;
}

static DEVICE_ATTR(screenstate, S_IRUSR | S_IWUSR, btl_show_screenState, btl_store_screenState);
/* -------------------------------------------------------------------- */

int bl_sysfs_init(struct bf_device *bf_dev)
{
    int ret;

    bl_kobj = kobject_create_and_add("bl_fingerprint_sysfs",NULL);
    if(bl_kobj == NULL) {
        BF_LOG("%s  subsystem_register failed\n",__func__);
        ret = -ENOMEM;
        return ret;
    }
    ret = sysfs_create_file(bl_kobj,&dev_attr_spicmd.attr);
   if(ret) {
        BF_LOG("%s sysfs_create_spicmd_file failed\n",__func__);
    }
    ret = sysfs_create_file(bl_kobj,&dev_attr_irq.attr);

    if(ret) {
        BF_LOG("%s sysfs_create_irq_file failed\n",__func__);
    }
    ret = sysfs_create_file(bl_kobj,&dev_attr_screenstate.attr);

    if(ret) {
        BF_LOG("%s sysfs_create_screenState_file failed\n",__func__);
    }
	
    return ret;
}

#if defined(FB_EVENT_NOTIFIER)
static int btl_fb_event_notify(struct notifier_block *self,unsigned long action, void *data)
 {
	struct fb_event *event = data;
	int blank_mode = *((int *)event->data);
	if(action == FB_EVENT_BLANK){
		switch (blank_mode) 
		{
			case FB_BLANK_UNBLANK:
				display_blank_flag = 100;
				sysfs_notify(bl_kobj, NULL, dev_attr_screenstate.attr.name);
				BF_LOG("screen on:display_blank_flag = %d\n",display_blank_flag);
				break;
			case FB_BLANK_POWERDOWN:
				display_blank_flag = 101;
				sysfs_notify(bl_kobj, NULL, dev_attr_screenstate.attr.name);
				BF_LOG("screen off:display_blank_flag = %d\n",display_blank_flag);
				break;
			default:
				break;
		}
	} 
   return NOTIFY_OK;
}

static struct notifier_block btl_fb_notifier = {
        .notifier_call = btl_fb_event_notify,
};
#endif


static int  bf_probe(struct spi_device *spi)
{
	struct bf_device *bf_dev = NULL;
	int status = -EINVAL;

	bf_dev = kzalloc(sizeof (struct bf_device), GFP_KERNEL);
	if (NULL == bf_dev){
		BF_LOG( "kzalloc bf_dev failed.");	
		status = -ENOMEM;
		goto err;
	}
	g_bf_dev=bf_dev;
#if USE_SPI1_4GB_TEST
	spi_setup_xfer(bf_dev);
#else
	bf_dev->tx_buf = (u8*)__get_free_pages(GFP_KERNEL,get_order(BUF_SIZE));
	bf_dev->rx_buf = (u8*)__get_free_pages(GFP_KERNEL,get_order(BUF_SIZE));
#endif

	status = bf_get_gpio_info_from_dts (bf_dev);
	if(status){
		return -1;
	}
	/* Initialize the driver data */
	BF_LOG( "bf config spi ");
	bf_dev->spi = spi;
	/* setup SPI parameters */
	bf_dev->spi->mode = SPI_MODE_0;
	bf_dev->spi->bits_per_word = 8;
	bf_dev->spi->max_speed_hz = 10 * 1000 * 1000;
	memcpy (&bf_dev->mtk_spi_config, &spi_init_conf, sizeof (struct mt_chip_conf));
	bf_dev->spi->controller_data = (void*)&bf_dev->mtk_spi_config;
	spi_setup (bf_dev->spi);    
	spi_set_drvdata (spi, bf_dev);

	bf_spi_clk_enable(bf_dev,1);

	status = misc_register(&bf_misc_device);
	if(status) {
		BF_LOG("bl229x_misc_device register failed\n");
		goto err;
	}


	BF_LOG("blestech_fp add secussed.");

#ifdef NEED_NETLINK_OPT
	/* netlink interface init */
	BF_LOG ("bf netlink config");
	if (bf_init_netlink(bf_dev) <0){
		BF_LOG ("bf_netlink create failed");
	}
#endif

	#ifdef NEED_INPUT_EVENT_OPT
 	bf_create_inputdev(bf_dev);
	#endif
	bf_dev->irq_count=0;
	bf_dev->sig_count=0;
	bf_hw_power(bf_dev, 1);
	bf_hw_reset(bf_dev);
	bf_enable_irq(bf_dev);

	init_waitqueue_head(&bf_dev->intwait);
	bl_sysfs_init(bf_dev);
	status = request_irq (bf_dev->irq_num, bf_eint_handler, IRQ_TYPE_EDGE_RISING , BF_DEV_NAME, bf_dev);
	if (!status)
		BF_LOG("irq thread request success!\n");
	else
		BF_LOG("irq thread request failed, retval=%d\n", status);
	BF_LOG ("---ok--");
#if defined(FB_EVENT_NOTIFIER)
	fb_register_client(&btl_fb_notifier);
#endif
	enable_irq_wake(bf_dev->irq_num);
	return 0;

err:	
	return status;
}

#ifdef CONFIG_OF
static struct of_device_id bf_of_table[] = {
	{.compatible = "mediatek,fingerprint", },
	{.compatible = "betterlife,betterlife-fp",},
	{.compatible = "mediatek,betterlife-fp", },
	{},
};
MODULE_DEVICE_TABLE(of, bf_of_table);
#endif 

static struct spi_driver bf_driver = {
	.driver = {
		.name	= BF_DEV_NAME,
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = bf_of_table,
#endif
	},
	.probe	= bf_probe,
	.remove = bf_remove,
};


static int bf_spi_init(void)
{
	int status = 0;

	status = spi_register_driver(&bf_driver);
	if (status < 0)
		BF_LOG( "Failed to register SPI driver.\n");
	return status;
}
static void bf_spi_exit(void)
{
	spi_unregister_driver (&bf_driver);
}

module_init (bf_spi_init);
module_exit (bf_spi_exit);


MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("sunshizheng@blestech.com");
MODULE_DESCRIPTION ("Blestech fingerprint sensor TEE driver.");

