#ifndef __BF_CUSTOM_H__
#define __BF_CUSTOM_H__

//#define BUILD_TEE
#include "bf_log.h"

//#define USE_CRC_CHECK
#ifdef BUILD_TEE

#if defined (ISEE_TEE)
#include <stdint.h>
#include <contrib/libtsfs/f_fs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#elif defined (TK_TEE)

#endif

#else //not TEE
#include "bf_types.h"
#include <stdlib.h>

#endif
#include "bf_tee_platform_api.h"
#ifndef u8
#define u8	uint8_t
#define u16     uint16_t
#define u32	uint32_t 
#define u64	uint64_t

#endif

#endif
