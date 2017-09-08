#ifndef __ALI_FP_INTERFACE_H__
#define __ALI_FP_INTERFACE_H__

enum {
    FP_OK = 0,
    FP_MATCHOK,
    FP_FINISH,

    FP_ERR = 1000,
    FP_TIMEOUT,
    FP_BADIMAGE,
    FP_FEATURELOW,
    FP_DUPLICATE,
    FP_NOTDUPLICATE,
    FP_MATCHFAIL,
    FP_NULL_DATA,
    FP_INVALID_FEATURE_LEN,
    FP_ALLOC_MEM_FAIL,
    FP_INVALID_FORMAT,
    FP_TUNING_FAIL,
    FP_INVALID_BUFFER_SIZE,
    FP_IO_ACCESS_FAIL,
    FP_NOT_TUNING,
    FP_CONNECT_SENSOR_FAIL,
    FP_CALL_FAILED,
    FP_NOT_CONNECT_SENSOR,
    FP_NULL_ENROLL_DATA,
    FP_NULL_FEATURE,
    FP_GETIMAGEFAIL
};

int  fp_init_device(void);

int  fp_deinit_device(void);
/**
 * Function Description: Clear the IRQ, when without fingerprint context.
 *
 * Return Value: FP_OK
 */
int  fp_clear_irq(void);

/*
 * Function Description:  When the device reboot, it will do update calibration action
 *
 *Return Value:FP_ERR, FP_OK
 *
 */
int  fp_update_calibration(void);

/*
 * Function Description:  Connect the sensor, initialize Algorithm
 * @return FP_OK or FP_ERR
 *
 */
int  fp_connect(void);

/*
 * Function Description:  Disconnect the sensor
 * @return FP_OK or FP_ERR
 *
 */
int  fp_disconnect(void);

/*
 * Function Description:  Check if user's finger press the sensor. (Finger down)
 * @return FP_OK or FP_ERR
 */
int  fp_finger_detect(void);

/*
 * Function Description:  Check if user's finger leave the sensor. (Finger up)
 * @return FP_OK or FP_ERR
 *
 */
int  fp_finger_leave(void);

/*
 * Function Description:  Capture image from the sensor.
 * @return FP_OK or other
 *
 */
int  fp_get_image(void);

/*
 * Function Description:  Get FP enrollment progress
 *
 * @param[out], ratio, enrollment progress percentage(enrollment complete when 100)
 *
 * @return FPC_OK - Enrol ok, not finish
 *         FP_FINISH - Enrol finish
 *         FP_NULL_DATA - if data is NULL
 *         FP_BADIMAGE - Bad image
 *         FP_ALLOC_MEM_FAIL - if memory allocation failed during the enrollment
 */
int  fp_do_enroll(unsigned char *ratio);

/*
 * Function Description:  Ends the enrollment and returns the enrolled finger template.
 * @param[in], uid:  user id, it supports multi-user mode
 * @param[in], fpid: fpid, it means finger id
 * @return FP_OK
 *         FP_ERR - if get template failed
 */
int  fp_finish_enroll(int uid, int fpid);

/*
 * Function Description:  Finger print duplicate check
 * @param[in], uid:  user id
 *  Return Value:FP_DUPLICATE,Duplicated;FP_NOTDUPLICATE,No duplicated fingerprint
 */
int  fp_duplicate_check(int uid);

/*
 * Function Description:  End the started identification. If there was a template update during the
 * identification process, the given template struct will be filled with
 * template data. If a null pointer is given the update will be discarded.
 *
 * @param[in],  uid: user id
 * @param[out], fpid:return the finger id which match
 *
 * @return FP_LIB_OK
 *         FP_MATCHOK - if template matches the fingerprint.
 *         FP_MATCHFAIL - NOT match
 */
int  fp_do_verify(int uid, int *fpid);

/*
 * Function Description:  Get uid list info.
 * @param[out], uids:  return uid array
 * @param[out], uidscount: return uid array size, it can set 5 as max value.
 * @return FP_OK or FP_ERR
 */
int  fp_get_uids(int *uids, int *uidsCount);

/*
 * Function Description:  Get fpid list info by uid.
 * @param[in], uid:  user id
 * @param[out], ids:  return fpid array
 * @param[out], idscount: return fpid array size, it can set 5 as max value.
 * @return FP_OK or FP_ERR
 */
int  fp_get_fpids(int uid, int *ids, int *idsCount);

/*
 * Function Description:  Get finger name by uid and fpid.
 * @param[in], uid:  user id
 * @param[in], fpid: finger id
 * @param[out], name: finger name
 * @param[out], len: finger name length.
 * @return FP_OK or FP_ERR
 */
int  fp_get_finger_name(int uid, int fpid, char *name, int *len);

/*
 * Function Description:  Set finger name by uid and fpid.
 * @param[in], uid:  user id
 * @param[in], fpid: finger id
 * @param[in], name: finger name
 * @param[in],  len: finger name length.
 * @return FP_OK or FP_ERR
 */
int  fp_set_finger_name(int uid, int fpid, char *name, int len);

/*
 * Function Description:  Delete all template and fpid information by uid.
 * @param[in], uid:  user id, it supports multi-user mode
 * @param[in], fpid: fpid, it means finger id
 * @return FP_OK or FP_ERR
 */
int  fp_delete_uid(int uid);

/*
 * Function Description:  Delete one template by uid and fpid.
 * @param[in], uid:  user id
 * @param[in], fpid: fpid, it means finger id
 * @return FP_OK or FP_ERR
 */
int  fp_delete_fpid(int uid, int fpid);

int  fp_suspend(void);
int  fp_resume(void);
int  fp_update_firmware();
int  fp_update_firmware_cfg();
int  fp_check_esd();
int  fp_algo_uninit();
int  fp_algo_init();
int  fp_algo_vendor(char *, int);


#define TAG       "btl_algo"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#endif   /* __ALI_FP_INTERFACE_H__ */
