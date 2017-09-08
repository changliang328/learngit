/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 */


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <time.h>
#include <unistd.h>
#include <math.h>
#include <android/log.h>

#include "btlfp.h"
#include "btl_algorithm_interface.h"
#include "btlbmp.h"
#include "btlcustom.h"
#include "auto_gain_dacp.h"
#include "finger_print.h"
#define FINGER_DEBUG            1

int TEMPLATE_SIZE = 512*1024;
#define ONE_TEMPLATE_SIZE                  512*1024
extern int Btl_SetTemplateMaxSize(int);
char mEnrolledIndex[MAX_FINGER_NUMS];
extern char*  dataFolderPath;
//capture image begin
extern int Create_256bmp(unsigned char* pData, int width, int height,unsigned char* pBMP, const char* pFileName);
extern int Create_infoFile(int context,int fid, int fid_subindex, int qual, int condition,int result, const char* pPath);
extern int Create_binFile(
    unsigned char* pData,
    const char* pFileName,
    int width,
    int height,
    int fid,
    int fid_subindex,
    int qual,
    int condition,
    int area,
    int contrast,
    int result,
    int context,
    int hour,
    int min,
    int sec,
    int matchflag
);
#if (SAVE_IMAGE)
int g_last_schedule;
#endif
//capture image end
/*----------------------------------------------------------b--------------*/


#define BL229X                         0
#define BL239X                         1




typedef struct {
    int32_t  quality;
    int32_t  area;
    int32_t  intensity;
    int32_t  contrast;
    int32_t  isDry;
    int32_t  imageVar;
    int32_t  imageValid;
    int32_t  condition;
    uint8_t  *p_image;
	int32_t  org_area;
	unsigned char  area_thre_shold;
} fp_image_t;

/*------------------------------------------------------------------------*/


extern const char* deviceNode;
/*------------------------------------------------------------------------*/
uint8_t   g_match_failed_times     = DEFAULT_MATCH_FAILED_TIMES;
uint8_t   g_shotkey_disable        = DEFAULT_SHOTKEY_DISABLE;
uint8_t   g_enroll_max_counts      = DEFAULT_ENROLL_COUNTS;

uint16_t  g_match_ck_fingerup_timeout  = 150;
uint16_t  g_enroll_ck_fingerup_timeout = 150;
uint8_t   g_algorithm_type = default_algorithm_type;
uint8_t   g_def_enhance_image = 0;
uint8_t   g_def_gain = 0x2f;

int8_t   g_CancelFlag = 0;
int32_t  g_fd = -1;
int32_t  g_schedule = 0;
uint16_t g_best_capture_contrast  = DEFAULT_CONTRAST_FOR_CAPTURE;

uint8_t  g_def_capture_contrast   = DEFAULT_CONTRAST_FOR_CAPTURE;
uint8_t  g_best_fingerup_contrast = DEFAULT_CONTRAST_FOR_FINGERUP;
uint8_t  g_enroll_trytimes        = DEFAULT_ENROLL_TRYTIMES;
uint8_t  g_match_trytimes         = DEFAULT_MATCH_TRYTIMES;
uint8_t  g_intensity_threshold    = DEFAULT_INTENSITY_THRESHOLD;
uint8_t  g_contrast_high_value    = DEFAULT_CONTRAST_HIGH;
uint8_t  g_contrast_low_value     = DEFAULT_CONTRAST_LOW;
uint8_t  g_match_quality_score_threshold   = DEFAULT_MATCH_SCORE_THRESHOLD;
uint8_t  g_enroll_quality_score_threshold  = DEFAULT_ENROLL_SCORE_THRESHOLD;
uint8_t  g_match_quality_area_threshold    = DEFAULT_MATCH_AREA_THRESHOLD;
uint8_t  g_enroll_quality_area_threshold   = DEFAULT_ENROLL_AREA_THRESHOLD;
uint8_t  g_far_value  = DEFAULT_FAR;
uint8_t  g_update_far_value  = DEFAULT_FAR;
uint8_t  g_debug_enable = 1;
uint8_t  g_contrast_direction     = 0;
uint8_t  g_contrast_finetune_step = 4;
uint8_t  g_capture_ck_empty_frames = BADIMAGE_TIMEOUT_COUNTS;
uint8_t  g_inited = 0;
uint8_t  g_online_update = 0;

uint16_t g_points_threshold = 2000;
uint16_t g_chip_id = 0x5183;
uint8_t g_width = 72;
uint8_t g_height = 128;
uint8_t g_area_max = 23;
uint8_t g_best_algo = 24;
int g_all_area = 0;


void btl_core_load_data(const char *pDbName,
                        int32_t   uid,
                        int32_t   fid,
                        uint8_t*  pFpData,
                        uint8_t*  pType,
                        int32_t*  pDataSize,
                        int32_t*  pStatus)
{
    *pStatus = 0;

    //LOGD("uid:%d,fid:%d",uid,fid);
    if (sql_load_fingerprint(pDbName,uid,fid,pFpData, pDataSize) == 0) {
        pFpData = &(pFpData[1]);
        *pType = pFpData[0];
        (*pDataSize)--;
        *pStatus = 1;
    }
}


/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
static int32_t btl_core_readFpImage (
    uint8_t *pimage,
    int32_t timeout __attribute__((unused)),
    int32_t device_fd,
    uint32_t contrast)
{
    int32_t rx_ret=0;

    if (g_CancelFlag == 1) {
        return -ERR_CANCEL;
    }

    LOGD("%s,cur agc: %d",__func__,contrast);

    rx_ret = ioctl(device_fd,BL229X_Adjustment_AGC,contrast);
    if (rx_ret == -1) {
        LOGE("%s, ioctl agc ret = %d ",__func__,rx_ret);
        return -ERR_IO;
    }

    rx_ret = ioctl(device_fd,BL229X_GETIMAGE,contrast);
    if (g_CancelFlag == 1) {
        return -ERR_CANCEL;
    }
    if (rx_ret != 0) {
        LOGE("%s, ioctl getimage ret = %d ",__func__,rx_ret);
        return -ERR_IO;
    }

    rx_ret = read(device_fd,pimage,(g_width * g_height));
    if (g_CancelFlag == 1) {
        return -ERR_IO;
    }
    if (rx_ret != 0) {
        LOGE("%s, read ret = %d ",__func__,rx_ret);
        return -ERR_IO;
    }

    return 0;

}

static int32_t btl_core_adjustContrast(int32_t curContrast,
                                       int32_t nDirect,
                                       uint8_t step,
                                       int32_t multi_value)
{
    int32_t finalContrast;

    if (nDirect)
        finalContrast = curContrast + step*multi_value;
    else
        finalContrast = curContrast - step*multi_value;

    return (int32_t)finalContrast;
}

/*----------------------------------------------------------------------
purpose:
return :
001 2
010 4
011 5.6
100 7
101 8
110 11.3
111 14

------------------------------------------------------------------------*/
float nGain1 = 14;
float nGain2 = 8;//11.3;
int nGain3 = 1;
int nGain4 = 1;
float factorB = 0.145;
static int32_t btl_find_best_contrast(int32_t curContrast,
                                       int32_t nDirect,
                                       int32_t ncurMeanValue,
                                       int32_t ndstMeanValue)
{
    int32_t finalContrast;
    int32_t deltaContrast = 0;
    float tempvalue = 0;
    tempvalue = nGain2*nGain3*nGain4*factorB;
	LOGD("curContrast=%d,nDirect=%d,ncurMeanValue=%d,ndstMeanValue=%d，tempvalue=%f",curContrast,nDirect,ncurMeanValue,ndstMeanValue,tempvalue);
	deltaContrast = (int32_t )((ndstMeanValue - ncurMeanValue) / tempvalue);
	LOGD("deltaContrast=%d",deltaContrast);
    if (nDirect)
        finalContrast = curContrast - deltaContrast;
    else
        finalContrast = curContrast + deltaContrast;
	LOGD("finalContrast=%d",finalContrast);
	if((finalContrast >= 250)||(finalContrast <= 0))
		finalContrast = g_def_capture_contrast;
    return (int32_t)finalContrast;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
static int32_t btl_core_captureImageWithAgc(
    fp_image_t* pFpImage,
    uint8_t tryTimes,
    int32_t nAreaThreShold,
    int32_t nScoreThreShold,
    uint8_t white_value_empty_image  __attribute__((unused)),
    int32_t points_threshold_empty_image  __attribute__((unused)),
    int32_t nDefaultContrast,
    int32_t *pAdjustFrames,

    fp_image_t* pFpImageOrg)
{
    int32_t  nIsDry = 0,nIntensity = 0,nImgVar = 0,nImgVar_MAX = 0;
    //uint8_t  imageA[g_width * g_height], imageB[g_width * g_height];
    int32_t  rimageStatus = 0;
    uint8_t  validFpCounts = 0;
    uint8_t  ioErrorCounts = 0;
    int32_t  nQua = 0;
    int32_t  nValidArea = 0;
    int8_t   pressed = 0;
    uint8_t  matchCompleted = 0;
    int32_t  nCheckImageQuality = -1;
    uint8_t  goodImage = 0;
    int32_t  curContrast;
    uint8_t  isValidFpImage = 0;
    int32_t  nCondition = 0;
	int32_t ret = 0;
	int32_t nCurMeanValue = 0;
	uint8_t*  imageA = pFpImage->p_image;
    LOGD("++%s sky g_contrast_direction=%d, %d",__func__,g_contrast_direction, pFpImage->area_thre_shold);

    if (g_CancelFlag)
        return -ERR_CANCEL;

    curContrast = nDefaultContrast;
    memset(imageA,0xff,g_width * g_height);

    while (matchCompleted == 0) {
        nValidArea = 0;
        btl_util_time_update(2);
        rimageStatus = btl_core_readFpImage (imageA,10,g_fd,curContrast);
        btl_util_time_diffnow("capture",2);
        if(rimageStatus == -ERR_CANCEL) 
           return -ERR_CANCEL;		
        /* -- bad image process start --*/
        if (rimageStatus == -ERR_IO) { //access driver error
            ioErrorCounts++;
            if (ioErrorCounts >= IOACCESS_TIMEOUT_COUNTS)
                return -ERR_EMPTY;
            continue;
        }

        nCurMeanValue = btl_cal_finerMean(imageA, g_width , g_height);
        if(nCurMeanValue <= 225)
			isValidFpImage = 1;

        pressed = 1;

		#if (SAVE_IMAGE)//(SAVE_IMAGE)
		memset(pFpImageOrg->p_image,0xff,g_width * g_height);
		memcpy(pFpImageOrg->p_image,imageA,g_width * g_height);
		Btl_CheckImageQuality(pFpImageOrg->p_image, g_width, g_height, 1, 1, &(pFpImageOrg->quality), &(pFpImageOrg->area),&(pFpImageOrg->condition),0);
		pFpImageOrg->contrast = curContrast;
		#endif
        //increment image counts
        validFpCounts++;
        
        /* -- process images quality from 31 to 79 --*/
        if(isValidFpImage) {
            nCheckImageQuality = -1;
            btl_util_time_update(3);
			ret = FP_ImagePreproc(imageA, g_width, g_height, 0, &(pFpImage->area_thre_shold), 0x0f);
			if(ret == 0)
			{
            	LOGD ("FP_ImagePreproc real_area: %d\n",pFpImage->area_thre_shold);
		        nCheckImageQuality = Btl_CheckImageQuality(imageA, g_width, g_height , nAreaThreShold, nScoreThreShold, &nQua, &nValidArea,&nCondition,0);
            	LOGD ("CheckImageQuality %d,qual :%d ,area: %d, condition:%d,curContrast=%d\n",nCheckImageQuality,nQua,nValidArea,nCondition,curContrast);
				#if (SAVE_IMAGE)
				{
					int min,sec,hour;
					{
						time_t timep;
						struct tm *p;
						time(&timep);
						p = gmtime(&timep);
						hour= p->tm_hour;
						min = p->tm_min;
						sec = p->tm_sec;
					}
					char imagename[256];
					memset(imagename,0,256);
					sprintf(imagename,"/data/system/users/0/fpdata/cap(%d-%d-%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_count(%d)_or.bmp",hour,min,sec,pFpImageOrg->contrast,pFpImageOrg->quality,pFpImageOrg->area,pFpImageOrg->condition,validFpCounts);
					setpixcol(pFpImageOrg->p_image,imagename);	
					memset(imagename,0,256);
				    sprintf(imagename,"/data/system/users/0/fpdata/cap(%d-%d-%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_count(%d)_eh.bmp",hour,min,sec,curContrast,nQua,nValidArea,nCondition,validFpCounts);
					setpixcol(imageA,imagename);
				}
				#endif
			}
			else{
				LOGD ("FP_ImagePreproc failed goodImage=%d ret=%d,real_area: %d",goodImage,ret,pFpImage->area_thre_shold);
				#if (SAVE_IMAGE)
				{
					int min,sec,hour;
					{
						time_t timep;
						struct tm *p;
						time(&timep);
						p = gmtime(&timep);
						hour= p->tm_hour;
						min = p->tm_min;
						sec = p->tm_sec;
					}
					char imagename[256];
					memset(imagename,0,256);
					sprintf(imagename,"/data/system/users/0/fpdata/cap(%d-%d-%d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_count(%d)_or.bmp",hour,min,sec,pFpImageOrg->contrast,pFpImageOrg->quality,pFpImageOrg->area,pFpImageOrg->condition,validFpCounts);
					setpixcol(pFpImageOrg->p_image,imagename);	
				}
				#endif
				goodImage = 0;		
			}
            btl_util_time_diffnow("Quality",3);

            if (nCheckImageQuality == 0) {
                if((nCurMeanValue >= 50)&&(nCurMeanValue <= 200))
		   	   {
				   goodImage = 1;
				   break;
		       }
            }

			REG_VALUE oldRegValue = {g_def_gain,0x48,curContrast};
			REG_VALUE newRegValue;
			ret = bl_AutoGainDacp(imageA, g_width , g_height,  &oldRegValue, &newRegValue,128,!g_contrast_direction);
			if(ret == 0)
			{
			curContrast = newRegValue.reg_value_dacp;
			LOGD ("ret=%d  oldGainDacp.dacp=%x newGainDacp.dacp=%x",ret,oldRegValue.reg_value_dacp, newRegValue.reg_value_dacp);
			}
			else
			{
				LOGD("autoGainDacp failed ret=%d",ret);
			}
        }
        if (validFpCounts >= tryTimes) {
        	break;
        }
    }

    if (pAdjustFrames)
        *pAdjustFrames = validFpCounts;
    LOGD ("g_def_enhance_image=%d",g_def_enhance_image);

    pFpImage->imageValid = 1;
    pFpImage->contrast   = curContrast;
    pFpImage->area       = nValidArea;
    pFpImage->quality    = nQua;
    pFpImage->isDry      = nIsDry;
    pFpImage->intensity  = nIntensity;
    pFpImage->imageVar   = nImgVar;
    pFpImage->condition  = nCondition;
	pFpImage->org_area = pFpImage->area_thre_shold;
    LOGD("--%s",__func__);
    if (goodImage) {
        g_best_capture_contrast = curContrast;
        LOGD ("validFpCount:%d, nIsDry:%d,best contrast:%d",validFpCounts,nIsDry,g_best_capture_contrast);
        return ERR_OK;
    } else
        return -ERR_AGC;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_test_captureImageWithAgc(
    uint8_t *pOutImage,
    uint8_t *pOutImageParams,
    int32_t nImageBufSize,
    int32_t fixed_contrast,
    int32_t tryTimes,
    int32_t nAreaThreShold,
    int32_t nScoreThreShold,
    int32_t contrast_step,
    int32_t best_intensity_threshold,
    int32_t *p_adjustFrames,
    int32_t *p_func_result
)
{
    int32_t def_contrast = 0;
    int tempFd, i;

    uint8_t* p_image = 0;
    uint8_t* p_debug_image = 0;
    uint8_t* p_org_image = 0;

    fp_image_t* p_image_params[10];
    fp_image_t  fpImage;

    int32_t  nCondition = 0;
    int32_t  nAdjustFrames = 0;

    LOGD("++%s",__func__);

    if (!g_inited)
        btl_core_getDefaultParams();

    LOGD("%s,g_contrast_direction:%d",__func__,g_contrast_direction);

    LOGD("%d,%d,%d,%d,%d,%d,%d",
         nImageBufSize,fixed_contrast,tryTimes,nAreaThreShold,nScoreThreShold,contrast_step,
         best_intensity_threshold);

    if (pOutImage == 0) {
        *p_func_result = -ERR_PARAMS;
        LOGE("%s,pOutImage == 0",__func__);
        return -1;
    }

	if (nImageBufSize < (tryTimes+1)*g_width * g_height){
		*p_func_result = -ERR_PARAMS;
	    LOGE("%s,nImageBufSize < (tryTimes+1)",__func__);
		return -2;
	}
	 if (!g_inited)
       		btl_core_getDefaultParams();

    for (i = 0; i < 5; i++){		
		if (g_CancelFlag == 1){	
		     *p_func_result = -ERR_CANCEL;
			 return -1;
		}
	    tempFd = open(deviceNode,O_RDWR);
        if (tempFd == -1){
            LOGE("%s,can't open device:%d ",__func__,tempFd);
        } else
            break;
    }

    if (i >= 5) {
        *p_func_result = -ERR_IO;
        return -1;
    }

    g_fd = tempFd;

    p_org_image = malloc(g_width * g_height);

    memset(p_org_image,0x0,g_width * g_height);

    if (!p_org_image) {
        LOGE("%s,p_org_image is 0",__func__);
        *p_func_result = -ERR_MEMORY;
        return -1;
    }

    def_contrast = fixed_contrast;

    fpImage.p_image =  p_org_image;

    p_image = pOutImage;

    p_debug_image = p_image + g_width * g_height;

    for (i = 0; i < 10; i++)
        p_image_params[i] = 0;

    for (i = 0; i < tryTimes; i++) {
        if (p_image_params[i])
            p_image_params[i]->p_image = p_debug_image + i*g_width * g_height;
    }

    LOGD("%s,trytimes:%d",__func__,tryTimes);
	fpImage.area_thre_shold = 50;
    *p_func_result = btl_core_captureImageWithAgc(
                         &fpImage,
                         tryTimes,
                         nAreaThreShold,
                         nScoreThreShold,
                         WHITE_POINT_VALUE,
                         g_points_threshold,
                         def_contrast,
                         &nAdjustFrames,
                         0);//p_image_params);
    close(g_fd);
    g_fd = -1;

    //calculate every image's params
    LOGD("adjust frames:%d",nAdjustFrames);
    for (i = 0; i < nAdjustFrames; i++) {
        int32_t nQua,nValidArea;
        if (p_image_params[i]) {
            Btl_CheckImageQuality(p_image_params[i]->p_image, g_width , g_height, nAreaThreShold, nScoreThreShold, &nQua, &nValidArea,&nCondition,0);
            p_image_params[i]->area = nValidArea;
            p_image_params[i]->quality = nQua;
            p_image_params[i]->condition = nCondition;
        }
    }
    memcpy(p_image,p_org_image,g_width * g_height);
    pOutImageParams[0] = fpImage.imageValid;
    pOutImageParams[1] = fpImage.quality;
    pOutImageParams[2] = fpImage.area;
    pOutImageParams[3] = fpImage.intensity;
    pOutImageParams[4] = fpImage.isDry;
    pOutImageParams[5] = fpImage.contrast;
    pOutImageParams[6] = fpImage.imageVar;
    pOutImageParams[7] = fpImage.condition;

    for (i = 0; i < 8; i++)
        LOGD("pOutImageParams:%d",pOutImageParams[i]);
    /*
    for (i = 0; i < nAdjustFrames; i++) {
        if (p_image_params[i]) {
            pOutImageParams[(i+1)*8+0] = p_image_params[i]->imageValid;
            pOutImageParams[(i+1)*8+1] = p_image_params[i]->quality;
            pOutImageParams[(i+1)*8+2] = p_image_params[i]->area;
            pOutImageParams[(i+1)*8+3] = p_image_params[i]->intensity;
            pOutImageParams[(i+1)*8+4] = p_image_params[i]->isDry;
            pOutImageParams[(i+1)*8+5] = p_image_params[i]->contrast;
            pOutImageParams[(i+1)*8+6] = p_image_params[i]->imageVar;
            pOutImageParams[(i+1)*8+7] = p_image_params[i]->condition;
        }
    }
    */

    for(i = 0; i < tryTimes; i++) {
        if (p_image_params[i]) {
            free(p_image_params[i]);
        }
    }

    if (p_org_image)
        free(p_org_image);

    *p_adjustFrames = nAdjustFrames;

    return 0;
}

int32_t btl_core_enhance_image(uint8_t *pOrgImg, uint8_t *pEnhanceImg,int enhance_type)
{
    if (Btl_EnhanceImage(pOrgImg, pEnhanceImg, g_width , g_height, enhance_type)) {
        return -1;
    }
    return 0;
}

/*----------------------------------------------------------------------
	purpose: Let sensor enter power down mode
	return :   sucess: 0; fail: -1
------------------------------------------------------------------------*/
int32_t btl_core_powerDown()
{
    int32_t tempFd;

    LOGD("++%s",__func__);
    tempFd = open(deviceNode,O_RDWR);
    if (tempFd == -1) {
        LOGE ("%s,can't open device!\n",__func__);
        return -1;
    }
    g_fd = tempFd;
    ioctl(g_fd,BL229X_POWERDOWN_MODE);
    close(g_fd);
    g_fd = -1;
    return 0;
}


/*----------------------------------------------------------------------
purpose: Cancel enrolling or matching .
return :   sucess: 0; fail: -1
------------------------------------------------------------------------*/

int32_t btl_core_cancelSensor(int8_t nCancel)
{
    g_CancelFlag = nCancel;
    LOGD("%s,g_CancelFlag:%d",__func__,g_CancelFlag);
    return g_CancelFlag;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/

int32_t btl_core_isFingerUp ()
{
    uint8_t image[g_width * g_height];
    uint8_t fingerUpFlag = 0;
    int32_t imageCheckRet = 0;
    uint8_t checkFingerUpMaxCounts = 3;
    uint8_t checkFingerDnMaxCounts = 3;
    uint8_t emptyImageCounts = 0;
    uint8_t validImageCounts = 0;
    uint8_t timeoutCouns = 0;
    uint8_t maxTimeOuts = 50;
    int32_t tempFd;
    uint32_t curContrast;


    tempFd = open(deviceNode,O_RDWR);

    if (tempFd == -1) {
        LOGE ("%s,can't open device!\n",__func__);
        return -2;
    }

    g_fd = tempFd;

    curContrast =  g_best_fingerup_contrast;

    while (g_CancelFlag != 1 && timeoutCouns < maxTimeOuts) {
        btl_core_readFpImage(image,100,g_fd,curContrast); //??ȡͼ??
        imageCheckRet = btl_preprocess_imageCheckEmpty(image,WHITE_POINT_VALUE,g_points_threshold,g_width * g_height);
        timeoutCouns++;
        if (imageCheckRet == -1) {
            validImageCounts = 0; //??ָ?ڴ???????????
            emptyImageCounts ++;
            if (emptyImageCounts >= checkFingerUpMaxCounts) {   //????5֡?հ?ͼ????˵????ָ?뿪
                fingerUpFlag = 1;
                break;
            }
        } else {
            emptyImageCounts = 0;
            validImageCounts++;
            if (validImageCounts >= checkFingerDnMaxCounts) {
                fingerUpFlag = 0;
                break;
            }
        }
    }
    close(g_fd);
    g_fd = -1;
    LOGD("fingerup flag:%d,%d %d",fingerUpFlag,g_points_threshold, g_CancelFlag);
    if (g_CancelFlag == 1) return -ERR_CANCEL;
    if (fingerUpFlag) return 0;
    if (timeoutCouns >= maxTimeOuts) return -2;
    return -1;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_registeFp(int *pRadio)
{
    int32_t updata = 0;
    int32_t nOL = 0;
    int32_t ret=0;
    int32_t reg_res = 0;
    uint8_t p_image[g_width * g_height];
    int32_t tempFd = -1;
    int32_t nConverage = 0;
    uint8_t i;
    const uint8_t nOpenTimes = 20;
    fp_image_t  fpImage;

	#if (SAVE_IMAGE)
	uint8_t p_image_org[g_width * g_height];
	fp_image_t  fpImage_org;
	fpImage_org.p_image = p_image_org;
	#endif

    LOGD("++%s",__func__);
    fpImage.p_image = p_image;

    for (i = 0; i < nOpenTimes; i++) {
        if (g_CancelFlag == 1)
            return -ERR_CANCEL;
        tempFd = open(deviceNode,O_RDWR);
        if (tempFd == -1) {
            LOGE("%s,can't open device:%d ",__func__,tempFd);
        } else
            break;
    }

    if ( i >= nOpenTimes) return -ERR_IO;

    g_fd = tempFd;
	fpImage.area_thre_shold = 95;
    ret = btl_core_captureImageWithAgc(
              &fpImage,
              g_enroll_trytimes,
              g_enroll_quality_area_threshold,
              g_enroll_quality_score_threshold,
              WHITE_POINT_VALUE,
              g_points_threshold,
              g_def_capture_contrast,
              0,
#if (SAVE_IMAGE)
              &fpImage_org
#else
			  0
#endif
			  );
    close(g_fd);
    g_fd = -1;

    if (ret != ERR_OK) {
        LOGE("%s,captureImage ret:%d", __func__,ret);
        return ret;
    }

    LOGD("start enroll");

    if (Btl_Enroll(fpImage.p_image,g_width , g_height,0, 0,&g_schedule,&nConverage,NULL) < 0){
		#if (SAVE_IMAGE)
        {
			char imagename[256];
			memset(imagename,0,256);
			sprintf(imagename,"/data/system/users/0/fpdata/ar_i(%2d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_or.bmp",
				g_schedule,fpImage_org.contrast,fpImage_org.quality,fpImage_org.area,fpImage_org.condition);
			setpixcol(fpImage_org.p_image,imagename);	

			memset(imagename,0,256);
            sprintf(imagename,"/data/system/users/0/fpdata/ar_i(%2d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_eh.bmp",
				g_schedule,fpImage.contrast,fpImage_org.quality,fpImage.area,fpImage.condition);
			setpixcol(fpImage.p_image,imagename);
        }
		#endif
		return -ERR_ENROLL;
	}

    LOGD("coverage:%d",nConverage);

	
	#if (SAVE_IMAGE)
    {
    	char imagename[256];
		
    	if(g_last_schedule!=g_schedule || g_schedule==0){
			g_last_schedule = g_schedule;
			memset(imagename,0,256);
			sprintf(imagename,"/data/system/users/0/fpdata/r_i(%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_or.bmp",
				g_schedule,fpImage_org.contrast,fpImage_org.quality,fpImage_org.area,fpImage_org.condition);
			setpixcol(fpImage_org.p_image,imagename);	

			memset(imagename,0,256);
		    sprintf(imagename,"/data/system/users/0/fpdata/r_i(%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_eh.bmp",
				g_schedule,fpImage.contrast,fpImage_org.quality,fpImage.area,fpImage.condition);
			setpixcol(fpImage.p_image,imagename);
		}else{
			memset(imagename,0,256);
			sprintf(imagename,"/data/system/users/0/fpdata/r_i(%d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_or.bmp",
				g_schedule,fpImage_org.contrast,fpImage_org.quality,fpImage_org.area,fpImage_org.condition);
			setpixcol(fpImage_org.p_image,imagename);	

			memset(imagename,0,256);
            sprintf(imagename,"/data/system/users/0/fpdata/r_i(%d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_eh.bmp",
				,g_schedule,fpImage.contrast,fpImage_org.quality,fpImage.area,fpImage.condition);
			setpixcol(fpImage.p_image,imagename);
		}
    }
	#endif
    *pRadio = g_schedule;

    LOGD("radio:%d",*pRadio);

    LOGD("--%s",__func__);
    return 0;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_getTemplate(uint8_t *pTemplateBuf, uint32_t *pTemplateSize)
{
    uint8_t type;
    int32_t size = -1;

    if (Btl_GetTemplateData(&pTemplateBuf[1], 512*1024,&type, &size) != 0)
        return -1;

    pTemplateBuf[0] = type;
    size++;
    *pTemplateSize = size;

    return 0;
}


/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_matchFp(const char* pDBName,
                         int32_t* index,
                         int32_t length,
                         int32_t *fingerID)
{

    int32_t i=0;
    int32_t nRet=0;
    unsigned char a_ucImage[g_width * g_height];
    int nTempFd;
    uint8_t* pTemplateBuffer;
    int32_t  nTemplateSize = 0;
    unsigned short usMatchScore = 0;
    int32_t nDecision  = DECISION_NON_MATCH;
    int32_t nID = 0;
    const uint8_t nOpenTimes = 20;

    fp_image_t	fpImage;

    fpImage.p_image = a_ucImage;

    btl_util_time_update(1);
    for (i = 0; i < nOpenTimes; i++) {
        if (g_CancelFlag)
            return -ERR_CANCEL;
        nTempFd = open(deviceNode,O_RDWR);
        if (nTempFd == -1) {
            LOGE("%s,can't open device:%d ",__func__,nTempFd);
        } else
            break;
    }
    if (i >= nOpenTimes) return -ERR_IO;
    g_fd = nTempFd;

    LOGD("pDBName %s",pDBName);
    ioctl(g_fd,BL229X_INIT);

    nRet = btl_core_captureImageWithAgc(
               &fpImage,
               g_match_trytimes,
               g_match_quality_area_threshold,
               g_match_quality_score_threshold,
               WHITE_POINT_VALUE,
               g_points_threshold,
               g_def_capture_contrast,
               0,
               0);
    close(g_fd);
    g_fd = -1;
    if (nRet != ERR_OK) {
        LOGE("%s, nRet = %d",__func__, nRet);
        return nRet;
    }

    pTemplateBuffer = (uint8_t *)malloc(TEMPLATE_SIZE);
    if (pTemplateBuffer == NULL)
        return -ERR_MEMORY;


    for (i = 0; i < length; i++) {
        if (g_CancelFlag)break;
        if (sql_load_fingerprint(pDBName,0,index[i],pTemplateBuffer, &nTemplateSize) == -1) {
            continue;
        };
        btl_util_time_update(2);
        LOGD("type:%d,size:%d",pTemplateBuffer[0],nTemplateSize);
        if ((nDecision = Btl_Match(
                             a_ucImage, // fp's image
                             g_width,       //  width
                             g_height,        //  height
                             &pTemplateBuffer[1],  //  template's buffer
                             pTemplateBuffer[0],   //  template's type
                             nTemplateSize - 1,    //  template's size
                             g_far_value,          //  value of 'False Acceptance Rate'
                             0,                    //  enhance flag
                             &usMatchScore))       //  score of matching
            > 0) {
            nID = i;
            btl_util_time_diffnow("algotime",2);
            break;
        }

    }
    if (pTemplateBuffer != NULL)
        free((void*)pTemplateBuffer);

    if (g_CancelFlag) {
        LOGD("%s, return as cacnel",__func__);
        return -ERR_CANCEL;
    }

    LOGD("usMatchScore %d",usMatchScore);

    if (nDecision == DECISION_MATCH) {
        *fingerID = index[nID];
        LOGD("%s, compare success",__func__);
        LOGD("decide score : %d",usMatchScore);
        LOGD("nDecision : %d, ID: %d",nDecision,*fingerID);
    } else {
        LOGE("%s, nDecision : %d",__func__,nDecision);
    }
    btl_util_time_diffnow("matchFp",1);
    return nDecision;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_matchFpB(const char* pDBName,
                          int32_t* fid_index,
                          int32_t fid_size,
                          int32_t uid,
                          int32_t *fingerID)
{

    int32_t i = 0,j = 0;
    int32_t nRet=0;
    unsigned char a_ucImage[g_width * g_height];
    int nTempFd;
    uint8_t* pTemplateBuffer[5];
    int32_t  nTemplateSize[5];
    unsigned short usMatchScore = 0;
    int32_t nDecision  = DECISION_NON_MATCH;
    int32_t nID = 0;
    const uint8_t nOpenTimes = 20;
    uint8_t* pTempBuf;
    uint8_t  ntype = 0;
    fp_image_t fpImage;

    fpImage.p_image = a_ucImage;

    btl_util_time_update(1);
    for (i = 0; i < nOpenTimes; i++) {
        if (g_CancelFlag)
            return -ERR_CANCEL;
        nTempFd = open(deviceNode,O_RDWR);
        if (nTempFd == -1) {
            LOGE("%s,can't open device:%d ",__func__,nTempFd);
        } else
            break;
    }
    if (i >= nOpenTimes) return -ERR_IO;
    g_fd = nTempFd;

    LOGD("pDBName %s",pDBName);
    ioctl(g_fd,BL229X_INIT);
    btl_util_time_update(2);
    nRet = btl_core_captureImageWithAgc(
               &fpImage,
               g_match_trytimes,
               g_match_quality_area_threshold,
               g_match_quality_score_threshold,
               WHITE_POINT_VALUE,
               g_points_threshold,
               g_def_capture_contrast,
               0,
               0);
    close(g_fd);
    g_fd = -1;
    btl_util_time_diffnow("capture",2);

    if (nRet != ERR_OK) {
        LOGE("%s, nRet = %d",__func__, nRet);
        return nRet;
    }

    for (i = 0; i < 5; i++) {
        pTemplateBuffer[i] = NULL;
        nTemplateSize[i] = 0;
    }

    for (i = 0; i < fid_size; i++) {
        pTemplateBuffer[i] = (uint8_t *)malloc(512*1024);
        if (pTemplateBuffer[i] == NULL) {
            for (j = i -1; j >=0; j--) {
                if (pTemplateBuffer[j])
                    free((void*)pTemplateBuffer[j]);
            }
            return -ERR_MEMORY;
        }
    }

    for (i = 0; i < fid_size; i++) {
        if (sql_load_fingerprint(pDBName,uid,fid_index[i],pTemplateBuffer[i], &nTemplateSize[i]) == 0) {
            nTemplateSize[i] -= 1;
        } else {
            for (i = 0; i < fid_size; i++)
                if (pTemplateBuffer[i] != NULL)
                    free((void*)pTemplateBuffer[i]);
            return -ERR_MEMORY;
        }
    }


    btl_util_time_update(2);
    nDecision = Btl_MatchB(
                    (const char*) pDBName, // name of template's database
                    a_ucImage,      // fp's image
                    g_width,            // width
                    g_height,             // height
                    g_far_value,    //  value of FAR
                    uid,            // user id
                    fid_size,       // numbers of enrolled templates
                    1,              // decide to enhance image
                    &nID,           // if match ok, nID save the matched index
                    pTemplateBuffer, // template's data buffer
                    nTemplateSize);  // template's size array

    btl_util_time_diffnow("algo",2);

    for (i = 0; i < fid_size; i++)
        if (pTemplateBuffer[i] != NULL)
            free((void*)pTemplateBuffer[i]);

    if (g_CancelFlag) {
        LOGD("%s, return as cacnel",__func__);
        return -ERR_CANCEL;
    }

    if (nDecision == DECISION_MATCH) {
        *fingerID = fid_index[nID];
        LOGD("nDecision : %d, ID: %d",nDecision,*fingerID);
    } else {
        LOGE("%s, nDecision : %d",__func__,nDecision);
    }

    btl_util_time_diffnow("matchFpB",1);
    return nDecision;
}

/*----------------------------------------------------------------------
 purpose:
 return :
 ------------------------------------------------------------------------*/
int32_t btl_core_matchFpC(
    const char* pDBName,
    int32_t* fid_index,
    int32_t  fid_size,
    int32_t  uid,
    int32_t *fingerID)
{

    int32_t i = 0,j = 0;
    int32_t nRet=0;
    unsigned char a_ucImage[g_width * g_height];
    int nTempFd = -1;
    uint8_t* pTemplateBuffer[5];
    int32_t  nTemplateSize[5];
    unsigned short usMatchScore = 0;
    int32_t nDecision  = DECISION_NON_MATCH;
    int32_t nID = 0;
    const uint8_t nOpenTimes = 20;
    fp_image_t fpImage;
    int32_t t1;
	unsigned char t2;
    int index[10];

	#if (SAVE_IMAGE)
	uint8_t p_image_org[g_width * g_height];
	fp_image_t  fpImage_org;
	fpImage_org.p_image = p_image_org;
	#endif

    LOGD("%s++",__func__);

    fpImage.p_image = a_ucImage;

    btl_util_time_update(1);
    for (i = 0; i < nOpenTimes; i++) {
        if (g_CancelFlag)
            return -ERR_CANCEL;
        nTempFd = open(deviceNode,O_RDWR);
        if (nTempFd == -1) {
            LOGE("%s,can't open device:%d ",__func__,nTempFd);
        } else
            break;
    }
    if (i >= nOpenTimes) return -ERR_IO;
    g_fd = nTempFd;

	fpImage.area_thre_shold = 80;

    LOGD("pDBName %s",pDBName);

    btl_util_time_update(2);
    nRet = btl_core_captureImageWithAgc(
               &fpImage,
               g_match_trytimes,
               g_match_quality_area_threshold,
               g_match_quality_score_threshold,
               WHITE_POINT_VALUE,
               g_points_threshold,
               g_def_capture_contrast,
               0,
#if (SAVE_IMAGE)
               &fpImage_org
#else
			   0
#endif		   
			   );
    close(g_fd);
    g_fd = -1;
    btl_util_time_diffnow("capture",2);

    if (nRet != ERR_OK) {
        LOGE("%s, nRet = %d",__func__, nRet);
		return nRet;
    }
    btl_util_time_update(2);      
	memset(nTemplateSize,0,5*sizeof(int32_t));
    for (i = 0; i < fid_size; i++) {
        pTemplateBuffer[i] = (uint8_t *)malloc(ONE_TEMPLATE_SIZE);
        if (pTemplateBuffer[i] == NULL) {
            for (j = i -1; j >=0; j--) {
                if (pTemplateBuffer[j])
                    free((void*)pTemplateBuffer[j]);
            }
        }
    }

    for (i = 0; i < fid_size; i++) {
        if (sql_load_fingerprint(pDBName,uid,fid_index[i],pTemplateBuffer[i], &nTemplateSize[i]) == 0) {
            nTemplateSize[i] -= 1;
        } else {
            for (i = 0; i < fid_size; i++){
                if (pTemplateBuffer[i] != NULL){
                    free((void*)pTemplateBuffer[i]);
				}
    		}
     	}

	}
	if(i != fid_size)
	   return -ERR_LOADDB;	
    btl_util_time_diffnow("sql",2);
    
    btl_util_time_update(3);
    nDecision = Btl_MatchC(
                    (const char*) pDBName, // name of template's database
                    a_ucImage, // fp's image
                    g_width ,
                    g_height,
                    g_far_value,    //  value of FAR
                    uid,             // user id
                    fid_size,       // numbers of enrolled templates
                    0,              // decide to enhance image
                    &nID,           // if match ok, nID save the matched index
                    pTemplateBuffer, // buffer of template's data
                    nTemplateSize,   // size of template's data
                    0,    //  update's template data
                    &t1,  // size of update's template data
                    &t2,  // type of update's template data
                    0,		// decide to need to dyn-update
                    g_update_far_value,// FAR for update
                    NULL);  

    btl_util_time_diffnow("algo",3);

    for (i = 0; i < fid_size; i++)
        if (pTemplateBuffer[i] != NULL)
            free((void*)pTemplateBuffer[i]);

    if (g_CancelFlag) {
        LOGD("%s, return as cancel",__func__);
        return -ERR_CANCEL;
    }
	#if (SAVE_IMAGE)
    {
        char imagename[256];
        //int  matchflag = 1;
		
		int min,sec,hour;
		{
		    time_t timep;
		    struct tm *p;
		    time(&timep);
		    p = gmtime(&timep);
		    hour= p->tm_hour;
		    min = p->tm_min;
		    sec = p->tm_sec;
		}
		
        if (nDecision == DECISION_MATCH) {
			memset(imagename,0,256);
			sprintf(imagename,"/data/system/users/0/fpdata/m_time(%d-%d-%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_or.bmp",
				hour,min,sec,fpImage_org.contrast,fpImage_org.quality,fpImage_org.area,fpImage_org.condition);
			setpixcol(fpImage_org.p_image,imagename);	

			memset(imagename,0,256);
		    sprintf(imagename,"/data/system/users/0/fpdata/m_time(%d-%d-%d)_o_dacp(%2x)_q(%d)_a(%d)_c(%d)_eh.bmp",
				hour,min,sec,fpImage.contrast,fpImage.quality,fpImage.area,fpImage.condition);
			setpixcol(fpImage.p_image,imagename);
            //sprintf(temp_buf,"/data/system/users/0/fpdata/m-ok-%d-%d-%d-%d.bin",abs(nRet),hour,min,sec);
            //matchflag = 0;
        } else{
			memset(imagename,0,256);
			sprintf(imagename,"/data/system/users/0/fpdata/m_time(%d-%d-%d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_or.bmp",
				hour,min,sec,fpImage_org.contrast,fpImage_org.quality,fpImage_org.area,fpImage_org.condition);
			setpixcol(fpImage_org.p_image,imagename);	

			memset(imagename,0,256);
		    sprintf(imagename,"/data/system/users/0/fpdata/m_time(%d-%d-%d)_f_dacp(%2x)_q(%d)_a(%d)_c(%d)_eh.bmp",
				hour,min,sec,fpImage.contrast,fpImage.quality,fpImage.area,fpImage.condition);
			setpixcol(fpImage.p_image,imagename);
		}
    }
	#endif


    if (nDecision == DECISION_MATCH) {
        *fingerID = fid_index[nID];
        LOGD("nDecision : %d, ID: %d",nDecision,*fingerID);
    } else {
        LOGE("%s, nDecision : %d",__func__,nDecision);
    }

    btl_util_time_diffnow("matchFpC",1);
    return nDecision;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_setInterruptWorkMode()
{
    int tempFd;
    int nTryOpenTimes = 50;
    int i;

    LOGD("%s",__func__);

    for (i = 0; i < nTryOpenTimes; i++) {
        tempFd = open(deviceNode,O_RDWR);
        if (tempFd == -1) {
            LOGE("%s, can't open device: %d",__func__,tempFd);
        } else
            break;
    }

    if (i == nTryOpenTimes) return -ERR_IO;

    g_fd = tempFd;
    ioctl(g_fd,BL229X_INTERRUPT_MODE);//进入中断模式
    close(g_fd);
    g_fd = -1;
    return 0;
}
/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_initIC(void)
{
    int tempFd;
    int nTryOpenTimes = 50;
    int i;

    LOGD("%s",__func__);

    for (i = 0; i < nTryOpenTimes; i++) {
        tempFd = open(deviceNode,O_RDWR);
        if (tempFd == -1) {
            LOGE("%s, can't open device: %d",__func__,tempFd);
        } else
            break;
    }

    if (i == nTryOpenTimes) return -ERR_IO;

    g_fd = tempFd;
    ioctl(g_fd,BL229X_INIT);
    close(g_fd);
    g_fd = -1;
    return 0;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_enrollStart()
{
    g_schedule = 0;

    if (Btl_GetMteStatus())
        Btl_UninitAlgo();
    Btl_InitAlgo();
    return 0;
}


/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_core_getDefaultParams()
{
    int tempFd;
    struct fingerprintd_params_t fingerprintd_params;
    chip_info_t chip_info;
	
    tempFd = open(deviceNode,O_RDWR);
    if (tempFd == -1) {
        LOGE("can't open file:%d ",tempFd);
        return -1;
    }
    g_fd = tempFd;
    if (btl_hw_getParams(g_fd,&fingerprintd_params) == 0) {
        if (fingerprintd_params.def_contrast) {
            g_def_capture_contrast  = fingerprintd_params.def_contrast;
            g_best_capture_contrast = g_def_capture_contrast;
        }
        if (fingerprintd_params.def_ck_fingerup_contrast)
            g_best_fingerup_contrast = fingerprintd_params.def_ck_fingerup_contrast;

		if (fingerprintd_params.def_enroll_ck_fingerup_timeout)
            g_enroll_ck_fingerup_timeout = fingerprintd_params.def_enroll_ck_fingerup_timeout;

		if (fingerprintd_params.def_match_ck_fingerup_timeout)
            g_match_ck_fingerup_timeout = fingerprintd_params.def_match_ck_fingerup_timeout;

		if (fingerprintd_params.def_ck_fingerup_contrast)
            g_best_fingerup_contrast = fingerprintd_params.def_ck_fingerup_contrast;
		  
        if (fingerprintd_params.def_match_failed_times >= 5)
            g_match_failed_times = fingerprintd_params.def_match_failed_times;
        if (fingerprintd_params.def_enroll_try_times)
            g_enroll_trytimes = fingerprintd_params.def_enroll_try_times;
        if (fingerprintd_params.def_match_try_times)
            g_match_trytimes = fingerprintd_params.def_match_try_times;
        if (fingerprintd_params.def_intensity_threshold)
            g_intensity_threshold = fingerprintd_params.def_intensity_threshold;
        if (fingerprintd_params.def_contrast_high_value)
           g_contrast_high_value = fingerprintd_params.def_contrast_high_value;
        if (fingerprintd_params.def_contrast_low_value)
            g_contrast_low_value = fingerprintd_params.def_contrast_low_value;
        if (fingerprintd_params.def_match_quality_score_threshold > 0
            && fingerprintd_params.def_match_quality_score_threshold <= 100)
            g_match_quality_score_threshold = fingerprintd_params.def_match_quality_score_threshold;
        if (fingerprintd_params.def_match_quality_area_threshold > 0
            && fingerprintd_params.def_match_quality_area_threshold <= 26)
            g_match_quality_area_threshold = fingerprintd_params.def_match_quality_area_threshold;
        if (fingerprintd_params.def_enroll_quality_score_threshold > 0
            && fingerprintd_params.def_enroll_quality_score_threshold <= 100)
            g_enroll_quality_score_threshold = fingerprintd_params.def_enroll_quality_score_threshold;
        if (fingerprintd_params.def_enroll_quality_area_threshold > 0
            && fingerprintd_params.def_enroll_quality_area_threshold <= 26)
            g_enroll_quality_area_threshold = fingerprintd_params.def_enroll_quality_area_threshold;
        if (fingerprintd_params.def_shortkey_disable)
            g_shotkey_disable = fingerprintd_params.def_shortkey_disable;
        if (fingerprintd_params.def_far_rate > BF_PB_FAR_1 && fingerprintd_params.def_far_rate < BF_PB_FAR_1000M)
            g_far_value = fingerprintd_params.def_far_rate;
        if (fingerprintd_params.def_max_samples > 0 && fingerprintd_params.def_max_samples < 32)
            g_enroll_max_counts = fingerprintd_params.def_max_samples;
        if (fingerprintd_params.def_debug_enable)
            g_debug_enable = fingerprintd_params.def_debug_enable;
        if (fingerprintd_params.def_contrast_direction)
            g_contrast_direction = fingerprintd_params.def_contrast_direction;
		if (fingerprintd_params.def_step_counts)
		    g_contrast_finetune_step = fingerprintd_params.def_step_counts;
		if (fingerprintd_params.def_algorithm_type)
			g_algorithm_type = fingerprintd_params.def_algorithm_type;
        if (fingerprintd_params.def_enhance_image)
	    	g_def_enhance_image = fingerprintd_params.def_enhance_image;
		if (fingerprintd_params.reserved1)
			g_points_threshold = fingerprintd_params.reserved1;

		if (fingerprintd_params.chip_id)
			g_chip_id = fingerprintd_params.chip_id;

		g_online_update = fingerprintd_params.def_update;
		g_def_gain = fingerprintd_params.def_gain;
    } else {
        g_best_capture_contrast           = DEFAULT_CONTRAST_FOR_CAPTURE;
        g_best_fingerup_contrast          = DEFAULT_CONTRAST_FOR_FINGERUP;
        g_match_failed_times              = DEFAULT_MATCH_FAILED_TIMES;
        g_match_trytimes                  = DEFAULT_MATCH_TRYTIMES;
        g_enroll_trytimes                 = DEFAULT_ENROLL_TRYTIMES;
        g_intensity_threshold             = DEFAULT_INTENSITY_THRESHOLD;
        g_contrast_high_value             = DEFAULT_CONTRAST_HIGH;
        g_contrast_low_value              = DEFAULT_CONTRAST_LOW;
        g_match_quality_score_threshold   = DEFAULT_MATCH_SCORE_THRESHOLD;
        g_enroll_quality_score_threshold  = DEFAULT_ENROLL_SCORE_THRESHOLD;
        g_match_quality_area_threshold    = DEFAULT_MATCH_AREA_THRESHOLD;
        g_enroll_quality_area_threshold   = DEFAULT_ENROLL_AREA_THRESHOLD;
        g_shotkey_disable                 = DEFAULT_SHOTKEY_DISABLE;
        g_far_value                       = DEFAULT_FAR;
        g_enroll_max_counts               = DEFAULT_ENROLL_COUNTS;
        g_debug_enable                    = 0;
        g_contrast_direction              = 0;
		g_online_update                   = 0;
    }
    //g_contrast_high_value = 255;
    //g_contrast_low_value = 0;
    g_def_enhance_image = 2;
    g_match_quality_score_threshold = 30;
    g_enroll_quality_score_threshold = 35;

	if(g_chip_id == 0x5183)
	{
		g_width = 112;
		g_height = 96;
		g_area_max = 26;
		g_best_algo = 28;
		TEMPLATE_SIZE = 150 * 1024;
		g_far_value = BF_PB_FAR_50000;
		g_update_far_value = BF_PB_FAR_500000;
		if (btl_hw_get_chip_info(g_fd,&chip_info)  == 0){
			g_contrast_direction = (chip_info.chip_driver_type == 0) ? 1:0;
		}
		LOGD("%s,%d",__func__,chip_info.chip_driver_type);
	}else if(g_chip_id == 0x5283)
	{
		g_width = 72;
		g_height = 128;
		g_area_max = 23;
		g_best_algo = 24;	
		TEMPLATE_SIZE = 150 * 1024;
		g_far_value = BF_PB_FAR_100000;
		g_update_far_value = BF_PB_FAR_500000;
	}else if(g_chip_id == 0x5383)
	{
		g_width = 80;
		g_height = 80;
		g_area_max = 15;
		g_best_algo = 20;		
		TEMPLATE_SIZE = 500 * 1024;
		g_far_value = BF_PB_FAR_20M;
		g_update_far_value = BF_PB_FAR_100M;
	}
	
    close(g_fd);
    g_fd = -1;
	
	LOGD("g_width = %d, g_height = %d, g_area_max = %d, g_best_algo = %d",
		g_width, g_height, g_area_max, g_best_algo);
	g_match_quality_area_threshold = g_area_max * 6/ 10;
	g_enroll_quality_area_threshold = g_area_max * 19 / 20;
    g_algorithm_type = g_best_algo;
    LOGD("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,0x%x,0x%x",
         __func__,
         g_def_capture_contrast,
         g_best_fingerup_contrast,
         g_enroll_ck_fingerup_timeout,
         g_match_ck_fingerup_timeout,
         g_match_failed_times,
         g_match_trytimes,
         g_enroll_trytimes,
         g_intensity_threshold,
         g_contrast_high_value,
         g_contrast_low_value,
         g_match_quality_score_threshold,
         g_enroll_quality_score_threshold,
         g_match_quality_area_threshold,
         g_enroll_quality_area_threshold,
         g_shotkey_disable,
         g_far_value,
         g_enroll_max_counts,
         g_debug_enable,
         g_contrast_direction,
         g_online_update,
         g_contrast_finetune_step,
         g_algorithm_type,
         g_points_threshold,
         g_def_enhance_image,
         g_def_gain,
         g_chip_id);

	LOGD("g_algorithm_type=%d",g_algorithm_type);
	Btl_SetAlgorithmType(g_algorithm_type);
	Btl_SetAlgorithmID(0x5183);
	Btl_SetTemplateMaxSize(TEMPLATE_SIZE);
	Btl_EnableDebug(g_debug_enable);

    g_inited = 1;
    return 0;

}
/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int  btl_core_setCancel(int8_t flag)
{
    g_CancelFlag = flag;
    return 0;
}
/*------------------------------------------------------------------------*/
int32_t btl_core_unInitAlgo()
{
    return Btl_UninitAlgo();

}

int32_t btl_core_setDebug(int debug)
{
    g_debug_enable = debug;

    return 0;
}

int32_t btl_core_InitAlgo()
{
    return Btl_InitAlgo();
}

int32_t btl_core_getMteStatus()
{
    return Btl_GetMteStatus();
}

/*-------------------------------------------------------------------------*/
int32_t btl_core_setAgc (uint32_t contrast)
{
    int32_t rx_ret=0;

    LOGD("%s,cur agc: %d",__func__,contrast);

    int32_t tempFd;

    LOGD("++%s",__func__);
    tempFd = open(deviceNode,O_RDWR);
    if (tempFd == -1) {
        LOGE ("%s,can't open device!\n",__func__);
        return -1;
    }

    rx_ret = ioctl(tempFd,BL229X_Adjustment_AGC,contrast);
    if (rx_ret == -1) {
        LOGE("%s, ioctl agc ret = %d ",__func__,rx_ret);
        return -ERR_IO;
    }
    close(tempFd);

    return 0;

}


int btl_core_mmiTest()
{
    int i=0;
    int j=0;
    int flag=0;
    int ret=0;
    char imagename[200];
    uint8_t t_image[g_width * g_height];
    fp_image_t fpImage;
    uint8_t tryTimes = 1;
    uint32_t quality_area_threshold  = g_match_quality_area_threshold;
    uint32_t quality_score_threshold = g_match_quality_score_threshold;

    LOGE("%s++",__func__);

    btl_core_setCancel(1);

    int tempFd = -1;
    tempFd = open(deviceNode,O_RDWR);
    if (tempFd == -1) {
        LOGE ("can't open BL239X!\n");
        btl_core_setCancel(0);
        return -ERR_OK;
    }
    btl_core_setCancel(0);
    g_fd = tempFd;

    fpImage.p_image = t_image;

    ret = btl_core_captureImageWithAgc(
              &fpImage,
              1,
              quality_area_threshold,
              quality_score_threshold,
              WHITE_POINT_VALUE,
              WHITE_POINT_COUNTS,
              g_def_capture_contrast,
              0,
			  0);
    close(g_fd);
    if (ret != ERR_OK)  {
        LOGE("%s, %d",__func__, ret);
    } else {
        LOGD("Save Picture");
        sprintf(imagename,"/storage/sdcard0/finger_001.bmp");
        setpixcol(t_image,imagename);
    }

    LOGE("%s--",__func__);
    return ret;
}

int btl_core_User_GetId(int *outbuffer)
{
    uint32_t buffer[20];
    int tempfd;
    tempfd = open(deviceNode,O_RDWR);
    if (tempfd == -1) {
        LOGE ("can't open bl229x!\n");
        return -1;
    }

    ioctl(tempfd,BL229X_GET_ID,buffer);//杩涘叆涓柇妯″紡
    close(tempfd);
    LOGW ("%x,%x",buffer[0],buffer[1]);
    *outbuffer  =  buffer[0];
    return 0;
}
