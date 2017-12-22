#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include "bf_bmp.h"
int save_image_bmp(unsigned char* col,char* name, int width, int height)
{
	BITMAPFILEHEADER bmpfileHead;	//BMP文件头
	BITMAPINFOHEADER bmpinfoHead;	//BMP信息头
	RGBQUAD bmpfilePallete[256];	//RGB调色板,256色
	uint32_t BNImgW;
	FILE *fp;
	int i;
	uint32_t foffset;

	//打开保存的图像文件
	if (NULL == (fp = fopen(name, "wb")))
		return -1;

	bf_tee_memset(&bmpfileHead, 0, sizeof(BITMAPFILEHEADER));
	bf_tee_memset(&bmpinfoHead, 0, sizeof(BITMAPINFOHEADER));
	bf_tee_memset(bmpfilePallete, 0, sizeof(RGBQUAD) * 256);
	//信息头
	bmpinfoHead.biSize = 40;		//该结构的大小固定为40字节
	bmpinfoHead.biWidth = width;	//图像宽度
	bmpinfoHead.biHeight = height;	//图像高度
	BNImgW = (width + 3) / 4 * 4;	//计算图像数据占用字节数时，图像宽度必须是4的整数倍
	bmpinfoHead.biSizeImage = BNImgW * height;	//图像大小
	bmpinfoHead.biPlanes = 1;
	bmpinfoHead.biBitCount = 8;

	//调色板
	for (i = 0; i < 256; i++)
		bmpfilePallete[i].rgbBlue = bmpfilePallete[i].rgbGreen =
				bmpfilePallete[i].rgbRed = (uint8_t) i;

	//文件头
	bmpfileHead.bfType = 0x4D42;	//"BM"
	bmpfileHead.bfOffBits = 1078; //sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;		//从文件头到实际图像数据的偏移字节数，三个结构的大小。
	bmpfileHead.bfSize = bmpfileHead.bfOffBits + bmpinfoHead.biSizeImage; //位图文件大小

	//写入文件
	//第一部分：文件头
	fwrite((uint8_t*) &bmpfileHead, sizeof(BITMAPFILEHEADER), 1, fp);
	foffset = sizeof(BITMAPFILEHEADER);

	//第二部分：信息头
	fseek(fp, foffset, SEEK_SET);
	fwrite((uint8_t*) &bmpinfoHead, sizeof(BITMAPINFOHEADER), 1, fp);
	foffset += sizeof(BITMAPINFOHEADER);

	//第三部分：调色板
	fseek(fp, foffset, SEEK_SET);
	fwrite(bmpfilePallete, sizeof(RGBQUAD), 256, fp);
	foffset += sizeof(RGBQUAD) * 256;

	//第四部分：图像数据
	for (i = 0; i < height; i++) {
		fseek(fp, foffset, SEEK_SET);
		fwrite(col + (height - 1 - i) * width, sizeof(uint8_t), BNImgW, fp);//图像数据从下到上，从从左到右写入文件中
		foffset += BNImgW;
	}
	fclose(fp);
	
	return 0;
}

