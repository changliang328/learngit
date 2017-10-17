#ifndef __BF_TYPES_H__
#define __BF_TYPES_H__
#include <linux/types.h>
#ifndef u8
#define u8	uint8_t
#define u16 uint16_t
#define u32	uint32_t
#endif

#include "bf_lib.h"
#define BF_MAX_FINGER	(5)
#define MAX_PATH_LENGTH (256)

typedef enum finger_detect_type_t
{
	FINGER_DETECT_DOWN,
	FINGER_DETECT_DOWN_PART,
	FINGER_DETECT_UP,
} finger_detect_type_t;

typedef struct bf_enroll_data{
   /* Progress of the current enroll process in percent */
    uint32_t progress;
    /* Quality for the image*/
    uint32_t quality;
    /* Status of current enroll attempt */
    bf_lib_enroll_result_t result;
    /* Number of successful enroll attempts so far */
    uint32_t nr_successful;
    /* Number of failed enroll attempts so far */
    uint32_t nr_failed;
    /* Size of the enrolled template */
    uint32_t enrolled_template_size;
    /* Coverage of the image*/
    uint32_t coverage;
    /* number of Islands of the registed images*/
    uint32_t nNbrOfIslands;
}bf_enroll_data_t;

typedef struct {
    /* Result of the identification attempt */
    bf_lib_identify_result_t result;
    /* Matching score */
    uint32_t score;
    /* Index of the identification template */
    int index;
    /* Size of the update template if one exits */
    uint32_t updated_template_size;
    int coverage;
    int quality;

    int sensorhub_fail_img_cnt;
    int sensorhub_anti_touch_cnt;
    int max_score;
} bf_identify_data_t;

#endif
