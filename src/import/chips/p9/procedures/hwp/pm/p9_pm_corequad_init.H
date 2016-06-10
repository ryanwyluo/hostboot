/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/hwp/pm/p9_pm_corequad_init.H $            */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2016                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file  p9_pm_corequad_init.H
/// @brief Establish safe settings for Core and Quad.
///
// *HWP HWP Owner: Greg Still <stillgs@us.ibm.com>
// *HWP FW Owner: Sangeetha T S <sangeet2@in.ibm.com>
// *HWP Team: PM
// *HWP Level: 2
// *HWP Consumed by: HS

#ifndef _P9_COREQ_IN_H_
#define _P9_COREQ_IN_H_

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <fapi2.H>
#include <p9_pm.H>
#include <p9_quad_scom_addresses.H>
#include <p9_hcd_common.H>

// function pointer typedef definition for HWP call support
typedef fapi2::ReturnCode (*p9_pm_corequad_init_FP_t)
(const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>&,
 const p9pm::PM_FLOW_MODE,
 const uint32_t,
 const uint32_t,
 const uint32_t);

// -----------------------------------------------------------------------------
// Function Prototype
// -----------------------------------------------------------------------------
extern "C"
{
///
/// @brief Perform necessary actions to Core and Quad PPMs as well as CMEs
///        for reset and initialization.
///
/// @param[in] i_target      Proc Chip target
/// @param[in] i_mode        Control mode for the procedure
///                          PM_INIT, PM_RESET
/// @param[in] i_cmeFirMask  Mask value for CME FIR
///                          (applied during PM_RESET phase)
/// @param[in] i_cppmErrMask Mask value for Core PPM Error
///                          (applied during PM_RESET phase)
/// @param[in] i_qppmErrMask Mask value for Quad PPM Error
///                          (applied during PM_RESET phase)
///
/// @return FAPI2_RC_SUCCESS on success or error return code
///
    fapi2::ReturnCode p9_pm_corequad_init(
        const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP>& i_target,
        const p9pm::PM_FLOW_MODE i_mode,
        const uint32_t i_cmeFirMask,
        const uint32_t i_cppmErrMask,
        const uint32_t i_qppmErrMask);
} // extern "C"

#endif // _P9_COREQ_IN_H_