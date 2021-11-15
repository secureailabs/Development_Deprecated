
/*********************************************************************************************
 *
 * @file TestRestFramework.cpp
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "CommandLine.h"
#include "ConsoleInputHelperFunctions.h"
#include "RestFramework.h"
#include "ThreadManager.h"

/********************************************************************************************/

static const char * gsc_szIpAddressCharacters = "0123456789.";

/********************************************************************************************/

int main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    std::cout << "----------REST API PORTAL----------" << std::endl;
    Word wPort = 6200;
    std::cout << "Initializing server at port " << wPort << std::endl;
    // Get the IP Address and port of our remote database service
    std::string strDatabaseServiceIp = "127.0.0.1";
    unsigned int unDatabaseServerPort = 6500;
    // Parse the command line
    StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);
    std::cout << oCommandLineArguments.ToString() << std::endl;

    if (true == oCommandLineArguments.IsElementPresent("DatabaseServerIp", ANSI_CHARACTER_STRING_VALUE_TYPE))
    {
        strDatabaseServiceIp = oCommandLineArguments.GetString("DatabaseServerIp");
        if (true == oCommandLineArguments.IsElementPresent("DatabaseServerPort", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            unDatabaseServerPort = std::stoi(oCommandLineArguments.GetString("DatabaseServerPort"));
        }
    }

    std::cout << "Registering plug-ins with DB service " << strDatabaseServiceIp << ":" << unDatabaseServerPort << std::endl;

    // Create singleton instance of the ThreadManager
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    StructuredBuffer oPluginInitializationVectors;
    oPluginInitializationVectors.PutString("DatabaseServerIp", strDatabaseServiceIp);
    oPluginInitializationVectors.PutUnsignedInt32("DatabaseServerPort", unDatabaseServerPort);

    // RestFramework will load plugins from "SharedLibraries/RestApiPortal" directory
    RestFramework oRestFramework(wPort, "./SharedLibraries/RestApiPortal", oPluginInitializationVectors);
    // Start the server
    oRestFramework.RunServer();
    // Join all threads created by the ThreadManager
    poThreadManager->JoinAllThreads();

    return 0;
}
