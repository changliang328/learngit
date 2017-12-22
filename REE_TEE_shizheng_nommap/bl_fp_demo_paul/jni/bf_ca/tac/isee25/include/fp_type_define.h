#ifndef __FP_TYPE_DEFINE_H__
#define __FP_TYPE_DEFINE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FP_OPERATION_ID 1
#define FP_USER_OPERATION_ID 2

#define MAX_FP_NAME 128
#define MAX_FP_VERSION 128

// for beanpod tee
typedef struct {
    uint32_t head1;
    uint32_t head2;
    uint32_t head3;
    uint32_t len;
} fp_cmd_header_t;

#ifdef __cplusplus
}
#endif

#endif // __FP_TYPE_DEFINE_H__
