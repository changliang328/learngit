/***********************************************************************************************************************
*
* Moudle  : templete manager  module
* File    : btl_templete.c
*
* By      : jaston
* Version : v1.0
* Date    : 2017-09-06  09:57
***********************************************************************************************************************/

#include "btl_templete.h"
#include "btl_debug.h"

static int need_init_manger = 1;
pTemplete_manager_t ptmanger = NULL;

#define FINGER_HEADER_OFFSET   10
#define FINGER_TYPE_OFFSET   20 // 10  index and  10 type offset
#define FINGER_SIZE_OFFSET   10 // 10  index and  10 type offset 
#define FINGER_TEMPLATE_HAEADSIZE  20



#define MAXTEMPLATESIZE   150*1024
const char *tdataFolderPath    = "/data/system/users/0/fpdata/templete.dat";



/*----------------------------------------------------------------------
purpose: contact str1  str2  
return :   success 0  fail: -1
------------------------------------------------------------------------*/

int BitSet(unsigned int *Number,int pos)
{

    return (*Number) |= 1<<(pos);
	 

}



int BitClear(unsigned int *Number,int pos) 
{
	return (*Number) &= ~(1<<(pos));
	  
}


 int BitGet(unsigned int *Number,int pos) 
 {
 	
 	 return ((*Number) >> (pos)&1);
	    
 }
char   str_contact(const char *str1, const char *str2, char *result)
{

    if (result == NULL) {
        LOGBD("btlfp_file Error: result is null ! \n");
        return -1;
    }
    strcpy(result, str1);
    strcat(result, str2);
    return 0;
}
char* myitoa(int file_index, char* num_char, int radix) 
{
    char num_index[] = "0123456789";
    unsigned unum;
    int i = 0, j, k;
    if (radix == 10 && file_index < 0) { 
        unum = (unsigned) -file_index;
        num_char[i++] = '-';
    } else {
        unum = (unsigned) file_index;
    }

    do {
        num_char[i++] = num_index[unum % (unsigned) radix];
        unum /= radix;
    } while (unum);

    num_char[i] = '\0';
    if (num_char[0] == '-') {
        k = 1;
    } else {
        k = 0;
    }

    char temp;
    for (j = k; j <= (i - 1) / 2; j++) {
        temp = num_char[j];
        num_char[j] = num_char[i - 1 + k - j];
        num_char[i - 1 + k - j] = temp;
    }
    return num_char;
}




/*----------------------------------------------------------------------
purpose: read finger index file
return :   void
------------------------------------------------------------------------*/
int btl_tmanager_ReadHeaderFile()
{
    int i;
	FILE *fpFile = NULL;
#ifdef FINGER_DEBUG
    LOGBD("btlfp_file btl_ReadIndexFile");
#endif
	if((fpFile  = fopen(tdataFolderPath,"r+")) != NULL)
		{
			
	        if(fread(&(ptmanger->tdb), sizeof(Templete_db_t), 1,fpFile) != 1)
	        	{
				LOGBD("btl_tmanager_ReadHeaderFile error !!");
				fclose(fpFile);
				return -1;
				}
			
			}else{
				 
				  LOGBD("btl_tmanager_OpenHeaderFile error !!");
				  return -2;

	}
  
#ifdef FINGER_DEBUG
        LOGBD("read ptmanger->tdb.fingerindex :%d",ptmanger->tdb.fingerindex);
#endif
		fclose(fpFile);
    return 0;
}





/*----------------------------------------------------------------------
purpose: write finger index file
return :   success 0  fail: -1 (open fail)    -2(write fail)
------------------------------------------------------------------------*/
int32_t btl_tmanager_WriteHeaderFile()
{

    int i = 0;
	FILE *fpFile = NULL;

#ifdef FINGER_DEBUG
        LOGBD("btl_WriteIndexFile ptmanger->tdb.fingerindex:%d",ptmanger->tdb.fingerindex);
#endif
  if((fpFile  = fopen(tdataFolderPath,"r+")) != NULL) {
				LOGBD("btl_WriteIndexFile open success ptmanger->tdb.fingerindex:%d",ptmanger->tdb.fingerindex);
				if(fwrite(&(ptmanger->tdb), sizeof(Templete_db_t), 1,fpFile) != 1)
					{
						LOGBE("btlfp_file btl_WriteIndexFile error line: %d ",__LINE__);
						fclose(fpFile);
						 return -2;
					}else{

				  LOGBD("btl_WriteIndexFile write success ptmanger->tdb.fingerindex:%d",ptmanger->tdb.fingerindex);
				
				}
     
		    } else{
		    		//file is not exist , may be create or copy from backup
		    	   if ((fpFile = fopen(tdataFolderPath,"w+")) != NULL) //create
				   	{
				   	if(fwrite(&(ptmanger->tdb), sizeof(Templete_db_t), 1,fpFile) != 1)
					 {				
						LOGE("btlfp_file btl_WriteIndexFile error line:%d ",__LINE__);
						fclose(fpFile);
						return -2;		
				}else{
				
						LOGE("btlfp_file btl_WriteIndexFile success  line:%d  !!!!!",__LINE__);
				}
		   }
    }
	fclose(fpFile);
    return 0;
}

int32_t btl_tmanager_create_databasefile()
{
    int i ;
    FILE *fpHeader = NULL;
    
        ptmanger->tdb.fingerindex = 0;

    if((fpHeader  = fopen(tdataFolderPath,"r+")) == NULL) {
			LOGBD("btl_create_database first time  %s",tdataFolderPath);
	        btl_tmanager_WriteHeaderFile();//creat if no file 
	        
	    	} else {
	        fclose(fpHeader);
	        btl_tmanager_ReadHeaderFile();
	    }
    LOGBD("btl_create_database %s",tdataFolderPath);
	
    return 0;
}




/*----------------------------------------------------------------------
purpose: Generate a filename by finger id
return :   void
------------------------------------------------------------------------*/
void btl_tmanager_generateFileName(char *fileName, int fid)
{
    char tmp[10];
    tmp[0] = 0;
    sprintf(tmp,"%d",fid);
    strcpy(fileName,tdataFolderPath);
    strcat(fileName,tmp);
}

int btl_tmanager_save_templete_by_id(int fid,PBL_TEMPLATE blTemplete)
	{
		
		char savestr[20];
		//finger_filename[0] = 0;
		int ret = 0;
		int tempsize  = blTemplete->templateSize;
		int temptype  = blTemplete->templateType;

		LOGBD(" %s   fid :%d  ",__func__,fid);
		if(fid <= 0)
			{
			LOGBD(" %s error fid :%d  ",__func__,fid);
			return 0;
		}
		BitSet(&ptmanger->tdb.fingerindex,fid);
		btl_tmanager_WriteHeaderFile();
		ptmanger->pArry_templete[fid -1]->templateSize = tempsize;
		ptmanger->pArry_templete[fid -1]->templateType = temptype;
		memcpy(ptmanger->pArry_templete[fid -1]->pTemplateData, blTemplete->pTemplateData, blTemplete->templateSize);
		
		
		
	

		myitoa(tempsize,savestr,10);
		LOGBD("btlfp_file btl_file_saveTemplate save tempsize %s",savestr);
		uint8_t *wg_buffer = (uint8_t *)malloc(tempsize + 20);
		memcpy(wg_buffer, savestr, 10);
	

		memset(savestr,0,10);
		
		myitoa(temptype,savestr,10);
	
		LOGBD("btlfp_file btl_file_saveTemplate temptype   %s",savestr);
	
		memcpy(wg_buffer + ptmanger->tdb.fingerSizeOffset, savestr, ptmanger->tdb.fingerSizeOffset);//type save

		memcpy(wg_buffer+	ptmanger->tdb.fingerTypeOffset , blTemplete->pTemplateData, tempsize);

		
		FILE* fp = fopen(tdataFolderPath, "r+");
		if (fp == NULL) 
			return -10;
	    fseek(fp, ( ptmanger->tdb.fingerHeaderOffset) + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset), 0);
		fwrite(wg_buffer, tempsize + ptmanger->tdb.fingerTypeOffset , 1, fp);
		
		free(wg_buffer);
		wg_buffer = NULL;
		
		   fclose(fp);
		return 0;
	}






/*----------------------------------------------------------------------
purpose: delete   Template by fid 
return :   success 0  fail: -1
------------------------------------------------------------------------*/

int btl_tmanager_delTemplate_by_id(int fid)
{
    char finger_filename[128];
    finger_filename[0] = 0;
	int ret = 0;
	int temp_zize = 0;
	FILE *fpFile = NULL;
	temp_zize = ptmanger->pArry_templete[fid-1]->templateSize + ptmanger->tdb.fingerTypeOffset ;

	uint8_t *wg_buffer = (uint8_t *)malloc(ptmanger->tdb.fingerUnitTmoduleOffset);

	if(fid <= 0)
		{
		free(wg_buffer);
		wg_buffer = NULL;
		LOGBD(" %s error fid :%d  ",__func__,fid);
		return 0;
	}
		BitClear(&ptmanger->tdb.fingerindex,fid);
		btl_tmanager_WriteHeaderFile();

   

		//update templetemanager
		memset(ptmanger->pArry_templete[fid-1]->pTemplateData,0,ptmanger->pArry_templete[fid-1]->templateSize);
		ptmanger->pArry_templete[fid-1]->templateSize = 0;
		ptmanger->pArry_templete[fid-1]->templateType = 0;
		//update end
		


		
		   memset(wg_buffer,0xff,ptmanger->tdb.fingerUnitTmoduleOffset);
		   if((fpFile	= fopen(tdataFolderPath,"r+")) != NULL) {

			   fseek(fpFile, ( ptmanger->tdb.fingerHeaderOffset  + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset)), 0);
			   
		       fwrite(wg_buffer, ptmanger->tdb.fingerUnitTmoduleOffset, 1, fpFile);
		   } 
		   
		   free(wg_buffer);
		   wg_buffer = NULL;
		   
		   fclose(fpFile);
	
		 
    return 0;
}



int btl_tmanager_get_templete_by_id(  int fid , PBL_TEMPLATE pblMulTemplete)
{
			char ret = 0;
			
			int size_read = 0 ;
			int type_read = 0 ;
			char read_size[10] = {0x00};
			char read_type[10] = {0x00};
				
			

				FILE *fp1 = fopen(tdataFolderPath, "r+");
				
				
				
				if (fp1 == NULL) {
					LOGBE(" open file %s fail",tdataFolderPath);
					pblMulTemplete->templateType =  0;
					pblMulTemplete->templateSize =  0;
					return -1;
				} else {
					
					fseek(fp1, ( ptmanger->tdb.fingerHeaderOffset + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset)), 0);
				    fread(read_size, 10, 1, fp1);
					size_read = atoi(read_size);
					if(size_read <=  0)
						{
						LOGBE(" size_read %d  error",size_read);
							pblMulTemplete->templateType =  0;
							pblMulTemplete->templateSize =  0;
						return -1;
					}
					
					LOGBE(" size_read = %d  success !!! ",size_read);
					fseek(fp1, ( ptmanger->tdb.fingerHeaderOffset  + ptmanger->tdb.fingerSizeOffset) + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset), 0);//type read
					fread(read_type, 10, 1, fp1);
					
					type_read = atoi(read_type);
					
					LOGBE(" type_read = %d  success !!! ",type_read);
					
					pblMulTemplete->templateType =  type_read;
					pblMulTemplete->templateSize =  size_read;

					LOGBE(" pblMulTemplete->templateType = %x  pblMulTemplete->templateSize = %d ",pblMulTemplete->templateType,pblMulTemplete->templateSize);
					
					fseek(fp1,(ptmanger->tdb.fingerHeaderOffset + ptmanger->tdb.fingerTypeOffset) + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset), 0);
					fread(pblMulTemplete->pTemplateData, size_read, 1, fp1);
					fclose(fp1);
				}

		return 0;

}




int btl_tmanager_resetTemplate()
{
    
		
    return 0;
}



int btl_tmanager_getTemplateSize(int fid)

{
			int index[10];
			int size_read = 0;
			char read_size[10] = {0x00};
			char ret = 0;

			FILE *fp1 = fopen(tdataFolderPath, "r+");
	
			if (fp1 == NULL) {
				LOGBE("btlfp_file btl_tmanager_getTemplateSize  file %s fail",tdataFolderPath);
				return 0;
			} else {
			// 
				fseek(fp1, ( ptmanger->tdb.fingerHeaderOffset) + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset), 0);
				fread(read_size, 10, 1, fp1);
				size_read = atoi(read_size);
				LOGBD("btlfp_file file size is %d \n",size_read);
				
			}
			fclose(fp1);
		    return  size_read;

}


int btl_tmanager_get_fingerid()
{
    int i=0;
	

    btl_tmanager_ReadHeaderFile();
	
    for (i = 1 ; i <= 5; i++) {
        if (BitGet(&ptmanger->tdb.fingerindex,i)== 0) {
            break;
        }
		
    }
    if(i > 5)
	i=5;
	
    return i;
}

int btl_tmanager_get_fingerNum(int *buffer)
{
    int i=0,FingerNum = 0;
    FILE *fileFp = NULL;


   btl_tmanager_ReadHeaderFile();
    for (i = 1 ; i <= 5; i++) {
		buffer[i-1] = 0;
        if (BitGet(&ptmanger->tdb.fingerindex,i) != 0) {
		    buffer[FingerNum] = i;
			LOGBE("btlfp_file buffer[%d]:%d  ",FingerNum,buffer[FingerNum]);
            FingerNum++;
        }
		
		
    }


    return FingerNum;
}

int btl_tmanager_delete_fingerprint_by_id(int fid)
	{
			
			char savestr[20];
			//finger_filename[0] = 0;
			int ret = 0;
			int tempsize  = ptmanger->pArry_templete[fid -1]->templateSize;
			int temptype  = ptmanger->pArry_templete[fid -1]->templateType;
	
			LOGBD(" %s	 fid :%d  ",__func__,fid);
			if(fid <= 0)
				{
				LOGBD(" %s error fid :%d  ",__func__,fid);
				return 0;
			}
			BitClear(&ptmanger->tdb.fingerindex,fid);
			btl_tmanager_WriteHeaderFile();
			ptmanger->pArry_templete[fid -1]->templateSize = 0;
			ptmanger->pArry_templete[fid -1]->templateType = 0;
			memset(ptmanger->pArry_templete[fid -1]->pTemplateData, 0xdd, tempsize);
			
			
			
		
	
			myitoa(0,savestr,10);
			LOGBD("btlfp_file btl_file_saveTemplate save tempsize %s",savestr);
			uint8_t *wg_buffer = (uint8_t *)malloc(tempsize + 20);
			memcpy(wg_buffer, savestr, 10);
		
	
			memset(savestr,0,10);
			
			myitoa(0,savestr,10);
		
			LOGBD("btlfp_file btl_file_saveTemplate temptype   %s",savestr);
		
			memcpy(wg_buffer + ptmanger->tdb.fingerSizeOffset, savestr, ptmanger->tdb.fingerSizeOffset);//type save
	
			memcpy(wg_buffer+	ptmanger->tdb.fingerTypeOffset , ptmanger->pArry_templete[fid -1]->pTemplateData, tempsize);
	
			
			FILE* fp = fopen(tdataFolderPath, "r+");
			if (fp == NULL) 
				return -10;
			fseek(fp, ( ptmanger->tdb.fingerHeaderOffset) + (fid - 1)*(ptmanger->tdb.fingerUnitTmoduleOffset), 0);
			fwrite(wg_buffer, tempsize + ptmanger->tdb.fingerTypeOffset , 1, fp);
			
			free(wg_buffer);
			wg_buffer = NULL;
			
			   fclose(fp);
			return 0;
		}


int32_t btl_tmanager_reset_databasefile()
{
    int i ;
	int ret = 0;
    FILE *fpFile = NULL;
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        mEnrolledIndex[i] = 0;

//	LOGBD("btlfp_file file_delete_fingerprint  fid:  %d",fid);
	


    ret = btl_tmanager_WriteHeaderFile();
	
	if(ret != 0)
		{
		LOGBD("btlfp_file btl_reset_databasefile error %d",ret);
		fclose(fpFile);
		return -1;
		}


    return 0;
}






int btl_tmanager_get_all_templete(PBL_TEMPLATE* pTemplate)
{
	
			int i = 0;
			int index[10];
			int tempfinger =0 ;
			btl_tmanager_get_fingerNum(index);
			LOGBE("%s +++++++++",__func__);
			for (i = 0; i < MAX_FINGER_NUMS ; i++) {
			
			if(index[i] != 0)
				{
						LOGBE("%s +++++++++index[%d]:%d  ",__func__,i,index[i]);
						tempfinger = index[i];
						btl_tmanager_get_templete_by_id(index[i],ptmanger->pArry_templete[tempfinger-1]);
				}
			}
			LOGBE("%s ---------",__func__);
		return  0;
}

int btl_tmanager_init_templete_manager()
{

		unsigned int i = 0;
		LOGBE("%s ++++",__func__);
		//malloc space for templetes
	    ptmanger = (pTemplete_manager_t)malloc(sizeof(Templete_manager_t));
		
		ptmanger->tdb.fingernum = MAX_FINGER_NUMS  ;//maybe detect from config
		ptmanger->pArry_templete = (PBL_TEMPLATE *)malloc(ptmanger->tdb.fingernum*sizeof(PBL_TEMPLATE));
		if(ptmanger->pArry_templete == NULL)
			{
			LOGBE("%s ++++  ptmanger->pArry_templete malloc error ",__func__);
			return -1;
		}
		LOGBE("%s ++++  ptmanger->pArry_templete malloc success !!!!",__func__);
		ptmanger->get_all_templete = btl_tmanager_get_all_templete;
		ptmanger->get_templete_by_id = btl_tmanager_get_templete_by_id;
		ptmanger->save_templete_by_id = btl_tmanager_save_templete_by_id ;
		ptmanger->get_all_templete =  btl_tmanager_get_all_templete ;
		ptmanger->get_enrolled_fingerNum =  btl_tmanager_get_fingerNum ;
		ptmanger->get_enroll_fingerid =  btl_tmanager_get_fingerid ;
		ptmanger->create_databasefile =  btl_tmanager_create_databasefile;
		ptmanger->delete_fingerprint_by_id =  btl_tmanager_delete_fingerprint_by_id;

		

		
		for (i = 0; i < ptmanger->tdb.fingernum; i++){
			ptmanger->pArry_templete[i] = (PBL_TEMPLATE)malloc(sizeof(BL_TEMPLATE));
			ptmanger->pArry_templete[i]->pTemplateData = malloc(TEMPLATE_SIZE);
			ptmanger->pArry_templete[i]->templateType = 0;
			ptmanger->pArry_templete[i]->templateSize = 0;
		}
	
		btl_tmanager_get_all_templete(ptmanger->pArry_templete);
		
		LOGBE("%s ----",__func__);
		return  0;
}




pTemplete_manager_t btl_get_templete_manager()
	{
	 	LOGBE("%s ++++++++++++++++++++++++++++++++++++",__func__);
		unsigned int i = 0; 
		if(need_init_manger == 1){
				
				need_init_manger = 0 ;
					
				//malloc space for templetes
			    ptmanger = (pTemplete_manager_t)malloc(sizeof(Templete_manager_t));
				if( ptmanger == NULL){
					
				  LOGBE("%s ++++  ptmanger  malloc error ",__func__);
				  return NULL;

				}
				LOGBE("%s ++++  ptmanger  malloc success !!!!  ",__func__);
				memset(ptmanger,0,sizeof(Templete_manager_t));
				memset(&ptmanger->tdb,0,sizeof(Templete_db_t));
				ptmanger->tdb.fingernum = MAX_FINGER_NUMS  ;//maybe detect from config
				ptmanger->pArry_templete = (PBL_TEMPLATE *)malloc(ptmanger->tdb.fingernum*sizeof(PBL_TEMPLATE));
				ptmanger->PMatch_templete = (PBL_TEMPLATE *)malloc(ptmanger->tdb.fingernum*sizeof(PBL_TEMPLATE));

				ptmanger->pCurrent_templete = (PBL_TEMPLATE)malloc(sizeof(BL_TEMPLATE));
				memset(ptmanger->pCurrent_templete,0,sizeof(BL_TEMPLATE));
				
				if(ptmanger->pArry_templete == NULL || ptmanger->PMatch_templete == NULL)
					{
					LOGBE("%s ++++  ptmanger->pArry_templete  PMatch_templete malloc error ",__func__);
					return NULL;
				}

				for (i = 0; i < ptmanger->tdb.fingernum; i++){
					ptmanger->pArry_templete[i] = (PBL_TEMPLATE)malloc(sizeof(BL_TEMPLATE));
					ptmanger->pArry_templete[i]->pTemplateData = malloc(TEMPLATE_SIZE);
					ptmanger->pArry_templete[i]->templateType = 0;
					ptmanger->pArry_templete[i]->templateSize = 0;

					ptmanger->PMatch_templete[i] = (PBL_TEMPLATE)malloc(sizeof(BL_TEMPLATE));
					ptmanger->PMatch_templete[i]->pTemplateData = malloc(TEMPLATE_SIZE);
					ptmanger->PMatch_templete[i]->templateType = 0;
					ptmanger->PMatch_templete[i]->templateSize = 0;
			}

				
				
				LOGBE("%s ++++  ptmanger->pArry_templete malloc success !!!!",__func__);
				

			 	ptmanger->tdb.fingerindex = 0;
			  	ptmanger->tdb.magicnum  = 0;//0xfdfcfbfa;
			  	ptmanger->tdb.fingerHeaderOffset = sizeof(Templete_db_t) ;
				ptmanger->tdb.fingerSizeOffset = FINGER_SIZE_OFFSET ;
			 	ptmanger->tdb.fingerTypeOffset = FINGER_TYPE_OFFSET;
			 	ptmanger->tdb.fingerTemplateOffset = 10*1024;
			    ptmanger->tdb.MaxtemplateSize  = MAXTEMPLATESIZE ;
			    ptmanger->tdb.fingerUnitTmoduleOffset = ptmanger->tdb.fingerSizeOffset + ptmanger->tdb.fingerTypeOffset + ptmanger->tdb.fingerTemplateOffset + ptmanger->tdb.MaxtemplateSize;
				ptmanger->get_all_templete = btl_tmanager_get_all_templete;
				ptmanger->get_templete_by_id = btl_tmanager_get_templete_by_id;
				ptmanger->save_templete_by_id = btl_tmanager_save_templete_by_id ;
				ptmanger->get_all_templete =  btl_tmanager_get_all_templete ;
				ptmanger->get_enrolled_fingerNum =  btl_tmanager_get_fingerNum ;
				ptmanger->get_enroll_fingerid =  btl_tmanager_get_fingerid ;
				ptmanger->create_databasefile =  btl_tmanager_create_databasefile;
				ptmanger->delete_fingerprint_by_id =  btl_tmanager_delete_fingerprint_by_id;

				

				
				
			 btl_tmanager_create_databasefile();
			 btl_tmanager_get_all_templete(ptmanger->pArry_templete);
	        LOGBE("%s init first time ok !!!!!!",__func__);		
		
		  }else{
		  
		  	//btl_tmanager_get_all_templete(ptmanger->pArry_templete);
			
					
			LOGBE("%s ++++ success ptmanger->need_init has been init !!!",__func__);
			
		  }
		  
	  	LOGBE("%s ------------------------------------------",__func__);		
		return  ptmanger;
}







