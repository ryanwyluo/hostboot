/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/plat/mem/prdfMemTdQueue.H $                 */
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

/** @file  prdfMemTdQueue.H
 *  @brief Support code for a Targeted Diagnostics weighted event queue.
 */

#ifndef __prdfMemTdQueue_H
#define __prdfMemTdQueue_H

// Framework includes
#include <iipServiceDataCollector.h>

// Platform includes
#include <prdfMemRank.H>

// Other includes
#include <algorithm>
#include <vector>

namespace PRDF
{

//------------------------------------------------------------------------------
//                               TdEntry class
//------------------------------------------------------------------------------

/** @brief Abstract class for a Targeted Diagnostics event. */
class TdEntry
{
  public: // enums, constants

    /**
     * @brief This enum will be used to indicate type of TD event requested to
     *        be handled.
     * @note  The order of the enums values is important because it is used for
     *        sorting the TdQueue by event type priority.
     */
    enum TdType
    {
        VCM_EVENT = 0,       ///< A Verify Chip Mark event.
        TPS_EVENT,           ///< A Two-Phase Scrub event.
        INVALID_EVENT = 0xf, ///< Used to denote that no event is in progress
    };

    /**
     * @brief This enum will indicate which VCM or TPS phase we are currently on
     */
    enum Phase
    {
        TD_PHASE_0,
        TD_PHASE_1,
        TD_PHASE_2,
    };

  public: // functions

    /** @brief Default destructor */
    virtual ~TdEntry() = default;

    /**
     * @brief  Each entry will have a set of steps that need to be performed.
     *         This function tells the procedure to move onto the next step.
     * @param  io_sc  The step code data struct.
     * @param  o_done Returns true if this procedure is complete, false
     *                otherwise.
     * @return Non-SUCCESS if an internal function fails, SUCCESS otherwise.
     */
    virtual uint32_t nextStep( STEP_CODE_DATA_STRUCT & io_sc,
                               bool & o_done ) = 0;

    /** @brief '==' operator */
    bool operator==( const TdEntry & i_e ) const
    {
        return ( this->iv_chip   == i_e.iv_chip   &&
                 this->iv_tdType == i_e.iv_tdType &&
                 this->iv_rank   == i_e.iv_rank      );
    }

    /**
     * @brief '<' operator
     * @note This only compares iv_tdType because the TdQueue only sorts by type
     */
    bool operator<( const TdEntry & i_e ) const
    {
        return this->iv_tdType < i_e.iv_tdType;
    }

    /** @return The event type */
    TdType getType() const { return iv_tdType; }

    /** @return The rank in which this event occurred */
    MemRank getRank() const { return iv_rank; }

    /** @return The event phase */
    Phase getPhase() const { return iv_phase; }

  protected: // functions

    /**
     * @brief Constructor
     * @param i_tdType  See TdType enum
     * @param i_chip    MCA or MBA chip
     * @param i_rank    Target rank
     */
    TdEntry( TdType i_tdType, ExtensibleChip * i_chip, MemRank i_rank ) :
        iv_chip(i_chip), iv_tdType(i_tdType), iv_rank(i_rank)
    {
        PRDF_ASSERT( (TARGETING::TYPE_MCA == iv_chip->getType()) ||
                     (TARGETING::TYPE_MBA == iv_chip->getType())    );
    }

  protected: // instance variables

    /** The chip associated with this entry. This isn't used for any
     *  comparisons, but is needed for every TD procedure. */
    ExtensibleChip * const iv_chip;

    const TdType  iv_tdType;     ///< The event type (see enum TdType).
    const MemRank iv_rank;       ///< The rank in which this event occurred.
    Phase iv_phase = TD_PHASE_0; ///< The event phase (see enum Phase).
};

//------------------------------------------------------------------------------
//                               TdQueue class
//------------------------------------------------------------------------------

/**
 * @brief This is a weighted queue for all Targeted Diagnostics events.
 * @note  Events with a higher priority will be moved ahead of lower priority
 *        events.
 */
class TdQueue
{
  public: // typedefs

    typedef          std::vector< TdEntry * >           Queue;
    typedef typename std::vector< TdEntry * >::iterator QueueItr;

  public: // functions

    /** @brief Destructor. */
    ~TdQueue() { for ( auto & a : iv_queue ) delete a; }

    /** @return TRUE if the queue is empty, FALSE otherwise. */
    bool empty() const { return iv_queue.empty(); }

    /**
     * @brief  Sorts the queue by priority order then returns the first entry.
     * @return The first entry in the queue.
     * @note   This is intended to be called only once for each time the TD
     *         controller needs to find the next TD procedure to do. If it is
     *         called multiple times, it is possible the list is reordered such
     *         that a new entry is moved to the front of the queue and is
     *         mistakenly removed via pop() before the TD controller is able to
     *         execute the request.
     */
    TdEntry * getNextEntry()
    {
        PRDF_ASSERT( !iv_queue.empty() );

        // TODO: RTC 66487 This function currently has the complexity of
        //       O(n lg n) because of the sorting. It is possible to optimize
        //       this to O(n) if we use push_heap()/pop_heap(). However, there
        //       is a problem were push_heap() could possibly reorder the queue
        //       while a TD procedure is in progress, which is undesirable.

        std::sort( iv_queue.begin(), iv_queue.end(),
                   [](TdEntry * a, TdEntry * b) { return *a < *b; } );

        return iv_queue.front();
    }

    /**
     * @brief Add new TD entry at the end of the queue.
     * @param i_e A TD entry.
     * @note  Only adds the entry to the queue if the entry does not already
     *        exist in the queue.
     */
    void push( TdEntry * i_e )
    {
        QueueItr it = std::find_if( iv_queue.begin(), iv_queue.end(),
                                    [=](TdEntry * a){return *i_e == *a;} );
        if ( iv_queue.end() == it )
        {
            iv_queue.push_back( i_e );
        }
        else
        {
            // The event is already in the queue. So free up the memory.
            delete i_e;
        }
    }

    /**
     * @brief Removes the entry at the beginning of the queue.
     */
    // TODO: RTC 66487 This function currently has a complexity of 0(n). It
    //       is preferred to have 0(1), which could be accomplished with by
    //       using a deque or list. Unfortunately, Hostboot currently does not
    //       support std::deque or std::list::sort(). Therefore, we must use a
    //       vector at this time.
    void pop()
    {
        PRDF_ASSERT( !iv_queue.empty() );

        delete *(iv_queue.begin());
        iv_queue.erase(iv_queue.begin());
    }

    /**
     * @return A constant reference to the queue.
     * @note   The only purpose for this is for FFDC.
     */
    const Queue & getQueue() const { return iv_queue; }

  private: // instance variables

    /** Stores all ranks that are marked for targeted diagnostics. */
    Queue iv_queue;
};

} // end namespace PRDF

#endif // __prdfMemTdQueue_H

