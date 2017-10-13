/*
 * Copyright (C) 2016 BetterLife Co.,Ltd.
 * All rights reserved.
 *
 */
#ifndef algorithm_interface_h
#define algorithm_interface_h

#define Android_VER

#define TAG          "btl_algo"

#define R5J00

#ifdef Android_VER
#include <android/log.h>
#else
#if defined(__cplusplus)

#define _EXTERN_C                extern "C"
#define _BEGIN_EXTERN_C          extern "C" {
#define _END_EXTERN_C            }

#else
#include <stdio.h>
#include <stdarg.h>

#define _EXTERN_C
#define _BEGIN_EXTERN_C
#define _END_EXTERN_C
#endif // defined(__cplusplus)
#define _TLAPI_EXTERN_C     _EXTERN_C
#define _TLAPI_NORETURN     _NORETURN

//_TLAPI_EXTERN_C void  tlApiLogvPrintf(const char* fmt, va_list args);
_TLAPI_EXTERN_C void  tlApiLogPrintf(const char* fmt, ...);
#endif
/* Decisions. */
#define DECISION_MATCH       1
#define DECISION_NON_MATCH   0

#if (defined R5J00)
#define default_algorithm_type  28  // neo_bf3182_algorithm
#else
#define default_algorithm_type  8  // neo_square_s_algorithm
#endif 


/** Algorithm used for small square sensors, approximately 5.8x5.8 mm
  * (115x115 pixels @ 500 dpi). */
// 1 hybrid_square_s_algorithm;

/* Same as above, but with extractor 'optimized for speed'. */
// 2 hybrid_square_s_speed_algorithm;

/* Same as above, but with extractor 'optimized for speed and memory'. */
// 3 hybrid_square_s_speed_mem_algorithm;

/* Same as above, but with using EHM instead of EHM-HR templates. */
// 4 hybrid_square_s_non_hr_algorithm;

/* Same as above, but with extractor 'optimized for speed'. */
// 5 hybrid_square_s_non_hr_speed_algorithm;

/* Same as above, but with extractor 'optimized for speed and memory'. */
// 6 hybrid_square_s_non_hr_speed_mem_algorithm;

/** Algorithm used for extra small square sensors, approximately 5x5 mm
  * (100x100 pixels @ 500 dpi). */
// 7 hybrid_square_xs_algorithm;


/** Algorithm used for small square sensors, approximately 5.8x5.8 mm
  * (115x115 pixels @ 500 dpi). */
// 8 neo_square_s_algorithm;

/** Same as above, but with extractor 'optimized for speed'. */
// 9 neo_square_s_speed_algorithm;

/** Algorithm used for extra small square sensors, approximately 5x5 mm
  * (100x100 pixels @ 500 dpi). */
// 10 neo_square_xs_algorithm;

/** Same as above, but with extractor 'optimized for speed'. */
// 11 neo_square_xs_speed_algorithm;

/** Algorithm used for medium rectangular sensors, approximately 10x4 mm
  * (200x80 pixels @ 500 dpi). */
// 12 pb_algorithmI neo_rectangular_m_algorithm;

/** Same as above, but with extractor 'optimized for speed'. */
// 13 pbif_const pb_algorithmI neo_rectangular_m_speed_algorithm;

/** Algorithm used for small rectangular sensors, approximately 10x3 mm
  * (200x60 pixels @ 500 dpi). */
// 14 pbif_const pb_algorithmI neo_rectangular_s_algorithm;

/** Same as above, but with extractor 'optimized for speed'. */
// 15 pbif_const pb_algorithmI neo_rectangular_s_speed_algorithm;

/** Neo algorithm for Henry sensor. Based on neo_square_xs_algorithm. Only used in R4H */
// 16 pbif_const pb_algorithmI neo_henry_algorithm;

/** Algorithm used for medium square sensors, approximately 6.5x6.5 mm
  * (130x130 pixels @ 500 dpi). */
// 17 pbif_const pb_algorithmI cardo_square_m_algorithm;

/** Algorithm used for small square sensors, approximately 5.8x5.8 mm
  * (115x115 pixels @ 500 dpi). */
// 18 pbif_const pb_algorithmI cardo_square_s_algorithm;

/** Algorithm used for extra small square sensors, approximately 5x5 mm
  * (100x100 pixels @ 500 dpi). */
// 19 pbif_const pb_algorithmI cardo_square_xs_algorithm;

/** Algorithm used for extra extra small square sensors, approximately 4x4 mm
  * (80x80 pixels @ 500 dpi). */
// 20 pbif_const pb_algorithmI cardo_square_xxs_algorithm;

/** Algorithm used for medium rectangular sensors, approximately 10x4 mm
  * (200x80 pixels @ 500 dpi). */
// 21 pbif_const pb_algorithmI cardo_rectangular_m_algorithm;

/** Algorithm used for small rectangular sensors, approximately 10x3 mm
  * (200x60 pixels @ 500 dpi). */
// 22 pbif_const pb_algorithmI cardo_rectangular_s_algorithm;

/* the following algorithms only be support in PB R5J00 version */
/** Ehm-nonhr algorithm for BF3182 sensor. Based on ehm_nonhr_rectangular_s_algorithm. */
// 23 pbif_const pb_algorithmI ehm_nonhr_bf3182_algorithm;

/** Neo algorithm for BF3182 sensor. Based on neo_rectangular_s_algorithm. */
// 24 pbif_const pb_algorithmI neo_bf3182_algorithm;

/** Same as above, but optimized for speed. */
// 25 pbif_const pb_algorithmI neo_bf3182_speed_algorithm;

/** Cardo algorithm for BF3182 sensor. Based on cardo_rectangular_s_algorithm. */
// 26 pbif_const pb_algorithmI cardo_bf3182_algorithm;

/** Ehm-nonhr algorithm for bf3290 sensor. Based on ehm_nonhr_square_xs_algorithm. */
// 27 pbif_const pb_algorithmI ehm_nonhr_bf3290_algorithm;

/** Neo algorithm for bf3290 sensor. Based on neo_square_xs_algorithm. */
// 28 pbif_const pb_algorithmI neo_bf3290_algorithm;

/** Same as above, but optimized for speed. */
// 29 pbif_const pb_algorithmI neo_bf3290_speed_algorithm;

/** Cardo algorithm for bf3290 sensor. Based on cardo_square_xs_algorithm. */
// 30 pbif_const pb_algorithmI cardo_bf3290_algorithm;

/** Algorithm used for small square sensors, approximately 5.8x5.8 mm
  * (115x115 pixels @ 500 dpi). */
// 35 spectral_square_s_algorithm;

/** Algorithm used for small square sensors, approximately 4.0x4.0 mm
  * (80x80 pixels @ 500 dpi). */
// 36 neo_square_xxs_speed_algorithm;
/** Algorithm used for small square sensors, approximately 4.0x4.0 mm
  * (80x80 pixels @ 500 dpi). */
// 37 neo_square_xxs_algorithm;

#if (defined R3S00)
typedef enum bf_pb_far_e {
    BF_PB_FAR_1               = 0,
    BF_PB_FAR_5               = 1,
    BF_PB_FAR_10              = 2,
    BF_PB_FAR_50              = 3,
    BF_PB_FAR_100             = 4,
    BF_PB_FAR_500             = 5,
    BF_PB_FAR_1000            = 6,
    BF_PB_FAR_5000            = 7,
    BF_PB_FAR_10000           = 8,
    BF_PB_FAR_50000           = 9,
    BF_PB_FAR_100000          = 10,
    BF_PB_FAR_500000          = 11,
    BF_PB_FAR_1000000         = 12,
    BF_PB_FAR_5M              = 13,
    BF_PB_FAR_10M             = 14,
    BF_PB_FAR_50M             = 15,
    BF_PB_FAR_100M            = 16,
    BF_PB_FAR_500M            = 17,
    BF_PB_FAR_1000M           = 18
} bf_pb_far_t;



#else

typedef enum bf_pb_far_e {
    BF_PB_FAR_1               = 0,
    BF_PB_FAR_2               = 1,
    BF_PB_FAR_5               = 2,
    BF_PB_FAR_10              = 3,
    BF_PB_FAR_20              = 4,
    BF_PB_FAR_50              = 5,
    BF_PB_FAR_100             = 6,
    BF_PB_FAR_200             = 7,
    BF_PB_FAR_500             = 8,
    BF_PB_FAR_1000            = 9,
    BF_PB_FAR_2K              = 10,
    BF_PB_FAR_5000            = 11,
    BF_PB_FAR_10000           = 12,
    BF_PB_FAR_20K             = 13,
    BF_PB_FAR_50000           = 14,
    BF_PB_FAR_100000          = 15,
    BF_PB_FAR_200K            = 16,
    BF_PB_FAR_500000          = 17,
    BF_PB_FAR_1000000         = 18,
    BF_PB_FAR_2M              = 19,
    BF_PB_FAR_5M              = 20,
    BF_PB_FAR_10M             = 21,
    BF_PB_FAR_20M             = 22,
    BF_PB_FAR_50M             = 23,
    BF_PB_FAR_100M            = 24,
    BF_PB_FAR_200M            = 25,
    BF_PB_FAR_500M            = 26,
    BF_PB_FAR_1000M           = 27,
    BF_PB_FAR_Inf             = 28
} bf_pb_far_t;
#endif


/** The condition of the fingerprint is soaked (very wet). */
//#define PB_CONDITION_SOAKED           0
#define BL_CONDITION_SOAKED             0
/** The condition of the fingerprint is bone dry (very dry). */
//#define PB_CONDITION_BONE_DRY        100
#define BL_CONDITION_BONE_DRY          100

/* Image qualities. */

/** The worst quality. */
//#define PB_IMAGE_QUALITY_WORST      0
#define BL_IMAGE_QUALITY_WORST        0
/** The best quality. */
//#define PB_IMAGE_QUALITY_BEST       100
#define BL_IMAGE_QUALITY_BEST         100

typedef struct btl_template_data {
    unsigned char *enrolled_template_data;
    int  enrolled_template_type;
    int  enrolled_template_size;
} btl_template_data_t;

typedef struct btl_authenticate_multiple_data {
    btl_template_data_t  enrolled_template_list[5];
    btl_template_data_t  authenticate_template;
    int                  nbr_of_enrolled_templates;
} btl_authenticate_multiple_data_t;

typedef struct btl_algorithm_config {
    /** The max number of subtemplates that may be stored in a multitemplate.
     * The value is set as an attribute in the template.
     * Default value is algorithm dependent. Max value is 256*/
    int max_nbr_of_subtemplates;

    /** Tells if the encoded multitemplate shall be locked for further updates
      * when enrollment is completed, i.e. if the ability to enroll additional
      * templates into the multitemplate shall be disabled. The value is set
      * as an attribute in the template. Default 0. */
    int lock_template_from_further_updates;

    /** Tells if multiple fingers are allowed to be enrolled in a multitemplate.
      * Default 0.
      * Note: For smaller sensors it is strongly recommended that
      * prevent_enrollment_of_multiple_fingers is set to 0 to allow for a good
      * enrollment, although that multiple fingers may be enrolled! */
    int prevent_enrollment_of_multiple_fingers;

    /** Tells if the enrollment controller shall keep duplicate templates instead of
      * rejecting them. Default 0. */
#if (defined R5B00 || defined R5J00)
    int keep_duplicate_templates;
#endif

    /** Maximum template size in bytes. Applicable at enrollment. When
      * non-zero this parameter enforces a limit on how large the
      * template may be, including dynamic updates. The value is set
      * as an attribute in the template. Default value is 0 - no limit.
      * Max size is 60 * 1024 for betterlife */
    uint32_t max_template_size;

    /** The minimum fingerprint area, in mm^2, required for each captured image
      * to be used for e.g. enrollment or verification. Default value is
      * algorithm dependent, but ranges from 70-80% of the total image area. */
    uint32_t minimum_area_per_template;

    /** The minimum fingerprint area, in mm^2 for the combined area of all of the
      * templates in the multitemplate, required to complete enrollment. Default
      * value is algorithm dependent. */
    uint32_t minimum_area;

    /** The minimum fingerprint quality, ranging from 1 (worse) to 100 (best),
      * required for each captured image to be used for e.g. enrollment or
      * verification. default 20. Max value is 256*/
    int minimum_quality;

} btl_algotithm_config_t;

#if 0 // for reference 
/** Sensor size. */
/** Sensor size. */
typedef enum {
    /** Unknown sensor size. */
    PB_SENSOR_SIZE_UNKNOWN = 0,

    /** A full size sensor, covering an entire finger. */
    PB_SENSOR_SIZE_FULL,

    /** Square sensors. */
    /** Extra Large (XL) square sensor, approximately 10x10 mm (200x200 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_SQUARE_XL,
    /** Large (L) square sensor, approximately 8x8 mm (160x160 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_SQUARE_L,
    /** Medium (M) square sensor, approximately 6.5x6.5 mm (130x130 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_SQUARE_M,
    /** Small (S) square sensor, approximately 5.8x5.8 mm (115x115 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_SQUARE_S,
    /** Extra Small (XS) square sensor, approximately 5x5 mm (100x100 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_SQUARE_XS,

    /** Medium (M) rectangular sensor, approximately 10x4 mm (200x80 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_RECTANGULAR_M,
    /** Small (S) rectangular sensor, approximately 10x3 mm (200x60 pixels @ 500 dpi). */
    PB_SENSOR_SIZE_RECTANGULAR_S,
} pb_sensor_size_t;

/* The possible return codes from the PB interfaces. */
typedef enum pb_rc_e {
    /** The function returned without errors. */
    PB_RC_OK = 0,

    /** The requested operation is not supported by the implementation. */
    PB_RC_NOT_SUPPORTED = 1,

    /** At least one of the parameters is invalid. */
    PB_RC_INVALID_PARAMETER = 2,

    /** The data passed to the function has the wrong format. */
    PB_RC_WRONG_DATA_FORMAT = 3,

    /** At least one buffer has an incorrect size. */
    PB_RC_WRONG_BUFFER_SIZE = 4,

    /** A function is called before the interface being initialized. */
    PB_RC_NOT_INITIALIZED = 5,

    /** The requested item was not found. */
    PB_RC_NOT_FOUND = 6,

    /** The function returned because the caller canceled it. */
    PB_RC_CANCELLED = 7,

    /** The operation timed-out before it could finish the operation. */
    PB_RC_TIMED_OUT = 8,

    /** Cannot allocate enough memory. */
    PB_RC_MEMORY_ALLOCATION_FAILED = 9,

    /** Unable to open a file. */
    PB_RC_FILE_OPEN_FAILED = 10,
    /** Unable to read from file. */
    PB_RC_FILE_READ_FAILED = 11,
    /** Unable to write to a file. */
    PB_RC_FILE_WRITE_FAILED = 12,

    /** Reader is not connected or not started. */
    PB_RC_READER_NOT_AVAILABLE = 13,

    /** Reader has been locked by another user. */
    PB_RC_READER_BUSY = 14,

    /** The enrollment failed because none of the images matched each
      * other. This only applies if multiple images are required for
      * enrollment. */
    PB_RC_ENROLLMENT_VERIFICATION_FAILED = 15,

    /** The finger is temporarily blocked due to brut force attack attempt.
      * As long as the finger is blocked, all verification attempts will fail. */
    PB_RC_FINGER_BLOCKED = 16,

    /** An undefined fatal error has occurred. This error code is used
      * for errors that "cannot happen" and isn't covered by any other
      * error code. */
    PB_RC_FATAL = 17,

    /** The selected sensor type and size is not supported by the
      * verifier to calculate score or make decisions. Choose one
      * of the supported algorithm combinations. */
    PB_RC_SENSOR_NOT_SUPPORTED = 18,

    /** The function has exceeded maximum capacity and cannot accept
     * more data. */
    PB_RC_CAPACITY = 19,

    /** No algorithm as been selected. This is a requirement for newer
     * versions of the API.
     */
    PB_RC_NO_ALGORITHM = 20,

} pb_rc_t;




#endif

typedef void (*load_fingerprint_data_t)(const char *pDbName,
                                        int   uid,
                                        int   fid,
                                        unsigned char*  pFpData,
                                        unsigned char*  pType,
                                        int*  pDataSize,
                                        int*  pStatus);

typedef void (*store_fingerprint_data_t)(const char *pDbName,
        unsigned char* pFpData,
        int  uid,
        int  fid,
        unsigned int nSize,
        int* pStatus);


typedef void (*get_pb_feedback_event)(uint8_t pb_feedback_event);

typedef enum {
    PB_FEEDBACK_EVENT_0 = 0, //NONE
    PB_FEEDBACK_EVENT_1 = 1, //MOVE_FINGER_MORE
    PB_FEEDBACK_EVENT_2 = 2, //MOVE_FINGER_LESS
    PB_FEEDBACK_EVENT_3 = 3, //DUPLICATE_TEMPLATE
    PB_FEEDBACK_EVENT_4 = 4  //TEMPLATE_ENROLLED
} pb_feedback_event_t;


/**
*
* Create and init algorithm
*
* return int:  0 : init success
*             <0 : init error
*
*
*
*/
int Btl_InitAlgo(void);

/**
*
* Release algorithm
*
* return int: 0: uninstall success
*
*
*/
int Btl_UninitAlgo(void);


/**
*
* Check image's quality
* @param[in] pImage: fingerprint's image
* @param[in] image_width: fingerprint image's width
* @param[in] image_height: fingerprint image's height
* @param[in] area_threshold: threshold of image's valid area, if it is 0,
*               the route will use default's value.
* @param[in] score_threshold: threshold of image's quality score, if it is 0,
*               the route will use default's value.
* @param[out] p_im_score: fingerprint's score of quality calculated by algorithm
* @param[out] p_im_area: fingerprint's valid area calculated by algorithm
* @param[out] p_im_condition: conditon of fp's image. 0:very wet; 100: very dry
* return   int:   0: image quality is ok
*
*/
int Btl_CheckImageQuality(
    unsigned char* pImage,
    int  image_width,
    int  image_height,
    int  area_threshold,
    int  score_threshold,
    int* p_im_score ,
    int* p_im_area,
    int* p_im_condition,
    int  enhance_image_enable);
/**
* Enroll fingeprint
* @param[in] pImage: fingerprint's image
* @param[in] image_width: fingerprint image's width
* @param[in] image_height: fingerprint image's height
* @param[in] check_quality_enable: set if check image quality before enrollment
* @param[in, out] p_reference_counts: Counts of template for a set of fingerprint
*                  if enrollment success, *p_reference_counts will increase
*
* @param[out] p_enrolled_coverage is the returned coverage, in percent, of the merged
*               fingerprint (computed from all captured fingerprints).
* @param[in] image_pb_template: image template, if it is 0, function will use image to enroll. 
*
* return int: 0: enroll success and *p_reference_counts add 1, others fail
*
*/
int Btl_Enroll(
    unsigned char *pImage,
    int  image_width,
    int  image_height,
    int  check_quality_enable,
    int  enhance_image_enable,
    int* p_reference_counts,
    int* p_enrolled_coverage,
    void *image_pb_template);


/**
* Create and get a set of template data, this function is called after all enrollment completed.
* @param[in, out]*p_data_buffer: buffer used to get template's data which don't include type from algorithm
* @param[in]  data_buffer_size: size of p_data_buffer
* @param[out] *p_template_type: type of template.
* @param[out] *p_template_size: actual size of template's data in buffer p_data_buffer point to .
*
* return  int:  0: Success
*
*
*/
int Btl_GetTemplateData(unsigned char* p_data_buffer,
                        int  data_buffer_size,
                        unsigned char* p_template_type,
                        int* p_template_size);


/** Verifies a verification template against one enrolled multitemplates and
  * delivers a decision based on the requested FAR level.
  *
  * NOTE: This function does not support automatic dynamic update of the enrolled
  *       template.
  *
  * @param[in] *pImage is pointer which point to image's buffer
  * @param[in] image_width is width of fp's image
  * @param[in] image_height is height of fp's image.
  * @param[in] false_accept_rate is the requested false accept rate, see
  *            PB_FAR_X.
  * @param[in] enhance_image_enable is parameters control if enhance image before verifying
  * @param[in] enrolled_template_data is buffer saved enrolled templates to verify against
  * @param[in] enrolled_template_size is size of enrolled template
  * @param[in] enrolled_template_type is type of enrolled template
  * @param[out] matched_score is matched score
  *
  * @return PB_DECISION_MATCH if verify successful, or verify fail or an error code.
  */
int Btl_Match(
    unsigned char* pImage,
    int  image_width,
    int  image_height,
    unsigned char*  enrolled_template_data,
    unsigned char   enrolled_template_type,
    unsigned int    enrolled_template_size,
    int  false_accept_rate,
    int  enhance_image_enable,
    unsigned short* matched_score);


/** Verifies a verification template against one or more enrolled multitemplates and
  * delivers a decision based on the requested FAR level.
  *
  * NOTE: This function does not support automatic dynamic update of the enrolled
  *       template.
  *
  * @param[in] *pDbName is the database's name for fp.
  * @param[in] *pImage is pointer which point to image's buffer
  * @param[in] image_width is width of fp's image
  * @param[in] image_height is height of fp's image.
  * @param[in] false_accept_rate is the requested false accept rate, see
  *            PB_FAR_X.
  * @param[in] uid is the user's id.
  * @param[in] num_fids is the number of enrolled templates
  *            to verify against.
  * @param[in] enhance_image_enable is parameters control if enhance image before verifying
  * @param[in] enrolled_template_data is buffer saved enrolled templates to verify against
  * @param[in] enrolled_template_data_size is buffer saved size of every enrolled templates
  * @param[out] matching_enrolled_template_index is the index of the enrolled template
  *             that matched the verification template, if any. May be set to 0
  *             if not needed.
  *
  * @return PB_DECISION_MATCH if verify successful, or verify fail or an error code.
  */
int Btl_MatchB(const char *pDbName,
               unsigned char *pImage,
               int  image_width,
               int  image_height,
               int  false_accept_rate,
               unsigned char  uid,
               int  nbr_of_enrolled_templates,
               int  enhance_image_enable,
               int  *matching_enrolled_template_index,
               unsigned char** enrolled_template_data,
               int* enrolled_template_data_size);

/** Verifies a verification template against one or more enrolled multitemplates and
  * delivers a decision based on the requested FAR level.
  *
  * NOTE: This function support automatic dynamic update of the enrolled
  *       template.
  *
  * @param[in] *pDbName is the database's name for fp.
  * @param[in] *pImage is pointer which point to image's buffer
  * @param[in] image_width is width of fp's image
  * @param[in] image_height is height of fp's image.
  * @param[in] false_accept_rate is the requested false accept rate, see
  *            PB_FAR_X.
  * @param[in] uid is the user's id.
  * @param[in] nbr_of_enrolled_templates is the number of enrolled templates
  *            to verify against.
  * @param[in] enhance_image_enable is parameters control if enhance image before verifying
  * @param[in] enrolled_template_data is buffer saved enrolled templates to verify against
  * @param[in] enrolled_template_data_size is buffer saved size of every enrolled templates
  * @param[out] matching_enrolled_template_index is the index of the enrolled template
  *             that matched the verification template, if any. May be set to 0
  *             if not needed.
  *
  * @param[out] update_template_data is data of updated template.
  *
  * @param[out] update_template_size is size of updated template.
  *
  * @param[out] update_template_type is type of updated template.
  * @param[out] update_enable decide to whether verified template is update into enrolled template.
  * @param[in]  update_false_accept_rate is false accept rate used for dynamic updating.
  * @param[in]  image_pb_template is template for captured fp's image, if it is 0, function will use image to match.
  * @return PB_DECISION_MATCH if verify successful, or verify fail or an error code.
  */
int Btl_MatchC(
    const char* pDbName,
    unsigned char* pImage,
    int  image_width,
    int  image_height,
    int  false_accept_rate,
    unsigned char  uid,
    int  nbr_of_enrolled_templates,
    int  enhance_image_enable,
    int *matching_enrolled_template_index,
    unsigned char **enrolled_template_data,
    int *enrolled_template_data_size,
    unsigned char *update_template_data,
    int *update_template_size,
    unsigned char *update_template_type,
    unsigned char update_enable,
    int  update_false_accept_rate,
    void *image_pb_template);

/** Verifies a verification template against one or more enrolled multitemplates and
  * delivers a decision based on the requested FAR level.
  *
  * NOTE: This function support automatic dynamic update of the enrolled
  *       template and can set far of dynamic update
  *
  * @param[in] *pDbName is the database's name for fp.
  * @param[in] *pImage is pointer which point to image's buffer
  * @param[in] image_width is width of fp's image
  * @param[in] image_height is height of fp's image.
  * @param[in] false_accept_rate is the requested false accept rate, see
  *            PB_FAR_X.
  * @param[in] uid is the user's id.
  * @param[in] nbr_of_enrolled_templates is the number of enrolled templates
  *            to verify against.
  * @param[in] enhance_image_enable is parameters control if enhance image before verifying
  * @param[in] enrolled_template_data is buffer saved enrolled templates to verify against
  * @param[in] enrolled_template_data_size is buffer saved size of every enrolled templates
  * @param[out] matching_enrolled_template_index is the index of the enrolled template
  *             that matched the verification template, if any. May be set to 0
  *             if not needed.
  *
  * @param[out] update_template_data is data of updated template.
  *
  * @param[out] update_template_size is size of updated template.
  *
  * @param[out] update_template_type is type of updated template.
  * @param[in] update_enable decide to whether verified template is update into enrolled template.
  * @param[in] update_false_accept_rate is far of dynamic update
  * @return PB_DECISION_MATCH if verify successful, or verify fail or an error code.
  */
int Btl_MatchD(
    unsigned char* pImage,
    int  image_width,
    int  image_height,
    int  false_accept_rate,
    int  enhance_image_enable,
    int * matched_enrolled_template_index,
    btl_authenticate_multiple_data_t *multiple_enrolled_data);

/**
*
* Enhance image's by pb's algorithm.
* @param[in] p_input_image: fingerprint's image
* @param[out]p_output_image: enhanced image of fp
* @param[in] image_width:  fingerprint image's width
* @param[in] image_height: fingerprint image's height
* @paran[in] enhance_type: 0:don't enhance; 1: pb enhance; 2: pb betterlife enhance
*
* return   int:   0: image quality is ok
*
*/
int  Btl_EnhanceImage(unsigned char *p_input_image,
                      unsigned char* p_output_image,
                      int image_width,
                      int image_height,
                      int enhance_type);

/**
*
* Create and init algorithm and configure parameters
*
* return int:  0 : init success
*             <0 : init error
*
*
*
*/
int Btl_InitAlgoWithConfig(btl_algotithm_config_t *algorithm_config_params);
int Btl_SetAlgorithmType(int algorithm_type_params);
int Btl_SetAlgorithmID(unsigned int id );
int Btl_GetCopyrightInfo(char* pCopyrightInfo);
int Btl_GetReleasedTime(char* pReleasedTime);
int Btl_GetSDKVersion(char* pSDKVer);
int Btl_GetAlgorithmVersion(char* pAlgorithmVersion);
int Btl_EnableDebug(int debugE);
int Btl_GetMteStatus();


int Btl_DynamicUpdate(
    unsigned char* p_original_image,
    int  image_width,
    int  image_height,
    int  false_accept_rate,
    int  enhance_image_enable,
    unsigned char* enrolled_template_data,
    int  enrolled_template_data_size,
    unsigned char* update_template_data,
    int* update_template_size,
    unsigned char* update_template_type
);


int Btl_SetFeedback(get_pb_feedback_event p);


int Btl_ImgExtractsTemplate(
    unsigned char* pImage,
    int  image_width,
    int  image_height,
    int  enhance_image_enable,
    void **extracts_template);

int Btl_DelImgTemplate(void *pb_fp_template);

struct iland_info {
     //struct point_data corners_data[4];
     char   latest_str[10];
     char   duplicate_str[10];
     int    flag;
};

struct ilands_data {
     struct iland_info *ilands_list[50];
     int nbr_of_islands;
};

typedef void (*get_pb_feedback_ilands)(struct ilands_data *iland_feedback);


int Btl_SetIandFeedback(get_pb_feedback_ilands p);
#endif
