#ifndef	_BL_TS_H_ 
#define	_BL_TS_H_ 

#include <linux/gpio.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define		BL8XXX_60			0xff//bl86XX,bl8878
#define		BL8XXX_61			0x02//bl8858,bl8868
#define		BL8XXX_63			0x03//bl8818,bl8128,bl8138



/***************BetterLife TS config**********************/

#define		TS_CHIP			BL8XXX_63//BL8XXX_61//BL8XXX_60//

#if(TS_CHIP == BL8XXX_60)
#define 	TS_I2C_SPEED		200	    //KHz
#else
#define 	TS_I2C_SPEED		200	    //KHz
#endif

#define		BL_FW_I2C_ADDR		0x28

#define		WAKEUP_GPIO				RK30_PIN4_PD0

#define		IRQ_GPIO					RK30_PIN4_PC2


#define		MAX_X_RES		1080
#define		MAX_Y_RES		1920	

#define		I2C_BUS_NUM		0x01

#define		I2C_MAX_TRANSFER_SIZE		0x08
#define		I2C_MAX_DMA_TRANSFER_SIZE	0x255

#define		SUPPORT_I2C_DMA		1

//#define		BL_FW_I2C_ADDR		0x28


#define		XY_SWAP_ENABLE		0

#define		X_REVERSE_ENABLE	0

#define		Y_REVERSE_ENABLE	0


#define		DEBUG_ENABLE		0

#define		FW_UPDATE_ENABLE	0



#define		MAX_POINT_NUM		5


/*****ac/usb power detect config*******/

#define		AC_USB_POWER_DETECT_ENABLE	0
#define		POLLING_TIME			3000//ms

#define		AC_POWER_ONLINE_STATUS_PATH	"/sys/class/power_supply/ac/online"
#define		USB_POWER_ONLINE_STATUS_PATH	"/sys/class/power_supply/usb/online"
#define		BATTERY_CHARGE_STATUS_PATH	"/sys/class/power_supply/battery/status"
#define		BATTERY_CAPACITY_LEVEL_PATH	"/sys/class/power_supply/battery/capacity"


/************vkeys config**************/
#define		HAVE_TOUCH_KEY			1
#define		KEY_NUM				3

#define		VKEYS_CODE			{KEY_MENU,KEY_HOMEPAGE,KEY_BACK}//{key1,key2,key3,key4}
						//{KEYn_X0,KEYn_Y0,KEYn_X1,KEYn_X2} //keyn,n=1,2,3,4
						//rule:x0<=x1,y0<=y1
#define		VKEYS_DIM			{{120,2050,120,2050},{360,2050,360,2050},{600,2050,600,2050}}

							
/***********proximity config**********/
#define		PROXIMITY_ENABLE			0

/*********gesture resume config**********/

#define		GESTURE_RESUME_ENABLE		0
#define		GESTURE_RESUME_KEY		KEY_POWER//KEY_WAKEUP//




/***************BetterLife TS config end**********************/






#define 	DOWN			0x00//00b
#define 	UP			0x01//01b
#define 	CONTACT			0x02//10b

/*BetterLife ts driver name*/
#define DRIVER_NAME			"Betterlife_ts"


#define	LOG_TAG				"Betterlife_ts"

#if(DEBUG_ENABLE)
#define 	DEBUG()			printk("___%s___\n",__func__);
#define 	XY_DEBUG(id,event,x,y)	printk("id = %d,event = %d,X = %d,Y = %d\n",id,event,x,y);
#else   
#define		DEBUG()
#define 	XY_DEBUG(id,event,x,y)
#endif


#define	BL_PRINTF(...)			printk(LOG_TAG ":" __VA_ARGS__)



struct xy_data {
	
	unsigned char	xh : 4;			// X coordinate High
	unsigned char	reserved : 2;		// reserved 
	unsigned char	event : 2;		// Action information, 00b: Down; 10b: contact/move; 01b: Up ; 11b:reserved
	unsigned char	xl;			// X coordinate Low
	unsigned char	yh : 4;			// Y coordinate High
	unsigned char	id : 4;			//ID information, from 0 to MAX_POINT_NUM-1
	unsigned char	yl;			// Y coordinate Low
	
	unsigned char	weight;			// Touch weight/Pressure
	unsigned char	area;			// Touch area	
};

struct ts_xy_data {
	
	unsigned char	reserved;		// reserved
	unsigned char	gesture;		// gesture
	unsigned char	point_num;		//touch point num
	struct xy_data 	point[MAX_POINT_NUM];
};


struct vkeys_value{
	
	unsigned short code;//keycode
	int x0;	//left;
	int y0;	//top;
	int x1;	//right;
	int y1;	//bottom;
};

struct ts_vkeys{

	unsigned char num;
	unsigned char press;
	unsigned char release;
	struct vkeys_value value[KEY_NUM];
};

struct ts_driver{

	
	struct i2c_client		*client;

	/* input devices */
	struct input_dev		*input_dev;			

	struct proc_dir_entry		*proc_entry;	

	struct task_struct 		*ts_thread;
	struct delayed_work  		dwork;

	#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend		early_suspend;
	#endif

	unsigned char			suspend;		//1:suspend,0:resume
	unsigned char			ac_usb_power_online;
	unsigned char			gesture_resume;
	unsigned char 			charging;
	unsigned char 			debug;
	
};

struct ts_config_info{      
        
	unsigned int	wakeup_gpio;
	unsigned int	rst_gpio;
	unsigned int	irq_gpio;
	unsigned int	irq_number;   
    unsigned int	max_x_res;
    unsigned int	max_y_res;
	unsigned char	max_point_number;
	unsigned char	i2c_bus_number; 	
    unsigned char	x_revert;
    unsigned char	y_revert;
    unsigned char	xy_exchange;  
	unsigned char	ctp_used;
              
};


struct ts_chip_info{      
        
	unsigned char	chip_id; 
	unsigned char	pj_id;            
};

struct	ts_info{
	
	unsigned char dev_id;
	struct ts_driver	driver;
	struct ts_config_info	config_info;
	struct ts_chip_info	chip_info;
	struct ts_xy_data	xy_data;
	struct ts_vkeys		vkeys;
};





//void bl_ts_set_wakeup(unsigned char level);
//void bl_ts_set_irq(void);
//int bl_ts_i2c_read(__u8 i2c_addr, u16 reg_addr, u8 *rxbuf, s32 len);
//int bl_ts_i2c_write(__u8 i2c_addr, u8 *txbuf, s32 len);
//int bl_ts_i2c_transfer(__u8 i2c_addr, __u8 *buf, __u16 len,__u8 rw);



#endif

