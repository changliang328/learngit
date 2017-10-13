#ifndef _BTL_BMP_H_
#define _BTL_BMP_H_

#include "btlcustom.h"

#pragma pack(push, 1)


typedef struct tagBITMAPFILEHEADER {

    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO;


typedef struct tagBITMAP {
    BITMAPFILEHEADER bfHeader;
    BITMAPINFO biInfo;
} BITMAPFILE;

typedef struct _LI_RGB {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} LI_RGB;

//生成BMP图片(无颜色表的位图):在RGB(A)位图数据的基础上加上文件信息头和位图信息头
int GenBmpFile(uint8_t *pData, uint8_t bitCountPerPix, uint32_t width, uint32_t height, const char *filename);
//获取BMP文件的位图数据(无颜色表的位图):丢掉BMP文件的文件信息头和位图信息头，获取其RGB(A)位图数据
uint8_t* GetBmpData(uint8_t *bitCountPerPix, uint32_t *width, uint32_t *height, const char* filename);
//释放GetBmpData分配的空间
void FreeBmpData(uint8_t *pdata);
//设置图像像素点值
void setpixcol(unsigned char* col,char* name);

void setpixcolram(unsigned char* col,uint8_t *pbmp);
#endif
