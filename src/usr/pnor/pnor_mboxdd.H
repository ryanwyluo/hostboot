/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/pnor/pnor_mboxdd.H $                                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#ifndef __PNOR_MBOXDD_H
#define __PNOR_MBOXDD_H

#include <limits.h>
#include <config.h>
namespace PNOR
{
class UdPnorDDParms;
}

//NOTE: Protocol Definition is here:
//   https://github.com/openbmc/mboxbridge/blob/master/Documentation/mbox_protocol.md

class astMbox;

/** @file pnor_mboxdd.H
 *  @brief Provides the interfaces to the PNOR via the
 *         MBOX protocol
 */

/**
 *  @brief  PNOR Device Driver Class
 *     Provides access to the PNOR flash via the LPC MBOX hardware
 */
class PnorDD
{

    public:
        /**
         * @brief Performs a PNOR Read Operation
         *
         * @parm o_buffer  Buffer to read data into
         * @parm io_buflen  Input: Number of bytes to read,
         *       Output: Number of bytes actually read
         * @parm i_address  Offset into flash to read
         *
         * @return Error from operation
         */
        errlHndl_t readFlash(void* o_buffer,
                             size_t& io_buflen,
                             uint64_t i_address);

        /**
         * @brief Performs a PNOR Write Operation
         *
         * @parm i_buffer  Buffer to write data from
         * @parm io_buflen  Input: Number of bytes to write,
         *       Output: Number of bytes actually written
         * @parm i_address  Offset into flash to write
         *
         * @return Error from operation
         */
        errlHndl_t writeFlash(const void* i_buffer,
                              size_t& io_buflen,
                              uint64_t i_address);

        /**
         * @brief Informs caller if PNORDD is using
         *        L3 Cache for fake PNOR or not.
         *
         * @return Indicate state of fake PNOR
         *         true = PNOR DD is using L3 Cache for fake PNOR
         *         false = PNOR DD not using L3 Cache for fake PNOR
         */
        bool usingL3Cache( );

        /**
         * @brief Retrieve size of NOR flash
         * @return Size of PNOR in bytes
         */
        uint32_t getNorSize( void );

        /**
         * @brief Retrieve bitstring of NOR workarounds
         * @return NOR workarounds (see VendorWorkarounds in norflash.H)
         */
        uint32_t getNorWorkarounds( void );

        /**
         * @brief Constructor
         *
         * @parm i_target     Processor Target connected to PNOR
         *       NOTE: i_target can only be used after targeting is loaded
         */
        PnorDD( TARGETING::Target* i_target = NULL );

        /**
         * @brief Destructor
         */
        ~PnorDD();

    protected:

        /**
         * @brief Write data to PNOR using Mbox LPC windows
         * @pre Mutex should already be locked before calling
         *
         * @parm[in] i_addr  PNOR flash Address to write
         * @parm[in] i_size  Amount of data to write, in bytes.
         * @parm[in] i_data  Buffer containing data to write
         *
         * @return Error from operation
         */
        errlHndl_t _writeFlash( uint32_t i_addr,
                                size_t i_size,
                                const void* i_data );

        /**
         * @brief Read data from PNOR using Mbox LPC windows
         * @pre Mutex should already be locked before calling
         *
         * @parm[in] i_addr  PNOR flash Address to read
         * @parm[in] i_size  Amount of data to read, in bytes.
         * @parm[out] o_data  Buffer to read data into
         *
         * @return Error from operation
         */
        errlHndl_t _readFlash( uint32_t i_addr,
                               size_t i_size,
                               void* o_data );

        /**
         * @brief Open a window if necessary and return adjusted
             *        LPC address and chunk size
         * @parm[in]  i_isWrite  Write or read window
         * @parm[in]  i_reqAddr  Requested flash offset
         * @parm[in]  i_reqSize  Requested size
         * @parm[out] o_lpcAddr  LPC offset for the requested offset
         * @parm[out] o_chunkLen i_reqSize adjusted to fit in the window
         *
         * @return Error from operation
         */
        errlHndl_t adjustMboxWindow(bool i_isWrite,
                                    uint32_t i_reqAddr,
                                    size_t i_reqSize,
                                    uint32_t& o_lpcAddr,
                                    size_t& o_chunkLen);

        /**
         * @brief Mark a range dirty in a write window
         * @parm[in] i_addr      Flash offset of the range
         * @parm[in] i_size      Size of the range
         *
         * @return Error from operation
         */
        errlHndl_t writeDirty(uint32_t i_addr, size_t i_size);

        /**
         * @brief Flush all pending dirty data to the flash
         *
         * @return Error from operation
         */
        errlHndl_t writeFlush(void);

        /**
         * @brief Read from LPC FW space
         *
         * @parm[in] i_offset    LPC offset
         * @parm[in] i_size      Size to read
         * @parm[out] o_buf      Output buffer
         *
         * @return Error from operation
         */

        errlHndl_t readLpcFw(uint32_t i_offset, size_t i_size, void* o_buf);
        /**
         * @brief Write to LPC FW space
         *
         * @parm[in] i_offset    LPC offset
         * @parm[in] i_size      Size to read
         * @parm[in] i_buf       Input buffer
         *
         * @return Error from operation
         */
        errlHndl_t writeLpcFw(uint32_t i_offset,
                              size_t i_size,
                              const void* i_buf);

    private: // Variables

        astMbox* iv_mbox;
        uint32_t iv_protocolVersion;
        //Block size is either 4k (V1) or BMC defined (V2)
        // the iv_blockShift parm is a representation of that size
        // as a power of 2. Most command and response args are specified
        // in some multiple of block size
        uint32_t iv_blockShift;
        uint32_t iv_flashSize;
        uint32_t iv_flashEraseSize;

        // Current window
        bool     iv_curWindowOpen;      // Currently open
        bool     iv_curWindowWrite;     // Write vs Read window
        uint32_t iv_curWindowOffset;    // Offset into flash
        uint32_t iv_curWindowSize;      // Size
        uint32_t iv_curWindowLpcOffset; // Offset into LPC FW space

        // Legacy v1 protocol
        uint32_t iv_readWindowSize;
        uint32_t iv_writeWindowSize;

        /**
         * @brief Global Mutex to prevent concurrent PNOR accesses to Master
         *        Proc. This needs to be static so we can mutex across multiple
         *        instances of PnorDD
         */
        static mutex_t cv_mutex;

        /**
         * @brief Class Mutex used to prevent concurrent PNOR accesses
         */
        mutex_t iv_mutex;

        /**
         * @brief Mutex pointer to either class-specific or global mutex to
         *        prevent concurrent PNOR accesses.
         *        Each class uses a mutex; some share the static cv_mutex
         */
        mutex_t* iv_mutex_ptr;

        /**
         * @brief Processor Target used to access PNOR device
         *
         */
        TARGETING::Target* iv_target;

        // Needed for testcases
        friend class PnorDdTest;

        // let the UserDetails classes see internal structures
        friend class PNOR::UdPnorDDParms;
};

#endif /* __PNOR_MBOXDD_H */
