/*
 * Copyright (C) 2015 Fingerprint Cards AB
 * All rights reserved.
 */

#ifndef INCLUSION_GUARD_FP_TYPES
#define INCLUSION_GUARD_FP_TYPES

typedef enum {
  FP_TA_OK,
  FP_TA_ERROR_GENERAL,
  FP_TA_ERROR_MEMORY,
  FP_TA_ERROR_PARAMETER,
  FP_TA_ERROR_IO,
  FP_TA_ERROR_IO_NO_FILE,
} FP_ta_return_t;

#endif // INCLUSION_GUARD_FP_TYPES
