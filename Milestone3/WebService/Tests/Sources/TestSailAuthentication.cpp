/*********************************************************************************************
 *
 * @file TestSailAuthentication.cpp
 * @author Shabana Akhtar Baig
 * @date 11 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"

/********************************************************************************************/

void TestLoginWithCorrectCredentials(void)
{
    __DebugFunction();

    // Correct credentials
    std::string strEmail = "user@test.com";
    std::string strPassword = "sailpassword";

    // Call Login with the correct credentials
    std::string strEosb = ::Login(strEmail, strPassword);

    // Eosb should be not null
    if (strEosb.size() > 0)
    {
        std::cout << "Test Logging in with valid credentials passed." << std::endl;
    }
    else
    {
        std::cout << "Test Logging in with valid credentials failed." << std::endl;
    }
}

/********************************************************************************************/

void TestLoginWithIncorrectCredentials(void)
{
    __DebugFunction();

    // Correct credentials
    std::string strEmail = "random@zebra.com";
    std::string strPassword = "1234567";

    // Call Login with the incorrect credentials
    std::string strEosb = ::Login(strEmail, strPassword);

    // Eosb should be null
    if (strEosb.size() == 0)
    {
        std::cout << "Test Logging in with invalid credentials passed." << std::endl;
    }
    else
    {
        std::cout << "Test Logging in with invalid credentials failed." << std::endl;
    }
}

/********************************************************************************************/

void TestGetBasicUserInformation(void)
{
    __DebugFunction();

    bool fSuccess = false;

    // Login with correct credentials
    std::string strEosb = ::Login("user@test.com", "sailpassword");
    // Check if the Eosb is not null
    if (strEosb.size() > 0)
    {
        StructuredBuffer oUserInformation(::GetBasicUserInformation(strEosb));
        // Test the contents of the User Information structured buffer
        if ((true == oUserInformation.IsElementPresent("OrganizationGuid", ANSI_CHARACTER_STRING_VALUE_TYPE))&&(true == oUserInformation.IsElementPresent("UserGuid", ANSI_CHARACTER_STRING_VALUE_TYPE))&&(true == oUserInformation.IsElementPresent("AccessRights", QWORD_VALUE_TYPE)))
        {
            fSuccess = true;
        }
    }

    if (true == fSuccess)
    {
        std::cout << "Test get basic user information passed." << std::endl;
    }
    else
    {
        std::cout << "Test get basic user information failed." << std::endl;
    }
}

/********************************************************************************************/

void RunSailAuthenticationTests(void)
{
    // Call unit tests
    ::TestLoginWithCorrectCredentials();
    ::TestLoginWithIncorrectCredentials();
    ::TestGetBasicUserInformation();
}