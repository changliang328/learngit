#include "bf_tac.h"
#include <inttypes.h>
#include "bf_log.h"
#include <endian.h>
#include "bf_algo.h"
#include "bf_types.h"

static uint64_t get_64bit_rand(void) 
{
	// This should use a cryptographically-secure random number generator like arc4random().
	// It should be generated inside of the TEE where possible. Here we just use something
	// very simple.
	LOGD("<%s> ^_^", __FUNCTION__);
	uint64_t r = (((uint64_t)rand()) << 32) | ((uint64_t)rand());
	return r != 0 ? r : 1;
}

int bf_tac_init(void * config)
{
	bf_core_init(config);
	free(config);
}

int bf_tac_uninit()
{
	bf_core_uninit();
}

int bf_tac_chip_reinit()
{
	bf_core_chip_reinit();
}

int bf_tac_fd_mode(int mode)
{
	return bf_core_fd_mode(mode);
}

int bf_tac_capture_image(bf_capture_data_t *capdata)
{
	return bf_core_capture_image(capdata);
}

int bf_tac_qualify_image(bf_capture_data_t *capdata)
{
	return bf_core_qualify_image(capdata);
}

int bf_tac_get_intStatus(int *status)
{
	return bf_core_get_intStatus(status);
}

int bf_tac_read_frame(char *image)
{
	return bf_core_read_frame(image);
}

int bf_tac_get_navigation_event(navigation_info *data)
{
	return bf_core_get_navigation_event(data);
}

int bf_tac_enroll(bf_enroll_data_t *enrolldata)
{
	return bf_core_enroll(enrolldata);
}

int bf_tac_identify(bf_identify_data_t *data)
{
	return bf_core_identify(data);
}

int bf_tac_identify_all(bf_identify_data_t *data)
{
	return bf_core_identify_all(data);
}

int bf_tac_capture_image_all(bf_capture_data_t *capdata)
{
	return bf_core_capture_image_all(capdata);
}

int bf_tac_get_navigation_event_all(navigation_info *data)
{
	return bf_core_get_navigation_event_all(data);
}

int bf_tac_new_fid(uint32_t* id)
{
	return bf_core_new_fid(id);
}

int bf_tac_delete_fid(uint32_t id)
{
	return bf_core_delete_fid(id);
}

int bf_tac_update_template_indic(uint32_t indic)
{
	return bf_core_update_template_indic(indic);
}

//用空图,或者finger_leave模式判断手指抬起.
int bf_tac_check_finger_lost(const BF_TAC_Handle_p pHandle, uint32_t* data)
{

}

// 设置challenge。识别过程用
int bf_tac_get_hw_auth_challenge(const BF_TAC_Handle_p pHandle, uint64_t* challenge)
{
	*challenge = get_64bit_rand();
	return 0;
}

//get authenticator_id
int bf_tac_get_template_db_id(const BF_TAC_Handle_p pHandle, uint64_t* id)
{
	*id = htobe64(1);;
	return 0;
}

//get template count
int bf_tac_get_template_count(const BF_TAC_Handle_p pHandle, uint32_t* count)
{
	return bf_core_get_template_count(count);
}

//indices fingerindex array
int bf_tac_get_indices(const BF_TAC_Handle_p pHandle, uint32_t* indices, uint32_t* index_count)
{
	return bf_core_get_indices(indices, index_count);
}

//某个index,对应的finger_id(跟template_id相同)
int bf_tac_get_template_id_from_index(const BF_TAC_Handle_p pHandle, uint32_t index, uint32_t* data)
{
	return bf_core_get_template_id_from_index(index, data);
}

//删除某个index的指纹
int bf_tac_delete_template(const BF_TAC_Handle_p pHandle, uint32_t index)
{
	return bf_core_delete_template(index);
}

//保存template db文件
int bf_tac_store_template_db(const BF_TAC_Handle_p pHandle)
{
	return bf_core_store_template_db();
}

//加载template_db,初始化template_manager
int bf_tac_load_user_db(const char* path, uint32_t path_len,uint32_t gid)
{
	return bf_core_load_user_db(path, path_len, gid);
}

int bf_tac_load_global_db(const BF_TAC_Handle_p pHandle, const char* path, uint32_t path_len)
{
}

//使用某个用户的,指纹列表
int bf_tac_set_active_fingerprint_set(const BF_TAC_Handle_p pHandle, int32_t fingerprint_set_key)
{
	return bf_core_set_active_fingerprint_set(fingerprint_set_key);
}

//check template version with algo version   return    -1 uncheck   / 0 check ok    /1  unmatch   /2 error
int bf_tac_check_template_version(const BF_TAC_Handle_p pHandle)
{

}
