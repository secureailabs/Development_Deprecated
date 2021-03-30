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

#include "getopt.h"

// struct containing valid command line options
struct option Options[] = {
    {"vm", no_argument, NULL, 'z'},
    {NULL, 0, NULL, 0}
};

/********************************************************************************************/

int main(int argc, char * argv[])
{
    __DebugFunction();

    // Check if any options supplied
    int nCharacter, nStatus, nSuccess = -1;
    bool fRegisterVm = false;
    while(-1 == nSuccess)
    {
        nCharacter = getopt_long(argc, argv, "z", Options, &nStatus);
        // No more short swicthes
        if (-1 == nCharacter)
        {
            nSuccess = 1;
        }
        else
        {
            if ('z' == nCharacter)
            {
                fRegisterVm = true;
                nSuccess = 1;
            }
            else 
            {
                // Invalid switch
                std::cout << "Usage: options [-z] [--vm]" << std::endl;
                nSuccess = -2; 
            }
        }
    }

    if (1 == nSuccess)
    {
        const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

        try
        {
            ::ClearScreen();

            std::cout << "************************\n  SAIL DATABASE TOOL\n************************\n" << std::endl;
            std::string strIpAddress = ::GetStringInput("IP address: ", 50, false, c_szValidInputCharacters);
            unsigned int unPortNumber = std::stoul(::GetStringInput("Port number: ", 50, false, c_szValidInputCharacters));

            DatabaseTools oDatabaseTools(strIpAddress.c_str(), unPortNumber);
            if (false == fRegisterVm)
            {
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
            else 
            {
                // Add a Virtual Machine, add VM branch and leaf events for DOO and RO
                oDatabaseTools.AddVirtualMachine();
            }
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
    }

    return 0;
}