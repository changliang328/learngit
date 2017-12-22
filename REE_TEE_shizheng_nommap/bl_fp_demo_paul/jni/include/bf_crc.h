#ifndef __BF_CRC_H__
#define __BF_CRC_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf_log.h"
#include "bf_custom.h"

#define CRC_INIT_BASE	(0xffffffff)


int CRC32_Check_Frame( unsigned int data_crc,unsigned char *buf ,int len);
int CRC32_Check_File(int fd , unsigned int *data_crc);
void CRC32_DTable_Init(void);
int btl_backup_database(int source , int destion);
int CRC32_Check_File_ByPath(const char* path , int offset , unsigned int *data_crc);
int btl_backup_database_byPath(const char* source , const char* destion);





#endif 
