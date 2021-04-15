/*********************************************************************************************
 *
 * @file SocketServer.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the SocketServer class that is used to drive a socket server
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "SocketServer.h"

#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

/********************************************************************************************
 *
 * @class SocketServer
 * @function SocketServer
 * @brief Constructor that uses a socket port identifier to initialize a network socket
 * @param[in] wPortIdentifier Port identifier of the network socket to initialize
 * @throw BaseException If the network socket fails to be initialized properly
 * @note
 *    The network socket created will be bound to all network adapters. There is currently
 *    no facility that enables developers to bind a network socket to a specific adapter
 *
 ********************************************************************************************/
SocketServer::SocketServer(
    _in Word wPortIdentifier
    )
    : m_nSocketDescriptor(-1), m_nPollingFileDescriptor(-1)
{
    __DebugFunction();
    // In this version of the constructor, we are using a standard network socket. As such,
    // we will be using a sockaddr_in structure to set things up
    struct sockaddr_in oSocketAddress;
    // Create the base network (standard) socket
    m_nSocketDescriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    _ThrowBaseExceptionIf((-1 == m_nSocketDescriptor), "socket() failed with errno = %d", errno);
    // Setup some socket options that make the address reusable. This deals with the kernel
    // not releasing sockets fast enough when they are closed.
    int nReuseAddress = 1;
    int nReturnCode = ::setsockopt(m_nSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, (const Byte *) &nReuseAddress, sizeof(nReuseAddress));
    nReturnCode = ::setsockopt(m_nSocketDescriptor, SOL_SOCKET, SO_REUSEPORT, (const Byte *) &nReuseAddress, sizeof(nReuseAddress));
    
    // Initialize the socket address structure
    ::memset(&oSocketAddress, 0, sizeof(oSocketAddress));
    oSocketAddress.sin_family = AF_INET;
    oSocketAddress.sin_addr.s_addr = INADDR_ANY;
    oSocketAddress.sin_port = htons(wPortIdentifier);
    // Bind the socket. This binds the socket to all network adapters
    nReturnCode = ::bind(m_nSocketDescriptor, (struct sockaddr *) &oSocketAddress, sizeof(oSocketAddress));
    _ThrowBaseExceptionIf((-1 == nReturnCode), "bind() failed with errno = %d", errno);
    // Make sure the socket listens for incoming connection requests
    nReturnCode = ::listen(m_nSocketDescriptor, 300);
    _ThrowBaseExceptionIf((-1 == nReturnCode), "listen() failed with errno = %d", errno);
    // Initialize epoll stuff
    struct epoll_event sEvent;
    m_nPollingFileDescriptor = ::epoll_create1(0);
    _ThrowBaseExceptionIf((-1 == m_nPollingFileDescriptor), "epoll_create1() failed with errno = %d", errno);
    // The sockets are all blocking sockets, but we use epoll to check whenever we try to
    // accept something. This will prevent the code from blocking when accept() is called
    sEvent.events = EPOLLIN;
    sEvent.data.fd = m_nSocketDescriptor;
    int nReturnValue = ::epoll_ctl(m_nPollingFileDescriptor, EPOLL_CTL_ADD, m_nSocketDescriptor, &sEvent);
    _ThrowBaseExceptionIf((0 != nReturnValue), "epoll_ctl() failed with errno = %d", errno);
}
/********************************************************************************************
 *
 * @class SocketServer
 * @function SocketServer
 * @brief Constructor that uses a UNIX domain socket name to initialize a UNIX domain socket
 * @param[in] c_szSocketServerAddress Name of the UNIX socket to create
 * @throw BaseException If the network socket fails to be initialized properly
 *
 ********************************************************************************************/
SocketServer::SocketServer(
    _in const char * c_szSocketServerAddress
    )
    : m_nSocketDescriptor(-1), m_nPollingFileDescriptor(-1)
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szSocketServerAddress);
    // In this version of the constructor, we are using a Unix domain socket. As such,
    // we will be using a sockaddr_un structure to set things up
    struct sockaddr_un oSocketAddress;
    // Create the base socket, which is a UNIX socket
    m_nSocketDescriptor = ::socket(AF_UNIX, SOCK_STREAM, 0);
    _ThrowBaseExceptionIf((-1 == m_nSocketDescriptor), "socket() failed with errno = %d", errno);
    // Setup some socket options that make the address reusable. This deals with the kernel
    // not releasing sockets fast enough when they are closed.
    int nReuseAddress = 1;
    int nReturnCode = ::setsockopt(m_nSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, (const Byte *) &nReuseAddress, sizeof(nReuseAddress));
    nReturnCode = ::setsockopt(m_nSocketDescriptor, SOL_SOCKET, SO_REUSEPORT, (const Byte *) &nReuseAddress, sizeof(nReuseAddress));
    
    // Initialize the socket address structure
    ::memset(&oSocketAddress, 0, sizeof(oSocketAddress));
    oSocketAddress.sun_family = AF_UNIX;
    ::strncpy(oSocketAddress.sun_path, c_szSocketServerAddress, (sizeof(oSocketAddress.sun_path) - 1));
    // Make sure that any existing linkage in the system is deleted before binding
    ::unlink(c_szSocketServerAddress);
    nReturnCode = ::bind(m_nSocketDescriptor, (struct sockaddr *) &oSocketAddress, sizeof(oSocketAddress));
    _ThrowBaseExceptionIf((-1 == nReturnCode), "bind() failed with errno = %d", errno);
    m_strUnixDomainAddress = c_szSocketServerAddress;
    // Make sure the socket listens for incoming connection requests
    nReturnCode = ::listen(m_nSocketDescriptor, 300);
    _ThrowBaseExceptionIf((-1 == nReturnCode), "listen() failed with errno = %d", errno);
    // Initialize epoll stuff
    struct epoll_event sEvent;
    m_nPollingFileDescriptor = ::epoll_create1(0);
    _ThrowBaseExceptionIf((-1 == m_nPollingFileDescriptor), "epoll_create1() failed with errno = %d", errno);
    // The sockets are all blocking sockets, but we use epoll to check whenever we try to
    // read something.
    sEvent.events = EPOLLIN;
    sEvent.data.fd = m_nSocketDescriptor;
    int nReturnValue = ::epoll_ctl(m_nPollingFileDescriptor, EPOLL_CTL_ADD, m_nSocketDescriptor, &sEvent);
    _ThrowBaseExceptionIf((0 != nReturnValue), "epoll_ctl() failed with errno = %d", errno);
}

/********************************************************************************************
 *
 * @class SocketServer
 * @function ~SocketServer
 * @brief Destructor which automatically closes the managed socket
 *
 ********************************************************************************************/

SocketServer::~SocketServer(void)
{
    __DebugFunction();

    if (-1 != m_nPollingFileDescriptor)
    {
        __DebugAssert(-1 != m_nSocketDescriptor);

        ::epoll_ctl(m_nPollingFileDescriptor, EPOLL_CTL_DEL, m_nSocketDescriptor, nullptr);
        ::close(m_nPollingFileDescriptor);
        ::unlink(m_strUnixDomainAddress.c_str());
        m_nPollingFileDescriptor = -1;
    }

    if (-1 != m_nSocketDescriptor)
    {
        ::close(m_nSocketDescriptor);
        m_nSocketDescriptor = -1;
    }
    
    if (0 < m_strUnixDomainAddress.size())
    {
        ::remove(m_strUnixDomainAddress.c_str());
    }
}

/********************************************************************************************
 *
 * @class SocketServer
 * @function WaitForConnection
 * @brief Method used to wait for an incoming connection
 * @param[in] unMillisecondTimeout Timeout in millisecond to wait before returning
 * @return true of a new connection is waiting to be accepted (i.e. call Accept right after)
 * @return false if there if the function returned because it timed out
 * @note
 *    This method does not actuall establish a socket or make a connection. All it does is
 *    show that there is a connection waiting to be accepted. If this method returns true,
 *    then the code should immediately call SocketServer::Accept
 *
 ********************************************************************************************/

bool __thiscall SocketServer::WaitForConnection(
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    bool fConnectionWaiting = false;

    struct epoll_event sPollingEvents;
    int nNumberOfEvents = ::epoll_wait(m_nPollingFileDescriptor, &sPollingEvents, 1, unMillisecondTimeout);
    if (0 < nNumberOfEvents)
    {
        fConnectionWaiting = true;
    }

    return fConnectionWaiting;
}

/********************************************************************************************
 *
 * @class SocketServer
 * @function Accept
 * @brief Method used to accept a waiting socket connection
 * @return A pointer to a socket
 * @throw BaseException If the attempt to accept the incoming socket fails
 *
 ********************************************************************************************/

Socket * __thiscall SocketServer::Accept(void)
{
    __DebugFunction();

    struct sockaddr_un sSocketAddress;
    socklen_t nLength = sizeof(sSocketAddress);
    
    // Make sure the zeroize the socket address information structure before calling accept()
    ::memset(&sSocketAddress, 0, sizeof(sSocketAddress));
    // There is a small chance that between the call to WaitForConnection() and Accept(), the
    // incoming connection attempt is abandonned. If this happens, calling accept() would end
    // up blocking, which is NOT desirable. As such, we do another quick check of how many
    // events are waiting using epoll_wait()
    //int nNumberOfEvents = ::epoll_wait(m_nPollingFileDescriptor, &sPollingEvents, 1, 0);
    //_ThrowBaseExceptionIf((0 == nNumberOfEvents), "Unexpected condition. No connections waiting.", nullptr);
    // Now we can call accept() with an almost 100% probability that this call will instantly
    // succeed and not block
    int nClientSocket = ::accept(m_nSocketDescriptor, (struct sockaddr *) &sSocketAddress, &nLength);
    _ThrowBaseExceptionIf((-1 == nClientSocket), "accept() failed with errno = %d", errno);

    return new Socket(nClientSocket);
}
