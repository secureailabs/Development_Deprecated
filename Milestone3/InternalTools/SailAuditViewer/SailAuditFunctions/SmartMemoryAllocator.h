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
#include <set>

/********************************************************************************************/

class SmartMemoryAllocator
{
    public:

        SmartMemoryAllocator(void);
        ~SmartMemoryAllocator(void);

        void * __thiscall Allocate(
            _in unsigned int unSizeInBytesDesired
            );
        void __thiscall Deallocate(
            _in void * pMemoryToDeallocate
            ) throw();

    private:

        std::set<void *> m_stlElementsAllocated;
};