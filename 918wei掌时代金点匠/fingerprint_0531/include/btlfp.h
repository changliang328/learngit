/*
 * Copyright (C) 2016 BetterLife.Co.Ltd. All rights  reserved.
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <android/log.h>
#include <string.h>

#include "btlcustom.h"
#include <pthread.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>



#define BL229X_IOCTL_MAGIC_NO         0xFC
#define BL229X_Adjustment_AGC         _IOW(BL229X_IOCTL_MAGIC_NO, 3, uint32_t)
#define BL229X_INIT				      _IO(BL229X_IOCTL_MAGIC_NO, 0)
#define BL229X_GETIMAGE    		      _IOW(BL229X_IOCTL_MAGIC_NO, 1, uint32_t)
#define BL229X_INTERRUPT_MODE         _IOW(BL229X_IOCTL_MAGIC_NO, 2, uint32_t)
#define BL229X_POWERDOWN_MODE         _IO(BL229X_IOCTL_MAGIC_NO,4)




#define BL229X_GET_ID	              _IOWR(BL229X_IOCTL_MAGIC_NO, 9,  uint32_t)
#define BL229X_GET_CHIP_INFO	      _IOWR(BL229X_IOCTL_MAGIC_NO, 10, uint32_t)

#define BL229X_ENABLE_INT	          _IOWR(BL229X_IOCTL_MAGIC_NO, 10,uint32_t)
#define BL229X_INIT_ARGS	          _IOWR(BL229X_IOCTL_MAGIC_NO, 11,uint32_t)
#define BL229X_GAIN_ADJUST            _IOWR(BL229X_IOCTL_MAGIC_NO, 12, uint32_t)
#define BL229X_REPROTKEY_NEEDED       _IOWR(BL229X_IOCTL_MAGIC_NO, 15, uint32_t)
#define BL229X_WAIT_FINGER_UP	      _IOWR(BL229X_IOCTL_MAGIC_NO, 16, uint32_t)


#define ERR_CANCEL                    2047
#define ERR_EMPTY                     2015
#define ERR_IO                        2014
#define ERR_AGC                       2016
#define ERR_ENROLL                    2019
#define ERR_MEMORY                    2020
#define ERR_PARAMS                    2021
#define ERR_LOADDB                    2022
#define ERR_EXIST					  2017



#define ERR_FPDOWN                    0


#define ERR_OK                        0


#define MAX_CONTRAST                  0xf8
#define MIN_CONTRAST                  0x20



#define DEFAULT_ENROLL_SCORE_THRESHOLD       70
#define DEFAULT_ENROLL_AREA_THRESHOLD        17

#define DEFAULT_MATCH_SCORE_THRESHOLD         50
#define DEFAULT_MATCH_AREA_THRESHOLD          9

#define BADIMAGE_TIMEOUT_COUNTS       3
#define IOACCESS_TIMEOUT_COUNTS       30


#define  MAX_FINGER_NUMS              5
#define  PATH_SIZE                    128
#define  ERROR_CACNCEL                -2047
#define  ERROR_UP                     -2015
#define  ERROR_MATCH                  0
#define  DEFAULT_MATCH_FAILED_TIMES        5
#define  DEFAULT_MATCH_CK_FPUP_FRAMES     100
#define  DEFAULT_ENROLL_CK_FPUP_FRAMES    100
#define  DEFAULT_MAX_ERROR_COUNTS         60

#define  DEFAULT_CONTRAST_FOR_FINGERUP 100
#define  DEFAULT_CONTRAST_FOR_CAPTURE  100

#define  DEFAULT_MATCH_TRYTIMES        5
#define  DEFAULT_ENROLL_TRYTIMES       8

#define  DEFAULT_INTENSITY_THRESHOLD   5


#define  DEFAULT_CONTRAST_HIGH         0xb4
#define  DEFAULT_CONTRAST_LOW          0x36

#define  WHITE_POINT_VALUE             0xf0
#define  WHITE_POINT_COUNTS            2000

#define  DEFAULT_SHOTKEY_DISABLE         1

#define  DEFAULT_ENROLL_COUNTS         10

#define  DEFAULT_FAR          BF_PB_FAR_50000

extern int TEMPLATE_SIZE;

/* Decisions. */
#define DECISION_MATCH       1
#define DECISION_NON_MATCH   0

typedef struct fingerprint_item {
    hw_auth_token_t auth_token;
    uint32_t  fid;
    uint32_t  gid;
    uint32_t  used;
} fingerprint_item_t;


typedef struct fingerprint_param {
    fingerprint_device_t *dev;
    hw_auth_token_t auth_token;
    uint32_t gid;
    uint32_t fid;
    uint32_t timeout_sec;
    uint64_t operation_id;
} fingerprint_param_t;

typedef struct fingerprint_update_data{   
	unsigned char *captured_image;
	unsigned char *matched_template_data;
	int  matched_template_type;
	int  matched_template_size;
	int  fid;
	int  uid;
}fingerprint_update_data_t;


typedef struct btl_fingerprint_template{
	unsigned char *fp_template_data;
	int fp_template_type;
	int fp_template_size;
}btl_fingerprint_template_t;

typedef struct fingerprint_var {
    uint64_t        token;
    uint32_t        gid;
    uint32_t        fid;
    int32_t         cancel;
    pthread_t       tid;
    pthread_mutex_t fingerprint_lock;
    pthread_mutex_t count_lock;
    pthread_cond_t  count_nonzero;
	pthread_mutex_t db_lock;
	pthread_mutex_t update_lock;
	btl_fingerprint_template_t enrolled_template[5];
	int             matched_index;
	unsigned char   *captured_image;
	fingerprint_update_data_t dyn_update_data;
    uint32_t        dacp;
    uint32_t        is_need_reload_templates;
} fingerprint_var_t;

//#define SAVE_IMAGE (1)

struct fingerprintd_params_t {
    uint8_t def_contrast;
    uint8_t def_ck_fingerup_contrast;
    uint8_t def_enroll_ck_fingerup_timeout;
    uint8_t def_match_ck_fingerup_timeout;
    uint8_t def_match_failed_times;
    uint8_t def_enroll_try_times;
    uint8_t def_match_try_times;
    uint8_t def_intensity_threshold;
    uint8_t def_contrast_high_value;
    uint8_t def_contrast_low_value;
    uint8_t def_match_quality_score_threshold;
    uint8_t def_match_quality_area_threshold;
    uint8_t def_enroll_quality_score_threshold;
    uint8_t def_enroll_quality_area_threshold;
    uint8_t def_shortkey_disable;
    uint8_t def_far_rate;
    uint8_t def_max_samples;
    uint8_t def_debug_enable;
    uint8_t def_contrast_direction;
    uint8_t def_update;
    uint8_t def_step_counts;
    uint8_t def_algorithm_type;
    uint16_t reserved1;	    
    uint8_t def_enhance_image;
    uint8_t def_gain;
	uint8_t finger_fd_thred_high;
	uint16_t chip_id;
};

typedef struct bl229x_chip_info{
	uint32_t  chip_id_num;
	uint8_t  chip_type;
	uint8_t chip_driver_type;
}chip_info_t;







#ifndef _btlfp_api
#define _btlfp_api


#ifdef __cplusplus
extern "C" {
#endif

int32_t  btl_core_cancelSensor(int8_t);
int32_t  btl_core_registeFp();
int32_t  btl_core_matchFp(const char*, int*, int, int32_t*);
int32_t  btl_core_matchFpB(
    const char*,
    int32_t*,
    int32_t,
    int32_t,
    int32_t*);

int32_t  btl_core_matchFpC(
    const char*,
    int32_t*,
    int32_t,
    int32_t,
    int32_t*);


int32_t  btl_core_getTemplate(uint8_t*, uint32_t*);
int32_t  btl_core_enrollStart();
int32_t  btl_core_setCancel(int8_t);
int32_t  btl_core_isFingerUp();
int32_t  btl_core_setInterruptWorkMode();
int32_t  btl_core_unInitAlgo();
int32_t  btl_core_getMteStatus();
int32_t  btl_core_getDefaultParams();


int32_t  btl_preprocess_imageCheckEmpty(
    uint8_t* pImage,
    uint8_t value_threshold,
    int32_t counts_threshold,
    int32_t size);
int32_t  btl_preprocess_computeImgMeanVar(
    unsigned char*,
    int32_t,
    int32_t,
    int32_t* ,
    int32_t* ,
    int32_t*);    
int32_t btl_cal_finerMean(unsigned char* gray,
        int32_t nImgHeight,
        int32_t nImgWidth);
        
int  btl_remove_imageBase(unsigned char *p_input_image,
                      unsigned char* p_output_image, 
                      int image_width, 
                      int image_height,
                      int meanValue);           
int32_t btl_cal_baseMean(unsigned char* gray,
        int32_t nImgHeight,
        int32_t nImgWidth,
        int32_t startX,
        int32_t startY);
        
int btl_preprocess_middleFilter(unsigned char* inData,int width,int height,unsigned char* outData);

void  btl_preprocess_histNormolize(unsigned char *inData, unsigned char *outData,int width,int height);



int32_t btl_hw_getParams(
    int32_t device_fd,
    struct fingerprintd_params_t*);

int32_t btl_hw_get_chip_info(int32_t device_fd, struct bl229x_chip_info *chip_info);

char  str_contact(const char *str1, const char *str2,char *reault);
char* myitoa(int file_index, char* num_char, int radix);
int btl_file_fingerid();
int btl_file_fingerNum();
int file_delete_fingerprint(int fid);
int btl_api_createFile();
int btl_file_saveTemplate(unsigned char *pTemplateBuf, int template_size, int fid);
int btl_get_fid_size(int32_t *fd_index);
int btl_file_getTemplate( uint8_t* pTemplateBuffer , int size_read ,int fid);
int btl_file_getTemplateSize(int fid);



int      sql_inquire_all_fingerprint(const char*);
int      sql_inquire_database_by_fid(const char*, int);
int      sql_update_blob(const char*,char* , int);
int      sql_delete_fingerprint(const char*, int, int);
int      sql_load_fingerprint(const char*, int, int, unsigned char*, int*);
int      sql_insert_fingerprint(const char* , int, int, unsigned char*, int);
int      sql_create_database(const char*);
int      sql_inquire_fingerpint_fid(const char*, int32_t*);
int 	 sql_backup_database(const char *dbNameFrom, const char *dbNameTo);

int      acquire_wake_lock(int, const char* );
int      reslease_wake_lock(const char*);

void btl_util_save_image(uint8_t*  image, int type , int fid ,int times ,int dacp , int ispass ,int ispre);

void     btl_util_time_update(uint8_t);
void     btl_util_time_diffnow(char *, uint8_t);
int      btl_util_sendevent(const char* arg1,char* arg2, char* arg3,char*arg4);

uint32_t btl_util_my_hton(uint32_t ip);
uint64_t btl_util_my_htonl(uint64_t ip);
int32_t btl_core_enhance_image(uint8_t *pOrgImg, uint8_t *pEnhanceImg,int enhance_type);
int32_t btl_core_setAgc (uint32_t contrast);
int32_t  btl_core_initIC(void);
int32_t btl_core_reportkey_ctl(int32_t enable);





extern uint8_t g_debug_enable;

#define LOGV(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_VERBOSE, TAG,__VA_ARGS__);}
#define LOGD(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_DEBUG  , TAG,__VA_ARGS__);}
#define LOGI(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_INFO   , TAG,__VA_ARGS__);}
#define LOGW(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_WARN   , TAG,__VA_ARGS__);}
#define LOGE(...) {if (g_debug_enable)__android_log_print(ANDROID_LOG_ERROR  , TAG,__VA_ARGS__);}


#ifdef __cplusplus
}
#endif
#endif

#define DBUG_V

#define TAG       "btl_algo"







