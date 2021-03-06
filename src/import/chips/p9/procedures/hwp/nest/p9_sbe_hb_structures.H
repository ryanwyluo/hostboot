/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/nest/p9_sbe_hb_structures.H $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
//------------------------------------------------------------------------------------
//
/// @file p9_sbe_hb_structures.H
/// @brief Structures that the SBE and HB will both use
//
// *HWP HWP Owner: Joshua Hannan jlhannan@us.ibm.com
// *HWP FW Owner: Thi Tran thi@us.ibm.com
// *HWP Team: Nest
// *HWP Level: 3
// *HWP Consumed by: SBE, HB
//-----------------------------------------------------------------------------------

#ifndef _SBE_HB_STRUCTURES_H_
#define _SBE_HB_STRUCTURES_H_

//-----------------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Structure definitions
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Constant definitions
//-----------------------------------------------------------------------------------

// Used for version checking as the BootloaderConfigData_t structure changes
enum SbeBootloaderVersion
{
    // Keep initial version formatted as it was originally
    INIT = 0x901,
    // Later versions use format [release:2][version:2]
    SAB_ADDED = 0x00090002,
    MMIO_BARS_ADDED = 0x00090003,
};

// Structure starts at the bootloader zero address
struct BootloaderConfigData_t
{
    uint32_t version;         // bytes  4:7  Version field so we know if there is new data being added
    uint8_t sbeBootSide;      // byte   8    0=SBE side 0, 1=SBE side 1   [ATTR_SBE_BOOT_SIDE]
    uint8_t pnorBootSide;     // byte   9    0=PNOR side A, 1=PNOR side B  [ATTR_PNOR_BOOT_SIDE]
    uint16_t pnorSizeMB;      // bytes 10:11 Size of PNOR in MB  [ATTR_PNOR_SIZE]
    uint64_t blLoadSize;      // bytes 12:19 Size of Load (Exception vectors and Bootloader)
    uint8_t secureAccessBit;  // byte  20
    uint64_t xscomBAR;        // bytes 21:28 XSCOM MMIO BAR
    uint64_t lpcBAR;          // bytes 29:36 LPC MMIO BAR
};

#endif
