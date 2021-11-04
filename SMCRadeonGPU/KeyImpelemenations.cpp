//
//  KeyImpelemenations.cpp
//  SMCRadeonGPU
//
//  Created by Nicolas Vollmar on 08.10.21.
//

#include "KeyImpelemenations.hpp"

#include <os/log.h>

SMC_RESULT GPUTempProvider::readAccess() {
    UInt16 temp = provider->getTemperature(index);
    
    UInt16 *ptr = reinterpret_cast<UInt16 *>(data);
    *ptr = VirtualSMCAPI::encodeIntSp(SmcKeyTypeSp78, temp);
    
    return SmcSuccess;
}
