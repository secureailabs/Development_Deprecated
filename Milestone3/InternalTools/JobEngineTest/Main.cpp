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
#include "IpcTransactionHelperFunctions.h"
#include "SocketServer.h"
#include "TlsClient.h"
#include "TlsTransactionHelperFunctions.h"

#include <iostream>

enum class EngineRequest
{
    eVmShutdown = 0,
    ePushSafeObject = 1,
    eSubmitJob = 2,
    ePullData = 3,
    ePushdata = 4,
    eSetParameters = 5,
    eHaltAllJobs = 6,
    eJobStatusSignal = 7,
    eConnectVirtualMachine = 8
};

void SendRequestToJobEngine(
    _in TlsNode * poTlsNode,
    _in StructuredBuffer & oStructuredBufferRequest
)
{
    __DebugFunction();

    oStructuredBufferRequest.PutString("EndPoint", "JobEngine");
    ::SendTlsData(poTlsNode, oStructuredBufferRequest.GetSerializedBuffer());
}

bool TestConnectVm(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Connect Virtual Machine!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eConnectVirtualMachine);
    oStructuredBufferRequest.PutString("Eosb", "e0d937b9471e4d2ea470d0c96d21574b");
    oStructuredBufferRequest.PutString("Username", "user@domain.com");

    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    auto stlSerializedBuffer = ::GetTlsDataBlocking(poTlsNode);
    StructuredBuffer oNewRequest(stlSerializedBuffer);
    std::cout << oNewRequest.ToString() << std::endl;

    return true;
}

bool TestPushSafeObject(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Submit Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushSafeObject);

    oStructuredBufferRequest.PutString("SafeObjectUuid", "{e0d937b9-471e-4d2e-a470-d0c96d21574b}");
    std::string test_code = "print(\"Hello Orchestrator\")\n";
    test_code += "f= open(\"{b89aef4d-35a9-4713-80cb-2ca70ba45ba6}.{b1244b9a-6f02-4866-8e28-a25d5ddc94df}\",\"w+\")\n";
    test_code += "f.write(\"This is the output\")\n";
    test_code += "f.close()\n";
    // test_code += "while True:\n";
    // test_code += "    pass\n";

    oStructuredBufferRequest.PutBuffer("Payload", (Byte *)test_code.c_str(), test_code.length());

    StructuredBuffer oStructuredBufferOfParameters;
    StructuredBuffer oFirstParameter;
    oFirstParameter.PutString("Metadata", "Will be added in future based on SafeObject");
    oStructuredBufferOfParameters.PutStructuredBuffer("{460c2512-9c5e-49bf-b805-691bbc08e65e}", oFirstParameter);

    oStructuredBufferRequest.PutStructuredBuffer("ParameterList", oStructuredBufferOfParameters);

    StructuredBuffer oStructuredBufferOutputParameter;
    oStructuredBufferOutputParameter.PutString("Uuid","{b1244b9a-6f02-4866-8e28-a25d5ddc94df}");
    oStructuredBufferOutputParameter.PutString("Metadata", "todo: will be added in future");

    oStructuredBufferRequest.PutStructuredBuffer("OutputParameter", oStructuredBufferOutputParameter);

    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestSubmitJob(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Submit Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eSubmitJob);
    oStructuredBufferRequest.PutString("SafeObjectUuid", "{e0d937b9-471e-4d2e-a470-d0c96d21574b}");
    oStructuredBufferRequest.PutString("JobUuid", "{b89aef4d-35a9-4713-80cb-2ca70ba45ba6}");
    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestSetParameters(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Set Parameters!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eSetParameters);
    oStructuredBufferRequest.PutString("JobUuid", "{b89aef4d-35a9-4713-80cb-2ca70ba45ba6}");
    oStructuredBufferRequest.PutString("ParameterUuid", "{460c2512-9c5e-49bf-b805-691bbc08e65e}");
    oStructuredBufferRequest.PutString("ValueUuid", "{36236adb-6ad5-4735-8265-5fea96c5c9cd}");
    oStructuredBufferRequest.PutUnsignedInt32("ValuesExpected", 1);
    oStructuredBufferRequest.PutUnsignedInt32("ValueIndex", 0);

    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

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
    oStructuredBufferRequest.PutString("DataId", "{36236adb-6ad5-4735-8265-5fea96c5c9cd}");
    std::vector<Byte> stlDataToPush = {'a', 'b', 'c', 'd', 0};
    oStructuredBufferRequest.PutBuffer("Data", stlDataToPush);

    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

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
    oStructuredBufferRequest.PutString("Filename", "{b89aef4d-35a9-4713-80cb-2ca70ba45ba6}.{b1244b9a-6f02-4866-8e28-a25d5ddc94df}");

    // Send the request and the response will come as a signal when avaialble
    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    return true;
}

bool TestHaltJobs(
    _in TlsNode * poTlsNode
)
{
    __DebugFunction();

    std::cout << "Testing Halt all Jobs!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eHaltAllJobs);

    // Send the request and wait for
    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

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

        std::cout << "Connecting Job Engine!!" << std::endl;
        TlsNode * poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 3500);

        ::TestConnectVm(poTlsNode);
        ::TestSetParameters(poTlsNode);
        ::TestPushSafeObject(poTlsNode);
        ::TestSubmitJob(poTlsNode);
        ::TestPushData(poTlsNode);
        ::TestPullData(poTlsNode);
        // ::TestHaltJobs(poTlsNode);
        // ::TestVmShutdown(poTlsNode);

        // After sending all the jobs, wait for the signals
        do
        {
            std::cout << "Waiting for signals..\n";
            auto stlSerializedBuffer = ::GetTlsDataBlocking(poTlsNode);
            StructuredBuffer oNewRequest(stlSerializedBuffer);

            std::cout << oNewRequest.ToString() << std::endl;
        } while(true);

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