//#include "bl_fingerprint.h"
#include "bl_process.h"
#include "auto_gain_dacp.h"
#include "finger_print.h"
#define PRESS_PIXCEL_VAL (230)

u32 cal_mean_value(struct bl_process_data *bl_pro_data,u8 *buf,u32 height,u32 width)
{
	int i,j;
	u32 sum_x = 0;
	u32 sum_y = 0;
	u32 area_count = 0;
	u32 mMeanValue = 0;
	u32 pixcelvalue = 0;
	
	for(i = 0;i < height - 1; i++)//ȥ\B5\F4\BAڱ\DF
	{
		for(j = 0;j < width; j++)
		{
			pixcelvalue = *(buf + i*width + j);
			mMeanValue += pixcelvalue;
			if(pixcelvalue < PRESS_PIXCEL_VAL)
			{
				sum_x += j;
				sum_y += i;	
				area_count++;	
			}
		}
	}
	mMeanValue = mMeanValue/width/height;
	if(mMeanValue < PRESS_PIXCEL_VAL)
	{
		bl_pro_data->uPosX = sum_x/area_count;
		bl_pro_data->uPosY = sum_y/area_count;
		bl_pro_data->uMeanValue = mMeanValue;
	}
}

int init_precess_data(struct bl_process_data *bl_pro_data,struct bl_process_config *config,u8 *img_buf,u32 height,u32 width)
{
	bl_pro_data->pImgdata_org = img_buf;
	bl_pro_data->ptPreconfig = config;
	bl_pro_data->pImgdata_enh = malloc(height * width); 
	bl_pro_data->height = height;
	bl_pro_data->width = width;
	return 0;
} 

int do_precess_by_config(struct bl_process_data *bl_pro_data)
{
	int ret = 0;
	struct bl_process_config *pconfig = bl_pro_data->ptPreconfig;
	u32 height = bl_pro_data->height;
	u32 width = bl_pro_data->width;
	u8 *imgOrg = bl_pro_data->pImgdata_org;
	u8 *imgEnh = bl_pro_data->pImgdata_enh;
	struct PBimage *pPBimage = &bl_pro_data->tbImage;
	u32 dacp_direction = pconfig->dacp_direction;
	
	if(pconfig->is_mean || pconfig->is_centroid)
	{
		cal_mean_value(bl_pro_data,imgOrg,height,width);
	}
	
	if(pconfig->is_xu_precess||pconfig->is_PB_score)
	{
		memcpy(imgEnh, imgOrg, height * width);
	}
	
	if(pconfig->is_xu_precess)
	{
		//ret = FP_ImagePreproc(imgEnh, width, height, 0, &(pconfig->uArea_xu_thres), 0x0f);
	}
	
	if(pconfig->is_PB_score)
	{
		//Btl_CheckImageQuality(imgEnh, width, height, 1, 1, &(pPBimage->quality), &(pPBimage->area),&(pPBimage->condition),0);
	}
	
	if(pconfig->is_PB_score_org)
	{
		//Btl_CheckImageQuality(imgOrg, width, height, 1, 1, &(pPBimage->quality), &(pPBimage->area),&(pPBimage->condition),0);
	}
	
	if(pconfig->is_auto_dacp)
	{
		//bl_AutoGainDacp(imgOrg, width , height,  &oldRegValue, &newRegValue,128, dacp_direction);
	}
}
