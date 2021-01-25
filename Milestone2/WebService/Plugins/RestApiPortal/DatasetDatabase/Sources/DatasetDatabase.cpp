/*********************************************************************************************
 *
 * @file DatasetDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 18 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatasetDatabase.h"

static DatasetDatabase * gs_oDatasetDatabase = nullptr;

/********************************************************************************************
 *
 * @function GetDatasetDatabase
 * @brief Create a singleton object of DatasetDatabase class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of DatasetDatabase class
 *
 ********************************************************************************************/

DatasetDatabase * __stdcall GetDatasetDatabase(void)
{
    __DebugFunction();

    if (nullptr == gs_oDatasetDatabase)
    {
        gs_oDatasetDatabase = new DatasetDatabase();
        _ThrowOutOfMemoryExceptionIfNull(gs_oDatasetDatabase);
    }

    return gs_oDatasetDatabase;
}

/********************************************************************************************
 *
 * @function ShutdownDatasetDatabase
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownDatasetDatabase(void)
{
    __DebugFunction();

    if (nullptr != gs_oDatasetDatabase)
    {
        gs_oDatasetDatabase->Release();
        gs_oDatasetDatabase = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function DatasetDatabase
 * @brief Constructor
 *
 ********************************************************************************************/

DatasetDatabase::DatasetDatabase(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

    this->InitializeUserAccounts();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function DatasetDatabase
 * @brief Copy Constructor
 * @param[in] c_oDatasetDatabase Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

DatasetDatabase::DatasetDatabase(
    _in const DatasetDatabase & c_oDatasetDatabase
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function ~DatasetDatabase
 * @brief Destructor
 *
 ********************************************************************************************/

DatasetDatabase::~DatasetDatabase(void)
{
    __DebugFunction();

    for (Dataset * oDataset : m_stlDatasets)
    {
        delete oDataset;
    }

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }

}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatasetDatabase::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "DatasetDatabase";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatasetDatabase::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{B39B82EB-ABC6-4C29-887D-0E954D03307D}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall DatasetDatabase::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function InitializeUserAccounts
 * @brief Insert user data
 *
 ********************************************************************************************/

void __thiscall DatasetDatabase::InitializeUserAccounts(void)
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
 * @class DatasetDatabase
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall DatasetDatabase::InitializePlugin(void)
{
    __DebugFunction();

    // Takes in an EOSB and sends back all datasets information within the organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/Dataset/GetSubmittedDataset");

    // Sends back list of all available datasets
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/Dataset/GetAvailableDataset");

    // Adds a dataset record in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatasetManager");

    // Deletes a dataset record in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatasetManager/DeleteDataset");

}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall DatasetDatabase::SubmitRequest(
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
        if ("/SAIL/Dataset/GetSubmittedDataset" == strResource)
        {
            stlResponseBuffer = this->GetListOfSubmittedDatasets(c_oRequestStructuredBuffer);
        }

        else if ("/SAIL/Dataset/GetAvailableDataset" == strResource)
        {
            stlResponseBuffer = this->GetListOfAvailableDatasets(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/DatasetManager" == strResource)
        {
            stlResponseBuffer = this->RegisterDataset(c_oRequestStructuredBuffer);
        }

        else if ("/SAIL/DatasetManager/DeleteDataset" == strResource)
        {
            stlResponseBuffer = this->DeleteDataset(c_oRequestStructuredBuffer);
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
 * @class DatasetDatabase
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall DatasetDatabase::GetResponse(
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
 * @class DatasetDatabase
 * @function GetListOfSubmittedDatasets
 * @brief Send back a full list of all datasets submitted by any of the Dataset Admins within the organizaion
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of submitted datasets
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::GetListOfSubmittedDatasets(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch dataset records from the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser and Organization associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full EOSB of DatasetAdmin
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to get list of submitted datasets.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin"
    unsigned int unUserIndex = -1;
    for (unsigned int unIndex = 0; ((-1 == unUserIndex) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())
            {
                unUserIndex = unIndex;
            }
            else
            {
                _ThrowBaseException("Error: User is not authorized for this transaction", nullptr);
            }
        }
    }
    _ThrowBaseExceptionIf((-1 == unUserIndex), "Error: User not found", nullptr);

    // Generate a StructuredBuffer containing all datasets submitted by users within the organization
    Dword wStatus = 404;
    Dataset * oDataset;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; unIndex < m_stlDatasets.size(); ++unIndex)
    {
        oDataset = m_stlDatasets[unIndex];
        // Add all datasets whose organization is the same as the requesting user's organization to the StructuredBuffer
        if (oDataset->GetDatasetOrganization() == m_stlUserAccounts[unUserIndex]->GetOrganization())
        {
            StructuredBuffer oDatasetMetadata;
            oDatasetMetadata.PutString("DatasetName", oDataset->GetDatasetName());
            oDatasetMetadata.PutString("UserUuid", oDataset->GetDatasetSubmittedBy());
            oDatasetMetadata.PutQword("DatasetSubmissionDate", oDataset->GetDatasetSubmissionDate());
            oResponse.PutStructuredBuffer(oDataset->GetDatasetUuid().c_str(), oDatasetMetadata);

            wStatus = 200;
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back transaction
    oResponse.PutWord("Status", wStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function GetListOfAvailableDatasets
 * @brief Send back a list of all available datasets
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing list of all available datasets
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::GetListOfAvailableDatasets(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch dataset records from the database

    StructuredBuffer oResponse;

    // Take in full EOSB of the caller
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to list available datasets.", nullptr);

    // Generate a StructuredBuffer containing all available datasets metadata
    Dword wStatus = 404;
    Dataset * oDataset;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; unIndex < m_stlDatasets.size(); ++unIndex)
    {
        oDataset = m_stlDatasets[unIndex];
        StructuredBuffer oDatasetMetadata;
        oDatasetMetadata.PutString("DatasetName", oDataset->GetDatasetName());
        oDatasetMetadata.PutString("DatasetOrganization", oDataset->GetDatasetOrganization());
        oDatasetMetadata.PutQword("DatasetSubmissionDate", oDataset->GetDatasetSubmissionDate());
        oResponse.PutStructuredBuffer(oDataset->GetDatasetUuid().c_str(), oDatasetMetadata);

        wStatus = 200;
    }
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back transaction
    oResponse.PutWord("Status", wStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function RegisterDataset
 * @brief Take in full EOSB of a data owner and add the dataset record to the database
 * @param[in] c_oRequest contains EOSB of the data owner and the dataset information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::RegisterDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: save dataset record in the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full EOSB of data owner
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to register a dataset.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())
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

    // Get the new dataset information
    std::string strDatasetName = c_oRequest.GetString("DatasetName");
    std::string strDatasetOrganization = c_oRequest.GetString("DatasetOrganization");
    Qword qwSubmissionDate = c_oRequest.GetQword("DatasetSubmissionDate");

    // Generate a UUID for the new dataset
    Guid oDatasetGuid;
    std::string strDatasetUuid = oDatasetGuid.ToString(eHyphensAndCurlyBraces);

    // Create a new dataset record
    Dataset * oDataset = new Dataset(strDatasetUuid, strDatasetName, strDatasetOrganization, strUserUuid, qwSubmissionDate);

    // Add new dataset record to the vector container
    ::pthread_mutex_lock(&m_sMutex);
    m_stlDatasets.push_back(oDataset);
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back status and database uuid
    oResponse.PutWord("Status", 200);
    oResponse.PutString("DatasetUuid", strDatasetUuid);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function DeleteDataset
 * @brief Take in full EOSB of a data owner and delete the dataset record from the database
 * @param[in] c_oRequest contains EOSB of the data owner and the dataset UUID
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::DeleteDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch dataset records from the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full EOSB of data owner
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to delete a dataset.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())
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

    // Get the dataset UUID
    std::string strDatasetUuid = c_oRequest.GetString("DatasetUuid");
    // Find the dataset and Verify that the user is the dataset owner
    Word wStatus = 404;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; ((wStatus == 404) && (unIndex < m_stlDatasets.size())); ++unIndex)
    {
        if ((strDatasetUuid == m_stlDatasets[unIndex]->GetDatasetUuid()) && (strUserUuid == m_stlDatasets[unIndex]->GetDatasetSubmittedBy()))
        {
            m_stlDatasets.erase(m_stlDatasets.begin() + unIndex);
            wStatus = 200;
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back status of the transaction
    oResponse.PutWord("Status", wStatus);

    return oResponse.GetSerializedBuffer();
}
