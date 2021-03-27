/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 22 March 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ConsoleInputHelperFunctions.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "TlsClient.h"
#include "StructuredBuffer.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <iterator>

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const char * gsc_szPrintableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szNumericCharacters = "0123456789";
static const char * gsc_szIpAddressCharacters = "0123456789.";
static const char * gsc_szAddRemoveNodeInputCharacters = "aArRdD";
static const char * gsc_szYesNoInputCharacters = "yYnN";

void BytesToFile(
    _in const std::string c_strFileName,
    _in const std::vector<Byte> c_stlFileData
)
{
    __DebugFunction();

    std::ofstream stlFileToWrite(c_strFileName, std::ios::out | std::ofstream::binary);
    std::copy(c_stlFileData.begin(), c_stlFileData.end(), std::ostreambuf_iterator<char>(stlFileToWrite));
    stlFileToWrite.close();
}

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
        _ThrowBaseException("Invalid File Path: %s", nullptr, c_strFileName.c_str());
    }
    return stlFileData;
}

void __stdcall PackageComputeServices()
{
    __DebugFunction();

    // Prepare the payload to send to the VM
    StructuredBuffer oPayloadToVm;
    // The instruction to execute after all the files are uploaded on the VM
    oPayloadToVm.PutString("Entrypoint", "./RootOfTrustProcess");

    // Set the type of VM, either it runs Computation or WebService
    oPayloadToVm.PutString("VirtualMachineType", "Computation");

    // A nested Structured Buffer containing all the executable files
    StructuredBuffer oFilesToPut;
    oFilesToPut.PutBuffer("RootOfTrustProcess", ::FileToBytes("RootOfTrustProcess"));
    oFilesToPut.PutBuffer("InitializerProcess", ::FileToBytes("InitializerProcess"));
    oFilesToPut.PutBuffer("SignalTerminationProcess", ::FileToBytes("SignalTerminationProcess"));
    oFilesToPut.PutBuffer("DataDomainProcess", ::FileToBytes("DataDomainProcess"));
    oFilesToPut.PutBuffer("ComputationalDomainProcess", ::FileToBytes("ComputationalDomainProcess"));
    oFilesToPut.PutBuffer("/usr/local/lib/python3.8/dist-packages/_DataConnector.so", ::FileToBytes("libDataConnector.so"));

    oPayloadToVm.PutStructuredBuffer("ExecutableFiles", oFilesToPut);

    ::BytesToFile("SecureComputationalVirtualMachine.binaries", oPayloadToVm.GetSerializedBuffer());
}

void __stdcall PackageWebService()
{
    __DebugFunction();

    StructuredBuffer oPayloadToVm;

    // The instruction to execute after all the files are uploaded on the VM
    oPayloadToVm.PutString("Entrypoint", "./RestApiPortal");

    // Set the type of VM, either it runs Computation or WebService
    oPayloadToVm.PutString("VirtualMachineType", "WebService");

    // A nested Structured Buffer containing all the executable files
    StructuredBuffer oFilesToPut;
    oFilesToPut.PutBuffer("DatabaseGateway", ::FileToBytes("DatabaseGateway"));
    oFilesToPut.PutBuffer("RestApiPortal", ::FileToBytes("RestApiPortal"));
    oFilesToPut.PutBuffer("SharedLibraries/DatabasePortal/libDatabaseManager.so", ::FileToBytes("SharedLibraries/DatabasePortal/libDatabaseManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libAccountDatabase.so", ::FileToBytes("SharedLibraries/RestApiPortal/libAccountDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libCryptographicKeyManagement.so", ::FileToBytes("SharedLibraries/RestApiPortal/libCryptographicKeyManagement.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libDigitalContractDatabase.so", ::FileToBytes("SharedLibraries/RestApiPortal/libDigitalContractDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libVirtualMachineManager.so", ::FileToBytes("SharedLibraries/RestApiPortal/libVirtualMachineManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libAuditLogManager.so", ::FileToBytes("SharedLibraries/RestApiPortal/libAuditLogManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libDatasetDatabase.so", ::FileToBytes("SharedLibraries/RestApiPortal/libDatasetDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libSailAuthentication.so", ::FileToBytes("SharedLibraries/RestApiPortal/libSailAuthentication.so"));

    // oFilesToPut.PutBuffer("", ::FileToBytes(""));
    oPayloadToVm.PutStructuredBuffer("ExecutableFiles", oFilesToPut);

    ::BytesToFile("WebServicesPortal.binaries", oPayloadToVm.GetSerializedBuffer());
}

/********************************************************************************************/

int __cdecl main(
    int nNumberOfArguments,
    char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    try
    {
        ::PackageWebService();
        ::PackageComputeServices();
    }

    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
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
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}
