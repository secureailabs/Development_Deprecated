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

    std::vector<Byte> stlSerializedParameters;
    TlsServer oTlsServer(6800);
    while (false == oTlsServer.WaitForConnection(1000))
    {
        // Put the thread into efficient sleep to give a chance for the other process
        // to connect. This reduces thread contention.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // There is a connection is waiting to be made!!!
    TlsNode * poTlsNode = oTlsServer.Accept();
    _ThrowBaseExceptionIf((nullptr == poTlsNode), "Unexpected nullptr returned from TlsServer.Accept()", nullptr);
    stlSerializedParameters = ::GetPayload(poTlsNode, 10*1000);

    StructuredBuffer oStructuredBufferResponse;
    oStructuredBufferResponse.PutString("Status", "Success");

    JsonValue * poJson = JsonValue::ParseStructuredBufferToJson(oStructuredBufferResponse);
    ::PutResponse(poTlsNode, poJson->ToString());

    // Close the connection
    poTlsNode->Release();

    return stlSerializedParameters;
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
