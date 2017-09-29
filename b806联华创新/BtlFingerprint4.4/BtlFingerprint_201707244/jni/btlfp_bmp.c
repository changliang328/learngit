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
#include "btlbmp.h"

#include "btlfp.h"

extern uint8_t g_width;
extern uint8_t g_height;



//生成BMP图片(无颜色表的位图):在RGB(A)位图数据的基础上加上文件信息头和位图信息头
int GenBmp(uint8_t *pData, uint8_t bitCountPerPix, uint32_t width, uint32_t height, uint8_t *pBMP)
{
    uint8_t *ptempBmp = pBMP;
    uint32_t bmppitch = ((width*bitCountPerPix + 31) >> 5) << 2;
    uint32_t filesize = bmppitch*height;

    BITMAPFILE bmpfile;
    uint32_t  size = 0;

    bmpfile.bfHeader.bfType = 0x4D42;
    bmpfile.bfHeader.bfSize = filesize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpfile.bfHeader.bfReserved1 = 0;
    bmpfile.bfHeader.bfReserved2 = 0;
    bmpfile.bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

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

    memcpy(ptempBmp,&(bmpfile.bfHeader), sizeof(BITMAPFILEHEADER));
    ptempBmp += sizeof(BITMAPFILEHEADER);
    size += sizeof(BITMAPFILEHEADER);

    memcpy(ptempBmp,&(bmpfile.biInfo.bmiHeader), sizeof(BITMAPINFOHEADER));
    ptempBmp += sizeof(BITMAPINFOHEADER);
    size += sizeof(BITMAPINFOHEADER);

    uint8_t *pEachLinBuf = (uint8_t*)malloc(bmppitch);
    memset(pEachLinBuf, 0, bmppitch);
    uint8_t BytePerPix = bitCountPerPix >> 3;
    uint32_t pitch = width * BytePerPix;
    if(pEachLinBuf) {
        int h;
        uint32_t w;
        for(h = height-1; h >= 0; h--) {
            for(w = 0; w < width; w++) {
                //copy by a pixel
                pEachLinBuf[w*BytePerPix+0] = pData[h*pitch + w*BytePerPix + 0];
                pEachLinBuf[w*BytePerPix+1] = pData[h*pitch + w*BytePerPix + 1];
                pEachLinBuf[w*BytePerPix+2] = pData[h*pitch + w*BytePerPix + 2];
            }
            memcpy(ptempBmp,pEachLinBuf,bmppitch);
            ptempBmp += bmppitch;
            size += bmppitch;
        }
        free(pEachLinBuf);
    }
    /*
        FILE *fp = fopen(filename, "wb");
        if(!fp)
        {
            printf("fopen failed : %s, %d\n", __FILE__, __LINE__);
            return 0;
        }

        fwrite(pBMP, 1, size, fp);
        fclose(fp);
    */
    return 1;
}


//生成BMP图片(无颜色表的位图):在RGB(A)位图数据的基础上加上文件信息头和位图信息头
int GenBmpFile(uint8_t *pData, uint8_t bitCountPerPix, uint32_t width, uint32_t height, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if(!fp) {
        printf("fopen failed : %s, %d\n", __FILE__, __LINE__);
        return 0;
    }
	LOGD("GenBmpFile 11111");

    uint32_t bmppitch = ((width*bitCountPerPix + 31) >> 5) << 2;
    uint32_t filesize = bmppitch*height;

    BITMAPFILE bmpfile;

    bmpfile.bfHeader.bfType = 0x4D42;
    bmpfile.bfHeader.bfSize = filesize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmpfile.bfHeader.bfReserved1 = 0;
    bmpfile.bfHeader.bfReserved2 = 0;
    bmpfile.bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

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

    fwrite(&(bmpfile.bfHeader), sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&(bmpfile.biInfo.bmiHeader), sizeof(BITMAPINFOHEADER), 1, fp);
	LOGD("GenBmpFile 22222");

    uint8_t *pEachLinBuf = (uint8_t*)malloc(bmppitch);
    memset(pEachLinBuf, 0, bmppitch);
	LOGD("GenBmpFile 33333");
    uint8_t BytePerPix = bitCountPerPix >> 3;
    uint32_t pitch = width * BytePerPix;
    if(pEachLinBuf) {
        int h;
        uint32_t w;
        for(h = height-1; h >= 0; h--) {
            for(w = 0; w < width; w++) {
                //copy by a pixel
                pEachLinBuf[w*BytePerPix+0] = pData[h*pitch + w*BytePerPix + 0];
                pEachLinBuf[w*BytePerPix+1] = pData[h*pitch + w*BytePerPix + 1];
                pEachLinBuf[w*BytePerPix+2] = pData[h*pitch + w*BytePerPix + 2];
            }
            fwrite(pEachLinBuf, bmppitch, 1, fp);
        }
        free(pEachLinBuf);
		LOGD("GenBmpFile 44444");
    }
    fclose(fp);
    return 1;
}



//释放GetBmpData分配的空间
void FreeBmpData(uint8_t *pdata)
{
    if(pdata) {
        free(pdata);
        pdata = NULL;
    }
}

//初始化图像
void setpixcol(unsigned char* col,char* name)
{
    LI_RGB pRGB[g_width][g_height];  // 定义位图数据
    unsigned int count=0;
    memset(pRGB, 0xff, sizeof(pRGB) ); // 设置背景为黑色
	LOGD("setpixcol 11111");
    // 在中间画一个10*10的矩形
    int i=0, j=0;
    for(i = 0; i < g_width; i++) {
        for( j = 0; j < g_height; j++) {
            count++;
            pRGB[i][j].b = col[count];
            pRGB[i][j].g = col[count];
            pRGB[i][j].r = col[count];
        }
    }
	LOGD("setpixcol 22222");
    GenBmpFile((uint8_t*)pRGB, 24, g_width, g_height, name);//生成BMP文件
}


void setpixcolram(unsigned char* col,uint8_t* pbmp)
{
    LI_RGB pRGB[g_width][g_height];  // 定义位图数据
    unsigned int count=0;
    memset(pRGB, 0xff, sizeof(pRGB) ); // 设置背景为黑色

    // 在中间画一个10*10的矩形
    int i=0, j=0;
    for(i = 0; i < g_width; i++) {
        for( j = 0; j < g_height; j++) {
            count++;
            pRGB[i][j].b = col[count];
            pRGB[i][j].g = col[count];
            pRGB[i][j].r = col[count];
        }
    }
    GenBmp((uint8_t*)pRGB, 24, g_width, g_height, pbmp);//生成BMP文件
}
