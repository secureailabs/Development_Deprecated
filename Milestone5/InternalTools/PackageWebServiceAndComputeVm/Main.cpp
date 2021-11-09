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
#include "FileUtils.h"

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

void __stdcall PackageComputeServices()
{
    __DebugFunction();

    // Prepare the payload to send to the VM
    StructuredBuffer oPayloadToVm;
    // The instruction to execute after all the files are uploaded on the VM
    oPayloadToVm.PutString("Entrypoint", "./CommunicationPortal");

    // Set the type of VM, either it runs Computation or WebService
    oPayloadToVm.PutString("VirtualMachineType", "Computation");

    // A nested Structured Buffer containing all the executable files
    StructuredBuffer oFilesToPut;
    oFilesToPut.PutBuffer("RootOfTrustProcess", ::ReadFileAsByteBuffer("RootOfTrustProcess"));
    oFilesToPut.PutBuffer("CommunicationPortal", ::ReadFileAsByteBuffer("CommunicationPortal"));
    oFilesToPut.PutBuffer("InitializerProcess", ::ReadFileAsByteBuffer("InitializerProcess"));
    oFilesToPut.PutBuffer("SignalTerminationProcess", ::ReadFileAsByteBuffer("SignalTerminationProcess"));
    oFilesToPut.PutBuffer("DataDomainProcess", ::ReadFileAsByteBuffer("DataDomainProcess"));
    oFilesToPut.PutBuffer("JobEngine", ::ReadFileAsByteBuffer("JobEngine"));
    oFilesToPut.PutBuffer("PrivacySentinel.py", ::ReadFileAsByteBuffer("PrivacySentinel.py"));
    oFilesToPut.PutBuffer("PrivacySentinelPolicy.json", ::ReadFileAsByteBuffer("PrivacySentinelPolicy.json"));

    oPayloadToVm.PutStructuredBuffer("ExecutableFiles", oFilesToPut);

    ::WriteBytesAsFile("SecureComputationalVirtualMachine.binaries", oPayloadToVm.GetSerializedBuffer());
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
    oFilesToPut.PutBuffer("DatabaseGateway", ::ReadFileAsByteBuffer("DatabaseGateway"));
    oFilesToPut.PutBuffer("RestApiPortal", ::ReadFileAsByteBuffer("RestApiPortal"));
    oFilesToPut.PutBuffer("SharedLibraries/DatabasePortal/libDatabaseManager.so", ::ReadFileAsByteBuffer("SharedLibraries/DatabasePortal/libDatabaseManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libAccountDatabase.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libAccountDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libCryptographicKeyManagement.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libCryptographicKeyManagement.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libDigitalContractDatabase.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libDigitalContractDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libVirtualMachineManager.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libVirtualMachineManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libAuditLogManager.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libAuditLogManager.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libDatasetDatabase.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libDatasetDatabase.so"));
    oFilesToPut.PutBuffer("SharedLibraries/RestApiPortal/libSailAuthentication.so", ::ReadFileAsByteBuffer("SharedLibraries/RestApiPortal/libSailAuthentication.so"));

    // oFilesToPut.PutBuffer("", ::ReadFileAsByteBuffer(""));
    oPayloadToVm.PutStructuredBuffer("ExecutableFiles", oFilesToPut);

    ::WriteBytesAsFile("WebServicesPortal.binaries", oPayloadToVm.GetSerializedBuffer());
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
