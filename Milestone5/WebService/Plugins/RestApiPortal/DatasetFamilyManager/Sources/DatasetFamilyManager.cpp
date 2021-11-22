/*********************************************************************************************
 *
 * @file DatasetFamilyManager.cpp
 * @author David Gascon
 * @date 28 Oct 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatasetFamilyManager.h"
#include "64BitHashes.h"
#include "IpcTransactionHelperFunctions.h"
#include "RequestHelpers.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"

static DatasetFamilyManager * gs_oDatasetFamilyManager = nullptr;

/*******************************************************************************
 * File constants
 ******************************************************************************/

/********************************************************************************************
 *
 * @function GetDatasetFamilyManager
 * @brief Create a singleton object of DatasetFamilyManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of DatasetFamilyManager class
 *
 ********************************************************************************************/
DatasetFamilyManager * __stdcall GetDatasetFamilyManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oDatasetFamilyManager)
    {
        gs_oDatasetFamilyManager = new DatasetFamilyManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oDatasetFamilyManager);
    }

    return gs_oDatasetFamilyManager;
}

/********************************************************************************************
 *
 * @function ShutdownDatasetFamilyManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/
void __stdcall ShutdownDatasetFamilyManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oDatasetFamilyManager)
    {
        gs_oDatasetFamilyManager->TerminateSignalEncountered();
        gs_oDatasetFamilyManager->Release();
        gs_oDatasetFamilyManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function DatasetFamilyManager
 * @brief Constructor
 *
 ********************************************************************************************/

DatasetFamilyManager::DatasetFamilyManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function DatasetFamilyManager
 * @brief Copy Constructor
 * @param[in] c_oDatasetFamilyManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

DatasetFamilyManager::DatasetFamilyManager(
    _in const DatasetFamilyManager & c_oDatasetFamilyManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function ~DatasetFamilyManager
 * @brief Destructor
 *
 ********************************************************************************************/

DatasetFamilyManager::~DatasetFamilyManager(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("DatasetFamilyManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatasetFamilyManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "DatasetFamilyManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatasetFamilyManager::GetUuid(void) const throw()
{
    __DebugFunction();

    // Genereated with uuidgen
    static const char * sc_szUuid = "{5EC02023-93C4-4F98-912A-B9AFB3ABE6FD}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall DatasetFamilyManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatasetFamilyManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall DatasetFamilyManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall DatasetFamilyManager::InitializePlugin(const StructuredBuffer& oInitializationVectors)
{
    __DebugFunction();

    StructuredBuffer oRegisterLeafEvents;
    StructuredBuffer oTestValueParameter;
    oTestValueParameter.PutByte("ElementType", UINT64_VALUE_TYPE);
    oTestValueParameter.PutBoolean("IsRequired", true);

    // Add required parameters for listing events
    StructuredBuffer oPostValueMultiply;
    oPostValueMultiply.PutStructuredBuffer("NumberToMultiply", oTestValueParameter);

    StructuredBuffer oEmptyStructuredBuffer;

    StructuredBuffer oRequiredStringElement;
    oRequiredStringElement.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oRequiredStringElement.PutBoolean("IsRequired", true);

    StructuredBuffer oOptionalStringElement;
    oOptionalStringElement.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oOptionalStringElement.PutBoolean("IsRequired", false);

    StructuredBuffer oRequriedBooleanElement;
    oRequriedBooleanElement.PutByte("ElementType", BOOLEAN_VALUE_TYPE);
    oRequriedBooleanElement.PutBoolean("IsRequired", true);

    StructuredBuffer oRequriedBufferElement;
    oRequriedBufferElement.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oRequriedBufferElement.PutBoolean("IsRequired", true);

    StructuredBuffer oDatasetFamilyStructuredBuffer;
    oDatasetFamilyStructuredBuffer.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oDatasetFamilyStructuredBuffer.PutBoolean("IsRequired", true);

    StructuredBuffer oCreateNewDatasetFamilyParameters;
    oCreateNewDatasetFamilyParameters.PutStructuredBuffer("Eosb", oRequriedBufferElement);
    oCreateNewDatasetFamilyParameters.PutStructuredBuffer("DatasetFamilyDescription",oRequiredStringElement);
    oCreateNewDatasetFamilyParameters.PutStructuredBuffer("DatasetFamilyTags",oOptionalStringElement);
    oCreateNewDatasetFamilyParameters.PutStructuredBuffer("DatasetFamilyTitle", oRequiredStringElement);

    StructuredBuffer oListDatasetFamilyParameters;
    oListDatasetFamilyParameters.PutStructuredBuffer("Eosb", oRequriedBufferElement);

    StructuredBuffer oPullDatasetFamilyParameters;
    oPullDatasetFamilyParameters.PutStructuredBuffer("Eosb", oRequriedBufferElement);
    oPullDatasetFamilyParameters.PutStructuredBuffer("DatasetFamilyGuid", oRequiredStringElement);

    StructuredBuffer oUpdateDatasetFamilyParameters;
    oUpdateDatasetFamilyParameters.PutStructuredBuffer("Eosb", oRequriedBufferElement);
    oUpdateDatasetFamilyParameters.PutStructuredBuffer("DatasetFamily", oDatasetFamilyStructuredBuffer);

    StructuredBuffer oDeleteDatasetFamilyParameters;
    oDeleteDatasetFamilyParameters.PutStructuredBuffer("Eosb", oRequriedBufferElement);
    oDeleteDatasetFamilyParameters.PutStructuredBuffer("DatasetFamilyGuid", oRequiredStringElement);

    m_oDictionary.AddDictionaryEntry("POST","/SAIL/DatasetFamilyManager/RegisterDatasetFamily",oCreateNewDatasetFamilyParameters, 1);

    m_oDictionary.AddDictionaryEntry("PUT","/SAIL/DatasetFamilyManager/UpdateDatasetFamily",oUpdateDatasetFamilyParameters, 1);

    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatasetFamilyManager/ListDatasetFamilies", oListDatasetFamilyParameters, 1);
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatasetFamilyManager/PullDatasetFamily", oPullDatasetFamilyParameters, 1);

    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatasetFamilyManager/DeleteDatasetFamily", oDeleteDatasetFamilyParameters, 1);

    m_strDatabaseServiceIpAddr = oInitializationVectors.GetString("DatabaseServerIp");
    m_unDatabaseServiceIpPort = oInitializationVectors.GetUnsignedInt32("DatabaseServerPort");

}


/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function RegisterDatasetFamily
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequest the structued buffer that contains the request's information
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns std::vector<Byte> stores the serialized response
 *
 ********************************************************************************************/
// Register dataset family
std::vector<Byte> __thiscall DatasetFamilyManager::RegisterDatasetFamily(
    _in const StructuredBuffer & c_oRequest
    )
{

    __DebugFunction();

    StructuredBuffer oResponse;
    TlsNode* poTlsNode{nullptr};
    Dword dwStatus;

    try
    {
        StructuredBuffer oUserInfo = ::GetUserInfoFromEosb(c_oRequest);
        std::string strNewGuid = Guid(eDatasetFamily).ToString(eHyphensAndCurlyBraces);
        Guid oDatasetFamilyOwner = oUserInfo.GetGuid("OrganizationGuid");

        std::string strDatasetFamilyOwner = oDatasetFamilyOwner.ToString(eHyphensAndCurlyBraces);
        std::vector<Byte> stlResponseBuffer;


        // Create a request to add dataset metadata to the database
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "POST");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterDatasetFamily");

        StructuredBuffer oDatasetFamily;
        oDatasetFamily.PutBoolean("DatasetFamilyActive", true);
        oDatasetFamily.PutString("DatasetFamilyDescription", c_oRequest.GetString("DatasetFamilyDescription"));
        oDatasetFamily.PutString("DatasetFamilyGuid", strNewGuid);
        oDatasetFamily.PutString("DatasetFamilyOwnerGuid", strDatasetFamilyOwner);

        if ( true == c_oRequest.IsElementPresent("DatasetFamilyTags", ANSI_CHARACTER_STRING_VALUE_TYPE) )
        {
            oDatasetFamily.PutString("DatasetFamilyTags", c_oRequest.GetString("DatasetFamilyTags"));
        }

        oDatasetFamily.PutString("DatasetFamilyTitle", c_oRequest.GetString("DatasetFamilyTitle"));
        oDatasetFamily.PutString("VersionNumber", "0x00000001");

        oRequest.PutStructuredBuffer("DatasetFamily",oDatasetFamily);

        std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oRequest);

        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
        // Send request packet
        poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

        std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
        _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
        unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
        std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);

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
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    if ( nullptr != poTlsNode )
    {
        poTlsNode->Release();
        poTlsNode = nullptr;
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function ListDatasetFamilies
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequest the structued buffer that contains the request's information
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns std::vector<Byte> stores the serialized response
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatasetFamilyManager::ListDatasetFamilies(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;
    TlsNode* poTlsNode{nullptr};
    Dword dwStatus = 404;

    try
    {
        std::vector<Byte> stlUserInfo = ::GetUserInfoFromEosb(c_oRequest);
        StructuredBuffer oUserInfo(stlUserInfo);
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Create a request to get the database manager to query for the organization GUID
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/DatasetFamilies");

            std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oRequest);
            poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);

            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
            std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;

            StructuredBuffer oDatabaseResponse(stlResponse);
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("DatasetFamilies", oDatabaseResponse.GetStructuredBuffer("DatasetFamilies"));
                dwStatus = 200;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function PullDatasetFamily
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequest the structued buffer that contains the request's information
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns std::vector<Byte> stores the serialized response
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatasetFamilyManager::PullDatasetFamily(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;
    TlsNode* poTlsNode{nullptr};
    Dword dwStatus = 404;

    try
    {
        std::vector<Byte> stlUserInfo = ::GetUserInfoFromEosb(c_oRequest);
        StructuredBuffer oUserInfo(stlUserInfo);
        if (200 == oUserInfo.GetDword("Status"))
        {
            std::string strDatasetFamilyGuid = c_oRequest.GetString("DatasetFamilyGuid");

            // Create a request to get the database manager to query for the organization GUID
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullDatatsetFamily");
            oRequest.PutString("DatasetFamilyGuid", strDatasetFamilyGuid);

            std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oRequest);
            poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);

            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
            std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;

            StructuredBuffer oDatabaseResponse(stlResponse);
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("DatasetFamily", oDatabaseResponse.GetStructuredBuffer("DatasetFamily"));
                dwStatus = 200;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function EditDatasetFamilyInformation
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequest the structued buffer that contains the request's information
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns std::vector<Byte> stores the serialized response
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatasetFamilyManager::EditDatasetFamilyInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    __DebugAssert( true == c_oRequest.IsElementPresent("DatasetFamily", INDEXED_BUFFER_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("DatasetFamilyActive", BOOLEAN_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("DatasetFamilyDescription", ANSI_CHARACTER_STRING_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("DatasetFamilyGuid", ANSI_CHARACTER_STRING_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("DatasetFamilyOwnerGuid", ANSI_CHARACTER_STRING_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("DatasetFamilyTitle", ANSI_CHARACTER_STRING_VALUE_TYPE) );
    __DebugAssert( true == c_oRequest.GetStructuredBuffer("DatasetFamily").IsElementPresent("VersionNumber", ANSI_CHARACTER_STRING_VALUE_TYPE) );

    StructuredBuffer oResponse;
    TlsNode* poTlsNode{nullptr};
    Dword dwStatus{204};

    try
    {
        StructuredBuffer oUserInfo = ::GetUserInfoFromEosb(c_oRequest);

        if ( 200 == oUserInfo.GetDword("Status")  )
        {
            Guid oUserOrganization = oUserInfo.GetGuid("OrganizationGuid");
            std::string strUserOrganizationGuid = oUserOrganization.ToString(eHyphensAndCurlyBraces);
            if ( strUserOrganizationGuid == c_oRequest.GetStructuredBuffer("DatasetFamily").GetString("DatasetFamilyOwnerGuid"))
            {
                // Create a request to get the database manager to query for the organization GUID
                StructuredBuffer oDatabaseRequest;
                oDatabaseRequest.PutString("PluginName", "DatabaseManager");
                oDatabaseRequest.PutString("Verb", "PUT");
                oDatabaseRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateDatasetFamily");
                oDatabaseRequest.PutStructuredBuffer("DatasetFamily", c_oRequest.GetStructuredBuffer("DatasetFamily"));

                std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oDatabaseRequest);
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);

                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;

                StructuredBuffer oDatabaseResponse(stlResponse);
                dwStatus = oDatabaseResponse.GetDword("Status");
                if (200 == dwStatus)
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                }
            }
            else
            {
                // The logged in user doesn't belong to the organization that owns this
                // datset family
                dwStatus = 404;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}


/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function DeleteDatasetFamily
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatasetFamilyManager::DeleteDatasetFamily(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;
    TlsNode* poTlsNode{nullptr};
    Dword dwStatus = 404;

    try
    {
        StructuredBuffer oUserInfo = ::GetUserInfoFromEosb(c_oRequest);

        if ( 200 == oUserInfo.GetDword("Status")  )
        {
            // TODO - When we have the associating of dataset families and datsets check
            // if any dataset is using this family
            bool fDatasetFamilyInUse{false};

            Guid oUserOrganization = oUserInfo.GetGuid("OrganizationGuid");
            std::string strUserOrganizationGuid = oUserOrganization.ToString(eHyphensAndCurlyBraces);

            if ( !fDatasetFamilyInUse )
            {
                // Create a request to get the database manager to query for the organization GUID
                StructuredBuffer oDatabaseRequest;
                oDatabaseRequest.PutString("PluginName", "DatabaseManager");
                oDatabaseRequest.PutString("Verb", "DELETE");
                oDatabaseRequest.PutString("Resource", "/SAIL/DatabaseManager/DeleteDatasetFamily");
                oDatabaseRequest.PutString("DatasetFamilyGuid", c_oRequest.GetString("DatasetFamilyGuid"));
                oDatabaseRequest.PutString("OrganizationGuid", strUserOrganizationGuid);

                std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oDatabaseRequest);
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);

                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;

                StructuredBuffer oDatabaseResponse(stlResponse);
                dwStatus = oDatabaseResponse.GetDword("Status");
            }
            else
            {
                std::vector<Byte> stlDatasetFamily = this->PullDatasetFamily(c_oRequest);
                StructuredBuffer oPullResponse(stlDatasetFamily);
                StructuredBuffer oDatasetFamily = oPullResponse.GetStructuredBuffer("DatasetFamily");
                oDatasetFamily.PutBoolean("DatasetFamilyActive",false);

                StructuredBuffer oEditDatasetFamilyRequest;
                oEditDatasetFamilyRequest.PutStructuredBuffer("DatasetFamily", oDatasetFamily);

                StructuredBuffer oDatabaseRequest;
                oDatabaseRequest.PutString("PluginName", "DatabaseManager");
                oDatabaseRequest.PutString("Verb", "PUT");
                oDatabaseRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateDatasetFamily");
                oDatabaseRequest.PutStructuredBuffer("DatasetFamily", oDatasetFamily);

                std::vector<Byte> stlRequest = ::CreateRequestPacketFromStructuredBuffer(oDatabaseRequest);
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);

                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *)stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;

                StructuredBuffer oDatabaseResponse(stlResponse);
                dwStatus = oDatabaseResponse.GetDword("Status");
            }
            if ( 200 == dwStatus )
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
/********************************************************************************************
 *
 * @class DatasetFamilyManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall DatasetFamilyManager::SubmitRequest(
    _in const StructuredBuffer & c_oRequestStructuredBuffer,
    _out unsigned int * punSerializedResponseSizeInBytes
    )
{
    __DebugFunction();

    __DebugAssert(punSerializedResponseSizeInBytes != nullptr);

    uint64_t un64Identifier = 0xFFFFFFFFFFFFFFFF;
    std::string strVerb = c_oRequestStructuredBuffer.GetString("Verb");
    std::string strResource = c_oRequestStructuredBuffer.GetString("Resource");
    // TODO: As an optimization, we should make sure to convert strings into 64 bit hashes
    // in order to speed up comparison. String comparisons WAY expensive.
    std::vector<Byte> stlResponseBuffer;

    TlsNode * poTlsNode{nullptr};
    StructuredBuffer responseBuffer;

    if ( "GET" == strVerb)
    {
        if ("/SAIL/DatasetFamilyManager/ListDatasetFamilies" == strResource)
        {
            stlResponseBuffer = ListDatasetFamilies(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DatasetFamilyManager/PullDatasetFamily" == strResource)
        {
            stlResponseBuffer = PullDatasetFamily(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/DatasetFamilyManager/RegisterDatasetFamily" == strResource)
        {
            stlResponseBuffer = RegisterDatasetFamily(c_oRequestStructuredBuffer);
        }
    }
    else if ("PUT" == strVerb)
    {
        if ("/SAIL/DatasetFamilyManager/UpdateDatasetFamily" == strResource)
        {
            stlResponseBuffer = EditDatasetFamilyInformation(c_oRequestStructuredBuffer);
        }
    }
    else if ("DELETE" == strVerb)
    {
        if ("/SAIL/DatasetFamilyManager/DeleteDatasetFamily" == strResource)
        {
            stlResponseBuffer = DeleteDatasetFamily(c_oRequestStructuredBuffer);
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
 * @class DatasetFamilyManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall DatasetFamilyManager::GetResponse(
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

