#include "btlfp.h"


#define FINGER_DEBUG            1

const char *strDataFileName = "fingerIndex.dat";
const char *dataIndexFileName= "/data/system/users/0/fpdata/fingerIndex.dat";
const char *dataFolderPath    = "/data/system/users/0/fpdata/";

char mEnrolledIndex[MAX_FINGER_NUMS];

char * str_contact(const char *str1, const char *str2)
{
    char * result;
    result = (char*) malloc(strlen(str1) + strlen(str2) + 1);
    if (!result) {
        LOGD("Error: malloc failed in concat! \n");
        exit(EXIT_FAILURE);
    }
    strcpy(result, str1);
    strcat(result, str2);
    return result;
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
    LOGD("btl_ReadIndexFile");
#endif
    if ((fpIndex = fopen(dataIndexFileName,"rb")) != NULL) {
        fread(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex);
        fclose(fpIndex);
    } else
        LOGE("btl_ReadIndexFile-Open Error");
#ifdef FINGER_DEBUG
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        LOGD("read mEnrolledIndex:%d",mEnrolledIndex[i]);
#endif
    return 1;
}


/*----------------------------------------------------------------------
purpose: write finger index file
return :   success 1
------------------------------------------------------------------------*/
static int32_t btl_WriteIndexFile()
{
    FILE *fpIndex = NULL;
    int i = 0;
#ifdef FINGER_DEBUG
    LOGD("btl_WriteIndexFile");
    for (i = 0; i < MAX_FINGER_NUMS; i++)
        LOGD("write mEnrolledIndex:%d",mEnrolledIndex[i]);
#endif
    if ((fpIndex = fopen(dataIndexFileName,"wb")) != NULL) {
        fwrite(mEnrolledIndex, sizeof(char), MAX_FINGER_NUMS,fpIndex);
        fclose(fpIndex);
    } else
        LOGE("btl_WriteIndexFile-Open Error");
    return 1;
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

    generateFileName(finger_filename,fid);
    mEnrolledIndex[fid-1] = fid;

    char savestr[10];
    sprintf(savestr,"%d",template_size);
    uint8_t *wg_buffer = (uint8_t *)malloc(template_size + 10);
    memcpy(wg_buffer, savestr, 10);
    memcpy(wg_buffer+10, pTemplateBuf, template_size);

    FILE* fp = fopen(finger_filename, "wb");
    if (fp == NULL) 
        return -10;
   
    fwrite(wg_buffer, template_size + 10, 1, fp);
    fclose(fp); 

    btl_WriteIndexFile();

    return 0;
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
    btl_ReadIndexFile();
    mEnrolledIndex[fid-1] = 0;
    btl_WriteIndexFile();
    return 0;
}

int btl_api_createFile()
{
    btl_create_databasefile();
    return 0;
}

