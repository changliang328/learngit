#ifndef __BL_FP_CORE__
#define __BL_FP_CORE__

#include <pthread.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include "bf_device.h"
#include "bf_mmi.h"
typedef void* BF_TAC_Handle_p;

#define DEBUG_IMGBUF_SIZE	(40*1024)

typedef enum worker_state_t
{
    STATE_NONE = -1,
    STATE_IDLE = 0,
    STATE_ENROLL = 1,
    STATE_AUTHENTICATE = 2,
    STATE_EXIT = 3,
    STATE_PM_SLEEP = 4,
    STATE_NAVIGATION = 5,
    STATE_MMITEST = 6,
    STATE_WAIT_FINGER_UP = 7,
    STATE_CANCEL = 8,
} worker_state_t;

typedef enum huawei_help_msg_t
{
    HUAWEI_FINGERPRINT_ACQUIRED_VENDOR_BASE = 2000,
    HUAWEI_FINGERPRINT_WAITTING_FINGER,
    HUAWEI_FINGERPRINT_DOWN ,
    HUAWEI_FINGERPRINT_UP,
    HUAWEI_ENROLL_DIRECTION_NA ,
    HUAWEI_ENROLL_DIRECTION_SW,
    HUAWEI_ENROLL_DIRECTION_S,
    HUAWEI_ENROLL_DIRECTION_SE,
    HUAWEI_ENROLL_DIRECTION_NW,
    HUAWEI_ENROLL_DIRECTION_N,
    HUAWEI_ENROLL_DIRECTION_NE,
    HUAWEI_ENROLL_DIRECTION_E,
    HUAWEI_ENROLL_DIRECTION_W,
    HUAWEI_ENROLL_HELP_NA ,
    HUAWEI_ENROLL_HELP_SAME_AREA ,
    HUAWEI_ENROLL_HELP_TOO_WET,
    HUAWEI_ENROLL_HELP_ALREADY_EXIST,
} huawei_help_msg_t;

typedef struct
{
	mmi_test_params_t *mmi_data;
    mmi_test_type  testType;
    unsigned int testResult;
    int isTesting;
    pthread_mutex_t mmi_mutex;
    pthread_cond_t mmi_idlecond;
    pthread_cond_t mmi_taskcond;
} mmi_info;

typedef enum
{
    CANCEL_NONE = 0,
    CANCEL_ENROLL = 1,
    CANCEL_AUTHENTICATE = 2,
} request_cancel_type;

typedef struct worker_thread_t
{
    pthread_t thread;
    pthread_t getScreenStateThread;
    pthread_mutex_t mutex;
    pthread_cond_t task_cond;
    pthread_cond_t idle_cond;
    int request;
    worker_state_t state;
    request_cancel_type c_type;
} worker_thread_t;

typedef struct bf_fingerprint_hal_device_t
{
    fingerprint_device_t device; //inheritance,240bytes
    worker_thread_t worker;//168 bytes
    pthread_mutex_t lock;	//40 bytes
    BF_TAC_Handle_p tac_handle;//8
    int32_t wait_finger_up;//4
    uint32_t current_gid;//4
    uint64_t challenge;//8
    uint64_t operation_id;
    uint64_t authenticator_id;
    bf_ree_device_t* ree_device;//8
    uint8_t *imagebuf;//8
    uint32_t height;
    uint32_t width;
	uint32_t screenstate;//4
	int32_t required_samples;//4
    int32_t navState;//4
    int32_t check_need_reenroll_finger;//4
 	mmi_info tMmiInfo;//160 bytes
 	uint32_t workstate;
    hw_auth_token_t hat;//69 bytes
    int debug;
} bf_fingerprint_hal_device_t;

int bf_start_mmi_test(bf_fingerprint_hal_device_t *dev);
int workerSetState(worker_thread_t* worker, worker_state_t state);
int bf_get_finger_data(bf_ca_app_data_t * fpdata);
#endif
