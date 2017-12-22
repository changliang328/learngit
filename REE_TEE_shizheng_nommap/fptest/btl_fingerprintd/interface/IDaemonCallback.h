#ifndef IDAEMONCALLBACK_H
#define IDAEMONCALLBACK_H
#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

	class IDaemonCallback : public IInterface {
		public:
		
			enum {
				
				ON_CAPTURE_IMAGE = IBinder::FIRST_CALL_TRANSACTION+0,
				ON_PRESS_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+1,
				ON_IMAGE_QUALITY_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+2,
				ON_ENROLL_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+3,
				ON_AUTH_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+4,
				ON_WRITE_WHITE_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+5,
				
			};
		
			virtual status_t onCaptureImage(const uint8_t* buf,ssize_t bufSize,uint32_t original) = 0;
			
			virtual status_t onPressCallback(uint32_t code) = 0;
			
			virtual status_t onImageQualityCallback(uint32_t quality) = 0;
			
			virtual status_t onEnrollCallback(uint32_t code,uint32_t remaining) = 0;
			
			virtual status_t onAuthCallback(uint32_t code) = 0;
			
			virtual status_t onWriteWhiteCallback(uint32_t code) = 0;


			DECLARE_META_INTERFACE(DaemonCallback);
	};

}
#endif