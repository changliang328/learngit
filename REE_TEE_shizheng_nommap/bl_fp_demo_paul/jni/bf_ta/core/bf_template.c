#include "bf_custom.h"
#include "bf_template.h"
#include "bf_log.h"
#include "bf_crc.h"
bf_template_manager_t *bf_template_manager_new(struct bf_algo_data *pbf_algo)
{
	bf_template_manager_t *pbf_template_mgr = bf_tee_malloc(sizeof(bf_template_manager_t));
	int ntemplatSize = pbf_algo->initParam.nMaxTemplateSize;

	int i = 0;
	int j = 0;

	//add by jaston for  check null pointer
	if(pbf_template_mgr == NULL)
	{
		BF_LOG("pbf_template_mgr malloc error ,exit here !!!!");
		return -1;//exit(EXIT_FAILURE);
	}
	//add end

	for(i = 0;i < BF_MAX_FINGER; i++)
	{
		pbf_template_mgr->fids[i] = 0;
		//模板数据
		pbf_template_mgr->bl_templates[i].pTemplateData = bf_tee_malloc(ntemplatSize);
		//add by jaston  for malloc check
		if (pbf_template_mgr->bl_templates[i].pTemplateData == NULL) {

			for (j = i -1; j >=0; j--) {
				if (pbf_template_mgr->bl_templates[j].pTemplateData)
					bf_tee_free((void*)pbf_template_mgr->bl_templates[j].pTemplateData);
				pbf_template_mgr->bl_templates[j].pTemplateData = NULL;
			}

			bf_tee_free((void*)pbf_template_mgr);
			pbf_template_mgr = NULL;

			BF_LOG("pbf_template_mgr->bl_templates.pTemplateData error ,exit here !!!!");
			return -1;//exit(EXIT_FAILURE);
		}
		//add end
		pbf_template_mgr->bl_templates[i].templateType = 0;       //模板类型
		pbf_template_mgr->bl_templates[i].templateSize = 0;       //模板数据的大小
		//BF_LOG("pTemplate[%d]=%x ,ntemplatSize %d",i,&pbf_template_mgr->bl_templates[i], ntemplatSize);
	}
	//BF_LOG("pbf_template_mgr->tCurTemplate=%x",&pbf_template_mgr->tCurTemplate);

	pbf_template_mgr->tCurTemplate.pTemplateData = bf_tee_malloc(ntemplatSize);


	//add by jaston  for malloc check
	if (pbf_template_mgr->tCurTemplate.pTemplateData== NULL) {

		for (j = BF_MAX_FINGER - 1 ; j >=0; j--) {
			if (pbf_template_mgr->bl_templates[j].pTemplateData)
				bf_tee_free((void*)pbf_template_mgr->bl_templates[j].pTemplateData);
		}
		bf_tee_free((void*)pbf_template_mgr);
		pbf_template_mgr = NULL;
		BF_LOG(" pbf_template_mgr->tCurTemplate.pTemplateData error ,exit here !!!!");
		return -1;//exit(EXIT_FAILURE);
	}
	//add end

	pbf_template_mgr->tCurTemplate.templateType = 0; 
	pbf_template_mgr->tCurTemplate.templateSize = 0;
	pbf_template_mgr->uMaxTemplateSize = ntemplatSize;
	//BF_LOG("pTemplateData=%x ,ntemplatSize %d",pbf_template_mgr->tCurTemplate.pTemplateData, ntemplatSize);
	//BF_LOG("pbf_template_mgr=%x",pbf_template_mgr);
	pbf_template_mgr->magicNumber = BF_DB_MAGIC;
	pbf_template_mgr->dbfd = 0;
	pbf_template_mgr->bkdbfd = 0;
#ifdef USE_CRC_CHECK
	pbf_template_mgr->crcNumber = CRC_INIT_BASE ;
	CRC32_DTable_Init();
#endif
	return pbf_template_mgr;
}

int bf_template_manager_destroy(bf_template_manager_t *pbf_template_mgr)
{
	int i = 0;
	for(i = 0;i < BF_MAX_FINGER; i++)
		bf_tee_free(pbf_template_mgr->bl_templates[i].pTemplateData);
	bf_tee_free(pbf_template_mgr->tCurTemplate.pTemplateData);
	bf_tee_free(pbf_template_mgr);
	
	return 0;
}

int bf_template_manager_init_from_dbfile(bf_template_manager_t *pbf_template_mgr)
{
	bf_template_manager_t headerbuf;
	int ntemplatSize = pbf_template_mgr->uMaxTemplateSize;
	char *db_path = pbf_template_mgr->db_path;
	char *dbbk_path = pbf_template_mgr->bk_db_path;
	int dbfd,bkdbfd;
	int offset = 0;
	int count;
	int i = 0;
	BF_LOG("+++++++++++++++\n");

	//Clear FingerTemplatesData
	for(i=0;i<BF_MAX_FINGER;i++)
	{
		pbf_template_mgr->fids[i]=0;
		pbf_template_mgr->bl_templates[i].templateSize=0;
		pbf_template_mgr->bl_templates[i].templateType=0;
		bf_tee_memset(pbf_template_mgr->bl_templates[i].pTemplateData,0,sizeof(pbf_template_mgr->bl_templates[i].templateSize));
	}
	//pbf_template_mgr->user_id=0;
	//Clear 
	
	dbfd = bf_tee_fd_open(db_path, 0);
	bkdbfd = bf_tee_fd_open(dbbk_path, 0);
	pbf_template_mgr->dbfd = dbfd;
	pbf_template_mgr->bkdbfd = bkdbfd;
	BF_LOG("dbfd=%d",dbfd);
	if(dbfd > 0)
	{//type 1 fd read
		bf_tee_fd_lseek(dbfd , 0, 0);//bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
		count = bf_tee_fd_read(dbfd, &headerbuf, sizeof(bf_template_manager_t));
		BF_LOG("headerbuf.magicNumber = %x  count = %d",headerbuf.magicNumber,count);
		//add by jaston for crc check
	#ifdef USE_CRC_CHECK
		unsigned int data_crc = 0;
		if((count > 0) && (headerbuf.magicNumber == BF_DB_MAGIC))
		{
			bf_tee_fd_lseek(dbfd , DB_DATA_OFFSET, SEEK_SET);
			if(CRC32_Check_File(dbfd,&data_crc) == 0)
			{	
				BF_LOG(" dbfd = %d data_crc = %x , crcNumber =  %x " ,dbfd,data_crc,headerbuf.crcNumber);
				if(headerbuf.crcNumber == data_crc)
				{
					BF_LOG("crc check success , copy to backup  !!!");
					bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
					bf_tee_fd_lseek(bkdbfd , 0, SEEK_SET);
					btl_backup_database(dbfd,bkdbfd);
				}else{
					BF_LOG("crc check error , copy from backup here !!!");
					bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
					bf_tee_fd_lseek(bkdbfd , 0, SEEK_SET);
					btl_backup_database(bkdbfd,dbfd);
					bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
					bf_tee_fd_read(dbfd, &headerbuf, sizeof(bf_template_manager_t));//read again for updating
				}
			}
		}
	#endif
		//add end
		if((count > 0) && (headerbuf.magicNumber == BF_DB_MAGIC))
		{
			bf_tee_fd_lseek(dbfd , DB_DATA_OFFSET, 0);
			//memcpy(pbf_template_mgr, &headerbuf, sizeof(bf_template_manager_t));
			for(i = 0;i < BF_MAX_FINGER; i++)
			{
				pbf_template_mgr->fids[i] = headerbuf.fids[i];
				pbf_template_mgr->bl_templates[i].templateType = headerbuf.bl_templates[i].templateType;//模板类型
				pbf_template_mgr->bl_templates[i].templateSize = headerbuf.bl_templates[i].templateSize;//模板数据的大小
				BF_LOG("headerbuf.fids[%d] = %d  templateType=%x ,ntemplatSize %d",i,headerbuf.fids[i],pbf_template_mgr->bl_templates[i].templateType, pbf_template_mgr->bl_templates[i].templateSize);
				count = bf_tee_fd_read(dbfd, pbf_template_mgr->bl_templates[i].pTemplateData, ntemplatSize);
			}
			pbf_template_mgr->uMaxTemplateSize = headerbuf.uMaxTemplateSize;
		}
	}else if(0 == dbfd)
	{//type2 bf_tee_fs_read
		count = bf_tee_fs_read(db_path, &headerbuf, 0, sizeof(bf_template_manager_t));
		BF_LOG("headerbuf.magicNumber = %x  count = %d",headerbuf.magicNumber,count);
			//add by jaston for crc check
			#ifdef USE_CRC_CHECK
			unsigned int data_crc = 0;
			char *dbbk_path = pbf_template_mgr->bk_db_path;
			if((count > 0) && (headerbuf.magicNumber == BF_DB_MAGIC))
			{
				if(CRC32_Check_File_ByPath(db_path, DB_DATA_OFFSET ,&data_crc) == 0)
				{	
							BF_LOG(" db_path = %s data_crc = %x , crcNumber =  %x " ,db_path,data_crc,headerbuf.crcNumber);

								if(headerbuf.crcNumber == data_crc)
								{
									BF_LOG("crc check success , copy to backup  !!!");

									btl_backup_database_byPath(db_path,dbbk_path);
					
								}else{

									BF_LOG("crc check error , copy from backup here !!!");
									btl_backup_database_byPath(dbbk_path,db_path);
									bf_tee_fs_read(db_path, &headerbuf, 0 ,sizeof(bf_template_manager_t));//read again for updating

						}
				}
			}
		#endif
	//add end
	
		offset += DB_DATA_OFFSET;
		if((count > 0) && (headerbuf.magicNumber == BF_DB_MAGIC))
		{
			for(i = 0;i < BF_MAX_FINGER; i++)
			{
				pbf_template_mgr->fids[i] = headerbuf.fids[i];
				pbf_template_mgr->bl_templates[i].templateType = headerbuf.bl_templates[i].templateType;//模板类型
				pbf_template_mgr->bl_templates[i].templateSize = headerbuf.bl_templates[i].templateSize;//模板数据的大小
				count = bf_tee_fs_read(db_path, pbf_template_mgr->bl_templates[i].pTemplateData, offset,ntemplatSize);
				BF_LOG("headerbuf.fids[%d] = %d  templateType=%x ,ntemplatSize %d",i,headerbuf.fids[i],pbf_template_mgr->bl_templates[i].templateType, pbf_template_mgr->bl_templates[i].templateSize);
				BF_LOG("db_path=%s [%d]offset=%d,count=%d",db_path, i, offset, count);
				offset += ntemplatSize;
			}
			pbf_template_mgr->uMaxTemplateSize = headerbuf.uMaxTemplateSize;
		}
	}else
	{
		BF_LOG("open failed %s,dbfd=%d\n",pbf_template_mgr->db_path,dbfd);
		BF_LOG("open failed %s,dbfd=%d\n",pbf_template_mgr->db_path,dbfd);
		BF_LOG("open failed %s,dbfd=%d\n",pbf_template_mgr->db_path,dbfd);
	}

	if(pbf_template_mgr->user_id == 0){
	 pbf_template_mgr->user_id = headerbuf.user_id;
	 pbf_template_mgr->authenticator_id = headerbuf.authenticator_id;
	 
	}
	//BF_LOG("load user_id=%llu  ",pbf_template_mgr->user_id);
	
	BF_LOG(" -----------");
	return 0;
}

int bf_template_manager_store_to_db(bf_template_manager_t *pbf_template_mgr)
{
	int ntemplatSize = pbf_template_mgr->uMaxTemplateSize;
	char *db_path = pbf_template_mgr->db_path;
	int dbfd,bkdbfd;
	int offset;
	int count;
	int i = 0;

	dbfd = pbf_template_mgr->dbfd;
	bkdbfd = pbf_template_mgr->bkdbfd;
	BF_LOG("dbfd=%d",dbfd);
	if(dbfd > 0)
	{
		bf_tee_fd_lseek(dbfd , 0, 0);
		bf_tee_fd_write(dbfd, pbf_template_mgr, sizeof(bf_template_manager_t));
		bf_tee_fd_lseek(dbfd , DB_DATA_OFFSET, 0);
		
		for(i = 0;i < BF_MAX_FINGER; i++)
		{
			count = bf_tee_fd_write(dbfd, pbf_template_mgr->bl_templates[i].pTemplateData, ntemplatSize);
			BF_LOG("[%d]templateType=%d,size=%d",i, pbf_template_mgr->bl_templates[i].templateType, pbf_template_mgr->bl_templates[i].templateSize);
		}

		//add by jaston for crc check
		#ifdef USE_CRC_CHECK
			
			unsigned int data_crc = 0;
			unsigned int pre_crc = 0;
			pre_crc = pbf_template_mgr->crcNumber;
			
				BF_LOG("pre_crc = %x  ",pre_crc);
			    bf_tee_fd_lseek(dbfd , DB_DATA_OFFSET, SEEK_SET);
				if(0 == CRC32_Check_File(dbfd,&data_crc))
				{	
				    BF_LOG(" data_crc = %x  ",data_crc);
					pbf_template_mgr->crcNumber  = data_crc;
					bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
		            bf_tee_fd_write(dbfd, pbf_template_mgr,sizeof(bf_template_manager_t));
					if(pre_crc == CRC_INIT_BASE)
						{
							BF_LOG(" first  store pre_crc = %x  #####",pre_crc);

							bf_tee_fd_lseek(dbfd , 0, SEEK_SET);
							bf_tee_fd_lseek(bkdbfd , 0, SEEK_SET);
							btl_backup_database(dbfd,bkdbfd);// just for the first enrolled finger to be backup
					  }
				}
		
		#endif
	//add end
	}else if(0 == dbfd)
	{
		offset = 0;
		count = bf_tee_fs_write(db_path, pbf_template_mgr, 0, sizeof(bf_template_manager_t));
		offset += DB_DATA_OFFSET;
		for(i = 0;i < BF_MAX_FINGER; i++)
		{
			count = bf_tee_fs_write(db_path, pbf_template_mgr->bl_templates[i].pTemplateData, offset,ntemplatSize);
			BF_LOG("db_path=%s [%d]offset=%d,count=%d",db_path, i, offset, count);
			offset += ntemplatSize;
		}
			//add by jaston for crc check
		#ifdef USE_CRC_CHECK
			char *dbbk_path = pbf_template_mgr->bk_db_path;

			unsigned int data_crc = 0;
			unsigned int pre_crc = 0;
			pre_crc = pbf_template_mgr->crcNumber;
			BF_LOG("pre_crc = %x  ",pre_crc);
			if(0 == CRC32_Check_File_ByPath(db_path,DB_DATA_OFFSET,&data_crc))
			{	
			    BF_LOG(" data_crc = %x  ",data_crc);
	
				pbf_template_mgr->crcNumber  = data_crc;
	            bf_tee_fs_write(db_path, pbf_template_mgr, 0 ,sizeof(bf_template_manager_t));
				if(pre_crc == CRC_INIT_BASE)
				btl_backup_database_byPath(db_path,dbbk_path);// just for the first enrolled finger to be backup
	
			}
		
		#endif
	//add end
	}else
	{
		BF_LOG("open failed %s",pbf_template_mgr->db_path);
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
			BF_LOG("bf_get_all_templates [%d]templateType=%d,size=%d",i, pbf_template_mgr->bl_templates[i].templateType, pbf_template_mgr->bl_templates[i].templateSize);
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
	return 0;
}
