/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/base/containerheader.C $                   */
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
#include <secureboot/containerheader.H>
#include "../common/securetrace.H"

// Quick change for unit testing
//#define TRACUCOMP(args...)  TRACFCOMP(args)
#define TRACUCOMP(args...)

namespace SECUREBOOT
{

void ContainerHeader::parse_header(const void* i_header)
{
    assert(i_header != NULL);
    const uint8_t* l_hdr = reinterpret_cast<const uint8_t*>(i_header);

    /*---- Parse ROM_container_raw ----*/
    // The rom code has a placeholder for the prefix in the first struct
    size_t l_size = offsetof(ROM_container_raw, prefix);
    safeMemCpyAndInc(&iv_headerInfo.hw_hdr, l_hdr, l_size);

    // Early check if magic number is valid, as a quick check to try and prevent
    // any storage exceptions while parsing header.
    assert(iv_headerInfo.hw_hdr.magic_number == ROM_MAGIC_NUMBER,
           "ContainerHeader: magic number = 0x%08X not valid",
           iv_headerInfo.hw_hdr.magic_number);

    /*---- Parse ROM_prefix_header_raw ----*/
    l_size = offsetof(ROM_prefix_header_raw, ecid);
    safeMemCpyAndInc(&iv_headerInfo.hw_prefix_hdr, l_hdr, l_size);

    // Get ECID array
    l_size = iv_headerInfo.hw_prefix_hdr.ecid_count * ECID_SIZE;
    safeMemCpyAndInc(&iv_headerInfo.hw_prefix_hdr.ecid, l_hdr, l_size);

    /*---- Parse ROM_prefix_data_raw ----*/
    l_size = offsetof(ROM_prefix_data_raw, sw_pkey_p);
    safeMemCpyAndInc(&iv_headerInfo.hw_prefix_data, l_hdr, l_size);

    // Get SW keys
    l_size = iv_headerInfo.hw_prefix_hdr.sw_key_count * sizeof(ecc_key_t);
    // Cache total software keys size
    iv_totalSwKeysSize = l_size;
    safeMemCpyAndInc(&iv_headerInfo.hw_prefix_data.sw_pkey_p, l_hdr, l_size);

    /*---- Parse ROM_sw_header_raw ----*/
    l_size = offsetof(ROM_sw_header_raw, ecid);
    safeMemCpyAndInc(&iv_headerInfo.sw_hdr, l_hdr, l_size);

    // Get ECID array
    l_size = iv_headerInfo.sw_hdr.ecid_count * ECID_SIZE;
    safeMemCpyAndInc(&iv_headerInfo.sw_hdr.ecid, l_hdr, l_size);

    /*---- Parse ROM_sw_sig_raw ----*/
    safeMemCpyAndInc(&iv_headerInfo.sw_sig.sw_sig_p, l_hdr, iv_totalSwKeysSize);

    // Parse hw and sw flags
    parseFlags();

    // Generate hw hash key
    genHwKeyHash();

    // After parsing check if header is valid, do some quick bound checks
    validate();

    // Debug printing
    print();
}

void ContainerHeader::print() const
{
#ifdef HOSTBOOT_DEBUG
    TRACFCOMP(g_trac_secure, ENTER_MRK"ContainerHeader::print");

    TRACFCOMP(g_trac_secure,"header content size 0x%X", iv_hdrBytesRead);

    /*---- Print ROM_container_raw ----*/
    TRACFCOMP(g_trac_secure,"magic_number 0x%X", iv_headerInfo.hw_hdr.magic_number);
    TRACFCOMP(g_trac_secure,"version 0x%X", iv_headerInfo.hw_hdr.version);
    TRACFCOMP(g_trac_secure,"container_size 0x%X", iv_headerInfo.hw_hdr.container_size);
    TRACFCOMP(g_trac_secure,"target_hrmor 0x%X", iv_headerInfo.hw_hdr.target_hrmor);
    TRACFCOMP(g_trac_secure,"stack_pointer 0x%X", iv_headerInfo.hw_hdr.stack_pointer);
    TRACFBIN(g_trac_secure,"hw_pkey_a", iv_headerInfo.hw_hdr.hw_pkey_a, 64);
    TRACFBIN(g_trac_secure,"hw_pkey_b", iv_headerInfo.hw_hdr.hw_pkey_b, 64);
    TRACFBIN(g_trac_secure,"hw_pkey_c", iv_headerInfo.hw_hdr.hw_pkey_c, 64);

    /*---- Print ROM_prefix_header_raw ----*/
    TRACFCOMP(g_trac_secure,"hw_flags 0x%X", iv_headerInfo.hw_prefix_hdr.flags);
    TRACFCOMP(g_trac_secure,"sw_key_count 0x%X", iv_headerInfo.hw_prefix_hdr.sw_key_count);
    TRACFBIN(g_trac_secure,"sw public key hash", iv_headerInfo.hw_prefix_hdr.payload_hash, SHA512_DIGEST_LENGTH);

    /*---- Print ROM_prefix_data_raw ----*/
    TRACFBIN(g_trac_secure,"sw_pkey_p", iv_headerInfo.hw_prefix_data.sw_pkey_p, sizeof(ecc_key_t));
    if (iv_headerInfo.hw_prefix_hdr.sw_key_count>1)
    {
        TRACFBIN(g_trac_secure,"sw_pkey_q", iv_headerInfo.hw_prefix_data.sw_pkey_q, sizeof(ecc_key_t));
    }
    if (iv_headerInfo.hw_prefix_hdr.sw_key_count>2)
    {
        TRACFBIN(g_trac_secure,"sw_pkey_r", iv_headerInfo.hw_prefix_data.sw_pkey_r, sizeof(ecc_key_t));
    }

    /*---- Print ROM_sw_header_raw ----*/
    TRACFCOMP(g_trac_secure,"payload_size 0x%X", iv_headerInfo.sw_hdr.payload_size );
    TRACFBIN(g_trac_secure,"payload_hash", iv_headerInfo.sw_hdr.payload_hash, SHA512_DIGEST_LENGTH);

    /*---- Print ROM_sw_sig_raw ----*/
    TRACFBIN(g_trac_secure,"sw_sig_p", iv_headerInfo.sw_sig.sw_sig_p, sizeof(ecc_key_t));
    if (iv_headerInfo.hw_prefix_hdr.sw_key_count>1)
    {
        TRACFBIN(g_trac_secure,"sw_sig_q", iv_headerInfo.sw_sig.sw_sig_q, sizeof(ecc_key_t));
    }
    if (iv_headerInfo.hw_prefix_hdr.sw_key_count>2)
    {
        TRACFBIN(g_trac_secure,"sw_sig_r", iv_headerInfo.sw_sig.sw_sig_r, sizeof(ecc_key_t));
    }

    TRACFCOMP(g_trac_secure, EXIT_MRK"ContainerHeader::print");
#endif
}

size_t ContainerHeader::totalContainerSize() const
{
    return iv_headerInfo.hw_hdr.container_size;
}

const ecc_key_t* ContainerHeader::hw_keys() const
{
    return &iv_headerInfo.hw_hdr.hw_pkey_a;
}

size_t ContainerHeader::payloadTextSize() const
{
    return iv_headerInfo.sw_hdr.payload_size;
}

const SHA512_t* ContainerHeader::payloadTextHash() const
{
    return &iv_headerInfo.sw_hdr.payload_hash;
}

size_t ContainerHeader::totalSwKeysSize() const
{
    return iv_totalSwKeysSize;
}

const ecc_key_t* ContainerHeader::sw_keys() const
{
    return &iv_headerInfo.hw_prefix_data.sw_pkey_p;
}

const SHA512_t* ContainerHeader::swKeyHash() const
{
    return &iv_headerInfo.hw_prefix_hdr.payload_hash;
}

const ecc_key_t* ContainerHeader::sw_sigs() const
{
    return &iv_headerInfo.sw_sig.sw_sig_p;
}

const sb_flags_t* ContainerHeader::sb_flags() const
{
    return &iv_sbFlags;
}

const SHA512_t* ContainerHeader::hwKeyHash() const
{
    return &iv_hwKeyHash;
}

void ContainerHeader::validate()
{
    iv_isValid = (iv_hdrBytesRead <= MAX_SECURE_HEADER_SIZE)
        && (iv_headerInfo.hw_hdr.magic_number == ROM_MAGIC_NUMBER)
        && (iv_headerInfo.hw_hdr.version == ROM_VERSION)
        && (iv_headerInfo.hw_prefix_hdr.ver_alg.version == ROM_VERSION)
        && (iv_headerInfo.hw_prefix_hdr.ver_alg.hash_alg == ROM_HASH_ALG)
        && (iv_headerInfo.hw_prefix_hdr.ver_alg.sig_alg == ROM_SIG_ALG)
        && (iv_headerInfo.hw_prefix_hdr.sw_key_count >= SW_KEY_COUNT_MIN)
        && (iv_headerInfo.hw_prefix_hdr.sw_key_count <= SW_KEY_COUNT_MAX)
        && (iv_headerInfo.sw_hdr.payload_size != 0);
}

void ContainerHeader::safeMemCpyAndInc(void* i_dest, const uint8_t* &io_hdr,
                                       const size_t i_size)
{
    assert(i_dest != NULL, "ContainerHeader: dest ptr NULL");
    assert(io_hdr != NULL, "ContainerHeader: current header location ptr NULL");
    assert(iv_pHdrStart != NULL, "ContainerHeader: start of header ptr NULL");

    TRACDCOMP(g_trac_secure,"dest: 0x%X src: 0x%X size: 0x%X",i_dest, io_hdr, i_size);

    // Determine if the memcpy is within the bounds of the container header
    iv_hdrBytesRead = io_hdr - iv_pHdrStart;
    assert( (iv_hdrBytesRead + i_size) <= MAX_SECURE_HEADER_SIZE,
            "ContainerHeader: memcpy is out of bounds of max header size");

    memcpy(i_dest, io_hdr, i_size);
    io_hdr += i_size;
}

bool ContainerHeader::isValid() const
{
    return iv_isValid;
}

void ContainerHeader::parseFlags()
{
    iv_sbFlags.hw_hb_fw = iv_headerInfo.hw_prefix_hdr.flags & HB_FW_FLAG;
    iv_sbFlags.hw_opal = iv_headerInfo.hw_prefix_hdr.flags & OPAL_FLAG;
    iv_sbFlags.hw_phyp = iv_headerInfo.hw_prefix_hdr.flags & PHYP_FLAG;
    iv_sbFlags.hw_lab_override =(  iv_headerInfo.hw_prefix_hdr.flags
                                 & LAB_OVERRIDE_FLAG);
    iv_sbFlags.hw_key_transition =(  iv_headerInfo.hw_prefix_hdr.flags
                                   & KEY_TRANSITION_FLAG);
}

void ContainerHeader::genHwKeyHash()
{
    // Generate and store hw hash key
    SECUREBOOT::hashBlob(&iv_headerInfo.hw_hdr.hw_pkey_a,
                         totalHwKeysSize, iv_hwKeyHash);
}

}; //end of SECUREBOOT namespace
