#define LOG_TAG "btl_fingerprintd"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <cutils/log.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <utils/threads.h>
#include <utils/String16.h>
#include <cutils/properties.h>
#include "BtlFingerprintService.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

namespace android {
	
	BtlFingerprintService* BtlFingerprintService::sInstance = NULL;
	
	const android::String16 BtlFingerprintService::descriptor("btl.fingerprint.BtlFingerprintService");

	const android::String16& BtlFingerprintService::getInterfaceDescriptor() const
	{
		return BtlFingerprintService::descriptor;
	}
	
    BtlFingerprintService::BtlFingerprintService()
	{
		ALOGD(" ---------------- create process");
    }
	
	BtlFingerprintService::~BtlFingerprintService() 
	{
		ALOGD(" ---------------- death process");
	}

	////0:SCREEN_ON,1:SCREEN_OFF
    void BtlFingerprintService::screenStateChanged(uint32_t screenState)
	{
        if(mProcessBridge!=NULL)
		{
			mProcessBridge->onScreenStateChanged(screenState);
		}
    }

	//0:default,1:cit,2:debug
	void BtlFingerprintService::setFingerprintMode(uint32_t mode) 
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->onFingerprintModeChanged(mode);
		}
    }
	
	void BtlFingerprintService::setDaemonCallback(const sp<IDaemonCallback>& callback)
	{
		mDaemonCallback = callback;
    }
	
	void BtlFingerprintService::captureImage(uint32_t type)
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->getCaptureImage(type);
		}
    }
	
	void BtlFingerprintService::onCaptureImage(const uint8_t* buf,const ssize_t bufSize,uint32_t type)
	{
		if(mDaemonCallback!=NULL)
		{
			mDaemonCallback->onCaptureImage(buf,bufSize,type);
		}
    }
	
	int32_t BtlFingerprintService::testSPI()
	{
		if(mProcessBridge!=NULL){
			return mProcessBridge->testSPI();
		}
		return -1;
	}
	
	int32_t BtlFingerprintService::testDeadPixel()
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->testDeadPixel();
		}
		return -1;
	}
	
	void BtlFingerprintService::testPress()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->testPress();
		}
    }
	
	void BtlFingerprintService::onPressCallback(uint32_t code)
	{
		if(mDaemonCallback!=NULL)
		{
			mDaemonCallback->onPressCallback(code);
		}
    }
	
	void BtlFingerprintService::testImageQuality()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->testImageQuality();
		}
    }
	
	void BtlFingerprintService::onImageQualityCallback(uint32_t quality)
	{
		if(mDaemonCallback!=NULL)
		{
			mDaemonCallback->onImageQualityCallback(quality);
		}
    }
	
	void BtlFingerprintService::testEnroll()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->testEnroll();
		}
    }
	
	void BtlFingerprintService::onEnrollCallback(uint32_t code,uint32_t remaining)
	{
		if(mDaemonCallback!=NULL)
		{
			mDaemonCallback->onEnrollCallback(code,remaining);
		}
    }
	
	void BtlFingerprintService::testAuth()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->testAuth();
		}
    }
	
	void BtlFingerprintService::onAuthCallback(uint32_t code)
	{
		if(mDaemonCallback!=NULL)
		{
			mDaemonCallback->onAuthCallback(code);
		}
    }
	
	uint8_t BtlFingerprintService::readSFR(uint16_t addr)
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->readSFR(addr);
		}
		return 0x0;
	}
	
	int32_t BtlFingerprintService::writeSFR(uint16_t addr, uint8_t value)
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->writeSFR(addr,value);
		}
		return -1;
	}
	
	unsigned short BtlFingerprintService::readSRAM(uint16_t addr)
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->readSRAM(addr);
		}
		return 0x0;
	}
	
	int32_t BtlFingerprintService::writeSRAM(uint16_t addr, unsigned short value)
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->writeSRAM(addr,value);
		}
		return -1;
	}
	
	int32_t BtlFingerprintService::getICVersion()
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->getICVersion();
		}
		return -1;
	}
	
	void BtlFingerprintService::changeSensorMode(uint32_t mode)
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->changeSensorMode(mode);
		}
    }
	
	
	void BtlFingerprintService::resetSensor()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->resetSensor();
		}
    }
	
	
	void BtlFingerprintService::downloadConfig()
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->downloadConfig();
		}
    }
	
	void BtlFingerprintService::changeGPIO(uint32_t value)
	{
		if(mProcessBridge!=NULL)
		{
			mProcessBridge->changeGPIO(value);
		}
	}
	
	int32_t BtlFingerprintService::getStateMachine()
	{
		if(mProcessBridge!=NULL)
		{
			return mProcessBridge->getStateMachine();
		}
		return -1;
	}
	
	void BtlFingerprintService::setProcessBridge(const sp<IProcessBridge>& callback)
	{
		mProcessBridge = callback;
	}
}
