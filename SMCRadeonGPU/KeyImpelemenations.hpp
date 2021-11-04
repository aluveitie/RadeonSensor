//
//  KeyImpelemenations.hpp
//  SMCRadeonGPU
//
//  Created by Nicolas Vollmar on 08.10.21.
//

#ifndef KeyImpelemenations_hpp
#define KeyImpelemenations_hpp

#include <libkern/libkern.h>
#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

#include "SMCRadeonGPU.hpp"

class RadeonSMCValue : public VirtualSMCValue {
protected:
    size_t index;
    RadeonSensor* provider;
public:
    RadeonSMCValue(RadeonSensor* provider, size_t index) : index(index), provider(provider) {}
};


class GPUTempProvider : public RadeonSMCValue { using RadeonSMCValue::RadeonSMCValue; protected: SMC_RESULT readAccess() override; };

#endif /* KeyImpelemenations_hpp */
