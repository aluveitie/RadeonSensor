//
//  SMCRadeonGPU.cpp
//  SMCRadeonGPU
//
//  Created by Aluveitie on 08.10.21.
//


#include <os/log.h>

#include "SMCRadeonGPU.hpp"

#include "KeyImpelemenations.hpp"

OSDefineMetaClassAndStructors(SMCRadeonGPU, IOService);

enum {
    MySmcKeyTypeSp78 = SMC_MAKE_KEY_TYPE ('s', 'p', '7', '8'),
};

bool SMCRadeonGPU::init(OSDictionary *dictionary) {
    if (!IOService::init(dictionary)) {
        return false;
    }
    
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU initialized");
    
    return true;
}

IOService *SMCRadeonGPU::probe(IOService *provider, SInt32 *score) {
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU probing");
    
    auto ptr = IOService::probe(provider, score);
    if (!ptr) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU failed to probe the parent");
        return nullptr;
    }
    
    fProvider = OSDynamicCast(RadeonSensor, provider);
    if(!fProvider) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU could not get RadeonSensor");
        return nullptr;
    }
    
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU setting up SMC keys");
    auto gpuCount = fProvider->getNumberOfCards();
    bool suc = true;
    for (auto i = 0; i < gpuCount; i++) {
        suc &= VirtualSMCAPI::addKey(KeyTGxD(i), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, MySmcKeyTypeSp78, new GPUTempProvider(fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxP(i), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, MySmcKeyTypeSp78, new GPUTempProvider(fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxd(i), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, MySmcKeyTypeSp78, new GPUTempProvider(fProvider, i)));
        suc &= VirtualSMCAPI::addKey(KeyTGxp(i), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, MySmcKeyTypeSp78, new GPUTempProvider(fProvider, i)));
        if (i == 0) {
            suc &= VirtualSMCAPI::addKey(KeyTGDD, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, MySmcKeyTypeSp78, new GPUTempProvider(fProvider, i)));
        }
    }

    qsort(const_cast<VirtualSMCKeyValue *>(vsmcPlugin.data.data()), vsmcPlugin.data.size(), sizeof(VirtualSMCKeyValue), VirtualSMCKeyValue::compare);
    
    if (!suc) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU setting up SMC keys failed");
    }
    
    return this;
}

void SMCRadeonGPU::free(void){
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU freeing up");
}

bool SMCRadeonGPU::start(IOService *provider) {
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU starting");
    
    if (!IOService::start(provider)) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU failed to start the parent");
        return false;
    }
    
    setProperty("VersionInfo", xStringify(MODULE_VERSION));
        
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    if (!vsmcNotifier) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU failed to register vsmc handler");
        return false;
    }

    return true;
}

bool SMCRadeonGPU::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
    if (sensors && vsmc) {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU got vsmc notification");
        auto &plugin = static_cast<SMCRadeonGPU *>(sensors)->vsmcPlugin;
        auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
        if (ret == kIOReturnSuccess) {
            os_log(OS_LOG_DEFAULT, "SMCRadeonGPU submitted plugin");
            return true;
        } else if (ret != kIOReturnUnsupported) {
            os_log(OS_LOG_DEFAULT, "SMCRadeonGPU plugin submission failure %X", ret);
            return false;
        } else {
            os_log(OS_LOG_DEFAULT, "SMCRadeonGPU plugin submission to non vsmc");
            return false;
        }
    } else {
        os_log(OS_LOG_DEFAULT, "SMCRadeonGPU got null vsmc notification");
        return false;
    }
}

void SMCRadeonGPU::stop(IOService *provider) {
    os_log(OS_LOG_DEFAULT, "SMCRadeonGPU stopping");
}
