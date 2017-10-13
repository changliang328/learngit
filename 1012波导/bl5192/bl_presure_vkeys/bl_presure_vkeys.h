#ifndef _BL_PRESURE_VKEYS_H_
#define _BL_PRESURE_VKEYS_H_
//#include "tpd.h"
#include "bl_chip.h"
/**************************************/
struct bl_presure_data {
	struct i2c_client *client;
	struct input_dev *inputdev;
	u32 chipid;
    u32 presure_irq_gpio;
    u32 presure_irq_num;
    u32 presure_reg_addr;
    u32 leftkey_reg_addr;
    u32 rightkey_reg_addr;
	u32 fw_version_high;
	u32 fw_version_low;
    struct notifier_block fb_notify;
};
/**************************************/
int bl_getPresure(int type);
static inline int bl_get_sensor_version(void);
static int bl_check_update_fw(void);


/**************************************/
#define PRESURE_SLAVE_ADRR (0x2d)//(0x28)//(0x38)
#define PRESURE_REG_ADDR_HIGH (0x44)
#define PRESURE_REG_ADDR_LOW (0x45)
#define THREHOLD_PRESURE (10)//(10)
#define PRESURE_REG_ADDR (0xb6)
#define LEFT_KEY_REG_ADDR	(0XB3)
#define RIGHT_KEY_REG_ADDR	(0XB4)
#define VERSION_HIGH_REG	(0XB6)
#define VERSION_LOW_REG	(0XB7)
#define KEYBIT_CENTER	(1)
#define KEYBIT_LEFT		(1<<1)
#define KEYBIT_RIGHT	(1<<2)
/**************************************/
#endif	//_BL_PRESURE_VKEYS_H_
