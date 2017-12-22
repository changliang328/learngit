/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CADEMO_IPC_H__
#define __CADEMO_IPC_H__
__BEGIN_DECLS



#define TADEMO_PORT "com.android.trusty.tademo"
#define TADEMO_MAX_BUFFER_LENGTH 1024

/*enum tademo_command {
	TA_REQ_SHIFT = 1,
	TA_RESP_BIT  = 1,

	TA_INCREASE       = (0 << TA_REQ_SHIFT),

};*/

 
enum tademo_command {
	TA_REQ_SHIFT = 1,
	TA_RESP_BIT  = 1,

	TA_INCREASE       = (0 << TA_REQ_SHIFT),
  TA_CALL_FROME_OTHER_TA 	  = (1 << TA_REQ_SHIFT),

	 CHIPS_FINGERPRINT_INIT_LIB = 10,
	 CHIPS_FINGERPRINT_GET_FID =  11,
	 CHIPS_FINGERPRINT_LOAD_FINGERPRINTS =12,
	 CHIPS_FINGERPRINT_PREPARE_ENROLL =13,
	 CHIPS_FINGERPRINT_ENROLL_FINGER=14,
	 CHIPS_FINGERPRINT_UPDATE_TEMPLATE=15,
	 CHIPS_FINGERPRINT_VERIFY=16,
	 CHIPS_FINGERPRINT_RENEW_FEATURE=17,
	 CHIPS_FINGERPRINT_DELETE_FINGER=18,
	 CHIPS_FINGERPRINT_GET_FINGERNUM=19,
	 CHIPS_FINGERPRINT_SENSOR_CONFIG=20,
	 CHIPS_FINGERPRINT_SET_SENSOR_MODE=21,
	 CHIPS_FINGERPRINT_DETECT_FINGER_DOWN=22,
	 CHIPS_FINGERPRINT_PROBE_SENSOR_ID=23,
	 CHIPS_FINGERPRINT_WRITE_16_CLOCK=24,
	 CHIPS_FINGERPRINT_WAKEUP_SPI=25,
	 CHIPS_FINGERPRINT_GET_AUTH_ID=26,
	 CHIPS_FINGERPRINT_GET_CHALLENGE=27,
	 CHIPS_FINGERPRINT_VERIFY_AUTH=28,
	 CHIPS_GET_AUTH_TOKEN =29,
	 CHIPS_FINGERPRINT_CMD50_READ_INFO=30,
	 CHIPS_FINGERPRINT_CMD51_WRITE_INFO=31,
	 CHIPS_FINGERPRINT_SCAN_AND_SAVE_IMAGE=32,
	 CHIPS_FINGERPRINT_SCAN_ONE_IMAGE=33,
	 CHIPS_FINGERPRINT_DEAD_PIXEL=34,  
	 CHIPS_FINGERPRINT_CHECK_IRQ=35,
	 CHIPS_FINGERPRINT_WRITE_DEL_FILE_TEST=36,
	 CHIPS_FINGERPRINT_DEL_FINGERPRINT_DB=37,
	 
};

typedef enum {
    ERROR_NONE = 0,
    ERROR_FIRST = 1,
    ERROR_UNKNOWN = 2,
} tademo_error_t;


/**
 * tademo_message - Serial header for communicating with ta server
 * @cmd: the command, one of xx, xx. Payload must be a serialized
 *       buffer of the corresponding request object.
 * @payload: start of the serialized command specific payload
 */
struct tademo_message {
    uint32_t cmd;
    uint8_t payload[0];
};




int trusty_cademo_connect();
int trusty_cademo_call(uint32_t cmd, void *in, uint32_t in_size, uint8_t *out,
                           uint32_t *out_size);
void trusty_cademo_disconnect();

__END_DECLS


#endif