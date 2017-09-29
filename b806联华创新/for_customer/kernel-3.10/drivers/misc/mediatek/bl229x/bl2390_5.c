/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
* 
*/

/*
* Revision: 1.0
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
#include <mach/irqs.h>
#include <linux/kthread.h>
#include <mach/mt_spi.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mach/mt_gpio.h>
#include <mach/emi_mpu.h>
#include <mach/mt_clkmgr.h>
#include <linux/spi/spidev.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/version.h>

#include <linux/wait.h>
#include <linux/input.h>
#include <linux/ctype.h>
#include <linux/earlysuspend.h>
#include <linux/miscdevice.h>
#include <linux/kobject.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <linux/wakelock.h>
#include "xz_sensor.h"

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#include <linux/delay.h>


#define SPI_DRV_NAME	"bl229x"
#define bl229x_height  96
#define bl229x_width   112
#define bl229x_image_size (bl229x_height*bl229x_width)
#define DMA_TRANSFER_SIZE (11264)

#define BL229X_SPI_CLOCK_SPEED 6*1000*1000//10*1000*1000

#define READIMAGE_BUF_SIZE	(12288) 

#define BL229X_IOCTL_MAGIC_NO			0xFC

#define INIT_BL229X				        _IO(BL229X_IOCTL_MAGIC_NO, 0)
#define BL229X_GETIMAGE			        _IOW(BL229X_IOCTL_MAGIC_NO, 1, u32)
#define BL229X_INITERRUPT_MODE		    _IOW(BL229X_IOCTL_MAGIC_NO, 2, u32)
#define BL229X_CONTRAST_ADJUST          _IOW(BL229X_IOCTL_MAGIC_NO, 3, u32)
#define BL229X_CONTRAST_ADJUST2		    _IOW(BL229X_IOCTL_MAGIC_NO, 3, u8)

#define BL229X_POWERDOWN_MODE1			_IO (BL229X_IOCTL_MAGIC_NO, 4)
#define BL229X_POWERDOWN_MODE2			_IO (BL229X_IOCTL_MAGIC_NO, 5)

#define BL229X_INTERRUPT_FLAGS1         _IOW(BL229X_IOCTL_MAGIC_NO, 4,  u32)
#define BL229X_INTERRUPT_FLAGS2         _IOW(BL229X_IOCTL_MAGIC_NO, 5,  u32)
#define BL229X_MULTIFUNCTIONAL_KEYCODE	_IOWR(BL229X_IOCTL_MAGIC_NO, 6, u32)
#define BL229X_TEST_MODE	            _IOWR(BL229X_IOCTL_MAGIC_NO, 7, u32)
#define BL229X_GET_ID	                _IOWR(BL229X_IOCTL_MAGIC_NO, 9, u32)
#define BL229X_GET_CHIP_INFO	        _IOWR(BL229X_IOCTL_MAGIC_NO, 14, u32)
#define BL229X_INIT_ARGS	            _IOWR(BL229X_IOCTL_MAGIC_NO, 11, u32)
#define BL229X_GAIN_ADJUST              _IOWR(BL229X_IOCTL_MAGIC_NO, 12, u32)



#define CHIP_ID_LOW		                (0x83)
#define CHIP_ID_HIGH	                (0x51)
#define GPIO_OUT_ZERO	                (0)
#define GPIO_OUT_ONE	                (1)


#define CHIP_VERSION                    (2)


#define BTL_DEBUG(fmt,arg...)          do{\
	if(bl229x_log)\
	printk("<btl-dbg>[%s:%d]"fmt"\n",__func__, __LINE__, ##arg);\
}while(0)

	
#define DRIVER_DEBUG            (1)
#define FP_DET_BY_FD            (0)
#define FP_DET_BY_NAV           (1)

#define INT_MO
#define FP_DETECT_METHOD        (FP_DET_BY_FD) //0 FD:mode, 1: NAV MODE


#define RESET_PIN_FAILED	(1)
#define SPI_PIN_FAILED		(2)
#define INT_PIN_FAILED		(3)

#define PRESS_MAX_TIMEOUT_COUNT	    (50)
#define ESD_RECOVERY_RETRY_TIMES    (3)

#define REPORT_DELAY_TIME              (20)

// sensor parameter
#define SENSOR_DEFAULT_GAIN_V1	       (0xB6)	//(0xB6)	//B6
#define SENSOR_DEFAULT_GAIN_V2	       (0xB6)	//(0xB6)	//B6
#define SENSOR_DEFAULT_GAIN_V3		   (0xB5)	
// AGC
#define SENSOR_DEFAULT_CONTRAST_V1     (80)
#define SENSOR_DEFAULT_CONTRAST_V2     (64)
#define SENSOR_DEFAULT_CONTRAST_V3     (191)//(0x8c)	
//#define SENSOR_DEFAULT_CONTRAST (0x70)
#define SENSOR_DEFAULT_DACN_V2		   (0x1)
#define SENSOR_DEFAULT_DACN_V3		   (0xff)	// DACN (N should be larger than P)

#define REPORT_KEY_DEBOUNCE_TIME    (HZ/10)

#define INIT_PARAM_SIZE              22


/*----------------------------------------------------------------------------*/
typedef struct bl229x_chip_info {
    u32 chip_id_num;         //chip id number
    u8 chip_type;            //chip type a/b/c...etc
    u8 chip_driver_type;     //active or inactive
} chip_info_t;
typedef struct bl229x_chip_params {
    u8 *m_chip_name;
    u32 m_contrast;
    u32 m_gain;
    u32 m_dacn;
} chip_params_t;
typedef enum {
    BTL_FP_BL229X=0,
    BTL_FP_BL2390E=1,
    BTL_FP_BL2180 = 2,
    BTL_FP_BF3290=3,
    BTL_FP_BF3180 = 4,
    BTL_FP_MAX=100,
} bl229x_chip_name_t;

typedef enum {
    SENSOR_T_MODE_IDLE = 0,
    SENSOR_T_MODE_FP,
    SENSOR_T_MODE_CAP,
    SENSOR_T_MODE_NAV,
    SENSOR_T_MODE_INI,
    SENSOR_T_MODE_RESET,
} sensor_testmode_t;


typedef enum {
    INT_MODE_NONE     = 0,
    INT_MODE_KEY  = 1
} mode_type;


//module_param(bl229x_log, int, 00664);
extern void mt_eint_ack(unsigned int eint_num);
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);

struct bl229x_data {
	struct spi_device *spi;
	u8 *image_buf;
	u8 *imagetxcmd;
	u8 *imagerxpix;
	struct semaphore mutex;
	struct early_suspend early_suspend;
    struct semaphore handler_mutex;
    u32 reset_gpio;
    u32 irq_gpio;
    u32 irq_num;
    u32 power_en_gpio;
	u8  interrupt_mode; // 0: none interrupt; 1: interrupt
	u8  is_frame_ready;
	u32 contrast;
	u32 agc; 
	s32 report_key;
	s32 report_delay;
	s32 reset;
	u8  opened;
	u8  fp_detect_method;
	u8  interrupt_coming;
	unsigned long report_timeout;
	struct mutex  spi_lock;
    struct task_struct* pIntThread;
	chip_info_t  chip_info;
    chip_params_t  chip_params;
};

static int  mt_spi_init(void);
static void  mt_spi_exit(void);
static int   bl229x_probe(struct spi_device *spi);
static int  bl229x_open(struct inode *inode, struct file *file);
static ssize_t bl229x_write(struct file *file, const char *buff,size_t count, loff_t *ppos);
static ssize_t bl229x_read(struct file *file, char *buff,size_t count, loff_t *ppos);
static int spi_send_cmd(struct bl229x_data *bl229x,u8 *tx,u8 *rx,u16 spilen);
static int  bl229x_dev_init(struct bl229x_data *spidev);
static int  bl229x_dev_capture_init(struct bl229x_data *bl229x,u32 timeout);

static long bl229x_ioctl(struct file *filp, unsigned int cmd,unsigned long arg) ;

static int  bl229x_read_image(struct bl229x_data *bl229x,uint32_t timeout);
static int  bl229x_release(struct inode *inode, struct file *file);
static void bl229x_eint_gpio_init(void);
static void bl229x_eint_handler(void);
static int bl229x_create_inputdev(void);
static int bl229x_thread_func(void *unused);
static int  bl229x_dev_interrupt_init(struct bl229x_data *bl229x, int mode);
static int spi_send_cmd_fifo(struct bl229x_data *bl229x,u8 *tx,u8 *rx,u16 spilen);
static int mtspi_set_mode(int mode);
static int bl229x_clear_interrupt(struct bl229x_data *bl229x);
static int bl229x_power_on(struct bl229x_data *bl229x,bool enable);

static int bl229x_read_chipid(struct bl229x_data *bl229x);

static int bl229x_suspend(struct device *dev);
static int bl229x_resume(struct device *dev);


static atomic_t suspended;
static struct bl229x_data *g_bl229x= NULL;
static struct input_dev *bl229x_inputdev = NULL;
static struct kobject *bl229x_kobj=NULL;
static u8 bl229x_log = DRIVER_DEBUG;
static int inttrupt_enabled;
static u8 spicmd_rsp[65];

static DECLARE_WAIT_QUEUE_HEAD(frame_waiter);
static DECLARE_WAIT_QUEUE_HEAD(int_waiter);


static struct mt_chip_conf spi_conf=
{

	.setuptime = 10,
	.holdtime = 10,
	.high_time = 8, //此处决定slk的频率
	.low_time =  8,
	.cs_idletime = 20, //10,
	//.ulthgh_thrsh = 0,

	.cpol = 0,
	.cpha = 0,

	.rx_mlsb = 1,  //先传高位
	.tx_mlsb = 1,

	.tx_endian = 0, //tx_endian 表示大端模式
	.rx_endian = 0,

	.com_mod = DMA_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,	


};

static const struct dev_pm_ops bl229x_pm = {
   .suspend = bl229x_suspend,
   .resume = bl229x_resume
};

static struct spi_driver bl229x_driver = {
	.driver = {
		.name	= "bl229x",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
    	.pm = &bl229x_pm,
	},
	.probe	= bl229x_probe,
};

static struct spi_board_info spi_board_bl229x[] __initdata = {
	[0] = {
		.modalias= "bl229x",
		.bus_num = 0,
		.chip_select=0,
		.mode = SPI_MODE_0,
		.max_speed_hz = 16000000,
	},
};
struct fingerprintd_params_t{
 u8 def_contrast;                   //contrast's default for captruing image
 u8 def_ck_fingerup_contrast;       //contrast's value for checking if finger up
 u8 def_enroll_ck_fingerup_timeout;   
 u8 def_match_ck_fingerup_timeout;                
 u8 def_match_failed_times;         //max failed times for match
 u8 def_enroll_try_times;           //tried times for balance of wet and dry when enroll
 u8 def_match_try_times;            //tried times for balance of wet and dry when match
 u8 def_intensity_threshold;        //intensity's threshold
 u8 def_contrast_high_value;        // high threshold of contrast
 u8 def_contrast_low_value;         // low threshold of contrast
 u8 def_match_quality_score_threshold; //score's threshold of quality, when match
 u8 def_match_quality_area_threshold;  //area's threshold of quality, when match
 u8 def_enroll_quality_score_threshold;//score's threshold of quality, when enroll
 u8 def_enroll_quality_area_threshold; //area's threshold of quality, when match
 u8 def_shortkey_disable;              //short key switch 
 u8 def_far_rate;                          
 u8 def_max_samples;                   // max sample for a finger
 u8 def_debug_enable;
 u8 def_contrast_direction;
 u8 def_update;
 u8 def_step_counts;
 u8 def_algorithm_type;
 u16 reserved1;
};


static struct fingerprintd_params_t fingerprintdParams = {
  .def_contrast = 100,
  .def_ck_fingerup_contrast = 100,
    .def_enroll_ck_fingerup_timeout = 150, //ms
    .def_match_ck_fingerup_timeout = 150,    //ms
  .def_match_failed_times = 5,
    .def_enroll_try_times = 5,
  .def_match_try_times = ５,
  .def_intensity_threshold = 10,
  .def_contrast_high_value = 176,
  .def_contrast_low_value = 68,
  .def_match_quality_score_threshold  = 50,
    .def_match_quality_area_threshold   = 18,
  .def_enroll_quality_score_threshold = 65,
    .def_enroll_quality_area_threshold  = 21,
  .def_shortkey_disable = 0,
    .def_far_rate = 14,
    .def_max_samples = 8,
  .def_debug_enable = DRIVER_DEBUG,
  .def_contrast_direction = 1,
    .def_update = 1,
    .def_step_counts = 4,
    .def_algorithm_type = 10,
    .reserved1 = 800,
};

struct wake_lock fp_suspend_lock;

/*----------------------------------------------------------------------------*/
#define	GPIO_FP_INT_PIN			    (GPIO2 | 0x80000000)//GPIO_FINGER_PRINT_EINT//(GPIO60 | 0x80000000)
#define	CUST_EINT_FP_INT_NUM		2//CUST_EINT_FINGER_NUM
#define	GPIO_FP_INT_PIN_M_EINT		GPIO_MODE_00

#define GPIO_FINGER_PRINT_RESET     (GPIO3 | 0x80000000)

#if defined(GPIO_SPI_CS_PIN)
#define		FPS11XX_SPI_SCK_PIN		GPIO_SPI_SCK_PIN             
#define		FPS11XX_SPI_SCK_PIN_M_GPIO	GPIO_SPI_SCK_PIN_M_GPIO
#define		FPS11XX_SPI_SCK_PIN_M_SPI_CK	GPIO_SPI_SCK_PIN_M_SPI_CKA
#define		FPS11XX_SPI_CS_PIN		GPIO_SPI_CS_PIN
#define		FPS11XX_SPI_CS_PIN_M_GPIO	GPIO_SPI_CS_PIN_M_GPIO
#define		FPS11XX_SPI_CS_PIN_M_SPI_CS	GPIO_SPI_CS_PIN_M_SPI_CSA

#define		FPS11XX_SPI_MOSI_PIN		GPIO_SPI_MOSI_PIN
#define		FPS11XX_SPI_MOSI_PIN_M_GPIO	GPIO_SPI_MOSI_PIN_M_GPIO
#define		FPS11XX_SPI_MOSI_PIN_M_SPI_MOSI	GPIO_SPI_MOSI_PIN_M_SPI_MOA//MOA

#define		FPS11XX_SPI_MISO_PIN		GPIO_SPI_MISO_PIN
#define		FPS11XX_SPI_MISO_PIN_M_GPIO	GPIO_SPI_MISO_PIN_M_GPIO
#define		FPS11XX_SPI_MISO_PIN_M_SPI_MISO	GPIO_SPI_MISO_PIN_M_SPI_MIA //MIA

#else

#define		FPS11XX_SPI_SCK_PIN		GPIO_SPI_SCK_PIN             
#define		FPS11XX_SPI_SCK_PIN_M_GPIO	GPIO_SPI_SCK_PIN_M_GPIO
#define		FPS11XX_SPI_SCK_PIN_M_SPI_CK	GPIO_SPI_SCK_PIN_M_SPI_CKA
#define		FPS11XX_SPI_CS_PIN		GPIO_SPI_CS_PIN
#define		FPS11XX_SPI_CS_PIN_M_GPIO	GPIO_SPI_CS_PIN_M_GPIO
#define		FPS11XX_SPI_CS_PIN_M_SPI_CS	GPIO_SPI_CS_PIN_M_SPI_CSA

#define		FPS11XX_SPI_MOSI_PIN		GPIO_SPI_MOSI_PIN
#define		FPS11XX_SPI_MOSI_PIN_M_GPIO	GPIO_SPI_MOSI_PIN_M_GPIO
#define		FPS11XX_SPI_MOSI_PIN_M_SPI_MOSI	GPIO_SPI_MOSI_PIN_M_SPI_MIA//MOA

#define		FPS11XX_SPI_MISO_PIN		GPIO_SPI_MISO_PIN
#define		FPS11XX_SPI_MISO_PIN_M_GPIO	GPIO_SPI_MISO_PIN_M_GPIO
#define		FPS11XX_SPI_MISO_PIN_M_SPI_MISO	GPIO_SPI_MISO_PIN_M_SPI_MOA //MIA

#endif


/*----------------------------------------------------------------------------*/
static void exchange_white_black(u8 *dst,u8 *src,int len)
{
	int i = 0;
	for( ;i < len;i++)
	{
		*(dst + i) = 0xff & (0xff - *(src + i));
	}
}
// 配置 IO 口使其工作在 SPI 模式 	
static void set_spi_mode(int enable)
{
	if(enable)
	{

		mt_set_gpio_mode(FPS11XX_SPI_CS_PIN,FPS11XX_SPI_CS_PIN_M_SPI_CS);
		mt_set_gpio_pull_enable(FPS11XX_SPI_CS_PIN,GPIO_PULL_DISABLE);
		//mt_set_gpio_pull_select(FPS11XX_SPI_CS_PIN,GPIO_PULL_UP);

		mt_set_gpio_mode(FPS11XX_SPI_SCK_PIN,FPS11XX_SPI_SCK_PIN_M_SPI_CK);
		mt_set_gpio_pull_enable(FPS11XX_SPI_SCK_PIN,GPIO_PULL_DISABLE);
		//mt_set_gpio_pull_select(FPS11XX_SPI_SCK_PIN,GPIO_PULL_DOWN);

		mt_set_gpio_mode(FPS11XX_SPI_MISO_PIN,FPS11XX_SPI_MISO_PIN_M_SPI_MISO);
		mt_set_gpio_pull_enable(FPS11XX_SPI_MISO_PIN,GPIO_PULL_DISABLE);
		//mt_set_gpio_pull_select(FPS11XX_SPI_MISO_PIN,GPIO_PULL_UP);
		mt_set_gpio_mode(FPS11XX_SPI_MOSI_PIN,FPS11XX_SPI_MOSI_PIN_M_SPI_MOSI);
		mt_set_gpio_pull_enable(FPS11XX_SPI_MOSI_PIN,GPIO_PULL_DISABLE);
		//mt_set_gpio_pull_select(FPS11XX_SPI_MOSI_PIN,GPIO_PULL_UP); 

	}
	else{

		mt_set_gpio_mode(FPS11XX_SPI_CS_PIN,FPS11XX_SPI_CS_PIN_M_GPIO);
		mt_set_gpio_dir(FPS11XX_SPI_CS_PIN,GPIO_DIR_IN);
		mt_set_gpio_pull_enable(FPS11XX_SPI_CS_PIN,GPIO_PULL_DISABLE);

		mt_set_gpio_mode(FPS11XX_SPI_SCK_PIN,FPS11XX_SPI_SCK_PIN_M_GPIO);
		mt_set_gpio_dir(FPS11XX_SPI_SCK_PIN,GPIO_DIR_IN);
		mt_set_gpio_pull_enable(FPS11XX_SPI_SCK_PIN,GPIO_PULL_DISABLE);

		mt_set_gpio_mode(FPS11XX_SPI_MISO_PIN,FPS11XX_SPI_MISO_PIN_M_GPIO);
		mt_set_gpio_dir(FPS11XX_SPI_MISO_PIN,GPIO_DIR_IN);
		mt_set_gpio_pull_enable(FPS11XX_SPI_MISO_PIN,GPIO_PULL_DISABLE);

		mt_set_gpio_mode(FPS11XX_SPI_MOSI_PIN,FPS11XX_SPI_MOSI_PIN_M_GPIO);
		mt_set_gpio_dir(FPS11XX_SPI_MOSI_PIN,GPIO_DIR_IN);
		mt_set_gpio_pull_enable(FPS11XX_SPI_MOSI_PIN,GPIO_PULL_DISABLE);
	}
}

// 选择工作与那种模式
static int mtspi_set_mode(int mode)
{
	struct mt_chip_conf* spi_par;
	spi_par = &spi_conf;
	if (!spi_par)
	{
		return -1;
	}
	if (1 == mode)
	{
		if (spi_par-> com_mod == DMA_TRANSFER)
		{
			return 0;
		}
		spi_par-> com_mod = DMA_TRANSFER;
	}
	else
	{
		if (spi_par-> com_mod == FIFO_TRANSFER)
		{
			return 0;
		}
		spi_par-> com_mod = FIFO_TRANSFER; 
	}

	spi_setup(g_bl229x->spi);

	return 0;
}


static int spi_send_cmd(struct bl229x_data *bl229x,u8 *tx,u8 *rx,u16 spilen)
{
	int ret=0;
	struct spi_message m;
	struct spi_transfer t = {
		.cs_change = 0,
		.delay_usecs = 5,
		.speed_hz = BL229X_SPI_CLOCK_SPEED,
		.tx_buf = tx,
		.rx_buf = rx,
		.len = spilen,
		.tx_dma = 0,
		.rx_dma = 0,
		.bits_per_word = 0,
	};
	mutex_lock(&bl229x->spi_lock);

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);
    ret= spi_sync(bl229x->spi,&m);

	mutex_unlock(&bl229x->spi_lock);
    return ret;
}


static __attribute__((unused)) int spi_send_cmd_fifo(struct bl229x_data *bl229x,u8 *tx,u8 *rx,u16 spilen)
{
	int ret=0;
	struct spi_message m;
	struct spi_transfer t = {
		.cs_change = 0,
		.delay_usecs = 5,
		.speed_hz = BL229X_SPI_CLOCK_SPEED,
		.tx_buf = tx,
		.rx_buf = rx,
		.len = spilen,
		.tx_dma = 0,
		.rx_dma = 0,
		.bits_per_word = 0,
	};
	mtspi_set_mode(0);  // fifo 模式
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	ret= spi_sync(bl229x->spi,&m);
	return ret;   
}

//-------------------------------------------------------------------------------------------------
static u8 bl229x_spi_read_reg(u8 nRegID)
{
    u8 nAddr;
    u8 data_tx[4];
    u8 data_rx[4];

    nAddr = nRegID << 1;
    nAddr &= 0x7F;

    data_tx[0] = nAddr;
    data_tx[1] = 0xff;
    
    spi_send_cmd(g_bl229x,data_tx,data_rx,2);
    return data_rx[1];
}


/*----------------------------------------------------------------------------*/
static u8 bl229x_spi_write_reg(u8 nRegID, u8 value)
{
    u8 nAddr;
    u8 data_tx[4];
    u8 data_rx[4];

    nAddr = nRegID << 1;
    nAddr |= 0x80;

    data_tx[0] = nAddr;
    data_tx[1] = value;
   
    spi_send_cmd(g_bl229x,data_tx,data_rx,2);
    return data_rx[1];
}


/*----------------------------------------------------------------------------*/
static u8 getSensorInterruptStatus(struct bl229x_data *bl229x)
{
    u8 unStatus = 0;

    unStatus = bl229x_spi_read_reg(REGA_INTR_STATUS);
   
    BTL_DEBUG("nStatus=%2x",unStatus);
 
    return unStatus;
}
/*----------------------------------------------------------------------------*/
void hexdump(const unsigned char *buf, const int num)
{
    int i;
    for(i = 0; i < num; i++) {
        printk("%02X ", buf[i]);
        if ((i+1)%8 == 0)
            printk("\n");
    }
    printk("\n");
    return;
}


/*----------------------------------------------------------------------------*/
static __attribute__((unused)) u8*  spi_read_frame(struct bl229x_data *bl229x)
{
    u8 nAddr;

    nAddr = REGA_FINGER_CAP << 1;
    nAddr &= 0x7F;

    memset(bl229x->image_buf, 0xff, READIMAGE_BUF_SIZE);
    memset(bl229x->imagetxcmd, 0x66, READIMAGE_BUF_SIZE);
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
    bl229x_spi_write_reg(REGA_RX_DACP_LOW, 0xd0);
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_CAP);
    msleep(50);
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_PRINT);

    bl229x->imagetxcmd[0] = nAddr;
	mtspi_set_mode(1); 
    spi_send_cmd(bl229x, bl229x->imagetxcmd, bl229x->image_buf, DMA_TRANSFER_SIZE);
    //hexdump(imagebuf,bl229x_image_size);
    mtspi_set_mode(0);
    return (bl229x->image_buf);
}


/*----------------------------------------------------------------------------*/
static u8* spi_read_frame_now(struct bl229x_data *bl229x)
{
    u8 nAddr;
    u8 data_tx[4];
    u8 data_rx[4];

    BTL_DEBUG("%s++\n",__func__);

    memset(bl229x->image_buf, 0xff, READIMAGE_BUF_SIZE);
    memset(bl229x->imagetxcmd, 0xff, READIMAGE_BUF_SIZE);
    memset(bl229x->imagerxpix, 0xff, READIMAGE_BUF_SIZE);

    //spi_write_reg(REGA_HOST_CMD, MODE_FG_PRINT);
    nAddr = REGA_HOST_CMD << 1;
    nAddr |= 0x80;
    data_tx[0] = nAddr;
    data_tx[1] = MODE_FG_PRINT;
    spi_send_cmd(g_bl229x,data_tx,data_rx,2);

	
    nAddr = REGA_FINGER_CAP << 1;
    nAddr &= 0x7F;
    bl229x->imagetxcmd[0] = nAddr;
    mtspi_set_mode(1);
    //spi_send_cmd(bl229x, imagetxcmd, imagebuf, DMA_TRANSFER_SIZE);
    spi_send_cmd(bl229x,  bl229x->imagetxcmd,  bl229x->imagerxpix, READIMAGE_BUF_SIZE);
    //hexdump(imagerxpix,112);
    exchange_white_black(bl229x->image_buf + 2,bl229x->imagerxpix + 2, DMA_TRANSFER_SIZE - 2);//omit 2 dummy bytes
    //memcpy(imagebuf, imagerxpix, READIMAGE_BUF_SIZE);

    mtspi_set_mode(0);

	BTL_DEBUG("%s--\n",__func__);
    return (bl229x->image_buf);
}


/*----------------------------------------------------------------------------*/
static __attribute__((unused)) u8* spi_read_frame_int(struct bl229x_data *bl229x)
{
    u8 nAddr;

    nAddr = REGA_FINGER_CAP << 1;
    nAddr &= 0x7F;

    memset(bl229x->image_buf, 0xff, READIMAGE_BUF_SIZE);
    memset(bl229x->imagetxcmd, 0x66, READIMAGE_BUF_SIZE);
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_PRINT);

    bl229x->imagetxcmd[0] = nAddr;
    mtspi_set_mode(1);
    spi_send_cmd(bl229x,  bl229x->imagetxcmd,  bl229x->image_buf, DMA_TRANSFER_SIZE);
    mtspi_set_mode(0);
    return ( bl229x->image_buf);
}

/*----------------------------------------------------------------------------*/
static int hw_reset(struct bl229x_data *bl229x)
{
    //test reset pin
    u32 pin_val= -1;
    bl229x_power_on(bl229x, 0);
    msleep(200);
    pin_val = mt_get_gpio_in(GPIO_FINGER_PRINT_RESET);
    if(GPIO_OUT_ZERO != pin_val)
        return -RESET_PIN_FAILED;
    BTL_DEBUG("%s rst pin_val=%d\n",__func__,pin_val);
    bl229x_power_on(bl229x, 1);
    pin_val =  mt_get_gpio_in(GPIO_FINGER_PRINT_RESET);
    if(GPIO_OUT_ONE != pin_val)
        return -RESET_PIN_FAILED;
    BTL_DEBUG("%s rst pin_val=%d\n",__func__,pin_val);
    msleep(20);
    return 0;
}


/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_agc(struct device *ddri,struct device_attribute *attr,char *buf)
{
	bl229x_spi_write_reg(REGA_RX_DACP_LOW,g_bl229x->contrast);
    return sprintf(buf,"agc var=%x  agc reg=%x\n",g_bl229x->contrast, (u8)bl229x_spi_read_reg(REGA_RX_DACP_LOW));
}


static ssize_t bl229x_store_agc(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    u8 nAddr;
    u8 data_tx[4];
    u8 data_rx[4];

    char *next;
    BTL_DEBUG("[bl229x]%s:\n\n", __func__);
    g_bl229x->contrast = simple_strtoul(buf, &next, 16);
    
    nAddr = REGA_RX_DACP_LOW << 1;
    nAddr |= 0x80;
    data_tx[0] = nAddr;
    data_tx[1] = g_bl229x->contrast;
    spi_send_cmd(g_bl229x,data_tx,data_rx,2);

    return size;
}
static DEVICE_ATTR(agc,0664,bl229x_show_agc,bl229x_store_agc);

/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_interrupt_mode(struct device *ddri,struct device_attribute *attr,char *buf)
{
    bl229x_dev_init(g_bl229x);
	bl229x_clear_interrupt(g_bl229x);
	return 1;
}

static ssize_t bl229x_store_interrupt_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    inttrupt_enabled = simple_strtoul(buf, &next, 10);
	if (inttrupt_enabled)
		hw_reset(g_bl229x);
	else 
        bl229x_power_on(g_bl229x,inttrupt_enabled);

    //if(inttrupt_enabled)
    //    mt_eint_unmask(g_bl229x->irq_num);
    //else
    //r    mt_eint_mask(g_bl229x->irq_num);
    return size;
}
static DEVICE_ATTR(interrupt,0664,bl229x_show_interrupt_mode,bl229x_store_interrupt_mode);

/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_readimage(struct device *ddri,struct device_attribute *attr,char *buf)
{
    bl229x_read_image(g_bl229x,100);
    return 0;
}

static DEVICE_ATTR(readimage,S_IWUSR|S_IRUGO,bl229x_show_readimage,NULL);

/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_spicmd(struct device *ddri,struct device_attribute *attr,char *buf)
{
    int count = 0;
    int buflen = 0;
	int param_size = spicmd_rsp[0];
    for(count = 0; count < param_size; count++) {
        buflen += sprintf(buf + buflen, "rsp[%d]=%x\n", count, spicmd_rsp[count+1]);
    }

	return buflen;
}

static ssize_t bl229x_store_spicmd(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    int count=0;
    u8 param[256];
	u8 w = 0;
	u8 tempBuf[128];
	u8 param_size;


    memset(tempBuf,0,128); 
	memcpy(tempBuf,buf,size);
	BTL_DEBUG("string:%s \n",tempBuf);


	w =  simple_strtoul(tempBuf, &next, 16);
	BTL_DEBUG("w:%d \n",w);
	BTL_DEBUG("next=%s \n ",next);
    while(!isxdigit(*next) && *next != 0)next++;
	if (*next == 0) {
		BTL_DEBUG("format error \n");
		spicmd_rsp[0] = 0;
		return 1; 
	}
	param_size = simple_strtoul(next, &next, 16);
    BTL_DEBUG("spicmd len=%d \n",param_size);
	
	for (count = 0; count < param_size; count++){	
      while(!isxdigit(*next) && *next != 0) 
	  	  next++;
	  if (*next == 0) break;

	  param[count] = simple_strtoul(next, &next, 16);
      BTL_DEBUG("param[%d]=%x \n ",count,param[count]);
	  BTL_DEBUG("next=%s \n ",next);
	}
	
	if (count != param_size){
	    BTL_DEBUG("format error -2  \n");
		spicmd_rsp[0] = 0;
		return 1; 
	}

	param[0] = param[0] << 1;
	if (w)
	   param[0] |= 0x80;
	else 
	   param[0] &= 0x7f;

	for(count = 0; count < param_size; count++) {
        BTL_DEBUG("param[%d]=%x\n", count,param[count]);
    }

    spicmd_rsp[0] = param_size;

	spi_send_cmd(g_bl229x, param, &spicmd_rsp[1], count);

    for(count = 0; count < param_size; count++) {
        BTL_DEBUG("rsp[%d]=%x\n", count, spicmd_rsp[count+1]);
    }
  
    return size;
}
static DEVICE_ATTR(spicmd, 0664, bl229x_show_spicmd, bl229x_store_spicmd);


/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_key_interrupt(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf, "\nkey_interrupt=%d\n", g_bl229x->report_key );
}

static ssize_t bl229x_store_key_interrupt(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    g_bl229x->report_key  = simple_strtoul(buf, &next, 10);
    return size;
}

static DEVICE_ATTR(key_interrupt, 0664, bl229x_show_key_interrupt, bl229x_store_key_interrupt);
/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_report_delay(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf, "\nreport_delay=%d\n", g_bl229x->report_delay);
}

static ssize_t bl229x_store_report_delay(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    g_bl229x->report_delay = simple_strtoul(buf, &next, 10);
    return size;
}

static DEVICE_ATTR(report_delay, 0664, bl229x_show_report_delay, bl229x_store_report_delay);

/*----------------------------------------------------------------------------*/
static int  bl229x_dev_selftest(struct bl229x_data *bl229x)
{
    //复位信号
    u32 pin_val= -1;
    int chip_id = 0;
	
    BTL_DEBUG("[bl229x]%s:\n\n", __func__);
    //test reset pin
    hw_reset(bl229x);
    //test spi pins
    chip_id = bl229x_read_chipid(g_bl229x);
    if(chip_id < 0)
        return -SPI_PIN_FAILED;
    //---------------------------------------------
	bl229x_spi_write_reg(REGA_HOST_CMD,MODE_IDLE);

	bl229x_spi_write_reg(REGA_HOST_CMD,MODE_FG_CAP);

    msleep(10);
    pin_val = mt_get_gpio_in(GPIO_FP_INT_PIN);
    BTL_DEBUG("%s int pin_val=%d\n",__func__,pin_val);
    if(GPIO_OUT_ONE != pin_val)
        return -INT_PIN_FAILED;

    bl229x_dev_init(g_bl229x);
    bl229x_dev_interrupt_init(g_bl229x,g_bl229x->fp_detect_method);

    return 0;
}

static ssize_t bl229x_show_selftest(struct device *ddri,struct device_attribute *attr,char *buf)
{
    int ret = 0;
    mt_eint_mask(g_bl229x->irq_num);
    ret = bl229x_dev_selftest(g_bl229x);
    mt_eint_unmask(g_bl229x->irq_num);
    return sprintf(buf, "\nselftest=%d interrupt_mode_flag=%d\n", ret,g_bl229x->interrupt_mode);
}

static ssize_t bl229x_store_selftest(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    return size;
}

static DEVICE_ATTR(selftest, 0664, bl229x_show_selftest, bl229x_store_selftest);
static ssize_t bl229x_show_debug(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf,"debug=%d\n",bl229x_log);
}
static ssize_t bl229x_store_debug(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    bl229x_log = simple_strtoul(buf, NULL, 16);
    BTL_DEBUG("bl229x_log=0x%x",bl229x_log);
    return size;
}
static DEVICE_ATTR(debug,0664,bl229x_show_debug,bl229x_store_debug);

static ssize_t bl229x_show_register(struct device *ddri,struct device_attribute *attr,char *buf)
{

    int count = 0;
    int buflen = 0;
    int param_size = spicmd_rsp[0];
    for(count = 0; count < param_size; count++) {
        buflen += sprintf(buf + buflen, "rsp[%d]=%x\n", count, spicmd_rsp[count+1]);
    }
    return param_size;
}
static ssize_t bl229x_store_register(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    int count=0;
    u8 param[64];
    u8 w = 0;
    u8 tempBuf[64];
    u8 param_size;
    memset(tempBuf,0,64);
    memcpy(tempBuf,buf,size);
    memset(spicmd_rsp,0,65);
    BTL_DEBUG("string:%s \n",tempBuf);
    w =  simple_strtoul(tempBuf, &next, 16);
    BTL_DEBUG("w:%d \n",w);
    BTL_DEBUG("next=%s \n ",next);
	if (w)
       param_size = 2;
	else 
	   param_size = 1;
    BTL_DEBUG("spicmd len=%d \n",param_size);
    for (count = 0; count < param_size; count++) {
        while(!isxdigit(*next) && *next != 0)
            next++;
        if (*next == 0) break;
        param[count] = simple_strtoul(next, &next, 16);
        BTL_DEBUG("param[%d]=%x \n ",count,param[count]);
        BTL_DEBUG("next=%s \n ",next);
    }
    if (count != param_size) {
        BTL_DEBUG("format error -2  \n");
        return 1;
    }
    param[0] = param[0] << 1;
    if (w)
        param[0] |= 0x80;
    else
        param[0] &= 0x7f;
    if (param_size > 64)
        return 1;
    for(count = 0; count < param_size; count++) {
        BTL_DEBUG("param[%d]=%x\n", count,param[count]);
    }
    spicmd_rsp[0] = param_size;
    spi_send_cmd(g_bl229x, param, &spicmd_rsp[1], count);
    for(count = 0; count < param_size; count++) {
        BTL_DEBUG("rsp[%d]=%x\n", count, spicmd_rsp[count+1]);
    }
    return size;
}
static DEVICE_ATTR(register,0664,bl229x_show_register,bl229x_store_register);
static ssize_t bl229x_show_mode(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf,"idle:0,fd:2,cap:4,nav:5 current cmd reg:0x%x",bl229x_spi_read_reg(REGA_HOST_CMD));
}
static ssize_t bl229x_store_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    u8 register_value = simple_strtoul(buf, NULL, 16);
    BTL_DEBUG("host cmd reg:%d",register_value);
    switch (register_value) {
    case MODE_IDLE:
        bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
        break;
    case MODE_NAVI:
    case MODE_FG_DT:
        bl229x_dev_interrupt_init(g_bl229x,g_bl229x->fp_detect_method);
        break;
    case MODE_FG_CAP:
        bl229x_dev_capture_init(g_bl229x,10);
        break;
    default:
        break;
    }
    return size;
}
static DEVICE_ATTR(mode,0664,bl229x_show_mode,bl229x_store_mode);
static ssize_t bl229x_show_image(struct device *ddri,struct device_attribute *attr,char *buf)
{
    int i;
    for(i = 0; i < DMA_TRANSFER_SIZE; i++) {
        printk("%02X ", g_bl229x->image_buf[i]);
        if ((i+1)%16 == 0)
            printk("\n");
    }
    printk("\n");
    return sprintf(buf,"look image's content by log \n");
}
static DEVICE_ATTR(image,0664,bl229x_show_image,NULL);
static ssize_t bl229x_show_init(struct device *ddri,struct device_attribute *attr,char *buf)
{
    bl229x_dev_init(g_bl229x);
    return sprintf(buf,"device is re-inited, contrast=0x%x  gain=0x%x,dacn=0x%x\n",g_bl229x->contrast, g_bl229x->chip_params.m_gain,g_bl229x->chip_params.m_dacn);
}
static DEVICE_ATTR(init,0664,bl229x_show_init,NULL);
static int  bl229x_dev_reset(struct bl229x_data *bl229x)
{
    u32 pin_val= -1;
    int chip_id = 0;
    BTL_DEBUG("[bl229x]%s:\n\n", __func__);
	bl229x->chip_info.chip_type = 0;
	bl229x->chip_info.chip_driver_type = 0;
    hw_reset(bl229x);
    chip_id = bl229x_read_chipid(g_bl229x);
    if(chip_id < 0)
        return -SPI_PIN_FAILED;
    bl229x_spi_write_reg(REGA_HOST_CMD,MODE_IDLE);
    msleep(10);
    pin_val = mt_get_gpio_in(GPIO_FP_INT_PIN);
    BTL_DEBUG("%s int pin_val=%d\n",__func__,pin_val);
    if(GPIO_OUT_ZERO != pin_val)
        return -INT_PIN_FAILED;
	bl229x_dev_init(g_bl229x);
	bl229x_dev_interrupt_init(g_bl229x,g_bl229x->fp_detect_method);
    return 0;
}
static ssize_t bl229x_show_reset(struct device *ddri,struct device_attribute *attr,char *buf)
{
    bl229x_dev_reset(g_bl229x);
    return sprintf(buf,"driver type: %d, contrast=0x%x  gain=0x%x\n", g_bl229x->chip_info.chip_driver_type,bl229x_spi_read_reg(REGA_RX_DACP_LOW), bl229x_spi_read_reg(REGA_GC_STAGE));
}
static DEVICE_ATTR(reset,0664,bl229x_show_reset,NULL);
static ssize_t bl229x_show_dacn(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf,"dacn var=0x%x\n",g_bl229x->chip_params.m_dacn);
}
static ssize_t bl229x_store_dacn(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    u8 i;
    g_bl229x->chip_params.m_dacn = simple_strtoul(buf, NULL, 16);
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
    for (i = 0x1D; i <= 0x28; i++) {
        bl229x_spi_write_reg(i, g_bl229x->chip_params.m_dacn);
    }
    return size;
}
static DEVICE_ATTR(dacn,0664,bl229x_show_dacn,bl229x_store_dacn);
static ssize_t bl229x_show_gain(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf,"gain_var =0x%x  gain_reg=0x%x\n",g_bl229x->chip_params.m_gain, bl229x_spi_read_reg(REGA_GC_STAGE));
}
static ssize_t bl229x_store_gain(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    g_bl229x->chip_params.m_gain = simple_strtoul(buf, NULL, 16);
	g_bl229x->contrast = g_bl229x->chip_params.m_contrast;
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
    bl229x_spi_write_reg(REGA_GC_STAGE,g_bl229x->chip_params.m_gain);
    return size;
}
static DEVICE_ATTR(gain,0664,bl229x_show_gain,bl229x_store_gain);
static ssize_t bl229x_show_contrast(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf,"contrast_var=0x%x contrast_reg=0x%x\n",g_bl229x->contrast, bl229x_spi_read_reg(REGA_RX_DACP_LOW));
}
static ssize_t bl229x_store_contrast(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    g_bl229x->chip_params.m_contrast  = simple_strtoul(buf, NULL, 16);
    g_bl229x->contrast = g_bl229x->chip_params.m_contrast;
    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
    bl229x_spi_write_reg(REGA_RX_DACP_LOW,g_bl229x->chip_params.m_contrast);
    return size;
}
static DEVICE_ATTR(contrast,0664,bl229x_show_contrast,bl229x_store_contrast);
/*----------------------------------------------------------------------------*/
static struct device_attribute *bl229x_attr_list[] = {
    &dev_attr_agc,
	&dev_attr_contrast,
    &dev_attr_gain,
    &dev_attr_dacn,
    &dev_attr_reset,
    &dev_attr_init,
    &dev_attr_mode,
    &dev_attr_register,
    &dev_attr_interrupt,
    &dev_attr_image,
    &dev_attr_readimage,
    &dev_attr_spicmd,
    &dev_attr_key_interrupt,
    &dev_attr_report_delay,
    &dev_attr_selftest
};

/*----------------------------------------------------------------------------*/
static int bl229x_read_chipid(struct bl229x_data *bl229x)
{
    u8 val_low = 0,val_high = 0,chip_type=0xFF,driver_type=0x5A;
    int chip_id = 0;

     u8  old_value = 0;
	u8  active_ic = 0;

     BTL_DEBUG("  ++\n");

	
  
	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);

     old_value = bl229x_spi_read_reg(REGA_VERSION_RD_EN);

     bl229x_spi_write_reg(REGA_VERSION_RD_EN, old_value|0x80);

    msleep(5);

    val_low = bl229x_spi_read_reg(REGA_RC_THRESHOLD_HIGH);//id reg low
    BTL_DEBUG("val_low=0x%x",val_low);
    if(CHIP_ID_LOW != val_low)
        return -SPI_PIN_FAILED;

    val_high = bl229x_spi_read_reg(REGA_FINGER_TD_THRED_LOW);//id reg high
    BTL_DEBUG("val_high=0x%x",val_high);
    if(CHIP_ID_HIGH != val_high)
        return -SPI_PIN_FAILED;

    chip_type = bl229x_spi_read_reg(REGA_FINGER_TD_THRED_HIGH);//ic type
    BTL_DEBUG("chip_type=0x%x",chip_type);
    chip_id =(val_high << 8) | (val_low & 0xff);
    BTL_DEBUG("chip_id=%x",chip_id);

     bl229x_spi_write_reg(REGA_VERSION_RD_EN, old_value);
    bl229x->chip_info.chip_id_num = chip_id;
    bl229x->chip_info.chip_type = chip_type;

    driver_type = bl229x_spi_read_reg(REGA_DRIVER_VERSION);//ic driver type
    active_ic  = driver_type &0x70;
    BTL_DEBUG("11 driver_type=0x%x,active_ic=%d\n",driver_type,active_ic);
        
    if (!active_ic) {
        driver_type = 0x5A;
        driver_type = bl229x_spi_read_reg(REGA_PUMP_VOLTAGE);//
        active_ic = ((driver_type& 0xF0) ==0x70)?0:0xFF;
    }
    bl229x->chip_info.chip_driver_type = active_ic;
    BTL_DEBUG("active_type=%d\n",active_ic);
    return chip_id;
}



/*----------------------------------------------------------------------------*/
static int bl229x_reset_for_esd(struct bl229x_data *bl229x)
{
    int recovery_count = 3;
	u8 cur_contrast = 0;
    int ret = 0;
    BTL_DEBUG("  ++\n");

	

	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
	
	cur_contrast = bl229x_spi_read_reg(REGA_RX_DACP_LOW);
	bl229x_spi_write_reg(REGA_RX_DACP_LOW,cur_contrast);
	if (bl229x_spi_read_reg(REGA_RX_DACP_LOW) == cur_contrast && cur_contrast != 0){
		if (mt_get_gpio_in(GPIO_FP_INT_PIN) == 0){
           BTL_DEBUG("int gpio is low");
		   return 1;
		}	    
	}
    BTL_DEBUG("ret: %d,count = %d\n",ret,recovery_count);
   
	bl229x->reset = 1;

    while((ret <= 0) && recovery_count--) {
        BTL_DEBUG("hw_reset\n");
        hw_reset(bl229x);
        ret = bl229x_read_chipid(bl229x);
        BTL_DEBUG("recovey_from_esd_failed  recovery_count=%d chip_id=%x\n", recovery_count, ret);
    }
    bl229x_dev_init(bl229x);
    bl229x_dev_interrupt_init(bl229x,g_bl229x->fp_detect_method);
    bl229x->reset = 0;
    BTL_DEBUG("  --\n");
    return 0;
}



/*----------------------------------------------------------------------------*/
static void fps_create_attributes(struct device *dev)
{
    int num = (int)(sizeof(bl229x_attr_list)/sizeof(bl229x_attr_list[0]));
    for (; num > 0;)
        device_create_file(dev, bl229x_attr_list[--num]);
}

static int fps_sysfs_init(void)
{
    int ret;
    bl229x_kobj = kobject_create_and_add("bl229x_sysfs",NULL);
    if(bl229x_kobj == NULL) {
        BTL_DEBUG("%s  subsystem_register failed\n",__func__);
        ret = -ENOMEM;
        return ret;
    }

    ret = sysfs_create_file(bl229x_kobj,&dev_attr_agc.attr);
	ret = sysfs_create_file(bl229x_kobj,&dev_attr_contrast.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_gain.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_dacn.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_reset.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_init.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_mode.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_register.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_interrupt.attr);
	ret = sysfs_create_file(bl229x_kobj,&dev_attr_image.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_readimage.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_spicmd.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_key_interrupt.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_report_delay.attr);
    ret = sysfs_create_file(bl229x_kobj,&dev_attr_selftest.attr);

    if(ret) {
        BTL_DEBUG("%s sysfs_create_file failed\n",__func__);
    }

    return ret;
}


// chip rev1 and driver ic rev0
int bl229x_dev_inactive_driver_init(struct bl229x_data *bl229x)
{
	// 1 reg from 0x1d to 0x28, write 0x0, do not effect read 0x28 int status.
	int i = 0;
	u8 nVal = 0;
    u8 nbackup =0;


	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);

    for (i = 0x1D; i <= 0x28; i++) {
        bl229x_spi_write_reg(i, bl229x->chip_params.m_dacn);
	}

	// 2. FD threshold[7:0]: 0x11 00 
	bl229x_spi_write_reg(REGA_FINGER_TD_THRED_LOW, 0x00);

	// FD threshold[13:8]: 0x10
	bl229x_spi_write_reg(REGA_FINGER_TD_THRED_HIGH, 0x01);

	// task: 3.1 decrement FD DT interval, 0x0A - 70ms, default clk - 0x06
	bl229x_spi_write_reg(REGA_FINGER_DT_INTERVAL_LOW, 0x00);
	bl229x_spi_write_reg(REGA_FINGER_DT_INTERVAL_HIGH, 0x06);

	// 3.2 32K clock change from 18K to 205K
	//BL_WriteSingleCmd(REGA_VERSION_RD_EN, 0x08);
	//BL_WriteSingleCmd(REGA_VERSION_RD_EN, 0x0F);

	// 3.4 32M fast clock changed to (v3.3, x65)->3.1M, (v2.8, x75)->3.5M
    if (bl229x->chip_info.chip_type ==1) { //b type
	bl229x_spi_write_reg(0x39, 0x75);
    }
	// 3.5 
	bl229x_spi_write_reg(REGA_NAVI_FRM1_LOW, 0x11);

	// 4 task 1: Frame num to 1 to test
    if (bl229x->chip_info.chip_type ==3) { //navi need
        nbackup =bl229x_spi_read_reg(0x17);
        nVal = nbackup&0x7F;
        bl229x_spi_write_reg(0x17, nVal);	// prepare to read REGA_FRAME_NUM
        nVal = bl229x_spi_read_reg(REGA_FRAME_NUM);
        nVal &= 0xE7;	// b'11100111
        bl229x_spi_write_reg(REGA_FRAME_NUM, nVal);
        bl229x_spi_write_reg(0x17, nbackup);
    } else {
	bl229x_spi_write_reg(0x17, 0x2C);	// prepare to read REGA_FRAME_NUM
	nVal = bl229x_spi_read_reg(REGA_FRAME_NUM);
	nVal &= 0xE7;	// b'11100111  
	bl229x_spi_write_reg(REGA_FRAME_NUM, nVal);
    }

	// 5. 3 scan rows, close 1,3, set reg 0x2D bit[4] to 1, (pd_rx\u5bc4\u5b58\u5668)
	//BL_WriteSingleCmd(0x2D, 0xF0);

    if (bl229x->chip_info.chip_type == 3) {
        bl229x_spi_write_reg(0x3d, 0x75);

        bl229x_spi_write_reg(0x32, 0x49);
        bl229x_spi_write_reg(0x33, 0x92);
        bl229x_spi_write_reg(0x35, 0x52);
    }
	// 6 set gain, contrast
	
    bl229x_spi_write_reg(REGA_GC_STAGE,bl229x->chip_params.m_gain);
    bl229x_spi_write_reg(REGA_RX_DACP_LOW,bl229x->chip_params.m_contrast);
	//bl229x_spi_write_reg(0xE0, 0x80);

	
	return 0;
}

static int  bl229x_dev_active_driver_init(struct bl229x_data *bl229x)
{
	// 1 reg from 0x1d to 0x28, write 0x38, do not effect read 0x28 int status.
	int i = 0;
	uint8_t nVal = 0;
    u8 nbackup =0;

    if (bl229x->reset)
        return -1;


	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);

    for (i = 0x1D; i <= 0x28; i++) {
        bl229x_spi_write_reg(i, bl229x->chip_params.m_dacn);
		//BL_WriteSingleCmd(i, 0x20);
	}

	// 2. FD threshold[7:0]: 0x11 00 
	bl229x_spi_write_reg(REGA_FINGER_TD_THRED_LOW, 0x00);

	// FD threshold[13:8]: 0x12 0x10
	//BL_WriteSingleCmd(REGA_FINGER_TD_THRED_HIGH, 0x10);
	bl229x_spi_write_reg(REGA_FINGER_TD_THRED_HIGH, 0x01);

	// 3.1 decrement FD DT interval, 0x0A - 70ms
	bl229x_spi_write_reg(REGA_FINGER_DT_INTERVAL_LOW, 0x00);
    bl229x_spi_write_reg(REGA_FINGER_DT_INTERVAL_HIGH, 0x0A);

	// 3.2 32K clock change from 18K to 205K
    if (bl229x->chip_info.chip_type ==3) { //c type
	bl229x_spi_write_reg(REGA_VERSION_RD_EN, 0x0F);
    }
	// 3.4 32M fast clock change to (v3.3, x65)->50M, 
	bl229x_spi_write_reg(0x39, 0x75);
	
	// 3.5 \u52a0\u5feb\u6a21\u62df\u7535\u8def\u5f00\u542f
	bl229x_spi_write_reg(REGA_NAVI_FRM1_LOW, 0x11);

	// 6 set gain, contrast


	// 4 task 1: Frame num to 1 to test
    nbackup =bl229x_spi_read_reg(0x17);
    nVal = nbackup&0x7F;
    bl229x_spi_write_reg(0x17, nVal);	// prepare to read REGA_FRAME_NUM
	nVal = bl229x_spi_read_reg(REGA_FRAME_NUM);
	nVal &= 0xE7;	// b'11100111  
	bl229x_spi_write_reg(REGA_FRAME_NUM, nVal);

    bl229x_spi_write_reg(0x17, nbackup);
    bl229x_spi_write_reg(0x17, 0xAC);
	// 5.1 image background reversing, set reg 0x0E bit[0] to 1
	nVal = bl229x_spi_read_reg(REGA_NAVI_FRM7_LOW);
	nVal &= 0xFE;
	nVal |= 0x01;
	bl229x_spi_write_reg(REGA_NAVI_FRM7_LOW, nVal);	

	// 5.2 3 scan rows, close 1,3, set reg 0x2D bit[4] to 1, (pd_rx\u5bc4\u5b58\u5668)
	//BL_WriteSingleCmd(0x2D, 0xF0);


    bl229x_spi_write_reg(REGA_GC_STAGE,bl229x->chip_params.m_gain);
    bl229x_spi_write_reg(REGA_RX_DACP_LOW,bl229x->chip_params.m_contrast);

	return 0;
}

/*----------------------------------------------------------------------------*/
static int  bl229x_dev_init(struct bl229x_data *bl229x)
{
    u8 val_low = 0,val_high = 0,chip_type=0xFF,driver_type=0x5A;
    int chip_id = 0;
    u8  old_value = 0;
	u8  active_ic = 0;

    if (bl229x->reset)
        return -1;

 
	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);

    old_value = bl229x_spi_read_reg(REGA_VERSION_RD_EN);
    bl229x_spi_write_reg(REGA_VERSION_RD_EN, old_value|0x80);

    msleep(5);
    // 3 set FD DT interval 
    val_low = bl229x_spi_read_reg(REGA_RC_THRESHOLD_HIGH);//id reg low
    val_high = bl229x_spi_read_reg(REGA_FINGER_TD_THRED_LOW);//id reg high
    chip_type = bl229x_spi_read_reg(REGA_FINGER_TD_THRED_HIGH);//ic type
	
    chip_id =(val_high << 8) | (val_low & 0xff);
	// 4 Navi DT internal 
    bl229x_spi_write_reg(REGA_VERSION_RD_EN, old_value);

    bl229x->chip_info.chip_type = chip_type;
    bl229x->chip_info.chip_id_num = chip_id;
    BTL_DEBUG("vv chip_type=0x%x,chip_id=0x%x\n",chip_type,chip_id);
   
    // 5 32K clock change from 18K to 205K
    driver_type = bl229x_spi_read_reg(REGA_DRIVER_VERSION);//ic driver type
    active_ic  = driver_type & 0x70;
    // 6 32M fast clock change to 60M	
    if (!active_ic) {
        driver_type = 0x5A;
        driver_type = bl229x_spi_read_reg(REGA_PUMP_VOLTAGE);//
        active_ic = ((driver_type& 0xF0) ==0x70)?0:0xFF;
    }
    BTL_DEBUG("yy driver_type=0x%x,active_ic=%d\n",driver_type,active_ic);
    bl229x->chip_info.chip_driver_type= active_ic;

    if (active_ic) {
        bl229x->chip_params.m_contrast =SENSOR_DEFAULT_CONTRAST_V2;
        bl229x->chip_params.m_gain = SENSOR_DEFAULT_GAIN_V2;
        bl229x->chip_params.m_dacn= SENSOR_DEFAULT_DACN_V2;
        bl229x->contrast = bl229x->chip_params.m_contrast ;
        fingerprintdParams.def_contrast = bl229x->contrast;
        fingerprintdParams.def_ck_fingerup_contrast = bl229x->contrast;
        bl229x_dev_active_driver_init(bl229x);//hasn't driver ic
    } else { //
        bl229x->chip_params.m_contrast = SENSOR_DEFAULT_CONTRAST_V3 ;
	
        bl229x->chip_params.m_gain = SENSOR_DEFAULT_GAIN_V3;
        bl229x->chip_params.m_dacn= SENSOR_DEFAULT_DACN_V3;
        bl229x->contrast = bl229x->chip_params.m_contrast ;
        fingerprintdParams.def_contrast = bl229x->contrast;
        fingerprintdParams.def_ck_fingerup_contrast = bl229x->contrast;
        bl229x_dev_inactive_driver_init(bl229x);//has driver ic
}

    return 0;
}

/*----------------------------------------------------------------------------*/
static int bl229x_contrast_init(struct bl229x_data *bl229x,unsigned long arg)
{
	u8 cur_contrast = 0; 

    BTL_DEBUG("  ++\n");

 
	g_bl229x->interrupt_mode = INT_MODE_NONE;

	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);

	bl229x_spi_write_reg(REGA_RX_DACP_LOW, arg);

	cur_contrast = bl229x_spi_read_reg(REGA_RX_DACP_LOW);

    BTL_DEBUG("last_agc=%x data_rx[1]=%x", (s32)arg, cur_contrast);
    if(cur_contrast != arg) 
		printk("%s,error !\n",__func__);     

	BTL_DEBUG("mode:%d \n",g_bl229x->interrupt_mode);


	
    BTL_DEBUG("  --\n");
    return 0;
}


/*----------------------------------------------------------------------------*/
static int bl229x_gain_init(struct bl229x_data *bl229x,unsigned long arg)
{
	u8 cur_gain = (u8) arg;

    BTL_DEBUG("  ++\n");
	
 
	g_bl229x->interrupt_mode = INT_MODE_NONE;

	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);


	bl229x_spi_write_reg(REGA_GC_STAGE,cur_gain);

	
    BTL_DEBUG("  --\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
static int  bl229x_dev_interrupt_init(struct bl229x_data *bl229x, int navOrfp)
{

    u8 val = 0;
	
    BTL_DEBUG("  ++\n");

	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);	
	
	val = bl229x_spi_read_reg(REGA_HOST_CMD);
	if (val != MODE_IDLE){
		BTL_DEBUG("err0 %d\n",val);
		bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
	}

	bl229x_spi_write_reg(REGA_RX_DACP_LOW, bl229x->contrast);

	val = bl229x_spi_read_reg(REGA_RX_DACP_LOW);
	if (val != bl229x->contrast){
		BTL_DEBUG("err1 %d\n",val);
		bl229x_spi_write_reg(REGA_RX_DACP_LOW, bl229x->contrast);
	}

	bl229x->interrupt_mode = INT_MODE_KEY;

    if (navOrfp)
	    bl229x_spi_write_reg(REGA_HOST_CMD, MODE_NAVI);
	else
        bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_DT);

    BTL_DEBUG("  --\n");
    return 0;
}



/*----------------------------------------------------------------------------*/
static int bl229x_read_image(struct bl229x_data *bl229x,u32 timeout)
{
    BTL_DEBUG("  ++\n");
    if (down_interruptible(&bl229x->handler_mutex))
        return -ERESTARTSYS;
	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
    memset(bl229x->image_buf, 0xff, READIMAGE_BUF_SIZE);
	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_CAP);
	up(&bl229x->handler_mutex);
	BTL_DEBUG("  --\n");
    return 0;
}
{
    BTL_DEBUG("  ++\n");

	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
  
	bl229x_spi_write_reg(REGA_HOST_CMD, MODE_FG_CAP);


	BTL_DEBUG("  --\n");
    return 0;
}


/*----------------------------------------------------------------------------*/
static int bl229x_set_testmode(struct bl229x_data *bl229x, u8 * params, u8 size)
{
	int testType;
	int i;
	int workmode;

    BTL_DEBUG("  ++\n");

	
	for (i = 0; i < size; i++)
		 BTL_DEBUG("%d\n",params[i]);

	
	testType = params[0];
	switch (testType){
        case 1:  // mode switch
            workmode = params[1];
			if (workmode == MODE_IDLE)
				bl229x_spi_write_reg(REGA_HOST_CMD, MODE_IDLE);
			else if (workmode == MODE_FG_DT)
				bl229x_dev_interrupt_init(bl229x,FP_DET_BY_FD);
			else if (workmode == MODE_FG_CAP)
				bl229x_read_image(bl229x,10);
			else if (workmode == MODE_NAVI)
				bl229x_dev_interrupt_init(bl229x,FP_DET_BY_NAV);
			break;
		case 2:  // init action 
		    if (params[1] == 1)
		       bl229x_dev_init(bl229x);
			else if (params[1] == 2){
				bl229x_dev_init(bl229x);
				bl229x_dev_interrupt_init(bl229x,0);
			}else if (params[1] == 2){
				bl229x_dev_init(bl229x);
				bl229x_dev_interrupt_init(bl229x,1);
			}
			break;
		case 3:  // enable or disabel interrupt
		    if (params[1] == 1)
				enable_irq(bl229x->irq_num);  
			else 
				disable_irq_nosync(bl229x->irq_num);
			break;
		case 4:  //reset
		    if (params[1] == 1){
		       hw_reset(bl229x);
			   bl229x_dev_init(bl229x);
		    }else if (params[1] == 2){
			   hw_reset(bl229x);
			   bl229x_dev_init(bl229x);
			   bl229x_dev_interrupt_init(bl229x,0);
			}else if (params[1] == 3){
			   hw_reset(bl229x);
			   bl229x_dev_init(bl229x);
			   bl229x_dev_interrupt_init(bl229x,1);
			}
			break;
		default:
			break;

	}

    BTL_DEBUG("  --\n");
    return 0;
}


/*----------------------------------------------------------------------------*/
static int bl229x_clear_interrupt(struct bl229x_data *bl229x)
{
    return bl229x_dev_interrupt_init(bl229x,g_bl229x->fp_detect_method);
}


//中断处理函数
static void bl229x_eint_handler(void)
{
	mt_eint_mask(CUST_EINT_FP_INT_NUM);
	BTL_DEBUG("\n");
	g_bl229x->interrupt_coming = 1;
	wake_lock_timeout(&fp_suspend_lock, HZ/2);
	wake_up_interruptible(&int_waiter);
}


//中断 IO 配置，gpio143 号引脚
static void bl229x_eint_gpio_init(void)
{
	// 设定管脚工作模式
	mt_set_gpio_mode(GPIO_FP_INT_PIN,GPIO_FP_INT_PIN_M_EINT);
	// 设定管脚方向
	mt_set_gpio_dir(GPIO_FP_INT_PIN,GPIO_DIR_IN);
	// 设置下拉使能
	mt_set_gpio_pull_enable(GPIO_FP_INT_PIN,GPIO_PULL_ENABLE);
	//设置下拉
	mt_set_gpio_pull_select(GPIO_FP_INT_PIN,GPIO_PULL_DOWN);

	//设置中断出发方式
   //CUST_EINTF_TRIGGER_HIGH | CUST_EINTF_TRIGGER_RISING
	mt_eint_registration(CUST_EINT_FP_INT_NUM, CUST_EINTF_TRIGGER_RISING,bl229x_eint_handler,0);
}

//电源开关AVDD（2.6V-3.6V），DVDD（1.8V），IOVDD（1.8V or 2.8V）,RST/SHUTDOWN pull high
//ESD recovery have to power off, AVDD must under control
static int bl229x_power_on(struct bl229x_data *bl229x,bool enable)
{
	if(enable)
	{	
		//mt_set_gpio_mode(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_MODE_00);
		//mt_set_gpio_dir(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_DIR_OUT);
		//mt_set_gpio_out(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_OUT_ONE);

		mt_set_gpio_mode(GPIO_FINGER_PRINT_RESET, GPIO_MODE_00);
		mt_set_gpio_dir(GPIO_FINGER_PRINT_RESET, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_FINGER_PRINT_RESET, GPIO_OUT_ONE);
	}
	else
	{
    	//mt_set_gpio_mode(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_MODE_00);
		//mt_set_gpio_dir(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_DIR_OUT);
		//mt_set_gpio_out(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_OUT_ZERO);
		mt_set_gpio_mode(GPIO_FINGER_PRINT_RESET, GPIO_MODE_00);
		mt_set_gpio_dir(GPIO_FINGER_PRINT_RESET, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_FINGER_PRINT_RESET, GPIO_OUT_ZERO);
	}
	return 0;
}


//线程处理函数

// 注册中断设备
static int bl229x_create_inputdev(void)
{
    bl229x_inputdev = input_allocate_device();
    if (!bl229x_inputdev) {
        BTL_DEBUG("bl229x_inputdev create faile!\n");
        return -ENOMEM;
    }
    __set_bit(EV_KEY,bl229x_inputdev->evbit);
    __set_bit(KEY_F10,bl229x_inputdev->keybit);		//68
    __set_bit(KEY_F11,bl229x_inputdev->keybit);		//88
    __set_bit(KEY_F12,bl229x_inputdev->keybit);		//88
    __set_bit(KEY_CAMERA,bl229x_inputdev->keybit);	//212
    __set_bit(KEY_POWER,bl229x_inputdev->keybit);	//116
    __set_bit(KEY_PHONE,bl229x_inputdev->keybit);  //call 169

    __set_bit(KEY_F1,bl229x_inputdev->keybit);	//69
    __set_bit(KEY_F2,bl229x_inputdev->keybit);	//60
    __set_bit(KEY_F3,bl229x_inputdev->keybit);	//61
    __set_bit(KEY_F4,bl229x_inputdev->keybit);	//62
    __set_bit(KEY_F5,bl229x_inputdev->keybit);	//63
    __set_bit(KEY_F6,bl229x_inputdev->keybit);	//64
    __set_bit(KEY_F7,bl229x_inputdev->keybit);	//65
    __set_bit(KEY_F8,bl229x_inputdev->keybit);	//66
    __set_bit(KEY_F9,bl229x_inputdev->keybit);	//67

    bl229x_inputdev->id.bustype = BUS_HOST;
    bl229x_inputdev->name = "bl229x_inputdev";
    if (input_register_device(bl229x_inputdev)) {
        printk("%s, register inputdev failed\n", __func__);
        input_free_device(bl229x_inputdev);
        return -ENOMEM;
    }

	
	g_bl229x->pIntThread = kthread_run(bl229x_thread_func,g_bl229x,"bl229x_thread");
	if (IS_ERR(g_bl229x->pIntThread))
	{
       BTL_DEBUG("kthread_run is faile\n");
       return -(PTR_ERR(g_bl229x->pIntThread));
	}

    return 0;
}

/* -------------------------------------------------------------------- */
static int is_need_lock(unsigned int cmd)
{
	int ret = 0;
    switch (cmd) {
    case INIT_BL229X:
    case BL229X_GETIMAGE:
    case BL229X_INITERRUPT_MODE:
    case BL229X_CONTRAST_ADJUST: 
    case BL229X_CONTRAST_ADJUST2: 	
    case BL229X_POWERDOWN_MODE1:
    case BL229X_POWERDOWN_MODE2:
    case BL229X_TEST_MODE:
	case BL229X_GAIN_ADJUST:
	case BL229X_GET_ID:
	case BL229X_INIT_ARGS:
		ret = 1;
        break;
    case BL229X_INTERRUPT_FLAGS1:
    case BL229X_INTERRUPT_FLAGS2:
    case BL229X_MULTIFUNCTIONAL_KEYCODE:
    default:
        ret = 0;
        break;

    }
	BTL_DEBUG("cmd=0x%x,ret=%d\n",cmd,ret);
	return ret;
}

/* -------------------------------------------------------------------- */
static long bl229x_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
{
    struct bl229x_data *bl229x = filp->private_data;
    struct spi_device *spi;
    int error=0;
    u32 user_regval = 0;
	u32 chipid;
	u8  dataBuf[64];


	BTL_DEBUG("%s\n",__func__);

    if (_IOC_DIR(cmd) & _IOC_READ)
        error = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        error = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

    if (error) {
        BTL_DEBUG("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
        return -EFAULT;
    }

	if (bl229x->reset){
		BTL_DEBUG("chip is in reseting\n");
		return -EBUSY;
	}
	
    spi = spi_dev_get(bl229x->spi) ;
    	
    if(is_need_lock(cmd))
    {
    	atomic_set(&is_busy, 1);
		mutex_lock(&bl229x->handler_mutex);
	}
    switch (cmd) {
    case INIT_BL229X:
        BTL_DEBUG("INIT_BL229X \n");
        error= bl229x_dev_init(bl229x);
        break;
    case BL229X_GETIMAGE:
        BTL_DEBUG("BL229X_GETIMAGE \n");
        error = bl229x_read_image(bl229x,arg);
        break;
    case BL229X_INITERRUPT_MODE:
        BTL_DEBUG("BL229X_INITERRUPT_MODE \n");
		mt_eint_mask(CUST_EINT_FP_INT_NUM);
        error = bl229x_dev_interrupt_init(bl229x,bl229x->fp_detect_method);
		mt_eint_unmask(CUST_EINT_FP_INT_NUM);
		atomic_set(&is_busy, 0);
        break;
    case BL229X_CONTRAST_ADJUST: 
    case BL229X_CONTRAST_ADJUST2: 	
        BTL_DEBUG("BL229X_CONTRAST_ADJUST1 \n");
        error= bl229x_contrast_init(bl229x,arg);
        break;
    case BL229X_POWERDOWN_MODE1:
    case BL229X_POWERDOWN_MODE2:
        BTL_DEBUG("BL229X_POWERDOWN_MODE1 \n");
        break;
    case BL229X_INTERRUPT_FLAGS1:
    case BL229X_INTERRUPT_FLAGS2:
        BTL_DEBUG("BL229X_INTERRUPT_FLAGS1 \n");
        user_regval = mt_get_gpio_in(GPIO_FP_INT_PIN);
        if (copy_to_user((void __user*)arg, &user_regval, sizeof(user_regval)) != 0) {
            error = -EFAULT;
        }
        break;
    case BL229X_MULTIFUNCTIONAL_KEYCODE:
        g_bl229x->report_key  = (int)arg;
		BTL_DEBUG("%s,report_key:%d\n",__func__,g_bl229x->report_key);
        break;		
    case BL229X_TEST_MODE:
	    if (copy_from_user(dataBuf,(void __user*)arg,32) != 0 ){
		   BTL_DEBUG("ERROR: BL229X_TEST_MODE\n");
		   error = -EFAULT;
		   break;
		}
        bl229x_set_testmode(bl229x,dataBuf, 32);
        break;
	case BL229X_GAIN_ADJUST:
		bl229x_gain_init(bl229x,arg);
		break;
	case BL229X_GET_ID:
		chipid = bl229x_read_chipid(bl229x);
		if (copy_to_user((void __user*)arg,&chipid,sizeof(u32)*1) != 0 ){
		   error = -EFAULT;
		}
		break;
	case BL229X_INIT_ARGS:
		if (copy_to_user((void __user*)arg,&fingerprintdParams,sizeof(fingerprintdParams)) != 0 ){
		   error = -EFAULT;
		}
		break;
    default:
        error = -ENOTTY;
        break;

    }
	if(is_need_lock(cmd))
		mutex_unlock(&bl229x->handler_mutex);
    return error;

}

/*----------------------------------------------------------------------------*/
static int bl229x_open(struct inode *inode, struct file *file)
{
	BTL_DEBUG("  ++\n");
	set_spi_mode(1);
	file->private_data = g_bl229x;
    BTL_DEBUG("  --\n");
    return 0;
}


/* -------------------------------------------------------------------- */
//д\B2\D9\D7\F7\A3\ACָ\CEƴ\AB\B8в\BB\D0\E8Ҫ\B6\D4\C6\E4\BD\F8\D0\D0д\B2\D9\D7\F7 \A1\A3\B9\CAֱ\BDӷ\B5\BB\D8 \B2\D9\D7\F7
static ssize_t bl229x_write(struct file *file, const char *buff,size_t count, loff_t *ppos)
{
    return -ENOMEM;
}

/* -------------------------------------------------------------------- */
// \B6\C1\B2\D9\D7\F7
static ssize_t bl229x_read(struct file *file, char  *buff,size_t count, loff_t *ppos)
{
    int ret=0;
    int timeout;
    struct bl229x_data *bl229x = file->private_data;
    ssize_t status = 0;
   
    BTL_DEBUG("  ++\n");


	BTL_DEBUG("mode:%d \n",g_bl229x->interrupt_mode);
    //wait_event_interruptible(frame_waiter, is_frame_ready !=0);
    timeout = wait_event_interruptible_timeout(frame_waiter, g_bl229x->is_frame_ready !=0, 50);

    BTL_DEBUG("timeout:%d, is_frame_ready : %d\n\n",timeout,g_bl229x->is_frame_ready);

    if (timeout == 0 && g_bl229x->is_frame_ready == 0) {
        BTL_DEBUG("read timeout\n\n");
        return -EFAULT;
    }
	
    BTL_DEBUG("is_frame_ready=%d\n\n",g_bl229x->is_frame_ready);

    //if(g_bl229x->is_frame_ready == 1) {
    //    spi_read_frame_now(bl229x);
        //is_frame_ready = 0;
    //}
    BTL_DEBUG("copy_to_user \n");
    ret = copy_to_user(buff, g_bl229x->image_buf + 2, count); //skip
    if (ret) {
        status = -EFAULT;
    }

    g_bl229x->is_frame_ready = 0;

    BTL_DEBUG("status: %d \n", (int)status);
    BTL_DEBUG("  --\n");
    return status;
}


/* -------------------------------------------------------------------- */
static int bl229x_release(struct inode *inode, struct file *file)
{
    int status = 0 ;
    struct bl229x_data *bl229x = file->private_data;

    BTL_DEBUG("  ++\n");
	if (bl229x->opened == 0) return status;

	bl229x->opened--;
    BTL_DEBUG("  --\n");
    return status;
}
/* -------------------------------------------------------------------- */
static void bl229x_early_suspend (struct early_suspend *h)
{
	BTL_DEBUG("[bl229x]%s:\n\n", __func__);

}

/* -------------------------------------------------------------------- */
static void bl229x_late_resume (struct early_suspend *h)
{
	BTL_DEBUG("[bl229x]%s:\n", __func__);
}

/* -------------------------------------------------------------------- */
static int bl229x_suspend (struct device *dev)
{

    struct bl229x_data *bl229x = dev_get_drvdata(dev);

  
    dev_err (&bl229x->spi->dev,"[bl229x]%s\n", __func__);
    atomic_set(&suspended, 1);
	BTL_DEBUG("\n");
    return 0;
}

/* -------------------------------------------------------------------- */
static int bl229x_resume (struct device *dev)
{
    struct bl229x_data *bl229x = dev_get_drvdata(dev);
	
    dev_err (&bl229x->spi->dev,"[bl229x]%s\n", __func__);
    atomic_set(&suspended, 0);
	BTL_DEBUG("\n");
    return 0;
}

/* -------------------------------------------------------------------- */
static int bl229x_thread_func(void *unused)
{
	struct bl229x_data *bl229x = (struct bl229x_data *)unused;
	struct sched_param param = {.sched_priority = RTPM_PRIO_TPD};
	u8 intStatus = 0;
	
	sched_setscheduler(current,SCHED_RR,&param);
	do
	{
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(int_waiter, g_bl229x->interrupt_coming !=0);
		set_current_state(TASK_RUNNING); 
		wait_event_interruptible_timeout(waiting_spi_prepare,!atomic_read(&suspended),msecs_to_jiffies(100));
		BTL_DEBUG("  ++\n");
		bl229x->interrupt_coming = 0;
		
	    BTL_DEBUG("mode:%d \n",g_bl229x->interrupt_mode);
        if (g_bl229x->fp_detect_method == FP_DET_BY_FD) 
		   intStatus = 2;
	    else if (g_bl229x->fp_detect_method == FP_DET_BY_NAV) 
		   intStatus = 4;
		mutex_lock(&bl229x->handler_mutex);
	    if (getSensorInterruptStatus(g_bl229x) != intStatus) {
	         if (bl229x_reset_for_esd(g_bl229x) == 1){
			     bl229x_dev_init(g_bl229x);
				 bl229x_dev_interrupt_init(g_bl229x,g_bl229x->fp_detect_method);			 
		     }
	     }else { 

			 g_bl229x->is_frame_ready = 1;
             if (g_bl229x->interrupt_mode == INT_MODE_KEY) {       
	             g_bl229x->interrupt_mode = INT_MODE_NONE;
		        input_report_key(bl229x_inputdev,g_bl229x->report_key ,1);
		        input_sync(bl229x_inputdev);

				while(mt_get_gpio_in(GPIO_FP_INT_PIN) != 0)
				{
					BTL_DEBUG("sky %x ++++\n",mt_get_gpio_in(GPIO_FP_INT_PIN));
					//msleep(g_bl229x->report_delay);
					if(atomic_read(&is_busy) == 1) {
						break;
					} else {
						spi_read_frame_now(g_bl229x);
						bl229x_dev_interrupt_init(g_bl229x,g_bl229x->fp_detect_method);
						msleep(g_bl229x->report_delay);
					}
					BTL_DEBUG("sky %x ----\n",mt_get_gpio_in(GPIO_FP_INT_PIN));
				}
		        input_report_key(bl229x_inputdev,g_bl229x->report_key ,0);
		        input_sync(bl229x_inputdev);
            } else {
            	spi_read_frame_now(g_bl229x);
                BTL_DEBUG("is_frame_ready:%d\n", g_bl229x->is_frame_ready);
                wake_up_interruptible(&frame_waiter);
            }
	     }
	     mutex_unlock(&bl229x->handler_mutex);
		 mt_eint_unmask(CUST_EINT_FP_INT_NUM);
	}while(!kthread_should_stop());

	return 0;
}
/*----------------------------------------------------------------------------*/
static const struct file_operations bl229x_fops = {
    .owner = THIS_MODULE,
    .open  = bl229x_open,
    .write = bl229x_write,
    .read  = bl229x_read,
    .release = bl229x_release,
    .unlocked_ioctl = bl229x_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = bl229x_ioctl,
#endif

};

/*----------------------------------------------------------------------------*/
static struct miscdevice bl229x_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = SPI_DRV_NAME,
    .fops = &bl229x_fops,
};

#define HENRRY_CHIP_ID (0x5183)
static int is_connected(struct bl229x_data *bl229x)
{
	if(HENRRY_CHIP_ID == bl229x_read_chipid(bl229x))
		return 0;
	else return -1;
}

/*----------------------------------------------------------------------------*/
static int  bl229x_probe(struct spi_device *spi)
{
    struct bl229x_data *bl229x = NULL;
    int err = 0;
	
    BTL_DEBUG("  ++\n\n");
    bl229x = kzalloc(sizeof(struct bl229x_data),GFP_KERNEL);
    if (!bl229x) {
        return -ENOMEM;
    }
	g_bl229x = bl229x;
    bl229x->image_buf = (u8*)__get_free_pages(GFP_KERNEL,get_order(READIMAGE_BUF_SIZE));
    bl229x->imagetxcmd = (u8*)__get_free_pages(GFP_KERNEL,get_order(READIMAGE_BUF_SIZE));;
    bl229x->imagerxpix = (u8*)__get_free_pages(GFP_KERNEL,get_order(READIMAGE_BUF_SIZE));;
   
    memset(bl229x->image_buf,0x00,get_order(READIMAGE_BUF_SIZE));
    memset(bl229x->imagetxcmd,0x00,get_order(READIMAGE_BUF_SIZE));
    memset(bl229x->imagerxpix,0x00,get_order(READIMAGE_BUF_SIZE));
    if (!bl229x->image_buf) {
        return -ENOMEM;
    }
    spi_set_drvdata(spi,bl229x);


    BTL_DEBUG("step-1\n");

	mutex_init(&bl229x->spi_lock);
	mutex_init(&bl229x->handler_mutex);

	BTL_DEBUG("step-2\n");

	g_bl229x->pIntThread = NULL;

	mt_eint_mask(CUST_EINT_FP_INT_NUM);

    bl229x_create_inputdev();
	
    bl229x->spi = spi;
    bl229x->spi->bits_per_word = 8;
    bl229x->spi->mode = SPI_MODE_0;
    bl229x->spi->controller_data = (void*)&spi_conf;
    spi_setup(bl229x->spi);

    set_spi_mode(1);
	
	bl229x_power_on(bl229x, 1);

	BTL_DEBUG("step-3\n");
   
    err = misc_register(&bl229x_misc_device);
    if(err) {
        BTL_DEBUG("bl229x_misc_device register failed\n");
        goto exit_misc_device_register_failed;
    }
	
    //  IRQF_TRIGGER_HIGH | IRQF_ONESHOT 
    bl229x_eint_gpio_init();
	
	//spi dma or fifo 方式
	mtspi_set_mode(0);
	
	fps_sysfs_init();
    fps_create_attributes(&spi->dev);

#if (CHIP_VERSION == 3)
	bl229x->contrast = SENSOR_DEFAULT_CONTRAST_V3;
#elif (CHIP_VERSION == 2)
	bl229x->contrast = SENSOR_DEFAULT_CONTRAST_V2;
#else 
	bl229x->contrast = SENSOR_DEFAULT_CONTRAST_V1;
#endif 

	bl229x->interrupt_mode = INT_MODE_NONE;
	bl229x->report_key = KEY_F11;
	bl229x->report_delay = REPORT_DELAY_TIME;
	bl229x->is_frame_ready = 0;
	bl229x->opened = 0;
	bl229x->interrupt_coming = 0;
	bl229x->fp_detect_method = FP_DETECT_METHOD;
	bl229x->report_timeout = jiffies;
	atomic_set(&suspended, 0);
    atomic_set(&is_busy, 0);
    
	bl229x->early_suspend.suspend = bl229x_early_suspend,
	bl229x->early_suspend.resume = bl229x_late_resume,
	bl229x->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1,
	register_early_suspend (&bl229x->early_suspend);
  
    bl229x_dev_init(bl229x);
    bl229x_dev_interrupt_init(bl229x,bl229x->fp_detect_method);
	
	wake_lock_init(&fp_suspend_lock, WAKE_LOCK_SUSPEND, "fp_wakelock");
	
    BTL_DEBUG("  --\n");
	
	mt_eint_unmask(CUST_EINT_FP_INT_NUM);
    return 0;

exit_misc_device_register_failed:
    kfree(bl229x);
	mt_eint_unmask(CUST_EINT_FP_INT_NUM);
	printk("%s,probe-error!!!!\n\n",__func__);
    return -1;
}

static int  mt_spi_init(void)
{
	int ret=0;
	BTL_DEBUG("%s",__func__);
	ret=spi_register_board_info(spi_board_bl229x,ARRAY_SIZE(spi_board_bl229x));
	ret=spi_register_driver(&bl229x_driver);

	return ret; 
}


static void  mt_spi_exit(void)
{
	spi_unregister_driver(&bl229x_driver);
}

module_init(mt_spi_init);
module_exit(mt_spi_exit);

MODULE_DESCRIPTION("BL229X fingerprint driver for mtk");
MODULE_AUTHOR("BetterLife@blestech.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("bl229x");
