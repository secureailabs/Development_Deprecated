/*********************************************************************************************
 *
 * @file RegressionTest.cpp
 * @author Shabana Akhtar Baig
 * @date 17 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "InteractiveClient.h"

/********************************************************************************************/

void RegressionTest(void)
{
    std::string strEosb;
    std::string strEmail = "lbart@igr.com";
    std::string strPassword = "sailpassword";
    std::string strRootEventGuid = "{00000000-0000-0000-0000-000000000000}";

    while (true)
    {
        try
        {
            strEosb = ::Login(strEmail, strPassword);
            _ThrowBaseExceptionIf((0 == strEosb.size()), "Login failed.", nullptr);
            StructuredBuffer oUserInformation(::GetBasicUserInformation(strEosb));
            std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
            ::ListOrganizations(strEosb);
            ::ListDigitalContracts(strEosb);
            ::GetListOfEvents(strEosb, strRootEventGuid, strOrganizationGuid, 0);
        }
        catch(BaseException oBaseException)
        {
            std::cout << oBaseException.GetExceptionMessage() << std::endl;
        }
        catch(...)
        {
            std::cout << "Unknown exception caught!" << std::endl;
        }
    }
}