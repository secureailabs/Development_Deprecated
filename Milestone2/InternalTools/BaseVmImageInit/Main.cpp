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

#include <iostream>
#include <iterator>
#include <fstream>

#include <sys/stat.h>
#include <unistd.h>

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Write the Byte array to a new Binary file
 * @param[in] c_strFileName Name of the file to write data
 * @param[in] c_stlFileData The byte vector to be written to the file
 *
 ********************************************************************************************/

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

    TlsNode * poTlsNode;

    StructuredBuffer oResponseStructuredBuffer;
    oResponseStructuredBuffer.PutString("Status", "Success");

    unsigned int unSpinner = 0;
    try
    {
        while(0 == unSpinner)
        {
            if (true == oTlsServer.WaitForConnection(1000))
            {
                std::cout << "New Connection: " << std::endl;
                poTlsNode = oTlsServer.Accept();
                _ThrowIfNull(poTlsNode, "Cannot establish connection.", nullptr);
                // Fetch the serialized Structure Buffer from the remote Initializer Tool
                std::vector<Byte> stlPayload = ::GetTlsTransaction(poTlsNode, 60*1000);
                _ThrowBaseExceptionIf((0 == stlPayload.size()), "Bad Initialization data", nullptr);
                // deserialize the buffer
                StructuredBuffer oVmInitializationInstructions(stlPayload);

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
                    ::BytesToFile(stlFilesList.at(i), oFilesStructuredBuffer.GetBuffer(stlFilesList.at(i).c_str()));
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
                // We just want to establish one connection with the Initializer client and then shut down
                // Either the Virtual Machine receives all the files and relevant startup data on the first
                // transaction or it just fails to initialize and Error is sent to the Initialization Tool.
                unSpinner = 1;
            }
        }
    }
    catch(const BaseException & oBaseException)
    {
        oResponseStructuredBuffer.PutString("Status", "Fail");
        oResponseStructuredBuffer.PutString("Error", oBaseException.GetExceptionMessage());
    }

    // Send the resposnse to the Remote Initializer Tool
    bool fResponseStatus = ::PutTlsTransaction(poTlsNode, oResponseStructuredBuffer.GetSerializedBuffer());

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
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