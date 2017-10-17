#ifndef __BL_FP_CORE__
#define __BL_FP_CORE__

#include <pthread.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>
#include "bf_device.h"
#include "bl_fingerprint.h"
typedef void* BF_TAC_Handle_p;
int32_t bl_fp_Init(void);
int32_t bl_fp_UnInit(void);
int32_t bl_fp__GetRawImage(uint8_t * pBmp, int32_t *params, int32_t *result);

#define DEBUG_IMGBUF_SIZE	(20*1024)

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

typedef enum socket_msg_t
{
    MSG_MMI_AUTO_TEST    = 11,
    MSG_MMI_INTERRUPT_TEST = 12,
    MSG_MMI_FAKE_FINGER = 13,
    MSG_SNR_SINGAL_IMAGE_TEST = 14,
    MSG_SNR_WHITE_IMAGE_TEST = 15,
    MSG_MMI_GET_RESULT   = 31,
    MSG_NAV_ENABLE           = 41,
    MSG_NAV_DISABLE         = 42,
    MSG_EXIT                        = 91,
} socket_msg_t;

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
typedef  enum
{
    MMI_TEST_NONE = 0,
    MMI_AUTO_TEST       = 1,  //for MMI1 and MMI2
    MMI_TYPE_INTERRUPT_TEST = 2,
    MMI_FAKE_FINGER_TEST = 3, //juse for MMI1
    MMI_SNR_SINGAL_IMAGE_TEST = 4,
    MMI_SNR_WHITE_IMAGE_TEST = 5,
} mmi_test_type;

typedef struct
{
    mmi_test_type  testType;
    unsigned int testResult;
} mmi_info;
typedef enum    //for struct mmi_info. testResult
{
    FP_MMI_TEST_PASS = 1,
    FP_MMI_TESTING = 2,
} mmi_test_result;

typedef enum
{
    CANCEL_NONE = 0,
    CANCEL_ENROLL = 1,
    CANCEL_AUTHENTICATE = 2,
} request_cancel_type;

typedef struct worker_thread_t
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t task_cond;
    pthread_cond_t idle_cond;
    int request;
    worker_state_t state;
    request_cancel_type c_type;
} worker_thread_t;

typedef struct bf_fingerprint_hal_device_t
{
    fingerprint_device_t device; //inheritance

    worker_thread_t worker;
    pthread_mutex_t lock;

    BF_TAC_Handle_p tac_handle;
    int wait_finger_up;
    uint32_t current_gid;
    uint64_t challenge;
    uint64_t user_id;
    hw_auth_token_t hat;
    uint8_t  *nonce;
    bool navState;
    mmi_info pMmiInfo;
    //hw_fp_hal_ex_t* hal_ex;
    bf_ree_device_t* ree_device;
    //fido_dev_t* fido;
    int8_t  check_need_reenroll_finger;//-1 uncheck  /0 check_match   /1 check_unmatch,template Auto_delete need reEnroll   /2 check error
    uint8_t need_liveness_authentication;//default 0 = without liveness   /  1 with liveness
    uint8_t required_samples;
    uint8_t *imagebuf;
} bf_fingerprint_hal_device_t;

#endif
