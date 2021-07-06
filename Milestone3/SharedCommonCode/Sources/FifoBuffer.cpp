/*********************************************************************************************
 *
 * @file FifoBuffer.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the FifoBuffer class
 *
 * The FifoBuffer object implements a first-in-first-out buffer mechanism which is
 * thread safe and allows developers to continuously read and write to the buffer. The order
 * of what is read respect the FIFO nature of the buffer.
 *
 ********************************************************************************************/

#include "FifoBuffer.h"
#include "Chronometer.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

#include <pthread.h>
#include <unistd.h>

#include <iostream>

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function FifoBuffer
 * @brief FifoBuffer class constructor
 *
 ********************************************************************************************/

FifoBuffer::FifoBuffer(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function ~FifoBuffer
 * @brief FifoBuffer class destructor
 *
 ********************************************************************************************/

FifoBuffer::~FifoBuffer(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function Read
 * @brief Reads a certain number of bytes from the FifoBuffer.
 * @param[in] unNumberOfBytesToRead Number of bytes to read out of the FifoBuffer
 * @return A buffer (std:vector<Byte>) of bytes containing @p unNumberOfBytesToRead bytes or, an empty buffer
 * @note
 *    This method tries to read unNumberOfBytesToRead from the internal buffer. This is an all or
 *    nothing operation. So if there isn't unNumberOfBytesToRead of data available, then
 *    this method returns an empty buffer (i.e. std::vector<Byte>)
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall FifoBuffer::Read(
    _in unsigned int unNumberOfBytesToRead
    )
{
    __DebugFunction();

    std::vector<Byte> stlReturnBuffer;

    if (0 < unNumberOfBytesToRead)
    {
        m_stlLock.lock();
        if (unNumberOfBytesToRead <= m_stlFifoBuffer.size())
        {
            stlReturnBuffer.assign(m_stlFifoBuffer.begin(), (m_stlFifoBuffer.begin() + unNumberOfBytesToRead));
            m_stlFifoBuffer.erase(m_stlFifoBuffer.begin(), (m_stlFifoBuffer.begin() + unNumberOfBytesToRead));
        }
        m_stlLock.unlock();
    }

    return stlReturnBuffer;
}

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function GetBytesInBuffer
 * @brief Get number of bytes in the buffer.
 * @return The bytes in the fifo buffer
 *
 ********************************************************************************************/

std::size_t __thiscall FifoBuffer::GetBytesInBuffer(void)
{
    __DebugFunction();

    std::size_t unSizeToReturn = 0;
    m_stlLock.lock();
    unSizeToReturn = m_stlFifoBuffer.size();
    m_stlLock.unlock();

    return unSizeToReturn;
}

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function WriteLock
 * @brief Lock down a specific number of contiguous bytes for writing.
 * @param[in] unNumberOfBytesToWrite Number of bytes to write into the FifoBuffer
 * @return Valid Byte * to the area of contiguous memory where @p unNumberOfBytesToWrite can be written to.
 * @return A nullptr if (0 == @p unNumberOfBytesToWrite)
 * @note
 *    The WriteLock() method only establishes a lock and returns a pointer to the area of memory
 *    where the actual bytes of data can be written to. This contiguous buffer of bytes
 *    is guaranteed as long as the lock is effective. It is okay to reserve MORE bytes than
 *    required. This is often used when the number of bytes to write is not known. At the time
 *    of unlock, the number of bytes to actually persist is specified.
 *
 ********************************************************************************************/

Byte * __thiscall FifoBuffer::WriteLock(
    _in unsigned int unNumberOfBytesToWrite
    )
{
    __DebugFunction();

    Byte * pbReturnedBuffer = nullptr;

    if (0 < unNumberOfBytesToWrite)
    {
        m_stlLock.lock();
        m_stlLockBuffer.resize(unNumberOfBytesToWrite);
        pbReturnedBuffer = &(m_stlLockBuffer[0]);
    }

    return pbReturnedBuffer;
}

/********************************************************************************************
 *
 * @class FifoBuffer
 * @function WriteUnlock
 * @brief Release the write lock and commit the incoming bytes into the FIFO buffer
 * @param[in] unNumberOfBytesToPersist Number of bytes to actually persist to the FifoBuffer
 * @throw BaseException If unNumberOfBytesToPersist > unNumberOfBytesToWrite specified when WriteLock() was called
 * @note
 *    This method commits @p unNumberOfBytesToPersist bytes to the internal FIFO buffer. It
 *    should be noted that @p unNumberOfBytesToPersist can be smaller than the original
 *    unNumberOfBytesToWrite that were reserved when FifoBuffer::WriteLock() was called.
 *
 ********************************************************************************************/

void __thiscall FifoBuffer::WriteUnlock(
    _in unsigned int unNumberOfBytesToPersist
    )
{
    __DebugFunction();

    bool fThrowException = false;

    // Recursively try to lock again to make sure whoever is calling WriteUnlock() was in
    // fact the SAME thread that called WriteLock(). This is because calling unlock
    // on a mutex that is not controlled by the current thread has an undefined behavior
    if (true == m_stlLock.try_lock())
    {
        // Check to make sure we are not trying to persist more bytes than what was
        // actually reserved in the m_stlLockBuffer. This is important since this
        // could turn into a buffer overrun attack
        if (unNumberOfBytesToPersist > m_stlLockBuffer.size())
        {
            // We cannot throw an exception now since there is some cleanup stuff to be
            // done with the mutex and the intermediate buffer
            fThrowException = true;
        }
        else
        {
            m_stlFifoBuffer.insert(m_stlFifoBuffer.end(), m_stlLockBuffer.begin(), (m_stlLockBuffer.begin() + unNumberOfBytesToPersist));
        }
        m_stlLockBuffer.clear();
        // Unlock the recursive try_lock()
        m_stlLock.unlock();
        // Now unlock the original lock()
        m_stlLock.unlock();
    }

    if (true == fThrowException)
    {
        _ThrowSimpleException("OUT_OF_BOUNDS EXCEPTION! unNumberOfBytesToPersist is greater than unNumberOfBytesToWrite specified when WriteLocked() was called");
    }
}