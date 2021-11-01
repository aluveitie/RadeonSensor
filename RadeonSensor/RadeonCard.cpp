//
//  RadeonCard.cpp
//  Based upon RadeonMonitor by Slice
//
//

#include "RadeonCard.hpp"
#include "RadeonChipsets.h"

OSDefineMetaClassAndStructors(RadeonCard, OSObject)

bool RadeonCard::initialize(IOPCIDevice* radeonDevice, UInt32 chipID) {
    if (!initializeFamily(chipID))
        return false;
    
    IOMemoryMap* mmio5 = NULL;
    radeonDevice->setMemoryEnable(true);
    IOMemoryDescriptor* theDescriptor;
    IOPhysicalAddress bar = (IOPhysicalAddress)((radeonDevice->configRead32(kIOPCIConfigBaseAddress5)) & ~0x3f);
    IOLog("RadeonSensor::RadeonCard register space5=%08lx\n", (long unsigned int)bar);
    theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x80000, kIODirectionOutIn); // | kIOMapInhibitCache);
    
    if(theDescriptor != NULL) {
        mmio5 = theDescriptor->map();
    }
    
    mmio = radeonDevice->mapDeviceMemoryWithIndex(1);
    if (mmio)    {
        mmioBase = (volatile UInt8 *)mmio->getVirtualAddress();
        IOLog("RadeonSensor::ATICard mmio_base=0x%llx\n", mmio->getPhysicalAddress());
    } else {
        IOLog("RadeonSensor::ATICard have no mmio\n");
        return false;
    }
    
    if (!mmioBase || chipFamily >= CHIP_FAMILY_HAWAII) {
        //    IOMemoryMap *   mmio5;
        //    mmio5 = VCard->mapDeviceMemoryWithIndex(4);
        if (mmio5 && mmio5->getPhysicalAddress() != 0) {
            mmio = mmio5;
            mmioBase = (volatile UInt8 *)mmio->getVirtualAddress();
        }
        IOLog("RadeonSensor::ATICard use mmio5 at 0x%llx\n", (unsigned long long)mmioBase);
    }
    
    return true;
}

IOReturn RadeonCard::getTemperature(UInt16* data) {
    switch (tempFamily) {
        case R9xx:
            return tahitiTemperature(data);
        case RCIx:
            return tahitiTemperature(data);
        case RAIx:
            return arcticTemperature(data);
        case RVEx:
            return vegaTemperature(data);
        default:
            return kIOReturnError;
    }
}

bool RadeonCard::initializeFamily(UInt32 chipID) {
    UInt16 devID = chipID & 0xffff;
    // Vega/Navi
    if (((devID == 0x66af)) ||
        ((devID >= 0x6860) && (devID <= 0x687F)) ||
        ((devID >= 0x7301) && (devID <= 0x73BF))
        ) {
        deviceId = devID;
        chipFamily = CHIP_FAMILY_VEGA;
        tempFamily = RVEx;
        IOLog("RadeonSensor::ATICard common ATI Radeon VEGA DID=%04lx\n", (long unsigned int)devID);
    }
    // Polaris RX 4xx/5xx
    else if (((devID >= 0x67C0) && (devID <= 0x67FF)) ||  //Polaris 10,11
               ((devID >= 0x6980) && (devID <= 0x699F))   //Polaris 12, RX550
               ) {
        deviceId = devID;
        chipFamily = CHIP_FAMILY_POLARIS;
        tempFamily = RAIx;
        IOLog("RadeonSensor::ATICard common ATI Radeon Polaris DID=%04lx\n", (long unsigned int)devID);
    }
    // SeaIsland R7-2xx, 3xx, 4xx, 5xx
    else if (((devID >= 0x67A0) && (devID <= 0x67BF)) ||  //Hawaii
               ((devID >= 0x6900) && (devID <= 0x693F)) ||  //Volcanic Island, Tonga
               ((devID >= 0x6600) && (devID <= 0x663F)) ||  //Oland
               ((devID >= 0x6640) && (devID <= 0x666F))) {  //Bonair & Hainan
        deviceId = devID;
        chipFamily = CHIP_FAMILY_HAWAII;
        tempFamily = RCIx;
        IOLog("RadeonSensor::ATICard common ATI Radeon SeaIsland DID=%04lx\n", (long unsigned int)devID);
    }
    // SouthernIsland HD7xxx, HD8xxx
    else if (((devID >= 0x6780) && (devID <= 0x679F)) ||  //Tahiti
              ((devID >= 0x6800) && (devID <= 0x683F))) {  //Pitcairn, Verde
        deviceId= devID;
        chipFamily = CHIP_FAMILY_PITCAIRN;
        tempFamily = R9xx;
        IOLog("RadeonSensor::ATICard common ATI Radeon SouthernIsland DID=%04lx\n", (long unsigned int)devID);
    } else {
        IOLog("RadeonSensor::ATICard unsupported card DID=%04lx\n", (long unsigned int)devID);
        return false;
    }
    
    return true;
}


UInt32 RadeonCard::read_smc(UInt32 reg) {
    UInt32 r;
    write32(SMC_IND_INDEX_0, (reg));
    r = read32(SMC_IND_DATA_0);
    return r;
}

UInt32 RadeonCard::read_ind(UInt32 reg) {
    //unsigned long flags;
    UInt32 r;
    //spin_lock_irqsave(&rdev->smc_idx_lock, flags);
    write32(mmSMC_IND_INDEX_11, reg);
    r = read32(mmSMC_IND_DATA_11);
    //spin_unlock_irqrestore(&rdev->smc_idx_lock, flags);
    return r;
}

UInt32 RadeonCard::read32(UInt32 reg) {
   return OSReadLittleInt32((mmioBase), reg);
}

void RadeonCard::write32(UInt32 reg, UInt32 val) {
   return OSWriteLittleInt32((mmioBase), reg, val);
}



IOReturn RadeonCard::tahitiTemperature(UInt16* data) {
    UInt32 temp, actual_temp = 0;
    for (int i=0; i<1000; i++) {  //attempts to ready
        temp = (read32(CG_SI_THERMAL_STATUS) & CTF_TEMP_MASK) >> CTF_TEMP_SHIFT;
        if ((temp >> 10) & 1) {
            actual_temp = 0;
        } else if ((temp >> 9) & 1){
            actual_temp = 255;
        } else {
            actual_temp = temp; //(temp >> 1) & 0xff;
            break;
        }
        IOSleep(10);
    }
    
    *data = (UInt16)(actual_temp & 0x1ff);
    //data[1] = 0;
    return kIOReturnSuccess;
}


IOReturn RadeonCard::arcticTemperature(UInt16* data) {
    UInt32 temp, actual_temp = 0;
    for (int i=0; i<1000; i++) {  //attempts to ready
        temp = (read_ind(CG_CI_MULT_THERMAL_STATUS) & CI_CTF_TEMP_MASK) >> CI_CTF_TEMP_SHIFT;
        if ((temp >> 10) & 1) {
            actual_temp = 0;
        } else if ((temp >> 9) & 1) {
            actual_temp = 255;
        } else {
            actual_temp = temp & 0x1ff; //(temp >> 1) & 0xff;
            break;
        }
        IOSleep(10);
    }
    
    *data = (UInt16)(actual_temp & 0x1ff);
    //data[1] = 0;
    return kIOReturnSuccess;
}

IOReturn RadeonCard::vegaTemperature(UInt16* data) {
    UInt32 temp, actual_temp = 0;
    
    temp = read32(mmTHM_TCON_CUR_TMP) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;
    actual_temp = temp & 0x1ff;
    *data = (UInt16)actual_temp;
    
    return kIOReturnSuccess;
}
