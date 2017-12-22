#ifndef IBtlFingerprintService_H  
#define IBtlFingerprintService_H 

#include <binder/IInterface.h>
#include "IDaemonCallback.h"
#include "IProcessBridge.h"

namespace android {

	class IBtlFingerprintService : public IInterface{
		public:
			DECLARE_META_INTERFACE(BtlFingerprintService);

			//0:SCREEN_ON,1:SCREEN_OFF
			virtual void screenStateChanged(uint32_t screenState) = 0;
			
			//0:default,1:cit,2:debug
			virtual void setFingerprintMode(uint32_t mode) = 0;
			
			virtual void setDaemonCallback(const sp<IDaemonCallback>& callback) = 0;
			
			virtual void captureImage(uint32_t type) = 0;
			
			virtual void onCaptureImage(const uint8_t* buf,ssize_t bufSize,uint32_t type) = 0;
			
			virtual int32_t testSPI() = 0;
			
			virtual int32_t testDeadPixel() = 0;

			virtual void testPress() = 0;
			
			virtual void onPressCallback(uint32_t state) = 0;
			
			virtual void testImageQuality() = 0;
			
			virtual void onImageQualityCallback(uint32_t quality) = 0;
			
			virtual void testEnroll() = 0;
			
			virtual void onEnrollCallback(uint32_t fingerId,uint32_t remaining) = 0;
			
			virtual void testAuth() = 0;
			
			virtual void onAuthCallback(uint32_t fingerId) = 0;
			
			virtual uint8_t readSFR(uint16_t addr) = 0;
	
			virtual int32_t writeSFR(uint16_t addr, uint8_t value) = 0;
	
			virtual unsigned short readSRAM(uint16_t addr) = 0;
	
			virtual int32_t writeSRAM(uint16_t addr, unsigned short value) = 0;
			
			virtual int32_t getICVersion() = 0;
	
			virtual void changeSensorMode(uint32_t mode) = 0;
	
			virtual void resetSensor() = 0;
			
			virtual void downloadConfig() = 0;
			
			virtual void changeGPIO(uint32_t value) = 0;
			
			virtual int32_t getStateMachine() = 0;
			
			virtual void setProcessBridge(const sp<IProcessBridge>& callback) = 0;

		};

	class BnBtlFingerprintService : public BnInterface<IBtlFingerprintService> {
		public:
			virtual status_t    onTransact( uint32_t code,
											const Parcel& data,
											Parcel* reply,
											uint32_t flags = 0);
		};

}
#endif //IBtlFingerprintService_H
