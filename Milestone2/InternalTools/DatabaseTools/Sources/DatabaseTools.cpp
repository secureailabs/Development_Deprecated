/*********************************************************************************************
 *
 * @file DatabaseTools.cpp
 * @author Shabana Akhtar Baig
 * @date 24 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DatabaseTools.h"

/********************************************************************************************/

void RunThreadedInserts(void)
{
    __DebugFunction();

    // Testing mongocxx::pool
    mongocxx::instance oInstance{}; // Create only one instance
    mongocxx::pool oPool{mongocxx::uri{"mongodb://localhost:27017"}};
    std::vector<std::thread> threads{};

    for (auto i : {0, 1, 2, 3, 4, 5}) {
        auto run = [&](std::int64_t j) {
            // Each client and collection can only be used in a single thread.
            auto client = oPool.acquire();
            auto coll = (*client)["TestThreading"]["TestCase"];

            bsoncxx::types::b_int64 index = {j};
            coll.insert_one(bsoncxx::builder::basic::make_document(kvp("x", index)));
        };

        std::thread runner{run, i};

        threads.push_back(std::move(runner));
    }

    for (auto&& runner : threads) {
        runner.join();
    }
}

/********************************************************************************************/

DatabaseTools::DatabaseTools(
    _in const char * c_szIpAddress, 
    _in unsigned int unPortNumber
    )
{
    __DebugFunction();

    // initialize web portal ip address and port number
    ::AddWebPortalConfigurations(c_szIpAddress, unPortNumber);
    // initialize accounts
    this->InitializeMembers();
}

/********************************************************************************************/

DatabaseTools::~DatabaseTools(void)
{
    __DebugFunction();
}

/********************************************************************************************/

void __thiscall DatabaseTools::InitializeMembers(void)
{
    __DebugFunction();

    // Add Organizations informations
    m_stlOrganizations.push_back({"Zerg", "Char", "Naruto", "Supply Generator", "overlord@zerg.com", "000-000-0000", "Saitama", "Network Engineer", "nydus@zerg.com", "000-000-0000"});
    m_stlOrganizations.push_back({"Terran", "Earth", "Eren", "Security Expert", "marine@terran.com", "000-000-0000", "Gon", "Hardware Engineer", "scv@t1erran.com", "000-000-0000"});
    // Add super admins information
    m_stlAdmins.push_back({"queen@zerg.com", "Queen of Blades", "Hardware Engineer", "000-000-0000", eAdmin});
    m_stlAdmins.push_back({"marine@terran.com", "Eren", "Security Expert", "000-000-0000", eAdmin});
    // Add other users information
    m_stlUsers.push_back({"larva@zerg.com", "Larva", "Security Expert", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back({"overlord@zerg.com", "Lord", "Supply Generator", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back({"zergling@zerg.com", "Zergling", "Network Engineer", "000-000-0000", eAuditor});
    m_stlUsers.push_back({"roach@zerg.com", "Roach", "Field Engineer", "000-000-0000", eDigitalContractAdmin});
    m_stlUsers.push_back({"lurker@zerg.com", "Lurker", "Market Discovery Expert", "000-000-0000", eDatasetAdmin});
    m_stlUsers.push_back({"scv@terran.com", "Gon", "Hardware Engineer", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back({"marauder@terran.com", "Naruto", "Supply Generator", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back({"reaper@terran.com", "Saitama", "Network Engineer", "000-000-0000", eAuditor});
    m_stlUsers.push_back({"ghost@terran.com", "Alex", "Field Engineer", "000-000-0000", eDigitalContractAdmin});
    m_stlUsers.push_back({"banshee@terran.com", "Yagami", "Market Discovery Expert", "000-000-0000", eDatasetAdmin});
    // Initialize number of other users to m_stlUsers size
    m_unNumberOfOtherUsers = 5;
}

/********************************************************************************************/

void __thiscall DatabaseTools::AddOrganizationsAndSuperAdmins(void)
{
    __DebugFunction();

    // Register organizations and super admins 
    for (unsigned int unIndex = 0; unIndex < m_stlOrganizations.size(); ++unIndex)
    {
        StructuredBuffer oOrganizationInformation;
        oOrganizationInformation.PutString("Email", m_stlAdmins.at(unIndex).m_strEmail);
        oOrganizationInformation.PutString("Password", m_strPassword);
        oOrganizationInformation.PutString("Name", m_stlAdmins.at(unIndex).m_strName);
        oOrganizationInformation.PutString("PhoneNumber", m_stlAdmins.at(unIndex).m_strPhoneNumber);
        oOrganizationInformation.PutString("Title", m_stlAdmins.at(unIndex).m_strTitle);
        oOrganizationInformation.PutString("OrganizationName", m_stlOrganizations.at(unIndex).m_strOrganizationName);
        oOrganizationInformation.PutString("OrganizationAddress", m_stlOrganizations.at(unIndex).m_strOrganizationAddress);
        oOrganizationInformation.PutString("PrimaryContactName", m_stlOrganizations.at(unIndex).m_strPrimaryContactName);
        oOrganizationInformation.PutString("PrimaryContactTitle", m_stlOrganizations.at(unIndex).m_strPrimaryContactTitle);
        oOrganizationInformation.PutString("PrimaryContactEmail", m_stlOrganizations.at(unIndex).m_strPrimaryContactEmail);
        oOrganizationInformation.PutString("PrimaryContactPhoneNumber", m_stlOrganizations.at(unIndex).m_strPrimaryContactPhoneNumber);
        oOrganizationInformation.PutString("SecondaryContactName", m_stlOrganizations.at(unIndex).m_strSecondaryContactName);
        oOrganizationInformation.PutString("SecondaryContactTitle", m_stlOrganizations.at(unIndex).m_strSecondaryContactTitle);
        oOrganizationInformation.PutString("SecondaryContactEmail", m_stlOrganizations.at(unIndex).m_strSecondaryContactEmail);
        oOrganizationInformation.PutString("SecondaryContactPhoneNumber", m_stlOrganizations.at(unIndex).m_strSecondaryContactPhoneNumber);
        // Add organizations and their super admins to the database
        bool fSuccess = ::RegisterOrganizationAndSuperUser(oOrganizationInformation);
        _ThrowBaseExceptionIf((false == fSuccess), "Error registering organization and its super admin.", nullptr);
    }

    std::cout << "Organizations and their admins added successfully." << std::endl;
}

/********************************************************************************************/

void __thiscall DatabaseTools::AddOtherUsers(void)
{
    __DebugFunction();

    // Register users for organizations
    for (unsigned int unIndex = 0; unIndex < m_stlOrganizations.size(); ++unIndex)
    {
        // Login to the web services
        std::string strEncodedEosb = Login(m_stlAdmins.at(unIndex).m_strEmail, m_strPassword);
        _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
        // Get organization guid from eosb
        StructuredBuffer oUserInformation(::GetBasicUserInformation(strEncodedEosb));
        std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
        unsigned int unUserIndex = unIndex * m_unNumberOfOtherUsers;     // calculate index based number of other users per organization
        for (; unUserIndex < (unIndex * m_unNumberOfOtherUsers + m_unNumberOfOtherUsers); ++unUserIndex)
        {
            StructuredBuffer oUserInformation;
            oUserInformation.PutString("Email", m_stlUsers.at(unUserIndex).m_strEmail);
            oUserInformation.PutString("Password", m_strPassword);
            oUserInformation.PutString("Name", m_stlUsers.at(unUserIndex).m_strName);
            oUserInformation.PutString("PhoneNumber", m_stlUsers.at(unUserIndex).m_strPhoneNumber);
            oUserInformation.PutString("Title", m_stlUsers.at(unUserIndex).m_strTitle);
            oUserInformation.PutQword("AccessRights", m_stlUsers.at(unUserIndex).m_qwAccessRights);
            ::RegisterUser(strEncodedEosb, strOrganizationGuid, oUserInformation);
        }
    }

    std::cout << "Users added successfully." << std::endl;;
}



