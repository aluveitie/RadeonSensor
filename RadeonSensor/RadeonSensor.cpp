//
//  RadeonSensor.cpp
//  RadeonSensor
//
//  Created by Aluveitie on 24.09.21.
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
    
    return true;
}

void RadeonSensor::free() {
    IOLog("%s[%p]::%s()\n", getName(), this, __FUNCTION__);
        
    super::free();
}

IOService* RadeonSensor::probe(IOService* provider, SInt32* score) {
    if (super::probe(provider, score) != this) {
        return 0;
    }
    
    bool ret = false;
    if (OSDictionary * dictionary = serviceMatching("IOPCIDevice")) {
        if (OSIterator * iterator = getMatchingServices(dictionary)) {
            IOPCIDevice* device = NULL;
            UInt32 vendor_id = 0;
            UInt32 class_id = 0;
            do {
                device = OSDynamicCast(IOPCIDevice, iterator->getNextObject());
                if (!device) {
                    break;
                }
                vendor_id = 0;
                OSData *data = OSDynamicCast(OSData, device->getProperty("vendor-id"));
                if (data) {
                    vendor_id = *(UInt32*)data->getBytesNoCopy();
                } else {
                    data = OSDynamicCast(OSData, device->getProperty("ATY,VendorID"));
                    if (data) {
                      vendor_id = *(UInt32*)data->getBytesNoCopy();
                    }
                }

                device_id = 0;
                data = OSDynamicCast(OSData, device->getProperty("device-id"));
                if (data) {
                    device_id = *(UInt32*)data->getBytesNoCopy();
                }

                class_id = 0;
                data = OSDynamicCast(OSData, device->getProperty("class-code"));
                if (data) {
                    class_id = *(UInt32*)data->getBytesNoCopy();
                }

                if ((vendor_id==0x1002) && (class_id == 0x030000)) {
                    InfoLog("found Radeon chip id=%x ", (unsigned int)device_id);
                    pciCard = device;
                    ret = true; //TODO - count a number of cards
                    break;
              }
              /*else {
               WarningLog("ATI Radeon not found!");
               }*/
            } while (device);
        }
    }
    
    if (ret) {
        return this;
    } else {
        return 0;
    }
}

bool RadeonSensor::start(IOService *provider) {
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    if(!super::start(provider)){
        IOLog("%s[%p]::failed to start\n", getName(), this);
        return false;
    }
    
    registerService();
    
    atiCard = new ATICard();
    atiCard->VCard = pciCard;
    atiCard->chipID = device_id;
    
    if (atiCard->initialize()) {
        IOLog("%s[%p]::initialized card %us", getName(), this, atiCard->chipID);
        return true;
    } else {
        IOLog ("[Warning] %s[%p]::could not initialize card", getName(), this);
        return false;
    }
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
