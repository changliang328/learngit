#ifndef __BF_TEMPLATE_H__
#define __BF_TEMPLATE_H__
#include "bf_types.h"
#include "bf_algo.h"

#define BF_DB_MAGIC	(0xBFBFFBFB)
#define DB_HEADER_OFFSET	(0)
#define DB_DATA_OFFSET	(512)
/*
typedef struct bf_template_t{
	u32 fid;
	BL_TEMPLATE bl_template;
}bf_template_t;
*/

typedef struct bf_template_manager{
	BL_TEMPLATE bl_templates[BF_MAX_FINGER];
	BL_TEMPLATE tCurTemplate;
	u32 fids[BF_MAX_FINGER];
	u32 indices[BF_MAX_FINGER];
	u32 indexcount;
	u32 uMaxTemplateSize;
	u32 magicNumber;
}bf_template_manager_t;

bf_template_manager_t *bf_template_manager_new(struct bf_algo_data *pbf_algo);
#endif
