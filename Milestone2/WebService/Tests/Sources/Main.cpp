/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Shabana Akhtar Baig
 * @date 07 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "InteractiveClient.h"

/********************************************************************************************/

int main()
{
    std::string strEosb;
    std::string strEmail = "lbart@igr.com";
    std::string strPassword = "sailpassword";
    std::string strRootEventGuid = "{00000000-0000-0000-0000-000000000000}";

    // Add webservices configuration
    ::AddWebPortalConfiguration("127.0.0.1", 6200);
    while (true)
    {
        try
        {
            strEosb = ::Login(strEmail, strPassword);
            StructuredBuffer oUserInformation(::GetBasicUserInformation(strEosb));
            std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
            _ThrowBaseExceptionIf((0 == strEosb.size()), "Login failed.", nullptr);
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