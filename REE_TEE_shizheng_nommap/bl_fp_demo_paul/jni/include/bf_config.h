#ifndef __BF_CONFIG_H__
#define __BF_CONFIG_H__
#include "bl_chip.h"
#include "cJSON.h"

#define MAX_INIT_REGS   (50)
//#define CONFIG_DEBUG


typedef struct {
	u32 tee_cmd;					//for tee
	u8 nMaxNbrofSubtemplates;  // 多模板所含子模板最大个数
	u8 bSupport360Rotate;      // 是否支持360度匹配, 1表示支持，0表示不支持
	u32 nAlgorithmType;      // 算法类型选择
	u32 nMaxTemplateSize;        // 多模板最大字节数

	u32 chipid;
	u32 chiptype;
	u32 fdwidth;
	u32 fdheight;
	u32 fdframe_num;
	u32 width;
	u32 height;
	u32 capframe_num;
	u32 paramslen;
	u32 force_loading;

	u32 nAlgoID;
	u32 pb_area_max;
	u32 pb_area_match_threhold;
	u32 pb_area_enroll_threhold;
	u32 nXuAreaMatch;
	u32 nXuAreaUpdate;
	u32 nXuAreaEnroll;
	u32 far_match;
	u32 far_update;
	u32 qscore_match;
	u32 qscore_enroll;
	u32 dacp_direction;
	u32 nVersion;
	u32 nRepairFlag;
	u32 nFingerDownValue;
	u32 nMaxSample;
	u32 nBadPixelMaxValue;
	u32 nBadPixelsNumThre;

	struct bl_reg_value hostcmd_reg;
	struct bl_reg_value fddacp_reg;
	struct bl_reg_value capdacp_reg;
	struct bl_reg_value fdgain_reg;
	struct bl_reg_value capgain_reg;
	struct bl_reg_value init_params[MAX_INIT_REGS];
}load_config_t;

void * bf_load_config(const char *configName);
int load_algo_params(cJSON * root, load_config_t *config);
int load_cmdreg_params(cJSON * root, load_config_t *config);
int load_chip_init_params(cJSON * root, load_config_t *config);
int load_single_chip_value(cJSON *root, load_config_t* config);
#endif
