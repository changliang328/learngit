#ifndef _BL_CHIP_H_
#define	_BL_CHIP_H_

#include "bl_ts.h"

#define		I2C_WRITE		0x00
#define		I2C_READ		0x01
#define		BL_FLASH_I2C_ADDR	0x2c
#define		CHIP_ID_REG		0xe7



#if(TS_CHIP == BL8XXX_60)

#define		MAX_FLASH_SIZE		0x8000
#define		PJ_ID_OFFSET		0x7b5b

#define 	I2C_TRANSFER_WSIZE      I2C_MAX_TRANSFER_SIZE//(128)
#define 	I2C_TRANSFER_RSIZE      I2C_MAX_TRANSFER_SIZE//(128)
#define 	I2C_VERFIY_SIZE         I2C_MAX_TRANSFER_SIZE//(128)

#define		FW_CHECKSUM_DELAY_TIME	100


enum BL86XX_flash_cmd {

	READ_MAIN_CMD		= 0X02,
	ERASE_ALL_MAIN_CMD	= 0X04,
	ERASE_PAGE_MAIN_CMD	= 0X06,
	WRITE_MAIN_CMD		= 0X08,
	RW_REGISTER_CMD		= 0X0a,
};

#elif(TS_CHIP == BL8XXX_61)

#define		MAX_FLASH_SIZE		0x8000
#define		PJ_ID_OFFSET		0xbb

#define 	I2C_TRANSFER_WSIZE      I2C_MAX_TRANSFER_SIZE//(64)
#define 	I2C_TRANSFER_RSIZE      I2C_MAX_TRANSFER_SIZE//(64)
#define 	I2C_VERFIY_SIZE         I2C_MAX_TRANSFER_SIZE//(64)

#define		FW_CHECKSUM_DELAY_TIME	100


enum BL88XX_flash_cmd {
	
	ERASE_SECTOR_MAIN_CMD	= 0X06,
	ERASE_ALL_MAIN_CMD	= 0X09,	
	RW_REGISTER_CMD		= 0X0a,
	READ_MAIN_CMD		= 0X0D,
	WRITE_MAIN_CMD		= 0X0F,
	WRITE_RAM_CMD		= 0X11,
	READ_RAM_CMD		= 0X12,
};

#elif(TS_CHIP == BL8XXX_63)

#define		MAX_FLASH_SIZE		0xc000
#define		PJ_ID_OFFSET		0xcb

#define 	I2C_TRANSFER_WSIZE      I2C_MAX_TRANSFER_SIZE//(256)
#define 	I2C_TRANSFER_RSIZE      I2C_MAX_TRANSFER_SIZE//(256)
#define 	I2C_VERFIY_SIZE         I2C_MAX_TRANSFER_SIZE//(256)

#define		FW_CHECKSUM_DELAY_TIME	250


enum BL88XX_flash_cmd {
	
	ERASE_SECTOR_MAIN_CMD	= 0X06,
	ERASE_ALL_MAIN_CMD	= 0X09,	
	RW_REGISTER_CMD		= 0X0a,
	READ_MAIN_CMD		= 0X0D,
	WRITE_MAIN_CMD		= 0X0F,
	WRITE_RAM_CMD		= 0X11,
	READ_RAM_CMD		= 0X12,
};
#endif

enum fw_reg {

	WORK_MODE_REG		= 0X00,
	CHECKSUM_REG		= 0x3f,
	CHECKSUM_CAL_REG	= 0x8a,
	AC_REG			= 0X8b,
	RESOLUTION_REG		= 0X98,
	LPM_REG			= 0Xa5,
	PROXIMITY_REG		= 0Xb0,
	PROXIMITY_FLAG_REG	= 0XB1,
	PJ_ID_REG		= 0Xb5,
};


enum work_mode {

	NORMAL_MODE		= 0x00,
	FACTORY_MODE		= 0x40,
};

enum lpm {

	ACTIVE_MODE		= 0x00,
	MONITOR_MODE		= 0x01,
	STANDBY_MODE		= 0x02,
	SLEEP_MODE		= 0x03,
	GESTURE_MODE		= 0x04,

};

enum checksum {

	CHECKSUM_READY		= 0x01,
	CHECKSUM_CAL		= 0xaa,
};

enum apk_cmd {

	WRITE_CMD		= 0x00,
	READ_CMD		= 0x01,
	WAKE_CMD		= 0x02,
	INTERRUPT_CMD		= 0x03,
	DRIVER_INFO		= 0x04,

};




//int bl_init_chip(struct ts_info *bl_ts);
//int bl_chip_switch_mode(unsigned char mode);
//int bl_send_power_state(unsigned char power_state);

//extern struct file_operations bl_apk_fops;

#endif


