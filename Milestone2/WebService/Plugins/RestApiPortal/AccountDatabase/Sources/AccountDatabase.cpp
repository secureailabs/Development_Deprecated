/*********************************************************************************************
 *
 * @file AccountDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 17 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "AccountDatabase.h"

static AccountDatabase * gs_oAccountDatabase = nullptr;

/********************************************************************************************
 *
 * @function GetAccountDatabase
 * @brief Create a singleton object of AccountDatabase class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of AccountDatabase class
 *
 ********************************************************************************************/

AccountDatabase * __stdcall GetAccountDatabase(void)
{
    __DebugFunction();

    if (nullptr == gs_oAccountDatabase)
    {
        gs_oAccountDatabase = new AccountDatabase();
        _ThrowOutOfMemoryExceptionIfNull(gs_oAccountDatabase);
    }

    return gs_oAccountDatabase;
}

/********************************************************************************************
 *
 * @function ShutdownAccountDatabase
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownAccountDatabase(void)
{
    __DebugFunction();

    if (nullptr != gs_oAccountDatabase)
    {
        gs_oAccountDatabase->Release();
        gs_oAccountDatabase = nullptr;
    }
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function AccountDatabase
 * @brief Constructor
 *
 ********************************************************************************************/

AccountDatabase::AccountDatabase(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function AccountDatabase
 * @brief Copy Constructor
 * @param[in] c_oAccountDatabase Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

AccountDatabase::AccountDatabase(
    _in const AccountDatabase & c_oAccountDatabase
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function ~AccountDatabase
 * @brief Destructor
 *
 ********************************************************************************************/

AccountDatabase::~AccountDatabase(void)
{
    __DebugFunction();

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }

}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AccountDatabase::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "AccountDatabase";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AccountDatabase::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{D9A1028F-D3ED-4B48-BE13-F0F236C14970}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall AccountDatabase::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall AccountDatabase::InitializePlugin(void)
{
    __DebugFunction();

    // Takes in an EOSB and sends user metadata
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/Account/GetUserInfo");

    // Takes in a StructuredBuffer containing new user information and create a new user account
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AccountManager/Users");

}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall AccountDatabase::SubmitRequest(
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
    if ("GET" == strVerb)
    {
        if ("/SAIL/Account/GetUserInfo" == strResource)
        {
            stlResponseBuffer = this->GetUserInfo(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/AccountManager/Users" == strResource)
        {
            stlResponseBuffer = this->RegisterUser(c_oRequestStructuredBuffer);
        }
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
 * @class AccountDatabase
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall AccountDatabase::GetResponse(
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
 * @class AccountDatabase
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::GetUserInfo(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch user account record from the database

    StructuredBuffer oUserMetadata;

    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");

    bool fFound = false;
    UserAccount * oUserAccount;

    // Find the requested user account
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; ((fFound == false ) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            oUserAccount = m_stlUserAccounts[unIndex];
            fFound = true;
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    _ThrowBaseExceptionIf((false == fFound), "Error: User not found", nullptr);

    // Generate a StructuredBuffer containing requested user's metadata
    oUserMetadata.PutString("Username", oUserAccount->GetUserName());
    oUserMetadata.PutString("Fullname", oUserAccount->GetFullName());
    oUserMetadata.PutString("Email", oUserAccount->GetEmail());
    oUserMetadata.PutString("Organization", oUserAccount->GetOrganization());
    oUserMetadata.PutString("ContactInformation", oUserAccount->GetContactInformation());
    oUserMetadata.PutQword("LastLogonTime", oUserAccount->GetLastLogon());
    oUserMetadata.PutWord("UserType", oUserAccount->GetTypeOfUser());

    return oUserMetadata.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function RegisterUser
 * @brief Add a user account to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user uuid
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::RegisterUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Insert record in the database

    StructuredBuffer oResponse;

    std::string strUserName = c_oRequest.GetString("Username");
    std::string strFullName = c_oRequest.GetString("Fullname");
    std::string strEmail = c_oRequest.GetString("Email");
    std::string strOrganization = c_oRequest.GetString("Organization");
    std::string strContactInformation = c_oRequest.GetString("ContactInformation");
    Qword qwLastLogon = c_oRequest.GetQword("LastLogonTime");
    Word wTypeOfUser = c_oRequest.GetWord("UserType");

    // Generate a UUID for the new user
    Guid oUserGuid;
    std::string strUserUuid = oUserGuid.ToString(eHyphensAndCurlyBraces);

    // Create a new user account
    UserAccount * oUserAccount = new UserAccount(strUserUuid, strUserName, strFullName, strEmail, strOrganization, strContactInformation, qwLastLogon, wTypeOfUser);

    // Add user account to the m_stlUserAccounts
    ::pthread_mutex_lock(&m_sMutex);
    m_stlUserAccounts.push_back(oUserAccount);
    ::pthread_mutex_unlock(&m_sMutex);

    oResponse.PutString("UserUuid", strUserUuid);

    return oResponse.GetSerializedBuffer();
}
