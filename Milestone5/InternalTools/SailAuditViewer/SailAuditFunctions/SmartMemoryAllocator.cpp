/*********************************************************************************************
 *
 * @file SmartMemoryAllocator.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of a smart memory allocator
 * @note
 *    The SmartMemoryAllocator is an object that should be implicitly allocated on the stack
 *    or in a class. It ensures that if an exception is thrown or an object instance is deleted,
 *    the memory allocations guarded by the SmartMemoryAllocator are in fact deallocated properly.
 * @par
 *    Developers should used the SmartMemoryAllocator instead of malloc() as often as possible
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "SmartMemoryAllocator.h"

/********************************************************************************************
 *
 * @class SmartMemoryAllocator
 * @function SmartMemoryAllocator
 * @brief Default constructor
 *
 ********************************************************************************************/

SmartMemoryAllocator::SmartMemoryAllocator(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class SmartMemoryAllocator
 * @function ~SmartMemoryAllocator
 * @brief Default destructor
 * @note
 *    Any lingering memory blocks marked for automatic deallocation will be deallocated here.
 *
 ********************************************************************************************/

SmartMemoryAllocator::~SmartMemoryAllocator(void)
{
    __DebugFunction();
    
    for (auto memoryBlock : m_stlElementsAllocated)
    {
        if (nullptr != memoryBlock)
        {
            ::free(memoryBlock);
        }
    }
    m_stlElementsAllocated.clear();
}

/********************************************************************************************
 *
 * @class SmartMemoryAllocator
 * @function Allocate
 * @brief Allocate @p unSizeInBytesDesired bytes of memory
 * @param[in] unSizeInBytesDesired Number of bytes to allocate
 * @param[in] fDeallocateAutomatically Make sure the allocated block gets automatically deallocated when then object is destroyed
 * @returns A pointer to the first byte of a newly allocated memory block
 * @throw BaseException Insufficient memory left to allocate the new memory block
 *
 ********************************************************************************************/

void * __thiscall SmartMemoryAllocator::Allocate(
    _in unsigned int unSizeInBytesDesired
    )
{
    __DebugFunction();
    
    void * pMemoryBlock = nullptr;
    
    if (0 < unSizeInBytesDesired)
    {
        pMemoryBlock = ::malloc(unSizeInBytesDesired);
        _ThrowOutOfMemoryExceptionIfNull(pMemoryBlock);
        m_stlElementsAllocated.insert(pMemoryBlock);
    }
    
    return pMemoryBlock;
}

/********************************************************************************************
 *
 * @class SmartMemoryAllocator
 * @function Deallocate
 * @brief Deallocate the memory block pointed to by @p pMemoryToDeallocate
 * @param[in] pMemoryToDeallocate Pointer to the memory block to deallocate
 *
 ********************************************************************************************/

void __thiscall SmartMemoryAllocator::Deallocate(
    _in void * pMemoryToDeallocate
    ) throw()
{
    __DebugFunction();
    
    if (nullptr != pMemoryToDeallocate)
    {
        // We always free the memory block no matter what
        ::free(pMemoryToDeallocate);
        // If the memory block is registered, make sure to remove it
        if (m_stlElementsAllocated.end() != m_stlElementsAllocated.find(pMemoryToDeallocate))
        {
            m_stlElementsAllocated.erase(pMemoryToDeallocate);
        }
    }
}