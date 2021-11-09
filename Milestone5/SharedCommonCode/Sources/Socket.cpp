/*********************************************************************************************
 *
 * @file Socket.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used to drive a network socket or UNIX socket
 *
 ********************************************************************************************/

#include "Chronometer.h"
#include "Socket.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>

/********************************************************************************************
 *
 * @class Socket
 * @function Socket
 * @brief Constructor that uses a socket descriptor value to initialize Socket
 * @param[in] nSocketDescriptor Socket descriptor of the socket to manage
 * @throw BaseException When an internal error occurs
 *
 ********************************************************************************************/

Socket::Socket(
    _in int nSocketDescriptor
    )
    : m_nSocketDescriptor(-1), m_nPollingFileDescriptor(-1)
{
    __DebugFunction();

    // Initialize epoll stuff. This mechanism is used to help prevent the code from blocking
    // when there is no available data for a read operation.
    struct epoll_event sEvent;
    m_nPollingFileDescriptor = ::epoll_create1(0);
    _ThrowBaseExceptionIf((-1 == m_nPollingFileDescriptor), "epoll_create1() failed with errno = %d", errno);

    // The sockets are all blocking sockets, but we use epoll to check whenever we try to
    // read something.
    sEvent.events = EPOLLIN;
    sEvent.data.fd = nSocketDescriptor;
    int nReturnValue = ::epoll_ctl(m_nPollingFileDescriptor, EPOLL_CTL_ADD, nSocketDescriptor, &sEvent);
    _ThrowBaseExceptionIf((0 != nReturnValue), "epoll_ctl() failed with errno = %d", errno);

    // Persist incoming parameters to class data members
    m_nSocketDescriptor = nSocketDescriptor;
}

/********************************************************************************************
 *
 * @class Socket
 * @function Socket
 * @brief Copy constructor
 * @param[in] c_oSocket Socket object to copy
 *
 ********************************************************************************************/

Socket::Socket(
    _in const Socket & c_oSocket
    ) throw()
    : m_nSocketDescriptor(-1), m_nPollingFileDescriptor(-1)
{
    __DebugFunction();

    m_nSocketDescriptor = c_oSocket.m_nSocketDescriptor;
    m_nPollingFileDescriptor = c_oSocket.m_nPollingFileDescriptor;
}

/********************************************************************************************
 *
 * @class Socket
 * @function ~Socket
 * @brief Destructor
 *
 ********************************************************************************************/

Socket::~Socket(void) throw()
{
    __DebugFunction();

    if (-1 != m_nPollingFileDescriptor)
    {
        __DebugAssert(-1 != m_nSocketDescriptor);

        ::epoll_ctl(m_nPollingFileDescriptor, EPOLL_CTL_DEL, m_nSocketDescriptor, nullptr);
        ::close(m_nPollingFileDescriptor);
        m_nPollingFileDescriptor = -1;
    }

    if (-1 != m_nSocketDescriptor)
    {
        ::close(m_nSocketDescriptor);
        m_nSocketDescriptor = -1;
    }
}

/********************************************************************************************
 *
 * @class Socket
 * @function Read
 * @brief Read @p unNumberOfDesiredBytes of data from the managed socket
 * @param[in] unNumberOfDesiredBytes Number of bytes to read
 * @param[in] unMillisecondTimeout Timeout in milliseconds
 * @return Empty buffer if there were not enough bytes to read or the timeout expired
 * @return Buffer with @p unNumberOfDesiredBytes bytes of data
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall Socket::Read(
    _in unsigned int unNumberOfDesiredBytes,
    _in unsigned int unMillisecondTimeout
    ) throw()
{
    __DebugFunction();

    std::vector<Byte> stlDestinationBuffer;
    try
    {
        if (0 < unNumberOfDesiredBytes)
        {
            stlDestinationBuffer = m_oIncomingBytes.Read(unNumberOfDesiredBytes);
            // If the size of stlDestinationBuffer is still 0, this means that the number of
            // bytes desired were not available on the FIFO buffer. So now, we have to try and
            // gets some data out of the socket and into the FIFO buffer before trying again. This
            // only happens if the unMillisecondTimeout is greater than 0 (the 0 value is the
            // recursion escape value)
            if ((0 == stlDestinationBuffer.size())&&(0 < unMillisecondTimeout))
            {
                int nNumberOfBytesRead = 0;
                Chronometer oChronometer;
                oChronometer.Start();

                // We are going to spin until the request for reading is satisfied or until the
                // timeout is reached
                unsigned int unNumberOfBytesToRead = 0;
                do
                {
                    unsigned int unBytesInFifoBuffer = m_oIncomingBytes.GetBytesInBuffer();
                    if (unBytesInFifoBuffer <= unNumberOfDesiredBytes)
                    {
                        unNumberOfBytesToRead = unNumberOfDesiredBytes - unBytesInFifoBuffer;
                    }
                    // The circular buffer doesn't have unNumberOfDesiredBytes already cached, so let's
                    // go and see if we can read some bytes off of the socket itself
                    struct epoll_event asPollingEvents[100];
                    int nNumberOfEvents = ::epoll_wait(m_nPollingFileDescriptor, asPollingEvents, 100, (unMillisecondTimeout - oChronometer.GetElapsedTimeWithPrecision(Millisecond)));
                    if (0 < nNumberOfEvents)
                    {
                        // Read as many bytes as we can possibly read, regardless of unNumberOfDesiredBytes
                        // Any excess will end up cached.
                        int nNumberOfBytesAvailable = 0;
                        if ((-1 != ::ioctl(m_nSocketDescriptor, FIONREAD, &nNumberOfBytesAvailable))&&(0 < nNumberOfBytesAvailable))
                        {
                            // Calling m_oIncomingBytes.WriteLock() returns a pointer to a buffer which ensures
                            // nNumberOfBytesAvailable contiguous bytes that you can write to
                            Byte * pbCircularBufferDestination = m_oIncomingBytes.WriteLock(nNumberOfBytesAvailable);
                            if (nullptr != pbCircularBufferDestination)
                            {
                                // Calling read() should not block since we already know that there is
                                // nNumberOfBytesAvailable of bytes available to read.
                                nNumberOfBytesRead = ::read(m_nSocketDescriptor, pbCircularBufferDestination, nNumberOfBytesAvailable);
                                // m_oIncomingBytes.WriteUnlock() knows how to handle 0 and -1, so we can
                                // call it directly with the results returned by read()
                                m_oIncomingBytes.WriteUnlock(nNumberOfBytesRead);
                            }
                        }
                    }
                }
                while (((unsigned int)(nNumberOfBytesRead) < unNumberOfBytesToRead) && (unMillisecondTimeout > oChronometer.GetElapsedTimeWithPrecision(Millisecond)));
                // We recursively call Read again, but this time with a 0 millisecond timeout. If not
                // enough bytes are waiting in the FIFO buffer, then this call will return a 0 sized
                // stlDestinationBuffer
                stlDestinationBuffer = this->Read(unNumberOfDesiredBytes, 0);
            }
        }
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return stlDestinationBuffer;
}

/********************************************************************************************
 *
 * @class Socket
 * @function Write
 * @brief Write @p unNumberOfBytesToWrite of data to the managed socket
 * @param[in] c_pbSourceBuffer Source buffer that contains the data to send
 * @param[in] unNumberOfBytesToWrite Number of bytes to send
 * @return -1 if the operation failed
 * @return Positive value representing the number of bytes written
 *
 ********************************************************************************************/

int __thiscall Socket::Write(
    _in const Byte * c_pbSourceBuffer,
    _in unsigned int unNumberOfBytesToWrite
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != c_pbSourceBuffer);
    __DebugAssert(0 < unNumberOfBytesToWrite);

    return ::write(m_nSocketDescriptor, c_pbSourceBuffer, unNumberOfBytesToWrite);
}