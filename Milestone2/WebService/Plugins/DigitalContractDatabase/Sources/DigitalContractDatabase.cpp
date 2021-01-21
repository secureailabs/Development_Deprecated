/*********************************************************************************************
 *
 * @file DigitalContractDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 23 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DigitalContractDatabase.h"

static DigitalContractDatabase * gs_oDigitalContractDatabase = nullptr;

/********************************************************************************************
 *
 * @function GetDigitalContractDatabase
 * @brief Create a singleton object of DigitalContractDatabase class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of DigitalContractDatabase class
 *
 ********************************************************************************************/

DigitalContractDatabase * __stdcall GetDigitalContractDatabase(void)
{
    __DebugFunction();

    if (nullptr == gs_oDigitalContractDatabase)
    {
        gs_oDigitalContractDatabase = new DigitalContractDatabase();
        _ThrowOutOfMemoryExceptionIfNull(gs_oDigitalContractDatabase);
    }

    return gs_oDigitalContractDatabase;
}

/********************************************************************************************
 *
 * @function ShutdownDigitalContractDatabase
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownDigitalContractDatabase(void)
{
    __DebugFunction();

    if (nullptr != gs_oDigitalContractDatabase)
    {
        gs_oDigitalContractDatabase->Release();
        gs_oDigitalContractDatabase = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function DigitalContractDatabase
 * @brief Constructor
 *
 ********************************************************************************************/

DigitalContractDatabase::DigitalContractDatabase(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

    this->InitializeUserAccounts();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function DigitalContractDatabase
 * @brief Copy Constructor
 * @param[in] c_oDigitalContractDatabase Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

DigitalContractDatabase::DigitalContractDatabase(
    _in const DigitalContractDatabase & c_oDigitalContractDatabase
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ~DigitalContractDatabase
 * @brief Destructor
 *
 ********************************************************************************************/

DigitalContractDatabase::~DigitalContractDatabase(void)
{
    __DebugFunction();

    for (DigitalContract * oDigitalContract : m_stlDigitalContracts)
    {
        delete oDigitalContract;
    }

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }

}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DigitalContractDatabase::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "DigitalContractDatabase";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DigitalContractDatabase::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{F238BA12-D94F-40BD-9F78-196ECD065326}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall DigitalContractDatabase::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function InitializeUserAccounts
 * @brief Insert user data
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::InitializeUserAccounts(void)
{
    __DebugFunction();

    m_stlUserAccounts.push_back(new UserAccount("{FEB1CAE7-0F10-4185-A1F2-DE71B85DBD25}", "johnsnow", "John Snow", "jsnow@example.com", "HBO", "1234567890", 999, 0x7));
    m_stlUserAccounts.push_back(new UserAccount("{C1F45EF0-AB47-4799-9407-CA8A40CAC159}", "aryastark", "Arya Stark", "astark@example.com", "HBO", "1234567890", 888, 0x2));
    m_stlUserAccounts.push_back(new UserAccount("{0A83BCF5-2845-4437-AEBE-E02DFB349BAB}", "belle", "Belle", "belle@example.com", "Walt Disney", "1234567890", 777, 0x1));
    m_stlUserAccounts.push_back(new UserAccount("{64E4FAC3-63C9-4844-BF82-1581F9C750CE}", "gaston", "Gaston", "gaston@example.com", "Walt Disney", "1234567890", 666, 0x6));
    m_stlUserAccounts.push_back(new UserAccount("{F732CA9C-217E-4E3D-BF25-E2425B480556}", "hermoinegranger", "Hermoine Granger", "hgranger@example.com", "Universal Studios", "1234567890", 555, 0x5));
    m_stlUserAccounts.push_back(new UserAccount("{F3FBE722-1A42-4052-8815-0ABDDB3F2841}", "harrypotter", "Harry Potter", "hpotter@example.com", "Universal Studios", "1234567890", 444, 0x4));
    m_stlUserAccounts.push_back(new UserAccount("{2B9C3814-79D4-456B-B64A-ED79F69373D3}", "antman", "Ant man", "antman@example.com", "Marvel Cinematic Universe", "1234567890", 333, 0x7));
    m_stlUserAccounts.push_back(new UserAccount("{B40E1F9C-C100-46B3-BD7F-C80EB1351794}", "spiderman", "Spider man", "spiderman@example.com", "Marvel Cinematic Universe", "1234567890", 222, 0x6));
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::InitializePlugin(void)
{
    __DebugFunction();

    // Takes in an EOSB and sends back all digital contracts associated with the users organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DigitalContract/MyDigitalContracts");

    // Sends back list of digital contracts in flux
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DigitalContract/WaitingDigitalContracts");

    // Adds a digital contract record in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DigitalContract/RegisterDigitalContract");
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall DigitalContractDatabase::SubmitRequest(
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
        if ("/SAIL/DigitalContract/MyDigitalContracts" == strResource)
        {
            stlResponseBuffer = this->GetListOfMyDigitalContracts(c_oRequestStructuredBuffer);
        }

        else if ("/SAIL/DigitalContract/WaitingDigitalContracts" == strResource)
        {
            stlResponseBuffer = this->GetListOfWaitingDigitalContracts(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/DigitalContract/RegisterDigitalContract" == strResource)
        {
            stlResponseBuffer = this->RegisterDigitalContract(c_oRequestStructuredBuffer);
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
 * @class DigitalContractDatabase
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall DigitalContractDatabase::GetResponse(
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
 * @class DigitalContractDatabase
 * @function GetListOfMyDigitalContracts
 * @brief Send back a full list of all digital contracts associated with the users organization
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of digital contracts associated with the users organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetListOfMyDigitalContracts(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch digital contract records from the database
    // TODO: Replace call to abstract class UserAccount::GetOrganization() with call to AccountDatabase plugin
    //       and get Organization associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full EOSB of user
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to get list of digital contracts.", nullptr);

    // Find user
    unsigned int unUserIndex = -1;
    for (unsigned int unIndex = 0; ((-1 == unUserIndex) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            unUserIndex = unIndex;
        }
    }
    _ThrowBaseExceptionIf((-1 == unUserIndex), "Error: User not found", nullptr);

    // Generate a StructuredBuffer containing all digital contracts associated with the users organization
    Dword wStatus = 404;
    DigitalContract * oDigitalContract;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; unIndex < m_stlDigitalContracts.size(); ++unIndex)
    {
        oDigitalContract = m_stlDigitalContracts[unIndex];
        // Add all digital contracts whose organization is the same as the requesting user's organization to the StructuredBuffer
        if ((oDigitalContract->GetDataOwnerOrganization() == m_stlUserAccounts[unUserIndex]->GetOrganization())||(oDigitalContract->GetResearcherOrganization() == m_stlUserAccounts[unUserIndex]->GetOrganization()))
        {
            StructuredBuffer oDigitalContractMetadata;
            oDigitalContractMetadata.PutString("DataOwnerOrganization", oDigitalContract->GetDataOwnerOrganization());
            oDigitalContractMetadata.PutString("ResearcherOrganization", oDigitalContract->GetResearcherOrganization());
            oDigitalContractMetadata.PutString("InvolvedDatasets", oDigitalContract->GetInvolvedDatasets());
            oDigitalContractMetadata.PutQword("CreationDate", oDigitalContract->GetCreationDate());
            oDigitalContractMetadata.PutQword("ExpirationDate", oDigitalContract->GetExpirationDate());
            // Add metadata of all digital contracts that associated with the users organization to response StructuredBuffer
            oResponse.PutStructuredBuffer(oDigitalContract->GetDigitalContractUuid().c_str(), oDigitalContractMetadata);

            wStatus = 200;
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    // Add transaction status
    oResponse.PutWord("Status", wStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetListOfWaitingDigitalContracts
 * @brief Send back a list of all digital contracts that are in flux
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing list of all digital contracts that are in flux
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetListOfWaitingDigitalContracts(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch digital contract records from the database

    StructuredBuffer oResponse;

    // Take in full EOSB of the caller
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to list waiting digital contracts.", nullptr);

    // Find user
    unsigned int unUserIndex = -1;
    for (unsigned int unIndex = 0; ((-1 == unUserIndex) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            unUserIndex = unIndex;
        }
    }
    _ThrowBaseExceptionIf((-1 == unUserIndex), "Error: User not found", nullptr);

    // Generate a StructuredBuffer containing metadata of all digital contracts that are in flux
    Dword wStatus = 404;
    DigitalContract * oDigitalContract;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; unIndex < m_stlDigitalContracts.size(); ++unIndex)
    {
        oDigitalContract = m_stlDigitalContracts[unIndex];
        // Add all digital contracts whose organization is the same as the requesting user's organization to the StructuredBuffer
        if ((oDigitalContract->GetDataOwnerOrganization() == m_stlUserAccounts[unUserIndex]->GetOrganization())||(oDigitalContract->GetResearcherOrganization() == m_stlUserAccounts[unUserIndex]->GetOrganization()))
        {
            if (false == oDigitalContract->IsDigitalContractApproved())
            {
                StructuredBuffer oDigitalContractMetadata;
                oDigitalContractMetadata.PutString("DataOwnerOrganization", oDigitalContract->GetDataOwnerOrganization());
                oDigitalContractMetadata.PutString("ResearcherOrganization", oDigitalContract->GetResearcherOrganization());
                oDigitalContractMetadata.PutString("InvolvedDatasets", oDigitalContract->GetInvolvedDatasets());
                oDigitalContractMetadata.PutQword("CreationDate", oDigitalContract->GetCreationDate());
                oDigitalContractMetadata.PutQword("ExpirationDate", oDigitalContract->GetExpirationDate());
                // Add metadata of all digital contracts that associated with the users organization to response StructuredBuffer
                oResponse.PutStructuredBuffer(oDigitalContract->GetDigitalContractUuid().c_str(), oDigitalContractMetadata);

                wStatus = 200;
            }
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    // Add transaction status
    oResponse.PutWord("Status", wStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function RegisterDigitalContract
 * @brief Take in full EOSB of a dataset admin or researcher and register the digital contract
 * @param[in] c_oRequest contains EOSB of the data owner and the digital contract information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::RegisterDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: save digital contract record in the database
    // TODO: Replace call to abstract class UserAccount function calls with call to AccountDatabase plugin
    //       and get UserAccount associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full EOSB of data owner
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to register a digital contract.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin" or "Researcher"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if ((true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())||(true == m_stlUserAccounts[unIndex]->IsResearcher()))
            {
                fFound = true;
            }
            else
            {
                _ThrowBaseException("Error: User is not authorized for this transaction", nullptr);
            }
        }
    }

    _ThrowBaseExceptionIf((false == fFound), "Error: User not found", nullptr);

    // Get the new digital contract information
    std::string strDataOwnerOrganization = c_oRequest.GetString("DataOwnerOrganization");
    std::string strResearcherOrganization = c_oRequest.GetString("ResearcherOrganization");
    std::string strInvolvedDatasets = c_oRequest.GetString("InvolvedDatasets");
    Qword qwCreationDate = c_oRequest.GetQword("CreationDate");
    Qword qwExpirationDate = c_oRequest.GetQword("ExpirationDate");

    // Generate a UUID for the new digital contract
    Guid oDigitalContractGuid;
    std::string strDigitalContractUuid = oDigitalContractGuid.ToString(eHyphensAndCurlyBraces);

    // Create a new digital contract record
    DigitalContract * oDigitalContract = new DigitalContract(strDigitalContractUuid, strDataOwnerOrganization, strResearcherOrganization, strInvolvedDatasets, qwCreationDate, qwExpirationDate);

    // Add new digital contract record to the vector container
    ::pthread_mutex_lock(&m_sMutex);
    m_stlDigitalContracts.push_back(oDigitalContract);
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back status and digital contract uuid
    oResponse.PutWord("Status", 200);
    oResponse.PutString("DigitalContractUuid", strDigitalContractUuid);

    return oResponse.GetSerializedBuffer();
}
