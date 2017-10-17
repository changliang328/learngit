#ifndef __BF_TAC_H__
#define __BF_TAC_H__
#include <linux/types.h>
#include <unistd.h>
#include "bf_hal.h"

//add by paul
int bf_tac_init(struct bf_fingerprint_hal_device_t *dev);
int bf_tac_fd_mode(int mode);
int bf_tac_get_intStatus(int *status);

//from old arch

int bf_tac_get_hw_auth_challenge(const BF_TAC_Handle_p pHandle, uint64_t* challenge);

int bf_tac_get_template_db_id(const BF_TAC_Handle_p pHandle, uint64_t* id);

int bf_tac_get_template_count(const BF_TAC_Handle_p pHandle, uint32_t* count);

int bf_tac_get_indices(const BF_TAC_Handle_p pHandle, uint32_t* indices, uint32_t* index_count);

int bf_tac_get_template_id_from_index(const BF_TAC_Handle_p pHandle, uint32_t index, uint32_t* data);

int bf_tac_delete_template(const BF_TAC_Handle_p pHandle, uint32_t index);

int bf_tac_store_template_db(const BF_TAC_Handle_p pHandle);

int bf_tac_load_user_db(const BF_TAC_Handle_p pHandle, const char* path, uint32_t path_len);

int bf_tac_load_global_db(const BF_TAC_Handle_p pHandle, const char* path, uint32_t path_len);
// 设置当前的用户。(后面的操作，数据将存到当前用户路径下)
int bf_tac_set_active_fingerprint_set(const BF_TAC_Handle_p pHandle, int32_t fingerprint_set_key);

//check template version with algo version   return    -1 uncheck   / 0 check ok    /1  unmatch   /2 error
int bf_tac_check_template_version(const BF_TAC_Handle_p pHandle);
#endif //INCLUSION_GUARD_BF_TAC
