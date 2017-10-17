#include "bf_image_process.h"
#include "finger_print.h"
#include "bf_log.h"

#define PRESS_PIXCEL_VAL (230)

int bf_cal_mean_value(bf_image_t *pbfimage)
{
	int i,j;
	int width = pbfimage->width;
	int height = pbfimage->height;
	u32 sum_x = 0;
	u32 sum_y = 0;
	u32 area_count = 0;
	u32 mMeanValue = 0;
	u32 pixcelvalue = 0;
	u8 *data = pbfimage->fpdata;
	
	for(i = 0;i < height; i++)
	{
		for(j = 0;j < width; j++)
		{
			pixcelvalue = *(data + i*width + j);
			mMeanValue += pixcelvalue;
			if(pixcelvalue < PRESS_PIXCEL_VAL)
			{
				sum_x += j;
				sum_y += i;	
				area_count++;	
			}
		}
	}
	mMeanValue = mMeanValue / width / height;
	if(mMeanValue < PRESS_PIXCEL_VAL)
	{
		pbfimage->uPosX = sum_x/area_count;
		pbfimage->uPosY = sum_y/area_count;
		pbfimage->uMeanValue = mMeanValue;
	}
	return 0;
}

int bf_do_xu_imageEnhance(bf_image_t *pbfimage, bf_algo_t *pbf_algo, u8 minValidArea)
{
	int ret = 0;
	u8 *imgData = pbfimage->fpdata;
	int width = pbfimage->width;
	int height = pbfimage->height;
	u8 nMinValidArea = minValidArea;
	
	ret = FP_ImagePreproc(imgData, width, height, 0, &nMinValidArea, pbf_algo->nRepairFlag);
	pbfimage->uXuArea = nMinValidArea;
	BF_LOG("nMinValidArea=%d ret=%d",nMinValidArea, ret);
	return ret;
}

int bf_do_PB_imageQuality(bf_image_t *pbfimage)
{
	unsigned char nQuality;
	unsigned char nArea;
	unsigned char nCondition; //范围[0,100]
	u8 *imgData = pbfimage->fpdata;
	int width = pbfimage->width;
	int height = pbfimage->height;
	int ret = 0;
	
	ret = bl_Alg_ImageQuality(imgData, width, height,&nQuality,&nArea, &nCondition);
	BF_LOG("nQuality=%d,nArea=%d,nCondition=%d ret=%d", nQuality, nArea, nCondition,ret);
	pbfimage->uPBscore = nQuality;
	pbfimage->uPBarea = nArea;
	pbfimage->uPBcondition = nCondition;
	return ret;
}
