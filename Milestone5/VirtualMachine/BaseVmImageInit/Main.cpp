/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 09 March 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "StructuredBuffer.h"
#include "SocketServer.h"
#include "TlsServer.h"
#include "TlsTransactionHelperFunctions.h"
#include "JsonValue.h"
#include "FileUtils.h"
#include "CurlRest.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <unistd.h>

/********************************************************************************************
 *
 * @function InitVirtualMachine
 * @brief Download the files and execute them on the Virtual Machine
 *
 ********************************************************************************************/

void __cdecl InitVirtualMachine(
    )
{
    __DebugFunction();

    TlsServer oTlsServer(9090);
    TlsNode * poTlsNode = nullptr;

    StructuredBuffer oResponseStructuredBuffer;
    oResponseStructuredBuffer.PutString("Status", "Fail");

    bool fSuccess = false;
    while(false == fSuccess)
    {
        // We will first try to download all the incoming package data that needs to be installed
        // on the VM and if this fails we try again.
        try
        {
            if (true == oTlsServer.WaitForConnection(1000))
            {
                std::cout << "New Connection" << std::endl;
                poTlsNode = oTlsServer.Accept();
                _ThrowIfNull(poTlsNode, "Cannot establish connection.", nullptr);

                // Fetch the serialized Structure Buffer from the remote
                std::vector<Byte> stlPayload = ::GetTlsTransaction(poTlsNode, 5*60*1000);
                _ThrowBaseExceptionIf((0 == stlPayload.size()), "Bad Initialization data", nullptr);

                StructuredBuffer oGetBinaryPackageInstructions(stlPayload);
                std::string strVerb = oGetBinaryPackageInstructions.GetString("Verb");
                std::string strContent = oGetBinaryPackageInstructions.GetString("Content");
                std::string strApiUri = oGetBinaryPackageInstructions.GetString("Uri");
                std::string strHost = oGetBinaryPackageInstructions.GetString("Host");

                std::vector<std::string> stlHeader;
                stlHeader.push_back("Host: " + strHost);
                stlHeader.push_back("Content-Length: " + std::to_string(strContent.length()));
                long nResponseCode = 0;
                std::vector<Byte> stlPackagedBinaries = ::RestApiCall(strHost, 443, strVerb, strApiUri, "", false, stlHeader, &nResponseCode);
                _ThrowBaseExceptionIf((200 != nResponseCode), "Failed to get Binaries package. Response code: %d", nResponseCode);
                _ThrowBaseExceptionIf((0 >= stlPackagedBinaries.size()), "Invalid Package received.", nullptr );

                // Deserialize the buffer
                StructuredBuffer oVmInitializationInstructions(stlPackagedBinaries);
                std::string strVmType = oVmInitializationInstructions.GetString("VirtualMachineType");
                if ("WebService" == strVmType)
                {
                    _ThrowBaseExceptionIf((-1 == ::mkdir("SharedLibraries", 0777)), "Could not create Shared Library", nullptr);
                    _ThrowBaseExceptionIf((-1 == ::mkdir("SharedLibraries/DatabasePortal", 0777)), "Could not create Shared Library", nullptr);
                    _ThrowBaseExceptionIf((-1 == ::mkdir("SharedLibraries/RestApiPortal", 0777)), "Could not create Shared Library", nullptr);
                }
                // There are two kinds of files that come with the structuredBuffer
                // The executable file and the non-executable files.
                // The executable files need execute permission while the normal files don't
                StructuredBuffer oFilesStructuredBuffer = oVmInitializationInstructions.GetStructuredBuffer("ExecutableFiles");
                std::vector<std::string> stlFilesList = oFilesStructuredBuffer.GetNamesOfElements();
                for (int i = 0; i < stlFilesList.size(); i++)
                {
                    std::cout << "File " << i << " " << stlFilesList.at(i) << std::endl;
                    ::WriteBytesAsFile(stlFilesList.at(i), oFilesStructuredBuffer.GetBuffer(stlFilesList.at(i).c_str()));
                    ::chmod(stlFilesList.at(i).c_str(), S_IRWXU);
                }
                // There are no non-executable files as of now but would be added sooner or later.

                // Once all the files are copied to the required location the Entrypoint is
                // executed in the forked process adn this process exits with a response.
                pid_t nProcessIdentifier = ::fork();
                _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "Fork has failed with errno = %d", errno);

                if (0 == nProcessIdentifier)
                {
                    // Child Process
                    // We need to forcefully destroy the Tls server as it acquires the lifetime of the child
                    // process and does not shut down the socket server.
                    oTlsServer.~TlsServer();
                    if (nullptr != poTlsNode)
                    {
                        poTlsNode->Release();
                    }

                    if ("WebService" == strVmType)
                    {
                        // We need two child process in Web Service, one for the "DatabaseGateway"
                        // while other for the RestApiPortal
                        pid_t nWebServiceFork = ::fork();
                        _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "WebService has failed with errno = %d", errno);

                        if (0 == nWebServiceFork)
                        {
                            // Execute the DatabaseGateway application provided
                            ::execl("./DatabaseGateway", "./DatabaseGateway", nullptr);
                            ::exit(0);
                        }
                        else
                        {
                            // Run the rest API portal
                            ::execl("./RestApiPortal", "./RestApiPortal", nullptr);
                            ::exit(0);
                        }
                    }
                    else if ("Computation" == strVmType)
                    {
                        // Execute the Entrypoint application provided
                        ::execl(oVmInitializationInstructions.GetString("Entrypoint").c_str(), oVmInitializationInstructions.GetString("Entrypoint").c_str(), nullptr);
                        ::exit(0);
                    }
                    else
                    {
                        _ThrowBaseException("Invalid VM type", nullptr);
                    }
                }

                oResponseStructuredBuffer.PutString("Status", "Success");

                // We just want to establish one connection with the Initializer client and then shut down
                // Either the Virtual Machine receives all the files and relevant startup data on the first
                // transaction or it just fails to initialize and Error is sent to the Initialization Tool.
                fSuccess = true;
            }
        }
        catch(const BaseException & oBaseException)
        {
            oResponseStructuredBuffer.PutString("Status", "Fail");
            oResponseStructuredBuffer.PutString("Error", oBaseException.GetExceptionMessage());
        }
        catch(std::exception & oException)
        {
            oResponseStructuredBuffer.PutString("Status", "Fail");
            oResponseStructuredBuffer.PutString("Error", oException.what());
        }

        // We again try to send the Status response to the initializer tool so that it could know if the
        // package was installed correctly without any error. But we don't want to risk a failure of this process
        // while that happens and some exception occurs.
        try
        {
            // Send the resposnse to the Remote Initializer Tool
            // There is a chance that this transaction may fail but in that case, we will continue to the run the
            // virtual machine and exit the init process and leave it on the discretion of the initialization tool
            if (nullptr != poTlsNode)
            {
                JsonValue * poResponseJson = JsonValue::ParseStructuredBufferToJson(oResponseStructuredBuffer);
                bool fResponseStatus = ::PutHttpResponse(poTlsNode, poResponseJson->ToString());
                poTlsNode->Release();
                poResponseJson->Release();
            }
        }
        
        catch(...)
        {
            std::cout << "Unexpected Error while sending init response.";
        }
    }
}

/********************************************************************************************/

int main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    try
    {
        ::InitVirtualMachine();
    }

    catch (BaseException oException)
    {
        std::cout << "BaseVmImageInit" << std::endl
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
        std::cout << "BaseVmImageInit" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}