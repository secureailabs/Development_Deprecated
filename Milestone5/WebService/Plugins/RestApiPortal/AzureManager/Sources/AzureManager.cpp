/*********************************************************************************************
 *
 * @file AzureManager.cpp
 * @author Shabana Akhtar Baig
 * @date 21 Jul 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "AzureManager.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"
#include "AzureHelper.h"

#include <thread>

static AzureManager * gs_oAzureManager = nullptr;

static SmartMemoryAllocator gs_oMemoryAllocator;

/********************************************************************************************
 *
 * @struct IpcServerParameters
 * @brief Struct used to pass in parameters to StartServerThread()
 *
 ********************************************************************************************/

typedef struct
{
    ThreadManager * poThreadManager;        /* Pointer to thread manager object */
    SocketServer * poIpcServer;          /* Pointer to socket server instance */
}
IpcServerParameters;

/********************************************************************************************
 *
 * @function StartIpcServerThread
 * @brief Starts up ipc server thread
 * @param[in] poVoidThreadParameter void pointer to IpcServerParameters instance
 * @return A null pointer
 *
 ********************************************************************************************/

static void * __stdcall StartIpcServerThread(
    _in void * poVoidThreadParameter
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poVoidThreadParameter);

    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) poVoidThreadParameter;
    __DebugAssert(nullptr != poIpcServerParameters->poThreadManager);
    __DebugAssert(nullptr != poIpcServerParameters->poIpcServer);

    try
    {
        AzureManager * poAzureManager = ::GetAzureManager();
        poAzureManager->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
    }
    
    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    poIpcServerParameters->poIpcServer->Release();
    gs_oMemoryAllocator.Deallocate(poVoidThreadParameter);

    return nullptr;
}

/********************************************************************************************
 *
 * @function StartIpcThread
 * @brief Starts up a connection thread
 * @param[in] poVoidThreadParameter void pointer to socket instance
 * @return A null pointer
 *
 ********************************************************************************************/

static void * __stdcall StartIpcThread(
    _in void * poVoidThreadParameter
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poVoidThreadParameter);
    
    Socket * poIpcSocket = (Socket *) poVoidThreadParameter;

    try
    {
        AzureManager * poAzureManager = ::GetAzureManager();
        poAzureManager->HandleIpcRequest(poIpcSocket);
    }
    
    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    poIpcSocket->Release();

    ::pthread_exit(nullptr);
}

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
 * @function GetAzureManager
 * @brief Create a singleton object of AzureManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of AzureManager class
 *
 ********************************************************************************************/

AzureManager * __stdcall GetAzureManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oAzureManager)
    {
        gs_oAzureManager = new AzureManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oAzureManager);
    }

    return gs_oAzureManager;
}

/********************************************************************************************
 *
 * @function ShutdownAzureManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownAzureManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oAzureManager)
    {
        gs_oAzureManager->TerminateSignalEncountered();
        gs_oAzureManager->Release();
        gs_oAzureManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function AzureManager
 * @brief Constructor
 *
 ********************************************************************************************/

AzureManager::AzureManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function AzureManager
 * @brief Copy Constructor
 * @param[in] c_oAzureManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

AzureManager::AzureManager(
    _in const AzureManager & c_oAzureManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function ~AzureManager
 * @brief Destructor
 *
 ********************************************************************************************/

AzureManager::~AzureManager(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("AzureManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AzureManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "AzureManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AzureManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{CE450136-BCDC-439A-9024-01940F0DA951}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall AzureManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall AzureManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall AzureManager::InitializePlugin(
    _in const StructuredBuffer& oInitializationVectors
    )
{
    __DebugFunction();

    // Add parameters for RegisterTemplate resource
    StructuredBuffer oRegisterTemplate;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterTemplate.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oTemplateData;
    oTemplateData.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oTemplateData.PutBoolean("IsRequired", true);
    StructuredBuffer oSubscriptionId;
    oSubscriptionId.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSubscriptionId.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("SubscriptionID", oSubscriptionId);
    StructuredBuffer oSecret;
    oSecret.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSecret.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("Secret", oSecret);
    StructuredBuffer oTenantId;
    oTenantId.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oTenantId.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("TenantID", oTenantId);
    StructuredBuffer oApplicationId;
    oApplicationId.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oApplicationId.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("ApplicationID", oApplicationId);
    StructuredBuffer oResourceGroup;
    oResourceGroup.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oResourceGroup.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("ResourceGroup", oResourceGroup);
    StructuredBuffer oVirtualNetwork;
    oVirtualNetwork.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVirtualNetwork.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("VirtualNetwork", oVirtualNetwork);
    StructuredBuffer oNetworkSecurityGroup;
    oNetworkSecurityGroup.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oNetworkSecurityGroup.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("NetworkSecurityGroup", oNetworkSecurityGroup);
    StructuredBuffer oVirtualMachineImageId;
    oVirtualMachineImageId.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVirtualMachineImageId.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("VirtualMachineImage", oVirtualMachineImageId);
    StructuredBuffer oHostRegion;
    oHostRegion.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oHostRegion.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("HostRegion", oHostRegion);
    StructuredBuffer oName;
    oName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oName.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("Name", oName);
    StructuredBuffer oDescription;
    oDescription.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDescription.PutBoolean("IsRequired", true);
    oTemplateData.PutStructuredBuffer("Description", oDescription);
    oRegisterTemplate.PutStructuredBuffer("TemplateData", oTemplateData);

    // Add parameters for ListTemplates resource
    StructuredBuffer oListTemplates;
    oListTemplates.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for PullTemplate resource
    StructuredBuffer oPullTemplate;
    oPullTemplate.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oTemplateGuid;
    oTemplateGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oTemplateGuid.PutBoolean("IsRequired", true);
    oPullTemplate.PutStructuredBuffer("TemplateGuid", oTemplateGuid);

    // Add parameters for UpdateTemplate
    StructuredBuffer oUpdateTemplate;
    oUpdateTemplate.PutStructuredBuffer("Eosb", oEosb);
    oUpdateTemplate.PutStructuredBuffer("TemplateGuid", oTemplateGuid);
    // Make "Secret" an optional field in UpdateTemplate API
    oSecret.PutBoolean("IsRequired", false);
    oTemplateData.PutStructuredBuffer("Secret", oSecret);
    oUpdateTemplate.PutStructuredBuffer("TemplateData", oTemplateData);

    // Add parameters for DeleteTemplate
    StructuredBuffer oDeleteTemplate;
    oDeleteTemplate.PutStructuredBuffer("Eosb", oEosb);
    oDeleteTemplate.PutStructuredBuffer("TemplateGuid", oTemplateGuid);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Stores azure settings template in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AzureManager/RegisterTemplate", oRegisterTemplate, 1);

    // Sends back list of all available azure settings templates
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AzureManager/ListTemplates", oListTemplates, 1);

    // Send back azure settings template associated with the requested TemplateGuid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AzureManager/PullTemplate", oPullTemplate, 1);

    // Updates an azure setting template in the database
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/AzureManager/UpdateTemplate", oUpdateTemplate, 1);

    // Deletes an azure setting template from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/AzureManager/DeleteTemplate", oDeleteTemplate, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{4B56D0E0-7A38-40C1-839A-B9BBCDDFE521}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("AzureManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);

    // Store our database service IP information
    m_strDatabaseServiceIpAddr = oInitializationVectors.GetString("DatabaseServerIp");
    m_unDatabaseServiceIpPort = oInitializationVectors.GetUnsignedInt32("DatabaseServerPort");
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall AzureManager::RunIpcServer(
    _in SocketServer * poIpcServer,
    _in ThreadManager * poThreadManager
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poIpcServer);

    while (false == m_fTerminationSignalEncountered)
    {
        // Wait for connection
        if (true == poIpcServer->WaitForConnection(1000))
        {
            Socket * poSocket = poIpcServer->Accept();
            if (nullptr != poSocket)
            {
                pthread_t connectionThread;
                int nStatus = ::pthread_create(&connectionThread, nullptr, StartIpcThread, (void *) poSocket);
                _ThrowBaseExceptionIf((0 != nStatus), "Error creating a thread with nStatus: %d.", nStatus);
                // Detach the thread as it terminates on its own by calling pthread_exit
                // Detaching the thread will make sure that the system recycles its underlying resources automatically
                ::pthread_detach(connectionThread);
            }
        }
    }
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall AzureManager::HandleIpcRequest(
    _in Socket * poSocket
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    std::vector<Byte> stlResponse;

    StructuredBuffer oRequestParameters(::GetIpcTransaction(poSocket, false));

    Dword dwTransactionType = oRequestParameters.GetDword("TransactionType");

    switch (dwTransactionType)
    {
        case 0x00000001 // PullAzureSettingsTemplate
        :
            stlResponse = this->PullAzureSettingsTemplate(oRequestParameters);
            break;
    }

    // Send back the response
    if ((0 < stlResponse.size())&&(false == ::PutIpcTransaction(poSocket, stlResponse)))
    {
        _ThrowBaseException("Error: Sending back Ipc response filed", nullptr);
    }
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall AzureManager::SubmitRequest(
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
        if ("/SAIL/AzureManager/ListTemplates" == strResource)
        {
            stlResponseBuffer = this->GetListOfAzureSettingsTemplates(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AzureManager/PullTemplate" == strResource)
        {
            stlResponseBuffer = this->PullAzureSettingsTemplate(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/AzureManager/RegisterTemplate" == strResource)
        {
            stlResponseBuffer = this->RegisterAzureSettingsTemplate(c_oRequestStructuredBuffer);
        }
    }
    else if ("PUT" == strVerb)
    {
        if ("/SAIL/AzureManager/UpdateTemplate" == strResource)
        {
            stlResponseBuffer = this->UpdateAzureSettingsTemplate(c_oRequestStructuredBuffer);
        }
    }
    else if ("DELETE" == strVerb)
    {
        if ("/SAIL/AzureManager/DeleteTemplate" == strResource)
        {
            stlResponseBuffer = this->DeleteAzureSettingsTemplate(c_oRequestStructuredBuffer);
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
 * @class AzureManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall AzureManager::GetResponse(
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
 * @class AzureManager
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::GetUserInfo(
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
 * @class AzureManager
 * @function GetListOfAzureSettingsTemplates
 * @brief Send back a list of all available azure settings templates of an organization
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing list of all available azure settings templates of an organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::GetListOfAzureSettingsTemplates(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to list of all available azure settings templates of an organization
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListAzureTemplates");
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
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
                    oResponse.PutStructuredBuffer("Templates", oDatabaseResponse.GetStructuredBuffer("Templates"));
                    dwStatus = 200;
                }
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function PullAzureSettingsTemplate
 * @brief Get azure settings template associated with the GUID
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing azure settings template associated with the GUID
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::PullAzureSettingsTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to get azure settings template
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullAzureTemplate");
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutString("TemplateGuid", c_oRequest.GetString("TemplateGuid"));
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
                    oResponse.PutStructuredBuffer("Template", oDatabaseResponse.GetStructuredBuffer("Template"));
                    dwStatus = 200;
                }
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function RegisterAzureSettingsTemplate
 * @brief Take in full EOSB and register azure settings template
 * @param[in] c_oRequest contains EOSB and the azure settings template information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::RegisterAzureSettingsTemplate(
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
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Create a template guid
                std::string strTemplateGuid = Guid(eAzureTemplate).ToString(eHyphensAndCurlyBraces);

                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to add azure settings template to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "POST");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterAzureTemplate");
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                StructuredBuffer oTemplateData = c_oRequest.GetStructuredBuffer("TemplateData");
                oTemplateData.PutDword("State", (Dword)AzureTemplateState::eInitializing);
                oTemplateData.PutString("Note", "-");
                oRequest.PutStructuredBuffer("TemplateData", oTemplateData);
                oRequest.PutString("TemplateGuid", strTemplateGuid);
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

                // Create a thread to check and create the VirtualNetwork and
                // NetworkSecurity for this template
                std::thread(&AzureManager::UpdateAzureTemplateResources, this, oTemplateData, strTemplateGuid, oUserInfo.GetBuffer("Eosb")).detach();

                // Check if DatabaseManager registered the template or not
                StructuredBuffer oDatabaseResponse(stlResponse);
                if (204 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 201;
                }
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

    // Send back status and database uuid
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function UpdateAzureSettingsTemplate
 * @brief Take in full EOSB and update azure settings template in the database
 * @param[in] c_oRequest contains EOSB and the template information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::UpdateAzureSettingsTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to update the azure settings template
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateAzureTemplate");
                oRequest.PutString("TemplateGuid", c_oRequest.GetString("TemplateGuid"));
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                StructuredBuffer oUpdatedAzureTemplate = c_oRequest.GetStructuredBuffer("TemplateData");
                oUpdatedAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eInitializing);
                oUpdatedAzureTemplate.PutString("Note", "-");
                oRequest.PutStructuredBuffer("TemplateData", oUpdatedAzureTemplate);
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

                // Create a thread to check and create the VirtualNetwork and
                // NetworkSecurity for this template
                std::thread(&AzureManager::UpdateAzureTemplateResources, this, oUpdatedAzureTemplate, c_oRequest.GetString("TemplateGuid"), oUserInfo.GetBuffer("Eosb")).detach();

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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function DeleteAzureSettingsTemplate
 * @brief Take in full EOSB of an admin and delete the template from the database
 * @param[in] c_oRequest contains EOSB of the admin and the template UUID
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::DeleteAzureSettingsTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Verify that the user is an Admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to delete the template
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "DELETE");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/DeleteAzureTemplate");
                oRequest.PutString("TemplateGuid", c_oRequest.GetString("TemplateGuid"));
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}


/********************************************************************************************
 *
 * @class AzureManager
 * @function UpdateAzureTemplateResources
 * @brief Creates a new NetworkSecurityGroup and VirtualNetwork if it does not exist
 * @param[in] c_oRequest contains EOSB of the admin and the template UUID
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

void __thiscall AzureManager::UpdateAzureTemplateResources(
    _in StructuredBuffer oAzureTemplate,
    _in const std::string c_strTemplateGuid,
    _in const std::vector<Byte> c_stlEosb
    )
{
    __DebugFunction();

    bool fInternalError = true;
    std::string strInternalError = "";

    try
    {
        std::string strSubscriptionIdentifier = oAzureTemplate.GetString("SubscriptionID");
        std::string strSecret = oAzureTemplate.GetString("Secret");
        std::string strTenantIdentifier = oAzureTemplate.GetString("TenantID");
        std::string strApplicationIdentifier = oAzureTemplate.GetString("ApplicationID");
        std::string strResourceGroup = oAzureTemplate.GetString("ResourceGroup");
        std::string strVirtualNetwork = oAzureTemplate.GetString("VirtualNetwork");
        std::string strNetworksecurityGroupName = oAzureTemplate.GetString("NetworkSecurityGroup");
        std::string strVirtualMachineImageName = oAzureTemplate.GetString("VirtualMachineImage");
        std::string strLocation = oAzureTemplate.GetString("HostRegion");

        StructuredBuffer oUpdateStatus;
        oUpdateStatus.PutString("TemplateGuid", c_strTemplateGuid);
        oUpdateStatus.PutBuffer("Eosb", c_stlEosb);

        // Attempt a login to azure and verify the credentials
        std::string strAzureAccessToken = ::LoginToMicrosoftAzureApiPortal(strApplicationIdentifier, strSecret, strTenantIdentifier);
        if (0 == strAzureAccessToken.length())
        {
            // Change the state to invalid credentials and throw exception
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eInvalidCredentials);
            oAzureTemplate.PutString("Note", "Kindly check the credentials!!");
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
            oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));
            fInternalError = false;
            _ThrowBaseException("Invalid Azure credentials", nullptr);
        }

        // Check if the Virtul Machine ImageId is a valid Azure Resource
        std::string strVirtualMachineImageId = ::CreateAzureResourceId(strSubscriptionIdentifier, strResourceGroup, "providers/Microsoft.Compute", "images", strVirtualMachineImageName);
        bool fDoesImageExist = ::DoesAzureResourceExist(strAzureAccessToken, strVirtualMachineImageId);
        if (false == fDoesImageExist)
        {
            // Update the state to creating Virutal machine image
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eCreatingImage);
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
            oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));

            StructuredBuffer oCopyImageResponse = ::CopyVirtualMachineImage(strAzureAccessToken, strSubscriptionIdentifier, strResourceGroup, strLocation, strVirtualMachineImageName);
            if ("Success" != oCopyImageResponse.GetString("Status"))
            {
                // Update the state if the creation failed
                oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eFailedCreatingImage);
                oAzureTemplate.PutString("Note", "Image Fail: "+oCopyImageResponse.GetString("error"));
                oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
                StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
                oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));
                fInternalError = false;
                _ThrowBaseException("Failed to copy image", nullptr);
            }
        }

        // Check if the Virtual Network Specified exists in the resourceGroup
        std::string strVirtualNetworkId = ::CreateAzureResourceId(strSubscriptionIdentifier, strResourceGroup, "providers/Microsoft.Network", "virtualNetworks", strVirtualNetwork);
        bool fVirtualNetworkExists = ::DoesAzureResourceExist(strAzureAccessToken, strVirtualNetworkId);
        if (false == fVirtualNetworkExists)
        {
            // If it does not exist update the state to CreatingVirtualNetwork
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eCreatingVirtualNetwork);
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
            oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));

            // Create a Virutal Network accordingly
            StructuredBuffer oVirtualNetworkCreateParameter;
            oVirtualNetworkCreateParameter.PutString("VirtualNetworkName", strVirtualNetwork);
            std::string strDeploymentParameters = ::CreateAzureParamterJson("https://confidentialvmdeployment.blob.core.windows.net/deployemnttemplate/VirtualNetwork.json?sp=r&st=2021-08-18T11:49:19Z&se=2022-05-31T19:49:19Z&spr=https&sv=2020-08-04&sr=b&sig=UepJBsssk48ON0SKPRo8G1IOc%2F4dysKVOjjQ%2B59iNxA%3D", oVirtualNetworkCreateParameter);
            StructuredBuffer oDeploymentResult = ::CreateAzureDeployment(strAzureAccessToken, strDeploymentParameters, strSubscriptionIdentifier, strResourceGroup, strLocation);
            if ("Success" != oDeploymentResult.GetString("Status"))
            {
                // Update the state if the creation failed
                oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eFailedCreatingVirtualNetwork);
                oAzureTemplate.PutString("Note", "Virtual Network Fail: "+oDeploymentResult.GetString("error"));
                oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
                StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
                oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));
                fInternalError = false;
                _ThrowBaseException("Failed to create Virtual Network", nullptr);
            }
            else
            {
                fVirtualNetworkExists = true;
            }
        }

        // Check if the Network Security Group exists
        std::string strNetworkSecurityGroupId = ::CreateAzureResourceId(strSubscriptionIdentifier, strResourceGroup, "providers/Microsoft.Network", "networkSecurityGroups", strNetworksecurityGroupName);
        bool fNetworkSecurityGroupExists = ::DoesAzureResourceExist(strAzureAccessToken, strNetworkSecurityGroupId);
        if ((true == fVirtualNetworkExists) && (false == fNetworkSecurityGroupExists))
        {
            // If it does not exist update the state to AzureTemplateState::eCreatingNetworkSecurityGroup
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eCreatingNetworkSecurityGroup);
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
            oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));

            // Create the Network Security Group
            StructuredBuffer oNetworkSecurityGroupCreateParameter;
            oNetworkSecurityGroupCreateParameter.PutString("NetworkSecurityGroupName", strNetworksecurityGroupName);
            std::string strDeploymentParameters = ::CreateAzureParamterJson("https://confidentialvmdeployment.blob.core.windows.net/deployemnttemplate/NetworkSecurityGroup.json?sp=r&st=2021-08-18T11:45:41Z&se=2022-08-31T19:45:41Z&spr=https&sv=2020-08-04&sr=b&sig=6NdypMlPI6D0UVzeux1HUY9KaRns%2BFjX2yluqPoMT1w%3D", oNetworkSecurityGroupCreateParameter);
            StructuredBuffer oDeploymentResult = ::CreateAzureDeployment(strAzureAccessToken, strDeploymentParameters, strSubscriptionIdentifier, strResourceGroup, strLocation);
            if ("Success" != oDeploymentResult.GetString("Status"))
            {
                // Update the state if it failed
                oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eFailedCreatingNetworkSecurityGroup);
                oAzureTemplate.PutString("Note", "Virtual Network Fail: " + oDeploymentResult.GetString("error"));
                oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
                StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
                oUpdateStatus.PutBuffer("Eosb", oStatusUpdateResponse.GetBuffer("Eosb"));
                fInternalError = false;
                _ThrowBaseException("Failed to create Network Security Group", nullptr);
            }
            else
            {
                fNetworkSecurityGroupExists = true;
            }
        }

        // If both are success, update the state of the Azure Template to AzureTemplateState::eReady
        if ((true == fVirtualNetworkExists) && (true == fNetworkSecurityGroupExists))
        {
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eReady);
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
        }

        // If we are here, then everything should have worked out fine
        fInternalError = false;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    try
    {
        if (true == fInternalError)
        {
            // Update the state if there was an internal error
            StructuredBuffer oUpdateStatus;
            oUpdateStatus.PutString("TemplateGuid", c_strTemplateGuid);
            oUpdateStatus.PutBuffer("Eosb", c_stlEosb);
            oAzureTemplate.PutDword("State", (Dword)AzureTemplateState::eInternalError);
            oAzureTemplate.PutString("Note", "Internal Error: " + strInternalError);
            oUpdateStatus.PutStructuredBuffer("TemplateData", oAzureTemplate);
            StructuredBuffer oStatusUpdateResponse(this->UpdateAzureTemplateState(oUpdateStatus));
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class AzureManager
 * @function UpdateAzureSettingsTemplate
 * @brief Take in full EOSB and update azure settings template in the database
 * @param[in] c_oRequest contains EOSB and the template information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AzureManager::UpdateAzureTemplateState(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket(m_strDatabaseServiceIpAddr.c_str(), m_unDatabaseServiceIpPort);
                // Create a request to update the azure settings template
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateAzureTemplate");
                oRequest.PutString("TemplateGuid", c_oRequest.GetString("TemplateGuid"));
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutStructuredBuffer("TemplateData", c_oRequest.GetStructuredBuffer("TemplateData"));
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
