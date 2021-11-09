/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 07 July 2021
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
#include "CommunicationPortal.h"

#include <iostream>
#include <iterator>

#include <sys/stat.h>
#include <unistd.h>

/********************************************************************************************/

void __stdcall RunRootOfTrust(void)
{
    __DebugFunction();

    pid_t nProcessIdentifier = ::fork();
    _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "Fork has failed with errno = %d", errno);
    // Are we the child process or the parent process?
    if (0 == nProcessIdentifier)
    {
        // We are the child process. Let's just call execl and run the RootOfTrust
        ::execl("RootOfTrustProcess", "RootOfTrustProcess", nullptr);
        ::exit(0);
    }
}

int main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    try
    {
        // The RootofTrustProcess must be run before Communication Portal and must register wit it
        ::RunRootOfTrust();

        // This is a blocking call and won't proceed until all required processes have registered
        CommunicationPortal oCommunicationPortal;
        oCommunicationPortal.WaitForProcessToRegister();

        // Once all processes have registerd we can create a TLS server and start listening to
        // requests and forward them accordingly
        oCommunicationPortal.StartServer(3500);
    }

    catch (BaseException oException)
    {
        std::cout << "CommunicationPortal" << std::endl
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
        std::cout << "CommunicationPortal" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}