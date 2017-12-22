#define LOG_TAG "btl_fingerprintd"
#include "IBtlFingerprintService.h"
#include <binder/Parcel.h>
#include <binder/IInterface.h>
#include <cutils/log.h>


namespace android {

	//////////////////client
	class BpDaemonCallback : public BpInterface<IDaemonCallback> {
		public:
			BpDaemonCallback(const sp<IBinder>& impl) : BpInterface<IDaemonCallback>(impl) {
			}
			
			virtual status_t onCaptureImage(const uint8_t* buf,ssize_t bufSize,uint32_t original) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeByteArray(bufSize,buf);
				data.writeInt32(original);
				return remote()->transact(ON_CAPTURE_IMAGE,data,&reply);
			}

			virtual status_t onPressCallback(uint32_t code) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeInt32(code);
				return remote()->transact(ON_PRESS_CALLBACK,data,&reply);
			}
			
			virtual status_t onImageQualityCallback(uint32_t quality) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeInt32(quality);
				return remote()->transact(ON_IMAGE_QUALITY_CALLBACK,data,&reply);
			}
			
			virtual status_t onEnrollCallback(uint32_t code,uint32_t remaining) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeInt32(code);
				data.writeInt32(remaining);
				return remote()->transact(ON_ENROLL_CALLBACK,data,&reply);
			}
			
			virtual status_t onAuthCallback(uint32_t code) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeInt32(code);
				return remote()->transact(ON_AUTH_CALLBACK,data,&reply);
			}
			
			virtual status_t onWriteWhiteCallback(uint32_t code) {
				Parcel data,reply;
				data.writeInterfaceToken(IDaemonCallback::getInterfaceDescriptor());
				data.writeInt32(code);
				return remote()->transact(ON_WRITE_WHITE_CALLBACK,data,&reply);
			}
			
	};

	IMPLEMENT_META_INTERFACE(DaemonCallback, "btl.fingerprint.IDaemonCallback");

};
