#include <linux/types.h>
#include <unistd.h>
#include "bf_log.h"
#include "bf_algo.h"
#include "auto_gain_dacp.h"

typedef enum {
    BL_CHIPID_2390E = 0x5183,
    BL_CHIPID_3290 = 0x5183,
    BL_CHIPID_3182 = 0x5283,
    BL_CHIPID_3390 = 0x5383,
    BL_CHIPID_3590 = 0x5483,
    BL_CHIPID_KILBY = 0xD0F0,
    BL_CHIPID_MAX,
} chip_id_t;

bf_algo_t *bf_algo_new(int chiptype)
{
	struct bf_algo_data *pbf_algo = malloc(sizeof(struct bf_algo_data));
	bl_Alg_GetVersion(&pbf_algo->nVersion);
	pbf_algo->nAlgoID = 0;
	pbf_algo->nRepairFlag = 0;
	//BF_LOG(" BL_TEMPLATE=%d %d %d",sizeof(unsigned char),sizeof(unsigned char*),sizeof(unsigned int ));
	if(chiptype == BL_CHIPID_3290)
	{
		pbf_algo->initParam. nAlgorithmType = AT_NEO_HENRY_SPEED; //bf3290
		pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
		pbf_algo->initParam. nMaxNbrofSubtemplates = 96;
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
		//pbf_algo->nRepairFlag = 0x02;
	}
	else if(chiptype == BL_CHIPID_3182)
	{
		pbf_algo->initParam. nAlgorithmType = AT_NEO_GALTON_SPEED;
		pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
		pbf_algo->initParam. nMaxNbrofSubtemplates = 96;
		pbf_algo->initParam. bSupport360Rotate = 1;
		pbf_algo->dacp_direction = ST_PASSIVE;
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
	}
	else if(chiptype == BL_CHIPID_3390)
	{
		pbf_algo->initParam. nAlgorithmType = AT_NEO_SQUARE_XXS;
		pbf_algo->initParam. nMaxTemplateSize = 150 * 1024;
		pbf_algo->initParam. nMaxNbrofSubtemplates = 96;
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
	}
	else if(chiptype == BL_CHIPID_3590)
	{
		pbf_algo->initParam. nAlgorithmType = AT_NEO_SQUARE_XXS;
		pbf_algo->initParam. nMaxTemplateSize = 150*1024;
		pbf_algo->initParam. nMaxNbrofSubtemplates = 96;
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
	}
	pbf_algo->nXuAreaMatch = 50;
	pbf_algo->nXuAreaUpdate = 95;
	pbf_algo->nXuAreaEnroll = 60;
	pbf_algo->nMaxSample = 15;
	
	bl_Alg_Init(pbf_algo->nAlgoID, &pbf_algo->initParam);
	BF_LOG("algo id=%d version=%d,width=%d,height=%d",pbf_algo->nAlgoID,pbf_algo->nVersion,pbf_algo->width,pbf_algo->height);
	BF_LOG("pb_area_max=%d,pb_area_match_threhold=%d,pb_area_enroll_threhold=%d",pbf_algo->pb_area_max,pbf_algo->pb_area_match_threhold,pbf_algo->pb_area_enroll_threhold);
	BF_LOG("far_match=%d far_update=%d,qscore_match=%d,qscore_enroll=%d,dacp_dir=%d",pbf_algo->far_match,pbf_algo->far_update, pbf_algo->qscore_match,pbf_algo->qscore_enroll,pbf_algo->dacp_direction);
	BF_LOG("algo type=%d,Tmaxsize=%d,TmaxNum=%d,Rotate=%d",pbf_algo->initParam. nAlgorithmType,	pbf_algo->initParam. nMaxTemplateSize,pbf_algo->initParam.nMaxNbrofSubtemplates,	pbf_algo->initParam. bSupport360Rotate);
	return pbf_algo;
}

int bf_algo_destroy(bf_algo_t *pbf_algo)
{
	bl_Alg_UnInit(pbf_algo->nAlgoID);
	free(pbf_algo);
	return 0;
}

