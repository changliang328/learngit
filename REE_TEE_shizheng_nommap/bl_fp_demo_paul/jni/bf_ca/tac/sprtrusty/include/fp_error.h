#ifndef __FP_ERROR_H__
#define __FP_ERROR_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum fp_error {
    FP_SUCCESS = 0,

    //ta error
    FP_ERROR_BASE = 1000,
    FP_ERROR_TA_DEAD = FP_ERROR_BASE + 50,
    FP_ERROR_OPEN_TA_FAILED = FP_ERROR_BASE + 51,
    FP_ERROR_CA_TO_TA = FP_ERROR_BASE + 52,
    FP_ERROR_FILE_OPERATOIN = FP_ERROR_BASE + 53,
    FP_ERROR_WRONG_DATA = FP_ERROR_BASE + 54  ,
    FP_ERROR_SPI_LOOKBACK = FP_ERROR_BASE + 55
} fp_error_t;

#ifdef __cplusplus
}
#endif

#endif 
