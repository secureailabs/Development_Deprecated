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

/********************************************************************************************/

int main()
{
    __DebugFunction();

    try
    {
        const char * c_szIpAddress = "127.0.0.1";
        unsigned int unPort = 6200;
        DatabaseTools oDatabaseTools(c_szIpAddress, unPort);
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
