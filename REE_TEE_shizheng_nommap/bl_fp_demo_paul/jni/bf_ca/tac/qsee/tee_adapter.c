#include <string.h>
#include <stdint.h>

#include "bf_log.h"
#include <pthread.h>  
#include "QSEEComAPI.h"
#include "hardware/hw_auth_token.h"

#define FPAPPNAME "fpchips"
#define APP_PATH1 "/system/etc/firmware"
#define APP_PATH2 "/firmware/image"
#define APP_BUF_SIZE (251*1024)
#define CLIENT_MSG		1

static struct QSEECom_handle* l_QSEEComHandle = NULL;

static pthread_mutex_t tee_mtx;

typedef struct ts_send_req{
    uint32_t cmd_id;
    uint32_t data;
    uint32_t data2;
}ts_send_req_t;

//for keymaster
#define KEYMASTERNAME "keymaster"
#define KM_BUF_SIZE (3*1024)
#define KEYMASTER_UTILS_CMD_ID  0x200UL

static struct QSEECom_handle* QSEEComHandle_km = NULL;

typedef struct  _km_get_auth_token_rsp_t{
    int status;
    uint32_t auth_token_key_offset;
    uint32_t auth_token_key_len;
}__attribute__ ((packed)) km_get_auth_token_rsp_t;

typedef enum {
    KEYMASTER_GET_AUTH_TOKEN_KEY            = (KEYMASTER_UTILS_CMD_ID + 5UL),
    KEYMASTER_LAST_CMD_ENTRY                = (int)0xFFFFFFFFULL
} keymaster_cmd_t;

typedef struct _km_get_auth_token_req_t {
    keymaster_cmd_t cmd_id;
    hw_authenticator_type_t auth_type;
}__attribute__ ((packed)) km_get_auth_token_req_t;


int tee_adapter_init_tzapp(void)
{
    int ret = -1;

    CF_ENTRY();

    if (l_QSEEComHandle != NULL)
    {
        LOGD("app-%s already loaded", FPAPPNAME);
        return 0;
    }

    ret = QSEECom_start_app(&l_QSEEComHandle, APP_PATH1 , FPAPPNAME, APP_BUF_SIZE);
    if (ret != 0)
    {
        LOGE("loading app-%s in %s failed! ret = %d", FPAPPNAME, APP_PATH1, ret); 
        ret = QSEECom_start_app(&l_QSEEComHandle, APP_PATH2 , FPAPPNAME, APP_BUF_SIZE); 
        if (ret != 0)
        {
	    LOGE("loading app-%s in %s failed! ret = %d", FPAPPNAME, APP_PATH2, ret); 
            return ret;
        }
    }
	
    LOGD("load %s success!", FPAPPNAME);
 
    pthread_mutex_init(&tee_mtx, NULL);

    CF_EXIT();
	
    return 0;
}


void tee_adapter_destroy_tzapp(void)
{
    int ret = -1;
	
    CF_ENTRY();
	
    if (l_QSEEComHandle != NULL)
    {
        ret = QSEECom_shutdown_app(&l_QSEEComHandle);
        if (ret != 0)
        {
	    LOGE("QSEECom_shutdown_app failed! ret = %d", ret); 
        }
        else 
        {
	    LOGD("QSEECom_shutdown_app success! ret = %d", ret); 
            l_QSEEComHandle = NULL;
        }  
    }
    else
    {
        LOGE("l_QSEEComHandle is NULL"); 
    }

    pthread_mutex_destroy(&tee_mtx);
	
    CF_EXIT();
}


/*
 *    |<-------------------------  APP_BUF_SIZE (251*1024) byte ------------------------->| 
 *    |___________________________________________________________________________________|
 *    |                  |                              | 
 *    |<--request_head-->|<-----------send_buf--------->|<----------receive_buf---------->|
 *    |      12byte      |         user_data_len        |           user_data_len         |    
 */
 static int ts_send_req(struct QSEECom_handle *l_QSEEComHandle, void *buf, unsigned int len)
{
    int32_t ret = -1;
    uint32_t send_len = 0;
    uint32_t recv_len = 0;
    ts_send_req_t* msgreq = NULL;
    uint8_t *msgrsp = NULL; 

    if (l_QSEEComHandle == NULL)
    {
        LOGE("bad parameter! l_QSEEComHandle is NULL");
    }
	
    if (buf == NULL && len != 0)
    {
        LOGE("bad parameter! buf is NULL && len(%u) != 0", len);
    }

    send_len = sizeof(ts_send_req_t) + len;
    recv_len = len;
	
    if (send_len & QSEECOM_ALIGN_MASK)
    {
        send_len = QSEECOM_ALIGN(send_len);
    }

    if (recv_len & QSEECOM_ALIGN_MASK)
    {
        recv_len = QSEECOM_ALIGN(recv_len);
    }

    if (send_len + recv_len > APP_BUF_SIZE)
    {
    	LOGE("data too large! (send_len + recv_len)(%u) > APP_BUF_SIZE(%d)", send_len + recv_len, APP_BUF_SIZE);
        return -1;
    }

    msgreq = (ts_send_req_t *)l_QSEEComHandle->ion_sbuffer;
    msgreq->cmd_id = CLIENT_MSG;
	msgreq->data = send_len - len;
	msgreq->data2 = 10;
    memcpy((uint8_t *)msgreq + sizeof(ts_send_req_t), buf, len);
    msgrsp = ((uint8_t *)l_QSEEComHandle->ion_sbuffer + send_len);

    QSEECom_set_bandwidth(l_QSEEComHandle, 1);
    ret = QSEECom_send_cmd(l_QSEEComHandle, msgreq, send_len, msgrsp, recv_len);
    if (ret != 0)
    {
    	LOGE("QSEECom_send_cmd failed!");
        QSEECom_set_bandwidth(l_QSEEComHandle, 0);
        return -1;
    }
	
    QSEECom_set_bandwidth(l_QSEEComHandle, 0);
    memcpy(buf, msgrsp, len);
	
    return ret;
}


int tee_adapter_invoke_command(char* buf, unsigned int len)
{
    int ret = -1;			

    if (buf == NULL || len == 0)
    {
        LOGE("bad parameter! buf is NULL or len(%u) is 0",len);
	return -2;
    }

    if (l_QSEEComHandle == NULL)
    {
        LOGE("l_QSEEComHandle is NULL!");
        return -1;
    }

    /* Send data using send command to QSEE application */
    pthread_mutex_lock(&tee_mtx);
    ret = ts_send_req(l_QSEEComHandle, buf, len);
    pthread_mutex_unlock(&tee_mtx);
    if (ret != 0)
    {
        LOGE("ts_send_req failed! ret = %d",ret);
    }

    return ret;
}



//for keymaster
static int ts_start_km(void)
{
    int ret = -1;
	
    CF_ENTRY();
	
    if (QSEEComHandle_km != NULL)
    {
        LOGD("app-%s already loaded", KEYMASTERNAME);
        return 0;
    }

    ret = QSEECom_start_app(&QSEEComHandle_km, APP_PATH1 , KEYMASTERNAME, KM_BUF_SIZE);
    if (ret != 0)
    {
        LOGE("loading app-%s in %s failed! ret = %d", KEYMASTERNAME, APP_PATH1, ret); 
        ret = QSEECom_start_app(&QSEEComHandle_km, APP_PATH2 , KEYMASTERNAME, KM_BUF_SIZE); 
        if (ret != 0)
        {
            LOGE("loading app-%s in %s failed! ret = %d", KEYMASTERNAME, APP_PATH2, ret); 
            return ret;
        }
    }
	
    CF_EXIT();

    return 0;
}


static int ts_shutdown_km(void)
{
    int ret = -1;
	
    CF_ENTRY();
	
    if (QSEEComHandle_km != NULL)
    {
        ret = QSEECom_shutdown_app(&QSEEComHandle_km);
        if (ret != 0)
        {
            LOGE("QSEECom_shutdown_app failed! ret = %d", ret); 
        }
        else 
        {
	    LOGD("QSEECom_shutdown_app success! ret = %d", ret); 
            QSEEComHandle_km = NULL;
        }  
    }
    else
    {
        LOGE("QSEEComHandle_km is NULL!"); 
    }
	
    CF_EXIT();

    return 0;
}


int ts_ca_get_key(void *key_data, uint32_t *key_len)
{
    int ret = -1;
    km_get_auth_token_req_t *msgreq = NULL;
    km_get_auth_token_rsp_t *msgrsp = NULL;
    uint32_t send_len = 0;
    uint32_t recv_len = 0;
	
    if (key_data == NULL || key_len == 0)
    {
        LOGE("bad parameter! key_data is NULL or key_len is NULL");
    }

	
    ret = ts_start_km();
    if (ret != 0)
    {
        LOGE("ts_start_km failed!");
        return -1;
    }
	
    msgreq = (km_get_auth_token_req_t*)QSEEComHandle_km->ion_sbuffer;
    send_len = QSEECOM_ALIGN(sizeof(km_get_auth_token_req_t));
    msgrsp = (km_get_auth_token_rsp_t*)(QSEEComHandle_km->ion_sbuffer + send_len);
    msgreq->cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
    msgreq->auth_type = HW_AUTH_FINGERPRINT;
    recv_len = KM_BUF_SIZE - send_len;
	
    if (send_len > (uint32_t)KM_BUF_SIZE || recv_len > (uint32_t)KM_BUF_SIZE)
    {
        LOGE("send_len(%u) > KM_BUF_SIZE(%d) or recv_len(%u) > %d", send_len, KM_BUF_SIZE, recv_len, KM_BUF_SIZE);
        ts_shutdown_km();
        return -1;
    }

    ret = QSEECom_send_cmd(QSEEComHandle_km, msgreq, send_len, msgrsp, recv_len);
    if (ret != 0) 
    {
        LOGE("QSEECom_send_cmd failed!");
        ts_shutdown_km();
        return ret;
    }

    if (msgrsp->status != 0 || msgrsp->auth_token_key_len <= 0)
    {
        LOGE("get key error, ret = %d, key_len = %d\n", msgrsp->status, msgrsp->auth_token_key_len);
        ts_shutdown_km();
        return -1;
    }

    memcpy(key_data, ((uint8_t *)msgrsp) + msgrsp->auth_token_key_offset, msgrsp->auth_token_key_len);
	
    *key_len = msgrsp->auth_token_key_len;

    LOGD("get key success, key_len= %d", msgrsp->auth_token_key_len);

    ts_shutdown_km();
	
    return ret;
}


