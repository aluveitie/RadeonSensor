/*
 *  ATICard.cpp
 *  FakeSMCRadeon
 *
 *  Created by Slice on 24.07.10.
 *  Copyright 2010 Applelife.ru. All rights reserved.
 *
 */

#include "ATICard.hpp"
#include "radeon_chipinfo_gen.h"
//#include "Sensor.h"
OSDefineMetaClassAndStructors(ATICard, OSObject)

bool ATICard::initialize() {
  IOMemoryMap *   mmio5 = NULL;
    rinfo = (RADEONCardInfo*)IOMalloc(sizeof(RADEONCardInfo));
    VCard->setMemoryEnable(true);
  IOMemoryDescriptor *    theDescriptor;
  IOPhysicalAddress bar = (IOPhysicalAddress)((VCard->configRead32(kIOPCIConfigBaseAddress5)) & ~0x3f);
  InfoLog("register space5=%08lx\n", (long unsigned int)bar);
  theDescriptor = IOMemoryDescriptor::withPhysicalAddress (bar, 0x80000, kIODirectionOutIn); // | kIOMapInhibitCache);
  
  if(theDescriptor != NULL) {
    mmio5 = theDescriptor->map();
  }

    /*
     // PCI dump
     for (int i=0; i<0xff; i +=16) {
        IOLog("%02lx: ", (long unsigned int)i);
        for (int j=0; j<16; j += 4) {
            IOLog("%08lx ", (long unsigned int)VCard->configRead32(i+j));
        }
        IOLog("\n");
     }
     */
  //Test for ElCapitan problem
/*  long unsigned int j = VCard->configRead32(0xAC);
  IOLog("PMIR value: %08lx\n", j);
  VCard->configWrite32(0xAC, (j & (~0x1)) | 0x2);
 */
  //
  /*
    for (UInt32 i = 0; (mmio = VCard->mapDeviceMemoryWithIndex(i)); i++) {
        long unsigned int mmio_base_phys = mmio->getPhysicalAddress();
        // Make sure we  select MMIO registers
        if (((mmio->getLength()) <= 0x00080000) && (mmio_base_phys != 0))
            break;
    }
   */
  mmio = VCard->mapDeviceMemoryWithIndex(1);
    if (mmio)    {
        mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
    InfoLog("mmio_base=0x%llx\n", mmio->getPhysicalAddress());
    } else {
        InfoLog(" have no mmio\n ");
        return false;
    }
    
  if(!getRadeonInfo()) { return false; }
  
  if (!mmio_base || rinfo->ChipFamily >= CHIP_FAMILY_HAWAII) {
//    IOMemoryMap *   mmio5;
//    mmio5 = VCard->mapDeviceMemoryWithIndex(4);
    if (mmio5 && mmio5->getPhysicalAddress() != 0) {
      mmio = mmio5;
      mmio_base = (volatile UInt8 *)mmio->getVirtualAddress();
    }
    InfoLog(" use mmio5 at 0x%llx\n", (unsigned long long)mmio_base);
  }

  switch (rinfo->ChipFamily) {
    case CHIP_FAMILY_R600:
    case CHIP_FAMILY_RV610:
    case CHIP_FAMILY_RV630:
    case CHIP_FAMILY_RV670:
      //setup_R6xx();
      tempFamily = R6xx;
      break;
    case CHIP_FAMILY_R700:
    case CHIP_FAMILY_R710:
    case CHIP_FAMILY_R730:
    case CHIP_FAMILY_RV740:
    case CHIP_FAMILY_RV770:
    case CHIP_FAMILY_RV790:
      //setup_R7xx();
      tempFamily = R7xx;
      break;
    case CHIP_FAMILY_Evergreen:
      //setup_Evergreen();
      tempFamily = R8xx;
      break;
    case   CHIP_FAMILY_PITCAIRN:
    case   CHIP_FAMILY_TAHITI:
    case   CHIP_FAMILY_VERDE:
      tempFamily = R9xx;
      break;
    case   CHIP_FAMILY_HAWAII:
    case   CHIP_FAMILY_OLAND:
    case   CHIP_FAMILY_BONAIRE:
    case   CHIP_FAMILY_HAINAN:
    case   CHIP_FAMILY_TONGA:
      tempFamily = RCIx;
      break;
    case   CHIP_FAMILY_POLARIS:
      tempFamily = RAIx;
      break;
    case   CHIP_FAMILY_VEGA:
      tempFamily = RVEx;
      break;
      
    default:
      InfoLog("sorry, but your card %04lx is not supported!\n", (long unsigned int)(rinfo->device_id));
      return false;
  }

    return true;
}

bool ATICard::getRadeonInfo() {
  UInt16 devID = chipID & 0xffff;

  RADEONCardInfo *devices = radeon_device_list;
  //rinfo = new RADEONCardInfo;
  //old devices
  while (devices->device_id != 0) {
    //IOLog("check %d/n", devices->device_id ); //Debug
    if ((devices->device_id & 0xffff) == devID ) {
      //      rinfo->device_id = devID;
      rinfo->device_id = devices->device_id;
      rinfo->ChipFamily = devices->ChipFamily;
      family = devices->ChipFamily;
      rinfo->igp = devices->igp;
      rinfo->is_mobility = devices->is_mobility;
      IOLog(" Found ATI Radeon %04lx\n", (long unsigned int)devID);
      return true;
    }
    devices++;
  }

  //Vega
  if (((devID == 0x66af)) ||
      ((devID >= 0x6860) && (devID <= 0x687F)) ||
      ((devID >= 0x7301) && (devID <= 0x73BF))
      ) {  //Vega
    rinfo->device_id = devID;
    rinfo->ChipFamily = CHIP_FAMILY_VEGA;
    family = CHIP_FAMILY_VEGA;
    rinfo->igp = 0;
    rinfo->is_mobility = false;
    InfoLog(" Common ATI Radeon VEGA DID=%04lx\n", (long unsigned int)devID);
    return true;

  //Polaris
  } else if (((devID >= 0x67C0) && (devID <= 0x67FF)) ||  //Polaris 10,11
             ((devID >= 0x6980) && (devID <= 0x699F))   //Polaris 12, RX550
             ) {
    rinfo->device_id = devID;
    rinfo->ChipFamily = CHIP_FAMILY_POLARIS;
    family = CHIP_FAMILY_POLARIS;
    rinfo->igp = 0;
    rinfo->is_mobility = false;
    InfoLog(" Common ATI Radeon Polaris DID=%04lx\n", (long unsigned int)devID);
    return true;

  //SeaIsland R7-2xx, 3xx, 4xx, 5xx,
  } else if (((devID >= 0x67A0) && (devID <= 0x67BF)) ||  //Hawaii
      ((devID >= 0x6900) && (devID <= 0x693F)) ||  //Volcanic Island, Tonga
      ((devID >= 0x6600) && (devID <= 0x663F)) ||  //Oland
      ((devID >= 0x6640) && (devID <= 0x666F))) {  //Bonair & Hainan
    rinfo->device_id = devID;
    rinfo->ChipFamily = CHIP_FAMILY_HAWAII;
    family = CHIP_FAMILY_HAWAII;
    rinfo->igp = 0;
    rinfo->is_mobility = false;
    InfoLog(" Common ATI Radeon SeaIsland DID=%04lx\n", (long unsigned int)devID);
    return true;
    //SouthernIsland HD7xxx
  } else if (((devID >= 0x6780) && (devID <= 0x679F)) ||  //Tahiti
               ((devID >= 0x6800) && (devID <= 0x683F))) {  //Pitcairn, Verde

    rinfo->device_id = devID;
    rinfo->ChipFamily = CHIP_FAMILY_PITCAIRN;
    family = CHIP_FAMILY_PITCAIRN;
    rinfo->igp = 0;
    rinfo->is_mobility = false;
    InfoLog(" Common ATI Radeon SouthernIsland DID=%04lx\n", (long unsigned int)devID);
    return true;
    //Evergreen HD5xxx and NothenIsland HD6xxx
  } else if (((devID & 0xFF00) == 0x6700) || ((devID & 0xFF00) == 0x6800)) {
    rinfo->device_id = devID;
    rinfo->ChipFamily = CHIP_FAMILY_Evergreen;
    family = CHIP_FAMILY_Evergreen;
    rinfo->igp = 0;
    rinfo->is_mobility = false;
    InfoLog(" Common ATI Radeon Evergreen/NothenIsland DID=%04lx\n", (long unsigned int)devID);
    //    IOLog("sorry, not supported yet, please report DeviceID=0x%x\n", devID);
    return true;
  }

  InfoLog("Unknown DeviceID!\n");
  return false;
}
/*
void ATICard::setup_R6xx() {
    char key[5];
    int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
    if (id == -1) {
        InfoLog("No new GPU SMC key!\n");
        return;
    }
    card_number = id;
    tempSensor = new R6xxTemperatureSensor(this, id, key, TYPE_SP78, 2);
    Caps = GPU_TEMP_MONITORING;
}
void ATICard::setup_R7xx() {
    char key[5];
    int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
    if (id == -1) {
        InfoLog("No new GPU SMC key!\n");
        return;
    }
    card_number = id;
    tempSensor = new R7xxTemperatureSensor(this, id, key, TYPE_SP78, 2);
    Caps = GPU_TEMP_MONITORING;
}
void ATICard::setup_Evergreen() {
    char key[5];
    int id = GetNextUnusedKey(KEY_FORMAT_GPU_DIODE_TEMPERATURE, key);
    if (id == -1) {
        InfoLog("No new GPU SMC key!\n");
        return;
    }
    card_number = id;
    tempSensor = new EverTemperatureSensor(this, id, key, TYPE_SP78, 2);
    Caps = GPU_TEMP_MONITORING;
}
*/
UInt32 ATICard::read32(UInt32 reg) {
    return INVID(reg);
}

void ATICard::write32(UInt32 reg, UInt32 val) {
    return OUTVID(reg, val);
}

//read_ind_pcie ->
/*
WREG32(mmPCIE_INDEX, reg);
(void)RREG32(mmPCIE_INDEX);
r = RREG32(mmPCIE_DATA);
*/

//GetClock
/*
PPSMC_Result amdgpu_ci_send_msg_to_smc(struct amdgpu_device *adev, PPSMC_Msg msg) {
  u32 tmp;
  int i;
  
  if (!amdgpu_ci_is_smc_running(adev)) {
    return PPSMC_Result_Failed;
  }
  
  WREG32(mmSMC_MESSAGE_0, msg);
  
  for (i = 0; i < adev->usec_timeout; i++) {
    tmp = RREG32(mmSMC_RESP_0);
    if (tmp != 0) {
      break;
    }
    udelay(1);
  }
  tmp = RREG32(mmSMC_RESP_0);
  
  return (PPSMC_Result)tmp;
}
amdgpu_ci_send_msg_to_smc(adev, PPSMC_MSG_API_GetSclkFrequency); //SCLK
amdgpu_ci_send_msg_to_smc(adev, PPSMC_MSG_API_GetMclkFrequency); //MCLK
clock = RREG32(mmSMC_MSG_ARG_0); units=100MHz
//get FAN
#define  CG_THERMAL_STATUS      0xC0300008
#define    FDO_PWM_DUTY(x)        ((x) << 9)
#define    FDO_PWM_DUTY_MASK      (0xff << 9)
#define    FDO_PWM_DUTY_SHIFT      9
#define  CG_FDO_CTRL1          0xC0300068
#define    FMAX_DUTY100(x)        ((x) << 0)
#define    FMAX_DUTY100_MASK      0x000000FF
#define    FMAX_DUTY100_SHIFT      0
RREG32_SMC based on 0x200
cik -> mmSMC_IND_INDEX_0             0x200
si  -> SMC_IND_INDEX_0               0x200
vi  -> mmSMC_IND_INDEX_11   0x1AC => 0x6b0
ni  -> TN_SMC_IND_INDEX_0            0x200
cz  ->
#define mmMP0PUB_IND_INDEX  0x180 => 0x600
#define mmMP0PUB_IND_DATA   0x181
int ci_fan_ctrl_get_fan_speed_percent(struct radeon_device *rdev, u32 *speed) {
  u32 duty, duty100;
  u64 tmp64;
  
  if (rdev->pm.no_fan)
    return -ENOENT;
  
  duty100 = (RREG32_SMC(CG_FDO_CTRL1) & FMAX_DUTY100_MASK) >> FMAX_DUTY100_SHIFT;
  duty = (RREG32_SMC(CG_THERMAL_STATUS) & FDO_PWM_DUTY_MASK) >> FDO_PWM_DUTY_SHIFT;
  
  if (duty100 == 0)
    return -EINVAL;
  
  tmp64 = (u64)duty * 100;
  do_div(tmp64, duty100);
  *speed = (u32)tmp64;
  
  if (*speed > 100) {
    *speed = 100;
  }
  
  return 0;
}
*/

UInt32 ATICard::read_smc(UInt32 reg) {
  UInt32 r;
  OUTVID(SMC_IND_INDEX_0, (reg));
  r = INVID(SMC_IND_DATA_0);
  return r;
}


UInt32 ATICard::read_ind(UInt32 reg) {
  //unsigned long flags;
  UInt32 r;
  //spin_lock_irqsave(&rdev->smc_idx_lock, flags);
  OUTVID(mmSMC_IND_INDEX_11, (reg));
  r = INVID(mmSMC_IND_DATA_11);
  //spin_unlock_irqrestore(&rdev->smc_idx_lock, flags);
  return r;
}

IOReturn ATICard::R6xxTemperatureSensor(UInt16* data) {
  UInt32 temp, actual_temp = 0;
  for (int i=0; i<1000; i++) {  //attempts to ready
    temp = (read32(CG_THERMAL_STATUS) & ASIC_T_MASK) >> ASIC_T_SHIFT;
    if ((temp >> 7) & 1) {
      actual_temp = 0;
    } else {
      actual_temp = temp & 0xff; //(temp >> 1)
      break;
    }
    IOSleep(10);
  }
  *data = (UInt16)(actual_temp & 0x1ff);
  //data[1] = 0;
  return kIOReturnSuccess;
}

IOReturn ATICard::R7xxTemperatureSensor(UInt16* data) {
  UInt32 temp, actual_temp = 0;
  for (int i=0; i<1000; i++) {  //attempts to ready
    temp = (read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;
    if ((temp >> 9) & 1) {
      actual_temp = 0;
    } else {
      actual_temp = (temp >> 1) & 0xff;
      break;
    }
    IOSleep(10);
  }
  
  *data = (UInt16)(actual_temp & 0x1ff);
  //data[1] = 0;
  return kIOReturnSuccess;
}

IOReturn ATICard::EverTemperatureSensor(UInt16* data) {
    UInt32 temp, actual_temp = 0;
    for (int i=0; i<1000; i++) {  //attempts to ready
        temp = (read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;
    if ((temp >> 10) & 1) {
            actual_temp = 0;
    } else if ((temp >> 9) & 1) {
            actual_temp = 255;
    } else {
            actual_temp = (temp >> 1) & 0xff;
            break;
        }
        IOSleep(10);
    }
    
    *data = (UInt16)(actual_temp & 0x1ff);
    //data[1] = 0;
    return kIOReturnSuccess;
}

IOReturn ATICard::TahitiTemperatureSensor(UInt16* data) {
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

IOReturn ATICard::HawaiiTemperatureSensor(UInt16* data) {
    UInt32 temp, actual_temp = 0;
    for (int i=0; i<1000; i++) {  //attempts to ready
        temp = (read_smc(CG_CI_MULT_THERMAL_STATUS) & CI_CTF_TEMP_MASK) >> CI_CTF_TEMP_SHIFT;
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

IOReturn ATICard::ArcticTemperatureSensor(UInt16* data) {
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

//#define mmTHM_TCON_CUR_TMP                    0x59800
//#define THM_TCON_CUR_TMP__CUR_TEMP__SHIFT     24

IOReturn ATICard::VegaTemperatureSensor(UInt16* data) {
  UInt32 temp, actual_temp = 0;

  temp = read32(mmTHM_TCON_CUR_TMP) >> THM_TCON_CUR_TMP__CUR_TEMP__SHIFT;
  actual_temp = temp & 0x1ff;
  *data = (UInt16)actual_temp;

  return kIOReturnSuccess;
}
