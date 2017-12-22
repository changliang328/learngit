#include "bf_crc.h"


static unsigned int DTable_CRC32[256];
#define CRC_32_POLYNOMIALS 0X04C11DB7
#define BUFSIZE     1024*4


void CRC32_DTable_Init(void)
{
    unsigned int i32, j32;
    unsigned int nData32 = 0;
    unsigned int nAccum32 = 0;

    for (i32 = 0; i32 < 256; i32++) {
        nData32 = (unsigned int) (i32 << 24);
        nAccum32 = 0;
        for (j32 = 0; j32 < 8; j32++) {
            if ((nData32 ^ nAccum32) & 0x80000000) {
                nAccum32 = (nAccum32 << 1) ^ CRC_32_POLYNOMIALS;
            } else {
                nAccum32 <<= 1;
            }
            nData32 <<= 1;
        }
        DTable_CRC32[i32] = nAccum32;
   } 
}  

unsigned int CRC32_Process_Byte(unsigned int dwCRC , unsigned char*pchMsg,unsigned short wDataLen)
{
    unsigned char chChar = 0x00;
    while (wDataLen--) {
        chChar = *pchMsg++;
        dwCRC = DTable_CRC32[((dwCRC >> 24) ^ chChar) & 0xff] ^ (dwCRC << 8);
    }

    return dwCRC;
}


int CRC32_Check_Frame( unsigned int data_crc,unsigned char *buf ,int len)
{

		
		data_crc = CRC32_Process_Byte(data_crc, buf, len);    
		return data_crc;  
    
}



int CRC32_Check_File(int fd , unsigned int *data_crc)
{
	int nread = 0;  
    unsigned char buf[BUFSIZE];  
    unsigned int crc = 0xffffffff;   
		while ((nread = bf_tee_fd_read(fd, buf, BUFSIZE)) > 0) {  
			   
		        crc = CRC32_Process_Byte(crc,buf, nread);  
		   }  
		
		 *data_crc = crc;  
		BF_LOG("###############  fd = %d data_crc = %x ",fd,*data_crc);  
			if (nread < 0) {  
		       BF_LOG("read error .\n"); 
			   *data_crc = 0;
				return -1;  
		   }  
		return 0;  
    
}



int btl_backup_database(int source , int destion)
{
	char *buf;

	int len = 0;
	int buflen = 200*1024;
	int i = 0;
	
	BF_LOG("##### start   +++++++++");
	
		if(source < 0)
		{
			BF_LOG("source fd  error+++++++++");
			return -1;
		}

	
		if(destion < 0)
		{
			 BF_LOG("destion fd	error+++++++++");
			 return -1;
		}

		buf = (char*)bf_tee_malloc(buflen);
		
		
		if(buf == NULL)
			{
			BF_LOG(" malloc	error+++++++++");
			return -1;

		}
		
		bf_tee_memset(buf,0,buflen);
		
		while((len = bf_tee_fd_read(source,buf,buflen)) > 0)
		{
			bf_tee_fd_write(destion,buf,len);
			i++;
		}
		BF_LOG(" write times:%d --------",i);
		bf_tee_free(buf);
		buf = NULL;
		
		BF_LOG(" #####  end   --------");
		return 0;
}


int CRC32_Check_File_ByPath(const char* path , int offset , unsigned int *data_crc)
{
	int nread = 0; 
    unsigned char buf[BUFSIZE];  
    unsigned int crc = 0xffffffff;   
		while ((nread = bf_tee_fs_read(path, buf, offset, BUFSIZE)) > 0) {  
			    offset += nread;
		        crc = CRC32_Process_Byte(crc,buf, nread);  
		   }  
		
		 *data_crc = crc;  
		BF_LOG("###############  path= %s data_crc = %x ",path,*data_crc);  
			if (nread < 0) {  
		       BF_LOG("read error .\n"); 
			   *data_crc = 0;
				return -1;  
		   }  
		return 0;  
    
}



int btl_backup_database_byPath(const char* source , const char* destion)
{
	char *buf;

	int len = 0;
	int buflen = 200*1024;
	int offset = 0;
	int i = 0;
	
	BF_LOG("##### start   +++++++++");
	
		if(source ==  NULL)
		{
			BF_LOG("source path  error+++++++++");
			return -1;
		}

	
		if(destion == NULL)
		{
			 BF_LOG("destion path	error+++++++++");
			 return -1;
		}

		buf = (char*)bf_tee_malloc(buflen);
		
		
		if(buf == NULL)
			{
			BF_LOG(" malloc	error+++++++++");
			return -1;

		}
		
		bf_tee_memset(buf,0,buflen);
		
		while((len = bf_tee_fs_read(source,buf,offset,buflen)) > 0)
		{
			bf_tee_fs_write(destion,buf,offset,len);
			offset += len;
			i++;
		}
		BF_LOG(" write times:%d --------",i);
		bf_tee_free(buf);
		buf = NULL;
		
		BF_LOG(" #####  end   --------");
		return 0;
}


