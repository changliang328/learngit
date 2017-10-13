#ifndef _BTL_CUSTOM_H_
#define _BTL_CUSTOM_H_


#define CHIP_BF3182 (1)

#ifdef CHIP_BF3182
	#define FINGER_WIDTH	(72)
	#define FINGER_HEIGHT	(128)
#else
	#define FINGER_WIDTH	(112)
	#define FINGER_HEIGHT	(96)
#endif

//if save the image when enroll and match failed
#define SAVE_IMAGE (1)

//if version is ANDROID N
//#define BTL_ANDROID_N (1)

//save the fingerprint template in file (not database)
#define  FINGER_DATA_FILE     1

#endif
