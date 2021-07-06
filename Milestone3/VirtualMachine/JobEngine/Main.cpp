/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 27 Sep 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CommandLine.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "StructuredBuffer.h"
#include "JobEngine.h"
#include "SocketClient.h"

#include <iostream>

JobEngine JobEngine::m_oJobEngine;

#define cout cout << std::this_thread::get_id() << " "

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
        // std::string strUnixAddressOfCommunicationModule = oCommandLineArguments.GetString("ipc");
        std::string strUnixAddressOfCommunicationModule = "{164085b7-ef20-4257-bc14-e1e08c908aaa}";

        // Connect to the Communication module for further communication
        std::cout << "Connecting to socket server on..." << std::endl;
        Socket * poSocket = ::ConnectToUnixDomainSocket(strUnixAddressOfCommunicationModule.c_str());

        // Get the singleton object of the Job Engine
        JobEngine & oJobEngine = JobEngine::Get();
        oJobEngine.StartServer(poSocket);
    }
    catch (BaseException oException)
    {
        std::cout << "JobEngine" << std::endl
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
        std::cout << "JobEngine" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}