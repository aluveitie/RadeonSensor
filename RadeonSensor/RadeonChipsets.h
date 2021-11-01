//
//

#ifndef RadeonChipsets_h
#define RadeonChipsets_h

// macOS using byte addressing instead of Linux u32 word addressing

#define CG_SI_THERMAL_STATUS 0x714
#define CTF_TEMP_MASK 0x0003fe00
#define CTF_TEMP_SHIFT 9

#define CG_CI_MULT_THERMAL_STATUS 0xC0300014
#define CI_CTF_TEMP_MASK 0x0003fe00
#define CI_CTF_TEMP_SHIFT 9

#define mmSMC_IND_INDEX_11  (0x1AC * 4)
#define mmSMC_IND_DATA_11   (0x1AD * 4)
#define mmSMC_MSG_ARG_0     (0xA4 * 4)
//#define mmSMC_MESSAGE_0     (0x008B * 4) //smu_6
#define mmSMC_MESSAGE_0     (0x94 * 4) /* SMU_7 */
#define mmSMC_RESP_0        (0x95 * 4) /* SMU_7 */

// SeaIsland+
#define SMC_IND_INDEX_0 0x200
#define SMC_IND_DATA_0  0x204

#define mmPCIE_INDEX    (0x000C * 4)
#define mmPCIE_DATA     (0x000D * 4)

// VEGA10+
#define mmTHM_TCON_CUR_TMP 0x59800
#define THM_TCON_CUR_TMP__CUR_TEMP__SHIFT 24

#endif /* RadeonChipsets_h */
