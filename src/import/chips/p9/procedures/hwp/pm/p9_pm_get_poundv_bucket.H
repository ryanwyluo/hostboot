/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/hwp/pm/p9_pm_get_poundv_bucket.H $        */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2015,2016                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file p9_pm_get_poundv_bucket.H
/// @brief Provide structure for voltageBucketData so that info from #V keyword
/// can be stored inside of this struct.
/// Also define prototype for p9_pm_get_poundv_bucket
///

#ifndef _P9_PM_GET_POUND_V_BUCKET_H_
#define _P9_PM_GET_POUND_V_BUCKET_H_

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <fapi2.H>

namespace fapi2
{

//Represents the data contained within a single #V (version=2) bucket
//Note: This structure should not be assumed to be binary identical
//to the contents of the VPD itself.
//Pack the struct so that we can get a consistent size with no
//extra padding
typedef struct __attribute__((__packed__))  voltageBucketData
{
    // bucket Id
    uint8_t bucketId;
    // Nominal
    uint16_t nomFreq;
    uint16_t VddNomVltg;
    uint16_t IddNomCurr;
    uint16_t VcsNomVltg;
    uint16_t IcsNomCurr;
    // PowerSave
    uint16_t PSFreq;
    uint16_t VddPSVltg;
    uint16_t IddPSCurr;
    uint16_t VcsPSVltg;
    uint16_t IcsPSCurr;
    // Turbo
    uint16_t turboFreq;
    uint16_t VddTurboVltg;
    uint16_t IddTurboCurr;
    uint16_t VcsTurboVltg;
    uint16_t IcsTurboCurr;
    //Ultra Turbo
    uint16_t uTurboFreq;
    uint16_t VddUTurboVltg;
    uint16_t IddUTurboCurr;
    uint16_t VcsUTurboVltg;
    uint16_t IcsUTurboCurr;
    //PowerBus
    uint16_t pbFreq;
    uint16_t VdnPbVltg;
    uint16_t IdnPbCurr;
    uint16_t VcsPbVltg;
    uint16_t IcsPbCurr;
} voltageBucketData_t;

}

//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
///
/// @brief Read #V bucket data from LRP record based upon
///        bucket ID read off of parent proc chip. This function
///        will pass out a structured format of the #V data of correct
///        endianness.
/// @param[in] i_target EQ chiplet target
/// @param[in] o_data structured data describing the #v bucket used
/// @return FAPI2_RC_SUCCESS if success, else error code.
///
fapi2::ReturnCode p9_pm_get_poundv_bucket(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target,
    fapi2::voltageBucketData_t& o_data);


#endif // _P9_PM_GET_POUND_V_BUCKET_H_