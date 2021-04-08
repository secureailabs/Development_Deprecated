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
#include "ExceptionRegister.h"
#include "SmartMemoryAllocator.h"

/********************************************************************************************
 *
 * @class SmartMemoryAllocator
 * @function SmartMemoryAllocator
 * @brief Default constructor
 *
 ********************************************************************************************/

SmartMemoryAllocator::SmartMemoryAllocator(void) throw()
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

SmartMemoryAllocator::~SmartMemoryAllocator(void) throw()
{
    __DebugFunction();
    
    m_stlLock.lock();
    for (auto const & memoryBlock : m_stlElementsAllocated)
    {
        if (true == memoryBlock.second)
        {
            __DebugWarning("Dangling memory block 0x%08X%08X deallocated.", HIGHDWORD(memoryBlock.first), LODWORD(memoryBlock.first));
            ::free(memoryBlock.first);
        }
    }
    m_stlElementsAllocated.clear();
    m_stlLock.unlock();
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
    _in unsigned int unSizeInBytesDesired,
    _in bool fDeallocateAutomatically
    )
{
    __DebugFunction();
    
    void * pMemoryBlock = nullptr;
    
    if (0 < unSizeInBytesDesired)
    {
        m_stlLock.lock();
        pMemoryBlock = ::malloc(unSizeInBytesDesired);
        _ThrowOutOfMemoryExceptionIfNull(pMemoryBlock);
        // Make sure to register the memory block
        m_stlElementsAllocated[pMemoryBlock] = fDeallocateAutomatically;
        m_stlLock.unlock();
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
        ::free(pMemoryToDeallocate);
        // If the memory block is registered, make sure to remove it
        m_stlLock.lock();
        if (m_stlElementsAllocated.end() != m_stlElementsAllocated.find(pMemoryToDeallocate))
        {
            m_stlElementsAllocated.erase(pMemoryToDeallocate);
        }
        m_stlLock.unlock();
    }
}