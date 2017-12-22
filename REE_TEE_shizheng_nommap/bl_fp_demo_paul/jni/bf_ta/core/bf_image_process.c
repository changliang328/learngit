#include "bf_image_process.h"
#include "finger_print.h"
#include "bf_log.h"
#include "bf_types.h"

int bf_cal_mean_value(bf_image_t *pbfimage,bf_algo_t *pbf_algo)
{
	int i,j;
	int width = pbfimage->width;
	int height = pbfimage->height;
	u32 sum_x = 0;
	u32 sum_y = 0;
	u32 area_count = 0;
	u32 mMeanValue = 0;
	u32 pixcelvalue = 0;
	u32 area = 0;
	u32 press_value = pbf_algo->nFingerDownValue;
	u8 *data = pbfimage->fpdata;
	
	for(i = 0;i < height; i++)
	{
		for(j = 0;j < width; j++)
		{
			pixcelvalue = *(data + i*width + j);
			mMeanValue += pixcelvalue;
			if(pixcelvalue < press_value)
			{
				sum_x += j;
				sum_y += i;	
				area_count++;	
			}
		}
	}
	area = area_count * 100 / width / height;
	if(area > 0)
	{
		mMeanValue = mMeanValue / width / height;
		pbfimage->uPosX = sum_x/area_count;
		pbfimage->uPosY = sum_y/area_count;
		pbfimage->uAreaCount = area_count;
		pbfimage->uMeanValue = mMeanValue;
	}else {
		pbfimage->uAreaCount = area_count;
		pbfimage->uMeanValue = 255;
	}
	return 0;
}

int bf_do_xu_imageEnhance_step2(bf_image_t *pbfimage, bf_algo_t *pbf_algo)
{
	int ret = 0;
	u8 *imgData = pbfimage->fpdata_enhance;
	int width = pbfimage->width;
	int height = pbfimage->height;
	
	//memcpy(imgData, pbfimage->fpdata, width * height);
	//save_image_bmp(imgData,"/data/system/users/0/fpdata/testorg.bmp", width, height);
	ret = FP_ImagePreproc_Step2(imgData, width, height);
	//save_image_bmp(imgData,"/data/system/users/0/fpdata/testenhance.bmp", width, height);
	BF_LOG("ret=%d", ret);
	return ret;
}

int bf_do_xu_imageEnhance_step1(bf_image_t *pbfimage, bf_algo_t *pbf_algo, u8 minValidArea)
{
	int ret = 0;
	u8 *imgData = pbfimage->fpdata_enhance;
	int width = pbfimage->width;
	int height = pbfimage->height;
	u8 nMinValidArea = minValidArea;
	bf_tee_memcpy((void *)imgData, pbfimage->fpdata, width * height);//memcpy(imgData, pbfimage->fpdata, width * height);
	//save_image_bmp(imgData,"/data/system/users/0/fpdata/testorg.bmp", width, height);
	ret = FP_ImagePreproc_Step1(imgData, width, height, 0, &nMinValidArea, pbf_algo->nRepairFlag);
	//save_image_bmp(imgData,"/data/system/users/0/fpdata/testenhance.bmp", width, height);
	pbfimage->uXuArea = nMinValidArea;
	BF_LOG("nMinValidArea=%d ret=%d",nMinValidArea, ret);
	return ret;
}


int bf_do_PB_imageQuality(bf_image_t *pbfimage)
{
	unsigned char nQuality;
	unsigned char nArea;
	unsigned char nCondition; //范围[0,100]
	u8 *imgData = pbfimage->fpdata_enhance;
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

int bf_do_xu_imageQuality(bf_image_t *pbfimage)
{
	unsigned char nQuality;
	unsigned char nArea;
	unsigned char nCondition; //范围[0,100]
	unsigned char nValidAreaMean;
	u8 *imgData = pbfimage->fpdata_enhance;
	int width = pbfimage->width;
	int height = pbfimage->height;
	int ret = 0;
	
	ret = FP_ImageQuality(imgData, width, height,&nQuality,&nArea, &nCondition, &nValidAreaMean);
	BF_LOG("nQuality=%d,nArea=%d,nCondition=%d nValidAreaMean =%d ret=%d", 
		nQuality, nArea, nCondition, nValidAreaMean, ret);
	pbfimage->uPBscore = nQuality;
	pbfimage->uPBarea = nArea;
	pbfimage->uPBcondition = nCondition;
	return ret;
}

