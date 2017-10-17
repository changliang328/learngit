#include "bf_image_info.h"

bf_image_t *bf_image_new(u32 width,u32 height)
{
	bf_image_t *pbfimage = malloc(sizeof(bf_image_t));
	pbfimage->width = width;
	pbfimage->height = height;
	pbfimage->fpdata = malloc(width * height);
	pbfimage->fpdata_enhance = malloc(width * height);
	return pbfimage;
}

int bf_image_destroy(bf_image_t *pbfimage)
{
	free(pbfimage->fpdata);
	free(pbfimage->fpdata_enhance);
	free(pbfimage);
	return 0;
}
