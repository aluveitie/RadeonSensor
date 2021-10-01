//
//  RadeonSensorUserClient.hpp
//  RadeonSensor
//
//  Created by Altoo on 24.09.21.
//

#ifndef RadeonSensorUserClient_hpp
#define RadeonSensorUserClient_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

#include "RadeonSensor.hpp"

class RadeonSensorUserClient : public IOUserClient {
    OSDeclareDefaultStructors(RadeonSensorUserClient)
    
private:
    RadeonSensor* mProvider;
    task_t mTask;
    
public:
    virtual bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;
    
    // IOUserClient methods
    virtual void stop(IOService* provider) override;
    virtual bool start(IOService* provider) override;
    
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments, IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) override;
};

#endif /* RadeonSensorUserClient_hpp */
