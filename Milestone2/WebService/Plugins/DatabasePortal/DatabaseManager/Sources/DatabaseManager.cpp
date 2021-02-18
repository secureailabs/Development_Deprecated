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
        oResponse.PutDword("Status", 200);
        oResponse.PutStructuredBuffer("BasicUserRecord", oBasicUserRecord);
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
    StructuredBuffer oRootEvent(this->GetListOfEvents(oGetRootRequest));
    std::string strRootGuid = oRootEvent.GetString("RootEventGuid");

    // Check if DC branch event exists
    StructuredBuffer oGetDCRequest;
    oGetDCRequest.PutString("ParentGuid", strRootGuid);
    oGetDCRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
    oGetDCRequest.PutStructuredBuffer("Filters", c_oRequest.GetStructuredBuffer("Filters"));
    StructuredBuffer oBranchEvent(this->GetListOfEvents(oGetDCRequest));
    StructuredBuffer oListOfEvents(oBranchEvent.GetStructuredBuffer("ListOfEvents"));

    Dword dwStatus = 404;
    if (0 < oListOfEvents.GetNamesOfElements().size())
    {
        oResponse.PutString("DCEventGuid", oListOfEvents.GetNamesOfElements()[0]);
        dwStatus = 200;
    }
    else 
    {
        oResponse.PutString("RootEventGuid", strRootGuid);
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
        bsoncxx::document::element strEventGuid = oAuditLogDocument->view()["EventGuid"];
        if (strEventGuid && strEventGuid.type() == type::k_utf8)
        {
            // Add root event to the response structured buffer 
            oResponse.PutString("RootEventGuid", strEventGuid.get_utf8().value.to_string().c_str());
        }
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
            // Get values from the document and generate the event log StructuredBuffer
            StructuredBuffer oEvent;
            bsoncxx::document::element strPlainTextObjectBlobGuid = oDocumentView["PlainTextObjectBlobGuid"];
            bsoncxx::document::element strEventGuid = oDocumentView["EventGuid"];
            bsoncxx::document::element strOrganizationGuid = oDocumentView["OrganizationGuid"];
            bsoncxx::document::element fIsLeaf = oDocumentView["IsLeaf"];

            // Get the PlainTextObjectBlob and then get the Object blob
            // Apply filters if any
            // If the object is not filtered out then add the audit log information to the Event structured buffer
            bool fAddToListOfEvents = false;
            if (strPlainTextObjectBlobGuid && strPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                Guid oPlainTextObjectBlobGuid(strPlainTextObjectBlobGuid.get_utf8().value.to_string().c_str());
                // Fetch events from the PlainTextObjectBlob collection associated with the event guid
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
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
                        bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces) << finalize);
                        if (bsoncxx::stdx::nullopt != oObjectDocument)
                        {
                            bsoncxx::document::element stlObjectBlob = oObjectDocument->view()["ObjectBlob"];
                            if (stlObjectBlob && stlObjectBlob.type() == type::k_binary)
                            {
                                // Apply supplied filters on audit logs
                                StructuredBuffer oObject(stlObjectBlob.get_binary().bytes, stlObjectBlob.get_binary().size);
                                std::vector<std::string> stlFilters = oFilters.GetNamesOfElements();
                                try 
                                {
                                    for (std::string stlFilter : stlFilters)
                                    {
                                        if ("MinimumDate" == stlFilter)
                                        {
                                            uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                            uint64_t unFilterMinimumDate = oFilters.GetUnsignedInt64("MinimumDate");
                                            _ThrowBaseExceptionIf((unObjectTimestamp < unFilterMinimumDate), "Object timestamp is less than the specified minimum date.", nullptr);
                                        }
                                        else if ("MaximumDate" == stlFilter)
                                        {
                                            uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                            uint64_t unFilterMaximumDate = oFilters.GetUnsignedInt64("MaximumDate");
                                            _ThrowBaseExceptionIf((unObjectTimestamp > unFilterMaximumDate), "Object timestamp is greater than the specified maximum date.", nullptr);
                                        }
                                        else if ("TypeOfEvents" == stlFilter)
                                        {
                                            Qword qwObjectEventType = oObject.GetQword("EventType");
                                            Qword qwFilterEventType = oFilters.GetQword("TypeOfEvents");
                                            _ThrowBaseExceptionIf((qwObjectEventType != qwFilterEventType), "Object type is not the same as the specified event type.", nullptr);
                                        }
                                        else if ("DCGuid" == stlFilter)
                                        {
                                            Word wType = oEvent.GetGuid("EventGuid").GetObjectType();
                                            _ThrowBaseExceptionIf((eAuditEventBranchNode != wType), "No DC guid exists for this type of object.", nullptr);
                                            _ThrowBaseExceptionIf((0 == oObject.GetUnsignedInt64("SequenceNumber")), "No DC guid exists for root node.", nullptr);
                                            StructuredBuffer oPlainTextMetadata(oObject.GetStructuredBuffer("PlainTextEventData"));
                                            std::string strPlainObjectDCGuid = oPlainTextMetadata.GetString("DCGuid");
                                            std::string strFilterDcGuid = oFilters.GetString("DCGuid");
                                            _ThrowBaseExceptionIf((1 != oPlainTextMetadata.GetDword("TypeOfBranch")), "The audit log is not for a digital contract", nullptr);
                                            _ThrowBaseExceptionIf((strPlainObjectDCGuid != strFilterDcGuid), "The DC guid does not match the requested dc guid", nullptr);
                                        }
                                        // TODO: Add VMGuid filters
                                        // else if ("VMGuid" == stlFilter)
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
                                    oEvent.PutBuffer("ObjectBlob", oObject.GetSerializedBuffer());
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
                    if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
                    {
                        oEvent.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
                    }
                    if (fIsLeaf && fIsLeaf.type() == type::k_bool)
                    {
                        oEvent.PutBoolean("isLeaf", fIsLeaf.get_bool().value);
                    }
                    // Add event to the response structured buffer that contains list of audit events 
                    oListOfEvents.PutStructuredBuffer(strEventGuid.get_utf8().value.to_string().c_str(), oEvent);
                }
            }
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

    Guid oObjectGuid, oPlainTextObjectBlobGuid;

    StructuredBuffer oNonLeafEvent(c_oRequest.GetStructuredBuffer("NonLeafEvent"));
    // Create an audit log event document
    bsoncxx::document::value oEventDocumentValue = bsoncxx::builder::stream::document{}
      << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
      << "OrganizationGuid" << oNonLeafEvent.GetString("OrganizationGuid")
      << "ParentGuid" << oNonLeafEvent.GetString("ParentGuid")
      << "EventGuid" << oNonLeafEvent.GetString("EventGuid")
      << "IsLeaf" << false
      << finalize;

    // Create an object document
    StructuredBuffer oObject;
    oObject.PutString("EventGuid", oNonLeafEvent.GetString("EventGuid"));
    oObject.PutString("ParentGuid", oNonLeafEvent.GetString("ParentGuid"));
    oObject.PutString("OrganizationGuid", oNonLeafEvent.GetString("OrganizationGuid"));
    oObject.PutQword("EventType", oNonLeafEvent.GetQword("EventType"));
    oObject.PutUnsignedInt64("Timestamp", oNonLeafEvent.GetUnsignedInt64("Timestamp"));
    oObject.PutUnsignedInt32("SequenceNumber", oNonLeafEvent.GetUnsignedInt32("SequenceNumber"));
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
        auto oResult = oAuditLogCollection.insert_one(oEventDocumentValue.view());
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Access Object collection
            mongocxx::collection oObjectCollection = oSailDatabase["Object"];
            // Insert document in the Object collection
            oResult = oObjectCollection.insert_one(oObjectDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access PlainTextObjectBlob collection
                mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                // Insert document in the PlainTextObjectBlob collection
                oResult = oPlainTextObjectCollection.insert_one(oPlainTextObjectDocumentValue.view());
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
        auto oResult = oAuditLogCollection.insert_many(stlEventDocuments);
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Access Object collection
            mongocxx::collection oObjectCollection = oSailDatabase["Object"];
            // Insert object documents in the Object collection
            oResult = oObjectCollection.insert_many(stlObjectDocuments);
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access PlainTextObjectBlob collection
                mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                // Insert plain text object blob documents in the PlainTextObjectBlob collection
                oResult = oPlainTextObjectCollection.insert_many(stlPlainTextObjectDocuments);
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
