//
//  SMCRadeonGPU.hpp
//  SMCRadeonGPU
//
//  Created by Aluveitie on 08.10.21.
//

#ifndef SMCRadeonGPU_hpp
#define SMCRadeonGPU_hpp

#undef EFIAPI

#include <IOKit/IOLib.h>

#include <RadeonSensor.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
//#include <VirtualSMCSDK/AppleSmc.h>
#define SMC_MAKE_IDENTIFIER(A, B, C, D) ((UINT32)(((UINT32)(D) << 24U) | ((UINT32)(C) << 16U) | ((UINT32)(B) << 8U) | (UINT32)(A)))
#define SMC_MAKE_KEY_TYPE(A, B, C, D) SMC_MAKE_IDENTIFIER ((A), (B), (C), (D))

class EXPORT SMCRadeonGPU : public IOService {
    OSDeclareDefaultStructors(SMCRadeonGPU)
    
    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEF") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEF";
    
    static constexpr SMC_KEY KeyTGxP(size_t i) { return SMC_MAKE_IDENTIFIER('T','G',KeyIndexes[i],'P'); }
    static constexpr SMC_KEY KeyTGxD(size_t i) { return SMC_MAKE_IDENTIFIER('T','G',KeyIndexes[i],'D'); }
    static constexpr SMC_KEY KeyTGxp(size_t i) { return SMC_MAKE_IDENTIFIER('T','G',KeyIndexes[i],'p'); }
    static constexpr SMC_KEY KeyTGxd(size_t i) { return SMC_MAKE_IDENTIFIER('T','G',KeyIndexes[i],'d'); }
    static constexpr SMC_KEY KeyTGDD = SMC_MAKE_IDENTIFIER('T','G','D','D');
    
    
    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };

public:
   
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual IOService *probe(IOService *provider, SInt32 *score) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn setPowerState(unsigned long state, IOService *whatDevice) override;
    
    static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
private:
    RadeonSensor* fProvider;
    
    IONotifier* vsmcNotifier { nullptr };
};

#endif /* SMCRadeonGPU_hpp */
