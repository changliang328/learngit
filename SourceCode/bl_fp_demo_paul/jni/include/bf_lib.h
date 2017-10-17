#ifndef _BF_LIB_H_
#define _BF_LIB_H_
#include <linux/types.h>
#include <unistd.h>
/* Version 0.5 */
/* Used by all bf_lib functions*/
typedef struct {
        uint32_t count;
        int32_t dx;
        int32_t dy;
} nav_res_t;
typedef enum {
    BF_LIB_OK,                            // 0
    BF_LIB_WAIT_EVENT_FINGER_PRESENT,     // 1
    BF_LIB_CAPTURE_DONE,                  // 2
    BF_LIB_ENABLE_EVENT_FINGER_PRESENT,   // 3
    BF_LIB_WAIT_TIME,                     // 4
    BF_LIB_FINGER_PRESENT,                // 5
    BF_LIB_FINGER_LOST,                   // 6
    BF_LIB_ERROR_TOO_FAST,                // 7
    BF_LIB_ERROR_TOO_SLOW,                // 8
    BF_LIB_ERROR_GENERAL,                 // 9
    BF_LIB_ERROR_SENSOR,                  // 10
    BF_LIB_ERROR_MEMORY,                  // 11
    BF_LIB_ERROR_PARAMETER,               // 12
    BF_LIB_FAIL_LOW_QUALITY,
    BF_LIB_FAIL_INVAILD_TOUCH,

    BF_TA_ERROR_CODE_BASE = 0xFFFF6000,
    BF_TA_ERROR_TA_NOT_INIT,
    BF_TA_ERROR_STATE,
    BF_TA_ERROR_LIB_INIT_FAIL,
    BF_TA_ERROR_INIT_ALG_PPLIB_FAIL,
    BF_TA_ERROR_EROLL_EXCEED_MAX_FINGERPIRNTS,
    BF_TA_ERROR_EROLL_NOT_COMPLETED,
    BF_TA_ERROR_EROLL_GET_TEMPLATE_FAIL,
    BF_TA_ERROR_EROLL_PACK_TEMPLATE_FAIL,

    BF_TA_DB_CODE_BASE = BF_TA_ERROR_CODE_BASE | 0x1000,
    BF_TA_ERROR_DB_FS_INIT_FAIL ,
    BF_TA_ERROR_CREAT_GLOBAL_DB_FAIL,
    BF_TA_ERROR_CREAT_USER_DB_FAIL,
    BF_TA_ERROR_DB_SIZE_OVERFLOW,
    BF_TA_ERROR_DB_GET_FP_ID_ERROR,
    BF_TA_ERROR_DB_GET_FPSET_ID_ERROR,
    BF_TA_ERROR_DB_GET_SECURE_USER_ID_ERROR,
    BF_TA_ERROR_DB_GLOBAL_DB_FILE_LOST,
    BF_TA_ERROR_DB_USER_DB_FILE_LOST,
    BF_TA_ERROR_DB_SINGLE_TEMPLATE_FILE_LOST,
} bf_lib_return_t;

typedef enum {
    BF_LIB_IDENTIFY_NO_MATCH,
    BF_LIB_IDENTIFY_MATCH,
    BF_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE,
} bf_lib_identify_result_t;

typedef enum {
    BF_LIB_ENROLL_SUCCESS,
/////huawei define start///
    BF_LIB_ENROLL_HELP_SAME_AREA ,
    BF_LIB_ENROLL_HELP_TOO_WET,
    BF_LIB_ENROLL_HELP_ALREADY_EXIST,
/////huawei define end///
    BF_LIB_ENROLL_TOO_MANY_ATTEMPTS,
    BF_LIB_ENROLL_TOO_MANY_FAILED_ATTEMPTS,
    BF_LIB_ENROLL_FAIL_NONE,
    BF_LIB_ENROLL_FAIL_LOW_QUALITY,
    BF_LIB_ENROLL_FAIL_LOW_COVERAGE,
    BF_LIB_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE,
} bf_lib_enroll_result_t;

typedef enum {
    BF_LIB_SENSOR_OK,
    BF_LIB_SENSOR_WORKING,
    BF_LIB_SENSOR_INITIALISING,
    BF_LIB_SENSOR_OUT_OF_ORDER,
    BF_LIB_SENSOR_MALFUNCTIONED,
    BF_LIB_SENSOR_FAILURE,
} bf_lib_sensor_status_t;

/*
 * Structure for templates stored in the database
*/
typedef struct {
    /* Pointer to a buffer for template data */
    uint8_t* tpl;
    /* Size of the buffer for template data */
    uint32_t size;
} bf_lib_template_t;

/*
 * Struct with data from an enroll attempt
 */
typedef struct {
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
    /* Size of the data part in the structure used for extended enroll */
    uint32_t extended_enroll_size;
    /* Coverage of the image*/
    uint32_t coverage;
    /* Used to indicate that touches are too similar */
    int8_t user_touches_too_immobile;
    int8_t guide_direction;
} bf_lib_enroll_data_t;

/* Structure used for extended enroll */
typedef struct {
    /* Data buffer for extended enroll */
    uint8_t* data;
    /* Size of the extended enroll data buffer */
    uint32_t size;
} bf_lib_extended_enroll_t;

typedef enum {
    NORMAL_IDENTIFY_CAC_IMAGE,
    SENSORHUB_IDENTIFY_CAC_IMAGE,
    SENSORHUB_IDENTIFY_FALLBACK_IMAGE,
} sensor_hub_image_t;
/* Data from the identification attempt */
typedef struct {
    /* Result of the identification attempt */
    bf_lib_identify_result_t result;
    /* Matching score */
    uint32_t score;
    /* Index of the identification template */
    uint32_t index;
    /* Size of the update template if one exits */
    uint32_t updated_template_size;
    int coverage;
    int quality;
    sensor_hub_image_t image_flag;
    int sensorhub_fail_img_cnt;
    int sensorhub_anti_touch_cnt;
    int max_score;
} bf_lib_identify_data_t;

/* Security levels for the matching function */
typedef enum {
    /* FAR 1/1000 */
    BF_LIB_SECURITY_LOW,
    /* FAR 1/10,000 */
    BF_LIB_SECURITY_REGULAR,
    /* FAR 1/50,000 */
    BF_LIB_SECURITY_HIGH,
    /* FAR 1/100,000 */
    BF_LIB_SECURITY_VERY_HIGH,
} bf_lib_security_t;

typedef struct {
    uint8_t* raw;
    uint8_t* enhanced;
    uint32_t size;
} bf_lib_image_storage_t;

/*
 * Initialize the library, load the database and check the sensor.
 *
 * @return BF_OK
 *
 */
bf_lib_return_t bf_lib_init(uint8_t* data, uint32_t* size);

uint32_t bf_lib_init_data_size(void);
/**
 * De-initialize the library.
 *
 * @return BF_OK
 *
 */
bf_lib_return_t bf_lib_deinit(void);

/*
 * Captures an image from the sensor, and stores it for use with enroll and
 * identify.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_SENSOR - if the image capture failed because of the
 *         communication with the sensor
 *         BF_LIB_ERROR_MEMORY - if memory allocation failed during the capture
 *         BF_LIB_ERROR_GENERAL - if other error occurred
*/
bf_lib_return_t bf_lib_capture_image(void);

/*
 * Captures an image from the sensor but doesn't run it through any processing.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_SENSOR - if the image capture failed because of the
 *         communication with the sensor
 *         BF_LIB_ERROR_MEMORY - if memory allocation failed during the capture
 *         BF_LIB_ERROR_GENERAL - if other error occurred
*/
bf_lib_return_t bf_lib_capture_image_uncalibrated(void);

/*
 * Begin the enrollment of a new finger.
 *
 * @return BF_OK
 *
 */
bf_lib_return_t bf_lib_begin_enroll(void);

/*
 * Use the currently stored image for enrollment.
 *
 * @param[out] data, the data from the enrollment.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if data is NULL
 *         BF_LIB_ERROR_MEMORY - if memory allocation failed during the enrollment
 */
bf_lib_return_t bf_lib_enroll(bf_lib_enroll_data_t* data);

/*
 * Get information for the extended enroll processes. The size of the data
 * structure were given in the previous bf_lib_enroll(void) call.
 *
 * @param[out] data, the data used for extend enroll.
 * @param[in/out] size, in:size is the size of the allocated data structure,
 *  out:size is the size of the used data structure. If out:size is larger than
 *  in:size call has to be redone with the larger value.
 *
 *  @return BF_OK
 *          BF_LIB_ERROR_PARAMETER - if the struct pointer is null
 *          BF_LIB_ERROR_MEM - if the size is too small
*/
bf_lib_return_t bf_lib_get_extended_enroll_data(bf_lib_extended_enroll_t* data,
        uint32_t* size);

/*
 * Ends the enrollment and returns the enrolled finger template.
 *
 * @param[out] tpl, a buffer with the template data for the enrolled finger.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if template is NULL or the size of the
 *         buffer is too small
 *         BF_LIB_ERROR_MEMORY - if the template extraction failed
 */
bf_lib_return_t bf_lib_end_enroll(bf_lib_template_t* tpl);

/*
 * Begin identify for the given template candidates.
 *
 * @param[in] candidates, the templates of the candidates to verify against
 * @param[in] size, size of the candidate array.
 * @param[in] security, the security level from bf_lib_security_t
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if candidates is a NULL pointer, size is
 *         zero or security is an invalid number
 *         BF_LIB_ERROR_MEMORY - if memory allocation fails during
 *         the identification process
*/
bf_lib_return_t bf_lib_begin_identify(bf_lib_template_t* candidates,
        uint32_t size, bf_lib_security_t security);

/*
 * Use the currently stored image for identification.
 *
 * @param data data from the identification process.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if data is a null pointer
 *         BF_LIB_ERROR_MEMORY - if the memory allocation failed during the
 *         identification
*/
bf_lib_return_t bf_lib_identify(bf_lib_identify_data_t* data);

/*
 * End the started identification. If there was a template update during the
 * identification process, the given template struct will be filled with
 * template data. If a null pointer is given the update will be discarded.
 *
 * @param[in/out] updated_template, a buffer for the updated template
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if template is NULL or the size of the buffer
 *         is too small, (the needed size is returned in the template data)
 */
bf_lib_return_t bf_lib_end_identify(bf_lib_template_t* updated_template);

/*
 * Check if a finger is present on the sensor
 *
 *    while (ret != BF_FINGER_PRESENT || abort != TRUE) {
 *       ret = bf_finger_present(void)
 *       switch (ret) {
 *           case BF_ENABLE_EVENT_FINGER_PRESENT:
 *               driver_init_finger_detect_event(void)
 *               break;
 *           case BF_WAIT_FOR_EVENT:
 *               driver_wait_for_event(void);
 *               break;
 *           case BF_ENABLE_EVENT_TIME:
 *               sleep(FINGER_PRESENT_WAIT_TIME);
 *               break;
 *           case BF_FINGER_LOST:
 *               break;
 *           default:
 *               ret = bf_end_identify(void);
 *               return -1;
 *       }
 * @param[out] wait_time, the time in ms to wait.
 *
 * @return BF_LIB_ENABLE_EVENT_FINGER_PRESENT - start the wait for finger
 *         present
 *         BF_LIB_WAITING_FOR_EVENT - if a finger present event is needed
 *         before checking
 *         BF_LIB_ENABLE_EVENT_TIME - all qualification for finger present
 *         not fullfilled, wait and try again,
 *         BF_LIB_FINGER_PRESENT - finger is present on the sensor
 *         BF_LIB_FINGER_LOST - finger is removed from the sensor
*/
int bf_lib_finger_present(uint32_t* wait_time);

/*
 * Check if the finger is removed from the sensor
 *
 * @return BF_LIB_FINGER_LOST - if finger is not on the sensor
 *         BF_LIB_ENABLE_EVENT_TIME - finger is on sensor, sleep and try later
 */
int bf_lib_check_finger_lost(uint32_t* wait_time);

/*
 * Injects an image to use for enroll or identify.
 * ONLY for debugging purpose
 *
 * @param img, a pointer to the image data.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if the image has the wrong format or other error
*/
bf_lib_return_t bf_lib_debug_inject_image(uint8_t* img, uint32_t size);

/*
 * Retrieves the image to use for enroll or identify.
 * ONLY for debugging purpose.
 *
 * @param[out] img an pointer to a mem area with the size of an image.
 *
 * @return BF_OK
 *         BF_LIB_ERROR_PARAMETER - if image is a NULL pointer or size is 0
*/
bf_lib_return_t bf_lib_debug_retrieve_image(uint8_t* img, uint32_t size);

/*
 * Puts the sensor in sleep mode with finger detect. Used to prepare the
 * sensor to wake the phone from sleep.
 */
bf_lib_return_t bf_lib_wakeup_setup(void);

/*
 * Used when phone "wake up" is triggered by the sensor. If qualification
 * passes the phone should be woken up.
 *
 * @return BF_FINGER_PRESENT - if the wake qualification passed
 *         BF_FINGER_LOST - if the qualification fails
*/
bf_lib_return_t bf_lib_wakeup_qualification(void);

#ifdef _BF_IMAGE_STORING_
/*
 * Provide buffers for storing the raw imaged and the image that is actually
 * used for enrollment or identification.
 *
 * @return BF_LIB_OK - if buffers are registered
 *         BF_LIB_PARAMETER_ERROR- if the size is wrong
 *
*/
bf_lib_return_t bf_lib_register_image_storage(bf_lib_image_storage_t* store);

#endif

/*
 * Returns status from the sensor module.
 *
 * @param[out] status, status from the sensor module
 *
 * @return BF_LIB_OK
 *         BF_LIB_ERROR_PARAMETER - if status is a NULL pointer.
*/
bf_lib_return_t bf_lib_get_sensor_status(bf_lib_sensor_status_t* status);
#endif
