#include "bl_fingerprint.h"
#include "bl_chip.h"
#include "bf_config.h"


/*******************************************************
init params for 2390e
*******************************************************/
static struct bl_reg_value reg_values_2390e[] = {
{0x11,0x0},
{0x12,0x4},
{0xa,0x0},
{0xb,0x5},
{0x2,0x11},
{0x17,0x2c},
{0xd,0x1b},
{0x17,0xac},
{0x3a,0xf},
{0x39,0x75},
{0xe,0x1},
{0x1d,0x0},
{0x1e,0x0},
{0x1f,0x0},
{0x20,0x0},
{0x21,0x0},
{0x22,0x0},
{0x23,0x0},
{0x24,0x0},
{0x25,0x0},
{0x26,0x0},
{0x27,0x0},
{0x28,0x0},
{0x31,0x7f},
{0x32,0x48},
{0x1b,0x3f},
{0xff, 0xff},
};
static struct bl_chip_params chip_params_2390e = {
	.params = reg_values_2390e,
	.hostcmd_reg = {0x13,0x0},
	.fddacp_reg = {0x1b,0x3f},
	.capdacp_reg = {0x1b,0x3f},
	.fdgain_reg = {0x31,0x7f},
	.capgain_reg = {0x31,0x7f},
	.fdwidth = 32,
	.fdheight = 32,
	.fdframe_num = 0,
	.width = 112,
	.height = 96,
	.capframe_num = 3,
};

/*******************************************************
init params for 3290
*******************************************************/
static struct bl_reg_value reg_values_3290[] = {
{0x12, 0x6},
{0x11, 0x0},
{0xa, 0x0},
{0xb, 0x5},
{0x2, 0x11},
{0x17, 0x2c},
{0xd, 0x1b},
{0x17, 0xac},
{0x2d, 0xf0},
{0x3d, 0x75},
{0x33, 0x92},
{0x35, 0x52},
{0x1d, 0xff},
{0x1e, 0xff},
{0x1f, 0xff},
{0x20, 0xff},
{0x21, 0xff},
{0x22, 0xff},
{0x23, 0xff},
{0x24, 0xff},
{0x25, 0xff},
{0x26, 0xff},
{0x27, 0xff},
{0x28, 0xff},
{0x31, 0x2e},
{0x32, 0x48},
{0x1b, 0x20},
{0xff, 0xff},
};
static struct bl_chip_params chip_params_3290 = {
	.params = reg_values_3290,
	.hostcmd_reg = {0x13,0x0},
	.fddacp_reg = {0x1b,0x70},
	.capdacp_reg = {0x1b,0x70},
	.fdgain_reg = {0x31,0x56},
	.capgain_reg = {0x31,0x56},
	.fdwidth = 32,
	.fdheight = 32,
	.fdframe_num = 0,
	.width = 112,
	.height = 96,
	.capframe_num = 3,
};

/*******************************************************
init params for 3182
*******************************************************/
static struct bl_reg_value reg_values_3182[] = {
{0x2d,0x60},
{0x2b,0x3b},
{0x10,0x8},
{0x11,0x0},
{0x12,0x4},
{0x25,0x0},
{0x26,0x5},
{0x1b,0x40},
{0x1c,0x40},
{0x1d,0xff},
{0x1e,0xff},
{0x31,0x3f},
{0x32,0x48},
{0xff, 0xff},
};
static struct bl_chip_params chip_params_3182 = {
	.params = reg_values_3182,
	.hostcmd_reg = {0x34,0x0},
	.fddacp_reg = {0x1b,0x40},
	.capdacp_reg = {0x1c,0x40},
	.fdgain_reg = {0x31,0x67},
	.capgain_reg = {0x31,0x67},
	.fdwidth = 32,
	.fdheight = 32,
	.fdframe_num = 0,
	.width = 72,
	.height = 128,
	.capframe_num = 7,
};

/*******************************************************
init params for 3390
*******************************************************/
static struct bl_reg_value reg_values_3390[] = {
{0x2b, 0x7b},
{0x10, 0x48},
{0x11, 0x0},
{0x12, 0x6},
{0x25, 0x0},
{0x26, 0x5},
{0x1b, 0xff},
{0x1c, 0xff},
{0x20, 0xff},
{0x9, 0xff},
{0xa, 0xff},
{0xb, 0xff},
{0xc, 0xff},
{0xd, 0xff},
{0xe, 0xff},
{0xf, 0xff},
{0x1d, 0x9e},
{0x1e, 0x9e},
{0x31, 0x38},
{0x32, 0x48},
{0xff, 0xff},
};
static struct bl_chip_params chip_params_3390 = {
	.params = reg_values_3390,
	.hostcmd_reg = {0x34,0x0},
	.fddacp_reg = {0x1d,0xaa},
	.capdacp_reg = {0x1e,0xaa},
	.fdgain_reg = {0x31,0x30},
	.capgain_reg = {0x31,0x30},
	.fdwidth = 80,
	.fdheight = 32,
	.fdframe_num = 0,
	.width = 80,
	.height = 80,
	.capframe_num = 7,
};

/*******************************************************
init params for 3590
*******************************************************/
static struct bl_reg_value reg_values_3590[] = {
{0x2b,0x38},
{0x10,0x58},
{0x11,0x0},
{0x12,0x6},
{0x25,0x0},
{0x26,0x5},
{0x1b,0x8c},
{0x1c,0x8c},
{0x1d,0x90},
{0x1e,0x90},
{0x31,0xff},
{0x32,0x33},
{0x17,0x1},
{0x2d,0xe1},
{0x39,0xb3},
{0xff, 0xff},
};
static struct bl_chip_params chip_params_3590 = {
	.params = reg_values_3590,
	.hostcmd_reg = {0x34,0x0},
	.fddacp_reg = {0x1b,0x90},
	.capdacp_reg = {0x1c,0x90},
	.fdgain_reg = {0x31,0x6e},
	.capgain_reg = {0x31,0x6e},
	.fdwidth = 64,
	.fdheight = 32,
	.fdframe_num = 0,
	.width = 64,
	.height = 80,
	.capframe_num = 7,
};

/*******************************************************
all chip init params 
*******************************************************/
static struct bl_chip_params *all_chip_params[BL_FP_CHIP_MAX] = {
	&chip_params_2390e,
	&chip_params_3290,
	&chip_params_3182,
	&chip_params_3390,
	&chip_params_3590,
};

int bl_add_chip_params(struct bl_fingerprint_data *bl_data,struct bl_chip_params *params)
{
	int i = 0;
	for (i = 0; i < BL_FP_CHIP_MAX; i++) {
		/* add tpd driver into list */
		if (all_chip_params[i] == NULL) {
			all_chip_params[i] = params;
			break;
		}
	}
	return 0;
}

/*******************************************************
select chip init params 
*******************************************************/

int bl_init_params(struct bl_fingerprint_data *bl_data)
{

	chip_params_t *chip_params = NULL;
	load_config_t *config = NULL;
	int i = 0;
	BTL_DEBUG(" bl_data->chiptype=%d ++\n",bl_data->chiptype);
	
	if(NULL != bl_data->private)
	{
		BTL_DEBUG("use config file params\n");
		config = (load_config_t*)bl_data->private;

		/*construct chip_params struct*/
		chip_params = malloc(sizeof(chip_params_t));
		chip_params->params = NULL;
		chip_params->params = malloc(MAX_INIT_REGS);
		if(!chip_params || !chip_params->params)
		{	
			goto err;
		}

		memcpy(chip_params->params, &config->init_params, MAX_INIT_REGS);
		chip_params->hostcmd_reg = config->hostcmd_reg;
		chip_params->fddacp_reg = config->fddacp_reg;
		chip_params->capdacp_reg = config->capdacp_reg;
		chip_params->fdgain_reg = config->fdgain_reg;
		chip_params->capgain_reg = config->capgain_reg;

#ifdef CONFIG_DEBUG
		while(chip_params->params[i].addr != 0xff)
		{
			BTL_DEBUG("addr:0x%x   value:0x%x",chip_params->params[i].addr, chip_params->params[i].value);
			i++;
		}
		BTL_DEBUG("hostcmd_reg addr:0x%x   value:0x%x",chip_params->hostcmd_reg.addr, chip_params->hostcmd_reg.value);
		BTL_DEBUG("fddacp_reg addr:0x%x   value:0x%x",chip_params->fddacp_reg.addr, chip_params->fddacp_reg.value);
		BTL_DEBUG("capdacp_reg addr:0x%x   value:0x%x",chip_params->capdacp_reg.addr, chip_params->capdacp_reg.value);
		BTL_DEBUG("fdgain_reg addr:0x%x   value:0x%x",chip_params->fdgain_reg.addr, chip_params->fdgain_reg.value);
		BTL_DEBUG("capgain_reg addr:0x%x   value:0x%x",chip_params->capgain_reg.addr, chip_params->capgain_reg.value);
#endif		

		chip_params->chipid = config->chipid;
		chip_params->fdwidth = config->fdwidth;
		chip_params->fdheight = config->fdheight;
		chip_params->fdframe_num = config->fdframe_num;
		chip_params->width = config->width;
		chip_params->height = config->height;
		chip_params->capframe_num = config->capframe_num;
		chip_params->paramslen = 0;		//not use.

#ifdef CONFIG_DEBUG
		BTL_DEBUG("chipid :0x%x",chip_params->chipid);
		BTL_DEBUG("fdwidth :%d",chip_params->fdwidth);
		BTL_DEBUG("fdheight :%d",chip_params->fdheight);
		BTL_DEBUG("fdframe_num :%d",chip_params->fdframe_num);
		BTL_DEBUG("width :%d",chip_params->width);
		BTL_DEBUG("height :%d",chip_params->height);
		BTL_DEBUG("capframe_num :%d",chip_params->capframe_num);
		BTL_DEBUG("paramslen :%d",chip_params->paramslen);
#endif
		bl_data->chip_params = chip_params;
	}
	else
	{
		BTL_DEBUG("load default config parameter!\n");
		bl_data->chip_params = all_chip_params[bl_data->chiptype];
	}
	return 0;

err:
	BTL_DEBUG("malloc fail\n");
	bl_data->chip_params = all_chip_params[bl_data->chiptype];
	if(chip_params->params)
		free(chip_params->params);
	if(chip_params)
		free(chip_params);
		
	return 0;
}
