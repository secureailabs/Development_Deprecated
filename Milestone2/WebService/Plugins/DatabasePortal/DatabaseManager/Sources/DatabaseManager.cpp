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

#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
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

    delete m_poMongoClient;

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
    m_poMongoClient = new mongocxx::client   // Use mongocxx client to connect to MongoDB instance
    {
        mongocxx::uri{"mongodb://localhost:27017"}
    };

    // Get basic user record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/BasicUser");

    // Get Confidential User Record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ConfidentialUser");

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

    // Access SailDatabase
    ::pthread_mutex_lock(&m_sMutex);
    mongocxx::database oSailDatabase = (*m_poMongoClient)["SailDatabase"];
    // Access BasicUser collection
    mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
    // Fetch basic user record associated with the qw64BitHash
    bool fFound = false;
    bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUser = oBasicUserCollection.find_one(document{} << "64BitHash" << (double)qw64BitHash << finalize);
    ::pthread_mutex_unlock(&m_sMutex);
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
            oBasicUserRecord.PutDword("AccountStatus", (Dword) (double)dwAccountStatus.get_double());
        }
        if (stlAccountKeyWrappedWithPDK && stlAccountKeyWrappedWithPDK.type() == type::k_binary)
        {
            oBasicUserRecord.PutBuffer("WrappedAccountKey", stlAccountKeyWrappedWithPDK.raw(), stlAccountKeyWrappedWithPDK.length());
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

    // Access SailDatabase
    ::pthread_mutex_lock(&m_sMutex);
    mongocxx::database oSailDatabase = (*m_poMongoClient)["SailDatabase"];
    // Access ConfidentialOrganizationOrUser collection
    mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
    // Fetch confidential user record associated with the strUserUuid
    bool fFound = false;
    bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialUser = oConfidentialUserCollection.find_one(document{} << "OrganizationOrUserUuid" << strUserUuid << finalize);
    ::pthread_mutex_unlock(&m_sMutex);
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

        if (stlIv && stlIv.type() == type::k_binary)
        {
            oConfidentialUserRecord.PutBuffer("IV", stlIv.raw(), stlIv.length());
        }
        if (stlTag && stlTag.type() == type::k_binary)
        {
            oConfidentialUserRecord.PutBuffer("Tag", stlTag.raw(), stlTag.length());
        }
        if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
        {
            oConfidentialUserRecord.PutBuffer("EncryptedSsb", stlEncryptedSsb.raw(), stlEncryptedSsb.length());
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
