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
#include "DataConnector.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "RootOfTrustNode.h"
#include "StructuredBuffer.h"
#include "SocketServer.h"
#include "StatusMonitor.h"
#include "ThreadManager.h"

#include <iostream>

static RootOfTrustNode * gs_poRootOfTrustNode = nullptr;

/********************************************************************************************/

static void * __stdcall InitializeResearchProcess(
    _in void * poParameter
    )
{
    __DebugFunction();

    Socket * poResearchProcessSocket = (Socket *) poParameter;
    DataConnector * poDataConnector = ::GetDataConnector();
    poDataConnector->HandleRequestsUntilClose(poResearchProcessSocket);

    return nullptr;
}

/********************************************************************************************/

static void __cdecl InitDataConnector(void)
{
    __DebugFunction();
    
    // Get the singleton DataConnector object
    DataConnector * poDataConnector = ::GetDataConnector();
    __DebugAssert(nullptr != poDataConnector);
    // Pull the dataset from the Root of Trust
    std::vector<Byte> c_stlDataset = gs_poRootOfTrustNode->GetDataset();
    // Load and Verify the contents of the dataset
    _ThrowBaseExceptionIf((false == poDataConnector->LoadAndVerify(c_stlDataset, gs_poRootOfTrustNode)), "Failed to verify the integrity of the DataSet", nullptr);
    // Get ready to wait for incoming connections. This includes setting up the
    // TerminationSignal object, the ThreadManager object as well as starting up
    // the SocketServer
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    StatusMonitor oStatusMonitor("static void __cdecl InitDataConnector()");
    SocketServer oSocketServer("/tmp/{0bd8a254-49e4-4b86-b1b8-f353c18013c5}");
    unsigned int unSpinner = 0;
    do
    {
        if (true == oSocketServer.WaitForConnection(1000))
        {
            Socket * poSocket = oSocketServer.Accept();
            if (nullptr != poSocket)
            {
                poThreadManager->CreateThread(nullptr, InitializeResearchProcess, (void *) poSocket);
            }
        }
    }
    while (false == oStatusMonitor.IsTerminating());

    // Wait for all of the threads to gracefull exit
    poThreadManager->JoinAllThreads();
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
        // Parse the command line
        StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);
        // First we initialize the RootOfTrustNode. Without that, nothing else matters
        gs_poRootOfTrustNode = new RootOfTrustNode(oCommandLineArguments.GetString("ipc").c_str());
        // Insert process specific code here. For now, just some stupid test code
        ::InitDataConnector();
    }

    catch (BaseException oException)
    {
        std::cout << "DataConnector" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m" << oException.GetExceptionMessage() << "\033[0m" << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    
    catch (...)
    {
        std::cout << "DataConnector" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    
    return 0;
}