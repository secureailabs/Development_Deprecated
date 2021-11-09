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
#include "CommandLine.h"

#include "getopt.h"

static const char * gsc_szPrintableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szIpAddressCharacters = "0123456789.";

// struct containing valid command line options
struct option Options[] = {
    {"create-vm", no_argument, NULL, 'a'},
    {"delete-db", no_argument, NULL, 'd'},
    {NULL, 0, NULL, 0}
};

/********************************************************************************************/

int main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    // Parse the command line
    StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);
    std::cout << oCommandLineArguments.ToString() << std::endl;

    // Get the IP Address of the SAIL WebPortal. It could either be present
    // as a commnad line argument or be provided separately
    std::string strIpAddress = "";
    unsigned int unPortNumber = 0;
    if (true == oCommandLineArguments.IsElementPresent("PortalIp", ANSI_CHARACTER_STRING_VALUE_TYPE))
    {
        strIpAddress = oCommandLineArguments.GetString("PortalIp");
        if (true == oCommandLineArguments.IsElementPresent("Port", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            unPortNumber = std::stoi(oCommandLineArguments.GetString("Port"));
        }
        else
        {
            std::cout << "Port option missing!! --PortalIp=<Ip address> --Port=<port number> [--a create-vm] [--d delete-db]\n";
        }
    }
    else
    {
        strIpAddress = ::GetStringInput("PortalIpAddress: ", 64, false, gsc_szIpAddressCharacters);
        unPortNumber = std::stoi(::GetStringInput("Port: ", 64, false, gsc_szIpAddressCharacters));
    }

    bool fRegisterVm = false, fDeleteDb = false;
    if (true == oCommandLineArguments.IsElementPresent("a", BOOLEAN_VALUE_TYPE))
    {
        fRegisterVm = true;
    }

    if (true == oCommandLineArguments.IsElementPresent("d", BOOLEAN_VALUE_TYPE))
    {
        fDeleteDb = true;
    }

    if (true == oCommandLineArguments.IsElementPresent("help", BOOLEAN_VALUE_TYPE))
    {
        std::cout << "DatabaseTools --PortalIp=<Ip address> --Port=<port number> [--a create-vm] [--d delete-db]\n";
    }

    try
    {
        ::ClearScreen();

        DatabaseTools oDatabaseTools(strIpAddress.c_str(), unPortNumber);
        if (true == fRegisterVm)
        {
            // Add a Virtual Machine, add VM branch and leaf events for DOO and RO
            oDatabaseTools.AddVirtualMachine();
        }
        else if (true == fDeleteDb)
        {
            // Delete database
            oDatabaseTools.DeleteDatabase();
            std::cout << "Database deleted!" << std::endl;
        }
        else
        {
            // Add organizations and their super admins
            oDatabaseTools.AddOrganizationsAndSuperAdmins();
            // Add other users for the organizations
            oDatabaseTools.AddOtherUsers();
            // Register dataset metadata
            oDatabaseTools.AddDatasets();
            // Register digital contracts for the organizations
            oDatabaseTools.AddDigitalContracts();
            // Accept digital contracts
            oDatabaseTools.AcceptDigitalContracts();
            // Activate digital contracts
            oDatabaseTools.ActivateDigitalContracts();

            oDatabaseTools.AddDatasetFamilies();
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

    return 0;
}