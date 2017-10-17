#ifndef _BL_CHIP_H_
#define _BL_CHIP_H_
// work mode
#define MODE_IDLE					0x00
#define MODE_RC_DT					0x01
#define MODE_FG_DT					0x02
#define MODE_FG_PRINT				0x03
#define MODE_FG_CAP					0x04
#define MODE_NAVI					0x05
#define MODE_CLK_CALI				0x06
#define MODE_PIEXL_TEST				0x07

//reg addr
#define RRG_FRAME_ROW_START	(0x14)
#define RRG_FRAME_ROW_LEN	(0x15)
#define RRG_FRAME_COL_START_LEN	(0x16)
#define REGA_FINGER_CAP		(0x27)
#define REGA_INTR_STATUS			(0x28)

#define FRAME_DUMMY_LEN	(2)

#ifndef u8
#define u8	uint8_t
#define u16 uint16_t
#define u32	uint32_t
#endif
#pragma pack(1)
#include <pthread.h> 
typedef enum {
    BL_FP_CHIP_2390E = 0,
    BL_FP_CHIP_3290 = 1,
    BL_FP_CHIP_3182 = 2,
    BL_FP_CHIP_3390 = 3,
    BL_FP_CHIP_KILBY = 4,
    BL_FP_CHIP_MAX,
} chip_type_t;
typedef enum {
    BL_CHIPID_2390E = 0x5183,
    BL_CHIPID_3290 = 0x5183,
    BL_CHIPID_3182 = 0x5283,
    BL_CHIPID_3390 = 0x5383,
    BL_CHIPID_KILBY = 0xD0F0,
    BL_CHIPID_MAX,
} chip_id_t;

struct bl_reg_value{
	u8 addr;
	u8 value;
};

typedef struct bl_chip_params {
	//int (*dev_init)(struct bl_fingerprint_data *bl_data);
	//int (*interrupt_init)(struct bl_fingerprint_data *bl_data);
	//int (*capture_init)(struct bl_fingerprint_data *bl_data);
	//int (*get_int_status)(struct bl_fingerprint_data *bl_data);	
	//int (*read_chipid)(struct bl_fingerprint_data *bl_data);	
	u32 chipid;
	struct bl_reg_value *params;
	struct bl_reg_value hostcmd_reg;
	struct bl_reg_value fddacp_reg;
	struct bl_reg_value capdacp_reg;
	struct bl_reg_value fdgain_reg;
	struct bl_reg_value capgain_reg;
	u32 fdwidth;
	u32 fdheight;
	u32 fdframe_num;
	u32 width;
	u32 height;
	u32 capframe_num;
	u32 paramslen;
} chip_params_t;

struct bl_fingerprint_data {
	int devfd;
	u32 buf_size;
    u32 reset_gpio;
    u32 irq_gpio;
    u32 irq_num;
    u32 power28_en_gpio;
    u32 power18_en_gpio;

	pthread_cond_t int_wait_cond;
	pthread_mutex_t int_mutex;

//chip params
	chip_params_t *chip_params;
	chip_type_t chiptype;
	u32 chipid;
	u32 is_force_set;
	u32 framewidth;
	u32 frameheight;
	u8 nStatus;
	u8 *tx_buf;
	u8 frame_num;
	u8 mode;
	u8 gain;
	u8 dacp;
//

};

int bl_init_params(struct bl_fingerprint_data *bl_data);
#endif
