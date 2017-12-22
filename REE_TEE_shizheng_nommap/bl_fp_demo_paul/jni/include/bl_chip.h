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
#define u64	uint64_t


#endif

struct bl_reg_value{
	u8 addr;
	u8 value;
};

typedef struct bl_chip_params {
	struct bl_reg_value *params;
	struct bl_reg_value hostcmd_reg;
	struct bl_reg_value fddacp_reg;
	struct bl_reg_value capdacp_reg;
	struct bl_reg_value fdgain_reg;
	struct bl_reg_value capgain_reg;
	u32 chipid;
	u32 fdwidth;
	u32 fdheight;
	u32 fdframe_num;
	u32 width;
	u32 height;
	u32 capframe_num;
	u32 paramslen;
} chip_params_t;

struct bl_fingerprint_data {
	chip_params_t *chip_params;
	int chiptype;
	int devfd;
	u32 buf_size;
    u32 reset_gpio;
    u32 irq_gpio;
    u32 irq_num;
    u32 power28_en_gpio;
    u32 power18_en_gpio;	
	u32 chipid;
	u32 is_force_set;
	u32 framewidth;
	u32 frameheight;
	void * bl_private;
	u8 *tx_buf;
	u8 nStatus;
	u8 frame_num;
	u8 mode;
	u8 gain;
	u8 dacp;
//

};
int bl_add_chip_params(struct bl_fingerprint_data *bl_data,struct bl_chip_params *params);
int bl_init_params(struct bl_fingerprint_data *bl_data);
#endif
