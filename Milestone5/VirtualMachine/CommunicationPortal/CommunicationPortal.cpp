/*********************************************************************************************
 *
 * @file CommunicationPortal.cpp
 * @author Prawal Gangwar
 * @date 08 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Communication Portal is a one-stop gateway to all the communication that happens on
 *      the Virtual Machine
 *
*********************************************************************************************/

#include "CommunicationPortal.h"
#include "Exceptions.h"
#include "DebugLibrary.h"
#include "SocketClient.h"
#include "SocketServer.h"
#include "TlsServer.h"
#include "IpcTransactionHelperFunctions.h"
#include "TlsTransactionHelperFunctions.h"
#include "Base64Encoder.h"
#include "ExceptionRegister.h"

#include <iostream>
#include <thread>
#include <future>

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function CommunicationPortal
 * @brief Constructor to create a CommunicationPortal object
 *
 ********************************************************************************************/

CommunicationPortal::CommunicationPortal(void)
{
    __DebugFunction();

    // Add the list of all the processes that are allowed to use the Communication Portal
    m_oSetOfAllowedProcesses.insert("JobEngine");
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function CommunicationPortal
 * @brief Destructor a CommunicationPortal object
 *
 ********************************************************************************************/

CommunicationPortal::~CommunicationPortal(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::StartServer(
    _in Word wPortNumber
)
{
    __DebugFunction();

    TlsServer oTlsServer(wPortNumber);
    do
    {
        if (true == oTlsServer.WaitForConnection(1000))
        {
            TlsNode * poTlsNode = oTlsServer.Accept();
            if (nullptr != poTlsNode)
            {
                std::thread(&CommunicationPortal::HandleConnection, this, poTlsNode).detach();
            }
        }
    }
    //TODO: put a condition here to stop
    while(true);
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::WaitForProcessToRegister(void)
{
    __DebugFunction();

    SocketServer oSocketServerForIpc(gc_strCommunicationPortalAddress.c_str());

    // This will wait for all the process running on the Virtual Machine to start and register
    // themseleves here before the portal would start accepting TLS requests from remote clients
    while(0 < m_oSetOfAllowedProcesses.size())
    {
        if (true == oSocketServerForIpc.WaitForConnection(1000))
        {
            Socket * poSocket = oSocketServerForIpc.Accept();
            if (nullptr != poSocket)
            {
                StructuredBuffer oStructuredBufferProcessInformation = StructuredBuffer(::GetIpcTransaction(poSocket, true));
                std::string strProcessName = oStructuredBufferProcessInformation.GetString("ProcessName");
                // Only add the process if it is in the list of process that are allowed to register
                if (m_oSetOfAllowedProcesses.end() != m_oSetOfAllowedProcesses.find(strProcessName))
                {
                    // Save the socket for the process in a map to be called directly when forwarding messages
                    m_stlMapOfProcessToIpcSocket.insert(std::make_pair(strProcessName, poSocket));

                    // Once the socket for further communication is saved, the Process is rmeoved from the list
                    m_oSetOfAllowedProcesses.erase(strProcessName);

                    std::cout << "Successfully Registered " << strProcessName << std::endl;
                }
            }
        }
    }
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::HandleConnection(
    _in TlsNode * const c_poTlsNode
)
{
    __DebugFunction();

    try
    {
        std::vector<std::thread> stlListOfThreads;
        std::cout << "New Connection.. Waiting for data" << std::endl;
        auto stlPacket = ::GetTlsTransaction(c_poTlsNode, 0);
        _ThrowBaseExceptionIf((0 == stlPacket.size()), "Empty Packet", nullptr);

        StructuredBuffer oStructuredBufferNewRequest(stlPacket);
        // If the connection was established with the JobEngine it will be a persistant connection,
        // otherwise it will be a single transaction function.
        std::string strEndPoint = oStructuredBufferNewRequest.GetString("EndPoint");
        if ("JobEngine" == strEndPoint)
        {
            // Send a kill signal to the TlsToIpc thread to kill the connection
            if (m_stlTlsToIpcConnectionConnected.end() != m_stlTlsToIpcConnectionConnected.find(strEndPoint))
            {
                if (true == m_stlTlsToIpcConnectionConnected.at(strEndPoint))
                {
                    m_stlKillTlsToIpcConnection.at(strEndPoint) = true;

                    // Busy wait for the existing connections to be terminated
                    std::cout << "Waiting for existing TlsToIpc thread to die" << std::endl;
                    while(true == m_stlTlsToIpcConnectionConnected.at(strEndPoint))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                }
                m_stlKillTlsToIpcConnection.at(strEndPoint) = false;
            }

            // A new thread is created for the reading requests on the TlsNode and forwarding them
            // to the IPC connection while forwarding the data read on IPC connection
            // will be handled in this thread.
            auto stlFutureToWait = std::async(&CommunicationPortal::PersistantConnectionTlsToIpc, this, c_poTlsNode, strEndPoint, oStructuredBufferNewRequest);

            // Wait for the IpcToTls connection to be terminated if it exists.
            if (m_stlIpcToTlsConnectionConnected.end() != m_stlIpcToTlsConnectionConnected.find(strEndPoint))
            {
                if (true == m_stlIpcToTlsConnectionConnected.at(strEndPoint))
                {
                    // Busy wait for the existing connections to be terminated
                    std::cout << "Waiting for existing IpcToTls thread to die" << std::endl;
                    while(true == m_stlIpcToTlsConnectionConnected.at(strEndPoint))
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                }
            }

            // This is blocking call and will exit only when the end connection signal is received via IPC
            // from the JobEngine
            std::cout << "Creating a PersistantConnectionIpcToTls thread" << std::endl;
            this->PersistantConnectionIpcToTls(c_poTlsNode, strEndPoint);

            // Once the Ipc to Tls is terminiated we also wait for TlsToIpc to end
            stlFutureToWait.get();

            std::cout << "\n\n\n########### Connection terminated ##############\n\n\n";
            fflush(stdout);
        }
        else
        {
            this->OneTimeConnectionHandler(c_poTlsNode, oStructuredBufferNewRequest);
        }
        c_poTlsNode->Release();
    }
    catch (BaseException & oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::PersistantConnectionTlsToIpc(
    _in TlsNode * const c_poTlsNode,
    _in const std::string c_strEndpoint,
    _in const StructuredBuffer oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        bool fKeepRunning = true;

        if (m_stlTlsToIpcConnectionConnected.end() != m_stlTlsToIpcConnectionConnected.find(c_strEndpoint))
        {
            m_stlTlsToIpcConnectionConnected.at(c_strEndpoint) = true;
        }
        else
        {
            m_stlTlsToIpcConnectionConnected.insert(std::make_pair(c_strEndpoint, true));
            m_stlKillTlsToIpcConnection.insert(std::make_pair(c_strEndpoint, false));
        }

        StructuredBuffer oStructuredBufferMessageToPass = oStructuredBuffer;
        std::vector<Byte> stlTlsPacket;

        do
        {
            if (true == fKeepRunning)
            {
                if (0 < stlTlsPacket.size())
                {
                    oStructuredBufferMessageToPass = stlTlsPacket;
                }
                std::string strEndpointName = oStructuredBufferMessageToPass.GetString("EndPoint");
                if (m_stlMapOfProcessToIpcSocket.end() != m_stlMapOfProcessToIpcSocket.find(strEndpointName))
                {
                    if (oStructuredBufferMessageToPass.IsElementPresent("EndConnection", BOOLEAN_VALUE_TYPE))
                    {
                        if (true == oStructuredBufferMessageToPass.GetBoolean("EndConnection"))
                        {
                            fKeepRunning = false;
                        }
                    }
                    else
                    {
                        ::PutIpcTransaction(m_stlMapOfProcessToIpcSocket.at(strEndpointName), oStructuredBufferMessageToPass);
                    }
                }
            }

            // Keep listening for 5 second, if no packet was received do it again.
            // But if the signal is received from the IPC to TLs thread for shutting this connection,
            // everything is ignored and the thread is shutdown.
            do
            {
                stlTlsPacket = ::GetTlsTransaction(c_poTlsNode, 5000);
                // Keep running if there is no new connection request or kill signal
                fKeepRunning = !m_stlKillTlsToIpcConnection.at(c_strEndpoint);
            }
            while((0 == stlTlsPacket.size()) && (true == fKeepRunning));
        }
        while(true == fKeepRunning);

        // Mark the connection as disconnected
        m_stlTlsToIpcConnectionConnected.at(c_strEndpoint) = false;
    }
    catch (BaseException & oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    std::cout << "\n\n\n Exiting PersistantConnectionTlsToIpc. Something happened.\n\n\n";
    fflush(stdout);

    return;
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::PersistantConnectionIpcToTls(
    _in TlsNode * const c_poTlsNode,
    _in const std::string c_strEndpoint
)
{
    __DebugFunction();

    bool fKeepRunning = true;

    if (m_stlIpcToTlsConnectionConnected.end() != m_stlIpcToTlsConnectionConnected.find(c_strEndpoint))
    {
        m_stlIpcToTlsConnectionConnected.at(c_strEndpoint) = true;
    }
    else
    {
        m_stlIpcToTlsConnectionConnected.insert(std::make_pair(c_strEndpoint, true));
    }

    do
    {
        StructuredBuffer oStructuredBuffer(::GetIpcTransaction(m_stlMapOfProcessToIpcSocket.at(c_strEndpoint), true));
        std::cout << "PersistantConnectionIpcToTls " << oStructuredBuffer.ToString() << std::endl;
        fflush(stdout);
        if (oStructuredBuffer.IsElementPresent("KeepAlive", BOOLEAN_VALUE_TYPE))
        {
            if (false == oStructuredBuffer.GetBoolean("KeepAlive"))
            {
                fKeepRunning = false;
            }
        }
        else if (oStructuredBuffer.IsElementPresent("Timeout", BOOLEAN_VALUE_TYPE))
        {
            if (true == oStructuredBuffer.GetBoolean("Timeout"))
            {
                fKeepRunning = false;
                // The TlsToIpc connection is closed here only if the connection timedout.
                // In case of new request the TlsToIpc connection is already killed.
                m_stlKillTlsToIpcConnection.at(c_strEndpoint) = true;
            }
        }
        if (true == fKeepRunning)
        {
            ::PutTlsTransaction(c_poTlsNode, oStructuredBuffer);
        }
    }
    while(fKeepRunning);

    // Mark the connection as disconnected
    m_stlIpcToTlsConnectionConnected.at(c_strEndpoint) = false;
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

void __thiscall CommunicationPortal::OneTimeConnectionHandler(
    _in TlsNode * const c_poTlsNode,
    _in const StructuredBuffer c_oStructuredBufferNewRequest
)
{
    __DebugFunction();

    try
    {
        std::string strEndPoint = c_oStructuredBufferNewRequest.GetString("EndPoint");
        if (m_stlMapOfProcessToIpcSocket.end() != m_stlMapOfProcessToIpcSocket.find(strEndPoint))
        {
            // Send the Request to the endpoint via IPC and then wait for the reponse.
            ::PutIpcTransaction(m_stlMapOfProcessToIpcSocket.at(strEndPoint), c_oStructuredBufferNewRequest);

            // This wait for response is a non-blocking and must be fulfilled within the required time
            ::PutTlsTransaction(c_poTlsNode, ::GetIpcTransaction(m_stlMapOfProcessToIpcSocket.at(strEndPoint), false));
        }
        else
        {
            StructuredBuffer oStructuredBufferFailureResponse;
            oStructuredBufferFailureResponse.PutDword("Status", 404);
            ::PutTlsTransaction(c_poTlsNode, oStructuredBufferFailureResponse.GetSerializedBuffer());
        }
    }
    catch (BaseException & oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class CommunicationPortal
 * @function StartServer
 * @brief Constructor to create a CommunicationPortal object
 * @param[in] poSocket Inout Request Params
 *
 ********************************************************************************************/

Socket * const __stdcall CommunicationPortal::RegisterProcess(
    _in const std::string & c_strProcessName
)
{
    __DebugFunction();

    // Establish a connection and register with the Communication Module
    Socket * poSocket = ::ConnectToUnixDomainSocket(gc_strCommunicationPortalAddress);

    StructuredBuffer oStructuredBufferProcessInformation;
    oStructuredBufferProcessInformation.PutString("ProcessName", c_strProcessName);

    ::PutIpcTransaction(poSocket, oStructuredBufferProcessInformation);

    return poSocket;
}
