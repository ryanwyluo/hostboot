/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/attn/common/attntarget.H $                       */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __ATTN_ATTNTARGET_H
#define __ATTN_ATTNTARGET_H

/**
 * @file attntarget.H
 *
 * @brief HBATTN Target service wrapper class definitions.
 */

#include "common/attnfwd.H"
#include <map>
#include <targeting/common/targetservice.H>


namespace ATTN
{

/**
 * @brief TargetService Hold the installed target service implementation.
 */
class TargetService
{
    public:

        /**
         * @brief getMcsList Obtain the list of MCS unit targets
         *           for the given proc chip target.
         *
         * @param[in] i_proc The proc chip target for which a
         *              list of MCS unit targets should be obtained.
         * @param[out] o_list The populated list of MCS unit targets.
         */
        void getMcsList(
                TARGETING::TargetHandle_t i_proc,
                TARGETING::TargetHandleList & o_list);

        /**
         * @brief getProc Find the requested membuf chip
         *           target parent proc chip target.
         *
         * @param[in] i_membuf The membuf chip target
         *             for which the parent proc chip target
         *             should be found.
         *
         * @retval[0] Did not find any associated proc chip target.
         * @retval[!0] Associated proc chip parent target.
         */
        TARGETING::TargetHandle_t getProc(
                TARGETING::TargetHandle_t i_membuf);

        /**
         * @brief getAllChips Get a list of chips.
         *
         * @param[out] o_list The populated list of chips.
         * @param[in] i_type The type of chip for which a list
         *              should be obtained.
         * @param[in] i_functional functional chip filter.
         */
        void getAllChips(
                TARGETING::TargetHandleList & o_list,
                TARGETING::TYPE i_type,
                bool i_functional = true);

        /**
         * @brief getMcs Find the requested membuf chip
         *           target parent MCS unit target.
         *
         * @param[in] i_membuf The membuf chip target
         *             for which the parent MCS unit target
         *             should be found.
         *
         * @retval[0] Did not find any associated MCS unit target.
         * @retval[!0] Associated MCS unit parent target.
         */
        TARGETING::TargetHandle_t getMcs(
                TARGETING::TargetHandle_t i_membuf);

        /**
         * @brief getMcs Find the MCS unit target given
         *           the proc chip parent target and MCS position.
         *
         * @param[in] i_proc The proc chip target for which the
         *                MCS unit target should be found.
         * @param[in] i_pos The MCS unit position for which the MCS
         *                unit target should be found.
         *
         * @retval[0] Did not find any associated MCS unit target.
         * @retval[!0] Associated MCS unit target.
         */
        TARGETING::TargetHandle_t getMcs(
                TARGETING::TargetHandle_t i_proc,
                uint64_t i_pos);

        /**
         * @brief getMcsPos Find the MCS unit position given
         *           the MCS unit target.
         *
         * @param[in] i_mcs The MCS unit target for which the
         *                MCS unit position should be found.
         * @param[out] o_pos The MCS unit position.
         */
        void getMcsPos(
                TARGETING::TargetHandle_t i_mcs,
                uint64_t & o_pos);

        /**
         * @brief getMembuf Find the requested MCS unit target
         *           child membuf chip target.
         *
         * @param[in] i_mcs The MCS unit target
         *             for which the child membuf chip target
         *             should be found.
         *
         * @retval[0] Did not find any associated membuf chip target.
         * @retval[!0] Associated membuf chip target.
         */
        TARGETING::TargetHandle_t getMembuf(
                TARGETING::TargetHandle_t i_mcs);

        /**
         * @brief getType Obtain the type of the provided target.
         *
         * @param[in] i_target The target for which the type
         *                should be obtained.
         *
         * @return The type of the provided target.
         */
        TARGETING::TYPE getType(
                TARGETING::TargetHandle_t i_target);

        /**
         * @brief getAttribute Obtain the attribute for the requested target.
         *
         * @param[in] i_attribute The attribute to be obtained.
         * @param[in] i_target The target for which the attribute
         *                should be obtained.
         * @param[out] o_val The value of the attribute.
         *
         * @retval[true] The attribute was found.
         * @retval[false] The attribute was not found.
         */
        bool getAttribute(
                TARGETING::ATTRIBUTE_ID i_attribute,
                TARGETING::TargetHandle_t i_target,
                uint64_t & o_val);

        /**
         *  @brief Returns the master processor chip target handle
         *
         *  Returns the master processor chip target handle.  On systems
         *  without an alternate master, it returns a handle to the only master,
         *  if found (NULL otherwise).  On systems with multiple potential
         *  masters, it returns a target handle to the acting master (NULL
         *  otherwise).  If targeting information is not yet accessible (very
         *  early in the host boot IPL), the returned target handle will be a
         *  sentinel value representing the master (whichever it may be)
         *
         *  @param[out] o_masterProcChipTarget Target handle referring to the
         *      current master processor chip (the one connected to PNOR)
         *  @param[in]  i_pNodeTarget
         *      Handle to the node target to search for the acting master
         *      processor.  If NULL, HB will search whatever node it's
         *      running on, and FSP will search the physical drawer containing
         *      the lowest ordered fabric node ID.         *
         *  @pre Target Service must be initialized
         *
         *  @post Master processor chip target returned or NULL is returned if
         *  targetservice is not yet initialized or user passed an invalid node
         *  target to get the master proc handle
         */
        void masterProcChipTargetHandle(
            TARGETING::Target*& o_masterProcChipTargetHandle,
            const TARGETING::Target* i_pNodeTarget = NULL) const;

        /**
         * @brief setImpl Set the active target service implementation.
         *
         * @param[in] i_impl The target service implementation to make active.
         */
        void setImpl(TargetServiceImpl & i_impl);

        /**
         * @brief ctor
         */
        TargetService();

    private:

        /**
         * @brief iv_impl The active targete service implementation.
         */
        TargetServiceImpl * iv_impl;

        /**
         * @brief copy Disabled.
         */
        TargetService(const TargetService &);

        /**
         * @brief assignment Disabled.
         */
        TargetService & operator=(const TargetService &);

        /**
         * @brief Give access to the default implementation.
         */
        friend class TargetServiceImpl;
};

/**
 * @brief TargetServiceImpl Target Service
 *           implementation interface requirement.
 *
 * Default implementation forwards calls to
 * the real target service.
 */
class TargetServiceImpl
{
    public:

        /**
         * @brief getMcsList Obtain the list of MCS unit targets
         *           for the given proc chip target.
         *
         * @param[in] i_proc The proc chip target for which a
         *              list of MCS unit targets should be obtained.
         * @param[out] o_list The populated list of MCS unit targets.
         */
        virtual void getMcsList(
                TARGETING::TargetHandle_t i_proc,
                TARGETING::TargetHandleList & o_list);

       /**
         * @brief getProc Find the requested membuf chip
         *           target parent proc chip target.
         *
         * @param[in] i_membuf The membuf chip target
         *             for which the parent proc chip target
         *             should be found.
         *
         * @retval[0] Did not find any associated proc chip target.
         * @retval[!0] Associated proc chip parent target.
         */
        virtual TARGETING::TargetHandle_t getProc(
                TARGETING::TargetHandle_t i_membuf);

        /**
         * @brief getMcs Find the requested membuf chip
         *           target parent MCS unit target.
         *
         * @param[in] i_membuf The membuf chip target
         *             for which the parent MCS unit target
         *             should be found.
         *
         * @retval[0] Did not find any associated MCS unit target.
         * @retval[!0] Associated MCS unit parent target.
         */
        virtual TARGETING::TargetHandle_t getMcs(
                TARGETING::TargetHandle_t i_membuf);

        /**
         * @brief getAllChips Get a list of chips.
         *
         * @param[out] o_list The populated list of chips.
         * @param[in] i_type The type of chip for which a list
         *              should be obtained.
         * @param[in] i_functional functional chip filter.
         */
        virtual void getAllChips(
                TARGETING::TargetHandleList & o_list,
                TARGETING::TYPE i_type,
                bool i_functional = true);

        /**
         * @brief getMcs Find the MCS unit target given
         *           the proc chip parent target and MCS position.
         *
         * @param[in] i_proc The proc chip target for which the
         *                MCS unit target should be found.
         * @param[in] i_pos The MCS unit position for which the MCS
         *                unit target should be found.
         *
         * @retval[0] Did not find any associated MCS unit target.
         * @retval[!0] Associated MCS unit target.
         */
        virtual TARGETING::TargetHandle_t getMcs(
                TARGETING::TargetHandle_t i_proc,
                uint64_t i_pos);

        /**
         * @brief getMcsPos Find the MCS unit position given
         *           the MCS unit target.
         *
         * @param[in] i_mcs The MCS unit target for which the
         *                MCS unit position should be found.
         * @param[out] o_pos The MCS unit position.
         */
        virtual void getMcsPos(
                TARGETING::TargetHandle_t i_mcs,
                uint64_t & o_pos);

        /**
         * @brief getMembuf Find the requested MCS unit target
         *           child membuf chip target.
         *
         * @param[in] i_mcs The MCS unit target
         *             for which the child membuf chip target
         *             should be found.
         *
         * @retval[0] Did not find any associated membuf chip target.
         * @retval[!0] Associated membuf chip target.
         */
        virtual TARGETING::TargetHandle_t getMembuf(
                TARGETING::TargetHandle_t i_mcs);

        /**
         * @brief getType Obtain the type of the provided target.
         *
         * @param[in] i_target The target for which the type
         *                should be obtained.
         *
         * @return The type of the provided target.
         */
        virtual TARGETING::TYPE getType(
                TARGETING::TargetHandle_t i_target);

        /**
         * @brief getAttribute Obtain the attribute for the requested target.
         *
         * @param[in] i_attribute The attribute to be obtained.
         * @param[in] i_target The target for which the attribute
         *                should be obtained.
         * @param[out] o_val The value of the attribute.
         *
         * @retval[true] The attribute was found.
         * @retval[false] The attribute was not found.
         */
        virtual bool getAttribute(
                TARGETING::ATTRIBUTE_ID i_attribute,
                TARGETING::TargetHandle_t i_target,
                uint64_t & o_val);

        /**
         *  @brief Returns the master processor chip target handle
         *
         *  Returns the master processor chip target handle.  On systems
         *  without an alternate master, it returns a handle to the only master,
         *  if found (NULL otherwise).  On systems with multiple potential
         *  masters, it returns a target handle to the acting master (NULL
         *  otherwise).  If targeting information is not yet accessible (very
         *  early in the host boot IPL), the returned target handle will be a
         *  sentinel value representing the master (whichever it may be)
         *
         *  @param[out] o_masterProcChipTarget Target handle referring to the
         *      current master processor chip (the one connected to PNOR)
         *  @param[in]  i_pNodeTarget
         *      Handle to the node target to search for the acting master
         *      processor.  If NULL, HB will search whatever node it's
         *      running on, and FSP will search the physical drawer containing
         *      the lowest ordered fabric node ID.         *
         *  @pre Target Service must be initialized
         *
         *  @post Master processor chip target returned or NULL is returned if
         *  targetservice is not yet initialized or user passed an invalid node
         *  target to get the master proc handle
         */
        virtual void masterProcChipTargetHandle(
            TARGETING::Target*& o_masterProcChipTargetHandle,
            const TARGETING::Target* i_pNodeTarget = NULL) const;

        /**
         * @brief installTargetService
         *
         * Make this the active target service implementation.
         */
        void installTargetService();

        /**
         * @brief dtor
         */
        virtual ~TargetServiceImpl();

        /**
         * @brief ctor
         */
        TargetServiceImpl();
};

inline void TargetService::getMcsList(
        TARGETING::TargetHandle_t i_proc,
        TARGETING::TargetHandleList & o_list)
{
    return iv_impl->getMcsList(i_proc, o_list);
}

inline TARGETING::TargetHandle_t TargetService::getProc(
        TARGETING::TargetHandle_t i_membuf)
{
    return iv_impl->getProc(i_membuf);
}

inline TARGETING::TargetHandle_t TargetService::getMcs(
        TARGETING::TargetHandle_t i_membuf)
{
    return iv_impl->getMcs(i_membuf);
}

inline TARGETING::TargetHandle_t TargetService::getMcs(
        TARGETING::TargetHandle_t i_proc,
        uint64_t i_pos)
{
    return iv_impl->getMcs(i_proc, i_pos);
}

inline void TargetService::getAllChips(
        TARGETING::TargetHandleList & o_list,
        TARGETING::TYPE i_type,
        bool i_functional)
{
    iv_impl->getAllChips(o_list, i_type, i_functional);
}

inline void TargetService::getMcsPos(
        TARGETING::TargetHandle_t i_mcs,
        uint64_t & o_pos)
{
    iv_impl->getMcsPos(i_mcs, o_pos);
}

inline TARGETING::TargetHandle_t TargetService::getMembuf(
        TARGETING::TargetHandle_t i_mcs)
{
    return iv_impl->getMembuf(i_mcs);
}

inline TARGETING::TYPE TargetService::getType(
        TARGETING::TargetHandle_t i_target)
{
    return iv_impl->getType(i_target);
}

inline bool TargetService::getAttribute(
        TARGETING::ATTRIBUTE_ID i_attribute,
        TARGETING::TargetHandle_t i_target,
        uint64_t & o_val)
{
    return iv_impl->getAttribute(i_attribute, i_target, o_val);
}

inline  void TargetService::masterProcChipTargetHandle(
            TARGETING::Target*& o_masterProcChipTargetHandle,
            const TARGETING::Target* i_pNodeTarget) const
{
    iv_impl->masterProcChipTargetHandle(o_masterProcChipTargetHandle,
                                        i_pNodeTarget );
}

inline void TargetService::setImpl(TargetServiceImpl & i_impl)
{
    iv_impl = &i_impl;
}

inline TargetService::TargetService() :
    iv_impl(&Singleton<TargetServiceImpl>::instance())
{
    // by default call the real target service
}

inline void TargetServiceImpl::installTargetService()
{
    getTargetService().setImpl(*this);
}

inline TargetServiceImpl::TargetServiceImpl() {}

}
#endif
