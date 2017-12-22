#ifndef IPROCESS_BRIDGE_H
#define IPROCESS_BRIDGE_H
#include <binder/IInterface.h>
#include <utils/RefBase.h>
#include <binder/Parcel.h>  

namespace android {

	class IProcessBridge : public IInterface {
		public:
		
			DECLARE_META_INTERFACE(ProcessBridge);
			
			virtual void onScreenStateChanged(uint32_t screenState) = 0;
			
			virtual void onFingerprintModeChanged(uint32_t mode) = 0;
			
			virtual void getCaptureImage(uint32_t type) = 0;
			
			virtual int32_t testSPI() = 0;
			
			virtual int32_t testDeadPixel() = 0;
			
			virtual void testPress() = 0;
			
			virtual void testImageQuality() = 0;	
			
			virtual void testEnroll() = 0;		
			
			virtual void testAuth() = 0;
			
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
			
		};

	class BnProcessBridge : public BnInterface<IProcessBridge> {
		public:
			virtual status_t    onTransact( uint32_t code,
											const Parcel& data,
											Parcel* reply,
											uint32_t flags = 0);
		};
}
#endif
