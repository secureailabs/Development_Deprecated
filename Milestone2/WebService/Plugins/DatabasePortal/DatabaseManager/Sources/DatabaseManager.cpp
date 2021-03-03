/*********************************************************************************************
 *
 * @file DatabaseManager.cpp
 * @author Shabana Akhtar Baig
 * @date 22 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"
#include "CryptoUtils.h"
#include "DateAndTime.h"
#include "64BitHashes.h"
#include "Utils.h"

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/client_session.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using bsoncxx::type;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

static DatabaseManager * gs_oDatabaseManager = nullptr;

/********************************************************************************************
 *
 * @function GetDatabaseManager
 * @brief Create a singleton object of DatabaseManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of DatabaseManager class
 *
 ********************************************************************************************/

DatabaseManager * __stdcall GetDatabaseManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oDatabaseManager)
    {
        gs_oDatabaseManager = new DatabaseManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oDatabaseManager);
    }

    return gs_oDatabaseManager;
}

/********************************************************************************************
 *
 * @function ShutdownDatabaseManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownDatabaseManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oDatabaseManager)
    {
        gs_oDatabaseManager->Release();
        gs_oDatabaseManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DatabaseManager
 * @brief Constructor
 *
 ********************************************************************************************/

DatabaseManager::DatabaseManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DatabaseManager
 * @brief Copy Constructor
 * @param[in] c_oDatabaseManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

DatabaseManager::DatabaseManager(
    _in const DatabaseManager & c_oDatabaseManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ~DatabaseManager
 * @brief Destructor
 *
 ********************************************************************************************/

DatabaseManager::~DatabaseManager(void)
{
    __DebugFunction();

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatabaseManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "DatabaseManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatabaseManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{E14809BF-179A-498F-8C4B-57D9415A91F3}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall DatabaseManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function InitializePlugin
 * @brief Connects to the database and initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall DatabaseManager::InitializePlugin(void)
{
    __DebugFunction();

    mongocxx::instance oMongoInstance{}; // Create only one instance
    mongocxx::uri oUri{"mongodb://localhost:27017"};

    m_poMongoPool = std::unique_ptr<mongocxx::pool>(new mongocxx::pool(oUri));

    // Get basic user record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/BasicUser");

    // Get Confidential User Record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ConfidentialUser");

    // Add a non-leaf audit log event
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/NonLeafEvent");

    // Add a leaf audit log event
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/LeafEvent");

    // Register an organization and the new user as it's super user
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterOrganization");

    // Add a new user
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterUser");
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall DatabaseManager::SubmitRequest(
    _in const StructuredBuffer & c_oRequestStructuredBuffer,
    _out unsigned int * punSerializedResponseSizeInBytes
    )
{
    __DebugFunction();

    uint64_t un64Identifier = 0xFFFFFFFFFFFFFFFF;
    std::string strVerb = c_oRequestStructuredBuffer.GetString("Verb");
    std::string strResource = c_oRequestStructuredBuffer.GetString("Resource");
    // TODO: As an optimization, we should make sure to convert strings into 64 bit hashes
    // in order to speed up comparison. String comparisons WAY expensive.
    std::vector<Byte> stlResponseBuffer;

    // Route to the requested resource
    try
    {
        if ("GET" == strVerb)
        {
            if ("/SAIL/DatabaseManager/BasicUser" == strResource)
            {
                stlResponseBuffer = this->GetBasicUserRecord(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ConfidentialUser" == strResource)
            {
                stlResponseBuffer = this->GetConfidentialUserRecord(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/GetDCEvent" == strResource)
            {
                stlResponseBuffer = this->DigitalContractBranchExists(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/Events" == strResource)
            {
                stlResponseBuffer = this->GetListOfEvents(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else if ("POST" == strVerb)
        {
            if ("/SAIL/DatabaseManager/NonLeafEvent" == strResource)
            {
                stlResponseBuffer = this->AddNonLeafEvent(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/LeafEvent" == strResource)
            {
                stlResponseBuffer = this->AddLeafEvent(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterOrganization" == strResource)
            {
                stlResponseBuffer = this->RegisterOrganization(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterUser" == strResource)
            {
                stlResponseBuffer = this->RegisterUser(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else
        {
            _ThrowBaseException("Invalid resource.", nullptr);
        }
    }
    catch(...)
    {
        StructuredBuffer oError;
        oError.PutDword("Status", 404);
        stlResponseBuffer = oError.GetSerializedBuffer();
    }

    // Return size of response buffer
    *punSerializedResponseSizeInBytes = stlResponseBuffer.size();
    __DebugAssert(0 < *punSerializedResponseSizeInBytes);

    // Save the response buffer and increment transaction identifier which will be assigned to the next transaction
    ::pthread_mutex_lock(&m_sMutex);
    if (0xFFFFFFFFFFFFFFFF == m_unNextAvailableIdentifier)
    {
        m_unNextAvailableIdentifier = 0;
    }
    un64Identifier = m_unNextAvailableIdentifier;
    m_unNextAvailableIdentifier++;
    m_stlCachedResponse[un64Identifier] = stlResponseBuffer;
    ::pthread_mutex_unlock(&m_sMutex);

    return un64Identifier;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall DatabaseManager::GetResponse(
    _in uint64_t un64Identifier,
    _out Byte * pbSerializedResponseBuffer,
    _in unsigned int unSerializedResponseBufferSizeInBytes
    )
{
    __DebugFunction();
    __DebugAssert(0xFFFFFFFFFFFFFFFF != un64Identifier);
    __DebugAssert(nullptr != pbSerializedResponseBuffer);
    __DebugAssert(0 < unSerializedResponseBufferSizeInBytes);

    bool fSuccess = false;

    ::pthread_mutex_lock(&m_sMutex);
    if (m_stlCachedResponse.end() != m_stlCachedResponse.find(un64Identifier))
    {
        __DebugAssert(0 < m_stlCachedResponse[un64Identifier].size());

        ::memcpy((void *) pbSerializedResponseBuffer, (const void *) m_stlCachedResponse[un64Identifier].data(), m_stlCachedResponse[un64Identifier].size());
        m_stlCachedResponse.erase(un64Identifier);
        fSuccess = true;
    }
    ::pthread_mutex_unlock(&m_sMutex);

    return fSuccess;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetBasicOrganizationRecord
 * @brief Fetch basic organization record associated with organization name from the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing basic organization record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetBasicOrganizationRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    std::string strOrganizationName = c_oRequest.GetString("OrganizationName");

    // ::pthread_mutex_lock(&m_sMutex);
    // Each cleint and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access BasicOrganization collection
    mongocxx::collection oBasicOrganizationCollection = oSailDatabase["BasicOrganization"];
    // Fetch Basic Organization record associated with the strOrganizationName
    bool fFound = false;
    bsoncxx::stdx::optional<bsoncxx::document::value> oBasicOrganization = oBasicOrganizationCollection.find_one(document{} << "OrganizationName" << strOrganizationName << finalize);
    // ::pthread_mutex_unlock(&m_sMutex);
    if (bsoncxx::stdx::nullopt != oBasicOrganization)
    {
        std::cout << bsoncxx::to_json(*oBasicOrganization) << std::endl;
        fFound = true;
    }

    Dword dwStatus = 404;
    if (true == fFound)
    {
        // Get values from the document and generate the BasicOrganization StructuredBuffer
        bsoncxx::document::view oBasicOrganizationView = oBasicOrganization->view();
        bsoncxx::document::element strOrganizationGuid = oBasicOrganizationView["OrganizationUuid"];
        bsoncxx::document::element strOrganizationName = oBasicOrganizationView["OrganizationName"];

        StructuredBuffer oBasicOrganizationRecord;

        if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
        {
            oBasicOrganizationRecord.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
        }
        if (strOrganizationName && strOrganizationName.type() == type::k_utf8)
        {
            oBasicOrganizationRecord.PutString("OrganizationName", strOrganizationName.get_utf8().value.to_string());
        }
        oResponse.PutStructuredBuffer("BasicOrganizationRecord", oBasicOrganizationRecord);

        dwStatus = 200;
    }
    
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetBasicUserRecord
 * @brief Fetch basic user record associated with the 64bithash from the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing basic user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetBasicUserRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Qword qw64BitHash = c_oRequest.GetQword("Passphrase");

    // ::pthread_mutex_lock(&m_sMutex);
    // Each cleint and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access BasicUser collection
    mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
    // Fetch basic user record associated with the qw64BitHash
    bool fFound = false;
    bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUser = oBasicUserCollection.find_one(document{} << "64BitHash" << (double)qw64BitHash << finalize);
    // ::pthread_mutex_unlock(&m_sMutex);
    if (bsoncxx::stdx::nullopt != oBasicUser)
    {
        std::cout << bsoncxx::to_json(*oBasicUser) << std::endl;
        fFound = true;
    }

    Dword dwStatus = 404;
    if (true == fFound)
    {
        // Get values from the document and generate the BasicUser StructuredBuffer
        bsoncxx::document::view oBasicUserView = oBasicUser->view();
        bsoncxx::document::element strUserGuid = oBasicUserView["UserUuid"];
        bsoncxx::document::element strOrganizationGuid = oBasicUserView["OrganizationUuid"];
        bsoncxx::document::element dwAccountStatus = oBasicUserView["AccountStatus"];
        bsoncxx::document::element stlAccountKeyWrappedWithPDK = oBasicUserView["WrappedAccountKey"];

        StructuredBuffer oBasicUserRecord;

        if (strUserGuid && strUserGuid.type() == type::k_utf8)
        {
            oBasicUserRecord.PutGuid("UserGuid", Guid(strUserGuid.get_utf8().value.to_string().c_str()));
        }
        if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
        {
            oBasicUserRecord.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
        }
        if (dwAccountStatus && dwAccountStatus.type() == type::k_double)
        {
            oBasicUserRecord.PutDword("AccountStatus", (Dword) dwAccountStatus.get_double().value);
        }
        if (stlAccountKeyWrappedWithPDK && stlAccountKeyWrappedWithPDK.type() == type::k_binary)
        {
            oBasicUserRecord.PutBuffer("WrappedAccountKey", stlAccountKeyWrappedWithPDK.get_binary().bytes, stlAccountKeyWrappedWithPDK.get_binary().size);
        }

        oResponse.PutStructuredBuffer("BasicUserRecord", oBasicUserRecord);
        dwStatus = 200;
    }
    
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetConfidentialUserRecord
 * @brief Fetch confidential user record from the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing confidential user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetConfidentialUserRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    std::string strUserUuid = c_oRequest.GetString("UserUuid");

    // ::pthread_mutex_lock(&m_sMutex);
    // Each cleint and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access ConfidentialOrganizationOrUser collection
    mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
    // Fetch confidential user record associated with the strUserUuid
    bool fFound = false;
    bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialUser = oConfidentialUserCollection.find_one(document{} << "OrganizationOrUserUuid" << strUserUuid << finalize);
    // ::pthread_mutex_unlock(&m_sMutex);
    if (bsoncxx::stdx::nullopt != oConfidentialUser)
    {
        std::cout << bsoncxx::to_json(*oConfidentialUser) << std::endl;
        fFound = true;
    }

    if (true == fFound)
    {
        // Get values from the document and generate the oConfidentialUser StructuredBuffer
        bsoncxx::document::view oConfidentialUserView = oConfidentialUser->view();
        bsoncxx::document::element stlIv = oConfidentialUserView["IV"];
        bsoncxx::document::element stlTag = oConfidentialUserView["Tag"];
        bsoncxx::document::element stlEncryptedSsb = oConfidentialUserView["EncryptedSsb"];

        StructuredBuffer oConfidentialUserRecord;

        oConfidentialUserRecord.PutGuid("OrganizationOrUserGuid", Guid(strUserUuid.c_str()));

        if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
        {
            oConfidentialUserRecord.PutBuffer("EncryptedSsb", stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
        }
        oResponse.PutDword("Status", 200);
        oResponse.PutStructuredBuffer("ConfidentialOrganizationOrUserRecord", oConfidentialUserRecord);
    }
    else
    {
        oResponse.PutDword("Status", 404);
    }

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DigitalContractBranchExists
 * @brief Given an organization guid fetch digital contract event guid, if exists
 * @param[in] c_oRequest contains the organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing digital contract guid
 *
 ********************************************************************************************/

 std::vector<Byte> __thiscall DatabaseManager::DigitalContractBranchExists(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    // Get root event guid
    StructuredBuffer oGetRootRequest;
    oGetRootRequest.PutString("ParentGuid", "{00000000-0000-0000-0000-000000000000}");
    oGetRootRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
    StructuredBuffer oFilters;
    oGetRootRequest.PutStructuredBuffer("Filters", oFilters);
    StructuredBuffer oRootEvent = StructuredBuffer(this->GetListOfEvents(oGetRootRequest)).GetStructuredBuffer("ListOfEvents");
    _ThrowBaseExceptionIf((0 == oRootEvent.GetNamesOfElements().size()), "Root event does not exist", nullptr);
    std::string strRootEventGuid = oRootEvent.GetStructuredBuffer(oRootEvent.GetNamesOfElements()[0].c_str()).GetGuid("EventGuid").ToString(eHyphensAndCurlyBraces);

    // Check if DC branch event exists
    StructuredBuffer oGetDCRequest;
    oGetDCRequest.PutString("ParentGuid", strRootEventGuid);
    oGetDCRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
    oGetDCRequest.PutStructuredBuffer("Filters", c_oRequest.GetStructuredBuffer("Filters"));
    StructuredBuffer oBranchEvent(this->GetListOfEvents(oGetDCRequest));
    StructuredBuffer oListOfEvents(oBranchEvent.GetStructuredBuffer("ListOfEvents"));

    Dword dwStatus = 404;
    if (0 < oListOfEvents.GetNamesOfElements().size())
    {
        StructuredBuffer oEvent(oListOfEvents.GetStructuredBuffer(oListOfEvents.GetNamesOfElements()[0].c_str()));
        oResponse.PutString("DCEventGuid", oEvent.GetGuid("EventGuid").ToString(eHyphensAndCurlyBraces));
        dwStatus = 200;
    }
    else 
    {
        oResponse.PutString("RootEventGuid", strRootEventGuid);
    }
    
    // Add transaction status
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetListOfEvents
 * @brief Fetch confidential user record from the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing confidential user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetListOfEvents(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    StructuredBuffer oListOfEvents;
    std::string strParentGuid = c_oRequest.GetString("ParentGuid");
    std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
    StructuredBuffer oFilters(c_oRequest.GetStructuredBuffer("Filters"));

    // ::pthread_mutex_lock(&m_sMutex);
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Get root event if parent guid is "{00000000-0000-0000-0000-000000000000}"
    // Otherwise return all the child events for the given parent guid
    if ("{00000000-0000-0000-0000-000000000000}" == strParentGuid)
    {
        bsoncxx::stdx::optional<bsoncxx::document::value> oAuditLogDocument = oSailDatabase["AuditLog"].find_one(document{} 
                                                                                                            << "ParentGuid" << strParentGuid 
                                                                                                            << "OrganizationGuid" << strOrganizationGuid
                                                                                                            << finalize);
        this->GetEventObjectBlob(oSailDatabase, oFilters, oAuditLogDocument->view(), &oListOfEvents);
    }
    else 
    {
        // Fetch events from the AuditLog collection associated with a parent guid
        mongocxx::cursor oAuditLogCursor = oSailDatabase["AuditLog"].find(document{} 
                                                                            << "ParentGuid" << strParentGuid 
                                                                            << "OrganizationGuid" << strOrganizationGuid
                                                                            << finalize);
        // ::pthread_mutex_unlock(&m_sMutex);
        // Parse all returned documents, apply filters, and add to the structured buffer containing the list of events
        for (auto&& oDocumentView : oAuditLogCursor)
        {
            this->GetEventObjectBlob(oSailDatabase, oFilters, oDocumentView, &oListOfEvents);
        }
    }

    // Send back status of the transaction and list of events
    oResponse.PutDword("Status", 200);
    oResponse.PutStructuredBuffer("ListOfEvents", oListOfEvents);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetEventObjectBlob
 * @brief Apply filters and add logs to the listofevents
 * @param[in] oDocumentView contains mongo document view
 * @param[out] poListOfEvents contains pointer to the structured buffer containing list of events
 * @throw BaseException Error StructuredBuffer element not found
 *
 ********************************************************************************************/

void __thiscall DatabaseManager::GetEventObjectBlob(
    _in const mongocxx::database & c_oSailDatabase,
    _in const StructuredBuffer & c_oFilters,
    _in const bsoncxx::document::view & c_oDocumentView,
    _out StructuredBuffer * poListOfEvents
    )
{
    __DebugFunction();

    // Get values from the document and generate the event log StructuredBuffer
    StructuredBuffer oEvent;
    bsoncxx::document::element strPlainTextObjectBlobGuid = c_oDocumentView["PlainTextObjectBlobGuid"];
    bsoncxx::document::element strEventGuid = c_oDocumentView["EventGuid"];
    bsoncxx::document::element strOrganizationGuid = c_oDocumentView["OrganizationGuid"];
    bsoncxx::document::element fIsLeaf = c_oDocumentView["IsLeaf"];

    // Get the PlainTextObjectBlob and then get the Object blob
    // Apply filters if any
    // If the object is not filtered out then add the audit log information to the Event structured buffer
    bool fAddToListOfEvents = false;
    uint32_t unObjectSequenceNumber = 0;
    if (strPlainTextObjectBlobGuid && strPlainTextObjectBlobGuid.type() == type::k_utf8)
    {
        Guid oPlainTextObjectBlobGuid(strPlainTextObjectBlobGuid.get_utf8().value.to_string().c_str());
        // Fetch events from the PlainTextObjectBlob collection associated with the event guid
        bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = c_oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                        << "PlainTextObjectBlobGuid" <<  oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces) 
                                                                                                                                        << finalize);
        if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
        {
            bsoncxx::document::element strObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
            if (strObjectGuid && strObjectGuid.type() == type::k_utf8)
            {
                Guid oObjectGuid(strObjectGuid.get_utf8().value.to_string().c_str());
                oEvent.PutGuid("ObjectGuid", oObjectGuid);
                // Fetch events from the Object collection associated with the object guid
                bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = c_oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces) << finalize);
                if (bsoncxx::stdx::nullopt != oObjectDocument)
                {
                    bsoncxx::document::element stlObjectBlob = oObjectDocument->view()["ObjectBlob"];
                    if (stlObjectBlob && stlObjectBlob.type() == type::k_binary)
                    {
                        StructuredBuffer oObject(stlObjectBlob.get_binary().bytes, stlObjectBlob.get_binary().size);
                        // Get event sequence number to; be used in the filter (if specified) and/or used as event structuredbuffer name (so that the events are sorted sequentially)
                        unObjectSequenceNumber = oObject.GetUnsignedInt32("SequenceNumber");
                        // Apply supplied filters on audit logs
                        std::vector<std::string> stlFilters = c_oFilters.GetNamesOfElements();
                        try 
                        {
                            for (std::string strFilter : stlFilters)
                            {
                                if ("SequenceNumber" == strFilter)
                                {
                                    uint32_t unFilterSequenceNumber = c_oFilters.GetUnsignedInt32("SequenceNumber");
                                    _ThrowBaseExceptionIf((unObjectSequenceNumber < unFilterSequenceNumber), "Object sequence number is less than the specified starting sequence number.", nullptr);
                                }
                                else if ("MinimumDate" == strFilter)
                                {
                                    uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                    uint64_t unFilterMinimumDate = c_oFilters.GetUnsignedInt64("MinimumDate");
                                    _ThrowBaseExceptionIf((unObjectTimestamp < unFilterMinimumDate), "Object timestamp is less than the specified minimum date.", nullptr);
                                }
                                else if ("MaximumDate" == strFilter)
                                {
                                    uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                    uint64_t unFilterMaximumDate = c_oFilters.GetUnsignedInt64("MaximumDate");
                                    _ThrowBaseExceptionIf((unObjectTimestamp > unFilterMaximumDate), "Object timestamp is greater than the specified maximum date.", nullptr);
                                }
                                else if ("TypeOfEvents" == strFilter)
                                {
                                    Qword qwObjectEventType = oObject.GetQword("EventType");
                                    Qword qwFilterEventType = c_oFilters.GetQword("TypeOfEvents");
                                    _ThrowBaseExceptionIf((qwObjectEventType != qwFilterEventType), "Object type is not the same as the specified event type.", nullptr);
                                }
                                else if ("DCGuid" == strFilter)
                                {
                                    // Word wType = Guid(strEventGuid.get_utf8().value.to_string().c_str()).GetObjectType();
                                    // _ThrowBaseExceptionIf((eAuditEventBranchNode != wType), "No DC guid exists for this type of object.", nullptr);
                                    StructuredBuffer oPlainTextMetadata(oObject.GetStructuredBuffer("PlainTextEventData"));
                                    std::string strPlainObjectDCGuid = oPlainTextMetadata.GetString("GuidOfDcOrVm");
                                    std::string strFilterDcGuid = c_oFilters.GetString("DCGuid");
                                    _ThrowBaseExceptionIf((1 != oPlainTextMetadata.GetDword("BranchType")), "The audit log is not for a digital contract", nullptr);
                                    _ThrowBaseExceptionIf((strPlainObjectDCGuid != strFilterDcGuid), "The DC guid does not match the requested dc guid", nullptr);
                                }
                                // TODO: Add VMGuid filters
                                // else if ("VMGuid" == strFilter)
                                // {

                                // }
                            }
                            fAddToListOfEvents = true;
                        }
                        catch (BaseException oException)
                        {
                            fAddToListOfEvents = false;
                        }
                        if (true == fAddToListOfEvents)
                        {
                            // If the audit log object is not filtered out then add it to the Event structured buffer
                            oEvent.PutStructuredBuffer("ObjectBlob", oObject);
                            // Add PlainTextObjectBlobGuid to the Event structured buffer
                            oEvent.PutGuid("PlainTextObjectBlobGuid", oPlainTextObjectBlobGuid);
                        }
                    }
                }
            }
        }
    }
    // If the object is not filtered out then add the audit log information to the Event structured buffer
    if (true == fAddToListOfEvents)
    {
        // Add other information to the event 
        if (strEventGuid && strEventGuid.type() == type::k_utf8)
        {
            oEvent.PutGuid("EventGuid", Guid(strEventGuid.get_utf8().value.to_string().c_str()));

            if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
            {
                oEvent.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
            }
            if (fIsLeaf && fIsLeaf.type() == type::k_bool)
            {
                oEvent.PutBoolean("isLeaf", fIsLeaf.get_bool().value);
            }
            // Add event to the response structured buffer that contains list of audit events 
            poListOfEvents->PutStructuredBuffer(std::to_string(unObjectSequenceNumber).c_str(), oEvent);
        }
    }
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetNextSequenceNumber
 * @brief Fetch next sequence number from the parent event and update the parent event's next sequence number
 * @param[in] c_oRequest contains organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Sequence number for a non leaf event
 *
 ********************************************************************************************/

uint32_t __thiscall DatabaseManager::GetNextSequenceNumber(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;
    uint32_t unSequenceNumber = -1;

    std::string strEventGuid = c_oRequest.GetString("EventGuid");
    std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");

    // ::pthread_mutex_lock(&m_sMutex);
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Get root event if it exists
    // Otherwise return 0 as the next sequence number
    
    bsoncxx::stdx::optional<bsoncxx::document::value> oAuditLogDocument = oSailDatabase["AuditLog"].find_one(document{} 
                                                                                                        << "EventGuid" << strEventGuid 
                                                                                                        << "OrganizationGuid" << strOrganizationGuid
                                                                                                        << finalize);
    if (oAuditLogDocument)
    {
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            bsoncxx::document::element unNextSequenceNumber = oAuditLogDocument->view()["NextSequenceNumber"];
            if (unNextSequenceNumber && unNextSequenceNumber.type() == type::k_double)
            {
                unSequenceNumber = (uint32_t) unNextSequenceNumber.get_double().value;
            }
            oSailDatabase["AuditLog"].update_one(*poSession, document{} << "EventGuid" << strEventGuid << finalize,
                                                document{} << "$set" << open_document <<
                                                "NextSequenceNumber" << (double)(unSequenceNumber + 1) << close_document << finalize);
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
        }
        catch (mongocxx::exception& e) 
        {
            std::cout << "Collection transaction exception: " << e.what() << std::endl;
        }
    }

    return unSequenceNumber;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function AddNonLeafEvent
 * @brief Add a non leaf audit log event to the database
 * @param[in] c_oRequest contains the event information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddNonLeafEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    // Create guids for the documents
    Guid oObjectGuid, oPlainTextObjectBlobGuid;
    uint32_t unSequenceNumber, unNextSequenceNumber = 0;
    StructuredBuffer oNonLeafEvent(c_oRequest.GetStructuredBuffer("NonLeafEvent"));
    // Get parent event's next sequence number and use it to assign a sequence number to the branch event
    StructuredBuffer oGetRoot;
    oGetRoot.PutString("EventGuid", oNonLeafEvent.GetString("ParentGuid"));
    oGetRoot.PutString("OrganizationGuid", oNonLeafEvent.GetString("OrganizationGuid"));
    unSequenceNumber = this->GetNextSequenceNumber(oGetRoot);
    if (-1 == unSequenceNumber)
    {
        unSequenceNumber = 0;
    }

    // Create an audit log event document
    bsoncxx::document::value oEventDocumentValue = bsoncxx::builder::stream::document{}
      << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
      << "OrganizationGuid" << oNonLeafEvent.GetString("OrganizationGuid")
      << "ParentGuid" << oNonLeafEvent.GetString("ParentGuid")
      << "EventGuid" << oNonLeafEvent.GetString("EventGuid")
      << "NextSequenceNumber" << (double) unNextSequenceNumber
      << "IsLeaf" << false
      << finalize;

    // Create an object document
    StructuredBuffer oObject;
    oObject.PutString("EventGuid", oNonLeafEvent.GetString("EventGuid"));
    oObject.PutString("ParentGuid", oNonLeafEvent.GetString("ParentGuid"));
    oObject.PutString("OrganizationGuid", oNonLeafEvent.GetString("OrganizationGuid"));
    oObject.PutQword("EventType", oNonLeafEvent.GetQword("EventType"));
    oObject.PutUnsignedInt64("Timestamp", oNonLeafEvent.GetUnsignedInt64("Timestamp"));
    oObject.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
    oObject.PutStructuredBuffer("PlainTextEventData", oNonLeafEvent.GetStructuredBuffer("PlainTextEventData"));
    bsoncxx::types::b_binary oObjectBlob
    {
        bsoncxx::binary_sub_type::k_binary,
        uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
        oObject.GetSerializedBufferRawDataPtr()
    };
    bsoncxx::document::value oObjectDocumentValue = bsoncxx::builder::stream::document{}
      << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
      << "ObjectType" << eAuditEventBranchNode
      << "IsEncrypted" << false
      << "ObjectBlob" << oObjectBlob
      << finalize;

    // Create a plain text object document
    bsoncxx::document::value oPlainTextObjectDocumentValue = bsoncxx::builder::stream::document{}
      << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
      << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
      << "ObjectType" << eAuditEventBranchNode
      << finalize;

    // ::pthread_mutex_lock(&m_sMutex);
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access AuditLog collection
    mongocxx::collection oAuditLogCollection = oSailDatabase["AuditLog"];
    // Create a transaction callback
    Dword dwStatus = 204;
    mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
    {
        // Insert document in the AuditLog collection
        auto oResult = oAuditLogCollection.insert_one(*poSession, oEventDocumentValue.view());
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Access Object collection
            mongocxx::collection oObjectCollection = oSailDatabase["Object"];
            // Insert document in the Object collection
            oResult = oObjectCollection.insert_one(*poSession, oObjectDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access PlainTextObjectBlob collection
                mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                // Insert document in the PlainTextObjectBlob collection
                oResult = oPlainTextObjectCollection.insert_one(*poSession, oPlainTextObjectDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    dwStatus = 200;
                }
            }
        }
    };
    // Create a session and start the transaction
    mongocxx::client_session oSession = oClient->start_session();
    try 
    {
        oSession.with_transaction(oCallback);
    }
    catch (mongocxx::exception& e) 
    {
        std::cout << "Collection transaction exception: " << e.what() << std::endl;
    }

    // ::pthread_mutex_unlock(&m_sMutex);

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function AddLeafEvent
 * @brief Add leaf audit log events to the database
 * @param[in] c_oRequest contains the event information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddLeafEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    // Get required parameters from the request
    std::string strIdentifierOfParent = c_oRequest.GetString("ParentGuid");
    StructuredBuffer oLeafEvents(c_oRequest.GetStructuredBuffer("LeafEvents"));
    std::vector<std::string> stlEvents = oLeafEvents.GetNamesOfElements();
    // Loop through the array of events, create document for each event, and add them to the relevant vector:
    // stlEventDocuments: holds audit log documents
    // stlObjectDocuments: holds object documents
    // stlPlainTextObjectDocuments: holds plain text object documents
    std::vector<bsoncxx::document::value> stlEventDocuments, stlObjectDocuments, stlPlainTextObjectDocuments;
    for (unsigned int unIndex = 0; unIndex < stlEvents.size(); ++unIndex)
    {
        Guid oObjectGuid, oPlainTextObjectBlobGuid;

        StructuredBuffer oEvent(oLeafEvents.GetStructuredBuffer(stlEvents[unIndex].c_str()));

        // Create an audit log event document and add it to stlEventDocuments vector
        stlEventDocuments.push_back(bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "OrganizationGuid" << oEvent.GetString("OrganizationGuid")
        << "ParentGuid" << strIdentifierOfParent
        << "EventGuid" << oEvent.GetString("EventGuid")
        << "IsLeaf" << true
        << finalize);

        // Create an object document and add it to stlObjectDocuments vector
        // Parse and cast event parameters as json arrays are not parsed by rest portal
        StructuredBuffer oObject;
        oObject.PutString("EventGuid", oEvent.GetString("EventGuid"));
        oObject.PutString("ParentGuid", strIdentifierOfParent);
        // Convert number type parameters to the required data type 
        oObject.PutQword("EventType", (Qword) oEvent.GetFloat64("EventType"));
        oObject.PutUnsignedInt64("Timestamp", (uint64_t) oEvent.GetFloat64("Timestamp"));
        oObject.PutUnsignedInt32("SequenceNumber", (uint32_t) oEvent.GetFloat64("SequenceNumber"));
        // Decode string first and then initialize a vector from it
        std::string strDecodedEventData = ::Base64Decode(oEvent.GetString("EncryptedEventData"));
        std::vector<Byte> stlEventData(strDecodedEventData.begin(), strDecodedEventData.end());
        oObject.PutBuffer("EncryptedEventData", stlEventData);
        // Create a binary blob to be inserted in the document
        bsoncxx::types::b_binary oObjectBlob
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
            oObject.GetSerializedBufferRawDataPtr()
        };
        stlObjectDocuments.push_back(bsoncxx::builder::stream::document{}
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectBlob" << oObjectBlob
        << finalize);

        // Create a plain text object document and add it to stlPlainTextObjectDocuments vector
        stlPlainTextObjectDocuments.push_back(bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectType" << eAuditEventPlainTextLeafNode
        << finalize);
    }

    // ::pthread_mutex_lock(&m_sMutex);
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access AuditLog collection
    mongocxx::collection oAuditLogCollection = oSailDatabase["AuditLog"];
    // Create a transaction callback
    Dword dwStatus = 204;
    mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
    {
        // Insert audit log documents in the AuditLog collection
        auto oResult = oAuditLogCollection.insert_many(*poSession, stlEventDocuments);
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Access Object collection
            mongocxx::collection oObjectCollection = oSailDatabase["Object"];
            // Insert object documents in the Object collection
            oResult = oObjectCollection.insert_many(*poSession, stlObjectDocuments);
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access PlainTextObjectBlob collection
                mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                // Insert plain text object blob documents in the PlainTextObjectBlob collection
                oResult = oPlainTextObjectCollection.insert_many(*poSession, stlPlainTextObjectDocuments);
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    dwStatus = 200;
                }
            }
        }
    };
    // Create a session and start the transaction
    mongocxx::client_session oSession = oClient->start_session();
    try 
    {
        oSession.with_transaction(oCallback);
    }
    catch (mongocxx::exception& e) 
    {
        std::cout << "Collection transaction exception: " << e.what() << std::endl;
    }

    // ::pthread_mutex_unlock(&m_sMutex);

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterOrganization
 * @brief Add a new organization to the database and call AddSuperUser() to add the organization's super user
 * @param[in] c_oRequest contains the organization information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource is called when a new user tries registering themselves. Their organziation 
 *  is registered first and the user is registered as the organization's super admin, if the organization
 *  is registered successfully.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterOrganization(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    // Check if a record with the OrganizationName exists in the database
    StructuredBuffer oRequest(c_oRequest.GetStructuredBuffer("Request"));
    StructuredBuffer oBasicOrganizationRecord(this->GetBasicOrganizationRecord(oRequest));
    if (404 == oBasicOrganizationRecord.GetDword("Status"))
    {
        // Create guids for the documents
        Guid oBasicOrganizationGuid, oConfidentialOrganizationGuid;
        // Create guid for organziation and organziation root key
        // Todo: get organziation root key guid
        std::string strOrganizationGuid = Guid(eOrganization).ToString(eHyphensAndCurlyBraces);
        std::string strOrganziationRootKeyGuid = Guid().ToString(eHyphensAndCurlyBraces);
        // Create guid for the super user
        std::string strUserGuid = Guid(eUser).ToString(eHyphensAndCurlyBraces);
        // Get organization name
        std::string strOrganizationName = oRequest.GetString("OrganizationName");

        // Create basic organziation document
        // TODO: add public root key certificate and wrapped account encryption keys
        bsoncxx::document::value oBasicOrganizationDocumentValue = bsoncxx::builder::stream::document{}
            << "OrganizationUuid" << strOrganizationGuid
            << "OrganizationName" << strOrganizationName
            << finalize;

        // Create confidential organziation document
        StructuredBuffer oConfidentialOrganziation;
        oConfidentialOrganziation.PutString("OrganziationUuid", strOrganizationGuid);
        oConfidentialOrganziation.PutString("OrganizationRootKeyUuid", strOrganziationRootKeyGuid);
        // TODO: get account wrapping key guid
        oConfidentialOrganziation.PutString("AccountWrappingKeyUuid", Guid().ToString(eHyphensAndCurlyBraces));
        oConfidentialOrganziation.PutString("OrganizationName", strOrganizationName);
        oConfidentialOrganziation.PutString("OrganizationAddress", oRequest.GetString("OrganizationAddress"));
        oConfidentialOrganziation.PutString("PrimaryContactName", oRequest.GetString("PrimaryContactName"));
        oConfidentialOrganziation.PutString("PrimaryContactTitle", oRequest.GetString("PrimaryContactTitle"));
        oConfidentialOrganziation.PutString("PrimaryContactEmail", oRequest.GetString("PrimaryContactEmail"));
        oConfidentialOrganziation.PutString("PrimaryContactPhoneNumber", oRequest.GetString("PrimaryContactPhoneNumber"));
        oConfidentialOrganziation.PutString("SecondaryContactName", oRequest.GetString("SecondaryContactName"));
        oConfidentialOrganziation.PutString("SecondaryContactTitle", oRequest.GetString("SecondaryContactTitle"));
        oConfidentialOrganziation.PutString("SecondaryContactEmail", oRequest.GetString("SecondaryContactEmail"));
        oConfidentialOrganziation.PutString("SecondaryContactPhoneNumber", oRequest.GetString("SecondaryContactPhoneNumber"));
        oConfidentialOrganziation.PutString("AccountCreatorUuid", strUserGuid);
        oConfidentialOrganziation.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
        // TODO: encrypt oConfidentialOrganization
        bsoncxx::types::b_binary oEncryptedSsb
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oConfidentialOrganziation.GetSerializedBufferRawDataSizeInBytes()),
            oConfidentialOrganziation.GetSerializedBufferRawDataPtr()
        };
        bsoncxx::document::value oConfidentialOrganziationDocumentValue = bsoncxx::builder::stream::document{}
        << "OrganizationOrUserUuid" << strOrganizationGuid
        << "EncryptedSsb" << oEncryptedSsb
        << finalize;

        // ::pthread_mutex_lock(&m_sMutex);
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access BasicOrganization collection
        mongocxx::collection oBasicOrganziationCollection = oSailDatabase["BasicOrganization"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the BasicOrganization collection
            auto oResult = oBasicOrganziationCollection.insert_one(*poSession, oBasicOrganizationDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access Object collection
                mongocxx::collection oConfidentialOrganizationCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
                // Insert document in the ConfidentialOrganizationOrUser collection
                oResult = oConfidentialOrganizationCollection.insert_one(*poSession, oConfidentialOrganziationDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    oRequest.PutString("OrganizationUuid", strOrganizationGuid);
                    oRequest.PutString("UserUuid", strUserGuid);
                    oRequest.PutQword("AccessRights", 7);
                    // Add new user as the super user of the organization
                    dwStatus = StructuredBuffer(this->AddSuperUser(oRequest)).GetDword("Status");
                    _ThrowBaseExceptionIf((201 != dwStatus), "Error creating super user", nullptr);
                    // Send back organization guid to register a root event
                    oResponse.PutString("OrganizationGuid", strOrganizationGuid);
                }
            }
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
        }
        catch (mongocxx::exception& e) 
        {
            std::cout << "Collection transaction exception: " << e.what() << std::endl;
        }
    }
    // ::pthread_mutex_unlock(&m_sMutex);

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function AddSuperUser
 * @brief Add organziation's super user to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource registers a new user as the super user of a newly registered organziation.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddSuperUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    // Create guids for the documents
    Guid oBasicUserGuid, oConfidentialUserGuid;
    // Create guids for user and user root key
    // Todo: get user root key guid
    std::string strUserGuid = c_oRequest.GetString("UserUuid");
    Guid oUserRootKeyGuid;

    std::string strEmail = c_oRequest.GetString("Email");
    std::string strPassword = c_oRequest.GetString("Password");
    // Get 64 bit hash of "strEmail/strPassword" string
    Qword qw64BitHashPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strPassword).c_str(), false);
    // Generate "strEmail/strPassword" derived key
    std::string strDerivedKey = ::Base64HashOfEmailPassword(strEmail, strPassword);
    // Generate account encryption key
    std::vector<Byte> stlAccountEncryptionKey = ::GenerateAccountKey();
    // Wrap account encryption key with strDerivedKey
    std::vector<Byte> stlWrappedAccountEncryptionKey = ::EncryptUsingPasswordKey(stlAccountEncryptionKey, strDerivedKey);

    // Create basic user document
    bsoncxx::types::b_binary oWrappedAccountEncryptionKey {bsoncxx::binary_sub_type::k_binary,
                                  uint32_t(stlWrappedAccountEncryptionKey.size()),
                                  stlWrappedAccountEncryptionKey.data()};
    bsoncxx::document::value oBasicUserDocumentValue = bsoncxx::builder::stream::document{}
        << "64BitHash" << (double) qw64BitHashPassphrase
        << "OrganizationUuid" << c_oRequest.GetString("OrganizationUuid")
        << "UserUuid" << strUserGuid
        << "AccountStatus" << (double) ACCOUNT_STATUS_NEW
        << "WrappedAccountKey" << oWrappedAccountEncryptionKey
        << finalize;

    // Create confidential user document
    StructuredBuffer oConfidentialUser;
    oConfidentialUser.PutGuid("UserGuid", Guid(strUserGuid.c_str()));
    oConfidentialUser.PutGuid("UserRootKeyGuid", oUserRootKeyGuid);
    oConfidentialUser.PutQword("AccessRights", c_oRequest.GetQword("AccessRights"));
    oConfidentialUser.PutString("Username", c_oRequest.GetString("Name"));
    oConfidentialUser.PutString("Title", c_oRequest.GetString("Title"));
    oConfidentialUser.PutString("Email", strEmail);
    oConfidentialUser.PutString("PhoneNumber", c_oRequest.GetString("PhoneNumber"));
    oConfidentialUser.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
    // Encrypt oConfidentialUser StructuredBuffer with password derived key
    std::vector<Byte> stlEncryptedWithPasswordDerivedKey = ::EncryptUsingPasswordKey(oConfidentialUser.GetSerializedBuffer(), strDerivedKey);
    // Encrypt stlEncryptedWithPasswordDerivedKey with sail secret key
    StructuredBuffer oEncrypedConfidentialUser = ::EncryptUsingSailSecretKey(stlEncryptedWithPasswordDerivedKey);
    bsoncxx::types::b_binary oEncryptedSsb
    {
        bsoncxx::binary_sub_type::k_binary,
        uint32_t(oEncrypedConfidentialUser.GetSerializedBufferRawDataSizeInBytes()),
        oEncrypedConfidentialUser.GetSerializedBufferRawDataPtr()
    };
    bsoncxx::document::value oConfidentialUserDocumentValue = bsoncxx::builder::stream::document{}
      << "OrganizationOrUserUuid" << strUserGuid
      << "EncryptedSsb" << oEncryptedSsb
      << finalize;

    // ::pthread_mutex_lock(&m_sMutex);
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Access BasicUser collection
    mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
    // Create a transaction callback
    Dword dwStatus = 204;
    mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
    {
        // Insert document in the BasicUser collection
        auto oResult = oBasicUserCollection.insert_one(*poSession, oBasicUserDocumentValue.view());
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Access Object collection
            mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
            // Insert document in the ConfidentialOrganizationOrUser collection
            oResult = oConfidentialUserCollection.insert_one(*poSession, oConfidentialUserDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                 dwStatus = 201;
            }
        }
    };
    // Create a session and start the transaction
    mongocxx::client_session oSession = oClient->start_session();
    try 
    {
        oSession.with_transaction(oCallback);
    }
    catch (mongocxx::exception& e) 
    {
        std::cout << "Collection transaction exception: " << e.what() << std::endl;
    }

    // ::pthread_mutex_unlock(&m_sMutex);

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterUser
 * @brief Add a new user to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource is called when an Admin user of an organization tries to register a new user.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    StructuredBuffer oRequest(c_oRequest.GetStructuredBuffer("Request"));
    std::string strEmail = oRequest.GetString("Email");
    std::string strPassword = oRequest.GetString("Password");
    // Get 64 bit hash of "strEmail/strPassword" string
    Qword qw64BitHashPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strPassword).c_str(), false);
    // Check if a record with the qw64BitHashPassphrase exists in the database
    StructuredBuffer oPassphrase;
    oPassphrase.PutQword("Passphrase", qw64BitHashPassphrase);
    StructuredBuffer oBasicUserRecord(this->GetBasicUserRecord(oPassphrase));

    if (404 == oBasicUserRecord.GetDword("Status"))
    {
        // Create guids for the documents
        Guid oBasicUserGuid, oConfidentialUserGuid;
        // Create guids for user and user root key
        // Todo: get user root key guid
        std::string strUserGuid = Guid(eUser).ToString(eHyphensAndCurlyBraces);
        Guid oUserRootKeyGuid;
        // Generate "strEmail/strPassword" derived key
        std::string strDerivedKey = ::Base64HashOfEmailPassword(strEmail, strPassword);
        // Generate account encryption key
        std::vector<Byte> stlAccountEncryptionKey = ::GenerateAccountKey();
        // Wrap account encryption key with strDerivedKey
        std::vector<Byte> stlWrappedAccountEncryptionKey = ::EncryptUsingPasswordKey(stlAccountEncryptionKey, strDerivedKey);

        // Create basic user document
        bsoncxx::types::b_binary oWrappedAccountEncryptionKey {bsoncxx::binary_sub_type::k_binary,
                                    uint32_t(stlWrappedAccountEncryptionKey.size()),
                                    stlWrappedAccountEncryptionKey.data()};
        bsoncxx::document::value oBasicUserDocumentValue = bsoncxx::builder::stream::document{}
            << "64BitHash" << (double) qw64BitHashPassphrase
            << "OrganizationUuid" << oRequest.GetString("OrganizationGuid")
            << "UserUuid" << strUserGuid
            << "AccountStatus" << (double) ACCOUNT_STATUS_NEW
            << "WrappedAccountKey" << oWrappedAccountEncryptionKey
            << finalize;

        // Create confidential user document
        StructuredBuffer oConfidentialUser;
        oConfidentialUser.PutGuid("UserGuid", Guid(strUserGuid.c_str()));
        oConfidentialUser.PutGuid("UserRootKeyGuid", oUserRootKeyGuid);
        oConfidentialUser.PutQword("AccessRights", oRequest.GetQword("AccessRights"));
        oConfidentialUser.PutString("Username", oRequest.GetString("Name"));
        oConfidentialUser.PutString("Title", oRequest.GetString("Title"));
        oConfidentialUser.PutString("Email", strEmail);
        oConfidentialUser.PutString("PhoneNumber", oRequest.GetString("PhoneNumber"));
        oConfidentialUser.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
        // Encrypt oConfidentialUser StructuredBuffer with password derived key
        std::vector<Byte> stlEncryptedWithPasswordDerivedKey = ::EncryptUsingPasswordKey(oConfidentialUser.GetSerializedBuffer(), strDerivedKey);
        // Encrypt stlEncryptedWithPasswordDerivedKey with sail secret key
        StructuredBuffer oEncrypedConfidentialUser = ::EncryptUsingSailSecretKey(stlEncryptedWithPasswordDerivedKey);
        bsoncxx::types::b_binary oEncryptedSsb
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oEncrypedConfidentialUser.GetSerializedBufferRawDataSizeInBytes()),
            oEncrypedConfidentialUser.GetSerializedBufferRawDataPtr()
        };
        bsoncxx::document::value oConfidentialUserDocumentValue = bsoncxx::builder::stream::document{}
        << "OrganizationOrUserUuid" << strUserGuid
        << "EncryptedSsb" << oEncryptedSsb
        << finalize;

        // ::pthread_mutex_lock(&m_sMutex);
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access BasicUser collection
        mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the BasicUser collection
            auto oResult = oBasicUserCollection.insert_one(*poSession, oBasicUserDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access Object collection
                mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
                // Insert document in the ConfidentialOrganizationOrUser collection
                oResult = oConfidentialUserCollection.insert_one(*poSession, oConfidentialUserDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    dwStatus = 201;
                }
            }
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
        }
        catch (mongocxx::exception& e) 
        {
            std::cout << "Collection transaction exception: " << e.what() << std::endl;
        }
    }

    // ::pthread_mutex_unlock(&m_sMutex);

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
