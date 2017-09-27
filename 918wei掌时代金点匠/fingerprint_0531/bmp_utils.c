/*
 * Copyright (C) 2016 BetterLife Corporation. All rights reserved.
 *
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <android/log.h>
#include "btlcustom.h"

#pragma pack(push, 1)

#define TAG "btl_algo"

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , TAG,__VA_ARGS__)


typedef struct tagBITMAPFILEHEADER {

    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    unsigned int biSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[256];
} BITMAPINFO;


typedef struct tagBITMAP {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFO biInfo;
} BITMAPFILE;

typedef struct _BMP_RGB {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} BMP_RGB;


int Create_256bmp(
	unsigned char* pData, 
	int width, 
	int height,
	unsigned char* pBMP, 
	const char* pFileName)
{
    unsigned char* pBuffer = pBMP;
    unsigned char* p = 0;
    const unsigned char bitCountPerPix = 8;
    const unsigned int bmppitch = ((width*bitCountPerPix + 31) >> 5) << 2;
    const unsigned int filesize = bmppitch*height;
    int  size = 0;
    int i = 0;
    unsigned char* pEachLinBuf = 0;
    unsigned char BytePerPix = 1;
    unsigned int pitch = width * BytePerPix;
    BITMAPFILE bmpfile;
    unsigned char w_file = 0;
    FILE* fp = 0;

    LOGD("%s",__func__);

    if (pBuffer == 0) {
        pBuffer = (unsigned char *)malloc(30*1024);
        w_file = 1;
        memset(pBuffer,0,30*1024);
    }
    p = pBuffer;
    bmpfile.bfHeader.bfType = 0x4D42;
    bmpfile.bfHeader.bfSize = filesize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    bmpfile.bfHeader.bfReserved1 = 0;
    bmpfile.bfHeader.bfReserved2 = 0;
    bmpfile.bfHeader.bfOffBits = sizeof(BITMAPINFO) + sizeof(BITMAPFILEHEADER);

    bmpfile.biInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpfile.biInfo.bmiHeader.biWidth = width;
    bmpfile.biInfo.bmiHeader.biHeight = height;
    bmpfile.biInfo.bmiHeader.biPlanes = 1;
    bmpfile.biInfo.bmiHeader.biBitCount = bitCountPerPix;
    bmpfile.biInfo.bmiHeader.biCompression = 0;
    bmpfile.biInfo.bmiHeader.biSizeImage = 0;
    bmpfile.biInfo.bmiHeader.biXPelsPerMeter = 0;
    bmpfile.biInfo.bmiHeader.biYPelsPerMeter = 0;
    bmpfile.biInfo.bmiHeader.biClrUsed = 0;
    bmpfile.biInfo.bmiHeader.biClrImportant = 0;

    for (i = 0; i < 256; i++) {
        bmpfile.biInfo.bmiColors[i].rgbBlue = i;
        bmpfile.biInfo.bmiColors[i].rgbGreen =  i;
        bmpfile.biInfo.bmiColors[i].rgbRed =  i;
        bmpfile.biInfo.bmiColors[i].rgbReserved = 0;
    }

    memcpy(p,&(bmpfile.bfHeader), sizeof(BITMAPFILEHEADER));
    p += sizeof(BITMAPFILEHEADER);
    size += sizeof(BITMAPFILEHEADER);

    memcpy(p,&(bmpfile.biInfo), sizeof(BITMAPINFO));
    p += sizeof(BITMAPINFO);
    size += sizeof(BITMAPINFO);
    pEachLinBuf = (unsigned char*)malloc(bmppitch);
    if(pEachLinBuf) {
        int h;
        int w;
        for(h = height-1; h >= 0; h--) {
            for(w = 0; w < width; w++) {
                //copy by a pixel
                pEachLinBuf[w*BytePerPix] = pData[h*pitch + w*BytePerPix];
            }
            memcpy(p,pEachLinBuf,bmppitch);
            p += bmppitch;
            size += bmppitch;
        }
        free(pEachLinBuf);
    }
    if (w_file) {
        fp = fopen(pFileName, "wb");
        if(!fp) {
            free(pBuffer);
            return 0;
        }
        fwrite(pBuffer,size,1,fp);
        fclose(fp);
        free(pBuffer);
    }
    LOGD("%s, %d",__func__,size);
    return 1;
}

int Create_binFile(
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
)
{
    unsigned char* pBuffer = 0;
    unsigned char* p = 0;
    const unsigned char bitCountPerPix = 8;
    const unsigned int bmppitch = ((width*bitCountPerPix + 31) >> 5) << 2;
    const unsigned int filesize = bmppitch*height;
    int  size = 0;
    int i = 0;
    unsigned char* pEachLinBuf = 0;
    unsigned char BytePerPix = 1;
    unsigned int pitch = width * BytePerPix;
    BITMAPFILE bmpfile;
    unsigned char w_file = 0;
    FILE* fp = 0;
    const char magic1='b';
    const char magic2='t';
    const char magic3='l';


    LOGD("%s",__func__);

    if (pBuffer == 0) {
        pBuffer = (unsigned char *)malloc(30*1024);
        w_file = 1;
        memset(pBuffer,0,30*1024);
    }
    p = pBuffer;
    bmpfile.bfHeader.bfType = 0x4D42;
    bmpfile.bfHeader.bfSize = filesize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    bmpfile.bfHeader.bfReserved1 = 0;
    bmpfile.bfHeader.bfReserved2 = 0;
    bmpfile.bfHeader.bfOffBits = sizeof(BITMAPINFO) + sizeof(BITMAPFILEHEADER);

    bmpfile.biInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpfile.biInfo.bmiHeader.biWidth = width;
    bmpfile.biInfo.bmiHeader.biHeight = height;
    bmpfile.biInfo.bmiHeader.biPlanes = 1;
    bmpfile.biInfo.bmiHeader.biBitCount = bitCountPerPix;
    bmpfile.biInfo.bmiHeader.biCompression = 0;
    bmpfile.biInfo.bmiHeader.biSizeImage = 0;
    bmpfile.biInfo.bmiHeader.biXPelsPerMeter = 0;
    bmpfile.biInfo.bmiHeader.biYPelsPerMeter = 0;
    bmpfile.biInfo.bmiHeader.biClrUsed = 0;
    bmpfile.biInfo.bmiHeader.biClrImportant = 0;

    for (i = 0; i < 256; i++) {
        bmpfile.biInfo.bmiColors[i].rgbBlue = i;
        bmpfile.biInfo.bmiColors[i].rgbGreen =  i;
        bmpfile.biInfo.bmiColors[i].rgbRed =  i;
        bmpfile.biInfo.bmiColors[i].rgbReserved = 0;
    }

    memcpy(p,&(bmpfile.bfHeader), sizeof(BITMAPFILEHEADER));
    p += sizeof(BITMAPFILEHEADER);
    size += sizeof(BITMAPFILEHEADER);

    memcpy(p,&(bmpfile.biInfo), sizeof(BITMAPINFO));
    p += sizeof(BITMAPINFO);
    size += sizeof(BITMAPINFO);
    pEachLinBuf = (unsigned char*)malloc(bmppitch);
    if(pEachLinBuf) {
        int h;
        int w;
        for(h = height-1; h >= 0; h--) {
            for(w = 0; w < width; w++) {
                //copy by a pixel
                pEachLinBuf[w*BytePerPix] = pData[h*pitch + w*BytePerPix];
            }
            memcpy(p,pEachLinBuf,bmppitch);
            p += bmppitch;
            size += bmppitch;
        }
        free(pEachLinBuf);
    }

    fp = fopen(pFileName, "wb");
    if(!fp) {
        free(pBuffer);
        return 0;
    }
    fwrite(&magic1,sizeof(char),1,fp);
    fwrite(&magic2,sizeof(char),1,fp);
    fwrite(&magic3,sizeof(char),1,fp);
    fwrite(pBuffer,size,1,fp);
    free(pBuffer);

    fwrite(&context,sizeof(int),1,fp);
    fwrite(&fid,sizeof(int),1,fp);
    fwrite(&fid_subindex,sizeof(int),1,fp);
    fwrite(&qual,sizeof(int),1,fp);
    fwrite(&condition,sizeof(int),1,fp);
    fwrite(&area,sizeof(int),1,fp);
    fwrite(&result,sizeof(int),1,fp);
    fwrite(&hour,sizeof(int),1,fp);
    fwrite(&min,sizeof(int),1,fp);
    fwrite(&sec,sizeof(int),1,fp);
	fwrite(&matchflag,sizeof(int),1,fp);
	fwrite(&contrast,sizeof(int),1,fp);
    fclose(fp);
    return 1;
}



