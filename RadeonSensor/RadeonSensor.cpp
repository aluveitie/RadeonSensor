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
    
    IOLog("RadeonSensor initialized");
    
    return true;
}

void RadeonSensor::free() {
    IOLog("RadeonSensor freeing up");
    
    delete [] atiCards;
        
    super::free();
}

IOService* RadeonSensor::probe(IOService* provider, SInt32* score) {
    if (super::probe(provider, score) != this) {
        return 0;
    }
    
    int count = 0;
    int maxCount = 4; // maximally support 4 GPUs
    ATICard** cards = new ATICard*[4];
    if (OSDictionary * dictionary = serviceMatching("IOPCIDevice")) {
        if (OSIterator * iterator = getMatchingServices(dictionary)) {
            UInt32 vendor_id = 0;
            UInt32 class_id = 0;
            IOPCIDevice* device = NULL;
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

                int device_id = 0;
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
                    IOLog("RadeonSensor found Radeon PCIe device id=%x", (unsigned int)device_id);
                    ATICard* atiCard = new ATICard();
                    atiCard->VCard = device;
                    atiCard->chipID = device_id;
                    cards[count] = atiCard;
                    
                    count++;
                    if (count == maxCount) {
                        break;
                    }
              }
            } while (device);
        }
    }
    
    if (count > 0) {
        nrOfCards = count;
        atiCards = cards;
        return this;
    } else {
        return 0;
    }
}

bool RadeonSensor::start(IOService *provider) {
    IOLog("RadeonSensor starting\n");
    
    if(!super::start(provider)){
        IOLog("RadeonSensor failed to start\n");
        return false;
    }
    
    registerService();
    
    for (int i = 0; i < nrOfCards; i++) {
        ATICard* atiCard = atiCards[i];
        if (atiCard->initialize()) {
            IOLog("RadeonSensor initialized card (%x)", atiCard->chipID);
        } else {
            IOLog("RadeonSensor could not initialize card (%x)", atiCard->chipID);
            return false;
        }
    }
    
    return true;
}

void RadeonSensor::stop(IOService *provider) {
    IOLog("RadeonSensor stopping\n");
    
    super::stop(provider);
}

void RadeonSensor::getTemperatures(UInt16 data[]) {
    if (nrOfCards == 0) {
        IOLog("RadeonSensor no GPU were found");
        return;
    }
    
    for (int i = 0; i < nrOfCards; i++) {
        UInt16 temp = 0;
        ATICard* atiCard = atiCards[i];
        switch (atiCard->tempFamily) {
            case R6xx:
                atiCard->R6xxTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case R7xx:
                atiCard->R7xxTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case R8xx:
                atiCard->EverTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case R9xx:
                atiCard->TahitiTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case RCIx:
                atiCard->TahitiTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case RAIx:
                atiCard->ArcticTemperatureSensor(&temp);
                data[i] = temp;
                break;
            case RVEx:
                atiCard->VegaTemperatureSensor(&temp);
                data[i] = temp;
                break;
            default:
                data[i] = temp;
                break;
        }
    }
    
    return;
}

UInt16 RadeonSensor::getNumberOfCards() {
    return nrOfCards;
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
