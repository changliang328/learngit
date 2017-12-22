#ifndef __BF_IMAGE_INFO_H__
#define __BF_IMAGE_INFO_H__
#include "bf_custom.h"
typedef struct bf_image{
	u8 *fpdata;
	u8 *fpdata_enhance;
	u32 width;
	u32 height;
	u32 uPosX;
	u32 uPosY;
	u32 uMeanValue;
	u32 uAreaCount;
	u32 uXuArea;
	u32 uPBscore;
	u32 uPBarea;
	u32 uPBcondition;
	u32 udacp;
	u32 gain;
	u32 hasExtracted; //tCurTemplate has extracted from image
}bf_image_t;
bf_image_t *bf_image_new(u32 width,u32 height);
int bf_image_destroy(bf_image_t *pbfimage);
#endif
