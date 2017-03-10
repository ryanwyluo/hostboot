/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/kernel/bltohbdatamgr.C $                                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#include <kernel/bltohbdatamgr.H>
#include <util/align.H>
#include <kernel/console.H>
#include <assert.h>

// Global and only BlToHbDataManager instance
BlToHbDataManager g_BlToHbDataManager;

////////////////////////////////////////////////////////////////////////////////
//--------------------------------- Private ----------------------------------//
////////////////////////////////////////////////////////////////////////////////

// Set static variables to control use
Bootloader::BlToHbData BlToHbDataManager::iv_data;
bool BlToHbDataManager::iv_instantiated = false;
bool BlToHbDataManager::iv_initialized = false;
bool BlToHbDataManager::iv_dataValid = false;
size_t BlToHbDataManager::iv_preservedSize = 0;

void BlToHbDataManager::validAssert() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access\n");
        kassert(iv_dataValid);
    }
}

void BlToHbDataManager::print() const
{
    if(iv_dataValid)
    {
        printkd("\nBlToHbData (all addr HRMOR relative):\n");
        printkd("-- eyeCatch = 0x%lX (%s)\n", iv_data.eyeCatch,
                                    reinterpret_cast<char*>(&iv_data.eyeCatch));
        printkd("-- version = 0x%lX\n", iv_data.version);
        printkd("-- branchtableOffset = 0x%lX\n", iv_data.branchtableOffset);
        printkd("-- SecureRom Addr = 0x%lX Size = 0x%lX\n", getSecureRomAddr(),
               iv_data.secureRomSize);
        printkd("-- HW keys' Hash Addr = 0x%lX Size = 0x%lX\n", getHwKeysHashAddr(),
               iv_data.hwKeysHashSize);
        printkd("-- HBB header Addr = 0x%lX Size = 0x%lX\n", getHbbHeaderAddr(),
               iv_data.hbbHeaderSize);
        printkd("-- Reserved Size = 0x%lX\n", iv_preservedSize);
        printkd("\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//---------------------------------- Public ----------------------------------//
////////////////////////////////////////////////////////////////////////////////

BlToHbDataManager::BlToHbDataManager()
{
    // Allow only one instantiation
    if (iv_instantiated)
    {
        printk("E> A BlToHbDataManager class instance already exists\n");
        kassert(!iv_instantiated);
    }
    iv_instantiated = true;
}

void BlToHbDataManager::initValid (const Bootloader::BlToHbData& i_data)
{
    // Allow only one initializer call
    if (iv_initialized)
    {
        printk("E> BlToHbDataManager class previously initialized\n");
        kassert(!iv_initialized);
    }

    // Simple assertion checks
    kassert(i_data.eyeCatch>0);
    kassert(i_data.version>0);
    kassert(i_data.branchtableOffset>0);
    kassert(i_data.secureRom!=nullptr);
    kassert(i_data.hwKeysHash!=nullptr);
    kassert(i_data.hbbHeader!=nullptr);
    kassert(i_data.secureRomSize>0);
    kassert(i_data.hwKeysHashSize>0);
    kassert(i_data.hbbHeaderSize>0);

    // Set internal static data
    iv_data.eyeCatch = i_data.eyeCatch;
    iv_data.version = i_data.version;
    iv_data.branchtableOffset = i_data.branchtableOffset;
    iv_data.secureRom = i_data.secureRom;
    iv_data.secureRomSize = i_data.secureRomSize;
    iv_data.hwKeysHash = i_data.hwKeysHash;
    iv_data.hwKeysHashSize = i_data.hwKeysHashSize;
    iv_data.hbbHeader = i_data.hbbHeader;
    iv_data.hbbHeaderSize = i_data.hbbHeaderSize;

    // Size of data that needs to be preserved and pinned.
    iv_preservedSize = ALIGN_PAGE(iv_data.secureRomSize +
                                 iv_data.hwKeysHashSize +
                                 iv_data.hbbHeaderSize );
    iv_initialized = true;
    iv_dataValid = true;
    print();
}

void BlToHbDataManager::initInvalid ()
{
    // Allow only one initializer call
    if (iv_initialized)
    {
        printk("E> BlToHbDataManager class previously initialized\n");
        kassert(!iv_initialized);
    }

    iv_initialized = true;
    iv_dataValid = false;
    print();
}

const uint64_t BlToHbDataManager::getBranchtableOffset() const
{
    validAssert();
    return iv_data.branchtableOffset;
}

const void* BlToHbDataManager::getSecureRom() const
{
    validAssert();
    return iv_data.secureRom;
}

const uint64_t BlToHbDataManager::getSecureRomAddr() const
{
    validAssert();
    return reinterpret_cast<uint64_t>(iv_data.secureRom);
}

const size_t BlToHbDataManager::getSecureRomSize() const
{
    validAssert();
    return iv_data.secureRomSize;
}

const void* BlToHbDataManager::getHwKeysHash() const
{
    validAssert();
    return iv_data.hwKeysHash;
}

const uint64_t BlToHbDataManager::getHwKeysHashAddr() const
{
    validAssert();
    return reinterpret_cast<uint64_t>(iv_data.hwKeysHash);
}

const size_t BlToHbDataManager::getHwKeysHashSize() const
{
    validAssert();
    return iv_data.hwKeysHashSize;
}

const void* BlToHbDataManager::getHbbHeader() const
{
    validAssert();
    return iv_data.hbbHeader;
}

const uint64_t BlToHbDataManager::getHbbHeaderAddr() const
{
    validAssert();
    return reinterpret_cast<uint64_t>(iv_data.hbbHeader);
}

const size_t BlToHbDataManager::getHbbHeaderSize() const
{
    validAssert();
    return iv_data.hbbHeaderSize;
}

const size_t BlToHbDataManager::getPreservedSize() const
{
    validAssert();
    return iv_preservedSize;
}

const bool BlToHbDataManager::isValid() const
{
    return iv_dataValid;
}