/*
 * (c) 2014 Jason.han <han_zf@neusoft.com>,
 *               
 *     economic rights: neusoft (CHINA)
 *
 * This file is part of N:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "fp_server.h"
#include "stdio.h"
#include "string.h"
#include <l4/log/log.h>
#if 0
#ifdef __cplusplus
extern "C" {
#endif
extern void init_main();
#ifdef __cplusplus
}
#endif
#endif
int main(void) {
   int ret = 0;
printf("FP TA ====2222== >>>/n");
  
   printf("FP TA ====2222== >>>/n");
 
   printf("FP TA ====2222== >>>/n");
   printf("FP TA ====2222== >>>/n");
 
#ifdef BUILD_TIMESTAMP
   LOG_INF("FP TA build at %s", BUILD_TIMESTAMP);
#endif
#ifdef BUILD_GIT_COMMIT
   LOG_INF("FP TA commit number %s", BUILD_GIT_COMMIT);
#endif
   printf("FP TA ====2222== >>>/n");
  
   printf("FP TA ====2222== >>>/n");
 
   printf("FP TA ====2222== >>>/n");
   printf("FP TA ====2222== >>>/n");
 
   ret = fp_main();
 
	// init_main();
  return ret;
}






