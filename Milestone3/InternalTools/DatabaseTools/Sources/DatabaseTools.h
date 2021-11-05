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

typedef struct UserInformation
{
    std::string m_strUserGuid;
    std::string m_strEmail;
    std::string m_strName;
    std::string m_strTitle;
    std::string m_strPhoneNumber;
    Qword m_qwAccessRights;

    UserInformation(
        _in const std::string & c_strEmail, 
        _in const std::string & c_strName, 
        _in const std::string c_strTitle, 
        _in const std::string c_strPhoneNumber,
        _in Qword qwAccessRights
        )
    {
        m_strEmail = c_strEmail;
        m_strName = c_strName;
        m_strTitle = c_strTitle;
        m_strPhoneNumber = c_strPhoneNumber;
        m_qwAccessRights = qwAccessRights;
    }
} 
UserInformation;

typedef struct OrganizationInformation
{
    std::string m_strOrganizationGuid;
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

    OrganizationInformation(
        _in const std::string & c_strName, 
        _in const std::string & c_strAddress, 
        _in const std::string c_strPrimaryContactName, 
        _in const std::string c_strPrimaryContactTitle,
        _in const std::string c_strPrimaryContactEmail, 
        _in const std::string c_strPrimaryContactPhoneNumber,
        _in const std::string c_strSecondaryContactName, 
        _in const std::string c_strSecondaryContactTitle,
        _in const std::string c_strSecondaryContactEmail, 
        _in const std::string c_strSecondaryContactPhoneNumber
        )
    {
        m_strOrganizationName = c_strName;
        m_strOrganizationAddress = c_strAddress;
        m_strPrimaryContactName = c_strPrimaryContactName;
        m_strPrimaryContactTitle = c_strPrimaryContactTitle;
        m_strPrimaryContactEmail = c_strPrimaryContactEmail;
        m_strPrimaryContactPhoneNumber = c_strPrimaryContactPhoneNumber;
        m_strSecondaryContactName = c_strSecondaryContactName;
        m_strSecondaryContactTitle = c_strSecondaryContactTitle;
        m_strSecondaryContactEmail = c_strSecondaryContactEmail;
        m_strSecondaryContactPhoneNumber = c_strSecondaryContactPhoneNumber;
    }
} 
OrganizationInformation;

typedef struct DigitalContractInformation
{
    std::string m_strTitle;
    uint64_t m_unSubscriptionDays;
    std::string m_strLegalAgreement;
    uint64_t m_unRetentionTime;
    std::string m_strDescription;
    std::string m_strHostForVM;
    uint64_t m_un64NoOfVM;
    uint64_t m_un64NoOfVCPU;
    std::string m_strHostRegion;

    DigitalContractInformation(
        _in const std::string & c_strTitle,
        _in uint64_t unSubscriptionDays, 
        _in const std::string & c_strLegalAgreement,
        _in uint64_t unRetentionTime,
        _in const std::string & c_strDescription,
        _in const std::string c_strHostForVM,
        _in uint64_t un64NoOfVM,
        _in uint64_t un64NoOfVCPU,
        _in const std::string c_strHostRegion
        )
    {
        m_strTitle = c_strTitle;
        m_unSubscriptionDays = unSubscriptionDays;
        m_strLegalAgreement = c_strLegalAgreement;
        m_unRetentionTime = unRetentionTime;
        m_strDescription = c_strDescription;
        m_strHostForVM = c_strHostForVM;
        m_un64NoOfVM = un64NoOfVM;
        m_un64NoOfVCPU = un64NoOfVCPU;
        m_strHostRegion = c_strHostRegion;
    }
} 
DigitalContractInformation;

typedef struct DatasetInformation
{
    std::string m_strDatasetGuid;
    std::string m_strVersionNumber;
    std::string m_strName;
    std::string m_strDescription;
    std::string m_strKeywords;
    uint64_t m_un64PublishTime;
    Byte m_bPrivacyLevel;
    std::string m_strLimitations;

    DatasetInformation(
        _in const std::string & c_strDatasetGuid,
        _in const std::string & c_strVersionNumber, 
        _in const std::string & c_strName,
        _in const std::string & c_strDescription,
        _in const std::string & c_strKeywords,
        _in uint64_t un64PublishTime,
        _in Byte bPrivacyLevel,
        _in const std::string & c_strLimitations
        )
    {
        m_strDatasetGuid = c_strDatasetGuid;
        m_strVersionNumber = c_strVersionNumber;
        m_strName = c_strName;
        m_strDescription = c_strDescription;
        m_strKeywords = c_strKeywords;
        m_un64PublishTime = un64PublishTime;
        m_bPrivacyLevel = bPrivacyLevel;
        m_strLimitations = c_strLimitations;
    }
} 
DatasetInformation;

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
        // Register datasets
        void __thiscall AddDatasets(void);
        // Register digital contracts
        void __thiscall AddDigitalContracts(void);

        // Accept all digital contracts
        void __thiscall AcceptDigitalContracts(void);

        // Activate all digital contracts
        void __thiscall ActivateDigitalContracts(void);

        // Add virtual machine
        void __thiscall AddVirtualMachine(void);

        // Add VM branch event node and leaf events for DOO
        void __thiscall RegisterVmAfterDataUpload(
            _in const std::string & c_strVmGuid
            );

        // Add VM branch event node and leaf events for RO
        void __thiscall RegisterVmForComputation(
            _in const std::string & c_strVmGuid
            );

        void __thiscall AddDatasetFamilies(void);

        // Delete database
        void __thiscall DeleteDatabase();

    private:

        // Data members
        std::string m_strPassword = "SailPassword@123";
        unsigned int m_unNumberOfOtherUsers;
        std::vector<OrganizationInformation> m_stlOrganizations;
        std::vector<UserInformation> m_stlAdmins;
        std::vector<UserInformation> m_stlUsers;
        std::vector<DatasetInformation> m_stlDatasets;
        std::vector<DigitalContractInformation> m_stlDigitalContracts;
        std::vector<std::string> m_stlDigitalContractGuids;
};

// Testing mongocxx::pool
extern void RunThreadedInserts(void);