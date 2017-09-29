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
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include "btlfp.h"
#include "btl_algorithm_interface.h"
#include "btlbmp.h"
#include "btlcustom.h"

#define caparam1				       65536
#define caparam2				       13
#define RIGHT_SHIFTBIT(val,nbits)      (val>>nbits)
#define caparam3			           39567
#define caparam4		               65536

#define BL229X                         0
#define BL239X                         1

extern uint8_t g_width;
extern uint8_t g_height;

void bubble_sort (int buf[], int n)
{
    int i, j, temp;
    for (j = 0; j < n - 1; j++)
        for (i = 0; i < n - 1 - j; i++)
            if(buf[i] > buf[i + 1]) {
                temp   = buf[i];
                buf[i] = buf[i+1];
                buf[i+1] = temp;
            }
}

void  btl_preprocess_histNormolize(unsigned char *pImg, unsigned char *pNormImg,int width,int height)
{
    int    hist[256];
    float  fpHist[256];
    float  eqHistTemp[256];
    int    eqHist[256];
    int size = height *width;
    int i ,j;

    memset(&hist,0x00,sizeof(int)*256);
    memset(&fpHist,0x00,sizeof(float)*256);
    memset(&eqHistTemp,0x00,sizeof(float)*256);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            unsigned char GrayIndex = pImg[i*width+j];
            hist[GrayIndex] ++ ;
        }
    }
    for (i = 0; i< 256; i++) {
        fpHist[i] = (float)hist[i]/(float)size;
    }
    for ( i = 1; i< 256; i++) {
        if (i == 0) {
            eqHistTemp[i] = fpHist[i];
        } else {
            eqHistTemp[i] = eqHistTemp[i-1] + fpHist[i];
        }
    }

    for (i = 0; i< 256; i++) {
        eqHist[i] = (int)(255.0 * eqHistTemp[i] + 0.5);
    }
    for (i = 0; i < height; i++) { //进行灰度映射 均衡化
        for (j = 0; j < width; j++) {
            unsigned char GrayIndex = pImg[i*width+j];
            pNormImg[i*width+j] = eqHist[GrayIndex];
        }
    }
}


/**************************************************************************************/

/**************************************************************************************/
int enhance_contrast(unsigned char* inData,int width,int height)
{
    int pixel[9]= {0};
    unsigned char mid;
    unsigned char temp;
    int flag;
    int m,i,j,x,h,w,y;
    int s,z,c = 0;
    int sum = 0;
    int average;
    int delta = 0;
    int val = 0;
    int nPercent = 5;

    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++) {
            if (inData[i*width+j] != 255) {
                sum += inData[i*width+j];
                c++;
            }
        }
    average = sum/c;
    LOGD("%s,%d",__func__,average);
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++) {
            val = inData[i*width+j];
            if (val != 255) {
                val  += (val - average) * nPercent;
                if(val>255) val=255;
                if(val<0) val=0;
                inData[i*width+j] = (unsigned char)val;
            }
        }
    return 0;
}

int btl_preprocess_middleFilter(unsigned char* inData,int width,int height,unsigned char* outData)
{
    int pixel[25]= {0}; //\u6ed1\u52a8\u7a97\u53e3\u7684\u50cf\u7d20\u503c\uff0c\u521d\u59cb\u4e3a0
    unsigned char mid = 0;//\u4e2d\u503c
    unsigned char temp = 0;//\u4e2d\u95f4\u53d8\u91cf
    int flag;
    int m,i,j,x,h,w,y;
    int fmin,fmax,fmed,B1,B2;

    LOGD("%s",__func__);

    for(j=2; j<height-2; j++) {
        for(i=2; i<width-2; i++) {
            m=0;
            for(y = j-2; y <= j+2; y++)
                for(x = i-2; x <= i+2; x++) {
                    pixel[m] = inData[y*width+x];
                    m=m+1;
                }
            bubble_sort(pixel,25);
            fmin = pixel[0];
            fmax = pixel[24];
            fmed = pixel[13];
            B1 = inData[width*j+i] - fmin;
            B2 = inData[width*j+i] - fmax;
            if (B1 > 0 && B2 < 0)
                mid = inData[width*j+i];
            else
                mid = fmed;
            //inData[width*j+i] = mid;
            outData[width*j+i]= mid;
        }
    }
    //enhance_contrast(outData,width,height);
    return 0;
}
/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
static int32_t imageCha(uint8_t* temp1,uint8_t* temp2)
{
    int32_t sum=0;
    int32_t count1=0,count2=0;
    int32_t i=0;
    for (i=0; i<g_width * g_height; i++) {
        sum+=((int32_t)temp1[i]-(int32_t)temp2[i]);
    }
    return (sum/(g_width * g_height));
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int64_t intSqrt(int64_t iin)
{
    int64_t i, j, i_neit, j_neit, result;
    int32_t iter, k, pk = 0;
    if (iin == 0) {
        return(0);
    }
    pk = 0;
    while (1) {
        if (iin<caparam1 / 2) {
            iin = iin << 2;
            pk++;
        } else if (iin>12 * caparam1) {
            iin = iin >> 2;
            pk--;
        } else {
            break;
        }
    }

    i = iin + caparam1;
    j = iin - caparam1;

    k = 4;
    for (iter = 1; iter <= caparam2; iter++) {
        if (j < 0) {
            i_neit = i + RIGHT_SHIFTBIT(j, iter);
            j_neit = j + RIGHT_SHIFTBIT(i, iter);
        } else {
            i_neit = i - RIGHT_SHIFTBIT(j, iter);
            j_neit = j - RIGHT_SHIFTBIT(i, iter);
        }
        i = i_neit;
        j = j_neit;
        if (iter == k) {
            if (j < 0) {
                i_neit = i + RIGHT_SHIFTBIT(j, iter);
                j_neit = j + RIGHT_SHIFTBIT(i, iter);
            } else {
                i_neit = i - RIGHT_SHIFTBIT(j, iter);
                j_neit = j - RIGHT_SHIFTBIT(i, iter);
            }
            k = 3 * k + 1;
            i = i_neit;
            j = j_neit;
        }
    }
    result = i*caparam3 / caparam4;

    if (pk>0) {
        result = result >> pk;//result=result/(double)(2^pk);
    } else if (pk<0) {
        result = result << (-pk);//result=result*(double)(2^(-pk));
    }

    return(result);
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_preprocess_imageCheckEmpty(uint8_t* pImage, uint8_t value_threshold, int32_t counts_threshold, int size)
{
    int32_t imagepix_empty  = 0;
    int32_t imagepix_content = 0;
    int32_t i=0;

    if (counts_threshold > size)
        counts_threshold = size;
	LOGD("value_threshold: 0x%x", value_threshold);
//	LOGD("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", pImage[1000], pImage[2000], pImage[3000], pImage[4000], pImage[5000], pImage[6000], pImage[7000], pImage[8000]);
    for (i=0; i < size; i++) {
        if (pImage[i] <  value_threshold)
            imagepix_content++;
        else
            imagepix_empty++;

        if (imagepix_content >= counts_threshold)
        {
			LOGD("%s,%d,%d",__func__,imagepix_empty,imagepix_content);
            return 0;
		}

    }
    LOGD("%s,%d,%d",__func__,imagepix_empty,imagepix_content);
    return -1;
}

int  btl_remove_imageBase(unsigned char *p_input_image,
                      unsigned char* p_output_image, 
                      int image_width, 
                      int image_height,
                      int meanValue)
{
    int32_t i=0;
    int32_t sum = 0;
    int32_t size = image_width*image_height;
    for (i=0; i < size; i++) {
        sum = p_output_image[i] - p_input_image[i] + meanValue;
        if(sum > 255)
        	p_output_image[i] = 255;
        else if(sum < 0)
            p_output_image[i] = 0;
        else
        	p_output_image[i] = sum;
    }
    return 0;
}

/*----------------------------------------------------------------------
purpose:
return :
------------------------------------------------------------------------*/
int32_t btl_preprocess_computeImgMeanVar(unsigned char* gray,
        int32_t nImgHeight,
        int32_t nImgWidth,
        int32_t* nIsDry,
        int32_t* nIntensity,
        int32_t* nVar)
{
    if (gray == NULL) {
        *nIsDry = 0;
        *nIntensity = 0;
        return 0;
    }
    int32_t nImgMeanVal;
    int32_t nImgVar;
    unsigned char lp;
    int32_t i=0,j=0,k=0,l=0;
    lp = 0;
    nImgMeanVal = 0;
    nImgVar = 0;
    int32_t nPatchSize = 16;
    int32_t nPatchArea = nPatchSize*nPatchSize;
    int32_t nValidPatchCt = 0;
    for (i = 0; i < nImgHeight; i+=nPatchSize) {
        for ( j = 0; j < nImgWidth; j += nPatchSize) {
            int32_t nLocalMeanVal = 0;
            int32_t nLocalVar = 0;
            for ( k = i; k < i + nPatchSize; k++) {
                for ( l = j; l < j + nPatchSize; l++) {
                    lp = *(gray + k*nImgWidth + l);
                    nLocalMeanVal += (int32_t)lp;
                }
            }
            nLocalMeanVal = nLocalMeanVal / nPatchArea;
            if ((nLocalMeanVal < 250)) {
                for ( k = i; k < i + nPatchSize; k++) {
                    for ( l = j; l < j + nPatchSize; l++) {
                        lp = *(gray + k*nImgWidth + l);
                        nLocalVar += ((int32_t)lp - nLocalMeanVal)*((int32_t)lp - nLocalMeanVal);
                    }
                }
                nLocalVar = nLocalVar / nPatchArea;
                nLocalVar = intSqrt(nLocalVar);
                nLocalVar = nLocalVar / 256;

                nImgMeanVal += nLocalMeanVal;
                nImgVar += nLocalVar;
                nValidPatchCt++;
            }
        }
    }

    if (nValidPatchCt == 0) {
        *nVar = 0;
        *nIsDry = 1;
        *nIntensity = 12;
        return 0;
    }

    nImgMeanVal = nImgMeanVal / nValidPatchCt;
    nImgVar = nImgVar / nValidPatchCt;
	LOGD("nImgMeanVal=%d",nImgMeanVal);
    if (nImgMeanVal > 140)
        * nIsDry = 1;
    else
        *nIsDry = 0;
    *nVar = nImgVar;
    *nIntensity = 12 - nImgVar/ 9;
    return nImgMeanVal;
}

/*----------------------------------------------------------------------
purpose:
return :

------------------------------------------------------------------------*/
int32_t btl_cal_finerMean(unsigned char* gray,
        int32_t nImgWidth,
        int32_t nImgHeight)
{
    if (gray == NULL) {
        return 0;
    }
    int32_t nImgMeanVal;
    unsigned char lp;
    int32_t i=0,j=0,k=0,l=0;
    lp = 0;
    nImgMeanVal = 0;
    int32_t nPatchSize = 16;
    int32_t nPatchArea = nPatchSize*nPatchSize;
    int32_t nValidPatchCt = 0;
    for (i = 0; i < nImgHeight; i+=nPatchSize) {
        for ( j = 0; j < nImgWidth; j += nPatchSize) {
            int32_t nLocalMeanVal = 0;
            for ( k = i; k < i + nPatchSize; k++) {
                for ( l = j; l < j + nPatchSize; l++) {
                    lp = *(gray + k*nImgWidth + l);
                    nLocalMeanVal += (int32_t)lp;
                }
            }
            nLocalMeanVal = nLocalMeanVal / nPatchArea;
            if ((nLocalMeanVal < 250)) {
                nImgMeanVal += nLocalMeanVal;
                nValidPatchCt++;
            }
        }
    }

    if (nValidPatchCt == 0) {
        return 0;
    }

    nImgMeanVal = nImgMeanVal / nValidPatchCt;
	LOGD("nImgMeanVal=%d",nImgMeanVal);
    return nImgMeanVal;
}

int32_t btl_cal_baseMean(unsigned char* gray,
        int32_t nImgHeight,
        int32_t nImgWidth,
        int32_t startX,
        int32_t startY)
{
    int32_t nImgMeanVal;
    unsigned char lp;
    int32_t i=0,j=0;
    int32_t nLocalMeanVal = 0;
    int32_t nValidPatchCt = 0;
    lp = 0;
    nImgMeanVal = 0;

    for (i = startY; i < nImgHeight - startY; i++) {
        for ( j = startX; j < nImgWidth - startX; j++) {
            lp = *(gray + i*nImgWidth + j);
            nLocalMeanVal += (int32_t)lp;
            nValidPatchCt++;
        }
    }
	nImgMeanVal = nLocalMeanVal/nValidPatchCt;
    return nImgMeanVal;
}

