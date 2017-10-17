#ifndef _BL_PROCESS_H_
#define _BL_PROCESS_H_
#define LOG_TAG "paultest"
#include <linux/types.h>
#include <unistd.h>
//#include <cutils/log.h>
#include <android/log.h>

//#define BTL_DEBUG(fmt, args...)  ALOGI("%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)
#define BTL_DEBUG(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"%5d: <%s>" fmt "\n",  __LINE__, __func__, ##args)

#ifndef u8
#define u8	uint8_t
#define u16 uint16_t
#define u32	uint32_t
#endif

struct bl_process_config {
	u32 is_mean;
	u32 is_centroid;
	u32 is_PB_score;
	u32 is_PB_score_org;
	u32 is_xu_precess;
	u32 uArea_xu_thres;
	u32 is_auto_dacp;
	u32 dacp_direction;
};

struct PBimage{
    int32_t  quality;
    int32_t  area;
    int32_t  imageValid;
    int32_t  condition;
	int32_t  org_area;
};

struct bl_process_data {
	struct bl_process_config *ptPreconfig;
	struct bl_fingerprint_data *ptBl_data;
	u32 height;
	u32 width;
	u32 uPosX;
	u32 uPosY;
	u32 uMeanValue;
	u32 uPBscore;
	u32 uPBscore_org;
	u32 uFuncmode;
	//
	struct PBimage tbImage;
	u8 *pImgdata_org;
	u8 *pImgdata_enh;
};
#endif
