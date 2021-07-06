/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CommandLine.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "GlobalMonitor.h"
#include "IpcTransactionHelperFunctions.h"
#include "RootOfTrustCore.h"
#include "StatusMonitor.h"
#include "SocketServer.h"
#include "StructuredBuffer.h"
#include "ThreadManager.h"

#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

/********************************************************************************************/

static RootOfTrustCore __stdcall RunInitializerProcess(void)
{
    __DebugFunction();
    
    // The ultimate goal of this function is to get the initialization parameters whic
    // are stored within a StructuredBuffer
    std::vector<Byte> stlSerializedParameters;
    // Create a unique IPC path. This value is very ephemeral and will not be required
    // past this function
    std::string strInitializerIpcPath = Guid().ToString(eRaw);
    // Create a TlsServer Unix Domain Socket using the unique IPC path. We need to start this
    // before calling fork() and execl() in order to prevent any sort of race condition that
    // might arise since the first thing the Initializer process does is connect to the
    // IPC tunnel with the RootOfTrust process
    SocketServer oSocketServer(strInitializerIpcPath.c_str());
    // Start the target application, which in this case is the initializer
    pid_t nProcessIdentifier = ::fork();
    _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "Fork has failed with errno = %d", errno);
    // Are we the child process or the parent process?
    if (0 == nProcessIdentifier)
    {
        // We are the child process. Let's just call execl and pass the IPC path as 
        // a parameter. The full command line will end up looking like "Initializer -ipc /SAIL/Ipc/C5133B267E92424896C4ACC898F30852"
        ::execl("InitializerProcess", "InitializerProcess", "-ipc", strInitializerIpcPath.c_str(), nullptr);
        ::exit(0);
    }
    else
    {
        // Spin until the InitializerProcess connects to the RootOfTrust process through IPC. This
        // could take a really long time since the InitializerProcess isn't going to connect
        // to the RootOfTrust process until it has been initialized, a process tha is asynchronous.
        // It could literally take hours and even days between starting up a naked VM and having
        // someone come along and use the initializer tool to initialize the VM. It should also
        // be noted that this is the only place where we do not check for a termination signal
        // since this VM is 'naked' and there is simply no way to send it a termination
        // signal.
        while (false == oSocketServer.WaitForConnection(1000))
        {
            // Put the thread into efficient sleep to give a chance for the other process
            // to connect. This reduces thread contention.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // There is a connection is waiting to be made!!!
        Socket * poSocket = oSocketServer.Accept();
        _ThrowBaseExceptionIf((nullptr == poSocket), "Unexpected nullptr returned from TlsServer.Accept()", nullptr);
        stlSerializedParameters = ::GetIpcTransaction(poSocket);
        // Close the connection
        poSocket->Release();
    }

    return RootOfTrustCore(stlSerializedParameters);
}

/********************************************************************************************/

static void __stdcall RunProcess(
    _in const char * c_szProcessName,
    _in const RootOfTrustCore & c_oRootOfTrustCore,
    _in const Guid & c_oDomainIdentifier
    )
{
    __DebugFunction();
    
    // Create a unique IPC path. This value is very ephemeral and will not be required
    // past this function
    std::string strTemporaryIpcPath = Guid().ToString(eRaw);
    // Create a TlsServer Unix Domain Socket using the unique IPC path. We need to start this
    // before calling fork() and execl() in order to prevent any sort of race condition that
    // might arise since the first thing the Initializer process does is connect to the
    // IPC tunnel with the RootOfTrust process
    SocketServer oSocketServer(strTemporaryIpcPath.c_str());
    // Start the target application, which in this case is the initializer
    pid_t nProcessIdentifier = ::fork();
    _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "Fork has failed with errno = %d", errno);
    // Are we the child process or the parent process?
    if (0 == nProcessIdentifier)
    {
        // We are the child process. Let's just call execl and pass the IPC path as 
        // a parameter. The full command line will end up looking like "DataDomainProcess -ipc /SAIL/Ipc/C5133B267E92424896C4ACC898F30852"
        ::execl(c_szProcessName, c_szProcessName, "-ipc", strTemporaryIpcPath.c_str(), nullptr);
    }
    else
    {
        // Wait until the remote process connects to the temporary Ipc
        while (false == oSocketServer.WaitForConnection(1000))
        {
            // Put the thread into efficient sleep to give a chance for the other process
            // to connect. This reduces thread contention.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // There is a connection is waiting to be made!!!
        Socket * poSocket = oSocketServer.Accept();
        _ThrowBaseExceptionIf((nullptr == poSocket), "Unexpected nullptr returned from TlsServer.Accept()", nullptr);
        // The connection has been made. At this point, we write the outgoing initialization
        // packet. There are only two elements in the initialization data
        // 1. Domain identifier of the process (either Computational or Data domain)
        // 2. The Ipc path to the RootOfTrustProcess
        StructuredBuffer oInitializationData;
        oInitializationData.PutGuid("YourDomainIdentifier", c_oDomainIdentifier);
        oInitializationData.PutString("RootOfTrustIpcPath", c_oRootOfTrustCore.GetRootOfTrustIpcPath());
        // Write the initialization data to the socket and wait for a response
        StructuredBuffer oResponse(::PutIpcTransactionAndGetResponse(poSocket, oInitializationData));
        // Close the connection
        poSocket->Release();
        // Throw an exception if one of the core processes failed to get started
        _ThrowBaseExceptionIf((false == oResponse.GetBoolean("Success")), "Failed to start the %d process", c_szProcessName);
    }
}

/********************************************************************************************/

int __cdecl main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();
    
    try
    {
        
        // Figure out whether or not the GlobalMonitor will be displaying status information
        // or not.
        StructuredBuffer oCommandLineParameters = ::ParseCommandLineParameters(nNumberOfArguments, (const char **) pszCommandLineArguments);
        // We need to start running the GlobalMonitor. We can run it with or without
        // actively displaying the status on STDOUT. Either way, the GlobalMonitor starts
        // a thread and will stay alive under the termination signal is found and until
        // all monitored elements have terminated or, several minutes have passed
        GlobalMonitor * poGlobalMonitor = ::GetGlobalMonitor();
        poGlobalMonitor->Run(oCommandLineParameters.IsElementPresent("DisplayStatus", BOOLEAN_VALUE_TYPE));
        // First we need to run the Initializer Process and then wait for the initialization
        // parameters to come in. This will end up initializing the root of trust engine
        // before it is able to run
        RootOfTrustCore oRootOfTrustCore = ::RunInitializerProcess();
        // Once the root of trust engine is initialized, we Run() the root of trust engine
        // which basically spawns it's own thread and waits for incoming connections. This
        // us basically what the RootOfTrust process does 99.9999% of the time.
        oRootOfTrustCore.RunIpcListener();
        // Now that the root of trust engine is running, we need to spin up the
        // DataConnector process. The Data Connector process will end up connecting
        // to the root of trust engine and query it for all the information that it needs
        // to initialize itself
        ::RunProcess("DataDomainProcess", oRootOfTrustCore, oRootOfTrustCore.GetDataDomainIdentifier());
        // Once the DataConnector process is running, we need to spin up the
        // the computation process. The computation process will end up connecting
        // to the root of trust engine and query it for all the information that it needs
        // to initialize itself
        ::RunProcess("ComputationalDomainProcess", oRootOfTrustCore, oRootOfTrustCore.GetComputationalDomainIdentifier());
        // Make sure to wait for all things 'RootOfTrust' to terminate before exiting
        // the try...catch block since doing so will destroy the RootOfTrust core object
        oRootOfTrustCore.WaitForTermination();
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
    }

    // All we have to do now is basically wait for ALL other threads to finish
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinAllThreads();
        
    return 0;
}