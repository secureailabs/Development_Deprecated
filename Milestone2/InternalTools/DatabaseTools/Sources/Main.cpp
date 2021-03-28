/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Shabana Akhtar Baig
 * @date 24 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DatabaseTools.h"
#include "ConsoleInputHelperFunctions.h"

/********************************************************************************************/

int main()
{
    __DebugFunction();

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    try
    {
        ::ClearScreen();

        std::cout << "************************\n  SAIL DATABASE TOOL\n************************\n" << std::endl;
        std::string strIpAddress = ::GetStringInput("IP address: ", 50, false, c_szValidInputCharacters);
        unsigned int unPortNumber = std::stoul(::GetStringInput("Port number: ", 50, false, c_szValidInputCharacters));

        DatabaseTools oDatabaseTools(strIpAddress.c_str(), unPortNumber);
        // Add organizations and their super admins
        oDatabaseTools.AddOrganizationsAndSuperAdmins();
        // Add other users for the organizations
        oDatabaseTools.AddOtherUsers();
        // Register digital contracts for the organizations
        oDatabaseTools.AddDigitalContracts();
        // Accept digital contracts
        oDatabaseTools.AcceptDigitalContracts();
        // Activate digital contracts
        oDatabaseTools.ActivateDigitalContracts();
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << std::endl;
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Error: Unknown exception caught." << std::endl;
    }

    return 0;
}
