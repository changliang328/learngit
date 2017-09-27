/*
 * Copyright (C) 2016 BetterLife Corporation. All rights reserved.
 *
 */
#include "btlfp.h"
#include "sqlite3.h"

//const char *table_basic = "create table tb(id integer primary key autoincrement, name TEXT , fid INTEGER, gid INTEGER, datafile CHAR(32))";
const char *table_basic =    "create table tb(fid INTEGER, gid INTEGER, datafile CHAR(32))";
const char *table_finger_1 = "create table if not exists finger(id integer primary key, fid INTEGER, uid INTEGER, used INTEGER, fingername varchar(128),fingerdata BLOB)";

const char *format_insert_finger = "insert into finger(uid,fid,used, fingername,fingerdata) values(?,?,?,?,?)";
const char *format_select_finger = "select * from finger where fid = %d";
const char *format_delete_finger = "delete from finger where fid = %d";
const char *format_inqure_finger = "select * from finger";

// const char *insert_finger1 = "insert into finger1(data) values (1, 'stenve')"
const char *database_name  = "fingerprint.db";

int inquire_all_fingerprint(sqlite3 *db)
{
    char format_string[] = "fingerprnt%d";
    char tempBuf[128];
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int count = 0;


    if (sqlite3_prepare_v2(db, format_inqure_finger, -1, &stat, 0) != SQLITE_OK) {
        LOGE ("%s, sqlite3_prepare error \n", __func__);
		sqlite3_finalize(stat);
        return -1;
    }

    while (sqlite3_step(stat) == SQLITE_ROW)
        count++;

    sqlite3_finalize(stat); //释放内存

    LOGD ("count:%d\n",count);


    return count;
}

int inquire_database_by_fid(sqlite3 *db, int fid)
{
    char tempBuf[128];
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int count = 0;
    int result;

    sprintf(tempBuf,format_select_finger,fid);

    //LOGD(tempBuf);

    if (sqlite3_prepare_v2(db, tempBuf, -1, &stat, 0) != SQLITE_OK) {
        LOGE ("%s, sqlite3_prepare error %s\n", __func__,sqlite3_errmsg(db));
		sqlite3_finalize(stat);
        return -1;
    }

    while (sqlite3_step(stat) == SQLITE_ROW)
        count++;

    sqlite3_finalize(stat); //释放内存

    LOGD ("%s,count:%d\n",__func__,count);

    return count;
}

int insert_fingerprint(sqlite3 *db,int  __unused uid, int fid, unsigned char *buffer, int size)
{
    char format_string[] = "fingerprnt%d";
    char tempBuf[20];
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int result;

    sprintf(tempBuf,format_string, fid);
    //LOGD (tempBuf);

    if (sqlite3_prepare_v2(db, format_insert_finger, -1, &stat, 0 ) != SQLITE_OK) {
        LOGE ("%s,sqlite3_prepare error \n",__func__);
		sqlite3_finalize(stat);
        return -1;
    }

    if (sqlite3_bind_int(stat, 1, uid) != SQLITE_OK) {
        LOGE ("%s,sqlite3_bind_int error - 1\n",__func__);
		sqlite3_finalize(stat);
        return -1;
    }

    if (sqlite3_bind_int(stat, 2, fid) != SQLITE_OK) {
        LOGE ("%s,sqlite3_bind_int error - 2\n",__func__);
		sqlite3_finalize(stat);
        return -1;
    }

    if (sqlite3_bind_int(stat, 3, 1) != SQLITE_OK) {
        LOGE ("%s,sqlite3_bind_int error - 3\n",__func__);
		sqlite3_finalize(stat);
        return -1;
    }

    if (sqlite3_bind_text(stat, 4, tempBuf,strlen(tempBuf),NULL) != SQLITE_OK) {
        LOGE ("%s,sqlite3_bind_text error \n",__func__);
        sqlite3_finalize(stat);
        return -1;
    }

    // 准备插入数据
    if (sqlite3_bind_blob(stat, 5, buffer, size, NULL ) != SQLITE_OK) {
        LOGE ("%s,sqlite3_bind_blob error \n",__func__);
        sqlite3_finalize(stat);
        return -1;
    }
    //把内容和字段绑

    result = sqlite3_step(stat);     //执行
    sqlite3_finalize(stat); //释放内存

    return 0;
}


int inquire_fingerprint_fid(sqlite3 *db, int32_t *buffer)
{
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int result;
    int count = 0;

    // 准备读入数据
    if (sqlite3_prepare_v2(db, format_inqure_finger, -1, &stat, 0) != SQLITE_OK) {
        LOGE ("%s, sqlite3_prepare error:%s \n", __func__, sqlite3_errmsg(db));
		sqlite3_finalize(stat);
        return -1;
    }

    // 读入数据
    while (sqlite3_step(stat) == SQLITE_ROW) {
        buffer[count++] = sqlite3_column_int(stat,1);
    }

    sqlite3_finalize(stat); //释放内存


    return count;
}


int sql_update_blob(const char *dbName,char *buffer,int size)
{
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int result;
    sqlite3 *db;
    int count = 0;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);
	if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_open_v2 fail:%s", __func__,sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }
    // 准备读入数据
    LOGD ("size:%d\n",size);
	result = sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0,0,0);
    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_exec fail:%s", __func__,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_prepare_v2(db, "update finger set fingerdata=? where fid=2", -1, &stat, 0) != SQLITE_OK) {
        LOGD ("%s, sqlite3_prepare error \n", __func__);
		sqlite3_finalize(stat);
		sqlite3_close(db);
        return -1;
    }
    if (sqlite3_bind_blob(stat, 1, buffer, size, NULL) != SQLITE_OK) {
        LOGD ("sqlite3_bind_int error \n");
		sqlite3_finalize(stat);
		sqlite3_close(db);
        return -1;
    } //把内容和字段绑
    result = sqlite3_step(stat);     //执行
    sqlite3_finalize(stat); //释放内存
    sqlite3_close(db);
    return 0;
}

int sql_delete_fingerprint(const char *dbName, int  __unused uid, int fid)
{
    char tempBuf [128];
    int result;

    sqlite3 *db;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);

    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_open_v2 fail:%s", __func__,sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

	result = sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0,0,0);
    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_exec fail:%s", __func__,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
	
    sprintf (tempBuf, format_delete_finger, fid);

    result = sqlite3_exec(db,tempBuf,NULL,NULL,NULL);

    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_exec fail:%s", __func__,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sql_load_fingerprint(const char *dbName, int  __unused uid, int fid, unsigned char *buffer, int *size)
{
    char format_string[] = "fingerprnt%d";
    char tempBuf[128];
    sqlite3_stmt  *stat; // 写二进制数据时要用的结构
    int result;
    char *pdbBuf;
    const char *pname;
    int buflen;
    int tfid, tuid, used;
    char *tempName;
    int i;
    sqlite3 *db;
    int count = 0;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);
	if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_open_v2 fail:%s", __func__,sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    *size = 0;

    sprintf(tempBuf,format_select_finger, fid);
    // 准备读入数据
    if (sqlite3_prepare_v2(db, tempBuf, -1, &stat, 0) != SQLITE_OK) {
        LOGE ("%s, sqlite3_prepare error \n", __func__);
		sqlite3_finalize(stat);
		sqlite3_close(db);
        return -1;
    }
    // 读入数据
    while (sqlite3_step(stat) == SQLITE_ROW) {
        tfid = sqlite3_column_int(stat,1);
        //LOGD ("fid:%d \n",tfid);
        tuid = sqlite3_column_int(stat,2);
        //LOGD ("uid:%d \n",tuid);
        used = sqlite3_column_int(stat,3);
        //LOGD ("used:%d \n",used);
        pname = (char *)sqlite3_column_text(stat,4);
        int uflen = sqlite3_column_bytes(stat, 4);
        //LOGD ("uflen:%d \n",uflen);
        tempName = (char *)malloc(uflen+1);
        for (i = 0; i < uflen; i++)
            tempName[i] = pname[i];

        tempName[uflen] = 0;

        //LOGD (tempName);
        //LOGD ("\n");

        free(tempName);

        //printf("%s",pname);
        pdbBuf = (char *)sqlite3_column_blob(stat,5);//得到纪录中的BLOB字段
        buflen = sqlite3_column_bytes(stat, 5);       //得到字段中数据的长度
        //LOGD ("buflen:%d \n",buflen);
        memcpy(&buffer[*size],pdbBuf,buflen);
        *size += buflen;
    }     //执行

    sqlite3_reset(stat);
    sqlite3_finalize(stat); //释放内存
    LOGD ("%s size:%d \n",__func__,*size);
    sqlite3_close(db);
    return 0;
}


int sql_insert_fingerprint(const char *dbName, int  __unused uid, int fid, unsigned char *buffer, int size)
{
    int result;
    sqlite3 *db;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);
	if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_open_v2 fail:%s", __func__,sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }
	result = sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0,0,0);
    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_exec fail:%s", __func__,sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
	
    insert_fingerprint(db,uid,fid, buffer,size);

    sqlite3_close(db);
    return 0;
}

int sql_inquire_fingerpint_fid(const char *dbName, int32_t *buffer)
{
    int result;
    sqlite3 *db;
    int count = 0;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);
    if( result != SQLITE_OK ) {
        LOGE("%s,sqlite3_open_v2 fail:%s", __func__,sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    // 准备读入数据
    count  = inquire_fingerprint_fid(db,buffer);
    sqlite3_close(db);
    return count;
}


int sql_inquire_all_fingerprint(const char *dbName)
{
    int result;
    sqlite3 *db;
    int count = 0;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL);

    if( result != SQLITE_OK ) {
        LOGE("sqlite3_open_v2 fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    result = sqlite3_exec(db, table_finger_1, NULL, NULL, NULL);

    if( result != SQLITE_OK ) {
        LOGE("sqlite3_exec fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    count = inquire_all_fingerprint(db);

    sqlite3_close(db);

    return count;
}

int sql_inquire_database_by_fid(const char *dbName, int fid)
{
    int result;
    sqlite3 *db;
    int count = 0;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE,NULL);

    if( result != SQLITE_OK ) {
        LOGE("sqlite3_open_v2 fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return 0;
    }

    count = inquire_database_by_fid(db,fid);

    sqlite3_close(db);

    return count;
}

int sql_create_database(const char *dbName)
{
    sqlite3 *db;
    int result;

    result = sqlite3_open_v2(dbName, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,NULL);

    if( result != SQLITE_OK ) {
        LOGE("sqlite3_open_v2 fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    result = sqlite3_exec(db, table_finger_1, NULL, NULL, NULL);

    if( result != SQLITE_OK ) {
        LOGE("sqlite3_exec fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    return 0;
}

int sql_backup_database(const char *dbNameFrom, const char *dbNameTo)
{
    sqlite3 *db;
	sqlite3 *pTo;
	sqlite3_backup *pBackup;
    int result;

	result = sqlite3_open_v2(dbNameFrom, &db, SQLITE_OPEN_READWRITE,NULL);
    if( result != SQLITE_OK ) {
        LOGE("sqlite3_open_v2 fail:%s", sqlite3_errmsg(db));
		sqlite3_close(db);
        return 0;
    }

    result = sqlite3_open_v2(dbNameTo, &pTo, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,NULL);
    if( result != SQLITE_OK ) {
        LOGE("sqlite3_open_v2 fail:%s", sqlite3_errmsg(pTo));
		sqlite3_close(db);
		sqlite3_close(pTo);
        return -1;
    }
	
	pBackup = sqlite3_backup_init(pTo, "main", db, "main");
	if(pBackup == NULL) {
        LOGE("sqlite3_backup_init fail:%s", sqlite3_errmsg(pTo));
		sqlite3_close(db);
		sqlite3_close(pTo);
        return -1;
    }
	
	sqlite3_backup_step(pBackup, -1);
	sqlite3_backup_finish(pBackup);
	
	sqlite3_close(db);
	sqlite3_close(pTo);
	return 0;
}

#ifdef DEBUG_SQL
int main(int args , char *argv[])
{
    char sql[128];
    sqlite3 *db;
    FILE *fd;
    char *bufferR = 0;
    char *bufferW = 0;
    int sizeR,sizeW;
    int i;


    if (args < 2) {
        printf ("%s,%s \n",argv[0],argv[1]);
        printf ("error ");
        return 0;
    }
    sqlite3_open(database_name, &db);  //打开（或新建）一个数据库
    memset(sql, '\0', 128);
    /* 新建一张表 */
    //strcpy(sql, "create table tb(id INTEGER PRIMARY KEY, data TEXT)");
    if (args >= 2) {
        if (argv[1][0] == '1') {
            strcpy(sql,table_basic);
            sqlite3_exec(db, sql, NULL, NULL, NULL);

            strcpy(sql,table_finger_1);
            sqlite3_exec(db, sql, NULL, NULL, NULL);
        }
    }

    /* 新建一个文件，把数据库的查询结果保存在该文件中 */
    bufferW = malloc(250*1024);
    if (bufferW == 0)return -1;
    bufferR = malloc(250*1024);
    if (bufferR == 0)
    {
    	free(bufferW);
    	return -1;
    }
    fd = fopen("./1.dat", "rb");
    fseek(fd,0,SEEK_END);
    sizeW = ftell(fd);
    fseek(fd,0,SEEK_SET);
    fread(bufferW, sizeof(char), sizeW, fd);
    fclose(fd);

    if (argv[1][0] == '1')
        ;//insert_fingerprint(db,0,1,bufferW,sizeW);
    else
        load_fingerprint(db,1,1,bufferR,&sizeR);



    //for (i = 0; i < 5; i++)
    //  read_blob(db,i+1,bufferR,&sizeR);

    sqlite3_close(db); //关闭数据库
    //if (sizeR != sizeW)
    //  printf("size not match \n");

    //for (i = 0; i < sizeR; i++)
    //  if (bufferR[i] != bufferW[i])
    //	   printf("content don't match on %d\n",i);
    free((void*)bufferW);
    free((void*)bufferR);

    return 0;
}
#endif
