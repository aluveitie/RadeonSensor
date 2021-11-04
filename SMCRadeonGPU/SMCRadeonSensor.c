//
//  SMCRadeonSensor.c
//  SMCRadeonSensor
//
//  Created by Aluveitie on 08.10.21.
//

#include <IOKit/IOLib.h>

#include <RadeonSensor.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

class SMCRadeonSensor : public IOService {
    OSDeclareDefaultStructors(SMCRadeonSensor)
    
    /**
     *  VirtualSMC service registration notifier
     */
    IONotifier *vsmcNotifier {nullptr};
    
    static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
    /**
     *  Registered plugin instance
     */
    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };
    /**
     *  Key name index mapping
     */
    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    
private:
    bool setupKeysVsmc();
    RadeonSensor *fProvider;
};
