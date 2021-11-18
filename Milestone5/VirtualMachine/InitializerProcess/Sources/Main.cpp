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
#include "HardCodedCryptographicKeys.h"
#include "TlsTransactionHelperFunctions.h"
#include "IpcTransactionHelperFunctions.h"
#include "SocketClient.h"
#include "TlsServer.h"
#include "StructuredBuffer.h"
#include "JsonValue.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>

/********************************************************************************************/

static std::vector<Byte> __stdcall WaitForInitializationParameters(void)
{
    __DebugFunction();

    StructuredBuffer oAllInitializationParameters;

    TlsServer oTlsServer(6800);
    while (false == oTlsServer.WaitForConnection(1000))
    {
        // Put the thread into efficient sleep to give a chance for the other process
        // to connect. This reduces thread contention.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // The first connection are the initialization parameters sent from the
    // SAIL backend which include the digital contract and VM information
    TlsNode * poTlsNode = oTlsServer.Accept();
    _ThrowBaseExceptionIf((nullptr == poTlsNode), "Unexpected nullptr returned from TlsServer.Accept()", nullptr);

    auto stlInitializationParameters = ::GetTlsTransaction(poTlsNode, 60*10*1000);
    _ThrowBaseExceptionIf((0 == stlInitializationParameters.size()), "Unexpected empty initialization parameters", nullptr);

    StructuredBuffer oBackendInitializationParameters(stlInitializationParameters);
    oAllInitializationParameters.PutString("NameOfVirtualMachine", oBackendInitializationParameters.GetString("NameOfVirtualMachine"));
    oAllInitializationParameters.PutString("IpAddressOfVirtualMachine", oBackendInitializationParameters.GetString("IpAddressOfVirtualMachine"));
    oAllInitializationParameters.PutString("VirtualMachineIdentifier", oBackendInitializationParameters.GetString("VirtualMachineIdentifier"));
    oAllInitializationParameters.PutString("ClusterIdentifier", oBackendInitializationParameters.GetString("ClusterIdentifier"));
    oAllInitializationParameters.PutString("DigitalContractIdentifier", oBackendInitializationParameters.GetString("DigitalContractIdentifier"));
    oAllInitializationParameters.PutString("DatasetIdentifier", oBackendInitializationParameters.GetString("DatasetIdentifier"));
    oAllInitializationParameters.PutString("RootOfTrustDomainIdentifier", oBackendInitializationParameters.GetString("RootOfTrustDomainIdentifier"));
    oAllInitializationParameters.PutString("ComputationalDomainIdentifier", oBackendInitializationParameters.GetString("ComputationalDomainIdentifier"));
    oAllInitializationParameters.PutString("DataConnectorDomainIdentifier", oBackendInitializationParameters.GetString("DataConnectorDomainIdentifier"));
    oAllInitializationParameters.PutString("VmEosb", oBackendInitializationParameters.GetString("VmEosb"));

    StructuredBuffer oStructuredBufferResponse;
    oStructuredBufferResponse.PutString("Status", "Success");
    ::PutTlsTransaction(poTlsNode, oStructuredBufferResponse);
    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
        poTlsNode = nullptr;
    }

    // Next step is to wait for the dataset from the Remote data connector, the process will wait for the dataset
    // and dataOwner information
    while (false == oTlsServer.WaitForConnection(1000))
    {
        // Put the thread into efficient sleep to give a chance for the other process
        // to connect. This reduces thread contention.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // The second connection are the initialization parameters sent from the
    // Remote data connector which has information about the dataowner and the dataset
    poTlsNode = oTlsServer.Accept();
    _ThrowBaseExceptionIf((nullptr == poTlsNode), "Unexpected nullptr returned from TlsServer.Accept()", nullptr);

    auto stlDatasetParameters = ::GetTlsTransaction(poTlsNode, 60*60*1000);
    _ThrowBaseExceptionIf((0 == stlDatasetParameters.size()), "Unexpected empty dataset parameters", nullptr);

    StructuredBuffer oRemoteDatasetParameters(stlDatasetParameters);
    oAllInitializationParameters.PutString("DataOwnerAccessToken", ::UnEscapeJsonString(oRemoteDatasetParameters.GetString("DataOwnerAccessToken").c_str()));
    oAllInitializationParameters.PutString("SailWebApiPortalIpAddress", oRemoteDatasetParameters.GetString("SailWebApiPortalIpAddress"));
    oAllInitializationParameters.PutString("DataOwnerUserIdentifier", oRemoteDatasetParameters.GetString("DataOwnerUserIdentifier"));
    oAllInitializationParameters.PutString("Base64EncodedDataset", ::UnEscapeJsonString(oRemoteDatasetParameters.GetString("Base64EncodedDataset").c_str()));
    oAllInitializationParameters.PutString("DataOwnerOrganizationIdentifier", oRemoteDatasetParameters.GetString("DataOwnerOrganizationIdentifier"));

    ::PutTlsTransaction(poTlsNode, oStructuredBufferResponse);
    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
        poTlsNode = nullptr;
    }

    return oAllInitializationParameters.GetSerializedBuffer();
}

/********************************************************************************************/

static void __stdcall InitializeRootOfTrust(
    _in const char * c_szIpcPathForInitialization,
    _in const std::vector<Byte> & c_oSerializedInitializationParameters
    )
{
    __DebugFunction();

    StructuredBuffer oInitializationData(c_oSerializedInitializationParameters);
    Socket * poSocket = ::ConnectToUnixDomainSocket(c_szIpcPathForInitialization);
    (void) ::PutIpcTransaction(poSocket, oInitializationData);
    poSocket->Release();
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
        std::cout << ">>> Initializer process" << std::endl;
        // Parse the command line
        StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);
        // First we wait for the initialization parameters
        std::vector<Byte> stdInitializationParameters = ::WaitForInitializationParameters();
        // Now we initialize the RootOfTrustNode. Without that, we will not know how to
        // communicate securely back to the RootOfTrust to initialize it.
        ::InitializeRootOfTrust(oCommandLineArguments.GetString("ipc").c_str(), stdInitializationParameters);
        std::cout << "<<< Initializer process" << std::endl;
    }

    catch (BaseException oException)
    {
        std::cout << "InitializerProcess" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
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
        std::cout << "InitializerProcess" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}
