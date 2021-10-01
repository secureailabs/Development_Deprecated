/*********************************************************************************************
 *
 * @file AuditLogManager.cpp
 * @author Shabana Akhtar Baig
 * @date 02 Feb 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "AuditLogManager.h"
#include "64BitHashes.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"

static AuditLogManager * gs_oAuditLogManager = nullptr;

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
        AuditLogManager * poAuditLogManager = ::GetAuditLogManager();
        poAuditLogManager->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
    __DebugAssert(nullptr != poIpcSocket);

    try
    {
        AuditLogManager * poAuditLogManager = ::GetAuditLogManager();
        poAuditLogManager->HandleIpcRequest(poIpcSocket);
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
 * @function GetAuditLogManager
 * @brief Create a singleton object of AuditLogManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of AuditLogManager class
 *
 ********************************************************************************************/

AuditLogManager * __stdcall GetAuditLogManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oAuditLogManager)
    {
        gs_oAuditLogManager = new AuditLogManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oAuditLogManager);
    }

    return gs_oAuditLogManager;
}

/********************************************************************************************
 *
 * @function ShutdownAuditLogManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownAuditLogManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oAuditLogManager)
    {
        gs_oAuditLogManager->TerminateSignalEncountered();
        gs_oAuditLogManager->Release();
        gs_oAuditLogManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function AuditLogManager
 * @brief Constructor
 *
 ********************************************************************************************/

AuditLogManager::AuditLogManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function AuditLogManager
 * @brief Copy Constructor
 * @param[in] c_oAuditLogManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

AuditLogManager::AuditLogManager(
    _in const AuditLogManager & c_oAuditLogManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function ~AuditLogManager
 * @brief Destructor
 *
 ********************************************************************************************/

AuditLogManager::~AuditLogManager(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("AuditLogManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AuditLogManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "AuditLogManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall AuditLogManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{9F049392-9EA7-4436-B737-1C40CC31CA38}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall AuditLogManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall AuditLogManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall AuditLogManager::InitializePlugin(void)
{
    __DebugFunction();

    // Add parameters for registering leaf events
    // Name, ElementType, and Range (if exists) are used by RestFrameworkRuntimeData::RunThread to vet request parameters.
    // Required parameters are marked by setting IsRequired to true
    // Otherwise the parameter is optional
    StructuredBuffer oRegisterLeafEvents;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterLeafEvents.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oIdentifierOfParentNode;
    oIdentifierOfParentNode.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oIdentifierOfParentNode.PutBoolean("IsRequired", true);
    oRegisterLeafEvents.PutStructuredBuffer("ParentGuid", oIdentifierOfParentNode);
    StructuredBuffer oEvents;
    oEvents.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oEvents.PutBoolean("IsRequired", true);
    oRegisterLeafEvents.PutStructuredBuffer("LeafEvents", oEvents);

    // Add required parameters for listing events
    StructuredBuffer oGetListOfEvents;
    oGetListOfEvents.PutStructuredBuffer("Eosb", oEosb);
    oGetListOfEvents.PutStructuredBuffer("ParentGuid", oIdentifierOfParentNode);
    StructuredBuffer oOrganizationGuid;
    oOrganizationGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oOrganizationGuid.PutBoolean("IsRequired", true);
    oGetListOfEvents.PutStructuredBuffer("OrganizationGuid", oOrganizationGuid);
    // Add optional filter parameters for fetching list of events
    StructuredBuffer oFilters;
    oFilters.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oFilters.PutBoolean("IsRequired", true);
    StructuredBuffer oSequenceNumber;
    oSequenceNumber.PutByte("ElementType", UINT32_VALUE_TYPE);
    oSequenceNumber.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("SequenceNumber", oSequenceNumber);
    StructuredBuffer oMinimumDate;
    oMinimumDate.PutByte("ElementType", UINT64_VALUE_TYPE);
    oMinimumDate.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("MinimumDate", oMinimumDate);
    StructuredBuffer oMaximumDate;
    oMaximumDate.PutByte("ElementType", UINT64_VALUE_TYPE);
    oMaximumDate.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("MaximumDate", oMaximumDate);
    StructuredBuffer oIdentifierOfDcNode;
    oIdentifierOfDcNode.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oIdentifierOfDcNode.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("DCGuid", oIdentifierOfDcNode);
    StructuredBuffer oIdentifierOfVmNode;
    oIdentifierOfVmNode.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oIdentifierOfVmNode.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("VMGuid", oIdentifierOfVmNode);
    StructuredBuffer oTypeOfEvents;
    oTypeOfEvents.PutByte("ElementType", QWORD_VALUE_TYPE);
    oTypeOfEvents.PutBoolean("IsRequired", false);
    oFilters.PutStructuredBuffer("TypeOfEvents", oTypeOfEvents);
    oGetListOfEvents.PutStructuredBuffer("Filters", oFilters);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Takes in a EOSB and registers leaf events
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AuditLogManager/LeafEvents", oRegisterLeafEvents, 1);

    // Takes in an EOSB and lists events
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AuditLogManager/GetListOfEvents", oGetListOfEvents, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("AuditLogManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall AuditLogManager::RunIpcServer(
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
 * @class AuditLogManager
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall AuditLogManager::HandleIpcRequest(
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
        case 0x00000001 // AddNonLeafEvent
        :
            stlResponse = this->AddNonLeafEvent(oRequestParameters);
            break;
        case 0x00000002 // GetListOfEvents
        :
            stlResponse = this->GetListOfEvents(oRequestParameters);
            break;
        case 0x00000003 // DigitalContractBranchExists
        :
            stlResponse = this->DigitalContractBranchExists(oRequestParameters);
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
 * @class AuditLogManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall AuditLogManager::SubmitRequest(
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
    if ("POST" == strVerb)
    {
        if ("/SAIL/AuditLogManager/LeafEvents" == strResource)
        {
            stlResponseBuffer = this->AddLeafEvent(c_oRequestStructuredBuffer);
        }
    }
    else if ("GET" == strVerb)
    {
        if ("/SAIL/AuditLogManager/GetListOfEvents" == strResource)
        {
            stlResponseBuffer = this->GetListOfEvents(c_oRequestStructuredBuffer);
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
 * @class AuditLogManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall AuditLogManager::GetResponse(
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
 * @class AuditLogManager
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::GetUserInfo(
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
            // Send back the user information
            oResponse.PutGuid("UserGuid", oEosb.GetGuid("UserId"));
            oResponse.PutGuid("OrganizationGuid", oEosb.GetGuid("OrganizationGuid"));
            // TODO: get user access rights from the confidential record, for now it can't be decrypted
            oResponse.PutQword("AccessRights", oEosb.GetQword("UserAccessRights"));
            // Send back the updated Eosb
            oResponse.PutBuffer("Eosb", oDecryptedEosb.GetBuffer("UpdatedEosb"));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
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
    
    // Send back the transaction status
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function AddLeafEvent
 * @brief Take in a full EOSB and non-leaf event, and call Database portal to store it in the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::AddNonLeafEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Create a request to store a non leaf event in the database
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "POST");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/NonLeafEvent");
        oRequest.PutStructuredBuffer("NonLeafEvent", c_oRequest.GetStructuredBuffer("NonLeafEvent"));
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
        
        // Check if DatabaseManager registered the events or not
        StructuredBuffer oDatabaseResponse(stlResponse);
        if (404 != oDatabaseResponse.GetDword("Status"))
        {
            dwStatus = 201;
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
 * @class AuditLogManager
 * @function AddLeafEvent
 * @brief Take in a full EOSB and leaf event(s), and call Database portal to store it in the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::AddLeafEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oStatus;

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
            // Create a request to store a non leaf event in the database
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "POST");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/LeafEvent");
            oRequest.PutString("ParentGuid", c_oRequest.GetString("ParentGuid"));
            oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
            oRequest.PutStructuredBuffer("LeafEvents", c_oRequest.GetStructuredBuffer("LeafEvents"));
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
            
            // Check if DatabaseManager registered the events or not
            StructuredBuffer oResponse(stlResponse);
            if (200 == oResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oStatus.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oStatus.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oStatus.PutDword("Status", dwStatus);

    return oStatus.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function GetListOfEvents
 * @brief Given an EOSB get list of audit log event(s) from the database
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing audit logs
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::GetListOfEvents(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oAuditLogs;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to fetch list of events
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/Events");
            oRequest.PutString("ParentGuid", c_oRequest.GetString("ParentGuid"));
            oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
            oRequest.PutStructuredBuffer("Filters", c_oRequest.GetStructuredBuffer("Filters"));
            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

            // Read header and body of the response
            std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            std::vector<Byte> stlResponse= poTlsNode->Read(unResponseDataSizeInBytes, 2000);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;
                
            StructuredBuffer oResponse(stlResponse);
            if (200 == oResponse.GetDword("Status"))
            {
                oAuditLogs.PutStructuredBuffer("ListOfEvents", oResponse.GetStructuredBuffer("ListOfEvents"));
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                dwStatus = 200;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oAuditLogs.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oAuditLogs.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oAuditLogs.PutDword("Status", dwStatus);

    return oAuditLogs.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AuditLogManager
 * @function DigitalContractBranchExists
 * @brief Given an organization guid fetch digital contract event guid, if exists
 * @param[in] c_oRequest contains the request body
 * @returns Digital contract event guid
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AuditLogManager::DigitalContractBranchExists(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oAuditLog;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Create a request to fetch list of events
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "GET");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/GetDCEvent");
        oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
        oRequest.PutStructuredBuffer("Filters", c_oRequest.GetStructuredBuffer("Filters"));
        std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
        // Send request packet
        poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

        // Read header and body of the response
        std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
        _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
        unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
        std::vector<Byte> stlResponse= poTlsNode->Read(unResponseDataSizeInBytes, 2000);
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
        // Make sure to release the poTlsNode
        poTlsNode->Release();
        poTlsNode = nullptr;
        
        StructuredBuffer oResponse(stlResponse);
        if (200 == oResponse.GetDword("Status"))
        {
            oAuditLog.PutString("DCEventGuid", oResponse.GetString("DCEventGuid"));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oAuditLog.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oAuditLog.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Send back status of the transaction
    oAuditLog.PutDword("Status", dwStatus);

    return oAuditLog.GetSerializedBuffer();
}