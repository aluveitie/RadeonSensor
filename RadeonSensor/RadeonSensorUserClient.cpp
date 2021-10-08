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

    IOLog("%s[%p]::%s(%p, %p, %u, %p)\n", getName(), this, __FUNCTION__, owningTask, securityToken, (unsigned int)type, properties);
    
    mTask = owningTask;
    mProvider = NULL;
    
    return true;
}

bool RadeonSensorUserClient::start(IOService* provider) {
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    mProvider = OSDynamicCast(RadeonSensor, provider);
    bool success = (mProvider != NULL);
    
    if (success)
        success = super::start(provider);
    
    return success;
}


void RadeonSensorUserClient::stop(IOService* provider){
    IOLog("%s[%p]::%s(%p)\n", getName(), this, __FUNCTION__, provider);
    
    mProvider = NULL;
    super::stop(provider);
}

IOReturn RadeonSensorUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) {
    IOLog("%s[%p]::%s(%d, %p, %p, %p, %p)\n", getName(), this, __FUNCTION__, selector, arguments, dispatch, target, reference);
    
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
