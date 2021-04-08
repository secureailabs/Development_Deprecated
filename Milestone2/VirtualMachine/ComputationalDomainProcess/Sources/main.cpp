/*********************************************************************************************
 *
 * @file main.cpp
 * @author Jingwei Zhang
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used for single job related issues
 *
 ********************************************************************************************/

/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CommandLine.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "IpcTransactionHelperFunctions.h"
#include "researchVM.h"
#include "RootOfTrustNode.h"
#include "StructuredBuffer.h"

#include <iostream>

/********************************************************************************************/

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
        // First we initialize the RootOfTrustNode. Without that, nothing else matters
        RootOfTrustNode oRootOfTrustNode(oCommandLineArguments.GetString("ipc").c_str());
        // Run the computation engine
        ComputationVM oTest(7000, 5, oRootOfTrustNode);
        oTest.Initialize();
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
