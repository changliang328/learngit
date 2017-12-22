#define LOG_TAG "btl_fingerprintd"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "BtlFingerprintService.h"


//int main(int argc, char** argv)
int main(void)
{
	ALOGI(" -------- ============== Starting  BtlFingerprintService " );
    android::sp<android::IServiceManager> serviceManager = android::defaultServiceManager();
	android::sp<android::BtlFingerprintService> proxy =
            android::BtlFingerprintService::getInstance();
    android::status_t ret = serviceManager->addService(
            android::BtlFingerprintService::descriptor, proxy);
    if (ret != android::OK) {
        ALOGE("Couldn't register BtlFingerprintService binder service!");
        return -1;
    }
    /*
     * We're the only thread in existence, so we're just going to process
     * Binder transaction as a single-threaded program.
     */
    android::IPCThreadState::self()->joinThreadPool();
    ALOGI("Done");
	return 0;
}
