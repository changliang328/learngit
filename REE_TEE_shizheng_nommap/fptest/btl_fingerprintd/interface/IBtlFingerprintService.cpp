#define LOG_TAG "btl_fingerprintd"

#include "IBtlFingerprintService.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <cutils/log.h>
#include <sys/types.h>  
  
#include <binder/IPCThreadState.h>  
#include <binder/IServiceManager.h> 


namespace android {

enum {
    SCREEN_STATE_CHANGED = IBinder::FIRST_CALL_TRANSACTION+0,
	SET_FINGERPRINT_MODE = IBinder::FIRST_CALL_TRANSACTION+1,
	SET_DAEMON_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+2,
	CAPTURE_IMAGE = IBinder::FIRST_CALL_TRANSACTION+3,
	ON_CAPTURE_IMAGE = IBinder::FIRST_CALL_TRANSACTION+4,
	TEST_SPI = IBinder::FIRST_CALL_TRANSACTION+5,
	TEST_DEAD_PIXEL = IBinder::FIRST_CALL_TRANSACTION+6,
	TEST_PRESS = IBinder::FIRST_CALL_TRANSACTION+7,
	ON_PRESS_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+8,
	TEST_IMAGE_QUALITY = IBinder::FIRST_CALL_TRANSACTION+9,
	ON_IMAGE_QUALITY_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+10,
	TEST_ENROLL = IBinder::FIRST_CALL_TRANSACTION+11,
	ON_ENROLL_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+12,
	TEST_AUTH = IBinder::FIRST_CALL_TRANSACTION+13,
	ON_AUTH_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+14,
	READ_SFR = IBinder::FIRST_CALL_TRANSACTION+15,
	WRITE_SFR = IBinder::FIRST_CALL_TRANSACTION+16,
	READ_SRAM = IBinder::FIRST_CALL_TRANSACTION+17,
	WRITE_SRAM = IBinder::FIRST_CALL_TRANSACTION+18,
	GET_IC_VERSION = IBinder::FIRST_CALL_TRANSACTION+19,
	CHANGE_SENSOR_MODE = IBinder::FIRST_CALL_TRANSACTION+20,
	RESET_SENSOR = IBinder::FIRST_CALL_TRANSACTION+24,
	DOWNLOAD_CONFIG = IBinder::FIRST_CALL_TRANSACTION+25,
	CHANGE_GPIO = IBinder::FIRST_CALL_TRANSACTION+26,
	GET_STATE_MACHINE = IBinder::FIRST_CALL_TRANSACTION+27,
	SET_PROCESS_BRIDGE_CALLBACK = IBinder::FIRST_CALL_TRANSACTION+28,
};

//------------------------------------proxy side--------------------------------

class BpBtlFingerprintService : public BpInterface<IBtlFingerprintService> {
	public:
		BpBtlFingerprintService(const sp<IBinder>& impl)
			: BpInterface<IBtlFingerprintService>(impl) {
		}
		virtual void screenStateChanged(uint32_t screenState) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(screenState);
			remote()->transact(SCREEN_STATE_CHANGED,data,&reply, IBinder::FLAG_ONEWAY);
		}
	
		virtual void setFingerprintMode(uint32_t mode) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(mode);
			remote()->transact(SET_FINGERPRINT_MODE,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void setDaemonCallback(const sp<IDaemonCallback>& callback) {
		  Parcel data, reply;
		  data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
		  //Adnroid 5.0
		  //data.writeStrongBinder(callback->asBinder());// TODO: important
		  //Android 6.0
		  data.writeStrongBinder(IInterface::asBinder(callback));
		  remote()->transact(SET_DAEMON_CALLBACK, data, &reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void captureImage(uint32_t type) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(type);
			remote()->transact(CAPTURE_IMAGE,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void onCaptureImage(const uint8_t* buf,ssize_t bufSize,uint32_t type) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeByteArray(bufSize,buf);
			data.writeInt32(type);
			remote()->transact(ON_CAPTURE_IMAGE,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual int32_t testSPI() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			return remote()->transact(TEST_SPI,data,&reply,0);
		}
		
		virtual int32_t testDeadPixel() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			return remote()->transact(TEST_DEAD_PIXEL,data,&reply,0);
		}
		
		virtual void testPress() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(TEST_DEAD_PIXEL,data,&reply,IBinder::FLAG_ONEWAY);
		}
		
		virtual void onPressCallback(uint32_t code) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(code);
			remote()->transact(ON_PRESS_CALLBACK,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void testImageQuality() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(TEST_IMAGE_QUALITY,data,&reply,IBinder::FLAG_ONEWAY);
		}
		
		virtual void onImageQualityCallback(uint32_t quality) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(quality);
			remote()->transact(ON_IMAGE_QUALITY_CALLBACK,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void testEnroll() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(TEST_ENROLL,data,&reply,IBinder::FLAG_ONEWAY);
		}
		
		virtual void onEnrollCallback(uint32_t fingerId,uint32_t remaining) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(fingerId);
			data.writeInt32(remaining);
			remote()->transact(ON_ENROLL_CALLBACK,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void testAuth() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(TEST_AUTH,data,&reply,IBinder::FLAG_ONEWAY);
		}
		
		virtual void onAuthCallback(uint32_t fingerId) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(fingerId);
			remote()->transact(ON_AUTH_CALLBACK,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual  uint8_t readSFR(uint16_t addr) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(addr);
			return remote()->transact(READ_SFR,data,&reply,0);
		}
		
		virtual  int32_t writeSFR(uint16_t addr, uint8_t value) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(addr);
			data.writeInt32(value);
			return remote()->transact(WRITE_SFR,data,&reply,0);
		}
		
		virtual  unsigned short readSRAM(uint16_t addr) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(addr);
			return remote()->transact(READ_SRAM,data,&reply,0);
		}
		
		virtual int32_t writeSRAM(uint16_t addr, unsigned short value) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(addr);
			data.writeInt32(value);
			return remote()->transact(WRITE_SRAM,data,&reply,0);
		}
		
		virtual int32_t getICVersion() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			return remote()->transact(GET_IC_VERSION,data,&reply,0);
		}
		
		virtual void changeSensorMode(uint32_t mode) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(mode);
			remote()->transact(CHANGE_SENSOR_MODE,data,&reply,IBinder::FLAG_ONEWAY);
		}
		
		virtual void resetSensor() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(RESET_SENSOR,data,&reply);
		}
		
		virtual void downloadConfig() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			remote()->transact(DOWNLOAD_CONFIG,data,&reply, IBinder::FLAG_ONEWAY);
		}
		
		virtual void changeGPIO(uint32_t value) {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			data.writeInt32(value);
			remote()->transact(CHANGE_GPIO,data,&reply);
		}
		
		virtual int32_t getStateMachine() {
			Parcel data,reply;
			data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
			return remote()->transact(GET_STATE_MACHINE,data,&reply,0);
		}
		
		virtual void setProcessBridge(const sp<IProcessBridge>& callback) {
		  Parcel data, reply;
		  data.writeInterfaceToken(IBtlFingerprintService::getInterfaceDescriptor());
		  //Adnroid 5.0
		  //data.writeStrongBinder(callback->asBinder());// TODO: important
		  //Android 6.0
		  data.writeStrongBinder(IInterface::asBinder(callback));
		  remote()->transact(SET_PROCESS_BRIDGE_CALLBACK, data, &reply, IBinder::FLAG_ONEWAY);
		}
		
};

IMPLEMENT_META_INTERFACE(BtlFingerprintService, "btl.fingerprint.BtlFingerprintService");

//------------------------------------server side--------------------------------
status_t BnBtlFingerprintService::onTransact (
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags){
    switch (code) {
		case SCREEN_STATE_CHANGED: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
            screenStateChanged(data.readInt32());
            reply->writeNoException();
            return NO_ERROR;
        } break;
        case SET_FINGERPRINT_MODE: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
            setFingerprintMode(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case SET_DAEMON_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			sp<IDaemonCallback> callback = interface_cast<IDaemonCallback>(data.readStrongBinder());// TODO: important!
            setDaemonCallback(callback);
            reply->writeNoException();
            return NO_ERROR; 
        }
		case CAPTURE_IMAGE: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			captureImage(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case ON_CAPTURE_IMAGE: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const ssize_t bufSize = data.readInt32();
			const void* pacData = data.readInplace(bufSize);
            uint8_t* val = (unsigned char*)malloc(bufSize);
			memcpy(val, pacData, bufSize);
			const uint8_t* buf = val;
			onCaptureImage(buf,bufSize,data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case TEST_SPI: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const int32_t ret = testSPI();
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case TEST_DEAD_PIXEL: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const int32_t ret = testDeadPixel();
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case TEST_PRESS: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			testPress();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case ON_PRESS_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			onPressCallback(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case TEST_IMAGE_QUALITY: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			testImageQuality();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case ON_IMAGE_QUALITY_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			onImageQualityCallback(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case TEST_ENROLL: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			testEnroll();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case ON_ENROLL_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			onEnrollCallback(data.readInt32(),data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case TEST_AUTH: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			testAuth();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case ON_AUTH_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			onAuthCallback(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case READ_SFR: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const uint16_t addr = data.readInt32();
			const uint8_t ret = readSFR(addr);
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case WRITE_SFR: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const uint16_t addr = data.readInt32();
			const uint8_t value = data.readInt32();
			const int32_t ret = writeSFR(addr,value);
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case READ_SRAM: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const uint16_t addr = data.readInt32();
			const unsigned short ret = readSRAM(addr);
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case WRITE_SRAM: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const uint16_t addr = data.readInt32();
			const unsigned short value = data.readInt32();
			const int32_t ret = writeSRAM(addr,value);
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case GET_IC_VERSION: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const int32_t ret = getICVersion();
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case CHANGE_SENSOR_MODE: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			changeSensorMode(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case RESET_SENSOR: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			resetSensor();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case DOWNLOAD_CONFIG: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			downloadConfig();
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case CHANGE_GPIO: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			changeGPIO(data.readInt32());
			reply->writeNoException();
            return NO_ERROR;
        } break;
		case GET_STATE_MACHINE: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			const int32_t ret = getStateMachine();
			reply->writeNoException();
			reply->writeInt32(ret);
            return NO_ERROR;
        } break;
		case SET_PROCESS_BRIDGE_CALLBACK: {
			data.enforceInterface(IBtlFingerprintService::getInterfaceDescriptor(),NULL);
			sp<IProcessBridge> callback = interface_cast<IProcessBridge>(data.readStrongBinder());// TODO: important!
            setProcessBridge(callback);
            reply->writeNoException();
            return NO_ERROR; 
        }
    }

    return BBinder::onTransact(code, data, reply, flags);
};

};
