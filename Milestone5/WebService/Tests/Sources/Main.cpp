/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Shabana Akhtar Baig
 * @date 07 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"

/********************************************************************************************/

int main()
{
    std::string strEosb;
    std::string strEmail = "user@test.com";
    std::string strPassword = "sailpassword";
    std::string strRootEventGuid = "{00000000-0000-0000-0000-000000000000}";

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    try
    {
        ::ClearScreen();

        std::cout << "************************\n  RUNNING TESTS \n************************\n" << std::endl;
        std::string strIpAddress = ::GetStringInput("IP address: ", 50, false, c_szValidInputCharacters);
        unsigned int unPortNumber = std::stoul(::GetStringInput("Port number: ", 50, false, c_szValidInputCharacters));
        // Add webservices configuration
        ::AddWebPortalConfiguration(strIpAddress.c_str(), unPortNumber);

        // Run AccountDatabase tests and create test users
        ::RunAccountDatabaseTests();

        // Run SailAuthentication tests
        ::RunSailAuthenticationTests();

        // Login with the test credentials created in AccountDatabase tests and pass valid Eosb to the other tests
        strEosb = ::Login(strEmail, strPassword);
        _ThrowBaseExceptionIf((0 == strEosb.size()), "Login failed.", nullptr);
        StructuredBuffer oUserInformation(::GetBasicUserInformation(strEosb));
        std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
        // Run the other tests
        std::string strDcGuid = ::RunDigitalContractDatabaseTests(strEosb, strOrganizationGuid);
        ::RunVirtualMachineManagerTests(strEosb, strDcGuid);

        // TODO: Once implemented, add calls to delete the test organization, its users, digital contract, virtual machine, and related audit logs
    }
    catch(BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Unknown exception caught!" << std::endl;
    }

    return 0;
}