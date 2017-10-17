#include "bf_template.h"
#include "bf_log.h"
bf_template_manager_t *bf_template_manager_new(struct bf_algo_data *pbf_algo)
{
	bf_template_manager_t *pbf_template_mgr = malloc(sizeof(bf_template_manager_t));
	int ntemplatSize = pbf_algo->initParam.nMaxTemplateSize;

	int i = 0;
	for(i = 0;i < BF_MAX_FINGER; i++)
	{
		pbf_template_mgr->fids[i] = 0;
      //模板数据
		pbf_template_mgr->bl_templates[i].pTemplateData = malloc(ntemplatSize);
		pbf_template_mgr->bl_templates[i].templateType = 0;       //模板类型
		pbf_template_mgr->bl_templates[i].templateSize = 0;       //模板数据的大小
		//BF_LOG("pTemplate[%d]=%x ,ntemplatSize %d",i,&pbf_template_mgr->bl_templates[i], ntemplatSize);
	}
	//BF_LOG("pbf_template_mgr->tCurTemplate=%x",&pbf_template_mgr->tCurTemplate);

	pbf_template_mgr->tCurTemplate.pTemplateData = malloc(ntemplatSize);
	pbf_template_mgr->tCurTemplate.templateType = 0; 
	pbf_template_mgr->tCurTemplate.templateSize = 0;
	pbf_template_mgr->uMaxTemplateSize = ntemplatSize;
	//BF_LOG("pTemplateData=%x ,ntemplatSize %d",pbf_template_mgr->tCurTemplate.pTemplateData, ntemplatSize);
	//BF_LOG("pbf_template_mgr=%x",pbf_template_mgr);
	pbf_template_mgr->magicNumber = BF_DB_MAGIC;
	return pbf_template_mgr;
}

int bf_template_manager_destroy(bf_template_manager_t *pbf_template_mgr)
{
	int i = 0;
	for(i = 0;i < BF_MAX_FINGER; i++)
		free(pbf_template_mgr->bl_templates[i].pTemplateData);
	free(pbf_template_mgr->tCurTemplate.pTemplateData);
	free(pbf_template_mgr);
	return 0;
}

int bf_template_manager_init_from_dbfile(bf_template_manager_t *pbf_template_mgr, int dbfd)
{
	bf_template_manager_t headerbuf;
	int ntemplatSize = pbf_template_mgr->uMaxTemplateSize;
	int count;
	int i = 0;
	
	lseek(dbfd , 0, SEEK_SET);
	count = read(dbfd, &headerbuf, sizeof(bf_template_manager_t));
	if((count > 0) && (headerbuf.magicNumber == BF_DB_MAGIC))
	{
		lseek(dbfd , DB_DATA_OFFSET, SEEK_SET);
		//memcpy(pbf_template_mgr, &headerbuf, sizeof(bf_template_manager_t));
		for(i = 0;i < BF_MAX_FINGER; i++)
		{
			pbf_template_mgr->fids[i] = headerbuf.fids[i];
			pbf_template_mgr->bl_templates[i].templateType = headerbuf.bl_templates[i].templateType;       //模板类型
			pbf_template_mgr->bl_templates[i].templateSize = headerbuf.bl_templates[i].templateSize;       //模板数据的大小
			count = read(dbfd, pbf_template_mgr->bl_templates[i].pTemplateData, ntemplatSize);
			BF_LOG("[%d]templateType=%d,size=%d",i ,headerbuf.bl_templates[i].templateType,headerbuf.bl_templates[i].templateSize);

		}
		pbf_template_mgr->uMaxTemplateSize = headerbuf.uMaxTemplateSize;

	}
	return 0;
}

int bf_template_manager_store_to_db(bf_template_manager_t *pbf_template_mgr, int dbfd)
{
	int ntemplatSize = pbf_template_mgr->uMaxTemplateSize;
	int count;
	int i = 0;

	lseek(dbfd , 0, SEEK_SET);
	write(dbfd, pbf_template_mgr, sizeof(bf_template_manager_t));
	
	lseek(dbfd , DB_DATA_OFFSET, SEEK_SET);
	for(i = 0;i < BF_MAX_FINGER; i++)
	{
		count = write(dbfd, pbf_template_mgr->bl_templates[i].pTemplateData, ntemplatSize);
		BF_LOG("[%d]templateType=%d,size=%d",i, pbf_template_mgr->bl_templates[i].templateType, pbf_template_mgr->bl_templates[i].templateSize);
	}
	return 0;
}

int bf_get_all_templates(bf_template_manager_t *pbf_template_mgr, BL_TEMPLATE **pbfAllTemplates)
{
	int i = 0;
	uint32_t* indices = pbf_template_mgr->indices;
	int indexcount = 0;
	
	for(i = 0;i < BF_MAX_FINGER; i++)
	{
		if(pbf_template_mgr->fids[i] != 0)
		{
			indices[indexcount] = i;
			pbfAllTemplates[indexcount] = &pbf_template_mgr->bl_templates[i];
			indexcount++;
		}
	}
	pbf_template_mgr->indexcount = indexcount;
	return 0;
}

int bf_template_get_indices(bf_template_manager_t *pbf_template_mgr, uint32_t* indices, uint32_t* count)
{
	int i = 0;
	*count = 0;
	
	for(i = 0;i < BF_MAX_FINGER;i++)
	{
		BF_LOG("%d",pbf_template_mgr->fids[i]);
		if(pbf_template_mgr->fids[i] != 0)
		{
			if(indices != NULL)
				indices[*count] = i;
			(*count)++;
		}
	}
	BF_LOG("count = %d", *count);
}
