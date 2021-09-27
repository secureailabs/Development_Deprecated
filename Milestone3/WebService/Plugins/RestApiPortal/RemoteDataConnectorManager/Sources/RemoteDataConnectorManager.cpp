/*********************************************************************************************
 *
 * @file RemoteDataConnectorManager.cpp
 * @author Shabana Akhtar Baig
 * @date 30 Jul 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "RemoteDataConnectorManager.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"

static RemoteDataConnectorManager * gs_oRemoteDataConnectorManager = nullptr;

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
        RemoteDataConnectorManager * poRemoteDataConnectorManager = ::GetRemoteDataConnectorManager();
        poRemoteDataConnectorManager->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
        RemoteDataConnectorManager * poRemoteDataConnectorManager = ::GetRemoteDataConnectorManager();
        poRemoteDataConnectorManager->HandleIpcRequest(poIpcSocket);
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
 * @function GetRemoteDataConnectorManager
 * @brief Create a singleton object of RemoteDataConnectorManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of RemoteDataConnectorManager class
 *
 ********************************************************************************************/

RemoteDataConnectorManager * __stdcall GetRemoteDataConnectorManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oRemoteDataConnectorManager)
    {
        gs_oRemoteDataConnectorManager = new RemoteDataConnectorManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oRemoteDataConnectorManager);
    }

    return gs_oRemoteDataConnectorManager;
}

/********************************************************************************************
 *
 * @function ShutdownRemoteDataConnectorManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownRemoteDataConnectorManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oRemoteDataConnectorManager)
    {
        gs_oRemoteDataConnectorManager->TerminateSignalEncountered();
        gs_oRemoteDataConnectorManager->Release();
        gs_oRemoteDataConnectorManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function RemoteDataConnectorManager
 * @brief Constructor
 *
 ********************************************************************************************/

RemoteDataConnectorManager::RemoteDataConnectorManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function RemoteDataConnectorManager
 * @brief Copy Constructor
 * @param[in] c_oRemoteDataConnectorManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

RemoteDataConnectorManager::RemoteDataConnectorManager(
    _in const RemoteDataConnectorManager & c_oRemoteDataConnectorManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function ~RemoteDataConnectorManager
 * @brief Destructor
 *
 ********************************************************************************************/

RemoteDataConnectorManager::~RemoteDataConnectorManager(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("VirtualMachineManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall RemoteDataConnectorManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "RemoteDataConnectorManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall RemoteDataConnectorManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{ED750518-6470-4DF9-AE67-86DEB031B354}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall RemoteDataConnectorManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall RemoteDataConnectorManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall RemoteDataConnectorManager::InitializePlugin(void)
{
    __DebugFunction();

    // Add parameters for RegisterRemoteDataConnector resource
    StructuredBuffer oRegisterRemoteDataConnector;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterRemoteDataConnector.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oConnectorGuid;
    oConnectorGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oConnectorGuid.PutBoolean("IsRequired", true);
    oRegisterRemoteDataConnector.PutStructuredBuffer("RemoteDataConnectorGuid", oConnectorGuid);
    StructuredBuffer oDatasetGuids;
    oDatasetGuids.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oDatasetGuids.PutBoolean("IsRequired", true);
    oRegisterRemoteDataConnector.PutStructuredBuffer("Datasets", oDatasetGuids);
    StructuredBuffer oVersion;
    oVersion.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVersion.PutBoolean("IsRequired", true);
    oRegisterRemoteDataConnector.PutStructuredBuffer("Version", oVersion);

    // Add parameters for ListRemoteDataConnectors resource
    StructuredBuffer oListConnectors;
    oListConnectors.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for PullRemoteDataConnector resource
    StructuredBuffer oPullConnector;
    oPullConnector.PutStructuredBuffer("Eosb", oEosb);
    oPullConnector.PutStructuredBuffer("RemoteDataConnectorGuid", oConnectorGuid);

    // Add parameters for UpdateRemoteDataConnector resource
    StructuredBuffer oUpdateConnector;
    oUpdateConnector.PutStructuredBuffer("Eosb", oEosb);
    oUpdateConnector.PutStructuredBuffer("RemoteDataConnectorGuid", oConnectorGuid);
    oUpdateConnector.PutStructuredBuffer("Datasets", oDatasetGuids);
    oUpdateConnector.PutStructuredBuffer("Version", oVersion);

    // Add parameters for ConnectorHeartBeat resource
    StructuredBuffer oConnectorHeartBeat;
    oConnectorHeartBeat.PutStructuredBuffer("Eosb", oEosb);
    oConnectorHeartBeat.PutStructuredBuffer("RemoteDataConnectorGuid", oConnectorGuid);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Stores remote data connector metadata in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/RemoteDataConnectorManager/RegisterConnector", oRegisterRemoteDataConnector, 1);

    // Sends back list of all available remote data connectors
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/RemoteDataConnectorManager/ListConnectors", oListConnectors, 1);

    // Send back remote data connector metadata associated with the requested RemoteDataConnectorGuid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/RemoteDataConnectorManager/PullConnector", oPullConnector, 1);

    // Updates a remote data connector's metadata in the database
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/RemoteDataConnectorManager/UpdateConnector", oUpdateConnector, 1);

    // Fetch list of VM ipaddressese that are waiting for the remote data connector's dataset(s)
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/RemoteDataConnectorManager/HeartBeat", oConnectorHeartBeat, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{9546C893-7F55-4FB7-BA63-B94B172105A0}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("VirtualMachineManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);
}

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall RemoteDataConnectorManager::RunIpcServer(
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
 * @class RemoteDataConnectorManager
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall RemoteDataConnectorManager::HandleIpcRequest(
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
        case 0x00000001 // GetListOfRemoteDataConnectors
        :
            stlResponse = this->GetListOfRemoteDataConnectors(oRequestParameters);
            break;
        default
        :
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
 * @class RemoteDataConnectorManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall RemoteDataConnectorManager::SubmitRequest(
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
        if ("/SAIL/RemoteDataConnectorManager/ListConnectors" == strResource)
        {
            stlResponseBuffer = this->GetListOfRemoteDataConnectors(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/RemoteDataConnectorManager/PullConnector" == strResource)
        {
            stlResponseBuffer = this->PullRemoteDataConnector(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/RemoteDataConnectorManager/RegisterConnector" == strResource)
        {
            stlResponseBuffer = this->RegisterRemoteDataConnector(c_oRequestStructuredBuffer);
        }
    }
    else if ("PUT" == strVerb)
    {
        if ("/SAIL/RemoteDataConnectorManager/UpdateConnector" == strResource)
        {
            stlResponseBuffer = this->UpdateRemoteDataConnector(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/RemoteDataConnectorManager/HeartBeat" == strResource)
        {
            stlResponseBuffer = this->ConnectorHeartBeat(c_oRequestStructuredBuffer);
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
 * @class RemoteDataConnectorManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall RemoteDataConnectorManager::GetResponse(
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
 * @class RemoteDataConnectorManager
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::GetUserInfo(
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
 * @class RemoteDataConnectorManager
 * @function GetListOfRemoteDataConnectors
 * @brief Send back a list of all available remote data connectors
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing list of all available remote data connectors
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::GetListOfRemoteDataConnectors(
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
            // if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to list of all available remote data connectors
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListRemoteDataConnectors");
                // OrganizationGuid provided in the Request is used preferably, otherwise the
                // one from the Eosb is used
                if (true == c_oRequest.IsElementPresent("OrganizationGuid", ANSI_CHARACTER_STRING_VALUE_TYPE))
                {
                    oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
                }
                else
                {
                    oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                }
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
                    oResponse.PutStructuredBuffer("Connectors", oDatabaseResponse.GetStructuredBuffer("Connectors"));
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

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function PullRemoteDataConnector
 * @brief Get remote data connector metadata associated with the GUID
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing metadata of a remote data connector associated with the GUID
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::PullRemoteDataConnector(
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
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to get remote data connector metadata
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullRemoteDataConnector");
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutString("RemoteDataConnectorGuid", c_oRequest.GetString("RemoteDataConnectorGuid"));
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
                    oResponse.PutStructuredBuffer("Connector", oDatabaseResponse.GetStructuredBuffer("Connector"));
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

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function RegisterRemoteDataConnector
 * @brief Take in full EOSB and register a remote data connector
 * @param[in] c_oRequest contains EOSB and the remote data connector information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::RegisterRemoteDataConnector(
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
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add remote data connector's metadata to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "POST");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterRemoteDataConnector");
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutString("UserGuid", oUserInfo.GetGuid("UserGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutStructuredBuffer("Datasets", c_oRequest.GetStructuredBuffer("Datasets"));
                oRequest.PutString("RemoteDataConnectorGuid", c_oRequest.GetString("RemoteDataConnectorGuid"));
                oRequest.PutString("Version", c_oRequest.GetString("Version"));
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
                
                // Check if DatabaseManager registered the connector or not
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
 * @class RemoteDataConnectorManager
 * @function UpdateRemoteDataConnector
 * @brief Take in full EOSB and update remote data connector in the database
 * @param[in] c_oRequest contains EOSB and the connector's information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::UpdateRemoteDataConnector(
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
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to update the remote data connector
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateRemoteDataConnector");
                oRequest.PutString("RemoteDataConnectorGuid", c_oRequest.GetString("RemoteDataConnectorGuid"));
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
                oRequest.PutStructuredBuffer("Datasets", c_oRequest.GetStructuredBuffer("Datasets"));
                oRequest.PutString("Version", c_oRequest.GetString("Version"));
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

/********************************************************************************************
 *
 * @class RemoteDataConnectorManager
 * @function ConnectorHeartBeat
 * @brief Send back list of VM ipaddressese that are waiting for the remote data connector's dataset(s)
 * @param[in] c_oRequest contains EOSB and the connector's information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall RemoteDataConnectorManager::ConnectorHeartBeat(
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
            // Get datasets that the remote data connector is serving
            StructuredBuffer oDatasets = StructuredBuffer(this->PullRemoteDataConnector(c_oRequest)).GetStructuredBuffer("Connector").GetStructuredBuffer("Datasets");
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to update the remote data connector
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/GetVmsWaitingForData");
            oRequest.PutStructuredBuffer("Datasets", oDatasets);
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

                // Only send the list of VMs which are waiting for data.
                StructuredBuffer oVirtualMachinesWaiting = oDatabaseResponse.GetStructuredBuffer("VirtualMachines");
                for (auto strVirtualMachineUuid : oVirtualMachinesWaiting.GetNamesOfElements())
                {
                    // Get the VM details
                    StructuredBuffer oGetVmStateRequest;
                    oGetVmStateRequest.PutDword("TransactionType", 0x00000005);
                    oGetVmStateRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                    oGetVmStateRequest.PutString("VirtualMachineGuid", strVirtualMachineUuid);
                    Socket * poIpcVirtualMachineManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
                    StructuredBuffer oGetVmStateResponse(::PutIpcTransactionAndGetResponse(poIpcVirtualMachineManager, oGetVmStateRequest, false));
                    poIpcVirtualMachineManager->Release();
                    poIpcVirtualMachineManager = nullptr;

                    StructuredBuffer oVirtualMachine = oGetVmStateResponse.GetStructuredBuffer("VirtualMachine");
                    if ((Dword)VirtualMachineState::eWaitingForData != oVirtualMachine.GetDword("State"))
                    {
                        oVirtualMachinesWaiting.RemoveElement(strVirtualMachineUuid.c_str());
                    }
                }

                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("VirtualMachines", oVirtualMachinesWaiting);
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