/*********************************************************************************************
 *
 * @file SocketClient.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Collection of helper functions that help clients connect to a socket
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "SocketClient.h"
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

/********************************************************************************************
 *
 * @function ConnectToUnixDomainSocket
 * @brief Function used to connect to a UNIX domain socket (i.e. IPC)
 * @param[in] c_strUnixDomainSocketPath Name of the UNIX domain socket
 * @return An initialized socket
 * @throw BaseException If the connection is not successful
 *
 ********************************************************************************************/

Socket * __stdcall ConnectToUnixDomainSocket(
    _in const std::string & c_strUnixDomainSocketPath
    )
{
    __DebugFunction();

    int nSocketDescriptor = 0;
    struct sockaddr_un sSocketAddress;

    // Create the base socket
    nSocketDescriptor = ::socket(AF_UNIX, SOCK_STREAM, 0);
    _ThrowBaseExceptionIf((-1 == nSocketDescriptor), "socket() failed with errno = %d", errno);

    // Initialize the socket address structure
    ::memset(&sSocketAddress, 0, sizeof(sSocketAddress));
    sSocketAddress.sun_family = AF_UNIX;
    ::strncpy(sSocketAddress.sun_path, c_strUnixDomainSocketPath.c_str(), (sizeof(sSocketAddress.sun_path) - 1));

    // Make sure that any existing linkage in the system is deleted before binding
    int nReturnValue = ::connect(nSocketDescriptor, (struct sockaddr *) &sSocketAddress, sizeof(sSocketAddress));
    _ThrowBaseExceptionIf((0 != nReturnValue), "connect() failed with errno = %d", errno);

    return new Socket(nSocketDescriptor);
}

/********************************************************************************************
 *
 * @function ConnectToNetworkSocket
 * @brief Function used to connect to a standard network socket (i.e. IPC)
 * @param[in] c_strTargetIpAddress Target IP address of the socket to connect to
 * @param[in] wPortIdentifier Port identifier of the socket to connect to
 * @return An initialized socket
 * @throw BaseException If the connection is not successful
 *
 ********************************************************************************************/

Socket * __stdcall ConnectToNetworkSocket(
    _in const std::string c_strTargetIpAddress,
    _in Word wPortIdentifier
    )
{
    __DebugFunction();

    int nSocketDescriptor = 0;
    struct sockaddr_in sSocketAddress;

    // Create the base socket
    nSocketDescriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    _ThrowBaseExceptionIf((-1 == nSocketDescriptor), "socket() failed with errno = %d", errno);

    // Initialize the socket address structure
    ::memset(&sSocketAddress, 0, sizeof(sSocketAddress));
    sSocketAddress.sin_family = AF_INET;
    sSocketAddress.sin_port = htons(wPortIdentifier);
    int nReturnValue = ::inet_pton(AF_INET, c_strTargetIpAddress.c_str(), &(sSocketAddress.sin_addr));
    _ThrowBaseExceptionIf((1 != nReturnValue), "inet_pton() failed with errno = %d", errno);

    // Now we can try to connect
    nReturnValue = ::connect(nSocketDescriptor, (struct sockaddr *) &sSocketAddress, sizeof(sSocketAddress));
    _ThrowBaseExceptionIf((0 != nReturnValue), "connect() failed with errno = %d", errno);

    return new Socket(nSocketDescriptor);
}
