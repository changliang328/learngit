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
#include <sys/stat.h>

#include <time.h>
#include <unistd.h>
#include <math.h>
#include <android/log.h>

#include "btlfp.h"
#include "btl_algorithm_interface.h"
#include "btlbmp.h"
#include "btlcustom.h"
#include "auto_gain_dacp.h"
extern int FP_ImagePreproc(unsigned char* imgData, int width, int height, unsigned char bgValue,unsigned char minValidArea);
#define FINGER_DEBUG            1

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

//capture image end
/*----------------------------------------------------------b--------------*/

#define IMAGE_SIZE                     FINGER_WIDTH*FINGER_HEIGHT
#define RAW_FINGERPRINT_SIZE	       IMAGE_SIZE

#define BL229X                         0
#define BL239X                         1


#define TEMPLATE_SIZE                  100*1024
#define ONE_TEMPLATE_SIZE                  80*1024

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
uint8_t  g_debug_enable = 1;
uint8_t  g_contrast_direction     = 0;
uint8_t  g_contrast_finetune_step = 4;
uint8_t  g_capture_ck_empty_frames = BADIMAGE_TIMEOUT_COUNTS;
uint8_t  g_inited = 0;
uint8_t  g_online_update = 0;
extern int g_fid_subindex;
extern int g_fid_index;
uint16_t g_points_threshold = 2000;
uint16_t g_background_value = 230;
#define MAX_PIPEBUFFER_SIZE		1024
static const char *navCtrlPipe = "/data/system/users/0/fpdata/navCtrlPipe";

//static const char *navCtrlPipe = "/data/user/0/com.blestech.navigation/navCtrlPipe";
//static const char *navCtrlPipe = "/data/user/0/com.btlfinger.fingerprintunlock/navCtrlPipe";

int wirte_navCtlPipe(char *data)
{
	int fd;
	//char buff[MAX_PIPEBUFFER_SIZE];
	int nwrite;
	LOGD("++ %s %s", __func__, data);
	if (access(navCtrlPipe, F_OK) == -1)
	{
		LOGE("nav ctl fifo file do not not exist\n");
		   //return -1;
	   if ((mkfifo(navCtrlPipe, 0666) < 0) /*&& (errno != EEXIST)*/)
	   {
		   LOGE("Cannot create nav ctl fifo file\n");
		   return -1;
	   }
	   LOGD("mkfifo nav ctl success\n");
	 }
	LOGD("++ %s 1111", __func__);
	//sscanf(data, "%s", buff);
	/* ÒÔÖ»Ð´×èÈû·½Ê½´ò¿ªFIFO¹ÜµÀ */
	fd = open(navCtrlPipe, O_WRONLY);//| O_NONBLOCK
	LOGD("++ %s 2222", __func__);
	if (fd == -1)
	{
		LOGE("Open fifo file error\n");
		return -1;
	}
	LOGD("++ %s 3333", __func__);
	/*Ïò¹ÜµÀÖÐÐ´Èë×Ö·û´®*/
	if ((nwrite = write(fd, data, strlen(data) + 1)) > 0)
	{
		LOGD("Write '%s' to nav ctl FIFO\n", data);
		close(fd);
		return -2;
	}
	close(fd);
	LOGD("-- %s", __func__);
	return 0;
}


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
int32_t btl_core_readFpImage (
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

    rx_ret = read(device_fd,pimage,(RAW_FINGERPRINT_SIZE));
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
float nGain2 = 11.3;//11.3(cover) or 8(coating)
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
	LOGD("curContrast=%d,nDirect=%d,ncurMeanValue=%d,ndstMeanValue=%dï¼Œtempvalue=%f",curContrast,nDirect,ncurMeanValue,ndstMeanValue,tempvalue);
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
    uint8_t contrast_step,
    uint8_t white_value_empty_image,
    int32_t points_threshold_empty_image,
    uint8_t intensity_threshold_for_best_contrast,
    int32_t nDefaultContrast,
    int32_t *pAdjustFrames,
    fp_image_t** p_image_params
)
{
    int32_t  nIsDry = 0,nIntensity = 0,nImgVar = 0,nImgVar_MAX = 0;
    uint8_t  imageA[RAW_FINGERPRINT_SIZE], imageB[RAW_FINGERPRINT_SIZE];
    int32_t  rimageStatus = 0;
    uint8_t  validFpCounts = 0;
    uint8_t  emptyImageCounts = 0;
    uint8_t  ioErrorCounts = 0;
    uint8_t  inValidImageCounts = 0;
    int32_t  nQua = 0, bestQua = 0;
    int32_t  nValidArea = 0;
    int8_t   pressed = 0;
    uint8_t  matchCompleted = 0;
    uint8_t  highQuailty = 80;
    uint8_t  midQuailty = 55;
    uint8_t  lowQuailty = 30;
    int32_t  nCheckImageQuality = -1;
    uint8_t  goodImage = 0;
    int32_t  curContrast;
    uint8_t  isValidFpImage = 0;
    int32_t  tempContrast = nDefaultContrast;
    int32_t  tempDryStatus = 0;
    int32_t  tempIntensity = 0;
    int32_t  nCondition = 0;
    uint8_t  pEnhanceData[RAW_FINGERPRINT_SIZE];    
    int32_t  temp_high_value = nDefaultContrast;
    int32_t  temp_low_value = nDefaultContrast;
	int32_t ret = 0;
	int32_t nCurMeanValue = 0;
		int32_t mybestContrast = 0;
    LOGD("++%s sky g_contrast_direction=%d",__func__,g_contrast_direction);

    if (g_CancelFlag)
        return -ERR_CANCEL;

    curContrast = nDefaultContrast;
    memset(imageA,0xff,RAW_FINGERPRINT_SIZE);

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

		isValidFpImage = 1;

        pressed = 1;

        //increment image counts
        validFpCounts++;
        
        /* -- process images quality from 31 to 79 --*/
        if(isValidFpImage) {
            nCheckImageQuality = -1;
            btl_util_time_update(3);
	    	memcpy(pFpImage->p_image,imageA,RAW_FINGERPRINT_SIZE);
			ret = FP_ImagePreproc(pFpImage->p_image, FINGER_WIDTH, FINGER_HEIGHT, g_background_value, 50);
			if(ret == 0)
			{
				#if (SAVE_IMAGE)
				char imagename[256];
				sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_org.bmp",nQua,nValidArea,nIntensity,curContrast);
				setpixcol(imageA,imagename);			
				sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_enhance.bmp",nQua,nValidArea,nIntensity,curContrast);
				setpixcol(pFpImage->p_image,imagename);
				#endif
		        nCheckImageQuality = Btl_CheckImageQuality(pFpImage->p_image, FINGER_WIDTH, FINGER_HEIGHT, nAreaThreShold, nScoreThreShold, &nQua, &nValidArea,&nCondition);
            	LOGD ("CheckImageQuality %d,qual :%d ,area: %d, condition:%d,curContrast=%d\n",nCheckImageQuality,nQua,nValidArea,nCondition,curContrast);
			}
			else{
				#if (SAVE_IMAGE)
				char imagename[256];
				sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_org.bmp",nQua,nValidArea,nIntensity,curContrast);
				setpixcol(imageA,imagename);			
				sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_enhance.bmp",nQua,nValidArea,nIntensity,curContrast);
				setpixcol(pFpImage->p_image,imagename);
				#endif
				LOGD ("FP_ImagePreproc failed goodImage=%d ret=%d",goodImage,ret);
				goodImage = 0;		
			}
            btl_util_time_diffnow("Quality",3);
        	nCurMeanValue = btl_cal_finerMean(imageA, FINGER_WIDTH, FINGER_HEIGHT);
            if (nCheckImageQuality == 0) {
		   	   if((nCurMeanValue >= 100)&&(nCurMeanValue <= 156))
		   	   {
				   goodImage = 1;
				   break;
		       }
            }

			//mybestContrast = btl_find_best_contrast(curContrast,g_contrast_direction,nCurMeanValue,128);
			REG_VALUE_3182 oldRegValue = {g_def_gain,0x6d,curContrast};
			REG_VALUE_3182 newRegValue;
			ret = bl_AutoGainDacp_3182(imageA, FINGER_WIDTH, FINGER_HEIGHT,  &oldRegValue, &newRegValue,128);
			curContrast = newRegValue.reg_value_0x1c;
			LOGD ("ret=%d mybestContrast=%x oldGainDacp.dacp=%x newGainDacp.dacp=%x",ret, mybestContrast,oldRegValue.reg_value_0x1c, newRegValue.reg_value_0x1c);
        }
        if (validFpCounts >= tryTimes) {
        	break;
        }
    }

    if (pAdjustFrames)
        *pAdjustFrames = validFpCounts;
    LOGD ("g_def_enhance_image=%d",g_def_enhance_image);
    if(g_def_enhance_image == 1){
		//btl_core_enhance_image(imageA,pEnhanceData, 2);
		//memcpy(pEnhanceData,imageA,FINGER_HEIGHT*FINGER_WIDTH);
		btl_core_enhance_image(imageA,pEnhanceData, 2);
        memcpy(pFpImage->p_image,pEnhanceData,RAW_FINGERPRINT_SIZE);
		    char imagename[256];
	    	sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_org.bmp",nQua,nValidArea,nIntensity,curContrast);
	    	setpixcol(imageA,imagename);			
	    	sprintf(imagename,"/data/system/users/0/fpdata/finger_%d_%d_%d_%d_enhance.bmp",nQua,nValidArea,nIntensity,curContrast);
	    	setpixcol(pEnhanceData,imagename);
    }else if(g_def_enhance_image == 2){
	    //memcpy(pFpImage->p_image,imageA,RAW_FINGERPRINT_SIZE);
	}else{
    	memcpy(pFpImage->p_image,imageA,RAW_FINGERPRINT_SIZE);
    }
    pFpImage->imageValid = 1;
    pFpImage->contrast   = curContrast;
    pFpImage->area       = nValidArea;
    pFpImage->quality    = nQua;
    pFpImage->isDry      = nIsDry;
    pFpImage->intensity  = nIntensity;
    pFpImage->imageVar   = nImgVar;
    pFpImage->condition  = nCondition;
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

	if (nImageBufSize < (tryTimes+1)*FINGER_WIDTH*FINGER_HEIGHT){
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

    p_org_image = malloc(FINGER_WIDTH*FINGER_HEIGHT);

    memset(p_org_image,0x0,FINGER_WIDTH*FINGER_HEIGHT);

    if (!p_org_image) {
        LOGE("%s,p_org_image is 0",__func__);
        *p_func_result = -ERR_MEMORY;
        return -1;
    }

    def_contrast = fixed_contrast;

    fpImage.p_image =  p_org_image;

    p_image = pOutImage;

    p_debug_image = p_image + FINGER_WIDTH*FINGER_HEIGHT;

    for (i = 0; i < 10; i++)
        p_image_params[i] = 0;

    for (i = 0; i < tryTimes; i++) {
        if (p_image_params[i])
            p_image_params[i]->p_image = p_debug_image + i*FINGER_WIDTH*FINGER_HEIGHT;
    }

    LOGD("%s,trytimes:%d",__func__,tryTimes);

    *p_func_result = btl_core_captureImageWithAgc(
                         &fpImage,
                         tryTimes,
                         nAreaThreShold,
                         nScoreThreShold,
                         contrast_step,
                         WHITE_POINT_VALUE,
                         g_points_threshold,
                         best_intensity_threshold,
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
            Btl_CheckImageQuality(p_image_params[i]->p_image, FINGER_WIDTH, FINGER_HEIGHT, nAreaThreShold, nScoreThreShold, &nQua, &nValidArea,&nCondition);
            p_image_params[i]->area = nValidArea;
            p_image_params[i]->quality = nQua;
            p_image_params[i]->condition = nCondition;
        }
    }
    memcpy(p_image,p_org_image,FINGER_WIDTH*FINGER_HEIGHT);
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
    if (Btl_EnhanceImage(pOrgImg, pEnhanceImg, FINGER_WIDTH,FINGER_HEIGHT, enhance_type)) {
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
    uint8_t image[RAW_FINGERPRINT_SIZE];
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
        btl_core_readFpImage(image,100,g_fd,curContrast); //??È¡Í¼??
        imageCheckRet = btl_preprocess_imageCheckEmpty(image,WHITE_POINT_VALUE,g_points_threshold,RAW_FINGERPRINT_SIZE);
        timeoutCouns++;
        if (imageCheckRet == -1) {
            validImageCounts = 0; //??Ö¸?Ú´???????????
            emptyImageCounts ++;
            if (emptyImageCounts >= checkFingerUpMaxCounts) {   //????5Ö¡?Õ°?Í¼????Ëµ????Ö¸?ë¿ª
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
    LOGD("fingerup flag:%d,%d",fingerUpFlag,g_points_threshold);
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
    uint8_t p_image[FINGER_WIDTH*FINGER_HEIGHT];
    int32_t tempFd = -1;
    int32_t nConverage = 0;
    uint8_t i;
    const uint8_t nOpenTimes = 20;
    fp_image_t  fpImage;

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
    ret = btl_core_captureImageWithAgc(
              &fpImage,
              g_enroll_trytimes,
              g_enroll_quality_area_threshold,
              g_enroll_quality_score_threshold,
              g_contrast_finetune_step,
              WHITE_POINT_VALUE,
              g_points_threshold,
              (uint8_t)g_intensity_threshold,
              g_def_capture_contrast,
              0,
              0);
    close(g_fd);
    g_fd = -1;

    if (ret != ERR_OK) {
        LOGE("%s,captureImage ret:%d", __func__,ret);
        {
            char temp_buf[128];
            sprintf(temp_buf,"/data/system/users/0/fpdata/r-%d-%d-%d.bin",abs(ret),g_fid_index,g_fid_subindex);
            Create_binFile(
                p_image,
                temp_buf,
                FINGER_WIDTH,
                FINGER_HEIGHT,
                g_fid_index,
                g_fid_subindex,
                fpImage.quality,
                fpImage.condition,
                fpImage.area,
                fpImage.contrast,
                abs(ret),
                0,
                0,
                0,
                0,
                0);
        }
        return ret;
    }

    LOGD("start enroll");

    if (Btl_Enroll(fpImage.p_image,FINGER_WIDTH,FINGER_HEIGHT,0, 0,&g_schedule,&nConverage) < 0)
        return -ERR_ENROLL;

    LOGD("coverage:%d",nConverage);
    {
        char temp_buf[128];
        sprintf(temp_buf,"/data/system/users/0/fpdata/r-%d-%d-%d.bin",ret,g_fid_index,g_fid_subindex);
        Create_binFile(p_image,
                       temp_buf,
                       FINGER_WIDTH,
                       FINGER_HEIGHT,
                       g_fid_index,
                       g_fid_subindex,
                       fpImage.quality,
                       fpImage.condition,
                       fpImage.area,
                       fpImage.contrast,
                       abs(ret),
                       0,
                       0,
                       0,
                       0,
                       0);
    }
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

    if (Btl_GetTemplateData(&pTemplateBuf[1], 256*1024,&type, &size) != 0)
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
    unsigned char a_ucImage[RAW_FINGERPRINT_SIZE];
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
               g_contrast_finetune_step,
               WHITE_POINT_VALUE,
               g_points_threshold,
               (uint8_t)g_intensity_threshold,
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
                             FINGER_WIDTH,       //  width
                             FINGER_HEIGHT,        //  height
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
    unsigned char a_ucImage[RAW_FINGERPRINT_SIZE];
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
               g_contrast_finetune_step,
               WHITE_POINT_VALUE,
               g_points_threshold,
               (uint8_t)g_intensity_threshold,
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
        pTemplateBuffer[i] = (uint8_t *)malloc(80*1024);
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
                    FINGER_WIDTH,            // width
                    FINGER_HEIGHT,             // height
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
    unsigned char a_ucImage[RAW_FINGERPRINT_SIZE];
    int nTempFd = -1;
    uint8_t* pTemplateBuffer[5];
    int32_t  nTemplateSize[5];
    unsigned short usMatchScore = 0;
    int32_t nDecision  = DECISION_NON_MATCH;
    int32_t nID = 0;
    const uint8_t nOpenTimes = 20;
    uint8_t* pTempBuf = NULL;
    uint8_t  ntype = 0;
    fp_image_t fpImage;
    uint8_t* pUpdateData = NULL;
    int32_t  nUpdateDataSize = 0;
    uint8_t  u8NeedUpdate = g_online_update;
    int min,sec,hour;
    int file_index = 0;
    int index_length = 0;
    int size_read = 0;
    char read_size[10] = {0x00};
    int index[10];
    int s = 0;

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

    LOGD("pDBName %s",pDBName);

    btl_util_time_update(2);
    nRet = btl_core_captureImageWithAgc(
               &fpImage,
               g_match_trytimes,
               g_match_quality_area_threshold,
               g_match_quality_score_threshold,
               g_contrast_finetune_step,
               WHITE_POINT_VALUE,
               g_points_threshold,
               g_intensity_threshold,
               g_def_capture_contrast,
               0,
               0);
    close(g_fd);
    g_fd = -1;
    btl_util_time_diffnow("capture",2);

    {
        time_t timep;
        struct tm *p;
        time(&timep);
        p = gmtime(&timep);
        hour= p->tm_hour;
        min = p->tm_min;
        sec = p->tm_sec;
    }

    if (nRet != ERR_OK) {
        LOGE("%s, nRet = %d",__func__, nRet);
        {
            char temp_buf[128];
            sprintf(temp_buf,"/data/system/users/0/fpdata/m-err-%d-%d-%d-%d.bin",abs(nRet),hour,min,sec);
            Create_binFile(fpImage.p_image,
                           temp_buf,
                           FINGER_WIDTH,
                           FINGER_HEIGHT,
                           g_fid_index,
                           g_fid_subindex,
                           fpImage.quality,
                           fpImage.condition,
                           fpImage.area,
                           fpImage.contrast,
                           abs(nRet),
                           1,
                           hour,
                           min,
                           sec,
                           2);
            return nRet;
        }
    }
    if (u8NeedUpdate)
        pUpdateData = (uint8_t *)malloc(ONE_TEMPLATE_SIZE);

    if (!pUpdateData && u8NeedUpdate)
        return -ERR_MEMORY;

    #if FINGER_DATA_FILE
    for (i = 0; i  < 8; i++)
        index[i] = 0;

    for (i = 0, s = 0; i < 5; i++) {
        //LOGD("User_ma_Match index:%d",mEnrolledIndex[i]);
        if (mEnrolledIndex[i] != 0) {
            index[s++] = i+1;
            index_length++;
        }
    }
    fid_size = index_length;

    #endif


    for (i = 0; i < 5; i++) {
        pTemplateBuffer[i] = NULL;
        nTemplateSize[i] = 0;
    }

    for (i = 0; i < fid_size; i++) {
        pTemplateBuffer[i] = (uint8_t *)malloc(ONE_TEMPLATE_SIZE);
        if (pTemplateBuffer[i] == NULL) {
            for (j = i -1; j >=0; j--) {
                if (pTemplateBuffer[j])
                    free((void*)pTemplateBuffer[j]);
            }
            if (pUpdateData)
                free(pUpdateData);
            return -ERR_MEMORY;
        }
    }
    #if FINGER_DATA_FILE
	for (file_index = 0; file_index < index_length; file_index++) {
			LOGD("btl_hal_match-->length: %d , file_index : %d",index_length,index[file_index]);
			int num = index[file_index];
			char *num_char = (char*)malloc(sizeof(char));
	
			if(num_char == NULL) {
				return 0;
			}
			myitoa(num,num_char,10);
	
			char *res_file_name = str_contact(dataFolderPath, num_char);
	
			if(num_char != NULL) {
				free(num_char);
				num_char = NULL;
			}
	
#ifdef FINGER_DEBUG
			LOGD("btl_hal_match-->filename : %s",res_file_name);
#endif
			FILE *fp1 = fopen(res_file_name, "rb");
	
			if (fp1 == NULL) {
				LOGE("btl_hal_match-->open file %s fail",res_file_name);
				return -1;
			} else {
				fread(read_size, 10, 1, fp1);
				size_read = atoi(read_size);
#ifdef FINGER_DEBUG
				LOGD("file size is %d \n",size_read);
#endif
				uint8_t* buffer_read = (uint8_t*)malloc(size_read);
	
				fseek(fp1, 10L, 0);
				fread(buffer_read, size_read, 1, fp1);
				fclose(fp1);
				LOGD("before memcpy(pTemplateBuffer...");
				memcpy(pTemplateBuffer[file_index],buffer_read,size_read);
				nTemplateSize[file_index] = size_read-1;
				free(buffer_read);
			}
		}
    #else
    for (i = 0; i < fid_size; i++) {
        if (sql_load_fingerprint(pDBName,uid,fid_index[i],pTemplateBuffer[i], &nTemplateSize[i]) == 0) {
            nTemplateSize[i] -= 1;
        } else {
            for (i = 0; i < fid_size; i++)
                if (pTemplateBuffer[i] != NULL)
                    free((void*)pTemplateBuffer[i]);

            if (pUpdateData)
                free(pUpdateData);
            return -ERR_MEMORY;
        }
    }
    #endif
    
    btl_util_time_update(3);
    nDecision = Btl_MatchC(
                    (const char*) pDBName, // name of template's database
                    a_ucImage, // fp's image
                    FINGER_WIDTH,
                    FINGER_HEIGHT,
                    g_far_value,    //  value of FAR
                    uid,             // user id
                    fid_size,       // numbers of enrolled templates
                    0,              // decide to enhance image
                    &nID,           // if match ok, nID save the matched index
                    pTemplateBuffer, // buffer of template's data
                    nTemplateSize,   // size of template's data
                    &pUpdateData[1],  //  update's template data
                    &nUpdateDataSize,  // size of update's template data
                    &ntype,            // type of update's template data
                    u8NeedUpdate); // decide to need to dyn-update

    btl_util_time_diffnow("algo",3);

    for (i = 0; i < fid_size; i++)
        if (pTemplateBuffer[i] != NULL)
            free((void*)pTemplateBuffer[i]);

    if (g_CancelFlag) {
        LOGD("%s, return as cancel",__func__);
        if (pUpdateData)
            free(pUpdateData);
        return -ERR_CANCEL;
    }

    {
        char temp_buf[128];
        int  matchflag = 1;

        if (nDecision == DECISION_MATCH) {
            sprintf(temp_buf,"/data/system/users/0/fpdata/m-ok-%d-%d-%d-%d.bin",abs(nRet),hour,min,sec);
            matchflag = 0;
        } else
            sprintf(temp_buf,"/data/system/users/0/fpdata/m-fail-%d-%d-%d-%d.bin",abs(nRet),hour,min,sec);

        Create_binFile(fpImage.p_image,
                       temp_buf,
                       FINGER_WIDTH,
                       FINGER_HEIGHT,
                       g_fid_index,
                       g_fid_subindex,
                       fpImage.quality,
                       fpImage.condition,
                       fpImage.area,
                       fpImage.contrast,
                       abs(nRet),
                       1,
                       hour,
                       min,
                       sec,
                       matchflag);
    }
    //update fignerprint data
    if (nDecision == DECISION_MATCH) {
        int32_t fid = fid_index[nID];
        if (u8NeedUpdate && nUpdateDataSize > 0) {
            pUpdateData[0] = ntype;
            LOGD("nUpdateDataSize : %d, uid:%d, fid:%d, type:%d",nUpdateDataSize,uid,fid,ntype);
            nUpdateDataSize++;
			#if FINGER_DATA_FILE
			file_delete_fingerprint(fid);
			btl_file_saveTemplate(pUpdateData, nUpdateDataSize, fid);
			#else
            sql_delete_fingerprint(pDBName, uid, fid);
            sql_insert_fingerprint(pDBName, uid, fid, pUpdateData, nUpdateDataSize);
			#endif
        }
    }

    if (pUpdateData)
        free(pUpdateData);

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
    ioctl(g_fd,BL229X_INTERRUPT_MODE);//è¿›å…¥ä¸­æ–­æ¨¡å¼
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
    g_match_quality_score_threshold = 40;
    g_enroll_quality_score_threshold = 45;

	if (btl_hw_get_chip_info(g_fd,&chip_info)  == 0){
		g_contrast_direction = (chip_info.chip_driver_type == 0) ? 1:0;
	}
	LOGD("%s,%d",__func__,chip_info.chip_driver_type);
	
    close(g_fd);
    g_fd = -1;

    LOGD("%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,0x%x",
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
         g_def_gain);

	Btl_SetAlgorithmType(g_algorithm_type);
	Btl_SetAlgorithmID(0x5183);
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
