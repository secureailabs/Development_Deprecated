/*********************************************************************************************
 *
 * @file SmartMemoryAllocator.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include <map>
#include <mutex>

/********************************************************************************************/

class SmartMemoryAllocator
{
    public:

        SmartMemoryAllocator(void) throw();
        ~SmartMemoryAllocator(void) throw();

        void * __thiscall Allocate(
            _in unsigned int unSizeInBytesDesired,
            _in bool fDeallocateAutomatically
            );
        void __thiscall Deallocate(
            _in void * pMemoryToDeallocate
            ) throw();

    private:

        std::mutex m_stlLock;
        std::map<void *, bool> m_stlElementsAllocated;
};