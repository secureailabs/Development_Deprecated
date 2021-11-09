/*********************************************************************************************
 *
 * @file TlsServer.cpp
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TlsServer class that is used to drive a socket server
 *
 ********************************************************************************************/

#include "TlsServer.h"
#include "SocketServer.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

/********************************************************************************************
 *
 * @class TlsServer
 * @function TlsServer
 * @brief Constructor that uses a socket port identifier to initialize a network socket
 * @param[in] wPortIdentifier Port identifier of the network socket to initialize
 * @throw BaseException If the network socket fails to be initialized properly
 *
 ********************************************************************************************/

TlsServer::TlsServer(
    _in Word wPortIdentifier
    )
{
    __DebugFunction();

    m_poSocketServer = new SocketServer(wPortIdentifier);
}

/********************************************************************************************
 *
 * @class TlsServer
 * @function TlsServer
 * @brief Constructor that uses a UNIX domain socket name to initialize a UNIX domain socket server
 * @param[in] c_szTLSServerAddress Name of the UNIX socket to create
 * @throw BaseException If the network socket fails to be initialized properly
 *
 ********************************************************************************************/

TlsServer::TlsServer(
    _in const char * c_szUnixSocketAddress
    )
{
    __DebugFunction();

    m_poSocketServer = new SocketServer(c_szUnixSocketAddress);
}

/********************************************************************************************
 *
 * @class TlsServer
 * @function ~TlsServer
 * @brief Destructor which automatically closes the managed socket
 *
 ********************************************************************************************/

TlsServer::~TlsServer(void)
{
    __DebugFunction();

    m_poSocketServer->Release();
}

/********************************************************************************************
 *
 * @class TlsServer
 * @function WaitForConnection
 * @brief Method used to wait for an incoming connection
 * @param[in] unMillisecondTimeout Timeout in millisecond to wait before returning
 * @return true of a new connection is waiting to be accepted (i.e. call Accept right after)
 * @return false if the function returned because it timed out
 * @note
 *    This method does not actuall establish a socket or make a connection. All it does is
 *    show that there is a connection waiting to be accepted. If this method returns true,
 *    then the code should immediately call TlsServer::Accept
 *
 ********************************************************************************************/

bool __thiscall TlsServer::WaitForConnection(
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    return m_poSocketServer->WaitForConnection(1000);
}

/********************************************************************************************
 *
 * @class TlsServer
 * @function Accept
 * @brief Method used to accept a waiting socket connection
 * @return A pointer to a TlsNode
 * @throw BaseException If the attempt to accept the incoming socket fails
 * @note
 *    This method also performs the handshake and initial TLS setup. Hence, it preferably
 *    should be called in a new thread so that the process is not occupied during the
 *    handhsake and connection setup.
 ********************************************************************************************/

//TODO: [MVP-34] Add the support for a crypto trust store.
TlsNode * __thiscall TlsServer::Accept(void)
{
    __DebugFunction();

    TlsNode * poTlsNode = nullptr;
    try
    {
        Socket * poSocket = m_poSocketServer->Accept();

        poTlsNode = new TlsNode(poSocket, eSSLModeServer);
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return poTlsNode;
}
