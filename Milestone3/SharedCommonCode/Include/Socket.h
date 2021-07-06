/*********************************************************************************************
 *
 * @file Socket.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "FifoBuffer.h"
#include "Object.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

/********************************************************************************************/

class Socket : public Object
{
    public:
    
        Socket(
            _in int nSocketDescriptor
            );
        Socket(
            _in const Socket & c_oSocket
            ) throw();
        ~Socket(void);
        
        // Callers can specify a unMillisecondTimeout on the read operations. If the methods
        // return 0, this means that the timeout expired before the read operation could be
        // fulfilled. The Read() operations will NEVER return a partial buffer. They will
        // ALWAYS either return 0 or return unNumberOfDesiredBytes, unless there is an
        // error and then the Read() methods return -1
        std::vector<Byte> __thiscall Read(
            _in unsigned int unNumberOfDesiredBytes,
            _in unsigned int unMillisecondTimeout
            ) throw();
        // Unlike the read methods, there is no timeout on Write(). The Write() methods will
        // block until the write operation is completed. The Write() methods will either
        // return the number of bytes written or -1 for error.
        int __thiscall Write(
            _in const Byte * c_pbSourceBuffer,
            _in unsigned int unNumberOfBytesToWrite
            ) const throw();
            
    private:
    
        // Private data members
        int m_nSocketDescriptor;
        int m_nPollingFileDescriptor;
        FifoBuffer m_oIncomingBytes;
};