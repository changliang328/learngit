/**
 * This file is part of N:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <ut_pf_fp.h>
#include <ut_pf_fp_ta.h>
#include <ut_sys_util.h>

void ut_pf_fp_reset(void);
void ut_pf_fp_init(void);

void ut_pf_fp_init(void)
{

}
void ut_pf_fp_reset(void)
{
}

int main(void) {
   ut_int32_t ret = 0;
   
   ut_sys_log("----- fp_ta  main start  2017080301 ----- \n");
   
#ifdef BUILD_TIMESTAMP
   ut_sys_log("FP TA build at %s", BUILD_TIMESTAMP);
#endif
#ifdef BUILD_COMMIT
   ut_sys_log("FP TA commit number %s", BUILD_COMMIT);
#endif
#ifdef FP_SENSOR_TYPE
   ut_sys_log("FP sensor library is %s.", FP_SENSOR_TYPE);
#else
   ut_sys_log("FP sensor type unknown.");
#endif
   ret = ut_pf_fp_main();
   
   return ret;
}
