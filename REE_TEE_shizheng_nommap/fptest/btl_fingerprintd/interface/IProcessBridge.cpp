#include "IProcessBridge.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>

namespace android {

	enum {
		ON_FINGERPRINT_SCREEN_STATE = IBinder::FIRST_CALL_TRANSACTION+0,
		ON_FINGERPRINT_MODE = IBinder::FIRST_CALL_TRANSACTION+1,
		GET_CAPTURE_IMAGE = IBinder::FIRST_CALL_TRANSACTION+2,
		TEST_SPI = IBinder::FIRST_CALL_TRANSACTION+3,
		TEST_DEAD_PIXEL = IBinder::FIRST_CALL_TRANSACTION+4,
		TEST_PRESS = IBinder::FIRST_CALL_TRANSACTION+5,
		TEST_IMAGE_QUALITY = IBinder::FIRST_CALL_TRANSACTION+6,
		TEST_ENROLL = IBinder::FIRST_CALL_TRANSACTION+7,
		TEST_AUTH = IBinder::FIRST_CALL_TRANSACTION+8,
		READ_SFR = IBinder::FIRST_CALL_TRANSACTION+9,
		WRITE_SFR = IBinder::FIRST_CALL_TRANSACTION+10,
		READ_SRAM = IBinder::FIRST_CALL_TRANSACTION+11,
		WRITE_SRAM = IBinder::FIRST_CALL_TRANSACTION+12,
		GET_IC_VERSION = IBinder::FIRST_CALL_TRANSACTION+13,
		CHANGE_SENSOR_MODE = IBinder::FIRST_CALL_TRANSACTION+14,
		RESET_SENSOR = IBinder::FIRST_CALL_TRANSACTION+17,
		DOWNLOAD_CONFIG = IBinder::FIRST_CALL_TRANSACTION+18,
		CHANGE_GPIO = IBinder::FIRST_CALL_TRANSACTION+19,
		GET_STATE_MACHINE = IBinder::FIRST_CALL_TRANSACTION+20,
	};

	//////////////////client
	class BpProcessBridge : public BpInterface<IProcessBridge> {
		public:
			BpProcessBridge(const sp<IBinder>& impl) : BpInterface<IProcessBridge>(impl) {
			}

			virtual void onScreenStateChanged(uint32_t screenState) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(screenState);
				remote()->transact(ON_FINGERPRINT_SCREEN_STATE,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void onFingerprintModeChanged(uint32_t mode) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(mode);
				remote()->transact(ON_FINGERPRINT_MODE,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void getCaptureImage(uint32_t type) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(type);
				remote()->transact(GET_CAPTURE_IMAGE,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual int32_t testSPI() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_SPI,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual int32_t testDeadPixel() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_DEAD_PIXEL,data,&reply,0);
				const int32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual void testPress() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_PRESS,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void testImageQuality() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_IMAGE_QUALITY,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void testEnroll() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_ENROLL,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void testAuth() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(TEST_AUTH,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual uint8_t readSFR(uint16_t addr) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(addr);
				remote()->transact(READ_SFR,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual int32_t writeSFR(uint16_t addr, uint8_t value) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(addr);
				data.writeInt32(value);
				remote()->transact(WRITE_SFR,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual unsigned short readSRAM(uint16_t addr) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(addr);
				remote()->transact(READ_SRAM,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual int32_t writeSRAM(uint16_t addr, unsigned short value) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(addr);
				data.writeInt32(value);
				remote()->transact(WRITE_SRAM,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual int32_t getICVersion() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(GET_IC_VERSION,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
			virtual void changeSensorMode(uint32_t mode) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(mode);
				remote()->transact(CHANGE_SENSOR_MODE,data,&reply,IBinder::FLAG_ONEWAY);
			}
		
			
			virtual void resetSensor() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(RESET_SENSOR,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void downloadConfig() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(DOWNLOAD_CONFIG,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual void changeGPIO(uint32_t value) {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				data.writeInt32(value);
				remote()->transact(CHANGE_GPIO,data,&reply,IBinder::FLAG_ONEWAY);
			}
			
			virtual int32_t getStateMachine() {
				Parcel data,reply;
				data.writeInterfaceToken(IProcessBridge::getInterfaceDescriptor());
				remote()->transact(GET_STATE_MACHINE,data,&reply,0);
				const uint32_t ret = reply.readInt32();
				return ret;
			}
			
	};

	IMPLEMENT_META_INTERFACE(ProcessBridge, "btl.fingerprint.IProcessBridge");

	////////////////server
	status_t BnProcessBridge::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) 
	{
		switch (code) 
		{
			case ON_FINGERPRINT_SCREEN_STATE: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				onScreenStateChanged(data.readInt32());
				return NO_ERROR;
			} 
			case ON_FINGERPRINT_MODE: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				onFingerprintModeChanged(data.readInt32());
				return NO_ERROR;
			} 
			case GET_CAPTURE_IMAGE: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				getCaptureImage(data.readInt32());
				return NO_ERROR;
			} 
			case TEST_SPI: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const int32_t ret = testSPI();
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case TEST_DEAD_PIXEL: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const int32_t ret = testDeadPixel();
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case TEST_PRESS: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				testPress();
				return NO_ERROR;
			} 
			case TEST_IMAGE_QUALITY: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				testImageQuality();
				return NO_ERROR;
			} break;
			case TEST_ENROLL: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				testEnroll();
				return NO_ERROR;
			} 
			case TEST_AUTH: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				testAuth();
				return NO_ERROR;
			} 
			case READ_SFR: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const uint16_t addr = data.readInt32();
				const uint8_t ret = readSFR(addr);
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case WRITE_SFR: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const uint16_t addr = data.readInt32();
				const uint8_t value = data.readInt32();
				const int32_t ret = writeSFR(addr,value);
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case READ_SRAM: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const uint16_t addr = data.readInt32();
				const unsigned short ret = readSRAM(addr);
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case WRITE_SRAM: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const uint16_t addr = data.readInt32();
				const unsigned short value = data.readInt32();
				const int32_t ret = writeSRAM(addr,value);
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case GET_IC_VERSION: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const int32_t ret = getICVersion();
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			case CHANGE_SENSOR_MODE: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				changeSensorMode(data.readInt32());
				return NO_ERROR;
			} break;
			case RESET_SENSOR: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				resetSensor();
				return NO_ERROR;
			} break;
			case DOWNLOAD_CONFIG: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				downloadConfig();
				return NO_ERROR;
			} break;
			case CHANGE_GPIO: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				changeGPIO(data.readInt32());
				return NO_ERROR;
			} break;
			case GET_STATE_MACHINE: {
				data.enforceInterface(IProcessBridge::getInterfaceDescriptor(),NULL);
				const int32_t ret = getStateMachine();
				reply->writeInt32(ret);
				return NO_ERROR;
			} break;
			default:
				return BBinder::onTransact(code, data, reply, flags);
		}
	}
}
