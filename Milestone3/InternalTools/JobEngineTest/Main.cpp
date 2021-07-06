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
    eJobStatusSignal = 7
};

bool TestPushSafeObject(
    _in Socket * poSocket
)
{
    __DebugFunction();

    std::cout << "Testing Submit Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushSafeObject);

    oStructuredBufferRequest.PutString("SafeObjectUuid", "{e0d937b9-471e-4d2e-a470-d0c96d21574b}");
    std::string test_code = "print(\"Hello Orchestrator\")\n";
    test_code += "f= open(\"{abf0a5ad-21a8-4b91-a4b6-07e09c9d8467}\",\"w+\")\n";
    test_code += "f.write(\"This is the output\")\n";
    test_code += "f.close()\n";
    // test_code += "while True:\n";
    // test_code += "    pass\n";

    oStructuredBufferRequest.PutBuffer("Payload", (Byte *)test_code.c_str(), test_code.length());

    StructuredBuffer oStructuredBufferOfParameters;

    StructuredBuffer oFirstParameter;

    oStructuredBufferOfParameters.PutStructuredBuffer("{460c2512-9c5e-49bf-b805-691bbc08e65e}", oFirstParameter);
    oStructuredBufferRequest.PutStructuredBuffer("ParameterList", oStructuredBufferOfParameters);

    // oStructuredBufferRequest.PutString("ResultId", "ResultId");

    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

    return true;
}

bool TestSubmitJob(
    _in Socket * poSocket
)
{
    __DebugFunction();

    std::cout << "Testing Submit Job!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eSubmitJob);
    oStructuredBufferRequest.PutString("SafeObjectUuid", "{e0d937b9-471e-4d2e-a470-d0c96d21574b}");
    oStructuredBufferRequest.PutString("JobUuid", "{b89aef4d-35a9-4713-80cb-2ca70ba45ba6}");
    oStructuredBufferRequest.PutString("OutFileName", "{abf0a5ad-21a8-4b91-a4b6-07e09c9d8467}");
    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

    return true;
}

bool TestSetParameters(
    _in Socket * poSocket
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

    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

    return true;
}

bool TestPushData(
    _in Socket * poSocket
)
{
    __DebugFunction();

    std::cout << "Testing Push Data!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushdata);
    oStructuredBufferRequest.PutString("DataId", "{36236adb-6ad5-4735-8265-5fea96c5c9cd}");
    std::vector<Byte> stlDataToPush = {'a', 'b', 'c', 'd', 0};
    oStructuredBufferRequest.PutBuffer("Data", stlDataToPush);

    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

    return true;
}

bool TestPullData(
    _in Socket * poSocket
)
{
    __DebugFunction();

    std::cout << "Testing Pull Data!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePullData);
    oStructuredBufferRequest.PutString("Filename", "{abf0a5ad-21a8-4b91-a4b6-07e09c9d8467}");

    // Send the request and the response will come as a signal when avaialble
    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

    // In a real use-case scenario the orchestrator will make an async call
    // to get the data file. It will not wait for it. Instead, as soon as it
    // sends the request, it will wait for the file to exist on it's own filesystem.
    // The orchestrator process will also be running a server which will be taking in
    // data from the JobEngine and acting upon it as needed.

    // This is just a workaround to test this feature.
    // Put a huge timout so that it can just wait for infinite. 11
    // StructuredBuffer oStrucutredBufferData(::GetIpcTransaction(poSocket));

    // std::cout << "Got the data: " << oStrucutredBufferData.GetBuffer("FileData").data() << std::endl;

    return true;
}

bool TestHaltJobs(
    _in Socket * poSocket
)
{
    __DebugFunction();

    std::cout << "Testing Halt all Jobs!!" << std::endl;

    StructuredBuffer oStructuredBufferRequest;
    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eHaltAllJobs);

    // Send the request and wait for
    ::PutIpcTransaction(poSocket, oStructuredBufferRequest);

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

        std::cout << "Waiting for connection form Job Engine!!" << std::endl;
        SocketServer oSocketServer("{164085b7-ef20-4257-bc14-e1e08c908aaa}");

        Socket * poSocket = nullptr;
        do
        {
            if (true == oSocketServer.WaitForConnection(1000))
            {
                poSocket = oSocketServer.Accept();
                if (nullptr != poSocket)
                {
                    _ThrowIfNull(poSocket, "Can't connect to socket client.", nullptr);
                    break;
                }
            }
        } while (true);

        ::TestPushSafeObject(poSocket);
        ::TestSubmitJob(poSocket);
        ::TestSetParameters(poSocket);
        ::TestPushData(poSocket);
        ::TestPullData(poSocket);
        // ::TestHaltJobs(poSocket);
        // ::TestVmShutdown(poSocket);
        // ::TestPushSafeObject(poSocket);
        // ::TestSubmitJob(poSocket);
        // ::TestSetParameters(poSocket);
        // ::TestPushData(poSocket);
        // ::TestPullData(poSocket);

        // After sending all the jobs, wait for the signals
        do
        {
            std::cout << "Waiting for signals..\n";
            auto stlSerializedBuffer = ::GetIpcTransaction(poSocket);
            StructuredBuffer oNewRequest(stlSerializedBuffer);

            std::cout << oNewRequest.ToString() << std::endl;
        } while(true);

        poSocket->Release();
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