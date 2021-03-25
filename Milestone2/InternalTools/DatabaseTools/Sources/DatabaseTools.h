/*********************************************************************************************
 *
 * @file DatabaseTools.h
 * @author Shabana Akhtar Baig
 * @date 24 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "64BitHashes.h"
#include "CryptoUtils.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "EntityTypes.h"
#include "Exceptions.h"
#include "InteractiveClient.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <iostream>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

// For testing mongocxx::pool
#include <sstream>
#include <thread>
#include <vector>
#include <mongocxx/pool.hpp>
using bsoncxx::builder::basic::kvp;

/********************************************************************************************/

typedef struct
{
    std::string m_strEmail;
    std::string m_strName;
    std::string m_strTitle;
    std::string m_strPhoneNumber;
    Qword m_qwAccessRights;
} UserInformation;

typedef struct
{
    std::string m_strOrganizationName;
    std::string m_strOrganizationAddress;
    std::string m_strPrimaryContactName;
    std::string m_strPrimaryContactTitle;
    std::string m_strPrimaryContactEmail;
    std::string m_strPrimaryContactPhoneNumber;
    std::string m_strSecondaryContactName;
    std::string m_strSecondaryContactTitle;
    std::string m_strSecondaryContactEmail;
    std::string m_strSecondaryContactPhoneNumber;
} OrganizationInformation;

/********************************************************************************************/

class DatabaseTools : public Object 
{
    public:

        // Constructor and Destructor
        DatabaseTools(
            _in const char * c_szIpAddress, 
            _in unsigned int unPortNumber
            );
        virtual ~DatabaseTools(void);

        // Initialize Organizations and users vectors
        void __thiscall InitializeMembers(void);

        // Register organizations and super admins
        void __thiscall AddOrganizationsAndSuperAdmins(void);

        // Register other users for each organization
        void __thiscall AddOtherUsers(void);

    private:

        // Data members
        std::string m_strPassword = "sailpassword";
        unsigned int m_unNumberOfOtherUsers;
        std::vector<OrganizationInformation> m_stlOrganizations;
        std::vector<UserInformation> m_stlAdmins;
        std::vector<UserInformation> m_stlUsers;
};

// Testing mongocxx::pool
extern void RunThreadedInserts(void);