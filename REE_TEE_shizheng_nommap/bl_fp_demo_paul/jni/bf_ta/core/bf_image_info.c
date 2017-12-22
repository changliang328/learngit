#include "bf_image_info.h"

bf_image_t *bf_image_new(u32 width,u32 height)
{
	bf_image_t *pbfimage = bf_tee_malloc(sizeof(bf_image_t));//malloc(sizeof(bf_image_t));
	//add by jaston for  check null pointer
	if(pbfimage == NULL)
		{
		//exit(EXIT_FAILURE);
		return -1;
	}
	//add end
	pbfimage->width = width;
	pbfimage->height = height;
	pbfimage->fpdata = bf_tee_malloc(width * height);
	//add by jaston for  check null pointer
	if(pbfimage->fpdata == NULL)
		{
		bf_tee_free(pbfimage);//free(pbfimage);
		return -1;//exit(EXIT_FAILURE);
	}
	//add end
	pbfimage->fpdata_enhance = bf_tee_malloc(width * height);
	
	//add by jaston for  check null pointer
	if(pbfimage->fpdata_enhance == NULL)
	{
		bf_tee_free(pbfimage->fpdata);//free(pbfimage->fpdata);
		bf_tee_free(pbfimage);//free(pbfimage);
		return -1;//exit(EXIT_FAILURE);
	}
	//add end
	return pbfimage;
}

int bf_image_destroy(bf_image_t *pbfimage)
{
	bf_tee_free(pbfimage->fpdata);//free(pbfimage->fpdata);
	bf_tee_free(pbfimage->fpdata_enhance);//free(pbfimage->fpdata_enhance);
	bf_tee_free(pbfimage);//free(pbfimage);
	return 0;
}
