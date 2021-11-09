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

void __thiscall CommunicationPortal::PersistantConnectionTlsToIpc(
    _in TlsNode * const c_poTlsNode,
    _in StructuredBuffer c_oStructuredBufferMessageToPass
)
{
    __DebugFunction();

    try
    {
        bool fKeepRunning = true;
        StructuredBuffer oStructuredBufferMessageToPass = c_oStructuredBufferMessageToPass;
        do
        {
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
            oStructuredBufferMessageToPass = StructuredBuffer(::GetTlsTransaction(c_poTlsNode, 0));
        }
        while(true == fKeepRunning);
    }
    catch (BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    std::cout << "\n\n\n Exiting PersistantConnectionTlsToIpc. Something happened.\n\n\n";
    fflush(stdout);
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
    _in Socket * const c_poSocketIpc,
    _in TlsNode * const c_poTlsNode
)
{
    __DebugFunction();

    bool fKeepRunning = true;
    do
    {
        StructuredBuffer oStructuredBuffer(::GetIpcTransaction(c_poSocketIpc, true));
        if (oStructuredBuffer.IsElementPresent("KeepAlive", BOOLEAN_VALUE_TYPE))
        {
            if (false == oStructuredBuffer.GetBoolean("KeepAlive"))
            {
                fKeepRunning = false;
            }
        }
        ::PutTlsTransaction(c_poTlsNode, oStructuredBuffer.GetSerializedBuffer());
    }
    while(fKeepRunning);
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
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        StructuredBuffer oStructuredBufferNewRequest(::GetTlsTransaction(c_poTlsNode, 0));

        // If the connection was established with the JobEngine it will be a persistant connection,
        // otherwise it will be a single transaction function.
        std::string strEndPoint = oStructuredBufferNewRequest.GetString("EndPoint");
        if ("JobEngine" == strEndPoint)
        {
            // A new thread is created for the reading requests on the TlsNode and forwarding them
            // to the IPC connection while forwarding the data read on IPC connection will be handled
            // in this thread.
            std::thread(&CommunicationPortal::PersistantConnectionTlsToIpc, this, c_poTlsNode, oStructuredBufferNewRequest).detach();

            // This is blocking call and will exit only when the end connection signal is received via IPC
            // from the JobEngine
            this->PersistantConnectionIpcToTls(m_stlMapOfProcessToIpcSocket.at(strEndPoint), c_poTlsNode);
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
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (std::exception & oException)
    {
        std::cout << "std::exception " << oException.what() << std::endl;
        ::RegisterUnknownException(__func__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
