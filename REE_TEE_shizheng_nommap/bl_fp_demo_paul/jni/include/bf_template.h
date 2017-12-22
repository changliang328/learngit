#ifndef __BF_TEMPLATE_H__
#define __BF_TEMPLATE_H__
#include "bf_types.h"
#include "bf_algo.h"

#define BF_DB_MAGIC	(0xBFBFFBFB)
#define DB_HEADER_OFFSET	(0)

/*
typedef struct bf_template_t{
	u32 fid;
	BL_TEMPLATE bl_template;
}bf_template_t;
*/

typedef struct bf_template_manager{
	BL_TEMPLATE bl_templates[BF_MAX_FINGER];
	BL_TEMPLATE tCurTemplate;
	u64 user_id;
	u64 authenticator_id;
	u32 fids[BF_MAX_FINGER];
	u32 indices[BF_MAX_FINGER];
	u32 indexcount;
	u32 uMaxTemplateSize;
	u32 magicNumber;
	u32 crcNumber;
	u8 db_path[MAX_PATH_LENGTH];
	u8 bk_db_path[MAX_PATH_LENGTH];
	int32_t dbfd;
	int32_t bkdbfd;
}bf_template_manager_t;
#define DB_DATA_OFFSET	((sizeof(struct bf_template_manager) / 1024 + 1) * 1024)
bf_template_manager_t *bf_template_manager_new(struct bf_algo_data *pbf_algo);
int bf_template_manager_destroy(bf_template_manager_t *pbf_template_mgr);
int bf_template_manager_init_from_dbfile(bf_template_manager_t *pbf_template_mgr);
int bf_template_get_indices(bf_template_manager_t *pbf_template_mgr, uint32_t* indices, uint32_t* count);
int bf_template_manager_store_to_db(bf_template_manager_t *pbf_template_mgr);
int bf_get_all_templates(bf_template_manager_t *pbf_template_mgr, BL_TEMPLATE **pbfAllTemplates);


#endif
