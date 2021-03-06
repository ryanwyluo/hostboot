/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/isteps/istep13/call_mss_draminit.C $                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

//Error handling and tracing
#include <errl/errlentry.H>
#include <errl/errlmanager.H>
#include <errl/errludtarget.H>
#include <isteps/hwpisteperror.H>
#include <initservice/isteps_trace.H>
#include <initservice/initserviceif.H>
#include <plat_trace.H>

//Istep 13 framework
#include <istepHelperFuncs.H>
#include "istep13consts.H"
#include "platform_vddr.H"

// targeting support
#include <targeting/common/commontargeting.H>
#include <targeting/common/util.H>
#include <targeting/common/utilFilter.H>

// fapi2 HWP invoker
#include  <fapi2/plat_hwp_invoker.H>

//From Import Directory (EKB Repository)
#include  <fapi2.H>
#include  <p9_mss_draminit.H>

using namespace ERRORLOG;
using namespace ISTEP;
using namespace ISTEP_ERROR;
using namespace TARGETING;

namespace ISTEP_13
{

void   mss_post_draminit( IStepError & io_stepError )
{
    errlHndl_t l_err = NULL;
    bool rerun_vddr = false;

    do {

        TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
            "mss_post_draminit entry" );

        set_eff_config_attrs_helper(DEFAULT, rerun_vddr);

        if ( rerun_vddr == false )
        {
            TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                       "mss_post_draminit: nothing to do" );
            break;
        }

        // Call mss_volt_vddr_offset to recalculate VDDR voltage
        TARGETING::Target* pSysTarget = nullptr;
        TARGETING::targetService().getTopLevelTarget(pSysTarget);
        assert((pSysTarget != nullptr),
                "mss_post_draminit: Code bug!  System target was NULL.");

        // only calculate if system supports dynamic voltage
        if (pSysTarget->getAttr< TARGETING::ATTR_SUPPORTS_DYNAMIC_MEM_VOLT >()
            == 1)
        {
            // Update mss_volt_vddr_offset_millivolts attribute
            l_err = computeDynamicMemoryVoltage<
                        TARGETING::ATTR_MSS_VDDR_PROGRAM,
                        TARGETING::ATTR_VDDR_ID>();
            if(l_err)
            {
                TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                    "ERROR 0x%08X: computeDynamicMemoryVoltage for "
                    "VDDR domain",
                    l_err->reasonCode());
                io_stepError.addErrorDetails(l_err);
                errlCommit(l_err,HWPF_COMP_ID);
            }
            else
            {
                TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                 "mss_post_draminit: mss_volt_vddr_offset_millivolts "
                 "successfully updated");
            }
        }
        else
        {
            TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                "mss_post_draminit: dynamic voltage not "
                "supported on this system");
            break;
        }

        // Call HWSV to call POWR code
        // This fuction has compile-time binding for different platforms
        l_err = platform_adjust_vddr_post_dram_init();

        if( l_err )
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                      "ERROR 0x%.8X: mss_post_draminit: "
                      "platform_adjust_vddr_post_dram_init() returns error",
                      l_err->reasonCode());

            // Create IStep error log and cross reference to error that occurred
            io_stepError.addErrorDetails( l_err );

            // Commit Error
            errlCommit( l_err, HWPF_COMP_ID );
        }

    } while(0);

    TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace, "mss_post_draminit exit" );
    return;
}

void* call_mss_draminit (void *io_pArgs)
{
    errlHndl_t l_err = NULL;

    IStepError l_stepError;

    TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace, "call_mss_draminit entry" );

    // Get all MCBIST targets
    TARGETING::TargetHandleList l_mcbistTargetList;
    getAllChiplets(l_mcbistTargetList, TYPE_MCBIST);

    for (const auto & l_mcbist_target : l_mcbistTargetList)
    {
        // Dump current run on target
        TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                "Running p9_mss_draminit HWP on "
                "target HUID %.8X", TARGETING::get_huid(l_mcbist_target));

        fapi2::Target<fapi2::TARGET_TYPE_MCBIST> l_fapi_mcbist_target
            (l_mcbist_target);

        FAPI_INVOKE_HWP(l_err, p9_mss_draminit, l_fapi_mcbist_target);

        if (l_err)
        {
            TRACFCOMP(ISTEPS_TRACE::g_trac_isteps_trace,
                    "ERROR 0x%.8X : p9_mss_draminit HWP returns error",
                    l_err->reasonCode());

            // capture the target data in the elog
            ErrlUserDetailsTarget(l_mcbist_target).addToLog(l_err);

            // Create IStep error log and cross reference to error that occurred
            l_stepError.addErrorDetails( l_err );

            // Commit Error
            errlCommit( l_err, HWPF_COMP_ID );
        }
        else
        {
            TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace,
                       "SUCCESS running p9_mss_draminit HWP on "
                       "target HUID %.8X", TARGETING::get_huid(l_mcbist_target));
        }

    }   // endfor   mcbist's

    // call POST_DRAM_INIT function
    if(INITSERVICE::spBaseServicesEnabled())
    {
        mss_post_draminit(l_stepError);
    }

    TRACFCOMP( ISTEPS_TRACE::g_trac_isteps_trace, "call_mss_draminit exit" );

    return l_stepError.getErrorHandle();
}

};
