#ifndef __FP_CA_ENTRY_H__
#define __FP_CA_ENTRY_H__

#include "fp_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VAR(v)     ((void)v)

fp_error_t fp_ca_open_session(void);
void fp_ca_close_session(void);
fp_error_t fp_ca_invoke_command_from_user(void *cmd, int len, uint8_t cmdType);

#ifdef __cplusplus
}
#endif

#endif 
