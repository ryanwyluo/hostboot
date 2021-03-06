/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/memory/lib/mcbist/mcbist.C $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

///
/// @file mcbist.C
/// @brief Run and manage the MCBIST engine
///
// *HWP HWP Owner: Brian Silver <bsilver@us.ibm.com>
// *HWP HWP Backup: Andre Marin <aamarin@us.ibm.com>
// *HWP Team: Memory
// *HWP Level: 2
// *HWP Consumed by: FSP:HB

#include <fapi2.H>
#include <lib/mcbist/mcbist.H>
#include <lib/utils/dump_regs.H>
#include <lib/workarounds/mcbist_workarounds.H>

using fapi2::TARGET_TYPE_MCBIST;
using fapi2::TARGET_TYPE_MCA;
using fapi2::TARGET_TYPE_MCS;

namespace mss
{

const std::pair<uint64_t, uint64_t> mcbistTraits<fapi2::TARGET_TYPE_MCBIST>::address_pairs[] =
{
    { START_ADDRESS_0, END_ADDRESS_0 },
    { START_ADDRESS_1, END_ADDRESS_1 },
    { START_ADDRESS_2, END_ADDRESS_2 },
    { START_ADDRESS_3, END_ADDRESS_3 },
};

const std::vector< mss::mcbist::op_type > mcbistTraits<fapi2::TARGET_TYPE_MCBIST>::FIFO_MODE_REQUIRED_OP_TYPES =
{
    mss::mcbist::op_type::WRITE            ,
    mss::mcbist::op_type::READ             ,
    mss::mcbist::op_type::READ_WRITE       ,
    mss::mcbist::op_type::WRITE_READ       ,
    mss::mcbist::op_type::READ_WRITE_READ  ,
    mss::mcbist::op_type::READ_WRITE_WRITE ,
    mss::mcbist::op_type::RAND_SEQ         ,
    mss::mcbist::op_type::READ_READ_WRITE  ,
};

namespace mcbist
{

///
/// @brief Load MCBIST maint pattern given a pattern
/// @tparam T, the fapi2::TargetType - derived
/// @tparam TT, the mcbistTraits associated with T - derived
/// @param[in] i_target the target to effect
/// @param[in] i_pattern an mcbist::patterns
/// @param[in] i_invert whether to invert the pattern or not
/// @note this overload disappears when we have real patterns.
/// @return FAPI2_RC_SUCCSS iff ok
///
template< >
fapi2::ReturnCode load_maint_pattern( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target, const pattern& i_pattern,
                                      const bool i_invert )
{
    // Init the fapi2 return code
    fapi2::current_err = fapi2::FAPI2_RC_SUCCESS;

    // Array access control
    fapi2::buffer<uint64_t> l_aacr;
    // Array access data
    fapi2::buffer<uint64_t> l_aadr;

    // first we must setup the access control register
    // Setup the array address
    // enable the auto increment bit
    // set ecc mode bit on
    l_aacr
    .writeBit<MCA_WREITE_AACR_BUFFER>(mss::states::OFF)
    .insertFromRight<MCA_WREITE_AACR_ADDRESS, MCA_WREITE_AACR_ADDRESS_LEN>(mss::mcbist::rmw_address::DW0)
    .writeBit<MCA_WREITE_AACR_AUTOINC>(mss::states::ON)
    .writeBit<MCA_WREITE_AACR_ECCGEN>(mss::states::ON);

    // This loop will be run twice to write the pattern twice.  Once per 64B write.
    // When MCBIST maint mode is in 64B mode it will only use the first 64B when in 128B mode
    // MCBIST maint will use all 128B (it will perform two consecutive writes)
    const auto l_ports =  mss::find_targets<fapi2::TARGET_TYPE_MCA>(i_target);
    // Init the port map

    for (const auto& p : l_ports)
    {
        l_aacr.insertFromRight<MCA_WREITE_AACR_ADDRESS, MCA_WREITE_AACR_ADDRESS_LEN>(mss::mcbist::rmw_address::DW0);

        for (auto l_num_reads = 0; l_num_reads < 2; ++l_num_reads)
        {
            FAPI_INF("Setting the array access control register.");
            FAPI_TRY( mss::putScom(p, MCA_WREITE_AACR, l_aacr) );

            for (const auto& l_cache_line : i_pattern)
            {
                fapi2::buffer<uint64_t> l_value_first  = i_invert ? ~l_cache_line.first : l_cache_line.first;
                fapi2::buffer<uint64_t> l_value_second = i_invert ? ~l_cache_line.second : l_cache_line.second;
                FAPI_INF("Loading cache line pattern 0x%016lx 0x%016lx", l_value_first, l_value_second);
                FAPI_TRY( mss::putScom(p, MCA_AADR, l_value_first) );

                // In order for the data to actually be written into the RMW buffer, we must issue a putscom to the MCA_AAER register
                // This register is used for the ECC, we will just write all zero to this register.  The ECC will be auto generated
                // when the aacr MCA_WREITE_AACR_ECCGEN bit is set
                FAPI_TRY( mss::putScom(p, MCA_AAER, 0) );

                // No need to increment the address because the logic does it automatically when MCA_WREITE_AACR_AUTOINC is set
                FAPI_TRY( mss::putScom(p, MCA_AADR, l_value_second) );

                // In order for the data to actually be written into the RMW buffer, we must issue a putscom to the MCA_AAER register
                // This register is used for the ECC, we will just write all zero to this register.  The ECC will be auto generated
                // when the aacr MCA_WREITE_AACR_ECCGEN bit is set
                FAPI_TRY( mss::putScom(p, MCA_AAER, 0) );
            }

            l_aacr.insertFromRight<MCA_WREITE_AACR_ADDRESS, MCA_WREITE_AACR_ADDRESS_LEN>(mss::mcbist::rmw_address::DW8);
        }
    }


fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Load MCBIST data compare mask registers
/// @tparam T, the fapi2::TargetType - derived
/// @tparam TT, the mcbistTraits associated with T - derived
/// @param[in] i_target the target to effect
/// @param[in] i_program the mcbist::program
/// @return FAPI2_RC_SUCCSS iff ok
///
template< >
fapi2::ReturnCode load_data_compare_mask( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target,
        const mcbist::program<TARGET_TYPE_MCBIST>& i_program )
{
    typedef mcbistTraits<TARGET_TYPE_MCBIST> TT;

    // Init the fapi2 return code
    fapi2::current_err = fapi2::FAPI2_RC_SUCCESS;

    // Load the MCBCM Data compare masks

    const auto l_ports =  mss::find_targets<fapi2::TARGET_TYPE_MCA>(i_target);
    FAPI_INF("Loading the MCBIST data compare mask registers!");

    for (const auto& p : l_ports)
    {
        FAPI_TRY( mss::putScom(p, TT::COMPARE_MASK, i_program.iv_compare_mask) );
    }

fapi_try_exit:
    return fapi2::current_err;

}

///
/// @brief Load MCBIST 24b random data seeds given a pattern index
/// @tparam T, the fapi2::TargetType - derived
/// @tparam TT, the mcbistTraits associated with T - derived
/// @param[in] i_target the target to effect
/// @param[in] i_random24_data_seed mcbist::random24_data_seed
/// @param[in] i_random24_map mcbist::random24_seed_map
/// @param[in] i_invert whether to invert the pattern or not
/// @note this overload disappears when we have real patterns.
/// @return FAPI2_RC_SUCCSS iff ok
///
template< >
fapi2::ReturnCode load_random24b_seeds( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target,
                                        const random24_data_seed& i_random24_data_seed,
                                        const random24_seed_map& i_random24_map,
                                        const bool i_invert )
{
    // Init the fapi2 return code
    fapi2::current_err = fapi2::FAPI2_RC_SUCCESS;

    const uint64_t l_random_addr0 = MCBIST_MCBRDS0Q;
    const uint64_t l_random_addr1 = MCBIST_MCBRDS1Q;
    uint64_t l_index = 0;
    uint64_t l_map_index = 0;
    uint64_t l_map_offset = 0;

    fapi2::buffer<uint64_t> l_mcbrsd0q;
    fapi2::buffer<uint64_t> l_mcbrsd1q;


    // We are going to loop through the random seeds and load them into the random seed registers
    // Because the 24b random data seeds share the same registers as the 24b random data byte LFSR maps
    // we will load those as well

    for (const auto& l_seed : i_random24_data_seed)
    {
        FAPI_INF("Loading 24b random seed index %ld ", l_index);
        fapi2::buffer<uint64_t> l_value  = i_invert ? ~l_seed : l_seed;

        // Print an informational message to indicate if a random seed is 0
        // TK Do we want an error here? 0 may be used on purpose to hold a byte at all 0 on purpose
        if ( l_value == 0 )
        {
            FAPI_INF("Warning: Random 24b data seed is set to 0 for seed index %d", l_index);
        }

        // If we are processing the first 24b random data seed we will add it to the fapi buffer
        // we won't load it yet because the second 24b seed will be loaded into the same register
        if ( l_index == 0 )
        {
            l_mcbrsd0q.insertFromRight<MCBIST_MCBRDS0Q_DGEN_RNDD_SEED0, MCBIST_MCBRDS0Q_DGEN_RNDD_SEED0_LEN>(l_value);
        }
        // The second 24b random data seed is loaded into the same register as the first seed
        // therefore we will add the second seed tothe fapi buffer and then issue the putscom
        else if (l_index == 1 )
        {
            l_mcbrsd0q.insertFromRight<MCBIST_MCBRDS0Q_DGEN_RNDD_SEED1, MCBIST_MCBRDS0Q_DGEN_RNDD_SEED1_LEN>(l_value);
            FAPI_INF("Loading 24b random seeds 0 and 1 0x%016lx ", l_mcbrsd0q);
            FAPI_TRY( mss::putScom(i_target, l_random_addr0, l_mcbrsd0q) );
        }
        // The third 24b random data seed occupies the same register as the random data byte maps.  Therefore we first
        // add the third random 24b data seed to the register and then loop through all of the byte mappings a total of
        // 9.  ach of the byte mappings associates a byte of the random data to a byte in the 24b random data LFSRs
        // Each byte map is offset by 4 bits in the register.
        else
        {
            l_mcbrsd1q.insertFromRight<MCBIST_MCBRDS1Q_DGEN_RNDD_SEED2, MCBIST_MCBRDS1Q_DGEN_RNDD_SEED2_LEN>(l_value);

            for (const auto& l_map : i_random24_map)
            {
                l_map_offset = MCBIST_MCBRDS1Q_DGEN_RNDD_DATA_MAPPING + (l_map_index * RANDOM24_SEED_MAP_FIELD_LEN);
                l_mcbrsd1q.insertFromRight(l_map, l_map_offset, RANDOM24_SEED_MAP_FIELD_LEN);
                FAPI_INF("Loading 24b random seed map index %ld ", l_map_index);
                FAPI_ASSERT( l_map_index < mss::mcbist::MAX_NUM_RANDOM24_MAPS,
                             fapi2::MSS_MEMDIAGS_INVALID_PATTERN_INDEX().set_INDEX(l_map_index),
                             "Attempting to load a 24b random data seed map which does not exist %d", l_map_index );
                ++l_map_index;
            }

            FAPI_TRY( mss::putScom(i_target, l_random_addr1, l_mcbrsd1q) );
        }

        FAPI_ASSERT( l_index < MAX_NUM_RANDOM24_SEEDS,
                     fapi2::MSS_MEMDIAGS_INVALID_PATTERN_INDEX().set_INDEX(l_index),
                     "Attempting to load a 24b random data seed which does not exist %d", l_index );
        ++l_index;
    }

fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Load a set of MCBIST subtests in to the MCBIST registers
/// @tparam T, the fapi2::TargetType - derived
/// @tparam TT, the mcbistTraits associated with T - derived
/// @param[in] the target to effect
/// @param[in] the mcbist::program
/// @return FAPI2_RC_SUCCSS iff ok
/// @note assumes the MCBIST engine has been configured.
///
template<>
fapi2::ReturnCode load_mcbmr( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target,
                              const mcbist::program<TARGET_TYPE_MCBIST>& i_program )
{

    // Leave if there are no subtests.
    if (0 == i_program.iv_subtests.size())
    {
        FAPI_INF("no subtests, nothing to do");
        return fapi2::current_err;
    }

    // List of the 8 MCBIST registers - each holds 4 subtests.
    static const std::vector< uint64_t > l_memory_registers =
    {
        MCBIST_MCBMR0Q, MCBIST_MCBMR1Q, MCBIST_MCBMR2Q, MCBIST_MCBMR3Q,
        MCBIST_MCBMR4Q, MCBIST_MCBMR5Q, MCBIST_MCBMR6Q, MCBIST_MCBMR7Q,
    };

    std::vector< uint64_t > l_memory_register_buffers =
    {
        0, 0, 0, 0, 0, 0, 0, 0,
    };

    static const size_t SUBTEST_PER_REG = 4;
    static const size_t SUBTEST_PER_PROGRAM = 32;

    static const auto BITS_IN_SUBTEST = sizeof(mcbist::subtest_t<TARGET_TYPE_MCBIST>().iv_mcbmr) * 8;
    static const auto LEFT_SHIFT = (sizeof(uint64_t) * 8) - BITS_IN_SUBTEST;

    ssize_t l_bin = -1;
    size_t l_register_shift = 0;

    // We'll shift this in to position to indicate which subtest is the last
    static const uint64_t l_done_bit( 0x8000000000000000 >> MCBIST_MCBMR0Q_MCBIST_CFG_TEST00_DONE );

    // TK: For now limit MCBIST programs to 32 subtests.
    const auto l_program_size = i_program.iv_subtests.size();
    FAPI_ASSERT( l_program_size <= SUBTEST_PER_PROGRAM,
                 fapi2::MSS_MCBIST_PROGRAM_TOO_BIG().set_PROGRAM_LENGTH(l_program_size),
                 "mcbist program of length %d exceeds arbitrary maximum of %d", l_program_size, SUBTEST_PER_PROGRAM );

    // Distribute the program over the 8 MCBIST subtest registers
    // We need the index, so increment thru i_program.iv_subtests.size()
    for (size_t l_index = 0; l_index < l_program_size; ++l_index)
    {
        l_bin = (l_index % SUBTEST_PER_REG) == 0 ? l_bin + 1 : l_bin;
        l_register_shift = (l_index % SUBTEST_PER_REG) * BITS_IN_SUBTEST;

        l_memory_register_buffers[l_bin] |=
            (uint64_t(i_program.iv_subtests[l_index].iv_mcbmr) << LEFT_SHIFT) >> l_register_shift;

        FAPI_DBG("putting subtest %d (0x%x) in MCBMR%dQ shifted %d 0x%016llx",
                 l_index, i_program.iv_subtests[l_index].iv_mcbmr, l_bin,
                 l_register_shift, l_memory_register_buffers[l_bin]);
    }

    // l_bin and l_register_shift are the values for the last subtest we'll tell the MCBIST about.
    // We need to set that subtest's done-bit so the MCBIST knows it's the end of the line
    l_memory_register_buffers[l_bin] |= l_done_bit >> l_register_shift;
    FAPI_DBG("setting MCBMR%dQ subtest %llu as the last subtest 0x%016llx",
             l_bin, l_register_shift, l_memory_register_buffers[l_bin]);

    // ... and slam the values in to the registers.
    // Could just decrement l_bin, but that scoms the subtests in backwards and is confusing
    for (auto l_index = 0; l_index <= l_bin; ++l_index)
    {
        FAPI_TRY( mss::putScom(i_target, l_memory_registers[l_index], l_memory_register_buffers[l_index]) );
    }

fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Poll the mcbist engine and check for errors
/// @tparam T the fapi2::TargetType - derived
/// @tparam TT the mcbistTraits associated with T - derived
/// @param[in] i_target the target to effect
/// @param[in] i_program, the mcbist program which is executing
/// @return fapi2::ReturnCode, FAPI2_RC_SUCCESS iff OK
///
template< fapi2::TargetType T, typename TT = mcbistTraits<T> >
fapi2::ReturnCode poll( const fapi2::Target<T>& i_target, const program<T>& i_program )
{
    fapi2::buffer<uint64_t> l_status;

    static const uint64_t l_done = fapi2::buffer<uint64_t>().setBit<TT::MCBIST_DONE>();
    static const uint64_t l_fail = fapi2::buffer<uint64_t>().setBit<TT::MCBIST_FAIL>();
    static const uint64_t l_in_progress = fapi2::buffer<uint64_t>().setBit<TT::MCBIST_IN_PROGRESS>();

    // A small vector of addresses to poll during the polling loop
    const std::vector<mss::poll_probe<fapi2::TARGET_TYPE_MCBIST>> l_probes =
    {
        {i_target, "mcbist current address", MCBIST_MCBMCATQ},
    };

    mss::poll(i_target, TT::STATQ_REG, i_program.iv_poll,
              [&l_status](const size_t poll_remaining, const fapi2::buffer<uint64_t>& stat_reg) -> bool
    {
        FAPI_DBG("mcbist statq 0x%llx, remaining: %d", stat_reg, poll_remaining);
        l_status = stat_reg;
        return l_status.getBit<TT::MCBIST_IN_PROGRESS>() != 1;
    },
    l_probes);

    // Check to see if we're still in progress - meaning we timed out.
    FAPI_ASSERT((l_status & l_in_progress) != l_in_progress,
                fapi2::MSS_MCBIST_TIMEOUT().set_TARGET_IN_ERROR(i_target),
                "MCBIST timed out %s", mss::c_str(i_target));

    // The control register has a bit for done-and-happy and a bit for done-and-unhappy
    if ( ((l_status & l_done) == l_done) || ((l_status & l_fail) == l_fail) )
    {
        FAPI_INF("MCBIST completed, processing errors");

        // We're done. It doesn't mean that there were no errors.
        FAPI_TRY( i_program.process_errors(i_target) );

        // If we're here there were no errors, but lets report if the fail bit was set anyway.
        FAPI_ASSERT( (l_status & l_fail) != l_fail,
                     fapi2::MSS_MCBIST_UNKNOWN_FAILURE()
                     .set_TARGET_IN_ERROR(i_target)
                     .set_STATUS_REGISTER(l_status),
                     "MCBIST reported a fail, but process_errors didn't find it 0x%016llx", l_status );

        // And if we're here all is good with the world.
        return fapi2::current_err;
    }

    FAPI_ASSERT(false,
                fapi2::MSS_MCBIST_MULTIPLE_FAIL_BITS()
                .set_TARGET_IN_ERROR(i_target)
                .set_STATUS_REGISTER(l_status),
                "MCBIST executed <shrug>. Something's not good 0x%016llx", l_status );

fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Execute the mcbist program
/// @param[in] i_target the target to effect
/// @param[in] i_program, the mcbist program to execute
/// @return fapi2::ReturnCode, FAPI2_RC_SUCCESS iff OK
///
template<>
fapi2::ReturnCode execute( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target,
                           const program<TARGET_TYPE_MCBIST>& i_program )
{
    typedef mcbistTraits<TARGET_TYPE_MCBIST> TT;

    fapi2::buffer<uint64_t> l_status;
    bool l_poll_result = false;
    poll_parameters l_poll_parameters;

    // Before we go off into the bushes, lets see if there are any instructions in the
    // program. If not, we can save everyone the hassle
    FAPI_ASSERT(0 != i_program.iv_subtests.size(),
                fapi2::MSS_MEMDIAGS_NO_MCBIST_SUBTESTS().set_TARGET(i_target),
                "Attempt to run an MCBIST program with no subtests on %s", mss::c_str(i_target));

    // Implement any mcbist work-arounds.
    // I'm going to do the unthinkable here - and cast away the const of the mcbist program input.
    // The work arounds need to change this, and so it needs to not be const. However, I don't want
    // to risk general const-correctness by changing the input parameter to non-const. So, I use
    // const_cast<> (ducks out of the way of the flying adjectives.) These are work-arounds ...
    FAPI_TRY( workarounds::mcbist::end_of_rank(i_target, const_cast<program<TARGET_TYPE_MCBIST>&>(i_program)) );

    FAPI_TRY( clear_errors(i_target) );

    // Configures the write/read FIFO bit
    FAPI_TRY( load_fifo_mode( i_target, i_program) );

    // Slam the address generator config
    FAPI_TRY( load_addr_gen(i_target, i_program) );

    // Slam the parameters in to the mcbist parameter register
    FAPI_TRY( load_mcbparm(i_target, i_program) );

    // Slam the configured address maps down
    FAPI_TRY( load_mcbamr( i_target, i_program) );

    // Slam the config register down
    FAPI_TRY( load_config( i_target, i_program) );

    // Slam the control register down
    FAPI_TRY( load_control( i_target, i_program) );

    // Load the patterns and any associated bits for random, etc
    FAPI_TRY( load_data_config( i_target, i_program) );

    // Load the thresholds
    FAPI_TRY( load_thresholds( i_target, i_program) );

    // Slam the subtests in to the mcbist registers
    // Always do this last so the action file triggers see the other bits set
    FAPI_TRY( load_mcbmr(i_target, i_program) );

    // Start the engine, and then poll for completion
    FAPI_TRY(start_stop(i_target, mss::START));

    // Verify that the in-progress bit has been set, so we know we started
    // Don't use the program's poll as it could be a very long time. Use the default poll.
    l_poll_result = mss::poll(i_target, TT::STATQ_REG, l_poll_parameters,
                              [&l_status](const size_t poll_remaining, const fapi2::buffer<uint64_t>& stat_reg) -> bool
    {
        FAPI_DBG("looking for mcbist start, mcbist statq 0x%llx, remaining: %d", stat_reg, poll_remaining);
        l_status = stat_reg;
        // We're done polling when either we see we're in progress or we see we're done.
        return (l_status.getBit<TT::MCBIST_IN_PROGRESS>() == true) || (l_status.getBit<TT::MCBIST_DONE>() == true);
    });

    // So we've either run/are running or we timed out waiting for the start.
    FAPI_ASSERT( l_poll_result == true,
                 fapi2::MSS_MEMDIAGS_MCBIST_FAILED_TO_START().set_TARGET(i_target),
                 "The MCBIST engine failed to start its program" );

    // If the user asked for async mode, we can leave. Otherwise, poll and check for errors
    if (!i_program.iv_async)
    {
        return mcbist::poll(i_target, i_program);
    }

fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Get a list of ports involved in the program
/// Specialization for program<fapi2::TARGET_TYPE_MCBIST>
/// @param[in] i_target the target for this program
/// @return vector of port targets
///
template<>
template<>
std::vector<fapi2::Target<fapi2::TARGET_TYPE_MCA>>
        program<fapi2::TARGET_TYPE_MCBIST>::get_port_list<fapi2::TARGET_TYPE_MCA>( const
                fapi2::Target<fapi2::TARGET_TYPE_MCBIST>& i_target )
{
    typedef mss::mcbistTraits<TARGET_TYPE_MCBIST> TT;

    std::vector<fapi2::Target<fapi2::TARGET_TYPE_MCA>> l_list;

    fapi2::buffer<uint64_t> l_ports_selected;
    // extract port sel to left of l_ports_selected so port relatve pos maps to bit number
    iv_control.extract<TT::PORT_SEL, TT::PORT_SEL_LEN>(l_ports_selected);

    for (const auto& p : find_targets<fapi2::TARGET_TYPE_MCA>(i_target))
    {
        if (l_ports_selected.getBit(mss::relative_pos<fapi2::TARGET_TYPE_MCBIST>(p)))
        {
            l_list.push_back(p);
        }
    }

    return l_list;
}

///
/// @brief Read entries from MCBIST Read Modify Write (RMW) array
/// Specialization for fapi2::TARGET_TYPE_MCA
/// @param[in] i_target the target to effect
/// @param[in] i_start_addr the array address to read first
/// @param[in] i_num_entries the number of array entries to read
/// @param[in] i_roll_over_for_compare_mode set to true if only using first
/// NUM_COMPARE_INFO_ENTRIES of array, so array address rolls over at correct value
/// @param[out] o_data vector of output data
/// @param[out] o_ecc_data vector of ecc data
/// @return FAPI2_RC_SUCCSS iff ok
/// @note The number of entries in the array depends on i_roll_over_for_compare_mode parameter:
/// (NUM_COMPARE_LOG_ENTRIES for false, NUM_COMPARE_INFO_ENTRIES for true) but user may read more than
/// that since reads work in a circular buffer fashion
///
template<>
fapi2::ReturnCode read_rmw_array(const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target,
                                 const uint64_t i_start_addr,
                                 const uint64_t i_num_entries,
                                 const bool i_roll_over_for_compare_mode,
                                 std::vector< fapi2::buffer<uint64_t> >& o_data,
                                 std::vector< fapi2::buffer<uint64_t> >& o_ecc_data)
{
    typedef mcbistTraits<fapi2::TARGET_TYPE_MCA> TT;

    fapi2::buffer<uint64_t> l_control_value;
    fapi2::buffer<uint64_t> l_data;
    uint64_t l_array_addr = i_start_addr;

    // Clear out any stale values from output vectors
    o_data.clear();
    o_ecc_data.clear();

    // Set the control register for the RMW array
    l_control_value.writeBit<TT::RMW_WRT_BUFFER_SEL>(TT::SELECT_RMW_BUFFER)
    // set start address
    .insertFromRight<TT::RMW_WRT_ADDRESS, TT::RMW_WRT_ADDRESS_LEN>(l_array_addr)
    // enable the auto increment bit
    .setBit<TT::RMW_WRT_AUTOINC>()
    // set ecc mode bit off
    .clearBit<TT::RMW_WRT_ECCGEN>();

    FAPI_INF("Setting the RMW array access control register.");
    FAPI_TRY( mss::putScom(i_target, TT::RMW_WRT_BUF_CTL_REG, l_control_value) );

    for (uint8_t l_index = 0; l_index < i_num_entries; ++l_index)
    {
        // Read info out of RMW array and put into output vector
        // Note that since we enabled AUTOINC above, reading ECC_REG will increment
        // the array pointer so the next DATA_REG read will read the next array entry
        FAPI_TRY( mss::getScom(i_target, TT::RMW_WRT_BUF_DATA_REG, l_data) );

        FAPI_INF("RMW data index %d is: %016lx", l_array_addr, l_data);
        o_data.push_back(l_data);

        // Need to read ecc register to increment array index
        FAPI_TRY( mss::getScom(i_target, TT::RMW_WRT_BUF_ECC_REG, l_data) );
        o_ecc_data.push_back(l_data);
        ++l_array_addr;

        // Need to manually roll over address if we go beyond NUM_COMPARE_INFO_ENTRIES
        // Since actual array is bigger than what is used for compare mode
        if (i_roll_over_for_compare_mode &&
            (l_array_addr >= TT::NUM_COMPARE_INFO_ENTRIES))
        {
            FAPI_INF("Rolling over the RMW array access control register address from %d to %d.", (i_start_addr + l_index), 0);
            l_control_value.clearBit<TT::RMW_WRT_ADDRESS, TT::RMW_WRT_ADDRESS_LEN>();
            FAPI_TRY( mss::putScom(i_target, TT::RMW_WRT_BUF_CTL_REG, l_control_value) );
            l_array_addr = 0;
        }
    }

fapi_try_exit:
    return fapi2::current_err;
}

///
/// @brief Read entries from MCBIST Read Buffer (RB) array
/// Specialization for fapi2::TARGET_TYPE_MCA
/// @param[in] i_target the target to effect
/// @param[in] i_start_addr the array address to read first
/// @param[in] i_num_entries the number of array entries to read
/// @param[out] o_data vector of output data
/// @param[out] o_ecc_data vector of ecc data
/// @return FAPI2_RC_SUCCSS iff ok
///
template<>
fapi2::ReturnCode read_rb_array(const fapi2::Target<fapi2::TARGET_TYPE_MCA>& i_target,
                                const uint64_t i_start_addr,
                                const uint64_t i_num_entries,
                                std::vector< fapi2::buffer<uint64_t> >& o_data,
                                std::vector< fapi2::buffer<uint64_t> >& o_ecc_data)
{
    typedef mcbistTraits<fapi2::TARGET_TYPE_MCS> TT;

    fapi2::buffer<uint64_t> l_control_value;
    fapi2::buffer<uint64_t> l_data;
    uint64_t l_array_addr = i_start_addr;

    const auto& l_mcs = mss::find_target<TARGET_TYPE_MCS>(i_target);

    // Clear out any stale values from output vectors
    o_data.clear();
    o_ecc_data.clear();

    // set BUFFER according to port position within MCS
    l_control_value.writeBit<TT::RB_BUFFER_SEL>(mss::relative_pos<fapi2::TARGET_TYPE_MCS>(i_target))
    // set start address
    .insertFromRight<TT::RB_ADDRESS, TT::RB_ADDRESS_LEN>(l_array_addr)
    // enable the auto increment bit
    .setBit<TT::RB_AUTOINC>();

    FAPI_INF("Setting the RB array access control register.");
    FAPI_TRY( mss::putScom(l_mcs, TT::RD_BUF_CTL_REG, l_control_value) );

    for (uint8_t l_index = 0; l_index < i_num_entries; ++l_index)
    {
        // Note that since we enabled AUTOINC above, reading ECC_REG will increment
        // the array pointer so the next DATA_REG read will read the next array entry
        FAPI_TRY( mss::getScom(l_mcs, TT::RD_BUF_DATA_REG, l_data) );
        FAPI_INF("RB data index %d is: 0x%016lx", l_array_addr, l_data);
        o_data.push_back(l_data);

        // Need to read ecc register to increment array index
        FAPI_TRY( mss::getScom(l_mcs, TT::RD_BUF_ECC_REG, l_data) );
        o_ecc_data.push_back(l_data);
        ++l_array_addr;

        // Array address automatically rolls over if we go beyond NUM_COMPARE_LOG_ENTRIES
        if (l_array_addr >= TT::NUM_COMPARE_LOG_ENTRIES)
        {
            l_array_addr = 0;
        }

    }

fapi_try_exit:
    return fapi2::current_err;
}

} // namespace MCBIST

// Note: outside of the mcbist namespace

///
/// @brief Dump the registers of an mcbist
/// @param[in] i_target, the mcbist in question
/// @return fapi2::FAPI2_RC_SUCCESS if ok
///
template<>
fapi2::ReturnCode dump_regs( const fapi2::Target<TARGET_TYPE_MCBIST>& i_target )
{
    return fapi2::current_err;
}


} // namespace
