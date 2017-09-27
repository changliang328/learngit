#ifndef _BTL_CUSTOM_H_
#define _BTL_CUSTOM_H_
/*
#define BTL_FP_BF3290 1
#define BTL_FP_BF3182 2
#define BTL_FP_BF3390 3

#define F_CHIP_TYPE BTL_FP_BF3390

#if	(F_CHIP_TYPE == BTL_FP_BF3182)
	#define FINGER_WIDTH	(72)
	#define FINGER_HEIGHT	(128)
	#define AREA_MAX		(23)
	#define BEST_ALGO		(24)
#elif (F_CHIP_TYPE == BTL_FP_BF3390) 
	#define FINGER_WIDTH	(80)
	#define FINGER_HEIGHT	(80)
	#define AREA_MAX		(15)
	#define BEST_ALGO		(20)
#else
	#define FINGER_WIDTH	(112)
	#define FINGER_HEIGHT	(96)
	#define AREA_MAX		(26)
	#define BEST_ALGO		(28)
#endif*/

//for MT6797 USE
#define USE_SPI1_4GB_TEST (0)

//if save the image when enroll and match failed
//#define SAVE_IMAGE (1)

//if version is ANDROID N
#define BTL_ANDROID_N (1)

#define BL_QUICK_WAKEUP_EN (0)
//save the fingerprint template in file (not database)
//#define  FINGER_DATA_FILE     1

#define USE_MATCH_C	(0)

#define MATCH_BEFORE_ENROLL (0)

#endif
