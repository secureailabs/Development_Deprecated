/*********************************************************************************************
 *
 * @file FifoBuffer.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "Object.h"
#include <mutex>
#include <vector>

/********************************************************************************************/

class FifoBuffer : public Object
{
    public:
    
        FifoBuffer(void);
        virtual ~FifoBuffer(void);

        std::vector<Byte> __thiscall Read(
            _in unsigned int unNumberOfBytesToRead
            );
        std::size_t __thiscall GetBytesInBuffer(void);

        Byte * __thiscall WriteLock(
            _in unsigned int unNumberOfBytesToWrite
            );
        void __thiscall WriteUnlock(
            _in unsigned int unNumberOfBytesToPersist
            );
            
    private:
    
        // This class is thread safe, so we need a mutex to guard access to m_stlLockBuffer and m_stlFifoBuffer
        std::recursive_mutex m_stlLock;
        // These buffers are used to host the data that is currently buffered or being buffered
        std::vector<Byte> m_stlFifoBuffer;
        std::vector<Byte> m_stlLockBuffer;
};