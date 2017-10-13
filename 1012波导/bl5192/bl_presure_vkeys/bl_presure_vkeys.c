/*
 * Copyright (C) 2017 Betterlife Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
//#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/input.h>

#include <linux/fs.h>
#include <linux/uaccess.h>

#include "bl_presure_vkeys.h"
#include "bl_fw.h"
/* #define TIMER_DEBUG */


#ifdef TIMER_DEBUG
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#endif

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>

#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
/**************************************/
#define PRESURE_DRIVER_NAME	"bl_presure"
#define ENABLE_IRQ_CTL
#define AUTO_UPDATE_FIRMWARE (1)

#define BTL_DEBUG(fmt, args...)  printk("btl-dbg-k:%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)


#ifdef CONFIG_TOUCHSCREEN_NB_STATUS_NODE  		/*add by dengwenhao 20170525 newbund*/
/*extern form "nb_tp_node.c"*/
extern  int isKeyRevert;
extern  int isNavBarEnable;
extern  int isKeyDisEnable;
#endif	



#ifdef ENABLE_IRQ_CTL
void vkey_func(struct work_struct *work);
/*
 *irq flag
 *irq_is_enabled : 1 enabled ; 0 disable
 */
static u8 irq_is_enabled = 1;	
//struct delayed_work vkey_delayed_work;
//INIT_DELAYED_WORK(&vkey_delayed_work, vkey_func);

DECLARE_DELAYED_WORK(vkey_delayed_work, vkey_func);
#endif		//ENABLE_IRQ_CTL

/**************************************/
static struct bl_presure_data g_bl_pdata = {
    .presure_irq_gpio = 123,
    .presure_reg_addr = PRESURE_REG_ADDR,
    .leftkey_reg_addr = LEFT_KEY_REG_ADDR,
    .rightkey_reg_addr = RIGHT_KEY_REG_ADDR,
    .fw_version_high = VERSION_HIGH_REG,
    .fw_version_low  = VERSION_LOW_REG,
};

/**************************************/
static const struct i2c_device_id presure_i2c_id[] = {{PRESURE_DRIVER_NAME,0},{}};
//static struct i2c_board_info __initdata i2c_devs={I2C_BOARD_INFO(PRESURE_DRIVER_NAME, PRESURE_SLAVE_ADRR)};
static struct i2c_client *g_bl_presure_client;

static int presure_bus_num = 5; //NFC
static int mslave_addr = 0x38; //NFC
static int mreg_addr = 0x3;//PRESURE_REG_ADDR_HIGH; //NFC
static int mreg_addr2 = 0x4;//PRESURE_REG_ADDR_LOW;//0x8d; //NFC

static int bl229x_i2c_read_reg(struct i2c_client *client,char reg)
{
	int ret = 0;
	char tmp[1];
	tmp[0] = reg;

	ret = i2c_master_send(client, tmp, 0x01);
	if (ret <= 0) {
		BTL_DEBUG("bl229x_i2c_read_reg 1 ret=%d\n", ret);
		goto EXIT_ERR;
	}
	ret = i2c_master_recv(client, tmp, 0x01);
	if (ret <= 0) {
		BTL_DEBUG("bl229x_i2c_read_reg 2 ret=%d\n", ret);
		goto EXIT_ERR;
	}
	BTL_DEBUG("bl229x_i2c_read_reg reg[0x%x]=%x \n", reg, tmp[0]);
	return tmp[0] ;

EXIT_ERR:
		BTL_DEBUG("bl229x_i2c_read_reg fail\n");
	
		return ret;
}

static int bl8818_i2c_txdata(struct i2c_client *client,char *txdata, int length)
{
	int ret = 0;

	struct i2c_msg msg[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	if (i2c_transfer(client->adapter, msg, 1) != 1) {
		ret = -EIO;
		pr_err("%s i2c write error: %d\n", __func__, ret);
	}

	return ret;
}

static int bl8818_write_reg(struct i2c_client *client,u8 addr, u8 para)
{
	u8 buf[3];
	int ret = -1;

	buf[0] = addr;
	buf[1] = para;
	ret = bl8818_i2c_txdata(client,buf, 2);
	if (ret < 0) {
		pr_err("write reg failed! %#x ret: %d", buf[0], ret);
		return -1;
	}

	return 0;
}

int bl_getPresure(int type){
	return bl229x_i2c_read_reg(g_bl_presure_client, g_bl_pdata.presure_reg_addr);
}

void bl_disable_irqdirect(void)
{
	BTL_DEBUG("disable vkey irq directly!\n");
	disable_irq_nosync(g_bl_pdata.presure_irq_num);
}

void bl_enable_irqdirect(void)
{
	BTL_DEBUG("enable vkey irq directly!\n");
	enable_irq(g_bl_pdata.presure_irq_num);
}

#ifdef ENABLE_IRQ_CTL
void bl_disable_irq(void)
{
	BTL_DEBUG("disable vkey irq!\n");
	if(irq_is_enabled)
	{
		disable_irq_nosync(g_bl_pdata.presure_irq_num);
		irq_is_enabled = 0;
	}else
	{	
		BTL_DEBUG("irq is alread disabled!\n");
	}
}

void bl_enable_irq(void)
{
	BTL_DEBUG("enable vkey irq!\n");
	if(!irq_is_enabled)
	{
		enable_irq(g_bl_pdata.presure_irq_num);
		irq_is_enabled = 1;
	}else
	{
		BTL_DEBUG("irq is alread enabled!\n");
	}

}

void vkey_func(struct work_struct *work)
{
	bl_enable_irq();
	BTL_DEBUG("irq_is_enabled : %d\n", irq_is_enabled);
}

void do_delaywork(u8 enable, unsigned long delay)
{
	if(enable)
		bl_enable_irq();
	else
		bl_disable_irq();
	cancel_delayed_work(&vkey_delayed_work);
	schedule_delayed_work(&vkey_delayed_work, delay);
}
#endif		//ENABLE_IRQ_CTL


/************************************************************************
* Name: fts_GetFirmwareSize
* Brief:  get file size
* Input: file name
* Output: no
* Return: file size
***********************************************************************/
static int fts_GetFirmwareSize(char *firmware_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[128];
	memset(filepath, 0, sizeof(filepath)); 

       sprintf(filepath, "/sdcard/%s", firmware_name);

	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) 
	{
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}

static int fts_ReadFirmware(char *firmware_name,
			       unsigned char *firmware_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;
	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "/sdcard/%s", firmware_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) 
	{
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, firmware_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);
	return 0;
}
/*
#define		I2C_MAX_TRANSFER_SIZE	(64)
#define		BL8XXX_60			0xff//bl86XX,bl8878
#define		BL8XXX_63			0x03//bl8818,bl8128,bl8138
#define		TS_CHIP			BL8XXX_63
#define 	I2C_TRANSFER_WSIZE      I2C_MAX_TRANSFER_SIZE//(128)
#define 	I2C_TRANSFER_RSIZE      I2C_MAX_TRANSFER_SIZE//(128)
#define 	I2C_VERFIY_SIZE         I2C_MAX_TRANSFER_SIZE//(128)
*/

#define SET_WAKEUP_HIGH		bl_ts_set_wakeup(1)
#define SET_WAKEUP_LOW		bl_ts_set_wakeup(0)
#if(SUPPORT_I2C_DMA)
static u8 *gpDMABuf_va = NULL;
static u32 gpDMABuf_pa = 0;
#endif
void bl_ts_set_wakeup(unsigned char level)
{
	unsigned int wakeup_gpio;
	BTL_DEBUG();
	
#if(TS_CHIP == BL8XXX_60)
	//wakeup_gpio = GPIO_CTP_RST_PIN;
#else
	wakeup_gpio = g_bl_pdata.presure_irq_gpio;//GPIO_CTP_EINT_PIN;
#endif
    gpio_direction_output(wakeup_gpio, level);
	gpio_set_value(wakeup_gpio, level);
}

static void bl_ts_wakeup(void)
{
	bl_ts_set_wakeup(1);
	mdelay(50);
	bl_ts_set_wakeup(0);
	mdelay(50);
	bl_ts_set_wakeup(1);
}

void bl_ts_set_irq(void)
{
	gpio_direction_input(g_bl_pdata.presure_irq_gpio);
}

#if(SUPPORT_I2C_DMA)
int bl_ts_i2c_transfer_dma(__u8 i2c_addr, __u8 *buf, __u16 len,__u8 rw)
{
	struct i2c_client *client = g_bl_presure_client;
    int ret;
    u8 *wr_buf = gpDMABuf_va;
    
    struct i2c_msg msg =
    {
        .addr = i2c_addr,
        .ext_flag = (client->ext_flag | I2C_ENEXT_FLAG | I2C_DMA_FLAG),
        .flags = rw,
        .buf = (u8*)gpDMABuf_pa,
        .len = len,
        .timing = TS_I2C_SPEED
    };
    
   if(rw){
   		ret = i2c_transfer(client->adapter, &msg, 1);
	    if (ret < 0)
	    {
	    	BTL_DEBUG("___i2c DMA transfer error___\n");
	        return -1;
	    }
	    memcpy(buf, wr_buf, len);
	    
   	}else{
		memcpy(wr_buf, buf, len);
	    
	    ret = i2c_transfer(client->adapter, &msg, 1);
	    if (ret < 0)
	    {
	    	BTL_DEBUG("___i2c DMA transfer error___\n");
	        return -1;
	    }
	}
    return 0;
}
#endif
int bl_ts_i2c_transfer_non_dma(__u8 i2c_addr, __u8 *buf, __u16 len,__u8 rw)
{
 	struct i2c_client *client = g_bl_presure_client;
    int ret;
	
    struct i2c_msg msg[] =
    {
        {         
            .addr = i2c_addr,
            .flags = rw,
            .buf = buf,
            .len = len,
            //.timing = TS_I2C_SPEED
        },
        
    };

	if(len>I2C_MAX_TRANSFER_SIZE){
		BTL_DEBUG("___i2c transfer error___\n");
		return -1;
	}
	
	ret = i2c_transfer(client->adapter, msg, 1);
	if(ret != 1){
		BTL_DEBUG("___i2c transfer error___ret=%d\n",ret);
		return -1;
	}

	return 0; 
}
int bl_i2c_transfer(__u8 i2c_addr, __u8 *buf, __u16 len,__u8 rw)
{
#if(SUPPORT_I2C_DMA)
	return bl_ts_i2c_transfer_dma(i2c_addr, buf,len,rw);
#else
	return bl_ts_i2c_transfer_non_dma(i2c_addr,buf,len,rw);
#endif

}

static int bl_flash_transfer(__u8 cmd,__u16 flash_start_addr, __u8 *buf, __u16 len,__u8 rw)
{
	unsigned char cmd_buf[6];
	unsigned short flash_end_addr;
	int ret;
		
	BTL_DEBUG();
	
	if(!len){
		BTL_DEBUG("___write flash len is 0x00,return___\n");
		return -1;	
	}

	flash_end_addr = flash_start_addr + len -1;

	if(flash_end_addr >= MAX_FLASH_SIZE){
		BTL_DEBUG("___write flash end addr is overflow,return___\n");
		return -1;	
	}

	cmd_buf[0] = cmd;
	cmd_buf[1] = ~cmd;
	cmd_buf[2] = flash_start_addr >> 0x08;
	cmd_buf[3] = flash_start_addr & 0xff;
	cmd_buf[4] = flash_end_addr >> 0x08;
	cmd_buf[5] = flash_end_addr & 0xff;

	ret = bl_i2c_transfer(BL_FLASH_I2C_ADDR, cmd_buf,6,I2C_WRITE);
	if(ret < 0){
		BTL_DEBUG("___%s:i2c write flash error___\n",__func__);
		return -1;
	}
	
	ret = bl_i2c_transfer(BL_FLASH_I2C_ADDR, buf,len,rw);
	if(ret < 0){
		BTL_DEBUG("___%s:i2c read flash error___\n",__func__);
		return -1;
	}
	
	return 0;
}

static int bl_erase_flash(void)
{
	unsigned char cmd[2];
	
	DEBUG();
	
	cmd[0] = ERASE_ALL_MAIN_CMD; 
	cmd[1] = ~cmd[0];

	return bl_i2c_transfer(BL_FLASH_I2C_ADDR,cmd, 0x02,I2C_WRITE);	
}

static int bl_verify_fw(const u8 *fwbin,int fwsize)
{
	unsigned int i,j =0;
	unsigned short size,len;
	unsigned short addr;
	unsigned char *read_buf;
	BTL_DEBUG("___fw varify start___fwsize=%d\n",fwsize);
	read_buf = kzalloc(I2C_TRANSFER_RSIZE, GFP_KERNEL);
	if(read_buf == NULL){
		return -1;
	}

	for(i=0;i<fwsize;){
		
		size = fwsize - i;
		if(size > I2C_TRANSFER_RSIZE){
			len = I2C_TRANSFER_RSIZE;
		}else{
			len = size;
		}

	#if(TS_CHIP == BL8XXX_60)
		if (i >= 0x4000){
	        	addr = i & 0x3fff;
		}else {
	        	addr = i | 0x4000;
		}
	#else
		addr = i;
	#endif
		
		if(bl_flash_transfer(READ_MAIN_CMD,addr, read_buf,len,I2C_READ)){
			BTL_DEBUG("___fw varify ___1111\n");
			goto VERIFY_ERROR;
		}
		BTL_DEBUG("___fw varify start___2222\n");
		if (memcmp(&fwbin[i],read_buf,len)){
			goto VERIFY_ERROR;
		}
		i += len;
		j += len;
		if(j>=1024){
			BTL_DEBUG("___fw varify start___3333 j=%d\n",j);
			j = 0;
			/*
			SET_WAKEUP_HIGH;
			udelay(500);
			SET_WAKEUP_LOW;
			mdelay(2);
			*/
		}		
	}

	kfree(read_buf);
	BTL_DEBUG("___fw varify success___\n");

	return 0;

VERIFY_ERROR:	
	kfree(read_buf);
	BTL_DEBUG("___fw varify fail___\n");

	return -1;
}

static int bl_download_fw(const u8 *fwbin,int fwsize)
{
	unsigned int i;
	unsigned short size,len;
	unsigned short addr;

	DEBUG();
	if(bl_erase_flash()){
		BTL_DEBUG("___erase flash fail___\n");
		return -1;
	}
	mdelay(50);
	SET_WAKEUP_HIGH;
	mdelay(5);
	SET_WAKEUP_LOW;
	mdelay(5);

	for(i=0;i<fwsize;){
		size = fwsize - i;
		if(size > I2C_TRANSFER_WSIZE){
			len = I2C_TRANSFER_WSIZE;
		}else{
			len = size;
		}

	#if(TS_CHIP == BL8XXX_60)
		if (i >= 0x4000){
	        	addr = i & 0x3fff;
		}else {
	        	addr = i | 0x4000;
		}
	#else
		addr = i;
	#endif
		if(bl_flash_transfer(WRITE_MAIN_CMD,addr, (__u8 *)&fwbin[i],len,I2C_WRITE)){
		//if(bl_write_flash(WRITE_MAIN_CMD,addr, (__u8 *)&fwbin[i],len)){
			return -1;
		}
		i += len;
		
	}

	return 0;	
}


static int bl_update_flash(unsigned char force_update,const u8 *fwbin,int fwsize)
{
	int retry;
	int ret = 0;

	BTL_DEBUG("___fw update start___fwsize=%d\n",fwsize);
	disable_irq_nosync(g_bl_pdata.presure_irq_num);
	SET_WAKEUP_LOW;
	mdelay(5);
	//force_update = 1;
	if(force_update || bl_verify_fw(fwbin,fwsize)){

		retry =3;
		while(retry--){

			ret = bl_download_fw(fwbin,fwsize);
			if(ret){
				continue;
			}

			ret = bl_verify_fw(fwbin,fwsize);
			if(ret){
				continue;
			}	
			break;		
		}
	
		if(ret){
			BTL_DEBUG("___fw update fail___\n");
		}else{
			BTL_DEBUG("___fw update success___\n");	
		}
	}else{
		BTL_DEBUG("___The fw is up to date___\n");
	}
	mdelay(5);
	SET_WAKEUP_HIGH;
	bl_ts_set_irq();
	enable_irq(g_bl_pdata.presure_irq_num);
	BTL_DEBUG("___fw update end___\n");

	return ret;
}


/************************************************************************
* Name: btl_fw_upgrade_with_app_file
* Brief:  upgrade with *.bin file
* Input: i2c info, file name
* Output: no
* Return: success =0
***********************************************************************/
int btl_fw_upgrade_with_app_file(struct i2c_client *client,
				       char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret = 0;
	int fwsize = fts_GetFirmwareSize(firmware_name);

	if (fwsize <= 0) 
	{
		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n",__func__);
		return -EIO;
	}
	if (fwsize < 8 || fwsize > 54 * 1024) 
	{
		dev_dbg(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}
	/*=========FW upgrade========================*/
	pbt_buf = kmalloc(fwsize + 1, GFP_ATOMIC);

	if (fts_ReadFirmware(firmware_name, pbt_buf)) 
	{
		dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n",__func__);
		kfree(pbt_buf);
		return -EIO;
	}
	
	/*call the upgrade function */
	//i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);
	bl_update_flash(1,pbt_buf,fwsize);

	kfree(pbt_buf);

	return i_ret;
}

static int gfw_version = 0;
static ssize_t bl229x_show_fwupdate(struct device *ddri,struct device_attribute *attr,char *buf)
{
	int version = bl_get_sensor_version();
    return sprintf(buf,"gfw_version=%x\n", version);
}
static ssize_t bl229x_store_fwupdate(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    gfw_version = btl_fw_upgrade_with_app_file(g_bl_presure_client, "blfirmware.bin");
    BTL_DEBUG("gfw_version=0x%x",gfw_version);
    return size;
}
static DEVICE_ATTR(fwupdate,0664,bl229x_show_fwupdate,bl229x_store_fwupdate);

/*----------------------------------------------------------------------------*/
static ssize_t bl229x_show_presure(struct device *ddri,struct device_attribute *attr,char *buf)
{
	u32 presure_value = 0;
	u8 value = 0;
	u8 value2 = 0;
	//value = bl229x_i2c_read(presure_bus_num , (mslave_addr << 1), mreg_addr);
	//value2 = bl229x_i2c_read(1 , 0x3c, 0xC);
	value = bl229x_i2c_read_reg(g_bl_presure_client,mreg_addr);
	value2 = bl229x_i2c_read_reg(g_bl_presure_client,mreg_addr2);
	presure_value = (value&0xf)|value2;
	//presure_value = bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.presure_reg_addr);
	//presure_value = (bl229x_i2c_read_reg(g_bl_presure_client,mreg_addr) << 8)|bl229x_i2c_read_reg(g_bl_presure_client,mreg_addr2);
	return sprintf(buf, "\nvalue=0x%x value2=0x%x presure=%x \n", value, value2, presure_value);
}

static ssize_t bl229x_store_presure(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	char *next;
	//g_bl_pdata.presure = simple_strtoul(buf, &next, 10);
	
	presure_bus_num = simple_strtoul(buf, &next, 16);
	mslave_addr = simple_strtoul(next+1, &next, 16);
	mreg_addr = simple_strtoul(next+1, &next, 16);
	mreg_addr2 = simple_strtoul(next+1, &next, 16);
	BTL_DEBUG("presure_bus_num = %x, mslave_addr=0x%x, mreg_addr=0x%x mreg_addr2=0x%x",presure_bus_num, mslave_addr, mreg_addr, mreg_addr2);
	return size;
}

static DEVICE_ATTR(presure, 0664, bl229x_show_presure, bl229x_store_presure);

/*
static int is_power_on;
static ssize_t bl229x_show_power_on(struct device *ddri,struct device_attribute *attr,char *buf)
{
    return sprintf(buf, "\npower_on=%d\n", is_power_on);
}

static ssize_t bl229x_store_power_on(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
	int ret;
    is_power_on = simple_strtoul(buf, &next, 10);
    if(1 == is_power_on)
    {
    	tpd->reg = regulator_get(tpd->tpd_dev, "vtouch");
		//ret = regulator_set_voltage(tpd->reg, 2800000, 2800000);	//set 2.8v
		ret = regulator_enable(tpd->reg);	//enable regulator
		if (ret)
			BTL_DEBUG("regulator_enable() failed!\n");
    }
    else
    {
    	tpd->reg = regulator_get(tpd->tpd_dev, "vtouch");
		//ret = regulator_set_voltage(tpd->reg, 2800000, 2800000);	//set 2.8v
		ret = regulator_disable(tpd->reg);	//enable regulator
		if (ret)
			BTL_DEBUG("regulator_enable() failed!\n");
    }
    return size;
}
static DEVICE_ATTR(power_on, 0664, bl229x_show_power_on, bl229x_store_power_on);
*/

/*----------------------------------------------------------------------------*/

irqreturn_t bl229x_presure_eint_handler(int irq,void *data)
{
	int value = 0;
	int key_bit = 0;
	int key_value = 0;
	static int keybit_pre = 0;
    struct input_dev *inputdev = g_bl_pdata.inputdev;


#if defined(CONFIG_TOUCHSCREEN_NB_STATUS_NODE)
		if((isKeyDisEnable ==1) || (isNavBarEnable == 1))
		{
			return IRQ_HANDLED;
		}
#endif


	/* mask for no pressure
	value = bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.presure_reg_addr);
	if(value > THREHOLD_PRESURE)
	{

        key_bit |= KEYBIT_CENTER;
	}
	*/

	value = bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.leftkey_reg_addr);
	if(value > THREHOLD_PRESURE)
	{
        key_bit |= KEYBIT_LEFT;
	}
	
	value = 0;
	value = bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.rightkey_reg_addr);
	if(value > THREHOLD_PRESURE)
	{
		key_bit |= KEYBIT_RIGHT;
	}
	
	value = key_bit ^ keybit_pre;
	
	/*mask for no pressure
	if(value & KEYBIT_CENTER)
	{
		if(key_bit & KEYBIT_CENTER)
		{
		    input_report_key(inputdev, KEY_HOMEPAGE ,1);
		    input_sync(inputdev);
        }
		else
		{
		    input_report_key(inputdev, KEY_HOMEPAGE ,0);
		    input_sync(inputdev);
        }
	}*/
	
	if(value & KEYBIT_LEFT)
	{
#if defined(CONFIG_TOUCHSCREEN_NB_STATUS_NODE)
		if(isKeyRevert == 1)
			key_value = KEY_BACK;
		else
			key_value = KEY_APP_SWITCH;
#else				
		key_value = KEY_BACK;
#endif
		if(key_bit & KEYBIT_LEFT)
		{
		    input_report_key(inputdev,  key_value,1);
		    input_sync(inputdev);
        }
		else
		{
		    input_report_key(inputdev, key_value ,0);
		    input_sync(inputdev);
        }
	}	
	if(value & KEYBIT_RIGHT)
	{
#if defined(CONFIG_TOUCHSCREEN_NB_STATUS_NODE)
		if(isKeyRevert == 1)
			key_value = KEY_APP_SWITCH;
		else
			key_value = KEY_BACK;
#else
		key_value = KEY_APP_SWITCH;
#endif
		if(key_bit & KEYBIT_RIGHT)
		{
		    input_report_key(inputdev, key_value ,1);
		    input_sync(inputdev);
        }
		else
		{
		    input_report_key(inputdev, key_value ,0);
		    input_sync(inputdev);
        }
	}

	keybit_pre = key_bit;
	BTL_DEBUG("key_bit=%d,value=%x",key_bit,value);

    return IRQ_HANDLED;
}

static int bl229x_parse_dt_gpio(struct device *dev,struct bl_presure_data *pdata)
{
	struct pinctrl *pinctrl1 = NULL;
    struct pinctrl_state *bl_touch_default = NULL;
    int error = -1;
    
    pinctrl1 = devm_pinctrl_get(dev);
    bl_touch_default = pinctrl_lookup_state(pinctrl1, "bl_touch_int");
    pinctrl_select_state(pinctrl1, bl_touch_default);
    
    pdata->presure_irq_gpio = of_get_named_gpio_flags(dev->of_node,
                     "int-gpio", 0, NULL);
    if (gpio_is_valid(pdata->presure_irq_gpio)) {
        error = gpio_request(pdata->presure_irq_gpio, "F_PRESURE_IRQ");
        if (error) {
            BTL_DEBUG("unable to request GPIO %d\n",pdata->presure_irq_gpio);
        }
        pdata->presure_irq_num = gpio_to_irq(pdata->presure_irq_gpio);
        error = gpio_direction_input(pdata->presure_irq_gpio);

    }
    
    return error;
}

static int bl229x_create_inputdev(struct bl_presure_data *pdata)
{
	struct input_dev *inputdev;
    inputdev = input_allocate_device();
    pdata->inputdev = inputdev;
    
    inputdev->id.bustype = BUS_HOST;
    inputdev->name = "bl_vkeys";
    if (input_register_device(inputdev)) {
        printk("%s, register inputdev failed\n", __func__);
        input_free_device(inputdev);
        return -ENOMEM;
    }
    __set_bit(EV_KEY,inputdev->evbit);
    __set_bit(KEY_BACK,inputdev->keybit);  
    __set_bit(KEY_HOMEPAGE,inputdev->keybit); 
    __set_bit(KEY_MENU,inputdev->keybit);
	__set_bit(KEY_APP_SWITCH,inputdev->keybit); 
    
    return 0;
}

/*----------------------------------------------------------------------------*/
static int fb_notifier_callback(struct notifier_block *self,
                                unsigned long event, void *data)
{
    struct fb_event *evdata = data;

    int *blank;


    if (evdata && evdata->data && event == FB_EVENT_BLANK )//&&

    {
        blank = evdata->data;
        if (*blank == FB_BLANK_UNBLANK)
        {
        	BTL_DEBUG();
            bl_ts_wakeup();
            bl_ts_set_irq();
            enable_irq(g_bl_pdata.presure_irq_num);
#ifdef ENABLE_IRQ_CTL
			irq_is_enabled = 1; 
#endif
        }
        else if (*blank == FB_BLANK_POWERDOWN)
        {
        	BTL_DEBUG();
        	disable_irq_nosync(g_bl_pdata.presure_irq_num);
        	bl8818_write_reg(g_bl_presure_client, 0xa5, 0x03);
#ifdef ENABLE_IRQ_CTL
			irq_is_enabled = 0; 
#endif
        }
    }

    return 0;
}

static inline int bl_get_sensor_version(void)
{
	int version = 0;
	version = bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.fw_version_high);
	version = (version << 8) | bl229x_i2c_read_reg(g_bl_presure_client,g_bl_pdata.fw_version_low);
	BTL_DEBUG("read sensor firmware version : %d", version);
	return version;
}

static int bl_check_update_fw(void)
{
	int version;
	int fw_version;
	int ret = -1;
	
	//1. read version reg
	version = bl_get_sensor_version();
	fw_version = (fwbin[0x94] << 8) | fwbin[0x95];
	BTL_DEBUG("read fwbin version : %d", fw_version);
	if(fw_version > version)
	{
		ret = bl_update_flash(1, fwbin, FW_SIZE);
	}
	if(ret)
		BTL_DEBUG("fail!");
	else
		gfw_version = bl_get_sensor_version();

	return ret;
}

static int bl_presure_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int error = 0;
    g_bl_presure_client = client;
    g_bl_pdata.client = client;
    
    error = bl229x_parse_dt_gpio(&client->dev, &g_bl_pdata);
    if(error)
    {
    	BTL_DEBUG("bl229x_parse_dt_gpio error=%d",error);
    }

	error = request_threaded_irq(g_bl_pdata.presure_irq_num, NULL, bl229x_presure_eint_handler, 
		IRQ_TYPE_LEVEL_LOW | IRQF_ONESHOT, PRESURE_DRIVER_NAME, &g_bl_pdata);
    if(error)
    {
    	BTL_DEBUG("request_threaded_irq error=%d",error);
    }
    
	error = bl229x_create_inputdev(&g_bl_pdata);
    if(error)
    {
    	BTL_DEBUG("bl229x_create_inputdev error=%d",error);
    }
    
	device_create_file(&client->dev,&dev_attr_fwupdate);
	device_create_file(&client->dev,&dev_attr_presure);
	
    g_bl_pdata.fb_notify.notifier_call = fb_notifier_callback;
    error = fb_register_client(&g_bl_pdata.fb_notify);
    if (error)
       BTL_DEBUG("Unable to register fb_notifier: %d\n",error);
#if AUTO_UPDATE_FIRMWARE
	error = bl_check_update_fw();
	if(error)
		BTL_DEBUG("check and update vkey firmware fail. %d\n",error);
#endif
    return 0;
}

static const struct of_device_id blpresure_dt_match[] = {
	{.compatible = "mediatek,bl_touch"},
	{},
};
MODULE_DEVICE_TABLE(of, blpresure_dt_match);

struct i2c_driver bl_presure_driver = {
    .probe = bl_presure_i2c_probe,
    .driver = {
		.of_match_table = of_match_ptr(blpresure_dt_match),
		.name =  PRESURE_DRIVER_NAME,
	},
    .id_table = presure_i2c_id,
};

int __init bl_presure_init(void)
{
	DEBUG();	
    //i2c_register_board_info(5, &i2c_devs, 1);
	return i2c_add_driver(&bl_presure_driver);
}

void __exit bl_presure_exit(void)
{
	DEBUG();
	
	i2c_del_driver(&bl_presure_driver);
	
}

module_init(bl_presure_init);
module_exit(bl_presure_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("BetterLife BetterLife@blestech.com");
MODULE_DESCRIPTION("BetterLife presure and vkeys driver.");

