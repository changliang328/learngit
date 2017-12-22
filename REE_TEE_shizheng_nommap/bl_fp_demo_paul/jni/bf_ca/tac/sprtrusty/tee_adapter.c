#include <tee_client_api.h>

#include "bf_log.h"
#include <pthread.h>  
#include <tipc.h>
#include <cademo_ipc.h>
#include <btl_fpdata.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define TRUSTY_DEVICE_NAME "/dev/trusty-ipc-dev0"
static struct tademo_message *msg=NULL;
static uint32_t SEND_BUF_SIZE = 512;
static uint32_t RECV_BUF_SIZE = 512;

static int handle_ = 0;
static pthread_mutex_t tee_mtx;

int tee_adapter_init_tzapp(void)
{ 

    CF_ENTRY();
    int rc = tipc_connect(TRUSTY_DEVICE_NAME, TADEMO_PORT);
    if (rc < 0) {
        return rc;
    }

    handle_ = rc;
    pthread_mutex_init(&tee_mtx, NULL);
    CF_EXIT();
    return 0;
}


void tee_adapter_destroy_tzapp(void)
{
     if (handle_ >= 0) {
        tipc_close(handle_);
	handle_ = -1;
    }
	pthread_mutex_destroy(&tee_mtx);
}


int trusty_cademo_call(uint32_t cmd, void *in, uint32_t in_size, uint8_t *out,
                           uint32_t *out_size) {
    struct tademo_message *msg=NULL;
    if (handle_ == 0) {
        LOGE("not connected\n");
        return -EINVAL;
    }

    size_t msg_size = in_size + sizeof(struct tademo_message);
    msg= malloc(msg_size);
    msg->cmd = cmd;
    memcpy(msg->payload, in, in_size);

   // LOGE("msg_size:%d\n",msg_size);

    ssize_t rc = write(handle_, msg, msg_size);
    free(msg);

    if (rc < 0) {
        LOGE("failed to send cmd (%d) to %s: %s\n", cmd,
                TADEMO_PORT, strerror(errno));
        return -errno;
    }

    rc = read(handle_, out, *out_size);
    if (rc < 0) {
        LOGE("failed to retrieve response for cmd (%d) to %s: %s\n",
                cmd, TADEMO_PORT, strerror(errno));
        return -errno;
    }

    if ((size_t) rc < sizeof(struct tademo_message)) {
        LOGE("invalid response size (%d)\n", (int) rc);
        return -EINVAL;
    }

    msg = (struct tademo_message *) out;

    if ((cmd | TA_RESP_BIT) != msg->cmd) {
        LOGE("invalid command (%d)\n", msg->cmd);
        return -EINVAL;
    }

    *out_size = ((size_t) rc) - sizeof(struct tademo_message);
    return rc;
}


int tee_adapter_invoke_command(char* buf, unsigned int len)
{
	CF_ENTRY();
	
	if (buf == NULL || len == 0){
	   LOGE("bad parameter");
	   return -2;
	}

	fp_data_t fp_data;
	memset(&fp_data,0,sizeof(fp_data_t));

	pthread_mutex_lock(&tee_mtx);
	
	int rc =tee_adapter_init_tzapp();
	if (rc < 0) {
	   LOGE("Error initializing trusty session: %d\n", rc);
	   return -1;
	    }
	rc = trusty_cademo_call((int)*buf, &fp_data,RECV_BUF_SIZE, (uint8_t *)&fp_data, &SEND_BUF_SIZE);
	if (rc < 0) {
		 LOGE("error (%d) calling  TA\n", rc);
	  }
	 tee_adapter_destroy_tzapp();

	pthread_mutex_unlock(&tee_mtx);

	CF_EXIT();

	return rc;
}


int ts_ca_get_key(void *key_data, uint32_t *key_len)
{
	LOGD("ts_ca_get_key no complete!");
	(void)key_data;
	(void)key_len;
	
	return 0;
}

