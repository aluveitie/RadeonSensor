//
//  RadeonSensorUserClient.cpp
//  RadeonSensor
//
//  Created by Aluveitie on 24.09.21.
//
#include <IOKit/IOLib.h>

#include "RadeonSensorUserClient.hpp"

#define super IOUserClient


OSDefineMetaClassAndStructors(RadeonSensorUserClient, IOUserClient);

bool RadeonSensorUserClient::initWithTask(task_t owningTask, void* securityToken, UInt32 type, OSDictionary* properties) {
    if (!owningTask)
        return false;
    
    if (!super::initWithTask(owningTask, securityToken, type)) {
        return false;
    }

    IOLog("RadeonSensor::UserClient init with task\n");
    
    mTask = owningTask;
    mProvider = NULL;
    
    return true;
}

bool RadeonSensorUserClient::start(IOService* provider) {
    IOLog("RadeonSensor::UserClient starting\n");
    
    mProvider = OSDynamicCast(RadeonSensor, provider);
    bool success = (mProvider != NULL);
    
    if (success)
        success = super::start(provider);
    
    return success;
}


void RadeonSensorUserClient::stop(IOService* provider){
    IOLog("RadeonSensor::UserClient stopping\n");
    
    mProvider = NULL;
    super::stop(provider);
}

IOReturn RadeonSensorUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) {    
    switch (selector) {
        //Get Kext version string
        case 0: {
            char version[] = xStringify(MODULE_VERSION);
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = sizeof(version);
            
            char *dataOut = (char*) arguments->structureOutput;
            for(uint32_t i = 0; i < arguments->structureOutputSize; i++){
                dataOut[i] = version[i];
            }
            
            break;
        }
        //Get number of cards
        case 1: {
            UInt16 numCards = mProvider->getNumberOfCards();
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numCards;
            
            arguments->structureOutputSize = 0;
            
            break;
        }
        //Get temperatures
        case 2: {
            UInt16 numCards = mProvider->getNumberOfCards();
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numCards;
            
            arguments->structureOutputSize = numCards * sizeof(UInt16);
            UInt16 *dataOut = (UInt16*) arguments->structureOutput;
            for (int i = 0; i < numCards; i++) {
                dataOut[i] = mProvider->getTemperature(i);
            }

            break;
        }
    }
    
    return kIOReturnSuccess;
}
