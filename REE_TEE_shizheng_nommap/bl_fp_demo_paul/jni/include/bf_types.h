#ifndef __BF_TYPES_H__
#define __BF_TYPES_H__
#include "bf_custom.h"
#include "./hardware/hw_auth_token.h"
#include "bf_lib.h"
#include "bf_config.h"
#define BF_MAX_FINGER	(5)
#define MAX_PATH_LENGTH (256)

#define QUALIFY_MODE_AUTHENTICATE	(0)
#define QUALIFY_MODE_ENROLL			(1)

#define NAVI_FINGER_DOWN_AREA	(10)
//"/dev/blestech_fp"; "/dev/bl229x"
#define BF_FP_DEV_NAME "/dev/blestech_fp"

enum mmidev_workstate_t
{
	MMI_DEV_STATE_OK = 1,
	MMI_DEV_STATE_FAILED = 2,
	MMI_DEV_STATE_NONE = 4000,
	MMI_DEV_STATE_CAPTURETEST,
	MMI_DEV_STATE_AUTOTEST,
};

typedef enum {
    BF_MMI_ACT_SET_PARAMS = 8000,
    BF_MMI_ACT_CAPTURE_WITH_DACP,
    BF_MMI_ACT_SET_IDLE_MODE,
    BF_MMI_ACT_CAPTURE_INIT,
    BF_MMI_ACT_GET_RAW_IMAGE,
    BF_MMI_ACT_MAX,
} BF_MMI_ACTION_t;

typedef struct mmi_test_params{
	uint32_t cmd;
	int action;
	int length;
	int result;
	uint8_t params[0];
}mmi_test_params_t;

typedef enum {
    BL_FP_CHIP_2390E = 0,
    BL_FP_CHIP_3290 = 1,
    BL_FP_CHIP_3182 = 2,
    BL_FP_CHIP_3390 = 3,
    BL_FP_CHIP_3590 = 4,
    BL_FP_CHIP_81192 = 5,
    BL_FP_CHIP_KILBY = 6,
    BL_FP_CHIP_MAX,
} chip_type_t;

typedef enum {
    BL_CHIPID_2390E = 0x5183,
    BL_CHIPID_3290 = 0x5183,
    BL_CHIPID_3182 = 0x5283,
    BL_CHIPID_3390 = 0x5383,
    BL_CHIPID_3590 = 0x5483,
    BL_CHIPID_81192 = 0x5683,
    BL_CHIPID_KILBY = 0xD0F0,
    BL_CHIPID_MAX,
} chip_id_t;

typedef  enum
{
//100
	BF_CMD_CORE_INIT = 100,
	BF_CMD_CORE_UNINIT,
	BF_CMD_CHIP_REINIT,
	BF_CMD_FD_MODE,
	BF_CMD_GET_INT_STATUS,
//105
	BF_CMD_CAPTURE_IMAGE,
	BF_CMD_QUALITY_IMAGE,
	BF_CMD_READ_FRAME,
	BF_CMD_GET_NAVIGATION_EVENT,
	BF_CMD_ENROLL,
//110
	BF_CMD_END_ENROLL,
	BF_CMD_IDENTIFY,
	BF_CMD_NEW_FINGERID,
	BF_CMD_DELETE_FINGERID,
	BF_CMD_UPDATE_TEMPLATE_BY_INDIC,
//115
	BF_CMD_CHECK_FINGER_LOST,
	BF_CMD_GET_HW_AUTH_CHALLENGE,
	BF_CMD_GET_TEMPLATE_DB_ID,
	BF_CMD_GET_TEMPLATE_COUNT,
	BF_CMD_GET_TEMPLATE_INDICS,
//120
	BF_CMD_GET_TEMPLATE_ID_FROM_INDEX,
	BF_CMD_DELETE_TEMPLATE_BY_INDIC,
	BF_CMD_STORE_TEMPLATE_DB,
	BF_CMD_LOAD_TEMPLATE_DB,
	BF_CMD_SET_FINGERPRINT_SET,
//125
	BF_CMD_CHECK_TEMPLATE_VERSION,
	BF_CMD_IDENTIFY_ALL,
	BF_CMD_CAPTURE_IMAGE_ALL,
	BF_CMD_GET_NAVIGATION_EVENT_ALL,
	BF_CMD_DO_MMI_TEST,
//130
	BF_CMD_GET_TIME,
	BF_CMD_GET_AUTH_TOKEN,
	BF_CMD_SEND_ENROLL_TOKEN,
	BF_CMD_GET_FINGER_DATA,
} bf_ca_cmd_type;

typedef  enum
{
	BF_INTSTATE_FINGER_DOWN = 1,
	BF_INTSTATE_FRAME_DONE = 2,
} bf_int_status_type;

typedef  enum
{
	STATE_DO_CAPTURE_START = 1,
	STATE_DO_CAPTURE_GOT_FIRST_FRAME,
	STATE_DO_CAPTURE_WAIT_FRAMEDONE,
	STATE_DO_CAPTURE_GOT_BEST_IMAGE,
	STATE_NORMAL_READ_FRAME_AND_DO_CAPTURE,
	STATE_DO_CAPTURE_ONLY,
	STATE_DO_CAPTURE_AND_WAIT_FRAMEDONE,
} bf_cap_state_type;

typedef  enum
{
    EVENT_HOLD	= 502,
    EVENT_CLICK	= 601,
    EVENT_DCLICK = 501,
    EVENT_UP	= 511,
    EVENT_DOWN	= 512,
    EVENT_LEFT	= 513,
    EVENT_RIGHT	= 514,
    EVENT_LOST	= -1,
} navievent_type;
typedef  enum
{
	NAVI_STATE_NONE = 0,
	NAVI_STATE_START = 1,
	NAVI_STATE_MEASURING,
	NAVI_STATE_END,
} navistate_type;

typedef struct
{
	int32_t result;
	int32_t downValue;
	int32_t eventValue;
	uint32_t uPosX;
	uint32_t uPosY;
	int32_t deltaX;
	int32_t deltaY;
	uint32_t uMeanValue;
	uint32_t uAreaCount;
	uint32_t width;
	uint32_t height;
    int32_t state;
    int32_t count;
    int32_t  lastevent;
} navigation_info;

struct bf_key_event {
	int code;
	int value;  
};

typedef enum finger_detect_type_t
{
	FINGER_DETECT_DOWN,
	FINGER_DETECT_DOWN_PART,
	FINGER_DETECT_UP,
} finger_detect_type_t;

typedef struct bf_capture_data{
	int32_t result;
	uint32_t mode;
	uint32_t width;
	uint32_t height;
	uint32_t uPosX;
	uint32_t uPosY;
	uint32_t uMeanValue;
	uint32_t uAreaCount;
	uint32_t uXuArea;
	uint32_t uPBscore;
	uint32_t uPBarea;
	uint32_t uPBcondition;
    int32_t count;
    int32_t state;
}bf_capture_data_t;

typedef struct bf_enroll_data{
    int32_t result;
   /* Progress of the current enroll process in percent */
    uint32_t progress;
    /* Quality for the image*/
    uint32_t quality;
    /* MaxNum samples of enroll*/
	uint32_t required_samples;
    /* Number of successful enroll attempts so far */
    uint32_t nr_successful;
    /* Number of failed enroll attempts so far */
    uint32_t nr_failed;
    /* Size of the enrolled template */
    uint32_t enrolled_template_size;
    /* Coverage of the image*/
    uint8_t coverage;
    /* number of Islands of the registed images*/
    uint8_t nNbrOfIslands;
}bf_enroll_data_t;

typedef struct {
    /* Result of the identification attempt */
   	int32_t result;
    /* Matching score */
    uint32_t score;
    /* Index of the identification template */
    int32_t index;
    uint32_t matchID;
    int32_t coverage;
    int32_t quality;

    int32_t indic;

} bf_identify_data_t;

typedef struct {
	uint32_t cmd;
	uint32_t width;
	uint32_t height;
	uint8_t *rawImage;
}bf_ca_image_t;

typedef  struct {
	uint32_t cmd;
	int32_t param;
} bf_ca_param1_t;

typedef  struct {
	uint32_t cmd;
	uint32_t indics[BF_MAX_FINGER];
} bf_ca_indics_t;

typedef  struct {
	uint32_t cmd;
	uint32_t path_len;
	uint32_t gid;
	uint8_t path[MAX_PATH_LENGTH];
} bf_ca_path_t;

typedef  struct {
	uint32_t cmd;
	uint64_t kernelsystime;
} bf_ca_systime_t;


typedef struct{
	uint32_t cmd;
	hw_auth_token_t hat;
}bt_ca_auth_token_t;


typedef struct {
	uint32_t cmd;
	uint32_t width;
	uint32_t height;
	uint32_t capdacp;
}bf_ca_app_data_t;

typedef  union{
	uint32_t cmd;
	bf_ca_param1_t cmdparam;
	navigation_info navidata;
	bf_enroll_data_t enrolldata;
	bf_capture_data_t capturedata;
	bf_identify_data_t identifydata;
	bf_ca_image_t caImage;
	bf_ca_indics_t caIndics;
	bf_ca_path_t caPath;
	load_config_t ca_config;
	mmi_test_params_t ca_mmi_params;
	bt_ca_auth_token_t  ca_authtoken_data;
	bf_ca_systime_t systime;
	bf_ca_app_data_t app_data;
} bf_ca_cmd_data_t;

#endif
