#include "btlfp.h"


#define FINGER_DEBUG            1

const char *strDataFileName = "fingerIndex.dat";
const char *dataIndexFileName= "/data/system/users/0/fpdata/fingerIndex.dat";
const char *dataFolderPath    = "/data/system/users/0/fpdata/";

char mEnrolledIndex[MAX_FINGER_NUMS];

/*----------------------------------------------------------------------
purpose: contact str1  str2  
return :   success 0  fail: -1
------------------------------------------------------------------------*/

char   str_contact(const char *str1, const char *str2, char *result)
{
   // char * result;
   // result = (char*) malloc(strlen(str1) + strlen(str2) + 1);
    if (result == NULL) {
        LOGD("btlfp_file Error: result is null ! \n");
       // exit(EXIT_FAILURE);
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
int btl_ReadIndexFile()
{
    FILE *fpIndex = NULL;
    int i;
#ifdef FINGER_DEBUG
    LOGD("btlfp_file btl_ReadIndexFile");
#endif
    if ((fpIndex = fopen(dataIndexFileName,"rb")) != NULL) {
        fread(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex);
        fclose(fpIndex);
    } else{
    
        LOGE("btl_ReadIndexFile-Open Error");
		return -1;

	}

#ifdef FINGER_DEBUG
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        LOGD("read mEnrolledIndex:%d",mEnrolledIndex[i]);
#endif
    return 0;
}

int btl_get_fid_size(int32_t *fd_index)
{
	int i = 0;
	FILE *fpIndex = NULL;
	#ifdef FINGER_DEBUG
    LOGD("btlfp_file btl_get_fid_size");
	#endif
    if ((fpIndex = fopen(dataIndexFileName,"rb")) != NULL) {
	    fread(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex);
	    fclose(fpIndex);
	} else
    	LOGE("btl_ReadIndexFile-Open Error");
	for (i = 0; i < MAX_FINGER_NUMS; i++)
	{
		fd_index[i] = mEnrolledIndex[i];
		if(mEnrolledIndex[i] == 0)
			break;			
	}
	LOGD("btlfp_file fid size = %d", i);
	return i;
}


int btl_get_fid_count(uint32_t *fd_index)
{
	int i = 0,j = 0;
	FILE *fpIndex = NULL;
	#ifdef FINGER_DEBUG
    LOGD("btlfp_file btl_get_fid_count");
	#endif
    if ((fpIndex = fopen(dataIndexFileName,"rb")) != NULL) {
	    fread(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex);
	    fclose(fpIndex);
	} else
    	LOGE("btl_ReadIndexFile-Open Error");
	for (i = 0; i < MAX_FINGER_NUMS; i++)
	{
		fd_index[i] = mEnrolledIndex[i];
		if(mEnrolledIndex[i] != 0)
			j++;			
	}
	#ifdef FINGER_DEBUG
	LOGD("btlfp_file btl_get_fid_count fid count  = %d", j);
	#endif
	return j;
}


/*----------------------------------------------------------------------
purpose: write finger index file
return :   success 0  fail: -1 (open fail)    -2(write fail)
------------------------------------------------------------------------*/
static int32_t btl_WriteIndexFile()
{
    FILE *fpIndex = NULL;
    int i = 0;
#ifdef FINGER_DEBUG
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        LOGD("btlfp_file btl_WriteIndexFile mEnrolledIndex:%d",mEnrolledIndex[i]);
#endif
    if ((fpIndex = fopen(dataIndexFileName,"wb")) != NULL) {
		if(fwrite(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex) != 5)
			{
				LOGE("btlfp_file btl_WriteIndexFile error");
				 fclose(fpIndex);
				 return -2;
		}
        fclose(fpIndex);
    } else{
    	return -1;
        LOGE("btlfp_file btl_WriteIndexFile  Open Error");
    }
    return 0;
}

static int32_t btl_create_databasefile()
{
    int i ;
    FILE *fpIndex = NULL;
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        mEnrolledIndex[i] = 0;

    if((fpIndex  = fopen(dataIndexFileName,"r")) == NULL) {
        btl_WriteIndexFile();
    } else {
        fclose(fpIndex);
        btl_ReadIndexFile();
    }
    LOGD("btl_create_database %s",dataIndexFileName);
    return 0;
}




/*----------------------------------------------------------------------
purpose: Generate a filename by finger id
return :   void
------------------------------------------------------------------------*/
void generateFileName(char *fileName, int fid)
{
    char tmp[10];
    tmp[0] = 0;
    sprintf(tmp,"%d",fid);
    strcpy(fileName,dataFolderPath);
    strcat(fileName,tmp);
}

int btl_file_saveTemplate(unsigned char *pTemplateBuf, int template_size, int fid)
{
    char finger_filename[128];
    finger_filename[0] = 0;
	int ret = 0;

    generateFileName(finger_filename,fid);
    mEnrolledIndex[fid-1] = fid;

    char savestr[10];
    //sprintf(savestr,"%s",template_size);
    myitoa(template_size,savestr,10);
	LOGD("btlfp_file btl_file_saveTemplate savestr %s",savestr);
    FILE* fp = fopen(finger_filename, "wb");
    if (fp == NULL) 
        return -10;
	
    uint8_t *wg_buffer = (uint8_t *)malloc(template_size + 10);
    memcpy(wg_buffer, savestr, 10);
    memcpy(wg_buffer+10, pTemplateBuf, template_size);


   
    fwrite(wg_buffer, template_size + 10, 1, fp);
    fclose(fp); 
    free(wg_buffer);
    wg_buffer = NULL;

   
	   ret = btl_WriteIndexFile();
	   
	   if(ret != 0)
		   {
		   LOGD("btlfp_file btl_reset_databasefile error %d",ret);
		   return -1;
	   }
   

    return 0;
}


/*----------------------------------------------------------------------
purpose: delete   Template by fid 
return :   success 0  fail: -1
------------------------------------------------------------------------*/

int btl_file_delTemplate(int fid)
{
    char finger_filename[128];
    finger_filename[0] = 0;
	int ret = 0;

    generateFileName(finger_filename,fid);
   // mEnrolledIndex[fid-1] = 0;
		if(access(finger_filename,F_OK) == 0)
			{
					if(remove(finger_filename) != 0)
					{
					LOGD(" %s finger_filename:%s  remove error  !!!",__func__,finger_filename);
					return -2;
				}

			}else{

			LOGD(" %s finger_filename:%s  access error	file is not exist",__func__,finger_filename);
			return -1;

		}
		

	
	    //ret = btl_WriteIndexFile();
		
		//if(ret != 0)
		//	{
		//	LOGD("btlfp_file btl_reset_databasefile error %d",ret);
		//	return -1;
		//}
	

    return 0;
}



int btl_file_getTemplate( uint8_t* pTemplateBuffer , int size_read ,int fid)
{
			char ret = 0;
			char res_file_name[128];
			char *num_char = (char*)malloc(sizeof(char));
				
				if(num_char == NULL) {
					return 0;
				}
			    LOGE("btlfp_file btl_file_getTemplate  fid:%d",fid);	
				myitoa(fid,num_char,10);
				LOGE("btlfp_file btl_file_getTemplate  num_char:%s",num_char);
			    ret = str_contact(dataFolderPath, num_char,res_file_name);
				LOGE("btlfp_file btl_file_getTemplate  res_file_name:%s",res_file_name);
				if(num_char != NULL) {
					free(num_char);
					num_char = NULL;
				}

				FILE *fp1 = fopen(res_file_name, "rb");
		
				if (fp1 == NULL) {
					LOGE(" open file %s fail",res_file_name);
					return -1;
				} else {
					fseek(fp1, 10L, 0);
					fread(pTemplateBuffer, size_read, 1, fp1);
					fclose(fp1);
				}

		return 0;

}




int btl_file_resetTemplate()
{
    char finger_filename[128];
    finger_filename[0] = 0;
	int i = 0;


	for(i = 1 ;i <= MAX_FINGER_NUMS ;i++)
		{
			memset(finger_filename,0,128);
		    generateFileName(finger_filename,i);
		    
			if(access(finger_filename,F_OK) == 0)
				{
					if(remove(finger_filename) != 0)
					{
					
						LOGD(" %s finger_filename:%s  remove error  !!!",__func__,finger_filename);
						return -2;
				}
			}
	  }
    return 0;
}



int btl_file_getTemplateSize(int fid)

{
			int index[10];
			int size_read = 0;
			char read_size[10] = {0x00};
			char res_file_name[128];
			char ret = 0;

		
			char *num_char = (char*)malloc(sizeof(char));
	
			if(num_char == NULL) {
				return 0;
			}
			LOGE("btlfp_file btl_file_getTemplateSize  fid:%d",fid);
			myitoa(fid,num_char,10);
			LOGE("btlfp_file btl_file_getTemplateSize  num_char:%s",num_char);
			ret = str_contact(dataFolderPath, num_char,res_file_name);
			LOGE("btlfp_file btl_file_getTemplateSize  res_file_name:%s",res_file_name);
			if(num_char != NULL) {
				free(num_char);
				num_char = NULL;
			}
	

			FILE *fp1 = fopen(res_file_name, "rb");
	
			if (fp1 == NULL) {
				LOGE("btlfp_file btl_file_getTemplateSize  file %s fail",res_file_name);
				return -1;
			} else {
				fseek(fp1, 0L, 0);
				fread(read_size, 10, 1, fp1);
				size_read = atoi(read_size);
				LOGD("btlfp_file file size is %d \n",size_read);
				fclose(fp1);
			}

	    return  size_read;

}


int btl_file_fingerid()
{
    int i=0,FingerNum = 0;
    FILE *fileFd = NULL;
    
    btl_ReadIndexFile();
    for (i = 1 ; i <= 5; i++) {
        if (mEnrolledIndex[i-1] == 0) {
            break;
        }
		
    }
    if(i > 5)
	i=5;
    return i;
}

int btl_file_fingerNum(int *buffer)
{
    int i=0,FingerNum = 0;
    FILE *fileFd = NULL;
    
    btl_ReadIndexFile();
    for (i = 1 ; i <= 5; i++) {
        if (mEnrolledIndex[i-1] != 0) {
	    buffer[FingerNum] = mEnrolledIndex[i-1];
            FingerNum++;
        }
		
    }
	
    return FingerNum;
}

int file_delete_fingerprint(int fid)
{
	int ret = 0 ;
	LOGD("btlfp_file file_delete_fingerprint  fid:  %d",fid);
	ret = btl_ReadIndexFile();
    if(ret != 0)
    	{
    	
		LOGD("btlfp_file file_delete_fingerprint  btl_ReadIndexFile error %d",ret);
		return -1;

	}
	
    mEnrolledIndex[fid-1] = 0;
	
    ret = btl_WriteIndexFile();
	
	    if(ret != 0)
			
    	{
    	
		LOGD("btlfp_file file_delete_fingerprint  btl_WriteIndexFile error %d",ret);
		return -2;

	}

	ret = btl_file_delTemplate(fid);

	    if(ret != 0)
			
    	{
    	
		LOGD("btlfp_file file_delete_fingerprint  btl_WriteIndexFile error %d",ret);
		return -3;

	}
    return 0;
}

static int32_t btl_reset_databasefile()
{
    int i ;
	int ret = 0;
    FILE *fpIndex = NULL;
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        mEnrolledIndex[i] = 0;

    ret = btl_WriteIndexFile();
	
	if(ret != 0)
		{
		LOGD("btlfp_file btl_reset_databasefile error %d",ret);
		return -1;
	}

	ret = btl_file_resetTemplate();
	
	if(ret != 0)
		
		{
		
		LOGD("btlfp_file btl_file_resetTemplate error %d",ret);
		return -2;

	}
	
    return 0;
}


int btl_api_createFile()
{
    btl_create_databasefile();
    return 0;
}

