//
//  RadeonSensor.cpp
//  RadeonSensor
//
//  Created by Altoo on 24.09.21.
//
#include <IOKit/IOLib.h>
#include "RadeonSensor.hpp"

#define super IOService

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

OSDefineMetaClassAndStructors(RadeonSensor, IOService);

bool RadeonSensor::init(OSDictionary *dictionary){
    if (!super::init(dictionary)) {
        return false;
    }
    
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, dictionary);
    
    //atiCard = new ATICard();
    
    return true;
}

void RadeonSensor::free() {
    IOLog("%s[%p]::%s()\n", getName(), this, __FUNCTION__);
        
    super::free();
}

bool RadeonSensor::start(IOService *provider) {
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    if(!super::start(provider)){
        IOLog("%s[%p]::failed to start\n", getName(), this);
        return false;
    }
    
    registerService();
    
    //atiCard->initialize();
    IOLog("Found Radeon: %us", atiCard->chipID);
    
    return true;
}

void RadeonSensor::stop(IOService *provider) {
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    super::stop(provider);
}

uint16_t RadeonSensor::getTemperature() {
    UInt16 data;
    if (atiCard == NULL) {
        IOLog("Not initialized");
        return 0;
    }
        
    switch (atiCard->tempFamily) {
        case R6xx:
            atiCard->R6xxTemperatureSensor(&data);
            return data;
        case R7xx:
            atiCard->R7xxTemperatureSensor(&data);
            return data;
        case R8xx:
            atiCard->EverTemperatureSensor(&data);
            return data;
        case R9xx:
            atiCard->TahitiTemperatureSensor(&data);
            return data;
        case RCIx:
            atiCard->TahitiTemperatureSensor(&data);
            return data;
        case RAIx:
            atiCard->ArcticTemperatureSensor(&data);
            return data;
        case RVEx:
            atiCard->VegaTemperatureSensor(&data);
            return data;
        default:
            return 0;
    }
}

EXPORT extern "C" kern_return_t radeonsensor_start(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-radpdbg");
       
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t radeonsensor_stop(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}
