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
#include "Base64Encoder.h"

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
    ::AddWebPortalConfiguration(c_szIpAddress, unPortNumber);
    // initialize accounts
    this->InitializeMembers();
}

/********************************************************************************************/

DatabaseTools::~DatabaseTools(void)
{
    __DebugFunction();

    m_stlOrganizations.clear();
    m_stlAdmins.clear();
    m_stlUsers.clear();
    m_stlDigitalContractGuids.clear();
    m_stlDigitalContracts.clear();
}

/********************************************************************************************/

void __thiscall DatabaseTools::InitializeMembers(void)
{
    __DebugFunction();

    // Add Organizations information
    m_stlOrganizations.push_back(OrganizationInformation{"International Genetics Research Facility", "New York City", "Lily Bart", "System Administrator", "lbart@igr.com", "000-000-0000", "Philip Carey", "Network Engineer", "pcarey@igr.com", "000-000-0000"});
    m_stlOrganizations.push_back(OrganizationInformation{"Mercy General Hospital", "Los Angeles", "Nick Adams", "System Administrator", "nadams@mghl.com", "000-000-0000", "Nora Helmer", "Network Engineer", "nhelmer@mghl.com", "000-000-0000"});
    // Add super admins information
    m_stlAdmins.push_back(UserInformation{"lbart@igr.com", "Lily Bart", "System Administrator", "000-000-0000", eAdmin});
    m_stlAdmins.push_back(UserInformation{"nadams@mgh.com", "Nick Adams", "System Administrator", "000-000-0000", eAdmin});
    // Add other users information
    m_stlUsers.push_back(UserInformation{"iarcher@igr.com", "Isabel Archer", "Security Expert", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back(UserInformation{"abede@igr.com", "Adam Bede", "Supply Generator", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back(UserInformation{"pcarey@igr.com", "Philip Carey", "Network Engineer", "000-000-0000", eAuditor});
    m_stlUsers.push_back(UserInformation{"achristie@igr.com", "Anna Christie", "Research Scientist", "000-000-0000", eDigitalContractAdmin});
    m_stlUsers.push_back(UserInformation{"rcory@igr.com", "Richard Cory", "Data Scientist", "000-000-0000", eDatasetAdmin});
    m_stlUsers.push_back(UserInformation{"sbowles@mghl.com", "Sally Bowles", "Hardware Engineer", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back(UserInformation{"ncarter@mghl.com", "Nick Carter", "Supply Generator", "000-000-0000", eOrganizationUser});
    m_stlUsers.push_back(UserInformation{"nhelmer@mghl.com", "Nora Helmer", "Network Engineer", "000-000-0000", eAuditor});
    m_stlUsers.push_back(UserInformation{"pmason@mghl.com", "Perry Mason", "Research Scientist", "000-000-0000", eDigitalContractAdmin});
    m_stlUsers.push_back(UserInformation("dquilp@mghl.com", "Daniel Quilp", "Data Scientist", "000-000-0000", eDatasetAdmin));
    // Initialize to the number of other users that will be added for each organization
    m_unNumberOfOtherUsers = 5;
    // Add digital contracts information
    std::string strLegalAgreement = "The Parties acknowledge and agree that this Agreement represents the entire agreement between the Parties. "
    "In the event that the Parties desire to change, add, or otherwise modify any terms, they shall do so in writing to be signed by both parties.";
    std::string strEulaSignedByDoo = "Company grants You a revocable, non-exclusive, non-transferable, limited right to use the dataset on a single machine.";
    std::string strEulaSignedByRo = "Company grants You a revocable, non-exclusive, non-transferable, limited right to use the dataset on a single machine.";
    m_stlDigitalContracts.push_back(DigitalContractInformation{10, strLegalAgreement, 16186603, strEulaSignedByDoo, strEulaSignedByRo});
    m_stlDigitalContracts.push_back(DigitalContractInformation{28, strLegalAgreement, 24117352, strEulaSignedByDoo, strEulaSignedByRo});
    m_stlDigitalContracts.push_back(DigitalContractInformation{35, strLegalAgreement, 60768913, strEulaSignedByDoo, strEulaSignedByRo});
    m_stlDigitalContracts.push_back(DigitalContractInformation{90, strLegalAgreement, 8090084, strEulaSignedByDoo, strEulaSignedByRo});
    m_stlDigitalContracts.push_back(DigitalContractInformation{120, strLegalAgreement, 18605667, strEulaSignedByDoo, strEulaSignedByRo});
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
        // Add organization guid to the organization structure
        m_stlOrganizations.at(unIndex).m_strOrganizationGuid = strOrganizationGuid;
        // Add m_unNumberOfOtherUsers for each organization
        // Calculate index based on the number of other users per organization
        unsigned int unUserIndex = unIndex * m_unNumberOfOtherUsers;  
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

/********************************************************************************************/

void __thiscall DatabaseTools::AddDigitalContracts(void)
{
    __DebugFunction();

    // First organization is the researcher organization and second organization is the data owner organization
    unsigned int unRoIndex = 0, unDooIndex = 1;
    // Login to the web services
    std::string strEncodedEosb = Login(m_stlAdmins.at(unRoIndex).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
    // Get organization guids for the organizations
    std::string strRoGuid = m_stlOrganizations.at(unRoIndex).m_strOrganizationGuid;
    // Add digital contract information
    std::string strDooGuid = m_stlOrganizations.at(unDooIndex).m_strOrganizationGuid;
    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DOOGuid", strDooGuid);
    // Register five digital contracts for the organizations
    for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
    {
        oDcInformation.PutUnsignedInt64("SubscriptionDays", m_stlDigitalContracts.at(unIndex).m_unSubscriptionDays);
        oDcInformation.PutString("LegalAgreement", m_stlDigitalContracts.at(unIndex).m_strLegalAgreement);
        // Register digital contract
        ::RegisterDigitalContract(strEncodedEosb, oDcInformation);
    }

    std::cout << "Digital contracts added successfully." << std::endl;
}

/********************************************************************************************/

void __thiscall DatabaseTools::AcceptDigitalContracts(void)
{
    __DebugFunction();

    // Login to the web services as the data owner's dataset admin
    // As the Rest API requires dataset admin privileges 
    unsigned int unDatasetAdminIndex = 9;
    std::string strEncodedEosb = Login(m_stlUsers.at(unDatasetAdminIndex).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
    // Get list of all digital contracts for the data owner organization
    for (std::string strGuid : StructuredBuffer(::ListDigitalContracts(strEncodedEosb)).GetNamesOfElements())
    {
        m_stlDigitalContractGuids.push_back(strGuid);
    }
    // Accept all five digital contracts
    for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
    {
        StructuredBuffer oDcInformation;
        oDcInformation.PutUnsignedInt64("RetentionTime", m_stlDigitalContracts.at(unIndex).m_unRetentionTime);
        oDcInformation.PutString("EULA", m_stlDigitalContracts.at(unIndex).m_strEulaSignedByDoo);
        oDcInformation.PutString("LegalAgreement", m_stlDigitalContracts.at(unIndex).m_strLegalAgreement);
        oDcInformation.PutString("DigitalContractGuid", m_stlDigitalContractGuids.at(unIndex));
        // Accept digital contract
        ::AcceptDigitalContract(strEncodedEosb, oDcInformation);
    }

    std::cout << "Digital contracts approved." << std::endl;
}

/********************************************************************************************/

void __thiscall DatabaseTools::ActivateDigitalContracts(void)
{
    __DebugFunction();

    // Login to the web services as the researcher's digital contract admin
    // As the Rest API requires digital contract admin privileges 
    unsigned int unDcAdminIndex = 3;
    std::string strEncodedEosb = Login(m_stlUsers.at(unDcAdminIndex).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);

    // Activate all five digital contracts
    for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
    {
        StructuredBuffer oDcInformation;
        oDcInformation.PutString("EULA", m_stlDigitalContracts.at(unIndex).m_strEulaSignedByRo);
        oDcInformation.PutString("LegalAgreement", m_stlDigitalContracts.at(unIndex).m_strLegalAgreement);
        oDcInformation.PutString("DigitalContractGuid", m_stlDigitalContractGuids.at(unIndex));
        // Activate digital contract
        ::ActivateDigitalContract(strEncodedEosb, oDcInformation);
    }

    std::cout << "Digital contracts activated." << std::endl;
}

/********************************************************************************************/

void __thiscall DatabaseTools::AddVirtualMachine(void)
{
    __DebugFunction();

    // Login to the web services with DOO admin credentials
    std::string strEncodedEosb = Login(m_stlAdmins.at(1).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
    // Get list of all digital contracts for the data owner organization
    for (std::string strGuid : StructuredBuffer(::ListDigitalContracts(strEncodedEosb)).GetNamesOfElements())
    {
        m_stlDigitalContractGuids.push_back(strGuid);
    }
    // Get imposter eosb
    std::string strIEosb = ::GetIEosb(strEncodedEosb);
    _ThrowBaseExceptionIf((0 == strIEosb.size()), "Exiting!", nullptr);
    // Add Vm information
    StructuredBuffer oVmInformation;
    oVmInformation.PutString("DigitalContractGuid", m_stlDigitalContractGuids.at(0));
    oVmInformation.PutString("IPAddress", "127.0.0.1");
    // Register Vm
    std::string strVmGuid = Guid(eVirtualMachine).ToString(eHyphensAndCurlyBraces);
    std::string strVmEosb = ::RegisterVirtualMachine(strIEosb, strVmGuid, oVmInformation);
    // Check if the virtual machine was registered successfully
    _ThrowBaseExceptionIf((0 == strVmEosb.size()), "Error occurred when registering a virtual machine.", nullptr);
    std::cout << "Virtual machine registered successfully." << std::endl;
    // Register VM for DOO and add leaf events
    this->RegisterVmAfterDataUpload(strVmGuid);
    // Register VM for RO and add leaf events
    this->RegisterVmForComputation(strVmGuid);
}

/********************************************************************************************/

void __thiscall DatabaseTools::RegisterVmAfterDataUpload(
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    // Login to the web services with DOO admin credentials
    std::string strEncodedEosb = Login(m_stlAdmins.at(1).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
    // Add Vm information
    StructuredBuffer oVmInformation;
    oVmInformation.PutString("DigitalContractGuid", m_stlDigitalContractGuids.at(0));
    // Generate VM branch event for DOO
    std::string strVmEventGuid = ::RegisterVmAfterDataUpload(strEncodedEosb, c_strVmGuid);
    // Check if the virtual machine was registered successfully
    _ThrowBaseExceptionIf((0 == strVmEventGuid.size()), "Error occurred when adding VM branch event for data owner organization", nullptr);
    std::cout << "VM branch event added for DOO." << std::endl;
    // Create vector representing event names
    std::vector<std::string> stlEventNames = {"VM_STARTED", "VM_INITIALIZED", "VM_DATASET_UPLOADED", "VM_READY", "CONNECT_FAILURE", "CONNECT_SUCCESS"};
    for (unsigned int unIndex = 0; unIndex < 4; ++unIndex)
    {
        stlEventNames.push_back("PUSH_DATA");
        stlEventNames.push_back("PUSH_FN");
        stlEventNames.push_back("RUN_FN");
        stlEventNames.push_back("CHECK_JOB");
        stlEventNames.push_back("PULL_DATA");
    }
    stlEventNames.push_back("INSPECT");
    stlEventNames.push_back("GET_TABLE");
    stlEventNames.push_back("DELETE_DATA");
    stlEventNames.push_back("LOGOFF");
    stlEventNames.push_back("VM_SHUTTING_DOWN");
    // Add leaf events information
    uint64_t un64EpochTimeInMilliseconds = ::GetEpochTimeInMilliseconds();
    StructuredBuffer oLeafEvents;
    for (unsigned int unIndex = 0; unIndex < 30; ++unIndex)
    {
        StructuredBuffer oEvent;
        oEvent.PutString("EventGuid", Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces));
        oEvent.PutQword("EventType", unIndex % 16);
        un64EpochTimeInMilliseconds += 1000;
        oEvent.PutUnsignedInt64("Timestamp", un64EpochTimeInMilliseconds);
        StructuredBuffer oEncryptedEventData;
        oEncryptedEventData.PutString("EventName", stlEventNames.at(unIndex));
        oEncryptedEventData.PutByte("EventType", unIndex + 1);
        StructuredBuffer oEventData;
        oEventData.PutUnsignedInt64("VersionNumber", 0x0000000100000001);
        oEventData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
        oEncryptedEventData.PutStructuredBuffer("EventData", oEventData);
        oEvent.PutString("EncryptedEventData", ::Base64Encode(oEncryptedEventData.GetSerializedBufferRawDataPtr(), oEncryptedEventData.GetSerializedBufferRawDataSizeInBytes()));
        oLeafEvents.PutStructuredBuffer(std::to_string(unIndex).c_str(), oEvent);
    }
    // Register leaf events for DOO 
    ::RegisterLeafEvents(strEncodedEosb, strVmEventGuid, oLeafEvents);
}

/********************************************************************************************/

void __thiscall DatabaseTools::RegisterVmForComputation(
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    // Login to the web services with RO admin credentials
    std::string strEncodedEosb = Login(m_stlAdmins.at(0).m_strEmail, m_strPassword);
    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
    // Add Vm information
    StructuredBuffer oVmInformation;
    oVmInformation.PutString("DigitalContractGuid", m_stlDigitalContractGuids.at(0));
    // Generate VM branch event for DOO
    std::string strVmEventGuid = ::RegisterVmForComputation(strEncodedEosb, c_strVmGuid);
    // Check if the virtual machine was registered successfully
    _ThrowBaseExceptionIf((0 == strVmEventGuid.size()), "Error occurred when adding VM branch event for researcher organization", nullptr);
    std::cout << "VM branch event added for RO." << std::endl;
    // Create vector representing event names
    std::vector<std::string> stlEventNames = {"VM_STARTED", "VM_INITIALIZED", "VM_DATASET_UPLOADED", "VM_READY", "CONNECT_FAILURE", "CONNECT_SUCCESS"};
    for (unsigned int unIndex = 0; unIndex < 5; ++unIndex)
    {
        stlEventNames.push_back("PUSH_DATA");
        stlEventNames.push_back("PUSH_FN");
        stlEventNames.push_back("RUN_FN");
        stlEventNames.push_back("CHECK_JOB");
        stlEventNames.push_back("PULL_DATA");
    }
    stlEventNames.push_back("INSPECT");
    stlEventNames.push_back("GET_TABLE");
    stlEventNames.push_back("DELETE_DATA");
    stlEventNames.push_back("LOGOFF");
    stlEventNames.push_back("VM_SHUTTING_DOWN");
    // Add leaf events information
    StructuredBuffer oLeafEvents;
    for (unsigned int unIndex = 0; unIndex < 30; ++unIndex)
    {
        StructuredBuffer oEvent;
        oEvent.PutString("EventGuid", Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces));
        oEvent.PutQword("EventType", unIndex % 16);
        oEvent.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
        StructuredBuffer oEncryptedEventData;
        oEncryptedEventData.PutString("EventName", stlEventNames.at(unIndex));
        oEncryptedEventData.PutByte("EventType", unIndex + 1);
        StructuredBuffer oEventData;
        oEventData.PutUnsignedInt64("VersionNumber", 0x0000000100000001);
        oEventData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
        oEncryptedEventData.PutStructuredBuffer("EventData", oEventData);
        oEvent.PutString("EncryptedEventData", ::Base64Encode(oEncryptedEventData.GetSerializedBufferRawDataPtr(), oEncryptedEventData.GetSerializedBufferRawDataSizeInBytes()));
        oLeafEvents.PutStructuredBuffer(std::to_string(unIndex).c_str(), oEvent);
    }
    // Register leaf events for DOO 
    ::RegisterLeafEvents(strEncodedEosb, strVmEventGuid, oLeafEvents);
}