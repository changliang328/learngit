#ifndef BF_TEE_SPI_H
#define BF_TEE_SPI_H

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/input.h>

//platform select
#define PLATFORM_MTK	0
#define PLATFORM_QCOM	1
#define PLATFORM_SPRD	2
#define PLATFORM_MT6739	3
#define PLATFORM_MT6797	4
#define PLATFORM_MTK_ANDROIDL	5
#define BL_PLATFORM 	PLATFORM_MTK

//headers include
#if  (BL_PLATFORM == PLATFORM_MTK) || (BL_PLATFORM == PLATFORM_MT6797)
#include "mt_spi.h"
#include <mach/mt_clkmgr.h>
	
#elif (BL_PLATFORM == PLATFORM_MT6739)
#include <linux/platform_data/spi-mt65xx.h>
#include <linux/clk.h>
struct mtk_spi {
	void __iomem *base;
	void __iomem *peri_regs;
	u32 state;
	int pad_num;
	u32 *pad_sel;
	struct clk *parent_clk, *sel_clk, *spi_clk;
	struct spi_transfer *cur_transfer;
	u32 xfer_len;
	struct scatterlist *tx_sgl, *rx_sgl;
	u32 tx_sgl_len, rx_sgl_len;
	const struct mtk_spi_compatible *dev_comp;
	u32 dram_8gb_offset;
};
#elif (BL_PLATFORM == PLATFORM_MTK_ANDROIDL)
#include <mach/mt_spi.h>
#define USE_BOARD_INFO	1
#endif

#define BF_DEV_NAME "blestech_fp"
#define BF_DEV_MAJOR 0	/* assigned */
#define BF_CLASS_NAME "blestech_fp"
#if (BL_PLATFORM != PLATFORM_MTK_ANDROIDL)
#define BF_PINCTRL
#endif
#define BF_SPI_SPEED_HZ	(7000000)

//#define NEED_OPT_POWER_ON_2V8
//#define NEED_OPT_POWER_ON_1V8

/*for power on*/
//#define NEED_OPT_POWER_ON	//power gpio
/* for netlink use */
//#define NEED_NETLINK_OPT
#define MAX_NL_MSG_LEN 16
#define NETLINK_BF  29

/*for kernel log*/
#define BLESTECH_LOG


#ifdef BLESTECH_LOG
#define BF_LOG(fmt,arg...)          do{printk("<bf_fp>[%s:%d]"fmt"\n",__func__, __LINE__, ##arg);}while(0)
#else
#define BF_LOG(fmt,arg...)   	   do{}while(0)
#endif

typedef enum 
{
	BF_NETLINK_CMD_BASE = 100,

	BF_NETLINK_CMD_TEST  = BF_NETLINK_CMD_BASE+1,
	BF_NETLINK_CMD_IRQ = BF_NETLINK_CMD_BASE+2,
	BF_NETLINK_CMD_SCREEN_OFF = BF_NETLINK_CMD_BASE+3,
	BF_NETLINK_CMD_SCREEN_ON = BF_NETLINK_CMD_BASE+4
}fingerprint_socket_cmd_t;

struct bf_key_event {
	int code;
	int value;  
};

struct bf_device {
	struct spi_device *spi;
	wait_queue_head_t intwait;
	struct input_dev *fp_inputdev;	
	u32 reset_gpio;
	u32 irq_gpio;
	u32 irq_num;
	u8 irq_count;
	u8 sig_count;
	u8 *tx_buf;
	u8 *rx_buf;
#ifdef NEED_OPT_POWER_ON_2V8
	u32 power_en_gpio;
#endif
#ifdef NEED_OPT_POWER_ON_1V8
	u32 power1v8_en_gpio;
#endif
	struct pinctrl *pinctrl_gpios;
	struct pinctrl_state *pins_default,*pins_fp_interrupt;	
	struct pinctrl_state *pins_reset_high, *pins_reset_low;
#ifdef NEED_OPT_POWER_ON_2V8
	struct pinctrl_state *pins_power_high, *pins_power_low;
#endifNEED_OPT_POWER_ON_1V8
	struct pinctrl_state
#ifdef  *pins_power_1v8_high, *pins_power_1v8_low;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#else
	struct notifier_block fb_notify;
#endif
#ifdef NEED_NETLINK_OPT
	/* for netlink use */
	struct sock *netlink_socket;
#endif
#if  (BL_PLATFORM == PLATFORM_MTK) || (BL_PLATFORM == PLATFORM_MT6797)||(BL_PLATFORM == PLATFORM_MTK_ANDROIDL)
	struct mt_chip_conf mtk_spi_config;
#elif (BL_PLATFORM == PLATFORM_MT6739)
	struct mtk_chip_config mtk_spi_config;
#endif
};

#endif

