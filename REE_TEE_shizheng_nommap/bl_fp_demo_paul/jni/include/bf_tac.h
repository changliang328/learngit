#ifndef __BF_TAC_H__
#define __BF_TAC_H__
#include <linux/types.h>
#include <unistd.h>
#include "bf_hal.h"
#include "bf_types.h"

//add by paul
int bf_tac_init(void * config);
int bf_tac_uninit();
int bf_tac_chip_reinit();
int bf_tac_fd_mode(int mode);
int bf_tac_get_intStatus(int *status);
int bf_tac_capture_image(bf_capture_data_t *capdata);
int bf_tac_qualify_image(bf_capture_data_t *capdata);
int bf_tac_read_frame(char *image);
int bf_tac_get_navigation_event(navigation_info *navidata);
int bf_tac_enroll(bf_enroll_data_t *enrolldata);
int bf_tac_end_enroll(bf_enroll_data_t *enrolldata);
int bf_tac_identify(bf_identify_data_t *identifydata);
int bf_tac_new_fid(uint32_t* id);
int bf_tac_delete_fid(uint32_t id);
int bf_tac_update_template_indic(uint32_t indic);
int bf_tac_check_finger_lost(const BF_TAC_Handle_p pHandle, uint32_t* data);

int bf_tac_identify_all(bf_identify_data_t *data);
int bf_tac_capture_image_all(bf_capture_data_t *capdata);
int bf_tac_get_navigation_event_all(navigation_info *navidata);

//from old arch

int bf_tac_get_hw_auth_challenge(const BF_TAC_Handle_p pHandle, uint64_t* challenge);

int bf_tac_get_template_db_id(const BF_TAC_Handle_p pHandle, uint64_t* id);

int bf_tac_get_template_count(const BF_TAC_Handle_p pHandle, uint32_t* count);

int bf_tac_get_indices(const BF_TAC_Handle_p pHandle, uint32_t* indices, uint32_t* index_count);

int bf_tac_get_template_id_from_index(const BF_TAC_Handle_p pHandle, uint32_t index, uint32_t* data);

int bf_tac_delete_template(const BF_TAC_Handle_p pHandle, uint32_t index);

int bf_tac_store_template_db(const BF_TAC_Handle_p pHandle);

int bf_tac_load_user_db(const char* path, uint32_t path_len,uint32_t gid);

int bf_tac_load_global_db(const BF_TAC_Handle_p pHandle, const char* path, uint32_t path_len);
// 设置当前的用户。(后面的操作，数据将存到当前用户路径下)
int bf_tac_set_active_fingerprint_set(const BF_TAC_Handle_p pHandle, int32_t fingerprint_set_key);

//check template version with algo version   return    -1 uncheck   / 0 check ok    /1  unmatch   /2 error
int bf_tac_check_template_version(const BF_TAC_Handle_p pHandle);
int bf_tac_get_time(uint64_t* hat_times);
uint64_t bf_tac_get_auth_token(hw_auth_token_t *auth);
int bf_tac_send_enroll_token(hw_auth_token_t *enroll_token);
int bf_tac_do_mmi_test(mmi_test_params_t *mmi_data);


#endif //INCLUSION_GUARD_BF_TAC
