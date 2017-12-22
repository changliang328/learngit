#include "bf_tac.h"
#include <inttypes.h>
#include "bf_log.h"
#include <endian.h>
#include "bf_algo.h"
#include "bf_types.h"

#include "tee_adapter.h"

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
	int ret = 0;
	int cmd;
	ret = tee_adapter_init_tzapp();
	if(ret)
	{
		tee_adapter_destroy_tzapp();
		return ret;
	}
	if(config != NULL)
	{
		load_config_t *load_config = (load_config_t *)config;
		load_config->tee_cmd = BF_CMD_CORE_INIT;
		tee_adapter_invoke_command(load_config, sizeof(load_config_t));
		ret = load_config->tee_cmd;
	}else
	{
		cmd = BF_CMD_CORE_INIT;
		tee_adapter_invoke_command(&cmd, sizeof(cmd));
		ret = cmd;
	}
	return ret;
}

int bf_tac_uninit()
{
	//bf_core_uninit();
	int cmd;
	int ret = 0;
	cmd = BF_CMD_CORE_UNINIT;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));
	tee_adapter_destroy_tzapp();
	return ret;
}

int bf_tac_chip_reinit()
{
	//bf_core_chip_reinit();
	int cmd;
	int ret = 0;
	cmd = BF_CMD_CHIP_REINIT;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));

	return ret;
}

int bf_tac_fd_mode(int mode)
{
	//return bf_core_fd_mode(mode);
	int ret = 0;
	BF_LOG("+++");
	bf_ca_param1_t cmdparam;
	
	cmdparam.cmd = BF_CMD_FD_MODE;
	cmdparam.param = mode;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));

	return ret;
}

int bf_tac_capture_image(bf_capture_data_t *capdata)
{
	//return bf_core_capture_image(capdata);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)capdata;
	data->cmd = BF_CMD_CAPTURE_IMAGE;
	ret = tee_adapter_invoke_command(data, sizeof(bf_capture_data_t));
	return ret;
}

int bf_tac_qualify_image(bf_capture_data_t *capdata)
{
	//return bf_core_qualify_image(capdata);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)capdata;
	data->cmd = BF_CMD_QUALITY_IMAGE;
	ret = tee_adapter_invoke_command(data, sizeof(bf_capture_data_t));
	return ret;
}

int bf_tac_get_intStatus(int *status)
{
	//return bf_core_get_intStatus(status);
	int cmd;
	int ret = 0;
	cmd = BF_CMD_GET_INT_STATUS;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));

	*status = cmd;
	return ret;
}

int bf_tac_read_frame(char *image)
{
	//return bf_core_read_frame(image);
	int ret = 0;
	bf_ca_image_t *pCaImage = (bf_ca_image_t *)image;
	pCaImage->cmd = BF_CMD_READ_FRAME;
	ret = tee_adapter_invoke_command(pCaImage, sizeof(pCaImage->cmd));
	return ret;
}

int bf_tac_get_navigation_event(navigation_info *navidata)
{
	//return bf_core_get_navigation_event(data);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)navidata;
	data->cmd = BF_CMD_GET_NAVIGATION_EVENT;
	ret = tee_adapter_invoke_command(data, sizeof(navigation_info));
	return ret;
}

int bf_tac_enroll(bf_enroll_data_t *enrolldata)
{
	//return bf_core_enroll(enrolldata);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)enrolldata;
	data->cmd = BF_CMD_ENROLL;
	ret = tee_adapter_invoke_command(data, sizeof(bf_enroll_data_t));
	return ret;
}

bf_tac_identify(bf_identify_data_t *identifydata)
{
	//return bf_core_identify(data);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)identifydata;
	data->cmd = BF_CMD_IDENTIFY;
	ret = tee_adapter_invoke_command(identifydata, sizeof(bf_identify_data_t));
	return ret;
}

bf_tac_identify_all(bf_identify_data_t *identifydata)
{
	//return bf_core_identify(data);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)identifydata;
	data->cmd = BF_CMD_IDENTIFY_ALL;
	ret = tee_adapter_invoke_command(identifydata, sizeof(bf_identify_data_t));
	return ret;
}

int bf_tac_capture_image_all(bf_capture_data_t *capdata)
{
	//return bf_core_capture_image_all(capdata);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)capdata;
	data->cmd = BF_CMD_CAPTURE_IMAGE_ALL;
	ret = tee_adapter_invoke_command(data, sizeof(bf_capture_data_t));
	return ret;
}

int bf_tac_get_navigation_event_all(navigation_info *navidata)
{
	//return bf_core_get_navigation_event_all(data);
	int ret = 0;
	bf_ca_cmd_data_t *data = (bf_ca_cmd_data_t *)navidata;
	data->cmd = BF_CMD_GET_NAVIGATION_EVENT_ALL;
	ret = tee_adapter_invoke_command(data, sizeof(navigation_info));
	return ret;
}

int bf_tac_new_fid(uint32_t* id)
{
	//return bf_core_new_fid(id);
	int cmd;
	int ret = 0;
	cmd = BF_CMD_NEW_FINGERID;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));

	*id = cmd;
	return ret;
}

int bf_tac_delete_fid(uint32_t id)
{
	//return bf_core_delete_fid(id);
	int ret = 0;
	
	bf_ca_param1_t cmdparam;
	
	cmdparam.cmd = BF_CMD_DELETE_FINGERID;
	cmdparam.param = id;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));

	return ret;
}

int bf_tac_update_template_indic(uint32_t indic)
{
	//return bf_core_update_template_indic(indic);
	int ret = 0;
	
	bf_ca_param1_t cmdparam;
	
	cmdparam.cmd = BF_CMD_UPDATE_TEMPLATE_BY_INDIC;
	cmdparam.param = indic;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));

	return ret;
}


//get time from ta
int bf_tac_get_time(uint64_t* hat_times)
{
	int ret = 0;
	bf_ca_cmd_data_t cmdparam;
	cmdparam.cmd=BF_CMD_GET_TIME;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));	
	*hat_times=cmdparam.systime.kernelsystime;	
	return ret;
}


//get auth token from ta
uint64_t bf_tac_get_auth_token(hw_auth_token_t *auth)
{
	int ret = 0;
	bf_ca_cmd_data_t cmdparam;
	cmdparam.cmd=BF_CMD_GET_AUTH_TOKEN;
	memcpy( (void*)&(cmdparam.ca_authtoken_data.hat),(void *)auth,sizeof(hw_auth_token_t));
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));
	memcpy(auth, (void*)&cmdparam.ca_authtoken_data.hat, sizeof(hw_auth_token_t));
	return ret;
}

int bf_tac_send_enroll_token(hw_auth_token_t *enroll_token)
{
        int ret = 0;
	bf_ca_cmd_data_t cmdparam;
	cmdparam.cmd=BF_CMD_SEND_ENROLL_TOKEN;
	memcpy( (void*)&(cmdparam.ca_authtoken_data.hat),(void *)enroll_token,sizeof(hw_auth_token_t));
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));
	memcpy(enroll_token, (void*)&cmdparam.ca_authtoken_data.hat, sizeof(hw_auth_token_t));
	BF_LOG("musktest: bf_tac_send_enroll_token ret = %d",ret );
	return ret;
}


//用空图,或者finger_leave模式判断手指抬起.
int bf_tac_check_finger_lost(const BF_TAC_Handle_p pHandle, uint32_t* data)
{

}

// 设置challenge。识别过程用
int bf_tac_get_hw_auth_challenge(const BF_TAC_Handle_p pHandle, uint64_t* random_challenge)
{
	*random_challenge = get_64bit_rand();
	return 0;
}

//get authenticator_id
int bf_tac_get_template_db_id(const BF_TAC_Handle_p pHandle, uint64_t* id)
{
	*id = htobe64(1);//htobe64(get_64bit_rand());//htobe64(1);
	return 0;
}

//get template count
int bf_tac_get_template_count(const BF_TAC_Handle_p pHandle, uint32_t* count)
{
	//return bf_core_get_template_count(count);
	int cmd;
	int ret = 0;
	cmd = BF_CMD_GET_TEMPLATE_COUNT;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));

	*count = cmd;
	return ret;
}

//indices fingerindex array
int bf_tac_get_indices(const BF_TAC_Handle_p pHandle, uint32_t* indices, uint32_t* index_count)
{
	//return bf_core_get_indices(indices, index_count);
	int ret = 0;
	uint32_t i = 0;
	bf_ca_indics_t mindics = {0};
	mindics.cmd = BF_CMD_GET_TEMPLATE_INDICS;
	
	ret = tee_adapter_invoke_command(&mindics, sizeof(mindics));
	*index_count = mindics.cmd;
	memcpy(indices, mindics.indics, mindics.cmd * sizeof(uint32_t));
	BF_LOG("count = %d",mindics.cmd);
	for(i = 0;i < mindics.cmd; i++)
	{
		BF_LOG("indics[%d]=%d", i, mindics.indics[i]);
	}
	return ret;
}

//某个index,对应的finger_id(跟template_id相同)
int bf_tac_get_template_id_from_index(const BF_TAC_Handle_p pHandle, uint32_t index, uint32_t* data)
{
	//return bf_core_get_template_id_from_index(index, data);
	int ret = 0;
	
	bf_ca_param1_t cmdparam;
	
	cmdparam.cmd = BF_CMD_GET_TEMPLATE_ID_FROM_INDEX;
	cmdparam.param = index;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));
	*data = cmdparam.cmd;

	return ret;
}

//删除某个index的指纹
int bf_tac_delete_template(const BF_TAC_Handle_p pHandle, uint32_t index)
{
	//return bf_core_delete_template(index);
	int ret = 0;
	
	bf_ca_param1_t cmdparam;
	
	cmdparam.cmd = BF_CMD_DELETE_TEMPLATE_BY_INDIC;
	cmdparam.param = index;
	ret = tee_adapter_invoke_command(&cmdparam, sizeof(cmdparam));

	return ret;
}

//保存template db文件
int bf_tac_store_template_db(const BF_TAC_Handle_p pHandle)
{
	//return bf_core_store_template_db();
	int cmd;
	int ret = 0;
	cmd = BF_CMD_STORE_TEMPLATE_DB;
	ret = tee_adapter_invoke_command(&cmd, sizeof(cmd));

	return ret;
}

//加载template_db,初始化template_manager
int bf_tac_load_user_db(const char* path, uint32_t path_len,uint32_t gid)
{
	//return bf_core_load_user_db(path, path_len);
	bf_ca_path_t mCaPath;
	int ret = 0;
	BF_LOG("%s %d",path,path_len);
	mCaPath.cmd = BF_CMD_LOAD_TEMPLATE_DB;
	mCaPath.path_len = path_len;
	mCaPath.gid = gid;
	memcpy(mCaPath.path, path, path_len);
	ret = tee_adapter_invoke_command(&mCaPath, sizeof(mCaPath));

	return ret;
}

int bf_tac_load_global_db(const BF_TAC_Handle_p pHandle, const char* path, uint32_t path_len)
{
}

//使用某个用户的,指纹列表
int bf_tac_set_active_fingerprint_set(const BF_TAC_Handle_p pHandle, int32_t fingerprint_set_key)
{
	//return bf_core_set_active_fingerprint_set(fingerprint_set_key);
	
}

//check template version with algo version   return    -1 uncheck   / 0 check ok    /1  unmatch   /2 error
int bf_tac_check_template_version(const BF_TAC_Handle_p pHandle)
{

}

int bf_tac_do_mmi_test(mmi_test_params_t *mmi_data)
{
	int ret = 0;
	mmi_data->cmd = BF_CMD_DO_MMI_TEST;
	ret = tee_adapter_invoke_command(mmi_data, sizeof(mmi_test_params_t) + mmi_data->length);
	BF_LOG("length = %d",mmi_data->length);
	return ret;
}

int bf_tac_get_finger_data(bf_ca_app_data_t *fpdata)
{
	int ret = 0;
	fpdata->cmd = BF_CMD_GET_FINGER_DATA;
	ret = tee_adapter_invoke_command(fpdata, sizeof(bf_ca_app_data_t));
}
