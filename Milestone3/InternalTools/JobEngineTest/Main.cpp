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
#include <fstream>

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

std::string g_strSafeObjectId = "D918DF8B49694EA7940B6EA1C16D98C7";
std::string g_strParameter0 = "";
std::string g_strParameter1 = "";
std::string g_strOutputId = "";
std::string g_strJobId = "4A4A340EA3ED49D9871418D1B25FC722";

std::vector<Byte> FileToBytes(
    const std::string c_strFileName
)
{
    __DebugFunction();

    std::vector<Byte> stlFileData;

    std::ifstream stlFile(c_strFileName.c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        unsigned int unFileSizeInBytes = (unsigned int) stlFile.tellg();
        stlFileData.resize(unFileSizeInBytes);
        stlFile.seekg(0, std::ios::beg);
        stlFile.read((char *)stlFileData.data(), unFileSizeInBytes);
        stlFile.close();
    }
    else
    {
        _ThrowBaseException("Invalid File Path", nullptr);
    }
    return stlFileData;
}

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

    StructuredBuffer oSafeObject(::FileToBytes(g_strSafeObjectId+".safe"));

    oStructuredBufferRequest.PutString("SafeObjectUuid", oSafeObject.GetString("Uuid"));
    std::string strTestCode = oSafeObject.GetString("Payload");
    oStructuredBufferRequest.PutBuffer("Payload", (Byte *)strTestCode.c_str(), strTestCode.length());

    StructuredBuffer oStructuredBufferOfParameters;
    StructuredBuffer oFirstParameter;
    oFirstParameter.PutString("Metadata", "Will be added in future based on SafeObject");
    g_strParameter0 = oSafeObject.GetStructuredBuffer("InputParamters").GetStructuredBuffer("0").GetString("Uuid");
    g_strParameter1 = oSafeObject.GetStructuredBuffer("InputParamters").GetStructuredBuffer("1").GetString("Uuid");
    oStructuredBufferOfParameters.PutStructuredBuffer(g_strParameter0.c_str(), oFirstParameter);
    oStructuredBufferOfParameters.PutStructuredBuffer(g_strParameter1.c_str(), oFirstParameter);

    oStructuredBufferRequest.PutStructuredBuffer("ParameterList", oStructuredBufferOfParameters);

    StructuredBuffer oStructuredBufferOutputParameter;
    g_strOutputId = oSafeObject.GetStructuredBuffer("OutputParamter").GetString("Uuid");
    oStructuredBufferOutputParameter.PutString("Uuid", g_strOutputId);
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
    oStructuredBufferRequest.PutString("SafeObjectUuid", g_strSafeObjectId);
    oStructuredBufferRequest.PutString("JobUuid", g_strJobId);
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
    oStructuredBufferRequest.PutString("JobUuid", g_strJobId);
    oStructuredBufferRequest.PutString("ParameterUuid", g_strParameter0);
    oStructuredBufferRequest.PutString("ValueUuid", "EAF3AF0900DB4660A22945ADB27E4205");
    oStructuredBufferRequest.PutUnsignedInt32("ValuesExpected", 1);
    oStructuredBufferRequest.PutUnsignedInt32("ValueIndex", 0);

    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::eSetParameters);
    oStructuredBufferRequest.PutString("JobUuid", g_strJobId);
    oStructuredBufferRequest.PutString("ParameterUuid", g_strParameter1);
    oStructuredBufferRequest.PutString("ValueUuid", "6C56C7C1F6A94005938B19E58CAA90A8");
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
    oStructuredBufferRequest.PutString("DataId", "EAF3AF0900DB4660A22945ADB27E4205");
    oStructuredBufferRequest.PutBuffer("Data", ::FileToBytes("value2"));
    ::SendRequestToJobEngine(poTlsNode, oStructuredBufferRequest);

    oStructuredBufferRequest.PutByte("RequestType", (Byte)EngineRequest::ePushdata);
    oStructuredBufferRequest.PutString("DataId", "6C56C7C1F6A94005938B19E58CAA90A8");
    oStructuredBufferRequest.PutBuffer("Data", ::FileToBytes("value3"));
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
    oStructuredBufferRequest.PutString("Filename", g_strJobId+"."+g_strOutputId);

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
        ::TestPushSafeObject(poTlsNode);
        ::TestSetParameters(poTlsNode);
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