#include "bf_core.h"
#include "bf_types.h"
#include "bf_image_process.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>    
#include <sys/stat.h>

static bf_core_t *gbf_core = NULL;

int bf_core_init()
{
	BF_LOG("++++");
	bf_core_t *pbf_core = malloc(sizeof(bf_core_t));
	int devfd = fingerprint_open();
	if(devfd < 0)
	{
		BF_LOG("ERROR:%d",errno);
		return devfd;	
	}
	//chip data
	pbf_core->bl_data = bl_fingerprint_data_new(devfd);

	//init algo
	pbf_core->pbf_algo = bf_algo_new(0x5183);
	
	//pbfimage,alloc 2buf ,one for origin,one enhance
	pbf_core->pbfimage = bf_image_new(pbf_core->pbf_algo->width,pbf_core->pbf_algo->height);
	
	//template manager, to load and save templates
	pbf_core->pbfTemplateMgr = bf_template_manager_new(pbf_core->pbf_algo);

	gbf_core = pbf_core;
	return 0;
}

int bf_core_uninit()
{
	BF_LOG("++++");
	fingerprint_close();
	destroy_fingerprint_data(gbf_core->bl_data);
	bf_algo_destroy(gbf_core->pbf_algo);
	bf_image_destroy(gbf_core->pbfimage);
	bf_template_manager_destroy(gbf_core->pbfTemplateMgr);
	free(gbf_core);
}

int bf_core_fd_mode(int mode)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	switch(mode)
	{
		case FINGER_DETECT_DOWN:
			bl_interrupt_init(bl_data);
			break;
		case FINGER_DETECT_DOWN_PART:
			break;
		case FINGER_DETECT_UP:
			break;
	}
	
}

int bf_core_get_intStatus(int *status)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	int value = 0;
	BF_LOG("++++");
	value = bl_spi_read_reg(REGA_INTR_STATUS);
	*status = value;
	return value;
}

static void exchange_white_black(u8 *dst,u8 *src,int len)
{
	int i = 0;
	for( ; i < len; i++) {
		*(dst + i) = 0xff & (0xff - *(src + i));
	}	
}

int bf_core_read_frame(char *image)
{
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	u8 *imgdata = pbfimage->fpdata;
	BF_LOG("++++");
	bl_spi_read_frame(bl_data,bl_data->buf_size);
	if(bl_data->chipid == BL_CHIPID_3290)
	{
		exchange_white_black(imgdata, bl_data->tx_buf + FRAME_DUMMY_LEN, width * height);
		memcpy(imgdata+((height-1) * width),imgdata+((height-2) * width), width);
	}
	else
		memcpy(imgdata, bl_data->tx_buf + FRAME_DUMMY_LEN, width * height);
	
	if(image != NULL)
		memcpy(image, imgdata, width * height);
	return 0;
}

int bf_core_enroll(bf_enroll_data_t *enrolldata)
{
	int ret;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	
	u8 *imgdata = pbfimage->fpdata;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	
	ret = bf_do_xu_imageEnhance(pbfimage, pbf_algo, pbf_algo->nXuAreaEnroll);
	if(ret == 0)
	{
		ret = bf_do_PB_imageQuality(pbfimage);
		BF_LOG("pbfTemplateMgr=%x,tCurTemplate=%x,pTemplateData=%x",pbfTemplateMgr,pbl_template,pbl_template->pTemplateData);
		bl_Alg_ExtractTemplate(
			pbf_algo->nAlgoID,                 //[in] 指定算法ID
			imgdata,                //[in] 指纹图像数据
			pbf_algo->width,                  //[in] 图像宽
			pbf_algo->height,                 //[in] 图像高
			pbl_template     //[out] 提取到的模板数据，支持预先分配数据内存
		);	
		BF_LOG("pbl_template %x %x",pbl_template->templateType, pbl_template->templateSize);
		
		bl_Alg_Enroll(
			pbf_algo->nAlgoID,               //[in] 指定算法ID
			imgdata,       					//[in] 指纹图像数据
			pbf_algo->width,                //[in] 图像宽，BF3290为112
			pbf_algo->height,               //[in] 图像高，BF3290为96
			pbl_template,     				//[in] 提前提取的指纹模板，传入这个模板可以加速录入，如果无输入0
			&enrolldata->progress,          //[out] 已成功录入模板数
			&enrolldata->coverage,           //[out] 指纹录入覆盖面积，范围[0,100]
			&enrolldata->nNbrOfIslands        //[out] 孤岛数量
		);
		BF_LOG("enrolldata %d %d %d",enrolldata->progress, enrolldata->coverage, enrolldata->nNbrOfIslands);
		if(pbf_algo->nMaxSample == enrolldata->progress)
		{
			bl_Alg_FinalizeEnroll(
				pbf_algo->nAlgoID,                   //[in] 指定算法ID
				&pbfTemplateMgr->bl_templates[gbf_core->curEnrollIndic]        //[out] 返回已经生成的模板数据，支持预先分配数据内存
			);
		}

	}

	return ret;
}

int bf_core_identify(bf_identify_data_t *data)
{
	int ret;
	struct bl_fingerprint_data *bl_data = gbf_core->bl_data;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	bf_image_t *pbfimage = gbf_core->pbfimage;
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	BL_TEMPLATE *pbl_template = &pbfTemplateMgr->tCurTemplate;
	BL_TEMPLATE *pbl_Alltemplates[5] = {0};
	
	u8 *imgdata = pbfimage->fpdata;
	int width = bl_data->framewidth;
	int height = bl_data->frameheight;
	
	ret = bf_do_xu_imageEnhance(pbfimage, pbf_algo, pbf_algo->nXuAreaMatch);
	if(ret == 0)
	{
		ret = bf_do_PB_imageQuality(pbfimage);
		BF_LOG("pbfTemplateMgr=%x,tCurTemplate=%x,pTemplateData=%x,ret=%d",pbfTemplateMgr,pbl_template,pbl_template->pTemplateData,ret);
		ret = bl_Alg_ExtractTemplate(
			pbf_algo->nAlgoID,                 //[in] 指定算法ID
			imgdata,                //[in] 指纹图像数据
			pbf_algo->width,                  //[in] 图像宽
			pbf_algo->height,                 //[in] 图像高
			pbl_template     //[out] 提取到的模板数据，支持预先分配数据内存
		);	
		BF_LOG("pbl_template %x %x ret=%d",pbl_template->templateType, pbl_template->templateSize, ret);
		
		//pbl_Alltemplates[0] = &pbfTemplateMgr->bl_templates[0];
		bf_get_all_templates(pbfTemplateMgr, pbl_Alltemplates);
		ret = bl_Alg_VerifyT(
			pbf_algo->nAlgoID,                   //[in] 指定算法ID
			pbl_template,         				//[in] 待认证指纹模板
			pbl_Alltemplates,  					//[in] 已录入多模板数组 
			pbfTemplateMgr->indexcount,     	//[in] 已录入模板数量	
			pbf_algo->far_match,                //[in] FAR
			&data->index                        //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
		);
		BF_LOG("bl_Alg_VerifyT ret=%d,index=%d,pbfTemplateMgr->bl_templates=%x",ret,data->index,pbfTemplateMgr->bl_templates);
	}
	return ret;
}

int bf_core_get_template_count(uint32_t* count)
{
	return bf_core_get_indices(NULL, count);
}

int bf_core_get_indices(uint32_t* indices, uint32_t* count)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_template_get_indices(pbfTemplateMgr, indices, count);
	return 0;
}

int bf_core_get_template_id_from_index(uint32_t index, uint32_t* data)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	*data = pbfTemplateMgr->fids[index];
	BF_LOG("index=%d,fid=%d",index, *data);
	return 0;
}

int bf_core_delete_template(uint32_t index)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	int ntemplatSize = pbf_algo->initParam.nMaxTemplateSize;
	BF_LOG("index=%d", index);
	pbfTemplateMgr->fids[index] = 0;
	memset(pbfTemplateMgr->bl_templates[index].pTemplateData, 0, ntemplatSize);
	pbfTemplateMgr->bl_templates[index].templateType = 0;       //模板类型
	pbfTemplateMgr->bl_templates[index].templateSize = 0;       //模板数据的大小
	return 0;
}

int bf_core_new_fid(uint32_t* id)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	int i = 0;
	
	for(i = 0;i < BF_MAX_FINGER;i++)
	{
		if(pbfTemplateMgr->fids[i] == 0)
		{
			pbfTemplateMgr->fids[i] = i + 1;//生成fid
			*id = pbfTemplateMgr->fids[i];
			gbf_core->curEnrollIndic = i;
			break;
		}
	}
	BF_LOG("fid=%d", *id);
	return *id;
}

int bf_core_delete_fid(uint32_t id)
{
	bf_template_manager_t *pbfTemplateMgr = gbf_core->pbfTemplateMgr;
	bf_algo_t *pbf_algo = gbf_core->pbf_algo;
	int i = 0;
	
	for(i = 0;i < BF_MAX_FINGER;i++)
	{
		if(pbfTemplateMgr->fids[i] == id)
		{
			pbfTemplateMgr->fids[i] = 0;//清除id
			bl_Alg_CancelEnroll(pbf_algo->nAlgoID);
			break;
		}
	}
	BF_LOG("fid=%d", id);
}

int bf_core_load_user_db(const char* path, uint32_t path_len)
{
	char db_path[MAX_PATH_LENGTH] = {0};
	char *db_name = "bf_fpdata.db";
	int dbfd = 0;

	if(memcmp(path, gbf_core->store_path,path_len))
	{
		memcpy(gbf_core->store_path, path, path_len);
		memcpy(db_path , path, path_len);
		db_path[path_len - 1] = '/';
		strncat(db_path + path_len,db_name,strlen(db_name));

		dbfd = open(db_path, O_RDWR | O_CREAT | O_SYNC, S_IRWXG|S_IRWXU);
		if(dbfd > 0)
		{
			gbf_core->dbfd = dbfd;
			bf_template_manager_init_from_dbfile(gbf_core->pbfTemplateMgr, dbfd);
		}
		else
		{
			BF_LOG("open failed %s,errno=%d",db_path,errno);
		}
		BF_LOG("change db_path=%s,dbfd=%d", db_path,dbfd);
	}
	return 0;
}

int bf_core_set_active_fingerprint_set(int32_t fingerprint_set_key)
{
	gbf_core->fpset_key = fingerprint_set_key;
	BF_LOG("fpset_key=%d", gbf_core->fpset_key);
	return 0;
}

int bf_core_store_template_db()
{
	return bf_template_manager_store_to_db(gbf_core->pbfTemplateMgr, gbf_core->dbfd);
}
