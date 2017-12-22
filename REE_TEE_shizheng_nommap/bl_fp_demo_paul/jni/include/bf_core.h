#ifndef __BF_CORE_H__
#define __BF_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

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
	u32 curEnrollIndic;
}bf_core_t;

int bf_core_init(void * config);
int bf_core_uninit(void);
int bf_core_chip_reinit(void);
int bf_core_fd_mode(int mode);
int bf_core_capture_image(bf_capture_data_t *capdata);
int bf_core_qualify_image(bf_capture_data_t *capdata);
int bf_core_get_intStatus(int *status);
int bf_core_read_frame(char *image);
int bf_core_get_navigation_event(navigation_info *data);
int bf_core_enroll(bf_enroll_data_t *enrolldata);

int bf_core_identify(bf_identify_data_t *data);
int bf_core_capture_image_all(bf_capture_data_t *capdata);
int bf_core_get_navigation_event_all(navigation_info *navidata);

int bf_core_get_template_count(uint32_t* count);
int bf_core_get_indices(uint32_t* indices, uint32_t* count);
int bf_core_get_template_id_from_index(uint32_t index, uint32_t* data);
int bf_core_delete_template(uint32_t index);
int bf_core_new_fid(uint32_t* id);
int bf_core_delete_fid(uint32_t id);
int bf_core_update_template_indic(uint32_t indic);
int bf_core_load_user_db(const char* path, uint32_t path_len,uint32_t user_id);
int bf_core_set_active_fingerprint_set(int32_t fingerprint_set_key);
int bf_core_store_template_db(void);
int bf_core_handler(uint32_t *buffer, uint32_t *buff_len);
void  bf_core_get_auth_token(hw_auth_token_t *auth_token); 
void bf_core_send_enroll_token(hw_auth_token_t *enroll_token);

#ifdef __cplusplus
}
#endif

#endif
