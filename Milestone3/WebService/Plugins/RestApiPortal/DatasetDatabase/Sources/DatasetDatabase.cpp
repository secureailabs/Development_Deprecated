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
#include "IpcTransactionHelperFunctions.h"
#include "SocketClient.h"
#include "TlsClient.h"

static DatasetDatabase * gs_oDatasetDatabase = nullptr;

/********************************************************************************************
 *
 * @function CreateRequestPacket
 * @brief Create a Tls request packet to send to the database portal
 * @param[in] c_oRequest StructuredBuffer containing the request parameters
 * @return Serialized request packet
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall CreateRequestPacket(
    _in const StructuredBuffer & c_oRequest
    )
{
    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(uint32_t) + c_oRequest.GetSerializedBufferRawDataSizeInBytes() + sizeof(Dword);

    std::vector<Byte> stlSerializedBuffer(unSerializedBufferSizeInBytes);
    Byte * pbSerializedBuffer = (Byte *) stlSerializedBuffer.data();

    // The format of the request data is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x436f6e74                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfRestRequestStructuredBuffer                                |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfRestRequestStructuredBuffer] RestRequestStructuredBuffer             |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x656e6420                                                                 |
    // +------------------------------------------------------------------------------------+

    *((Dword *) pbSerializedBuffer) = 0x436f6e74;
    pbSerializedBuffer += sizeof(Dword);
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) c_oRequest.GetSerializedBufferRawDataSizeInBytes();
    pbSerializedBuffer += sizeof(uint32_t);
    ::memcpy((void *) pbSerializedBuffer, (const void *) c_oRequest.GetSerializedBufferRawDataPtr(), c_oRequest.GetSerializedBufferRawDataSizeInBytes());
    pbSerializedBuffer += c_oRequest.GetSerializedBufferRawDataSizeInBytes();
    *((Dword *) pbSerializedBuffer) = 0x656e6420;

    return stlSerializedBuffer;
}

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
        gs_oDatasetDatabase->TerminateSignalEncountered();
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
    m_fTerminationSignalEncountered = false;
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
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall DatasetDatabase::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
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

    // Add parameters for RegisterDataset resource
    StructuredBuffer oRegisterDataset;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterDataset.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oDatasetGuid;
    oDatasetGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDatasetGuid.PutBoolean("IsRequired", true);
    oRegisterDataset.PutStructuredBuffer("DatasetGuid", oDatasetGuid);
    StructuredBuffer oDatasetMetadata;
    oDatasetMetadata.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oDatasetMetadata.PutBoolean("IsRequired", true);
    StructuredBuffer oVersionNumber;
    oVersionNumber.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVersionNumber.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("VersionNumber", oVersionNumber);
    StructuredBuffer oDatasetName;
    oDatasetName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDatasetName.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("DatasetName", oDatasetName);
    StructuredBuffer oDescription;
    oDescription.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDescription.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("Description", oDescription);
    StructuredBuffer oKeywords;
    oKeywords.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oKeywords.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("Keywords", oKeywords);
    StructuredBuffer oPublishDate;
    oPublishDate.PutByte("ElementType", UINT64_VALUE_TYPE);
    oPublishDate.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("PublishDate", oPublishDate);
    StructuredBuffer oPrivacyLevel;
    oPrivacyLevel.PutByte("ElementType", BYTE_VALUE_TYPE);
    oPrivacyLevel.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("PrivacyLevel", oPrivacyLevel);
    StructuredBuffer oLimitations;
    oLimitations.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oLimitations.PutBoolean("IsRequired", true);
    oDatasetMetadata.PutStructuredBuffer("JurisdictionalLimitations", oLimitations);
    oRegisterDataset.PutStructuredBuffer("DatasetData", oDatasetMetadata);

    // Add parameters for ListDatasets resource
    StructuredBuffer oListDatasets;
    oListDatasets.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for PullDataset resource
    StructuredBuffer oPullDataset;
    oPullDataset.PutStructuredBuffer("Eosb", oEosb);
    oPullDataset.PutStructuredBuffer("DatasetGuid", oDatasetGuid);

    // Add parameters for DeleteDataset
    StructuredBuffer oDeleteDataset;
    oDeleteDataset.PutStructuredBuffer("Eosb", oEosb);
    oDeleteDataset.PutStructuredBuffer("DatasetGuid", oDatasetGuid);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Stores dataset metadata in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatasetManager/RegisterDataset", oRegisterDataset, 1);

    // Sends back list of all available datasets
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatasetManager/ListDatasets", oListDatasets, 1);

    // Send back metadata of the dataset associated with the requested DatasetGuid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatasetManager/PullDataset", oPullDataset, 1);

    // Deletes a dataset record from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatasetManager/DeleteDataset", oDeleteDataset, 1);
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
        if ("/SAIL/DatasetManager/ListDatasets" == strResource)
        {
            stlResponseBuffer = this->GetListOfAvailableDatasets(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DatasetManager/PullDataset" == strResource)
        {
            stlResponseBuffer = this->PullDataset(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/DatasetManager/RegisterDataset" == strResource)
        {
            stlResponseBuffer = this->RegisterDataset(c_oRequestStructuredBuffer);
        }
    }
    else if ("DELETE" == strVerb)
    {
        if ("/SAIL/DatasetManager/DeleteDataset" == strResource)
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
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::GetUserInfo(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    Socket * poIpcCryptographicManager = nullptr;

    try
    {
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");

        StructuredBuffer oDecryptEosbRequest;
        oDecryptEosbRequest.PutDword("TransactionType", 0x00000007);
        oDecryptEosbRequest.PutBuffer("Eosb", stlEosb);

        // Call CryptographicManager plugin to get the decrypted eosb
        poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
        StructuredBuffer oDecryptedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest, false));
        poIpcCryptographicManager->Release();
        poIpcCryptographicManager = nullptr;
        if ((0 < oDecryptedEosb.GetSerializedBufferRawDataSizeInBytes())&&(201 == oDecryptedEosb.GetDword("Status")))
        {
            StructuredBuffer oEosb(oDecryptedEosb.GetStructuredBuffer("UserInformation").GetStructuredBuffer("Eosb"));
            // Send back the updated Eosb
            oResponse.PutBuffer("Eosb", oDecryptedEosb.GetBuffer("UpdatedEosb"));
            // Send back the user information
            oResponse.PutGuid("UserGuid", oEosb.GetGuid("UserId"));
            oResponse.PutGuid("OrganizationGuid", oEosb.GetGuid("OrganizationGuid"));
            // TODO: get user access rights from the confidential record, for now it can't be decrypted
            oResponse.PutQword("AccessRights", oEosb.GetQword("UserAccessRights"));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poIpcCryptographicManager)
    {
        poIpcCryptographicManager->Release();
    }

    // Add status code for the transaction
    oResponse.PutDword("Status", dwStatus);

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

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to list of all datasets
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListDatasets");
            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), stlRequest.size());

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;

            StructuredBuffer oDatabaseResponse(stlResponse);
            if (200 == oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("Datasets", oDatabaseResponse.GetStructuredBuffer("Datasets"));
                dwStatus = 200;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetDatabase
 * @function PullDataset
 * @brief Get metadata of the dataset associated with the GUID
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing metatdata of the dataset
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetDatabase::PullDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get metadata of the dataset
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullDataset");
            oRequest.PutString("DatasetGuid", c_oRequest.GetString("DatasetGuid"));
            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), stlRequest.size());

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;

            StructuredBuffer oDatabaseResponse(stlResponse);
            if (200 == oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("Dataset", oDatabaseResponse.GetStructuredBuffer("Dataset"));
                dwStatus = 200;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

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

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        StructuredBuffer oUserInfo = this->GetUserInfo(c_oRequest);
        if (200 == oUserInfo.GetDword("Status"))
        {
            // TODO: Verify that the user is a "DatasetAdmin"
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to add dataset metadata to the database
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "POST");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterDataset");
            oRequest.PutString("DatasetGuid", c_oRequest.GetString("DatasetGuid"));
            oRequest.PutString("DataOwnerGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
            oRequest.PutStructuredBuffer("DatasetData", c_oRequest.GetStructuredBuffer("DatasetData"));
            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;
            
            // Check if DatabaseManager registered the dataset or not
            StructuredBuffer oDatabaseResponse(stlResponse);
            if (204 != oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status and database uuid
    oResponse.PutDword("Status", dwStatus);

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

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Verify that the user is a "DatasetAdmin"
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eDatasetAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to delete the dataset
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "DELETE");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/DeleteDataset");
                oRequest.PutString("DatasetGuid", c_oRequest.GetString("DatasetGuid"));
                oRequest.PutString("DataOwnerGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), stlRequest.size());

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;

                StructuredBuffer oDatabaseResponse(stlResponse);
                if (200 == oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
