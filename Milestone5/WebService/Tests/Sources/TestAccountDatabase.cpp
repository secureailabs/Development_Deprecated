/*********************************************************************************************
 *
 * @file TestAccountDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 11 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"

 #include <assert.h>

/********************************************************************************************/

void TestRegisterOrganizationAndSuperUser(void)
{
    __DebugFunction();

    bool fSuccess = false;

    StructuredBuffer oOrganizationInformation;
    oOrganizationInformation.PutString("Email", "user@test.com");
    oOrganizationInformation.PutString("Password", "sailpassword");
    oOrganizationInformation.PutString("Name", "Test Super User");
    oOrganizationInformation.PutString("PhoneNumber", "000-000-0000");
    oOrganizationInformation.PutString("Title", "Test user");
    oOrganizationInformation.PutString("OrganizationName", "Test organization");
    oOrganizationInformation.PutString("OrganizationAddress", "N/A");
    oOrganizationInformation.PutString("PrimaryContactName", "Test user 2");
    oOrganizationInformation.PutString("PrimaryContactTitle", "Test user");
    oOrganizationInformation.PutString("PrimaryContactEmail", "user2@test.com");
    oOrganizationInformation.PutString("PrimaryContactPhoneNumber", "000-000-0000");
    oOrganizationInformation.PutString("SecondaryContactName", "Test user 3");
    oOrganizationInformation.PutString("SecondaryContactTitle", "Test user");
    oOrganizationInformation.PutString("SecondaryContactEmail", "user3@test.com");
    oOrganizationInformation.PutString("SecondaryContactPhoneNumber", "000-000-0000");

    fSuccess = ::RegisterOrganizationAndSuperUser(oOrganizationInformation);

    // Check fSuccess
    assert(true == fSuccess);
    // Print success message
    std::cout << "Test register organization and super user passed." << std::endl;
}

/********************************************************************************************/

void TestRegisterUser(void)
{
    __DebugFunction();

    bool fSuccess = false;

    // Login with correct credentials
    std::string strEosb = ::Login("user@test.com", "sailpassword");
    // Check if the Eosb is not null
    if (strEosb.size() > 0)
    {
        // Get organization guid
        std::string strOrganizationGuid = StructuredBuffer(::GetBasicUserInformation(strEosb)).GetString("OrganizationGuid");
        StructuredBuffer oDcAdminInformation;
        oDcAdminInformation.PutString("Email", "user4@test.com");
        oDcAdminInformation.PutString("Password", "sailpassword");
        oDcAdminInformation.PutString("Name", "Test user 4");
        oDcAdminInformation.PutString("PhoneNumber", "000-000-0000");
        oDcAdminInformation.PutString("Title", "Test user");
        oDcAdminInformation.PutQword("AccessRights", 0x4);
        StructuredBuffer oDatasetAdminInformation;
        oDatasetAdminInformation.PutString("Email", "user5@test.com");
        oDatasetAdminInformation.PutString("Password", "sailpassword");
        oDatasetAdminInformation.PutString("Name", "Test user 5");
        oDatasetAdminInformation.PutString("PhoneNumber", "000-000-0000");
        oDatasetAdminInformation.PutString("Title", "Test user");
        oDatasetAdminInformation.PutQword("AccessRights", 0x5);

        // Register Digital Contract admin
        fSuccess = ::RegisterUser(strEosb, strOrganizationGuid, oDcAdminInformation);
        assert (true == fSuccess);
        // Register Dataset admin
        fSuccess = ::RegisterUser(strEosb, strOrganizationGuid, oDatasetAdminInformation);
        assert (true == fSuccess);
    }

    // Check fSuccess
    assert (true == fSuccess);
    
    std::cout << "Test register user passed." << std::endl;
}

/********************************************************************************************
* TODO: Add test cases for other account database functions once they are fully implemented
********************************************************************************************/

void RunAccountDatabaseTests(void)
{
    // Call unit tests
    ::TestRegisterOrganizationAndSuperUser();
    ::TestRegisterUser();
}