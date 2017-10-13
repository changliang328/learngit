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

#include "btlcustom.h"
#include "btlfp.h"
#include "btl_algorithm_interface.h"
#include "btlbmp.h"

#define caparam1				       65536
#define caparam2				       13
#define RIGHT_SHIFTBIT(val,nbits)      (val>>nbits)
#define caparam3			           39567
#define caparam4		               65536

#define BL229X                         0
#define BL239X                         1

extern uint8_t g_width;
extern uint8_t g_height;

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

    for (i=0; i < size; i++) {
        if (pImage[i] <  value_threshold)
            imagepix_content++;
        else
            imagepix_empty++;

        if (imagepix_content >= counts_threshold)
            return 0;
    }
    LOGD("%s,%d,%d",__func__,imagepix_empty,imagepix_content);
    return -1;
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

