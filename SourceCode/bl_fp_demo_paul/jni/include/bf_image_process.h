#ifndef __BF_IMAGE_PROCESS_H__
#define __BF_IMAGE_PROCESS_H__
#include "bf_image_info.h"
#include "bf_algo.h"
int bf_cal_mean_value(bf_image_t *pbfimage);
int bf_do_xu_imageEnhance(bf_image_t *pbfimage, bf_algo_t *pbf_algo, u8 minValidArea);
int bf_do_PB_imageQuality(bf_image_t *pbfimage);
#endif
