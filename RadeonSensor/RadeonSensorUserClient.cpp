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
            //Get temperature
            case 1: {
                arguments->scalarOutputCount = 0;
                arguments->structureOutputSize = 1 * sizeof(uint64_t);
                uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
                
                uint16_t temp = mProvider->getTemperature();
                dataOut[0] = (uint64_t)temp;
            }
    }
    
    return kIOReturnSuccess;
}
