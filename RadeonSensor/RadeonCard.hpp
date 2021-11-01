//
//  RadeonCard.hpp
//  Based upon RadeonMonitor by Slice
//

#ifndef RadeonCard_hpp
#define RadeonCard_hpp

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>


enum TempFamilies {
    R9xx,
    RCIx,
    RAIx,
    RVEx,
};

enum ChipFamily {
  CHIP_FAMILY_PITCAIRN,
  CHIP_FAMILY_HAWAII,
  CHIP_FAMILY_POLARIS,
  CHIP_FAMILY_VEGA,
};

class RadeonCard: public OSObject {
    OSDeclareDefaultStructors(RadeonCard)
    
private:
    UInt32 deviceId;
    UInt16 chipFamily;
    UInt16 tempFamily;
    
    volatile UInt8* mmioBase;
    IOMemoryMap *   mmio;
    UInt32          caps;
    UInt32          tReg;
    int             cardNumber;
        
public:
    bool        initialize(IOPCIDevice* radeonDevice, UInt32 chipID);
    IOReturn    getTemperature(UInt16* data);
    
private:
    bool        initializeFamily(UInt32 chipID);
    
    UInt32      read32(UInt32 reg);
    void        write32(UInt32 reg, UInt32 val);
    UInt32      read_ind(UInt32 reg);
    UInt32      read_smc(UInt32 reg);
    
    IOReturn    tahitiTemperature(UInt16* data);
    IOReturn    arcticTemperature(UInt16* data);
    IOReturn    vegaTemperature(UInt16* data);
};

#endif /* RadeonCard_hpp */
