/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 28 Sep 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CommandLine.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "TlsTransactionHelperFunctions.h"

#include <iostream>

enum class EngineRequest
{
    eShutdown = 0,
    ePushSafeObject = 1,
    eSubmitJob = 2,
    ePullData = 3,
    ePushdata = 4,
    eSetParameters = 5,
    eHaltAllJobs = 6,
    eJobStatusSignal = 7
};


bool TestPushSafeObject(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Submit Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushSafeObject);

    oStructuredBufferRequest.PutString("SafeObjectUuid", "safeObjectFileName");
    std::string test_code = "print(\"Hello World\")\n";
    oStructuredBufferRequest.PutBuffer("Payload", (Byte *)test_code.c_str(), test_code.length());

    // StructuredBuffer oStructuredBufferOfParameters;
    // oStructuredBufferOfParameters.PutString("1", "inputParameter");
    // oStructuredBufferRequest.PutStructuredBuffer("Parameters", oStructuredBufferOfParameters);

    // oStructuredBufferRequest.PutString("ResultId", "ResultId");

    ::PutTlsTransaction(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestRunJob(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Run Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    // oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eRunJob);
    oStructuredBufferRequest.PutString("SafeObjectId", "safeObjectFileName");
    ::PutTlsTransaction(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestPushData(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Push Data!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushdata);
    oStructuredBufferRequest.PutString("DataId", "inputParameter");
    std::vector<Byte> stlDataToPush = {'a', 'b', 'c', 'd', 0};
    oStructuredBufferRequest.PutBuffer("Data", stlDataToPush);

    ::PutTlsTransaction(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestPullData(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Pull Data!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePullData);
    oStructuredBufferRequest.PutString("Filename", "outputFile");

    // Send the request and wait for
    ::PutTlsTransaction(poTlsNode, oStructuredBufferRequest);

    // In a real use-case scenario the orchestrator will make an async call
    // to get the data file. It will not wait for it. Instead, as soon as it
    // sends the request, it will wait for the file to exist on it's own filesystem.
    // The orchestrator process will also be running a server which will be taking in
    // data from the JobEngine and acting upon it as needed.

    // This is just a workaround to test this feature.
    // Put a huge timout so that it can just wait for infinite. 11
    StructuredBuffer oStrucutredBufferData(::GetTlsTransaction(poTlsNode, 1000000000));

    std::cout << "Got the data: " << oStrucutredBufferData.GetBuffer("FileData").data() << std::endl;

    return true;
}

bool TestEndConnection(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Server Shut!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eShutdown);

    // Send the request and wait for
    ::PutTlsTransaction(poTlsNode, oStructuredBufferRequest);

    return true;
}

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

        std::cout << "Connecting to Job Engine!!" << std::endl;
        TlsNode * poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 8888);

        ::TestPushSafeObject(poTlsNode);
        // ::TestRunJob(poTlsNode);
        ::TestPushData(poTlsNode);
        ::TestPullData(poTlsNode);
        // ::TestEndConnection(poTlsNode);

        poTlsNode->Release();
    }

    catch (BaseException oException)
    {
        std::cout << "JobEngineTest" << std::endl
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
        std::cout << "JobEngineTest" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}