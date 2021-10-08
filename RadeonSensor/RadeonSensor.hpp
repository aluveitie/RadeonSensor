//
//  RadeonSensor.hpp
//  RadeonSensor
//
//  Created by Aluveitie on 24.09.21.
//

#ifndef RadeonSensor_hpp
#define RadeonSensor_hpp

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>

#include "RadeonMonitor/ATICard.hpp"

#include <i386/proc_reg.h>
#include <libkern/libkern.h>

#include <Headers/kern_efi.hpp>
#include <Headers/kern_util.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_time.hpp>



class RadeonSensor : public IOService {
    OSDeclareDefaultStructors(RadeonSensor)
    
public:
    virtual bool init(OSDictionary* dictionary) override;
    virtual IOService* probe(IOService* provider, SInt32* score) override;
    virtual void free(void) override;
    
    virtual bool start(IOService* provider) override;
    virtual void stop(IOService* provider) override;
    
    uint16_t getTemperature();
    

private:
    UInt32 device_id = 0;
    ATICard* atiCard;
    IOPCIDevice* pciCard;
};

#endif /* RadeonSensor_hpp */
