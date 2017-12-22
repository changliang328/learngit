#include "bf_custom.h"
#include "bf_algo.h"
#include "auto_gain_dacp.h"
#include "bf_config.h"
#include "stdio.h"

#include "bf_types.h"

bf_algo_t *bf_algo_new(int chiptype, void * config)
{
	struct bf_algo_data *pbf_algo = bf_tee_malloc(sizeof(struct bf_algo_data));
	load_config_t *configs = NULL;
	//add by jaston for check null pointer
	if(pbf_algo == NULL)
		{
		BF_LOG("bf_algo_data  null , exit !!! ");
		return -1;//exit(0);
	}
	//add end
	bl_Alg_GetVersion(&pbf_algo->nVersion);
	pbf_algo->nAlgoID = 0;
	pbf_algo->nRepairFlag = 0;
	//BF_LOG(" BL_TEMPLATE=%d %d %d",sizeof(unsigned char),sizeof(unsigned char*),sizeof(unsigned int ));
		pbf_algo->nXuAreaMatch = 50;
		pbf_algo->nXuAreaUpdate = 80;
		pbf_algo->nXuAreaEnroll = 95;
	if(NULL != config)
	{		
		BF_LOG("use params from cfg file");
		configs = (load_config_t*)config;
		pbf_algo->initParam. nAlgorithmType = configs->nAlgorithmType;
		pbf_algo->initParam. nMaxTemplateSize = configs->nMaxTemplateSize;
		pbf_algo->initParam. run_interleaved = NULL;
		pbf_algo->initParam. bSupport360Rotate = configs->bSupport360Rotate;
		pbf_algo->width = configs->width;
		pbf_algo->height = configs->height;
		pbf_algo->pb_area_max = configs->pb_area_max;
		pbf_algo->pb_area_match_threhold = configs->pb_area_match_threhold;
		pbf_algo->pb_area_enroll_threhold = configs->pb_area_enroll_threhold;
		pbf_algo->far_match = configs->far_match;
		pbf_algo->far_update = configs->far_update;
		pbf_algo->qscore_match = configs->qscore_match;
		pbf_algo->qscore_enroll = configs->qscore_enroll;
		pbf_algo->dacp_direction = configs->dacp_direction;

		pbf_algo->nXuAreaMatch = configs->nXuAreaMatch;
		pbf_algo->nXuAreaUpdate = configs->nXuAreaUpdate;
		pbf_algo->nXuAreaEnroll = configs->nXuAreaEnroll;
		pbf_algo->nMaxSample = configs->nMaxSample;
		pbf_algo->nFingerDownValue = configs->nFingerDownValue;	
		pbf_algo->nBadPixelMaxValue  = configs->nBadPixelMaxValue;
		pbf_algo->nBadPixelsNumThre  = configs->nBadPixelsNumThre;

	}else
	{
		if(chiptype == BL_CHIPID_3290)
		{
			pbf_algo->initParam. nAlgorithmType = AT_NEO_HENRY_SPEED; //bf3290
			pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
			pbf_algo->initParam. run_interleaved = NULL;
			pbf_algo->initParam. bSupport360Rotate = 1;
			pbf_algo->width = 112;
			pbf_algo->height = 96;
			pbf_algo->pb_area_max = 100;
			pbf_algo->pb_area_match_threhold = pbf_algo->pb_area_max * 1 / 10;
			pbf_algo->pb_area_enroll_threhold = pbf_algo->pb_area_max * 19 / 20;
			pbf_algo->far_match = BL_FAR_50000;
			pbf_algo->far_update = BL_FAR_100000;
			pbf_algo->qscore_match = 30;
			pbf_algo->qscore_enroll = 35;
			pbf_algo->dacp_direction = ST_PASSIVE;
			pbf_algo->nBadPixelMaxValue  = 10;
			pbf_algo->nBadPixelsNumThre  = 140;
			pbf_algo->nXuAreaMatch = 50;
			pbf_algo->nXuAreaUpdate = 80;
			pbf_algo->nXuAreaEnroll = 80;
		}
		else if(chiptype == BL_CHIPID_3182)
		{
			pbf_algo->initParam. nAlgorithmType = AT_NEO_GALTON_SPEED;
			pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
			pbf_algo->initParam. run_interleaved = NULL;
			pbf_algo->initParam. bSupport360Rotate = 1;
			pbf_algo->width = 72;
			pbf_algo->height = 128;
			pbf_algo->pb_area_max = 100;
			pbf_algo->pb_area_match_threhold = pbf_algo->pb_area_max * 1 / 10;
			pbf_algo->pb_area_enroll_threhold = pbf_algo->pb_area_max * 19 / 20;
			pbf_algo->far_match = BL_FAR_50000;
			pbf_algo->far_update = BL_FAR_100000;
			pbf_algo->qscore_match = 30;
			pbf_algo->qscore_enroll = 35;
			pbf_algo->dacp_direction = ST_PASSIVE;
			pbf_algo->nBadPixelMaxValue  = 10;
			pbf_algo->nBadPixelsNumThre  = 140;
			pbf_algo->nXuAreaMatch = 50;
			pbf_algo->nXuAreaUpdate = 80;
			pbf_algo->nXuAreaEnroll = 80;

		}
		else if(chiptype == BL_CHIPID_3390)
		{
			pbf_algo->initParam. nAlgorithmType = AT_NEO_SQUARE_XXS;
			pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
			pbf_algo->initParam. run_interleaved = NULL;
			pbf_algo->initParam. bSupport360Rotate = 1;
			pbf_algo->width = 80;
			pbf_algo->height = 80;
			pbf_algo->pb_area_max = 100;
			pbf_algo->pb_area_match_threhold = pbf_algo->pb_area_max * 1 / 10;
			pbf_algo->pb_area_enroll_threhold = pbf_algo->pb_area_max * 19 / 20;
			pbf_algo->far_match = BL_FAR_50000;
			pbf_algo->far_update = BL_FAR_100000;
			pbf_algo->qscore_match = 30;
			pbf_algo->qscore_enroll = 35;
			pbf_algo->dacp_direction = ST_PASSIVE;
			pbf_algo->nBadPixelMaxValue  = 10;
			pbf_algo->nBadPixelsNumThre  = 100;
		}
		else if(chiptype == BL_CHIPID_3590)
		{
			pbf_algo->initParam. nAlgorithmType = AT_CARDO_SQUARE_XXS;
			pbf_algo->initParam. nMaxTemplateSize = 150*1024;
			pbf_algo->initParam. run_interleaved = NULL;
			pbf_algo->initParam. bSupport360Rotate = 1;
			pbf_algo->width = 64;
			pbf_algo->height = 80;
			pbf_algo->pb_area_max = 100;
			pbf_algo->pb_area_match_threhold = pbf_algo->pb_area_max * 1 / 10;
			pbf_algo->pb_area_enroll_threhold = pbf_algo->pb_area_max * 19 / 20;
			pbf_algo->far_match = BL_FAR_50000;
			pbf_algo->far_update = BL_FAR_100000;
			pbf_algo->qscore_match = 30;
			pbf_algo->qscore_enroll = 35;
			pbf_algo->dacp_direction = ST_JACK;
			pbf_algo->nBadPixelMaxValue  = 10;
			pbf_algo->nBadPixelsNumThre  = 100;
		}else if(chiptype == BL_CHIPID_81192)
		{
			pbf_algo->initParam. nAlgorithmType = AT_HYBRID_SQUARE_XL_SPEED_MEM;
			pbf_algo->initParam. nMaxTemplateSize = 50*1024;
			pbf_algo->initParam. run_interleaved = NULL;
			pbf_algo->initParam. bSupport360Rotate = 1;
			pbf_algo->width = 192;
			pbf_algo->height = 192;
			pbf_algo->pb_area_max = 100;
			pbf_algo->pb_area_match_threhold = pbf_algo->pb_area_max * 1 / 10;
			pbf_algo->pb_area_enroll_threhold = pbf_algo->pb_area_max * 19 / 20;
			pbf_algo->far_match = BL_FAR_50000;
			pbf_algo->far_update = BL_FAR_100000;
			pbf_algo->qscore_match = 30;
			pbf_algo->qscore_enroll = 35;
			pbf_algo->dacp_direction = ST_JACK;
			pbf_algo->nBadPixelMaxValue  = 10;
			pbf_algo->nBadPixelsNumThre  = 100;
			
			pbf_algo->nXuAreaMatch = 50;
			pbf_algo->nXuAreaUpdate = 70;
			pbf_algo->nXuAreaEnroll = 70;
		}

		pbf_algo->nMaxSample = 3;
		pbf_algo->nFingerDownValue = 230;		
	}
	
	
	
	bl_Alg_Init(pbf_algo->nAlgoID, &pbf_algo->initParam);
	BF_LOG("algo id=%d version=%d,width=%d,height=%d",pbf_algo->nAlgoID,pbf_algo->nVersion,pbf_algo->width,pbf_algo->height);
	BF_LOG("pb_area_max=%d,pb_area_match_threhold=%d,pb_area_enroll_threhold=%d",pbf_algo->pb_area_max,pbf_algo->pb_area_match_threhold,pbf_algo->pb_area_enroll_threhold);
	BF_LOG("far_match=%d far_update=%d,qscore_match=%d,qscore_enroll=%d,dacp_dir=%d",pbf_algo->far_match,pbf_algo->far_update, pbf_algo->qscore_match,pbf_algo->qscore_enroll,pbf_algo->dacp_direction);
	BF_LOG("algo type=%d,Tmaxsize=%d,Rotate=%d",pbf_algo->initParam. nAlgorithmType,	pbf_algo->initParam. nMaxTemplateSize,	pbf_algo->initParam. bSupport360Rotate);
	BF_LOG("\n nBadPixelMaxValue%d\n\
		nBadPixelsNumThre:%d\n", 	pbf_algo->nBadPixelMaxValue,
									pbf_algo->nBadPixelsNumThre);
	return pbf_algo;
}

int bf_algo_destroy(bf_algo_t *pbf_algo)
{
	bl_Alg_UnInit(pbf_algo->nAlgoID);
	bf_tee_free(pbf_algo);
	return 0;
}

