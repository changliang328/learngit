#ifndef __BF_CORE_H__
#define __BF_CORE_H__
#include "bl_fingerprint.h"
#include "bf_algo.h"
#include "bf_image_info.h"
#include "bf_template.h"

typedef struct bf_core_t{
	struct bl_fingerprint_data *bl_data;
	bf_algo_t *pbf_algo;
	bf_image_t *pbfimage;
	bf_template_manager_t *pbfTemplateMgr;
	u8 store_path[MAX_PATH_LENGTH];
	int32_t fpset_key;
	int32_t dbfd;
	u32 curEnrollIndic;
}bf_core_t;

int bf_core_get_intStatus(int *status);
int bf_core_get_template_count(uint32_t* count);
int bf_core_get_indices(uint32_t* indices, uint32_t* count);
#endif
