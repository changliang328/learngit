#include <binder/BinderService.h>
#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "interface/IBtlFingerprintService.h"
#include "interface/IDaemonCallback.h"
#include "interface/IProcessBridge.h"

namespace android {
class BtlFingerprintService : public BinderService<BtlFingerprintService>, public BnBtlFingerprintService {
	public:
		static BtlFingerprintService* getInstance() {
			if (sInstance == NULL) {
				sInstance = new BtlFingerprintService();
			}
			return sInstance;
		}
		
		//0:SCREEN_ON,1:SCREEN_OFF
		virtual void screenStateChanged(uint32_t screenState);
			
		//0:default,1:cit,2:debug
		virtual void setFingerprintMode(uint32_t mode);
		
		virtual void setDaemonCallback(const sp<IDaemonCallback>& callback);
		
		virtual void captureImage(uint32_t type);
			
		virtual void onCaptureImage(const uint8_t* buf,ssize_t bufSize,uint32_t type);
		
		virtual int32_t testSPI();
		
		virtual int32_t testDeadPixel();
		
		virtual void testPress();
			
		virtual void onPressCallback(uint32_t code);
		
		virtual void testImageQuality();
		
		virtual void onImageQualityCallback(uint32_t quality);
			
		virtual void testEnroll();
			
		virtual void onEnrollCallback(uint32_t fingerId,uint32_t remaining);
			
		virtual void testAuth();
			
		virtual void onAuthCallback(uint32_t fingerId);
		
		virtual uint8_t readSFR(uint16_t addr);
		
		virtual int32_t writeSFR(uint16_t addr, uint8_t value);
		
		virtual unsigned short readSRAM(uint16_t addr);
		
		virtual int32_t writeSRAM(uint16_t addr, unsigned short value);
		
		virtual int32_t getICVersion();
	
		virtual void changeSensorMode(uint32_t mode);
	
		virtual void resetSensor();
			
		virtual void downloadConfig();
			
		virtual void changeGPIO(uint32_t value);
			
		virtual int32_t getStateMachine();
		
		virtual void setProcessBridge(const sp<IProcessBridge>& callback);

		virtual const android::String16& getInterfaceDescriptor() const;
		
		static const android::String16 descriptor;
		
		~BtlFingerprintService();
		
	protected:
	
		sp<IDaemonCallback> mDaemonCallback;
		
		sp<IProcessBridge> mProcessBridge;
	private:
		static BtlFingerprintService* sInstance;
		
		BtlFingerprintService();
	};
}
