#include "bf_custom.h"
#include "bf_core.h"
#include "bf_types.h"
#include "bf_crc.h"
#include "bf_image_process.h"
#include<stdio.h>
#include <sys/types.h>    
#include "auto_gain_dacp.h"
#include "bf_tee_platform_api.h"
#include "bf_log.h"
#include "finger_print.h"

static uint8_t g_hmac_key[32];
static uint32_t g_hmac_key_len = 32;
static bf_core_t *gbf_core = NULL;

int bf_core_init(void * config)
{
	int i=0;
	BF_LOG("++++");

	if(gbf_core != NULL)
		return 0;
	bf_tee_platform_t *pbf_plat = bf_tee_platform_init();
	bf_core_t *pbf_core = bf_tee_malloc(sizeof(bf_core_t));
	//chip data
	pbf_core->bl_data = bl_fingerprint_data_new(pbf_plat->devfd, config);

	//add by jaston for check null pointer,restart renew
	if(pbf_core->bl_data == NULL)
	{
		BF_LOG("bl_fingerprint_data_new error,exit here !!!");
		goto err;
	}
	//add end

	//init algo
	pbf_core->pbf_algo = bf_algo_new(pbf_core->bl_data->chipid, config);
	
	//pbfimage,alloc 2buf ,one for origin,one enhance
	pbf_core->pbfimage = bf_image_new(pbf_core->pbf_algo->width,pbf_core->pbf_algo->height);
	
	//template manager, to load and save templates
	pbf_core->pbfTemplateMgr = bf_template_manager_new(pbf_core->pbf_algo);

	g_hmac_key_len = sizeof(g_hmac_key);
	bf_tee_hmac_key(g_hmac_key, g_hmac_key_len);
#if 0
        for( i=0;i<g_hmac_key_len;i++)
	  {
		  BF_LOG("hmac_key[%d]=%x  \n",i, g_hmac_key[i]);
	  }
#endif	
	gbf_core = pbf_core;

	return 0;
err:
	BF_LOG("++++");
	if(pbf_core)
		bf_tee_free(pbf_core);
	return -1;
}

int bf_core_uninit()
{
	BF_LOG("++++");
	#ifndef BUILD_TEE
	if(gbf_core != NULL)
	{
		destroy_fingerprint_data(gbf_core->bl_data);
		bf_algo_destroy(gbf_core->pbf_algo);
		bf_image_destroy(gbf_core->pbfimage);
		bf_template_manager_destroy(gbf_core->pbfTemplateMgr);
		bf_tee_free(gbf_core);
		gbf_core = NULL;
	}
	#endif
	return 0;
}

int bf_core_chip_reinit()
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	
	esd_recovery_chip_params(bl_data);
	return 0;
}

int bf_core_fd_mode(int mode)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	switch(mode)
	{
		case FINGER_DETECT_DOWN:
			bl_interrupt_init(bl_data);
			break;
		case FINGER_DETECT_DOWN_PART:
			break;
		case FINGER_DETECT_UP:
			break;
	}
	return 0;
}

int bf_core_capture_image(bf_capture_data_t *capdata)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	int mode = capdata->mode;
	int isNeedAutoDacp = 0;
	u8 nMinValidArea = 0;
	uint32_t nMinPBscore = 0;	
	int ret = 0;
	int value = 0;

	BF_LOG("capdata->state=%d",capdata->state);
	if(mode == QUALIFY_MODE_AUTHENTICATE)
	{
		nMinValidArea = pbf_algo->nXuAreaMatch;
		nMinPBscore = pbf_algo->qscore_match;
	}
	else
	{
		nMinValidArea = pbf_algo->nXuAreaEnroll;
		nMinPBscore = pbf_algo->qscore_enroll;
	}
	
	bf_core_get_intStatus(&value);
	switch(capdata->state)
	{
		case STATE_DO_CAPTURE_START:
			if((value == BF_INTSTATE_FINGER_DOWN) && ((bl_data->chipid == BL_CHIPID_3590)||(bl_data->chipid == BL_CHIPID_81192)))
			{
				bl_capture_init_framenum(bl_data, 0);
				capdata->state = STATE_DO_CAPTURE_GOT_FIRST_FRAME;
				
			}else if(value == BF_INTSTATE_FRAME_DONE)
			{
				bf_core_read_frame(NULL);
				isNeedAutoDacp = 1;
				capdata->state = STATE_DO_CAPTURE_WAIT_FRAMEDONE;
			}else
			{
				capdata->result = BF_LIB_ERROR_SENSOR;
			}
			break;
		case STATE_DO_CAPTURE_GOT_FIRST_FRAME:
			if(value == BF_INTSTATE_FRAME_DONE)
			{
				bf_core_read_frame(NULL);
				isNeedAutoDacp = 1;
				capdata->state = STATE_DO_CAPTURE_WAIT_FRAMEDONE;
			}else
			{
				capdata->result = BF_LIB_ERROR_SENSOR;
			}
			break;
		case STATE_DO_CAPTURE_WAIT_FRAMEDONE:
			if(value == BF_INTSTATE_FRAME_DONE)
			{
				bf_core_read_frame(NULL);
				ret = bf_do_xu_imageEnhance_step1(pbfimage, pbf_algo, nMinValidArea);
				if(ret == 0)
				{
					ret = bf_do_xu_imageQuality(pbfimage);
					if(pbfimage->uPBcondition < 20 && pbfimage->uPBscore < 50)
					{
						ret = BF_LIB_FAIL_INVAILD_TOUCH;
						isNeedAutoDacp = 1;
					}else 
					{
						if(pbfimage->uPBscore < nMinPBscore)
						{
							ret = BF_LIB_FAIL_LOW_QUALITY;
							isNeedAutoDacp = 1;
						}
						else
						{
							ret = bf_do_xu_imageEnhance_step2(pbfimage, pbf_algo);
							capdata->state = STATE_DO_CAPTURE_GOT_BEST_IMAGE;
							isNeedAutoDacp = 0;
						}
					}

				}else{
					isNeedAutoDacp = 1;
				}
				capdata->result = ret;
				
			}else
			{
				capdata->result = BF_LIB_ERROR_SENSOR;
			}
			break;
	}
	if(isNeedAutoDacp)
	{
		REG_VALUE oldRegValue = {bl_data->gain,0x48,bl_data->dacp};
		REG_VALUE newRegValue;
		ret = bl_AutoGainDacp(pbfimage->fpdata, width , height, NULL, &oldRegValue, &newRegValue,128,pbf_algo->dacp_direction);
		if(ret == 0)
		{
			bl_data->dacp = newRegValue.reg_value_dacp;
			BF_LOG("ret=%d oldGainDacp.dacp=%x newGainDacp.dacp=%x",ret, oldRegValue.reg_value_dacp, newRegValue.reg_value_dacp);
			bl_capture_init(bl_data);
		}
		else
		{
			capdata->result = ret;
			BF_LOG("autoGainDacp failed ret=%d",ret);
			goto out;
		}
	}

out:
	return ret;
}
static int bf_core_get_best_image(int mode)
{
	int ret = 0;
	int value = 0;
	int trycount = 5000;
	int autodacpCount = 3;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	u8 nMinValidArea = 0;
	uint32_t nMinPBscore = 0;	

	if(mode == QUALIFY_MODE_AUTHENTICATE)
	{
		nMinValidArea = pbf_algo->nXuAreaMatch;
		nMinPBscore = pbf_algo->qscore_match;
	}
	else
	{
		nMinValidArea = pbf_algo->nXuAreaEnroll;
		nMinPBscore = pbf_algo->qscore_enroll;
	}

	bf_core_get_intStatus(&value);
	if((value == BF_INTSTATE_FINGER_DOWN) && ((bl_data->chipid == BL_CHIPID_3590)||(bl_data->chipid == BL_CHIPID_81192)))
	{
		bl_capture_init_framenum(bl_data, 0);
		while((bf_core_get_intStatus(&value) != BF_INTSTATE_FRAME_DONE)&&(trycount > 0))
		{
			trycount--;
		}
		if(trycount <= 0)
		{
			ret = BF_LIB_ERROR_SENSOR;
			goto out;
		}		
	}
	
	if(value == BF_INTSTATE_FRAME_DONE)	
	{
		bf_core_read_frame(NULL);
		//auto dacp
		while(--autodacpCount)
		{
			trycount = 5000;
			REG_VALUE oldRegValue = {bl_data->gain,0x48,bl_data->dacp};
			REG_VALUE newRegValue;
	
			ret = bl_AutoGainDacp(pbfimage->fpdata, width , height, NULL, &oldRegValue, &newRegValue,128,pbf_algo->dacp_direction);
			if(ret == 0)
			{
				bl_data->dacp = newRegValue.reg_value_dacp;
				BF_LOG("ret=%d oldGainDacp.dacp=%x newGainDacp.dacp=%x",ret, oldRegValue.reg_value_dacp, newRegValue.reg_value_dacp);
				bl_capture_init(bl_data);
			}
			else
			{
				BF_LOG("autoGainDacp failed ret=%d",ret);
				goto out;
			}
			while((bf_core_get_intStatus(&value) != BF_INTSTATE_FRAME_DONE)&&(trycount > 0))
			{
				trycount--;
			}
			if(trycount <= 0)
			{
				ret = BF_LIB_ERROR_SENSOR;
				goto out;
			}
			bf_core_read_frame(NULL);

			ret = bf_cal_mean_value(pbfimage, pbf_algo);
			ret = bf_do_xu_imageEnhance_step1(pbfimage, pbf_algo, nMinValidArea);
			if(ret == 0)
			{
				ret = bf_do_xu_imageQuality(pbfimage);
				if(pbfimage->uPBcondition < 20 && pbfimage->uPBscore < 50)
				{
					ret = BF_LIB_FAIL_INVAILD_TOUCH;
					break;
				}else 
				{
					if(pbfimage->uPBscore < nMinPBscore)
					{
						ret = BF_LIB_FAIL_LOW_QUALITY;
						break;
					}
				}
			}else
				goto out;
			
			ret = bf_do_xu_imageEnhance_step2(pbfimage, pbf_algo);
			if(ret == 0)
				break;
			else
				goto out;
		}
	}else
	{
		ret = BF_LIB_ERROR_SENSOR;
	}
	
out:
	BF_LOG("ret=%d",ret);
	return ret;
}

int bf_core_capture_image_all(bf_capture_data_t *capdata)
{
	int ret = 0;
	ret = bf_core_get_best_image(capdata->mode);
	capdata->result = ret;
	return ret;
}

int bf_core_qualify_image(bf_capture_data_t *capdata)
{
	int ret;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	u8 nMinValidArea = 0;
	uint32_t nMinPBscore = 0;
	capdata->result = 0;
	
	if(capdata->mode == QUALIFY_MODE_AUTHENTICATE)
	{
		nMinValidArea = pbf_algo->nXuAreaMatch;
		nMinPBscore = pbf_algo->qscore_match;
	}
	else
	{
		nMinValidArea = pbf_algo->nXuAreaEnroll;
		nMinPBscore = pbf_algo->qscore_enroll;
	}
	ret = bf_cal_mean_value(pbfimage, pbf_algo);
	ret = bf_do_xu_imageEnhance_step1(pbfimage, pbf_algo, nMinValidArea);
	if(ret == 0)
	{
		ret = bf_do_xu_imageQuality(pbfimage);
		if(ret != 0)
			capdata->result = ret;
		ret = bf_do_xu_imageEnhance_step2(pbfimage, pbf_algo);
		if(ret != 0)
			capdata->result = ret;
		capdata->uPosX = pbfimage->uPosX;
		capdata->uPosY = pbfimage->uPosY;
		capdata->uMeanValue = pbfimage->uMeanValue;
		capdata->uXuArea = pbfimage->uXuArea;
		capdata->uPBscore = pbfimage->uPBscore;
		capdata->uPBarea = pbfimage->uPBarea;
		capdata->uPBcondition = pbfimage->uPBcondition;
		if(pbfimage->uPBscore < nMinPBscore)
			capdata->result = BF_LIB_FAIL_LOW_QUALITY;
	}else{
		capdata->result = ret;
	}

	return ret;
}

int bf_core_get_intStatus(int *status)
{
	int value = 0;
	//BF_LOG("++++");
	value = bl_spi_read_reg(REGA_INTR_STATUS);
	*status = value;
	return value;
}

static void exchange_white_black(u8 *dst,u8 *src,int len)
{
	int i = 0;
	for( ; i < len; i++) {
		*(dst + i) = 0xff & (0xff - *(src + i));
	}	
}


static void exchange_odd_even_cols(u8 *src,int nWidth,int nHeight)
{
	u8 tempBuffer[16];
	u8 *pLine;
	int i = 0;
	int j = 0;
	u8 *pImgData = src; 

	for (i = 0; i < nHeight; i++)
	{
		pLine = pImgData + i*nWidth;
		for (j = 0; j < nWidth; j += 16)
		{
			tempBuffer[0] = pLine[j];
			tempBuffer[1] = pLine[j + 8];
			tempBuffer[2] = pLine[j + 1];
			tempBuffer[3] = pLine[j + 9];
			tempBuffer[4] = pLine[j + 2];
			tempBuffer[5] = pLine[j + 10];
			tempBuffer[6] = pLine[j + 3];
			tempBuffer[7] = pLine[j + 11];
			tempBuffer[8] = pLine[j + 4];
			tempBuffer[9] = pLine[j + 12];
			tempBuffer[10] = pLine[j + 5];
			tempBuffer[11] = pLine[j + 13];
			tempBuffer[12] = pLine[j + 6];
			tempBuffer[13] = pLine[j + 14];
			tempBuffer[14] = pLine[j + 7];
			tempBuffer[15] = pLine[j + 15];
			bf_tee_memcpy(pLine + j, tempBuffer, 16);
		}
	}
}

int bf_core_read_frame(char *image)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	u8 *imgdata = pbfimage->fpdata;
	BF_LOG("++++");
	bl_spi_read_frame(bl_data,bl_data->buf_size);
	if(bl_data->chipid == BL_CHIPID_3290)
	{
		exchange_white_black(imgdata, bl_data->tx_buf + FRAME_DUMMY_LEN, width * height);
		bf_tee_memcpy(imgdata+((height-1) * width),imgdata+((height-2) * width), width);
	}else if(bl_data->chipid == BL_CHIPID_3590)
	{
		bf_tee_memcpy(imgdata, bl_data->tx_buf + FRAME_DUMMY_LEN, width * height);
		exchange_odd_even_cols(imgdata, width, height);
	}
	else
		bf_tee_memcpy(imgdata, bl_data->tx_buf + FRAME_DUMMY_LEN, width * height);
	
	if(image != NULL)
		bf_tee_memcpy(image, imgdata, width * height);
		
	pbfimage->hasExtracted = 0;
	BF_LOG("----");
	return 0;
}


int bf_core_get_navigation_event(navigation_info *data)
{
	int ret = 0;
	int value = 0;
	int trycount = 5000;
	int status = 0;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	data->result = 0;
	
	if(data->eventValue == 0)//press down from fd mode
	{
		if((bl_data->chipid != BL_CHIPID_3590)&&(bl_data->chipid != BL_CHIPID_81192))
		{//fd read frame
			status = STATE_NORMAL_READ_FRAME_AND_DO_CAPTURE;

		}else if((bl_data->chipid == BL_CHIPID_3590)||(bl_data->chipid == BL_CHIPID_81192))
		{
			status = STATE_DO_CAPTURE_ONLY;
			//status = STATE_DO_CAPTURE_AND_WAIT_FRAMEDONE;
		}
	}else{
			status = STATE_NORMAL_READ_FRAME_AND_DO_CAPTURE;
	}
	//
	bf_core_get_intStatus(&value);
	switch(status)
	{
		case STATE_NORMAL_READ_FRAME_AND_DO_CAPTURE:

			if(value == BF_INTSTATE_FRAME_DONE)	
			{
				bf_core_read_frame(NULL);
				ret = bf_cal_mean_value(pbfimage, pbf_algo);
				data->height = pbfimage->height;
				data->width = pbfimage->width;
				data->uPosX = pbfimage->uPosX;
				data->uPosY = pbfimage->uPosY;
				data->uAreaCount = pbfimage->uAreaCount;
				data->uMeanValue = pbfimage->uMeanValue;
				bl_capture_init_framenum(bl_data, 0);
			}
			break;
		case STATE_DO_CAPTURE_ONLY:
			if(value == BF_INTSTATE_FINGER_DOWN)
			{
				bl_capture_init_framenum(bl_data, 0);
			}
			break;
		case STATE_DO_CAPTURE_AND_WAIT_FRAMEDONE:
			if(value == BF_INTSTATE_FINGER_DOWN)
			{
				bl_capture_init_framenum(bl_data, 0);
				while((bf_core_get_intStatus(&value) != BF_INTSTATE_FRAME_DONE)&&(trycount > 0))
				{
					trycount--;
				}
				if(trycount <= 0)
				{
					data->result = BF_LIB_ERROR_SENSOR;
					goto out;
				}
				bf_core_read_frame(NULL);
				ret = bf_cal_mean_value(pbfimage, pbf_algo);
				data->height = pbfimage->height;
				data->width = pbfimage->width;
				data->uPosX = pbfimage->uPosX;
				data->uPosY = pbfimage->uPosY;
				data->uAreaCount = pbfimage->uAreaCount;
				data->uMeanValue = pbfimage->uMeanValue;
				bl_capture_init_framenum(bl_data, 0);
			}
			break;
	}
	
out:
	return 0;
}

int bf_core_enroll(bf_enroll_data_t *enrolldata)
{
	int ret;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	
	u8 *imgdata = pbfimage->fpdata_enhance;
	
	//BF_LOG("pbfTemplateMgr=%x,tCurTemplate=%x,pTemplateData=%x",pbfTemplateMgr,pbl_template,pbl_template->pTemplateData);
	if(pbfimage->hasExtracted == 0)
	{
		ret = bl_Alg_ExtractTemplate(
			pbf_algo->nAlgoID,                 //[in] 指定算法ID
			imgdata,                //[in] 指纹图像数据
			pbf_algo->width,                  //[in] 图像宽
			pbf_algo->height,                 //[in] 图像高
			pbl_template     //[out] 提取到的模板数据，支持预先分配数据内存
		);	
		pbfimage->hasExtracted = 1;
	}
	//BF_LOG("pbl_template %x %x",pbl_template->templateType, pbl_template->templateSize);
	
	ret = bl_Alg_Enroll(
		pbf_algo->nAlgoID,               //[in] 指定算法ID
		imgdata,       					//[in] 指纹图像数据
		pbf_algo->width,                //[in] 图像宽，BF3290为112
		pbf_algo->height,               //[in] 图像高，BF3290为96
		pbl_template,     				//[in] 提前提取的指纹模板，传入这个模板可以加速录入，如果无输入0
		&enrolldata->progress,          //[out] 已成功录入模板数
		&enrolldata->coverage,           //[out] 指纹录入覆盖面积，范围[0,100]
		&enrolldata->nNbrOfIslands        //[out] 孤岛数量
	);
	BF_LOG("enrolldata %d %d %d",enrolldata->progress, enrolldata->coverage, enrolldata->nNbrOfIslands);
	if(enrolldata->required_samples == enrolldata->progress)
	{
		bl_Alg_FinalizeEnroll(
			pbf_algo->nAlgoID,                   //[in] 指定算法ID
			&pbfTemplateMgr->bl_templates[gbf_core->curEnrollIndic]        //[out] 返回已经生成的模板数据，支持预先分配数据内存
		);
	}

	return ret;
}

int bf_core_identifyT(bf_identify_data_t *data)
{
	int ret;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	BL_TEMPLATE *pbl_Alltemplates[5] = {0};
	
	u8 *imgdata = pbfimage->fpdata_enhance;
	
	//BF_LOG("pbfTemplateMgr=%x,tCurTemplate=%x,pTemplateData=%x,ret=%d",pbfTemplateMgr,pbl_template,pbl_template->pTemplateData,ret);
	if(pbfimage->hasExtracted == 0)
	{
		ret = bl_Alg_ExtractTemplate(
			pbf_algo->nAlgoID,                 //[in] 指定算法ID
			imgdata,                //[in] 指纹图像数据
			pbf_algo->width,                  //[in] 图像宽
			pbf_algo->height,                 //[in] 图像高
			pbl_template     //[out] 提取到的模板数据，支持预先分配数据内存
		);	
		pbfimage->hasExtracted = 1;
	}
	//BF_LOG("pbl_template %x %x ret=%d",pbl_template->templateType, pbl_template->templateSize, ret);
	
	//pbl_Alltemplates[0] = &pbfTemplateMgr->bl_templates[0];
	ret = bf_get_all_templates(pbfTemplateMgr, pbl_Alltemplates);
	ret = bl_Alg_VerifyT(
		pbf_algo->nAlgoID,                   //[in] 指定算法ID
		pbl_template,         				//[in] 待认证指纹模板
		pbl_Alltemplates,  					//[in] 已录入多模板数组 
		pbfTemplateMgr->indexcount,     	//[in] 已录入模板数量	
		pbf_algo->far_match,                //[in] FAR
		&data->index                        //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
	);
	data->matchID = pbfTemplateMgr->fids[pbfTemplateMgr->indices[data->index]];
	data->indic = pbfTemplateMgr->indices[data->index];
	BF_LOG("bl_Alg_VerifyT ret=%d,index=%d",ret,data->index);
return ret;
}

int bf_core_identify(bf_identify_data_t *data)
{
	int ret;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	BL_TEMPLATE *pbl_Alltemplates[5] = {0};
	
	u8 *imgdata = pbfimage->fpdata_enhance;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	
	ret = bf_get_all_templates(pbfTemplateMgr, pbl_Alltemplates);

	ret = bl_Alg_Verify(
		pbf_algo->nAlgoID,                   //[in] 指定算法ID
		imgdata,           //[in] 指纹图像数据
		width,                              //[in] 图像宽
		height,                             //[in] 图像高
		pbl_Alltemplates,  //[in] 已录入模板数组 
		pbfTemplateMgr->indexcount,     //[in] 已录入模板数量	
		pbf_algo->far_match,                       //[in] FAR
		&data->index ,                       //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
		pbl_template          //[out] 匹配成功后，返回匹配过程抽取的模板，可能不完整，需调用bl_Alg_UpdateMutilTemplatesEx来更新模板，如果不需要，传入空指针即可
	);
	data->matchID = pbfTemplateMgr->fids[pbfTemplateMgr->indices[data->index]];
	data->indic = pbfTemplateMgr->indices[data->index];
	BF_LOG("bl_Alg_Verify ret=%d,index=%d ",ret,data->index);
	return ret;
}

int bf_core_identify_all(bf_identify_data_t *data)
{
	int ret;
	int value = 0;

	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	
	ret = bf_core_get_best_image(QUALIFY_MODE_AUTHENTICATE);
	data->result = ret;
	if(ret != 0)
	{
		goto out;
	}

	bf_core_identify(data);
	BF_LOG("bl_Alg_VerifyT ret=%d,index=%d",ret,data->index);

out:
	return ret;
}

int bf_core_get_template_count(uint32_t* count)
{
	return bf_core_get_indices(NULL, count);
}

int bf_core_get_indices(uint32_t* indices, uint32_t* count)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_template_get_indices(pbfTemplateMgr, indices, count);
	return 0;
}

int bf_core_get_template_id_from_index(uint32_t index, uint32_t* data)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	*data = pbfTemplateMgr->fids[index];
	BF_LOG("index=%d,fid=%d",index, *data);
	return 0;
}

int bf_core_delete_template(uint32_t index)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	int ntemplatSize = pbf_algo->initParam.nMaxTemplateSize;
	BF_LOG("index=%d", index);
	pbfTemplateMgr->fids[index] = 0;
	bf_tee_memset(pbfTemplateMgr->bl_templates[index].pTemplateData, 0, ntemplatSize);
	pbfTemplateMgr->bl_templates[index].templateType = 0;       //模板类型
	pbfTemplateMgr->bl_templates[index].templateSize = 0;       //模板数据的大小
	return 0;
}

int bf_core_new_fid(uint32_t* id)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	int i = 0;
	
	for(i = 0;i < BF_MAX_FINGER;i++)
	{
		if(pbfTemplateMgr->fids[i] == 0)
		{
			pbfTemplateMgr->fids[i] = i + 1;//生成fid
			*id = pbfTemplateMgr->fids[i];
			gbf_core->curEnrollIndic = i;
			break;
		}
	}
	BF_LOG("fid=%d", *id);
	return *id;
}

int bf_core_delete_fid(uint32_t id)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	int i = 0;
	
	for(i = 0;i < BF_MAX_FINGER;i++)
	{
		if(pbfTemplateMgr->fids[i] == id)
		{
			pbfTemplateMgr->fids[i] = 0;//清除id
			bl_Alg_CancelEnroll(pbf_algo->nAlgoID);
			break;
		}
	}
	BF_LOG("fid=%d", id);
	return 0;
}

int bf_core_update_template_indic(uint32_t indic)
{
	int ret = 0;
	char result = 1;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	u8 *imgdata = pbfimage->fpdata_enhance;
	uint32_t tpsize = pbfTemplateMgr->bl_templates[indic].templateSize;
	uint32_t nArea = pbf_algo->nXuAreaUpdate;
	if(tpsize > (pbf_algo->initParam.nMaxTemplateSize * 19 / 20))
		nArea = pbf_algo->nXuAreaEnroll;

	FP_CheckBadPixelsNum(imgdata, pbf_algo->width, pbf_algo->height,pbf_algo->nBadPixelMaxValue, 
		pbf_algo->nBadPixelsNumThre, &result);
	
	BF_LOG("uXuArea=%d, nXuAreaUpdate=%d \
		badpixelcheck:%d",pbfimage->uXuArea, nArea, result);

	if(result != 0)
		return -1;		//模板图像存在裂纹或划痕过多
	if(pbfimage->uXuArea > nArea)
	{
		ret = bl_Alg_UpdateMutilTemplatesEx(
			pbf_algo->nAlgoID,                   //[in] 指定算法ID
			imgdata,           //[in] 指纹图像数据
			pbf_algo->width,                              //[in] 图像宽
			pbf_algo->height,                             //[in] 图像高
			pbl_template,   //[in] 由bl_Alg_Verify得到的模板,如果没有，传入空指针即可
			&pbfTemplateMgr->bl_templates[indic],   //[in] 待更新多模板
			&pbfTemplateMgr->bl_templates[indic]    //[out] 更新后多模板，支持预先分配数据内存
			);
			
		bf_core_store_template_db();
	}
	return ret;
}

static char *strncat_m(char *dest,const char *str,int n)
{
        char *cp=dest;

        while(*cp!='\0') ++cp;

        while(n&&(*cp++=*str++)!='\0')
        {
                --n;
        }

        return dest;
}

 
//字符串倒序
char* str_reverse(char* str)
{
    int n = strlen(str) / 2;
    int i = 0;
    char tmp = 0;

    for(i = 0; i < n; i++)
    {
        tmp  = str[i];
        str[i] = str[strlen(str)-i-1];
        str[strlen(str)-i-1] = tmp;
    }

    return str;
}

//bf_itoa 函数：将整形数转换为ｓ
void bf_itoa (int n,char s[])
{
	int i,j,sign;
	if((sign=n)<0)//记录符号
	n=-n;//使n成为正数
	i=0;
	do{
	       s[i++]=n%10+'0';//取下一个数字
	}
	while ((n/=10)>0);//删除该数字
	if(sign<0)
	s[i++]='-';
	s[i]='\0';
	s=str_reverse(s);
}


//strncat_path：将 （db文件路径+db文件名+整型gid+文件后缀名） 连接到同一个字符串
void strncat_path(char *db_path,char *db_name,unsigned int gid,char *file_extension)
{
  char bf_gid[10]={0};
  char *bf_db_path;
  int malloc_size=100;
  bf_db_path=malloc(malloc_size);
  bf_tee_memset(bf_db_path,0,malloc_size);
  #if defined (INCLUDE_ROUTER) //  use /data/system/users/0/fpdata/
  bf_tee_memcpy(bf_db_path,db_path,strlen(db_path));
  #endif
  bf_tee_memset(db_path,0,strlen(db_path));
  bf_itoa(gid,bf_gid);
  strncat_m(bf_db_path+strlen(bf_db_path),db_name,strlen(db_name));
  strncat_m(bf_db_path+strlen(bf_db_path),bf_gid,strlen(bf_gid));
  strncat_m(bf_db_path+strlen(bf_db_path),file_extension,strlen(file_extension));
  
  bf_tee_memcpy(db_path,bf_db_path,strlen(bf_db_path));
  free(bf_db_path);
}


int bf_core_load_user_db(const char* path, uint32_t path_len,uint32_t gid)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	char *db_path = pbfTemplateMgr->db_path;
	char *db_name = "bffpdb";
	char *db_file_extension=".dat";
	//add by jaston  for crc check
	#ifdef USE_CRC_CHECK
	char *dbbk_name = "bffpdbbk";
	char *bk_db_path = pbfTemplateMgr->bk_db_path;
	#endif
	//add end
	if(bf_tee_memcmp(path, gbf_core->store_path,path_len))
	{
		bf_tee_memcpy(gbf_core->store_path, path, path_len);
		bf_tee_memcpy(db_path , path, path_len);
		db_path[path_len - 1] = '/';
		db_path[path_len] = '\0';
		strncat_m(db_path + path_len,db_name,strlen(db_name));
		#ifdef BUILD_TEE
		BF_LOG("before::db_path =%s  db_name=%s  gid=%d \n",db_path,db_name,gid);
		//snprintf(db_path,1024,"%s%d%s",db_name,gid,".dat");
		strncat_path(db_path,db_name,gid,db_file_extension);
		#ifdef USE_CRC_CHECK
		//snprintf(bk_db_path,1024,"%s%d%s",dbbk_name,gid,".dat");
		strncat_path(bk_db_path,1024,"%s%d%s",dbbk_name,gid,".dat");
		#endif
		//add end
		#else
		//add by jaston  for crc check
		#ifdef USE_CRC_CHECK
		bf_tee_memcpy(bk_db_path , path, path_len);
		bk_db_path[path_len - 1] = '/';
		bk_db_path[path_len] = '\0';
		strncat_m(bk_db_path + path_len,dbbk_name,strlen(dbbk_name));
		#endif
		//add end
		#endif
		bf_template_manager_init_from_dbfile(pbfTemplateMgr);
	}
	BF_LOG("after::db_path =%s  db_name=%s  gid=%d \n",db_path,db_name,gid);
	BF_LOG("db_path=%s, gbf_core->store_path = %s  user_id=%llu", db_path,gbf_core->store_path,(long long unsigned int)gbf_core->pbfTemplateMgr->user_id);
	return 0;
}

int bf_core_set_active_fingerprint_set(int32_t fingerprint_set_key)
{
	gbf_core->fpset_key = fingerprint_set_key;
	BF_LOG("fpset_key=%d", gbf_core->fpset_key);
	return 0;
}

int bf_core_store_template_db()
{
	return bf_template_manager_store_to_db(gbf_core->pbfTemplateMgr);
}

int bf_core_do_mmit_test(mmi_test_params_t *data, uint32_t *buff_len)
{
	mmi_test_params_t *pmmi_params = data;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	uint32_t *pudacp = (uint32_t *)&pmmi_params->params[0];
	int ret = 0;
	int value = 0;
	
	switch(pmmi_params->action){
		case BF_MMI_ACT_CAPTURE_WITH_DACP:
				bl_data->dacp = *pudacp;
		case BF_MMI_ACT_CAPTURE_INIT:
				BF_LOG("dacp=%d", bl_data->dacp);
				bl_capture_init(bl_data);
				break;
		case BF_MMI_ACT_GET_RAW_IMAGE:
				bf_core_get_intStatus(&value);
				if(value == BF_INTSTATE_FRAME_DONE)
				{
					bf_core_read_frame(&pmmi_params->params[0]);

				}else{
					memcpy(&pmmi_params->params[0], pbfimage->fpdata, pmmi_params->length);
					ret = BF_LIB_ERROR_SENSOR;
				}
				pmmi_params->length = bl_data->frameheight * bl_data->framewidth;
				*buff_len = sizeof(mmi_test_params_t) + pmmi_params->length;
				break;
		case BF_MMI_ACT_SET_IDLE_MODE:
			bl_set_chip_idlemode(bl_data);
			break;
	}
	data->result = ret;
	return ret;
}


static uint64_t _htobe64(uint64_t q)
{
    union {
        uint64_t q;
        uint8_t b[8];
    } v, w;
    v.q = q;
    w.b[0] = v.b[7];
    w.b[1] = v.b[6];
    w.b[2] = v.b[5];
    w.b[3] = v.b[4];
    w.b[4] = v.b[3];
    w.b[5] = v.b[2];
    w.b[6] = v.b[1];
    w.b[7] = v.b[0];
    return w.q;
}

/*static uint32_t htonl(uint32_t h)
{
    union {
        uint32_t h;
        uint8_t b[4];
    } v, w;

    v.h = h;
    w.b[0] = v.b[3];
    w.b[1] = v.b[2];
    w.b[2] = v.b[1];
    w.b[3] = v.b[0];

    return w.h;
}*/


int generate_hmac(hw_auth_token_t *hat)
{	uint8_t hmac_key[32], hmac[32];
	uint32_t hmac_key_len = 0, hmac_len = 0;
	uint8_t *hash_buf=NULL;
	size_t	hash_len = 0;
	hash_len =	sizeof(*hat) - sizeof(hat->hmac);
	hash_buf = (uint8_t *)bf_tee_malloc(hash_len);
	if (NULL == hash_buf){
		BF_LOG("%s hash_buf malloc failed.\n",__func__);
		return -1;
	}
	memset(hash_buf,0x0,hash_len);
	memcpy(hash_buf,(uint8_t *)hat,hash_len);
	memset(hmac, 0, sizeof(hmac));
	hmac_len = sizeof(hmac);
	bf_tee_hmac_key(g_hmac_key, g_hmac_key_len);
	bf_tee_hmac_sha256(hash_buf, hash_len,g_hmac_key, g_hmac_key_len,&hmac);
	memcpy(hat->hmac,hmac,sizeof(hat->hmac));

	bf_tee_free(hash_buf);
	hash_buf=NULL;
	return 0;
}
void bf_core_send_enroll_token(hw_auth_token_t *enroll_token) 
{   
	hw_auth_token_t hat;    
	int result = 0 ;    
	gbf_core->pbfTemplateMgr->user_id=enroll_token->user_id;    
	gbf_core->pbfTemplateMgr->authenticator_id=enroll_token->authenticator_id;  
	memset(&hat, 0x0, sizeof(hw_auth_token_t)); 
	memcpy(&hat, enroll_token,sizeof(hw_auth_token_t)); 
	memset(&hat.hmac, 0x0,sizeof(hat.hmac));    /*gen hmac*/    
	BF_LOG("%s,generate_hmac.", __func__);  
	generate_hmac(&hat);    
	result = bf_tee_memcpy(&hat.hmac,enroll_token->hmac,sizeof(enroll_token->hmac));
	if (0 != result){	   
		BF_LOG("%s,hmac failed.", __func__);    
	}   
	BF_LOG("%s,gen hmac memcmp result=%d.", __func__,result);
}
void  bf_core_get_auth_token(hw_auth_token_t *auth_token)  
{	uint8_t *hmac_key = NULL;	
	uint32_t hmac_key_len;	uint32_t hash_len;  	
	uint64_t timestamp=0;	 	
	if (auth_token == NULL) { 		
		goto out; 	
	}		
	hmac_key = (uint8_t *)bf_tee_malloc(sizeof(g_hmac_key));	
	if (hmac_key == NULL) {		
		BF_LOG("%s +%d [%s]malloc for hmac_key failed!\n",__FILE__,__LINE__,__func__);
		return;	
	}	
	bf_tee_memset(hmac_key,0,sizeof(g_hmac_key));	
	bf_tee_memcpy(hmac_key,g_hmac_key,sizeof(g_hmac_key));	
	hmac_key_len = g_hmac_key_len;		
	timestamp = bf_tee_get_time();	
	auth_token->timestamp = _htobe64(timestamp);	
	auth_token->user_id=gbf_core->pbfTemplateMgr->user_id;	
	auth_token->timestamp = timestamp;	
	BF_LOG("fp_hmac_sha256 timestamp=0x%llx[0x%llx].\n",timestamp,auth_token->timestamp);
	//auth_token->user_id=gbf_core->pbfTemplateMgr->user_id;
	hash_len = (uint32_t)((uint8_t *)&(auth_token->hmac) - (uint8_t *)auth_token);	
	bf_tee_hmac_sha256((uint8_t*)auth_token,hash_len,g_hmac_key, g_hmac_key_len,auth_token->hmac);	
	memset(auth_token->hmac, 0x0, sizeof(auth_token->hmac));	
	generate_hmac(auth_token);	
	BF_LOG("+%d [%s]  fp_hmac_sha256 done.\n",__LINE__,__func__);
out:	
	if (hmac_key != NULL) {		
		bf_tee_free(hmac_key);	
	} 	
	return; 
}

int bf_core_set_fpdata(bf_ca_app_data_t *fpdata)
{
	fpdata->width = gbf_core->pbf_algo->width;
	fpdata->height = gbf_core->pbf_algo->height;
	fpdata->capdacp = gbf_core->bl_data->chip_params->capdacp_reg.value;
	/*BF_LOG("sjx set fpdata:\
		width:%d, height:%d, capdacp:%d\n", fpdata->width,
											fpdata->height,
											fpdata->capdacp);*/
	return 0;
}

int bf_core_handler(uint32_t *buffer, uint32_t *buff_len)
{
	bf_ca_cmd_data_t *cmddata = (bf_ca_cmd_data_t *)buffer;
	BF_LOG("handler_cmd %d", cmddata->cmd);
	switch(cmddata->cmd)
	{
		case BF_CMD_CORE_INIT:
			if(*buff_len > sizeof(cmddata->cmd))
				cmddata->cmd = bf_core_init(&cmddata->ca_config);
			else
				cmddata->cmd = bf_core_init(NULL);
			break;
		case BF_CMD_CORE_UNINIT:
			cmddata->cmd = bf_core_uninit();
			break;
		case BF_CMD_CHIP_REINIT:
			cmddata->cmd = bf_core_chip_reinit();
			break;
		case BF_CMD_FD_MODE:
			cmddata->cmd = bf_core_fd_mode(cmddata->cmdparam.param);
			break;
		case BF_CMD_GET_INT_STATUS:
			bf_core_get_intStatus((int *)&cmddata->cmd);
			break;
		case BF_CMD_CAPTURE_IMAGE:
			bf_core_capture_image(cmddata);
			break;
		case BF_CMD_QUALITY_IMAGE:
			bf_core_qualify_image(cmddata);
			break;
		case BF_CMD_READ_FRAME:
			bf_core_read_frame(NULL);
			break;
		case BF_CMD_GET_NAVIGATION_EVENT:
			bf_core_get_navigation_event(cmddata);
			break;
		case BF_CMD_ENROLL:
			bf_core_enroll(cmddata);
			break;
		case BF_CMD_END_ENROLL:
			break;
		case BF_CMD_IDENTIFY:
			bf_core_identify(cmddata);
			break;
		case BF_CMD_IDENTIFY_ALL:
			bf_core_identify_all(cmddata);
			break;
		case BF_CMD_CAPTURE_IMAGE_ALL:
			bf_core_capture_image_all(cmddata);
			break;
		case BF_CMD_NEW_FINGERID:
			bf_core_new_fid(&cmddata->cmd);
			break;
		case BF_CMD_DELETE_FINGERID:
			bf_core_delete_fid(cmddata->cmdparam.param);
			break;
		case BF_CMD_UPDATE_TEMPLATE_BY_INDIC:
			bf_core_update_template_indic(cmddata->cmdparam.param);
			break;
		case BF_CMD_CHECK_FINGER_LOST:
			break;
		case BF_CMD_GET_HW_AUTH_CHALLENGE:
			break;
		case BF_CMD_GET_TEMPLATE_DB_ID:
			break;
		case BF_CMD_GET_TEMPLATE_COUNT:
			bf_core_get_template_count(&cmddata->cmd);
			break;
		case BF_CMD_GET_TEMPLATE_INDICS:
			bf_core_get_indices(cmddata->caIndics.indics, &cmddata->caIndics.cmd);
			break;
		case BF_CMD_GET_TEMPLATE_ID_FROM_INDEX:
			bf_core_get_template_id_from_index(cmddata->cmdparam.param, &cmddata->cmdparam.cmd);
			break;
		case BF_CMD_DELETE_TEMPLATE_BY_INDIC:
			bf_core_delete_template(cmddata->cmdparam.param);
			break;
		case BF_CMD_STORE_TEMPLATE_DB:
			bf_core_store_template_db();
			break;
		case BF_CMD_LOAD_TEMPLATE_DB:
			BF_LOG("cmddata->caPath.gid =%d ", cmddata->caPath.gid);
			bf_core_load_user_db(cmddata->caPath.path, cmddata->caPath.path_len,cmddata->caPath.gid);
			break;
		case BF_CMD_SET_FINGERPRINT_SET:
			break;
		case BF_CMD_CHECK_TEMPLATE_VERSION:
			break;
		case BF_CMD_DO_MMI_TEST:
			bf_core_do_mmit_test(cmddata, buff_len);
			break;
		case BF_CMD_GET_TIME:
			cmddata->systime.kernelsystime= bf_tee_get_time();
			cmddata->cmd=0;
			break;
		case BF_CMD_GET_AUTH_TOKEN:
			bf_core_get_auth_token((hw_auth_token_t  *)&(cmddata->ca_authtoken_data.hat));
			break;
		case BF_CMD_SEND_ENROLL_TOKEN:
			bf_core_send_enroll_token((hw_auth_token_t  *)&(cmddata->ca_authtoken_data.hat));
			break;
		case BF_CMD_GET_FINGER_DATA:
			bf_core_set_fpdata(&cmddata->app_data);
			break;
		default:
			break;
	}
	return cmddata->cmd;
}

