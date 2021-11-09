/*********************************************************************************************
 *
 * @file CryptographicKeyManagementPlugin.cpp
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "CryptographicKeyManagementPlugin.h"
#include "HardCodedCryptographicKeys.h"
#include "Base64Encoder.h"
#include "CryptographicEngine.h"
#include "ThreadManager.h"
#include "DateAndTime.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketServer.h"
#include "TlsClient.h"

#include <unistd.h>
#include <fstream>

#include <sys/stat.h>

// Singleton Object, can be declared anywhere, but only once
static CryptographicKeyManagementPlugin * gs_oCryptographicKeyManagementPlugin = nullptr;

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
        CryptographicKeyManagementPlugin * poCryptographicKeyManagementPlugin = CryptographicKeyManagementPlugin::Get();
        poCryptographicKeyManagementPlugin->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
        CryptographicKeyManagementPlugin * poCryptographicKeyManagementPlugin = CryptographicKeyManagementPlugin::Get();
        poCryptographicKeyManagementPlugin->HandleIpcRequest(poIpcSocket);
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
 * @class CryptographicKeyManagementPlugin
 * @function Get
 * @brief Get a reference to a singleton object of CryptographicKeyManagementPlugin class
 * @return Return the reference to the singleton object of CryptographicKeyManagementPlugin
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin * __stdcall CryptographicKeyManagementPlugin::Get(void)
{
    __DebugFunction();

    if (nullptr == gs_oCryptographicKeyManagementPlugin)
    {
        gs_oCryptographicKeyManagementPlugin = new CryptographicKeyManagementPlugin();
        _ThrowOutOfMemoryExceptionIfNull(gs_oCryptographicKeyManagementPlugin);
    }

    return gs_oCryptographicKeyManagementPlugin;
}

/********************************************************************************************
 *
 * @function ShutdownCryptographicKeyManagementPlugin
 * @brief Release the object resources
 *
 ********************************************************************************************/

void __stdcall ShutdownCryptographicKeyManagementPlugin(void)
{
    __DebugFunction();

    if (nullptr != gs_oCryptographicKeyManagementPlugin)
    {
        gs_oCryptographicKeyManagementPlugin->TerminateSignalEncountered();
        gs_oCryptographicKeyManagementPlugin->Release();
        gs_oCryptographicKeyManagementPlugin = nullptr;
    }
}

/********************************************************************************************
 *
 * @function ManageEphemeralKeys
 * @brief To to rotate keys after a particular interval
 *
 ********************************************************************************************/

void * __stdcall ManageEphemeralKeys(void *)
{
    __DebugFunction();

    CryptographicKeyManagementPlugin * poCryptographicKeyManagementPlugin = CryptographicKeyManagementPlugin::Get();
    poCryptographicKeyManagementPlugin->RotateEphemeralKeys();

    return nullptr;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CryptographicKeyManagementPlugin
 * @brief Constructor
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin::CryptographicKeyManagementPlugin(void)
    : m_oPluginGuid("{30998245-A931-4518-9A9D-FB0F43F1F02D}"), m_strPluginName("CryptographicKeyManagementPlugin")
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_sEosbKeyMutex = PTHREAD_MUTEX_INITIALIZER;
    m_sEosbCacheMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function ~CryptographicKeyManagementPlugin
 * @brief Destructor
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin::~CryptographicKeyManagementPlugin(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("CryptographicManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall CryptographicKeyManagementPlugin::GetName(void) const throw()
{
    __DebugFunction();

    return m_strPluginName.c_str();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall CryptographicKeyManagementPlugin::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{30998245-A931-4518-9A9D-FB0F43F1F02D}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall CryptographicKeyManagementPlugin::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall CryptographicKeyManagementPlugin::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
    // Wake up the thread responsible for rotating keys
    m_oRotateKeysConditionalVariable.notify_one();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function RotateEphemeralKeys
 * @brief Creates Ephemeral Keys and rotates them at a fixed interval
 *
 ********************************************************************************************/

void __thiscall CryptographicKeyManagementPlugin::RotateEphemeralKeys()
{
    while (false == m_fTerminationSignalEncountered)
    {
        ::pthread_mutex_lock(&m_sEosbKeyMutex);
        if (Guid((const char *)nullptr) != m_oGuidEosbCurrentKey)
        {
            m_oGuidEosbPredecessorKey = m_oGuidEosbCurrentKey;
        }

        CryptographicKey oCurrentKey(KeySpec::eAES256);
        oCurrentKey.StoreKey();
        m_oGuidEosbCurrentKey = oCurrentKey.GetKeyGuid();
        ::pthread_mutex_unlock(&m_sEosbKeyMutex);

        // Block for 20 minutes, or until notified
        std::mutex oMutex;
        std::unique_lock<std::mutex> oLock(oMutex);
        m_oRotateKeysConditionalVariable.wait_until(oLock, std::chrono::system_clock::now() + std::chrono::minutes(20));
    }
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall CryptographicKeyManagementPlugin::InitializePlugin(void)
{
    __DebugFunction();

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    _ThrowIfNull(poThreadManager, "GetThreadManager not found.", nullptr);

    SocketServer * poIpcServer = new SocketServer("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("CryptographicManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);

    StructuredBuffer oRefreshEosb;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRefreshEosb.PutStructuredBuffer("Eosb", oEosb);
    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Re-encrypts the old Eosb with the latest key and returns the fresh Eosb
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/CryptographicManager/User/RefreshEosb", oRefreshEosb, 0);

    StructuredBuffer oSignMessageDigest;
    oSignMessageDigest.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oMessageDigest;
    oMessageDigest.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oMessageDigest.PutBoolean("IsRequired", true);
    oSignMessageDigest.PutStructuredBuffer("MessageDigest", oMessageDigest);
    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Sign the message digest with the specified key or the user key
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/CryptographicManager/User/SignMessageDigest", oSignMessageDigest, 0);

    StructuredBuffer oVerifySignature;
    oVerifySignature.PutStructuredBuffer("Eosb", oEosb);
    oVerifySignature.PutStructuredBuffer("MessageDigest", oMessageDigest);
    StructuredBuffer oSignature;
    oSignature.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oSignature.PutBoolean("IsRequired", true);
    oVerifySignature.PutStructuredBuffer("Signature", oSignature);
    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Verify the signature with the userkey and message digest
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/CryptographicManager/User/VerifySignature", oVerifySignature, 0);

    // TODO: Remove this resource in the future or figure out how the Initializer is going to get the IEosb
    StructuredBuffer oGetIEosb;
    oGetIEosb.PutStructuredBuffer("Eosb", oEosb);
    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Takes in an EOSB and sends back an imposter EOSB (IEOSB)
    // IEOSB has restricted rights and thus minimizes security risks when initializing and logging onto VM's
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/CryptographicManager/User/GetIEosb", oGetIEosb, 0);

    // Generate the ephemeral Keys and keep rotating them every 20 minutes and at a time keep only the
    // latest key active and it's predecessor for the grace period.
    m_unKeyRotationThreadID = poThreadManager->CreateThread(nullptr, ::ManageEphemeralKeys, nullptr);
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall CryptographicKeyManagementPlugin::RunIpcServer(
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
 * @class CryptographicKeyManagementPlugin
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall CryptographicKeyManagementPlugin::HandleIpcRequest(
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
        case 0x00000001 // GetEosb
        :
            stlResponse = this->GenerateEosb(oRequestParameters);
            break;
        case 0x00000002 // RegisterEosb
        :
            stlResponse = this->RegisterEosb(oRequestParameters);
            break;
        case 0x00000003 // UnregisterEosb
        :
            stlResponse = this->UnregisterEosb(oRequestParameters);
            break;
        case 0x00000004 // CreateDigitalSignature
        :
            stlResponse = this->CreateDigitalSignature(oRequestParameters);
            break;
        case 0x00000005 // ModifyEosbAccessRights
        :
            stlResponse = this->ModifyEosbAccessRights(oRequestParameters);
            break;
        case 0x00000006 // CheckEosbAccessRights
        :
            stlResponse = this->CheckEosbAccessRights(oRequestParameters);
            break;
        case 0x00000007 // GetUserInfoAndUpdateEosb which registers and unregisters an Eosb
        :
            stlResponse = this->GetUserInfoAndUpdateEosb(oRequestParameters);
            break;
        case 0x00000008 // UpdateUserAccessRights
        :
            stlResponse = this->UpdateUserAccessRights(oRequestParameters);
            break;
        case 0x00000009 // UpdateUserInformation
        :
            stlResponse = this->UpdateUserInformation(oRequestParameters);
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
 * @class CryptographicKeyManagementPlugin
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall CryptographicKeyManagementPlugin::SubmitRequest(
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
        if ("/SAIL/CryptographicManager/User/RefreshEosb" == strResource)
        {
            stlResponseBuffer = this->RefreshEosb(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/CryptographicManager/User/SignMessageDigest" == strResource)
        {
            stlResponseBuffer = this->SignMessageDigest(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/CryptographicManager/User/VerifySignature" == strResource)
        {
            stlResponseBuffer = this->VerifySignature(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/CryptographicManager/User/GetIEosb" == strResource)
        {
            // TODO: Remove this else clause in the future or figure out how the Initializer is going to get the IEosb
            StructuredBuffer oIEosbRequest;
            oIEosbRequest.PutBuffer("Eosb", c_oRequestStructuredBuffer.GetBuffer("Eosb"));
            // TODO: Add common types to a common shared folder
            oIEosbRequest.PutQword("AccessRights", 2);  // 2 is for IEosb access rights
            stlResponseBuffer = this->ModifyEosbAccessRights(oIEosbRequest);
        }
    }

    // Return size of response buffer
    *punSerializedResponseSizeInBytes = stlResponseBuffer.size();
    _ThrowBaseExceptionIf((0 >= *punSerializedResponseSizeInBytes), "Error processing the request.", nullptr);

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
 * @class CryptographicKeyManagementPlugin
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
 ********************************************************************************************/

 bool __thiscall CryptographicKeyManagementPlugin::GetResponse(
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
 * @class CryptographicKeyManagementPlugin
 * @function GenerateEosb
 * @brief Create a new Eosb for the login request
 * @param[in] c_oStructuredBufferRequest contains the request body
 * @throw BaseException on failure
 * @returns Buffer with encrypted Eosb and decryption information
 * @note
 *      The c_oStructuredBufferRequest StructuredBuffer should have
 *        +----------------------------------------------------------------------------------+
 *        | ["Passphrase":String] {Required} Password to generate key                        |
 *        +----------------------------------------------------------------------------------+
 *        | ["ConfidentialUserRecord":StructuredBuffer] {Required} Double Encrypted          |
 *                                                                 Confidential User Record  |
 *        +----------------------------------------------------------------------------------+
 *        | ["BasicUserRecord":StructuredBuffer] {Required} Basic User record                    |
 *        +----------------------------------------------------------------------------------+
 *
 *      The ConfidentialUserRecord must atleast have the following elements:
 *        +----------------------------------------------------------------------------------+
 *        | ["IV":Buffer] {Required} IV used to encrypt data with SAIL Key                   |
 *        +----------------------------------------------------------------------------------+
 *        | ["TAG":Buffer] {Required} Aes Tag for decryption authentication                  |
 *        +----------------------------------------------------------------------------------+
 *        | ["SailKeyEncryptedConfidentialUserRecord":Buffer] {Required} SAIL Key Encrypted
 *                                                                  User Confidential record |
 *        +----------------------------------------------------------------------------------+
 *
 *      The BasicUserRecord must atleast have the following elements:
 *        +----------------------------------------------------------------------------------+
 *        | ["WrapedAccountKey":Buffer] {Required} Account Key Wrapped with Password Key     |
 *        +----------------------------------------------------------------------------------+
 *        | ["UserId":Buffer] {Required} User Guid                                           |
 *        +----------------------------------------------------------------------------------+
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GenerateEosb(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    std::vector<Byte> stlEosb;

    try 
    {
        const std::string strPassphrase = c_oStructuredBufferRequest.GetString("Passphrase");
        const StructuredBuffer oStructuredBufferConfidentialUserRecord = c_oStructuredBufferRequest.GetStructuredBuffer("ConfidentialOrganizationOrUserRecord");
        const StructuredBuffer oStructuredBufferBasicUserRecord = c_oStructuredBufferRequest.GetStructuredBuffer("BasicUserRecord");

        // Fetch the reference to the Cryptographic Engine Singleton Object
        const CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

        // Generate a password derived Key
        // Not using the Engine fucntion call to generate the key as that is used when the key is
        // generated and stored to the file systems and only the Guid of the Key is returned.
        CryptographicKeyUniquePtr oPasswordDerivedWrapKey = std::make_unique<CryptographicKey>(KeySpec::ePDKDF2, HashAlgorithm::eSHA256, strPassphrase);

        // Set the parameters for unwrapping/decrypting the key
        // Password derived keys also derive IV from the same password. So, not needed.
        StructuredBuffer oDecryptParams;

        // Unwrap the Account Key from the Basic Record using the password derived key
        std::vector<Byte> stlAccountKey;
        OperationID oUnwrapKeyOperationId = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, std::move(oPasswordDerivedWrapKey));
        oCryptographicEngine.OperationUpdate(oUnwrapKeyOperationId, oStructuredBufferBasicUserRecord.GetBuffer("WrappedAccountKey"), stlAccountKey);
        bool fDecryptStatus = oCryptographicEngine.OperationFinish(oUnwrapKeyOperationId, stlAccountKey);
        _ThrowBaseExceptionIf((false == fDecryptStatus), "Account key Decryption using Password Key failed.", nullptr);

        // Generate account encryption key 
        CryptographicKeyUniquePtr oAccountEncryptionKey = std::make_unique<CryptographicKey>(KeySpec::ePDKDF2, HashAlgorithm::eSHA256, std::string(stlAccountKey.begin(), stlAccountKey.end()));

        // Get the Double Encrypted Confidential User Record and first decrypt it using the
        // AES-GCM SAIL Secret key and then using the account encryption key
        StructuredBuffer oEncryptedUserRecord(oStructuredBufferConfidentialUserRecord.GetBuffer("EncryptedSsb"));
        oDecryptParams.PutBuffer("IV", oEncryptedUserRecord.GetBuffer("IV"));
        oDecryptParams.PutBuffer("TAG", oEncryptedUserRecord.GetBuffer("TAG"));

        // TODO: don't use the hard-coded key eventually.
        Guid oSailSecretKey = "76A426D93D1F4F82AFA48843140EF603";
        struct stat buffer;
        if (0 != stat("76A426D93D1F4F82AFA48843140EF603.key", &buffer))
        {
            std::vector<Byte> c_stlSailKeyFile = {
                0xCD, 0xAB, 0x9C, 0xE5, 0x3B, 0x93, 0x52, 0x2C, 0xB2, 0x9A, 0x7D, 0xE5, 0x57, 0x55, 0x12, 0x98,
                0xC1, 0x6F, 0x29, 0xE8, 0x02, 0x00, 0x00, 0x00, 0x83, 0x63, 0x74, 0x83, 0x3D, 0x00, 0x00, 0x00,
                0x34, 0x01, 0xFB, 0x54, 0x12, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x0D, 0x02, 0x0F,
                0x01, 0x4B, 0x65, 0x79, 0x00, 0x0B, 0x04, 0x0C, 0x03, 0xE5, 0x26, 0x7B, 0x98, 0xA6, 0x03, 0x84,
                0x1F, 0x6C, 0x5F, 0x40, 0xF4, 0x4D, 0xAE, 0x3D, 0x1C, 0xE8, 0x06, 0x0E, 0xEB, 0xC1, 0xCB, 0x68,
                0xD7, 0x0C, 0x0C, 0x60, 0x28, 0xEF, 0xA2, 0x21, 0x8A, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63, 0x74,
                0x83, 0x28, 0x00, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x08, 0x00, 0x00, 0x00, 0x07, 0x00,
                0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x4B, 0x65, 0x79, 0x53, 0x70, 0x65, 0x63, 0x00, 0x0B, 0x04,
                0x0C, 0x03, 0x41, 0x45, 0x53, 0x32, 0x35, 0x36, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x5E, 0xC9, 0xBA,
                0xDC};

            std::ofstream oKeyFile("76A426D93D1F4F82AFA48843140EF603.key", std::ios::binary);
            oKeyFile.write((const char *)c_stlSailKeyFile.data(), c_stlSailKeyFile.size());
            oKeyFile.close();
        }

        // Decrypt confidential record using the sail secret key
        std::vector<Byte> stlUserKeyEncryptedUserRecord;
        OperationID oDecryptionID = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, oSailSecretKey, &oDecryptParams);
        oCryptographicEngine.OperationUpdate(oDecryptionID, oEncryptedUserRecord.GetBuffer("SailKeyEncryptedConfidentialUserRecord"), stlUserKeyEncryptedUserRecord);
        fDecryptStatus = oCryptographicEngine.OperationFinish(oDecryptionID, stlUserKeyEncryptedUserRecord);
        _ThrowBaseExceptionIf((false == fDecryptStatus), "Confidential User Record Decryption using SAIL Key failed.", nullptr);

        // Password Derived Keys have their own IV derived form the password. So they are not needed
        // Decryption using the Password derived key uses the AES-CFB mode becasue
        // the authenticity of the data has already been ensured in the previous GCM decryption
        // So the tag in not needed in Decryption Paramaters.
        oDecryptParams.RemoveElement("IV");
        oDecryptParams.RemoveElement("TAG");

        // Decrypt the confidential record decrypted by the sail secret key now by using the account encryption key
        std::vector<Byte> stlSerializedPlainTextConfidentialUserRecord;
        OperationID oUserKeyDecryptionID = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, std::move(oAccountEncryptionKey), &oDecryptParams);
        oCryptographicEngine.OperationUpdate(oUserKeyDecryptionID, stlUserKeyEncryptedUserRecord, stlSerializedPlainTextConfidentialUserRecord);
        fDecryptStatus = oCryptographicEngine.OperationFinish(oUserKeyDecryptionID, stlSerializedPlainTextConfidentialUserRecord);
        _ThrowBaseExceptionIf((false == fDecryptStatus), "Confidential User Record Decryption using User Account Key failed.", nullptr);

        // Use the Basic User Record and Confidential User Record to get the session Eosb
        StructuredBuffer oPlainTextConfidentialUserRecord(stlSerializedPlainTextConfidentialUserRecord);

        StructuredBuffer oStructuredBufferEosb;
        oStructuredBufferEosb.PutGuid("UserId", oStructuredBufferBasicUserRecord.GetGuid("UserGuid"));
        oStructuredBufferEosb.PutGuid("OrganizationGuid", oStructuredBufferBasicUserRecord.GetGuid("OrganizationGuid"));
        oStructuredBufferEosb.PutString("OrganizationName", oStructuredBufferBasicUserRecord.GetString("OrganizationName"));
        oStructuredBufferEosb.PutGuid("SessionId", Guid());
        oStructuredBufferEosb.PutBuffer("AccountKey", stlAccountKey);
        oStructuredBufferEosb.PutQword("AccessRights", 1);  // 1 is for eEosb access right
        // TODO: Do not add user access rights, user name, title and email in the Eosb
        // It is added in the Eosb for now because there is no way decrypting the confidential record anywhere else
        oStructuredBufferEosb.PutString("Username", oPlainTextConfidentialUserRecord.GetString("Username"));
        oStructuredBufferEosb.PutString("Title", oPlainTextConfidentialUserRecord.GetString("Title"));
        oStructuredBufferEosb.PutString("Email", oPlainTextConfidentialUserRecord.GetString("Email"));
        oStructuredBufferEosb.PutString("PhoneNumber", oPlainTextConfidentialUserRecord.GetString("PhoneNumber"));
        oStructuredBufferEosb.PutQword("UserAccessRights", oPlainTextConfidentialUserRecord.GetQword("AccessRights"));
        oStructuredBufferEosb.PutUnsignedInt64("Timestamp", oPlainTextConfidentialUserRecord.GetUnsignedInt64("TimeOfAccountCreation"));
        oStructuredBufferEosb.PutGuid("UserRootKeyId", oPlainTextConfidentialUserRecord.GetGuid("UserRootKeyGuid"));
        
        // Generate Eosb from the Ssb
        stlEosb = this->CreateEosbFromPlainSsb(oStructuredBufferEosb.GetSerializedBuffer());
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        stlEosb.clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        stlEosb.clear();
    }

    return stlEosb;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function RefreshEosb
 * @brief Encrypt the Eosb with the new Eosb key
 * @param[in] c_oStructuredBufferRequest contains the request body
 * @throw BaseException on failure
 * @returns Buffer with encrypted Eosb and decryption information
 * @note
 *      The c_oStructuredBufferRequest StructuredBuffer should have
 *        +----------------------------------------------------------------------------------+
 *        | ["Eosb":Buffer] {Required} eosb to refresh                                       |
 *        +----------------------------------------------------------------------------------+
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::RefreshEosb(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponseEosb;

    Dword dwStatus = 404;

    try 
    {
        std::vector<Byte> stlEncryptedEsobBuffer = c_oStructuredBufferRequest.GetBuffer("Eosb");

        // Esob Buffer Header check
        _ThrowBaseExceptionIf((*((Qword *)m_stlEosbHeader.data()) != *((Qword *)stlEncryptedEsobBuffer.data())), "Invalid Header", nullptr);

        std::size_t unEsobCounter = 0;
        unEsobCounter += m_stlEosbHeader.size();
        unEsobCounter += AES_GCM_IV_LENGTH;
        unEsobCounter += AES_TAG_LENGTH;
        Guid oGuidEsobEncryptKey(stlEncryptedEsobBuffer.data() + unEsobCounter);
        // Check the EosbKey Guid and procees only if it was encrypted with the predecessor Key
        // TODO: add locks
        if (m_oGuidEosbCurrentKey == oGuidEsobEncryptKey)
        {
            oResponseEosb.PutBuffer("Eosb", stlEncryptedEsobBuffer);
            dwStatus = 201;
        }
        else if (m_oGuidEosbPredecessorKey == oGuidEsobEncryptKey)
        {
            std::vector<Byte> stlPlainTextEsob = this->GetPlainTextSsbFromEosb(stlEncryptedEsobBuffer);

            // Put the Esob buffer into the StrucutredBuffer
            oResponseEosb.PutBuffer("Eosb", this->CreateEosbFromPlainSsb(stlPlainTextEsob));
            dwStatus = 201;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponseEosb.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseEosb.Clear();
    }

    // Send back status of the transaction
    oResponseEosb.PutDword("Status", dwStatus);

    return oResponseEosb.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CreateEosbFromPlainSsb
 * @brief Helper function to put all EOSB data into a buffer
 * @param[in] c_oStructuredBufferRequest contains the request body
 * @throw BaseException on failure
 * @returns Buffer with encrypted Eosb and decryption information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetPlainTextSsbFromEosb(
    _in const std::vector<Byte> & stlEncryptedEsobBuffer
    )
{
    __DebugFunction();

    std::vector<Byte> stlPlainTextEsob;

    std::size_t unEsobCounter = 0;
    // Esob Buffer Header
    _ThrowBaseExceptionIf((*((Qword *)m_stlEosbHeader.data()) != *((Qword *)stlEncryptedEsobBuffer.data())), "Invalid Header", nullptr);
    unEsobCounter += m_stlEosbHeader.size();

    StructuredBuffer oStructuredBufferDecryptionParameters;
    // AES GCM IV
    oStructuredBufferDecryptionParameters.PutBuffer("IV", stlEncryptedEsobBuffer.data() + unEsobCounter, AES_GCM_IV_LENGTH);
    unEsobCounter += AES_GCM_IV_LENGTH;
    // AES GCM TAG
    oStructuredBufferDecryptionParameters.PutBuffer("TAG", stlEncryptedEsobBuffer.data() + unEsobCounter, AES_TAG_LENGTH);
    unEsobCounter += AES_TAG_LENGTH;
    // Identifier of key used for encryption
    Guid oGuidEsobEncryptKey(stlEncryptedEsobBuffer.data() + unEsobCounter);
    unEsobCounter += 16;
    // Check the EosbKey Guid and procees only if it was encrypted with the predecessor Key
    // TODO: add locks
    if ((m_oGuidEosbPredecessorKey == oGuidEsobEncryptKey) || (m_oGuidEosbCurrentKey == oGuidEsobEncryptKey))
    {
        // Size in bytes of encrypted SSB
        unsigned int unSizeOfEncryptedSsb = *(unsigned int *)(stlEncryptedEsobBuffer.data() + unEsobCounter);
        unEsobCounter += sizeof(unsigned int);

        // Get the Eosb Ssb encrypted with the old key in its grace period
        std::vector<Byte> stlEncryptedSsb(stlEncryptedEsobBuffer.data() + unEsobCounter, stlEncryptedEsobBuffer.data() + unEsobCounter + unSizeOfEncryptedSsb);
        unEsobCounter += unSizeOfEncryptedSsb;

        // Esob Buffer Footer
        _ThrowBaseExceptionIf((*((Qword *)m_stlEosbFooter.data()) != *((Qword *)(stlEncryptedEsobBuffer.data() + unEsobCounter))), "Invalid Footer. Expected %ull Got %ull", nullptr);

        CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();
        OperationID oEsobDecryptId = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, oGuidEsobEncryptKey, &oStructuredBufferDecryptionParameters);
        oCryptographicEngine.OperationUpdate(oEsobDecryptId, stlEncryptedSsb, stlPlainTextEsob);
        bool fDeryptionStatus = oCryptographicEngine.OperationFinish(oEsobDecryptId, stlPlainTextEsob);
        _ThrowBaseExceptionIf((false == fDeryptionStatus), "Eosb decryption failed", nullptr);
    }

    return stlPlainTextEsob;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CreateEosbFromPlainSsb
 * @brief Helper function to put all EOSB data into a buffer
 * @param[in] c_oStructuredBufferRequest contains the request body
 * @throw BaseException on failure
 * @returns Buffer with encrypted Eosb and decryption information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::CreateEosbFromPlainSsb(
    _in const std::vector<Byte> & c_stlPlainTextSsb
    )
{
    __DebugFunction();

    std::vector<Byte> stlResponseEosb;

    // Fetch the reference to the Cryptographic Engine Singleton Object
    const CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    // Generate an IV to Encrypt the Eosb serialized Buffer
    const std::vector<Byte> stlAesInitializationVector = ::GenerateRandomBytes(AES_GCM_IV_LENGTH);

    // Fetch the Guid of the current Eosb Encryption Key
    ::pthread_mutex_lock(&m_sEosbKeyMutex);
    const Guid oGuidEncryptKey = m_oGuidEosbCurrentKey;
    ::pthread_mutex_unlock(&m_sEosbKeyMutex);

    // Create a Request Structured Buffer with an initialization vector for the
    // AES-GCM encryption of the Eosb
    StructuredBuffer oStructuredBufferEosbEncryptRequest;
    oStructuredBufferEosbEncryptRequest.PutBuffer("IV", stlAesInitializationVector);

    // This buffer will hold the encrypted serialized Eosb
    std::vector<Byte> stlEncryptedSsb;
    OperationID pEncryptEosbId = oCryptographicEngine.OperationInit(CryptographicOperation::eEncrypt, oGuidEncryptKey, &oStructuredBufferEosbEncryptRequest);
    oCryptographicEngine.OperationUpdate(pEncryptEosbId, c_stlPlainTextSsb, stlEncryptedSsb);
    bool fEncryptStatus = oCryptographicEngine.OperationFinish(pEncryptEosbId, stlEncryptedSsb);

    // Extract the 16 byte AES GCM Tag from the encrypted Cipher Text and resize the original buffer
    std::vector<Byte> stlAesGcmTag(AES_TAG_LENGTH);
    ::memcpy(stlAesGcmTag.data(), stlEncryptedSsb.data() + (stlEncryptedSsb.size() - AES_TAG_LENGTH), AES_TAG_LENGTH);
    stlEncryptedSsb.resize(stlEncryptedSsb.size() - AES_TAG_LENGTH);

    // Fill the output buffer
    const unsigned int unSizeOfEsobInBytes = m_stlEosbHeader.size() + stlAesInitializationVector.size() + stlAesGcmTag.size() + oGuidEncryptKey.ToString(eRaw).length() +  sizeof(uint32_t) + stlEncryptedSsb.size() + m_stlEosbFooter.size();
    // Call reserve to just allocate memory and not initialize with
    stlResponseEosb.reserve(unSizeOfEsobInBytes);

    // Header
    stlResponseEosb.insert(stlResponseEosb.end(), m_stlEosbHeader.begin(), m_stlEosbHeader.end());
    // AES GCM IV
    stlResponseEosb.insert(stlResponseEosb.end(), stlAesInitializationVector.begin(), stlAesInitializationVector.end());
    // AES GCM TAG
    stlResponseEosb.insert(stlResponseEosb.end(), stlAesGcmTag.begin(), stlAesGcmTag.end());
    // Identifier of key used for encryption
    std::vector<Byte> strEosbKeyID = oGuidEncryptKey.GetRawData();
    stlResponseEosb.insert(stlResponseEosb.end(), strEosbKeyID.begin(), strEosbKeyID.end());
    // Size in bytes of encrypted SSB
    const unsigned int unSizeOfEncryptedSsb = stlEncryptedSsb.size();
    stlResponseEosb.push_back(unSizeOfEncryptedSsb >> 0);
    stlResponseEosb.push_back(unSizeOfEncryptedSsb >> 8);
    stlResponseEosb.push_back(unSizeOfEncryptedSsb >> 16);
    stlResponseEosb.push_back(unSizeOfEncryptedSsb >> 24);
    // Encrypted SSB containing DC information
    stlResponseEosb.insert(stlResponseEosb.end(), stlEncryptedSsb.begin(), stlEncryptedSsb.end());
    // Footer
    stlResponseEosb.insert(stlResponseEosb.end(), m_stlEosbFooter.begin(), m_stlEosbFooter.end());

    return stlResponseEosb;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function RegisterEosb
 * @brief Decrypt the Eosb and return the decrypted Eosb without the AES key
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer
 * @note
 *      The RequiredParameters StructuredBuffer should have
 *        +-----------------------------------------------------------------------------------+
 *        | ["Eosb":Buffer] {Required} Encrypted Eosb to register                             |
 *        +-----------------------------------------------------------------------------------+
 *
 *      The Return StructuredBuffer is the decrypted Digital Contract without the AES accont
 *      key as defined in the CreateEosb key
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::RegisterEosb(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponseEosb;

    Dword dwStatus = 405;

    try 
    {
        std::vector<Byte> stlEncryptedEsobBuffer = c_oStructuredBufferRequest.GetBuffer("Eosb");

        // Esob Buffer Header check
        _ThrowBaseExceptionIf((*((Qword *)m_stlEosbHeader.data()) != *((Qword *)stlEncryptedEsobBuffer.data())), "Invalid Header", nullptr);

        // Get the Guid of the key used to encrypt the Eosb
        std::size_t unEsobCounter = 0;
        unEsobCounter += m_stlEosbHeader.size();
        unEsobCounter += AES_GCM_IV_LENGTH;
        unEsobCounter += AES_TAG_LENGTH;
        Guid oGuidEsobEncryptKey(stlEncryptedEsobBuffer.data() + unEsobCounter);

        // Check the EosbKey Guid and procees only if it was encrypted with the predecessor Key
        // TODO: add locks
        if ((m_oGuidEosbCurrentKey == oGuidEsobEncryptKey) || (m_oGuidEosbPredecessorKey == oGuidEsobEncryptKey))
        {
            std::vector<Byte> stlPlainTextEsob = this->GetPlainTextSsbFromEosb(stlEncryptedEsobBuffer);

            StructuredBuffer oStructuredBufferEosbWithoutKey(stlPlainTextEsob);
            std::vector<Byte> stlAccountKey = oStructuredBufferEosbWithoutKey.GetBuffer("AccountKey");
            oStructuredBufferEosbWithoutKey.RemoveElement("AccountKey");

            // Cache the key, which acts as a proof of authentic and valid Eosb for furhter Cryptographic Operations
            // Just the presence of the 64bit hash of the strucutred Buffer is sufficient to conclude that
            // the Eosb is valid.
            ::pthread_mutex_lock(&m_sEosbCacheMutex);
            m_stlCachedKeysForEosb.insert(std::make_pair<Qword, std::vector<Byte>>(oStructuredBufferEosbWithoutKey.Get64BitHash(), std::move(stlAccountKey)));
            ::pthread_mutex_unlock(&m_sEosbCacheMutex);

            oResponseEosb.PutStructuredBuffer("Eosb", oStructuredBufferEosbWithoutKey);
            dwStatus = 201;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponseEosb.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseEosb.Clear();
    }

    // Send back the status of the transaction
    oResponseEosb.PutDword("Status", dwStatus);

    return oResponseEosb.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function UnregisterEosb
 * @brief Add the AES key to the Ssb and Encrypt it with Current Eosb Key.
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer
 * @note
 *      The RequiredParameters StructuredBuffer should have
 *        +-----------------------------------------------------------------------------------+
 *        | ["Eosb":Buffer] {Required} Encrypted Eosb to register                             |
 *        +-----------------------------------------------------------------------------------+
 *
 *      The Return StructuredBuffer is the decrypted Digital Contract without the AES accont
 *      key as defined in the CreateEosb key
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::UnregisterEosb(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponseEosb;

    Dword dwStatus = 404;

    StructuredBuffer oStructuredBufferEosbWithoutKey = c_oStructuredBufferRequest.GetStructuredBuffer("Eosb");

    try
    {
        ::pthread_mutex_lock(&m_sEosbCacheMutex);
        oStructuredBufferEosbWithoutKey.PutBuffer("AccountKey", m_stlCachedKeysForEosb.at(oStructuredBufferEosbWithoutKey.Get64BitHash()));
        m_stlCachedKeysForEosb.erase(oStructuredBufferEosbWithoutKey.Get64BitHash());
        ::pthread_mutex_unlock(&m_sEosbCacheMutex);
        oResponseEosb.PutBuffer("Eosb", this->CreateEosbFromPlainSsb(oStructuredBufferEosbWithoutKey.GetSerializedBuffer()));
        dwStatus = 201;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponseEosb.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseEosb.Clear();
        dwStatus = 500;
    }

    // Send back status of the transaction
    oResponseEosb.PutDword("Status", dwStatus);

    return oResponseEosb.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function UpdateUserAccessRights
 * @brief Decrypt the Eosb, update the user access rights and return the encrypted Eosb
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized buffer containing the updated Eosb
 * @note For internal user by plugins only
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::UpdateUserAccessRights(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::vector<Byte> stlEncryptedEsob = c_oRequest.GetBuffer("Eosb");
        // Get plain text Eosb
        StructuredBuffer oPlainTextEosb(this->GetPlainTextSsbFromEosb(stlEncryptedEsob));
        // Update the user access rights
        oPlainTextEosb.PutQword("UserAccessRights", c_oRequest.GetQword("UserAccessRights"));
        // Encrypt the updated Eosb
        std::vector<Byte> stlUpdatedEosb = this->CreateEosbFromPlainSsb(oPlainTextEosb.GetSerializedBuffer());
        // Add Updated Eosb to the response
        oResponse.PutBuffer("UpdatedEosb", stlUpdatedEosb);
        dwStatus = 200;
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
        dwStatus = 500;
    }

    // Add status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function UpdateUserInformation
 * @brief Decrypt the Eosb, update the user information and return the encrypted Eosb
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized buffer containing the updated Eosb
 * @note For internal use by plugins only
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::UpdateUserInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::vector<Byte> stlEncryptedEsob = c_oRequest.GetBuffer("Eosb");
        // Get plain text Eosb
        StructuredBuffer oPlainTextEosb(this->GetPlainTextSsbFromEosb(stlEncryptedEsob));
        // Update the user information
        oPlainTextEosb.PutString("Username", c_oRequest.GetString("Username"));
        oPlainTextEosb.PutString("Title", c_oRequest.GetString("Title"));
        oPlainTextEosb.PutString("PhoneNumber", c_oRequest.GetString("PhoneNumber"));
        // Encrypt the updated Eosb
        std::vector<Byte> stlUpdatedEosb = this->CreateEosbFromPlainSsb(oPlainTextEosb.GetSerializedBuffer());
        // Add Updated Eosb to the response
        oResponse.PutBuffer("UpdatedEosb", stlUpdatedEosb);
        dwStatus = 200;
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
        dwStatus = 500;
    }

    // Add status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function ModifyEosbAccessRights
 * @brief Decrypt the Eosb, update the access rights, and return the encrypted Eosb
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized buffer containing the modified Eosb
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::ModifyEosbAccessRights(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::vector<Byte> stlEncryptedEsob = c_oRequest.GetBuffer("Eosb");
        // Get plain text Eosb
        StructuredBuffer oPlainTextEosb(this->GetPlainTextSsbFromEosb(stlEncryptedEsob));
        // Update the access rights
        oPlainTextEosb.PutQword("AccessRights", c_oRequest.GetQword("AccessRights"));
        // Encrypt the updated Eosb
        std::vector<Byte> stlUpdatedEosb = this->CreateEosbFromPlainSsb(oPlainTextEosb.GetSerializedBuffer());
        // Add Updated Eosb to the response
        oResponse.PutBuffer("UpdatedEosb", stlUpdatedEosb);
        dwStatus = 200;
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
        dwStatus = 500;
    }

    // Add status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CheckEosbAccessRights
 * @brief Decrypt the Eosb, check if the Eosb access rights are the same as the request access rights
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns true if access rights are the same
 * @return false otherwise
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::CheckEosbAccessRights(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::vector<Byte> stlEncryptedEsob = c_oRequest.GetBuffer("Eosb");
        // Get plain text Eosb
        StructuredBuffer oPlainTextEosb(this->GetPlainTextSsbFromEosb(stlEncryptedEsob));
        // Compare the Eosb access rights with the request access rights bitmap
        bool fEqual = false;
        if (c_oRequest.GetQword("AccessRights") == oPlainTextEosb.GetQword("AccessRight"))
        {
            fEqual = true;
        }
        // Add the bool and status to the response
        oResponse.PutBoolean("IsEqual", fEqual);
        dwStatus = 200;
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
        dwStatus = 500;
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function SignMessageDigest
 * @brief Add the AES key to the Ssb and Encrypt it with Current Eosb Key.
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer
 * @note
 *      The RequiredParameters StructuredBuffer should have
 *        +-----------------------------------------------------------------------------------+
 *        | ["Eosb":Buffer] {Required} Encrypted Eosb to register                             |
 *        +-----------------------------------------------------------------------------------+
 *        | ["MessageDigest":Buffer] {Required} Message Digest to sign                        |
 *        +-----------------------------------------------------------------------------------+
 *
 *      The Return StructuredBuffer is the decrypted Digital Contract without the AES accont
 *      key as defined in the CreateEosb key
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::SignMessageDigest(
    _in const StructuredBuffer & c_oStructuredBufferRequest
)
{
    __DebugFunction();

    StructuredBuffer oResponseSignature;

    Dword dwStatus = 400;

    try
    {
        // TODO: rmeove this later and don't use a hard coded key
        Guid oSignKeyGuid = "609A09889F32452393D8CDB97C4A41B4";
        struct stat buffer;
        if (0 != stat("609A09889F32452393D8CDB97C4A41B4.key", &buffer))
        {
            std::vector<Byte> c_stlSignKeyFile = {
                0xCD, 0xAB, 0x9C, 0xE5, 0xBC, 0xC6, 0xCA, 0x2E, 0x4E, 0x48, 0xBA, 0x28, 0x90, 0x48, 0xB8, 0x72,
                0xCF, 0xE8, 0xFE, 0x59, 0x03, 0x00, 0x00, 0x00, 0x83, 0x63, 0x74, 0x83, 0xCD, 0x06, 0x00, 0x00,
                0x34, 0x01, 0xFB, 0x54, 0x03, 0x0B, 0x00, 0x00, 0x00, 0xA9, 0x06, 0x00, 0x00, 0x0D, 0x02, 0x0F,
                0x01, 0x50, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65, 0x4B, 0x65, 0x79, 0x00, 0x0B, 0x04, 0x0C, 0x03,
                0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20, 0x50, 0x52, 0x49, 0x56, 0x41,
                0x54, 0x45, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x4D, 0x49, 0x49, 0x45,
                0x76, 0x51, 0x49, 0x42, 0x41, 0x44, 0x41, 0x4E, 0x42, 0x67, 0x6B, 0x71, 0x68, 0x6B, 0x69, 0x47,
                0x39, 0x77, 0x30, 0x42, 0x41, 0x51, 0x45, 0x46, 0x41, 0x41, 0x53, 0x43, 0x42, 0x4B, 0x63, 0x77,
                0x67, 0x67, 0x53, 0x6A, 0x41, 0x67, 0x45, 0x41, 0x41, 0x6F, 0x49, 0x42, 0x41, 0x51, 0x43, 0x73,
                0x66, 0x41, 0x48, 0x30, 0x6A, 0x75, 0x42, 0x50, 0x45, 0x68, 0x55, 0x75, 0x0A, 0x4A, 0x73, 0x71,
                0x76, 0x6B, 0x57, 0x71, 0x58, 0x30, 0x6F, 0x55, 0x68, 0x33, 0x31, 0x65, 0x41, 0x33, 0x4D, 0x46,
                0x54, 0x6B, 0x67, 0x6B, 0x54, 0x4B, 0x39, 0x4D, 0x4C, 0x58, 0x77, 0x6A, 0x66, 0x65, 0x73, 0x30,
                0x61, 0x63, 0x41, 0x57, 0x44, 0x69, 0x75, 0x43, 0x6F, 0x77, 0x41, 0x6C, 0x73, 0x53, 0x44, 0x5A,
                0x47, 0x2F, 0x74, 0x63, 0x35, 0x76, 0x6C, 0x58, 0x6B, 0x64, 0x78, 0x51, 0x38, 0x0A, 0x78, 0x41,
                0x59, 0x4E, 0x4F, 0x65, 0x74, 0x4A, 0x6E, 0x56, 0x43, 0x37, 0x35, 0x43, 0x49, 0x58, 0x69, 0x49,
                0x65, 0x58, 0x51, 0x67, 0x4C, 0x70, 0x78, 0x44, 0x71, 0x4A, 0x4B, 0x78, 0x61, 0x55, 0x32, 0x6D,
                0x34, 0x42, 0x56, 0x53, 0x67, 0x4D, 0x55, 0x51, 0x34, 0x4A, 0x6A, 0x44, 0x68, 0x5A, 0x68, 0x44,
                0x42, 0x6A, 0x76, 0x35, 0x77, 0x41, 0x6A, 0x59, 0x4E, 0x39, 0x59, 0x37, 0x55, 0x43, 0x0A, 0x6B,
                0x66, 0x6E, 0x30, 0x59, 0x4C, 0x49, 0x78, 0x48, 0x4A, 0x63, 0x47, 0x57, 0x6B, 0x66, 0x73, 0x71,
                0x6D, 0x31, 0x70, 0x37, 0x50, 0x58, 0x72, 0x46, 0x53, 0x32, 0x41, 0x76, 0x41, 0x48, 0x47, 0x34,
                0x75, 0x56, 0x78, 0x32, 0x39, 0x42, 0x62, 0x2F, 0x33, 0x4B, 0x63, 0x4A, 0x37, 0x51, 0x33, 0x70,
                0x70, 0x46, 0x78, 0x39, 0x63, 0x4B, 0x72, 0x46, 0x64, 0x65, 0x44, 0x4D, 0x37, 0x33, 0x32, 0x0A,
                0x67, 0x44, 0x6B, 0x57, 0x36, 0x46, 0x66, 0x42, 0x65, 0x4E, 0x45, 0x78, 0x77, 0x61, 0x49, 0x51,
                0x6F, 0x2B, 0x75, 0x4B, 0x71, 0x52, 0x62, 0x7A, 0x55, 0x53, 0x63, 0x64, 0x31, 0x6A, 0x54, 0x52,
                0x77, 0x44, 0x44, 0x64, 0x58, 0x54, 0x47, 0x39, 0x33, 0x64, 0x65, 0x59, 0x61, 0x65, 0x5A, 0x2F,
                0x75, 0x4B, 0x6F, 0x77, 0x5A, 0x62, 0x38, 0x6B, 0x54, 0x36, 0x43, 0x78, 0x35, 0x2F, 0x38, 0x65,
                0x0A, 0x75, 0x53, 0x36, 0x36, 0x35, 0x79, 0x49, 0x42, 0x79, 0x31, 0x51, 0x4A, 0x6D, 0x34, 0x5A,
                0x44, 0x6C, 0x74, 0x34, 0x42, 0x48, 0x43, 0x63, 0x6E, 0x47, 0x49, 0x53, 0x30, 0x42, 0x41, 0x67,
                0x32, 0x61, 0x70, 0x73, 0x42, 0x33, 0x61, 0x43, 0x6E, 0x4F, 0x75, 0x5A, 0x64, 0x49, 0x49, 0x4A,
                0x6C, 0x45, 0x54, 0x34, 0x47, 0x39, 0x35, 0x2B, 0x58, 0x65, 0x30, 0x48, 0x47, 0x4D, 0x74, 0x4F,
                0x66, 0x0A, 0x77, 0x50, 0x53, 0x44, 0x55, 0x79, 0x36, 0x48, 0x41, 0x67, 0x4D, 0x42, 0x41, 0x41,
                0x45, 0x43, 0x67, 0x67, 0x45, 0x41, 0x59, 0x33, 0x53, 0x5A, 0x75, 0x41, 0x64, 0x4F, 0x5A, 0x61,
                0x6C, 0x36, 0x6C, 0x77, 0x4C, 0x39, 0x4B, 0x69, 0x45, 0x43, 0x51, 0x6F, 0x71, 0x38, 0x41, 0x31,
                0x30, 0x42, 0x32, 0x71, 0x62, 0x76, 0x76, 0x70, 0x31, 0x64, 0x4B, 0x44, 0x58, 0x44, 0x32, 0x6A,
                0x47, 0x78, 0x0A, 0x4C, 0x78, 0x4E, 0x6A, 0x41, 0x70, 0x53, 0x48, 0x53, 0x57, 0x4D, 0x68, 0x79,
                0x59, 0x76, 0x35, 0x6F, 0x76, 0x73, 0x38, 0x65, 0x4E, 0x46, 0x6E, 0x44, 0x72, 0x2B, 0x6D, 0x54,
                0x54, 0x63, 0x77, 0x62, 0x78, 0x31, 0x6B, 0x6B, 0x2B, 0x6E, 0x4B, 0x74, 0x73, 0x62, 0x45, 0x44,
                0x36, 0x4D, 0x71, 0x32, 0x55, 0x44, 0x37, 0x56, 0x67, 0x64, 0x56, 0x49, 0x4B, 0x2F, 0x71, 0x52,
                0x4C, 0x2B, 0x59, 0x0A, 0x70, 0x4C, 0x6F, 0x53, 0x5A, 0x69, 0x68, 0x52, 0x45, 0x46, 0x45, 0x49,
                0x64, 0x41, 0x37, 0x72, 0x53, 0x77, 0x71, 0x43, 0x30, 0x48, 0x46, 0x69, 0x70, 0x5A, 0x72, 0x7A,
                0x72, 0x37, 0x42, 0x33, 0x5A, 0x67, 0x55, 0x41, 0x43, 0x4A, 0x35, 0x64, 0x41, 0x52, 0x36, 0x49,
                0x49, 0x72, 0x79, 0x5A, 0x37, 0x6D, 0x67, 0x53, 0x38, 0x4E, 0x6F, 0x57, 0x72, 0x72, 0x65, 0x6D,
                0x5A, 0x69, 0x79, 0x50, 0x0A, 0x4E, 0x52, 0x71, 0x5A, 0x67, 0x74, 0x6C, 0x61, 0x78, 0x35, 0x52,
                0x45, 0x62, 0x31, 0x49, 0x2B, 0x36, 0x72, 0x33, 0x50, 0x2F, 0x5A, 0x31, 0x67, 0x70, 0x45, 0x48,
                0x6E, 0x34, 0x61, 0x66, 0x51, 0x76, 0x70, 0x6E, 0x38, 0x67, 0x72, 0x51, 0x39, 0x61, 0x6D, 0x48,
                0x61, 0x6A, 0x55, 0x50, 0x39, 0x6B, 0x4B, 0x4D, 0x6B, 0x43, 0x51, 0x64, 0x6A, 0x39, 0x6C, 0x77,
                0x46, 0x34, 0x4B, 0x69, 0x2B, 0x0A, 0x6D, 0x31, 0x6E, 0x73, 0x6F, 0x2F, 0x61, 0x74, 0x34, 0x39,
                0x6B, 0x67, 0x5A, 0x73, 0x4B, 0x59, 0x35, 0x64, 0x39, 0x30, 0x52, 0x61, 0x6A, 0x52, 0x65, 0x31,
                0x55, 0x41, 0x4A, 0x38, 0x6C, 0x58, 0x36, 0x32, 0x63, 0x63, 0x52, 0x39, 0x2B, 0x2F, 0x75, 0x36,
                0x37, 0x6D, 0x6D, 0x47, 0x70, 0x6B, 0x65, 0x57, 0x69, 0x4B, 0x71, 0x7A, 0x46, 0x6C, 0x55, 0x53,
                0x44, 0x68, 0x59, 0x48, 0x79, 0x4F, 0x0A, 0x33, 0x79, 0x51, 0x5A, 0x72, 0x46, 0x63, 0x76, 0x6C,
                0x73, 0x32, 0x4D, 0x73, 0x70, 0x5A, 0x55, 0x78, 0x63, 0x46, 0x68, 0x39, 0x4B, 0x4B, 0x4B, 0x42,
                0x74, 0x4D, 0x2F, 0x50, 0x32, 0x69, 0x65, 0x33, 0x69, 0x72, 0x4B, 0x6A, 0x55, 0x75, 0x51, 0x71,
                0x51, 0x4B, 0x42, 0x67, 0x51, 0x44, 0x68, 0x6A, 0x72, 0x34, 0x34, 0x33, 0x4B, 0x61, 0x49, 0x42,
                0x4E, 0x6C, 0x76, 0x38, 0x6A, 0x55, 0x4F, 0x0A, 0x7A, 0x37, 0x59, 0x76, 0x2F, 0x56, 0x50, 0x75,
                0x37, 0x63, 0x31, 0x78, 0x4F, 0x53, 0x47, 0x51, 0x72, 0x64, 0x39, 0x48, 0x6A, 0x78, 0x4F, 0x6F,
                0x74, 0x61, 0x41, 0x4E, 0x57, 0x4A, 0x56, 0x4C, 0x7A, 0x48, 0x71, 0x52, 0x4E, 0x6E, 0x56, 0x34,
                0x6C, 0x4E, 0x61, 0x55, 0x45, 0x67, 0x4B, 0x75, 0x66, 0x7A, 0x30, 0x4A, 0x6C, 0x54, 0x77, 0x6E,
                0x6A, 0x59, 0x2F, 0x43, 0x68, 0x63, 0x55, 0x46, 0x0A, 0x4F, 0x58, 0x7A, 0x4A, 0x30, 0x5A, 0x4F,
                0x35, 0x31, 0x4B, 0x4F, 0x6E, 0x76, 0x38, 0x58, 0x41, 0x76, 0x50, 0x50, 0x58, 0x54, 0x42, 0x2F,
                0x62, 0x55, 0x48, 0x72, 0x41, 0x2F, 0x71, 0x37, 0x58, 0x31, 0x72, 0x4B, 0x50, 0x45, 0x2F, 0x48,
                0x61, 0x6D, 0x49, 0x6E, 0x2B, 0x78, 0x75, 0x77, 0x2F, 0x44, 0x66, 0x52, 0x41, 0x57, 0x71, 0x7A,
                0x67, 0x68, 0x73, 0x51, 0x37, 0x77, 0x7A, 0x44, 0x41, 0x0A, 0x4C, 0x39, 0x38, 0x6A, 0x57, 0x4E,
                0x6E, 0x64, 0x79, 0x63, 0x74, 0x70, 0x4B, 0x4B, 0x4F, 0x42, 0x61, 0x46, 0x44, 0x2F, 0x46, 0x49,
                0x65, 0x62, 0x61, 0x77, 0x4B, 0x42, 0x67, 0x51, 0x44, 0x44, 0x77, 0x34, 0x64, 0x76, 0x54, 0x38,
                0x56, 0x57, 0x55, 0x51, 0x74, 0x63, 0x44, 0x6A, 0x30, 0x30, 0x52, 0x63, 0x77, 0x73, 0x53, 0x79,
                0x53, 0x53, 0x69, 0x61, 0x61, 0x6C, 0x46, 0x32, 0x38, 0x5A, 0x0A, 0x63, 0x37, 0x6B, 0x4C, 0x31,
                0x6F, 0x45, 0x58, 0x79, 0x65, 0x70, 0x33, 0x6C, 0x6D, 0x4D, 0x4D, 0x64, 0x45, 0x76, 0x4B, 0x4A,
                0x5A, 0x2F, 0x56, 0x65, 0x57, 0x54, 0x52, 0x37, 0x41, 0x35, 0x2B, 0x6D, 0x77, 0x58, 0x44, 0x32,
                0x70, 0x49, 0x43, 0x2B, 0x39, 0x37, 0x66, 0x41, 0x62, 0x4E, 0x2B, 0x71, 0x7A, 0x37, 0x56, 0x2F,
                0x62, 0x77, 0x71, 0x56, 0x75, 0x49, 0x66, 0x79, 0x51, 0x58, 0x68, 0x0A, 0x69, 0x69, 0x6A, 0x30,
                0x6A, 0x30, 0x4D, 0x42, 0x37, 0x42, 0x51, 0x78, 0x46, 0x72, 0x45, 0x64, 0x51, 0x63, 0x43, 0x42,
                0x4F, 0x4D, 0x67, 0x44, 0x41, 0x6F, 0x48, 0x4D, 0x47, 0x32, 0x64, 0x50, 0x42, 0x37, 0x69, 0x33,
                0x7A, 0x49, 0x54, 0x47, 0x53, 0x71, 0x6B, 0x6A, 0x74, 0x74, 0x4B, 0x5A, 0x55, 0x73, 0x64, 0x61,
                0x73, 0x52, 0x37, 0x70, 0x4E, 0x76, 0x79, 0x36, 0x6E, 0x74, 0x6B, 0x50, 0x0A, 0x4B, 0x39, 0x4A,
                0x4B, 0x75, 0x50, 0x36, 0x38, 0x56, 0x51, 0x4B, 0x42, 0x67, 0x48, 0x72, 0x69, 0x6F, 0x74, 0x34,
                0x6C, 0x30, 0x4A, 0x44, 0x41, 0x57, 0x35, 0x6F, 0x79, 0x30, 0x68, 0x64, 0x4D, 0x73, 0x4D, 0x78,
                0x36, 0x62, 0x74, 0x54, 0x34, 0x4E, 0x2F, 0x70, 0x63, 0x6A, 0x74, 0x33, 0x63, 0x37, 0x49, 0x31,
                0x32, 0x39, 0x4E, 0x58, 0x77, 0x68, 0x63, 0x39, 0x66, 0x6D, 0x49, 0x38, 0x67, 0x0A, 0x53, 0x78,
                0x4A, 0x38, 0x49, 0x67, 0x6E, 0x63, 0x78, 0x50, 0x38, 0x4D, 0x78, 0x4F, 0x73, 0x2B, 0x70, 0x51,
                0x6C, 0x6F, 0x68, 0x63, 0x70, 0x69, 0x78, 0x4A, 0x54, 0x4D, 0x2F, 0x6D, 0x62, 0x33, 0x52, 0x2F,
                0x5A, 0x49, 0x4F, 0x50, 0x79, 0x41, 0x54, 0x64, 0x30, 0x68, 0x6A, 0x4F, 0x4B, 0x52, 0x7A, 0x79,
                0x4E, 0x58, 0x39, 0x4F, 0x38, 0x75, 0x62, 0x49, 0x4A, 0x69, 0x2F, 0x75, 0x7A, 0x57, 0x0A, 0x71,
                0x70, 0x64, 0x71, 0x33, 0x56, 0x51, 0x6A, 0x30, 0x4F, 0x4B, 0x2B, 0x69, 0x34, 0x31, 0x61, 0x30,
                0x39, 0x79, 0x50, 0x79, 0x69, 0x6E, 0x56, 0x49, 0x6D, 0x6F, 0x51, 0x30, 0x4B, 0x6D, 0x55, 0x4B,
                0x7A, 0x52, 0x39, 0x76, 0x73, 0x79, 0x6E, 0x6D, 0x53, 0x75, 0x6B, 0x6D, 0x56, 0x66, 0x79, 0x64,
                0x6C, 0x5A, 0x53, 0x59, 0x41, 0x4D, 0x62, 0x41, 0x6F, 0x47, 0x41, 0x4A, 0x44, 0x48, 0x42, 0x0A,
                0x6C, 0x36, 0x59, 0x44, 0x45, 0x56, 0x74, 0x34, 0x6F, 0x7A, 0x4F, 0x4B, 0x77, 0x32, 0x42, 0x6F,
                0x53, 0x49, 0x51, 0x34, 0x4A, 0x64, 0x50, 0x48, 0x65, 0x4E, 0x79, 0x6C, 0x55, 0x4F, 0x70, 0x68,
                0x65, 0x7A, 0x76, 0x55, 0x73, 0x47, 0x5A, 0x48, 0x73, 0x49, 0x61, 0x67, 0x6C, 0x34, 0x76, 0x6B,
                0x2B, 0x32, 0x49, 0x4C, 0x64, 0x44, 0x65, 0x32, 0x6D, 0x55, 0x31, 0x77, 0x76, 0x4A, 0x33, 0x43,
                0x0A, 0x36, 0x68, 0x77, 0x2B, 0x52, 0x4D, 0x6A, 0x32, 0x77, 0x76, 0x71, 0x59, 0x69, 0x6B, 0x48,
                0x31, 0x49, 0x7A, 0x46, 0x2F, 0x6D, 0x63, 0x42, 0x6F, 0x6E, 0x49, 0x48, 0x5A, 0x55, 0x59, 0x4C,
                0x75, 0x34, 0x5A, 0x43, 0x66, 0x54, 0x66, 0x71, 0x74, 0x48, 0x51, 0x70, 0x61, 0x41, 0x74, 0x50,
                0x47, 0x57, 0x4E, 0x33, 0x37, 0x74, 0x50, 0x72, 0x41, 0x72, 0x74, 0x45, 0x65, 0x50, 0x37, 0x46,
                0x6F, 0x0A, 0x79, 0x4B, 0x72, 0x4B, 0x6E, 0x62, 0x4C, 0x75, 0x4A, 0x4D, 0x6C, 0x48, 0x6C, 0x4D,
                0x75, 0x47, 0x53, 0x6C, 0x46, 0x58, 0x4B, 0x72, 0x31, 0x63, 0x36, 0x74, 0x41, 0x31, 0x78, 0x36,
                0x56, 0x4E, 0x67, 0x7A, 0x31, 0x43, 0x38, 0x2B, 0x55, 0x43, 0x67, 0x59, 0x45, 0x41, 0x30, 0x31,
                0x36, 0x39, 0x49, 0x55, 0x2F, 0x73, 0x54, 0x4C, 0x70, 0x72, 0x32, 0x64, 0x62, 0x73, 0x62, 0x33,
                0x34, 0x6F, 0x0A, 0x30, 0x6A, 0x50, 0x36, 0x32, 0x75, 0x6A, 0x38, 0x58, 0x41, 0x46, 0x54, 0x53,
                0x47, 0x6C, 0x38, 0x45, 0x48, 0x5A, 0x72, 0x71, 0x53, 0x31, 0x55, 0x51, 0x49, 0x61, 0x75, 0x73,
                0x39, 0x7A, 0x49, 0x42, 0x30, 0x62, 0x55, 0x62, 0x58, 0x58, 0x34, 0x7A, 0x2B, 0x34, 0x46, 0x54,
                0x2B, 0x59, 0x57, 0x35, 0x67, 0x72, 0x6B, 0x79, 0x73, 0x57, 0x4A, 0x34, 0x77, 0x55, 0x72, 0x59,
                0x32, 0x6B, 0x61, 0x0A, 0x38, 0x46, 0x36, 0x32, 0x43, 0x6D, 0x65, 0x6D, 0x44, 0x35, 0x42, 0x30,
                0x58, 0x66, 0x4E, 0x36, 0x63, 0x4A, 0x35, 0x6C, 0x48, 0x35, 0x78, 0x69, 0x43, 0x6D, 0x75, 0x4A,
                0x54, 0x64, 0x2B, 0x32, 0x79, 0x48, 0x79, 0x36, 0x66, 0x78, 0x49, 0x66, 0x42, 0x66, 0x55, 0x4F,
                0x7A, 0x39, 0x31, 0x48, 0x2F, 0x32, 0x67, 0x7A, 0x5A, 0x53, 0x4B, 0x50, 0x47, 0x65, 0x6F, 0x53,
                0x42, 0x63, 0x6C, 0x63, 0x0A, 0x47, 0x35, 0x69, 0x6F, 0x4A, 0x7A, 0x58, 0x2B, 0x76, 0x69, 0x72,
                0x78, 0x47, 0x73, 0x70, 0x37, 0x69, 0x4D, 0x50, 0x30, 0x67, 0x7A, 0x51, 0x3D, 0x0A, 0x2D, 0x2D,
                0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x50, 0x52, 0x49, 0x56, 0x41, 0x54, 0x45, 0x20, 0x4B,
                0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63, 0x74,
                0x83, 0x29, 0x00, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x4B, 0x65, 0x79, 0x53, 0x70, 0x65, 0x63, 0x00, 0x0B, 0x04,
                0x0C, 0x03, 0x52, 0x53, 0x41, 0x32, 0x30, 0x34, 0x38, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63,
                0x74, 0x83, 0xE7, 0x01, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x0A, 0x00, 0x00, 0x00, 0xC4,
                0x01, 0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x50, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x4B, 0x65, 0x79,
                0x00, 0x0B, 0x04, 0x0C, 0x03, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20,
                0x50, 0x55, 0x42, 0x4C, 0x49, 0x43, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A,
                0x4D, 0x49, 0x49, 0x42, 0x49, 0x6A, 0x41, 0x4E, 0x42, 0x67, 0x6B, 0x71, 0x68, 0x6B, 0x69, 0x47,
                0x39, 0x77, 0x30, 0x42, 0x41, 0x51, 0x45, 0x46, 0x41, 0x41, 0x4F, 0x43, 0x41, 0x51, 0x38, 0x41,
                0x4D, 0x49, 0x49, 0x42, 0x43, 0x67, 0x4B, 0x43, 0x41, 0x51, 0x45, 0x41, 0x72, 0x48, 0x77, 0x42,
                0x39, 0x49, 0x37, 0x67, 0x54, 0x78, 0x49, 0x56, 0x4C, 0x69, 0x62, 0x4B, 0x72, 0x35, 0x46, 0x71,
                0x0A, 0x6C, 0x39, 0x4B, 0x46, 0x49, 0x64, 0x39, 0x58, 0x67, 0x4E, 0x7A, 0x42, 0x55, 0x35, 0x49,
                0x4A, 0x45, 0x79, 0x76, 0x54, 0x43, 0x31, 0x38, 0x49, 0x33, 0x33, 0x72, 0x4E, 0x47, 0x6E, 0x41,
                0x46, 0x67, 0x34, 0x72, 0x67, 0x71, 0x4D, 0x41, 0x4A, 0x62, 0x45, 0x67, 0x32, 0x52, 0x76, 0x37,
                0x58, 0x4F, 0x62, 0x35, 0x56, 0x35, 0x48, 0x63, 0x55, 0x50, 0x4D, 0x51, 0x47, 0x44, 0x54, 0x6E,
                0x72, 0x0A, 0x53, 0x5A, 0x31, 0x51, 0x75, 0x2B, 0x51, 0x69, 0x46, 0x34, 0x69, 0x48, 0x6C, 0x30,
                0x49, 0x43, 0x36, 0x63, 0x51, 0x36, 0x69, 0x53, 0x73, 0x57, 0x6C, 0x4E, 0x70, 0x75, 0x41, 0x56,
                0x55, 0x6F, 0x44, 0x46, 0x45, 0x4F, 0x43, 0x59, 0x77, 0x34, 0x57, 0x59, 0x51, 0x77, 0x59, 0x37,
                0x2B, 0x63, 0x41, 0x49, 0x32, 0x44, 0x66, 0x57, 0x4F, 0x31, 0x41, 0x70, 0x48, 0x35, 0x39, 0x47,
                0x43, 0x79, 0x0A, 0x4D, 0x52, 0x79, 0x58, 0x42, 0x6C, 0x70, 0x48, 0x37, 0x4B, 0x70, 0x74, 0x61,
                0x65, 0x7A, 0x31, 0x36, 0x78, 0x55, 0x74, 0x67, 0x4C, 0x77, 0x42, 0x78, 0x75, 0x4C, 0x6C, 0x63,
                0x64, 0x76, 0x51, 0x57, 0x2F, 0x39, 0x79, 0x6E, 0x43, 0x65, 0x30, 0x4E, 0x36, 0x61, 0x52, 0x63,
                0x66, 0x58, 0x43, 0x71, 0x78, 0x58, 0x58, 0x67, 0x7A, 0x4F, 0x39, 0x39, 0x6F, 0x41, 0x35, 0x46,
                0x75, 0x68, 0x58, 0x0A, 0x77, 0x58, 0x6A, 0x52, 0x4D, 0x63, 0x47, 0x69, 0x45, 0x4B, 0x50, 0x72,
                0x69, 0x71, 0x6B, 0x57, 0x38, 0x31, 0x45, 0x6E, 0x48, 0x64, 0x59, 0x30, 0x30, 0x63, 0x41, 0x77,
                0x33, 0x56, 0x30, 0x78, 0x76, 0x64, 0x33, 0x58, 0x6D, 0x47, 0x6E, 0x6D, 0x66, 0x37, 0x69, 0x71,
                0x4D, 0x47, 0x57, 0x2F, 0x4A, 0x45, 0x2B, 0x67, 0x73, 0x65, 0x66, 0x2F, 0x48, 0x72, 0x6B, 0x75,
                0x75, 0x75, 0x63, 0x69, 0x0A, 0x41, 0x63, 0x74, 0x55, 0x43, 0x5A, 0x75, 0x47, 0x51, 0x35, 0x62,
                0x65, 0x41, 0x52, 0x77, 0x6E, 0x4A, 0x78, 0x69, 0x45, 0x74, 0x41, 0x51, 0x49, 0x4E, 0x6D, 0x71,
                0x62, 0x41, 0x64, 0x32, 0x67, 0x70, 0x7A, 0x72, 0x6D, 0x58, 0x53, 0x43, 0x43, 0x5A, 0x52, 0x45,
                0x2B, 0x42, 0x76, 0x65, 0x66, 0x6C, 0x33, 0x74, 0x42, 0x78, 0x6A, 0x4C, 0x54, 0x6E, 0x38, 0x44,
                0x30, 0x67, 0x31, 0x4D, 0x75, 0x0A, 0x68, 0x77, 0x49, 0x44, 0x41, 0x51, 0x41, 0x42, 0x0A, 0x2D,
                0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x50, 0x55, 0x42, 0x4C, 0x49, 0x43, 0x20, 0x4B,
                0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x5E, 0xC9, 0xBA,
                0xDC};

            std::ofstream oKeyFile("609A09889F32452393D8CDB97C4A41B4.key", std::ios::binary);
            oKeyFile.write((const char *)c_stlSignKeyFile.data(), c_stlSignKeyFile.size());
            oKeyFile.close();
        }

        StructuredBuffer oRegisterEosbResponse(this->RegisterEosb(c_oStructuredBufferRequest));
        if (201 == oRegisterEosbResponse.GetDword("Status"))
        {
            // TODO: Get the user Guid and then use the user guid to fetch the Data Owner key
            StructuredBuffer oDecryptedEosb = oRegisterEosbResponse.GetStructuredBuffer("Eosb");
            Guid oUserGuid = oDecryptedEosb.GetGuid("UserId");

            // Sign the message
            CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();
            std::vector<Byte> stlSignature = oCryptographicEngine.GenerateSignature(oSignKeyGuid, c_oStructuredBufferRequest.GetBuffer("MessageDigest"));

            // Unregister the Eosb
            StructuredBuffer oUnregisterEosbResponse(this->UnregisterEosb(oRegisterEosbResponse));
            if (201 == oUnregisterEosbResponse.GetDword("Status"))
            {
                oResponseSignature.PutBuffer("Eosb", oUnregisterEosbResponse.GetBuffer("Eosb"));
                oResponseSignature.PutBuffer("Signature", stlSignature);
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponseSignature.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseSignature.Clear();
    }

    // Send back status of the transaction
    oResponseSignature.PutDword("Status", dwStatus);

    return oResponseSignature.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function VerifySignature
 * @brief Add the AES key to the Ssb and Encrypt it with Current Eosb Key.
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer
 * @note
 *      The RequiredParameters StructuredBuffer should have
 *        +-----------------------------------------------------------------------------------+
 *        | ["Eosb":Buffer] {Required} Encrypted Eosb to register                             |
 *        +-----------------------------------------------------------------------------------+
 *        | ["MessageDigest":Buffer] {Required} Message Digest to sign                        |
 *        +-----------------------------------------------------------------------------------+
 *        | ["Signature":Buffer] {Required} Signature to verify                               |
 *        +-----------------------------------------------------------------------------------+
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::VerifySignature(
    _in const StructuredBuffer & c_oStructuredBufferRequest
)
{
    __DebugFunction();

    StructuredBuffer oResponseSignature;

    Dword dwStatus = 400;

    try
    {
        StructuredBuffer oRegisterEosbResponse(this->RegisterEosb(c_oStructuredBufferRequest));

        if (201 == oRegisterEosbResponse.GetDword("Status"))
        {
            // Get the user Guid and then use the user guid to fetch the user key to sign
            StructuredBuffer oDecryptedEosb = oRegisterEosbResponse.GetStructuredBuffer("Eosb");
            Guid oUserGuid = oDecryptedEosb.GetGuid("UserId");

            // Verify the signature
            CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();
            Guid oSignKeyGuid = "609A09889F32452393D8CDB97C4A41B4";
            bool fSignatureVerifyStatus = oCryptographicEngine.VerifySignature(oSignKeyGuid, c_oStructuredBufferRequest.GetBuffer("Signature"), c_oStructuredBufferRequest.GetBuffer("MessageDigest"));

            // Unregister the Eosb
            StructuredBuffer oUnregisterEosbResponse(this->UnregisterEosb(oRegisterEosbResponse));
            if (201 == oUnregisterEosbResponse.GetDword("Status"))
            {
                oResponseSignature.PutBuffer("Eosb", oUnregisterEosbResponse.GetBuffer("Eosb"));
                oResponseSignature.PutBoolean("Verify", fSignatureVerifyStatus);
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponseSignature.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseSignature.Clear();
    }

    // Send back status of the transaction
    oResponseSignature.PutDword("Status", dwStatus);

    return oResponseSignature.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CreateDigitalSignature
 * @brief Digitally sign given buffer with root key
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer containing signed buffer
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::CreateDigitalSignature(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    
    try 
    {
        // TODO: remove this later and don't use a hard coded key
        Guid oSignKeyGuid = "609A09889F32452393D8CDB97C4A41B4";
        struct stat buffer;
        if (0 != stat("609A09889F32452393D8CDB97C4A41B4.key", &buffer))
        {
            std::vector<Byte> c_stlSignKeyFile = {
                0xCD, 0xAB, 0x9C, 0xE5, 0xBC, 0xC6, 0xCA, 0x2E, 0x4E, 0x48, 0xBA, 0x28, 0x90, 0x48, 0xB8, 0x72,
                0xCF, 0xE8, 0xFE, 0x59, 0x03, 0x00, 0x00, 0x00, 0x83, 0x63, 0x74, 0x83, 0xCD, 0x06, 0x00, 0x00,
                0x34, 0x01, 0xFB, 0x54, 0x03, 0x0B, 0x00, 0x00, 0x00, 0xA9, 0x06, 0x00, 0x00, 0x0D, 0x02, 0x0F,
                0x01, 0x50, 0x72, 0x69, 0x76, 0x61, 0x74, 0x65, 0x4B, 0x65, 0x79, 0x00, 0x0B, 0x04, 0x0C, 0x03,
                0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20, 0x50, 0x52, 0x49, 0x56, 0x41,
                0x54, 0x45, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x4D, 0x49, 0x49, 0x45,
                0x76, 0x51, 0x49, 0x42, 0x41, 0x44, 0x41, 0x4E, 0x42, 0x67, 0x6B, 0x71, 0x68, 0x6B, 0x69, 0x47,
                0x39, 0x77, 0x30, 0x42, 0x41, 0x51, 0x45, 0x46, 0x41, 0x41, 0x53, 0x43, 0x42, 0x4B, 0x63, 0x77,
                0x67, 0x67, 0x53, 0x6A, 0x41, 0x67, 0x45, 0x41, 0x41, 0x6F, 0x49, 0x42, 0x41, 0x51, 0x43, 0x73,
                0x66, 0x41, 0x48, 0x30, 0x6A, 0x75, 0x42, 0x50, 0x45, 0x68, 0x55, 0x75, 0x0A, 0x4A, 0x73, 0x71,
                0x76, 0x6B, 0x57, 0x71, 0x58, 0x30, 0x6F, 0x55, 0x68, 0x33, 0x31, 0x65, 0x41, 0x33, 0x4D, 0x46,
                0x54, 0x6B, 0x67, 0x6B, 0x54, 0x4B, 0x39, 0x4D, 0x4C, 0x58, 0x77, 0x6A, 0x66, 0x65, 0x73, 0x30,
                0x61, 0x63, 0x41, 0x57, 0x44, 0x69, 0x75, 0x43, 0x6F, 0x77, 0x41, 0x6C, 0x73, 0x53, 0x44, 0x5A,
                0x47, 0x2F, 0x74, 0x63, 0x35, 0x76, 0x6C, 0x58, 0x6B, 0x64, 0x78, 0x51, 0x38, 0x0A, 0x78, 0x41,
                0x59, 0x4E, 0x4F, 0x65, 0x74, 0x4A, 0x6E, 0x56, 0x43, 0x37, 0x35, 0x43, 0x49, 0x58, 0x69, 0x49,
                0x65, 0x58, 0x51, 0x67, 0x4C, 0x70, 0x78, 0x44, 0x71, 0x4A, 0x4B, 0x78, 0x61, 0x55, 0x32, 0x6D,
                0x34, 0x42, 0x56, 0x53, 0x67, 0x4D, 0x55, 0x51, 0x34, 0x4A, 0x6A, 0x44, 0x68, 0x5A, 0x68, 0x44,
                0x42, 0x6A, 0x76, 0x35, 0x77, 0x41, 0x6A, 0x59, 0x4E, 0x39, 0x59, 0x37, 0x55, 0x43, 0x0A, 0x6B,
                0x66, 0x6E, 0x30, 0x59, 0x4C, 0x49, 0x78, 0x48, 0x4A, 0x63, 0x47, 0x57, 0x6B, 0x66, 0x73, 0x71,
                0x6D, 0x31, 0x70, 0x37, 0x50, 0x58, 0x72, 0x46, 0x53, 0x32, 0x41, 0x76, 0x41, 0x48, 0x47, 0x34,
                0x75, 0x56, 0x78, 0x32, 0x39, 0x42, 0x62, 0x2F, 0x33, 0x4B, 0x63, 0x4A, 0x37, 0x51, 0x33, 0x70,
                0x70, 0x46, 0x78, 0x39, 0x63, 0x4B, 0x72, 0x46, 0x64, 0x65, 0x44, 0x4D, 0x37, 0x33, 0x32, 0x0A,
                0x67, 0x44, 0x6B, 0x57, 0x36, 0x46, 0x66, 0x42, 0x65, 0x4E, 0x45, 0x78, 0x77, 0x61, 0x49, 0x51,
                0x6F, 0x2B, 0x75, 0x4B, 0x71, 0x52, 0x62, 0x7A, 0x55, 0x53, 0x63, 0x64, 0x31, 0x6A, 0x54, 0x52,
                0x77, 0x44, 0x44, 0x64, 0x58, 0x54, 0x47, 0x39, 0x33, 0x64, 0x65, 0x59, 0x61, 0x65, 0x5A, 0x2F,
                0x75, 0x4B, 0x6F, 0x77, 0x5A, 0x62, 0x38, 0x6B, 0x54, 0x36, 0x43, 0x78, 0x35, 0x2F, 0x38, 0x65,
                0x0A, 0x75, 0x53, 0x36, 0x36, 0x35, 0x79, 0x49, 0x42, 0x79, 0x31, 0x51, 0x4A, 0x6D, 0x34, 0x5A,
                0x44, 0x6C, 0x74, 0x34, 0x42, 0x48, 0x43, 0x63, 0x6E, 0x47, 0x49, 0x53, 0x30, 0x42, 0x41, 0x67,
                0x32, 0x61, 0x70, 0x73, 0x42, 0x33, 0x61, 0x43, 0x6E, 0x4F, 0x75, 0x5A, 0x64, 0x49, 0x49, 0x4A,
                0x6C, 0x45, 0x54, 0x34, 0x47, 0x39, 0x35, 0x2B, 0x58, 0x65, 0x30, 0x48, 0x47, 0x4D, 0x74, 0x4F,
                0x66, 0x0A, 0x77, 0x50, 0x53, 0x44, 0x55, 0x79, 0x36, 0x48, 0x41, 0x67, 0x4D, 0x42, 0x41, 0x41,
                0x45, 0x43, 0x67, 0x67, 0x45, 0x41, 0x59, 0x33, 0x53, 0x5A, 0x75, 0x41, 0x64, 0x4F, 0x5A, 0x61,
                0x6C, 0x36, 0x6C, 0x77, 0x4C, 0x39, 0x4B, 0x69, 0x45, 0x43, 0x51, 0x6F, 0x71, 0x38, 0x41, 0x31,
                0x30, 0x42, 0x32, 0x71, 0x62, 0x76, 0x76, 0x70, 0x31, 0x64, 0x4B, 0x44, 0x58, 0x44, 0x32, 0x6A,
                0x47, 0x78, 0x0A, 0x4C, 0x78, 0x4E, 0x6A, 0x41, 0x70, 0x53, 0x48, 0x53, 0x57, 0x4D, 0x68, 0x79,
                0x59, 0x76, 0x35, 0x6F, 0x76, 0x73, 0x38, 0x65, 0x4E, 0x46, 0x6E, 0x44, 0x72, 0x2B, 0x6D, 0x54,
                0x54, 0x63, 0x77, 0x62, 0x78, 0x31, 0x6B, 0x6B, 0x2B, 0x6E, 0x4B, 0x74, 0x73, 0x62, 0x45, 0x44,
                0x36, 0x4D, 0x71, 0x32, 0x55, 0x44, 0x37, 0x56, 0x67, 0x64, 0x56, 0x49, 0x4B, 0x2F, 0x71, 0x52,
                0x4C, 0x2B, 0x59, 0x0A, 0x70, 0x4C, 0x6F, 0x53, 0x5A, 0x69, 0x68, 0x52, 0x45, 0x46, 0x45, 0x49,
                0x64, 0x41, 0x37, 0x72, 0x53, 0x77, 0x71, 0x43, 0x30, 0x48, 0x46, 0x69, 0x70, 0x5A, 0x72, 0x7A,
                0x72, 0x37, 0x42, 0x33, 0x5A, 0x67, 0x55, 0x41, 0x43, 0x4A, 0x35, 0x64, 0x41, 0x52, 0x36, 0x49,
                0x49, 0x72, 0x79, 0x5A, 0x37, 0x6D, 0x67, 0x53, 0x38, 0x4E, 0x6F, 0x57, 0x72, 0x72, 0x65, 0x6D,
                0x5A, 0x69, 0x79, 0x50, 0x0A, 0x4E, 0x52, 0x71, 0x5A, 0x67, 0x74, 0x6C, 0x61, 0x78, 0x35, 0x52,
                0x45, 0x62, 0x31, 0x49, 0x2B, 0x36, 0x72, 0x33, 0x50, 0x2F, 0x5A, 0x31, 0x67, 0x70, 0x45, 0x48,
                0x6E, 0x34, 0x61, 0x66, 0x51, 0x76, 0x70, 0x6E, 0x38, 0x67, 0x72, 0x51, 0x39, 0x61, 0x6D, 0x48,
                0x61, 0x6A, 0x55, 0x50, 0x39, 0x6B, 0x4B, 0x4D, 0x6B, 0x43, 0x51, 0x64, 0x6A, 0x39, 0x6C, 0x77,
                0x46, 0x34, 0x4B, 0x69, 0x2B, 0x0A, 0x6D, 0x31, 0x6E, 0x73, 0x6F, 0x2F, 0x61, 0x74, 0x34, 0x39,
                0x6B, 0x67, 0x5A, 0x73, 0x4B, 0x59, 0x35, 0x64, 0x39, 0x30, 0x52, 0x61, 0x6A, 0x52, 0x65, 0x31,
                0x55, 0x41, 0x4A, 0x38, 0x6C, 0x58, 0x36, 0x32, 0x63, 0x63, 0x52, 0x39, 0x2B, 0x2F, 0x75, 0x36,
                0x37, 0x6D, 0x6D, 0x47, 0x70, 0x6B, 0x65, 0x57, 0x69, 0x4B, 0x71, 0x7A, 0x46, 0x6C, 0x55, 0x53,
                0x44, 0x68, 0x59, 0x48, 0x79, 0x4F, 0x0A, 0x33, 0x79, 0x51, 0x5A, 0x72, 0x46, 0x63, 0x76, 0x6C,
                0x73, 0x32, 0x4D, 0x73, 0x70, 0x5A, 0x55, 0x78, 0x63, 0x46, 0x68, 0x39, 0x4B, 0x4B, 0x4B, 0x42,
                0x74, 0x4D, 0x2F, 0x50, 0x32, 0x69, 0x65, 0x33, 0x69, 0x72, 0x4B, 0x6A, 0x55, 0x75, 0x51, 0x71,
                0x51, 0x4B, 0x42, 0x67, 0x51, 0x44, 0x68, 0x6A, 0x72, 0x34, 0x34, 0x33, 0x4B, 0x61, 0x49, 0x42,
                0x4E, 0x6C, 0x76, 0x38, 0x6A, 0x55, 0x4F, 0x0A, 0x7A, 0x37, 0x59, 0x76, 0x2F, 0x56, 0x50, 0x75,
                0x37, 0x63, 0x31, 0x78, 0x4F, 0x53, 0x47, 0x51, 0x72, 0x64, 0x39, 0x48, 0x6A, 0x78, 0x4F, 0x6F,
                0x74, 0x61, 0x41, 0x4E, 0x57, 0x4A, 0x56, 0x4C, 0x7A, 0x48, 0x71, 0x52, 0x4E, 0x6E, 0x56, 0x34,
                0x6C, 0x4E, 0x61, 0x55, 0x45, 0x67, 0x4B, 0x75, 0x66, 0x7A, 0x30, 0x4A, 0x6C, 0x54, 0x77, 0x6E,
                0x6A, 0x59, 0x2F, 0x43, 0x68, 0x63, 0x55, 0x46, 0x0A, 0x4F, 0x58, 0x7A, 0x4A, 0x30, 0x5A, 0x4F,
                0x35, 0x31, 0x4B, 0x4F, 0x6E, 0x76, 0x38, 0x58, 0x41, 0x76, 0x50, 0x50, 0x58, 0x54, 0x42, 0x2F,
                0x62, 0x55, 0x48, 0x72, 0x41, 0x2F, 0x71, 0x37, 0x58, 0x31, 0x72, 0x4B, 0x50, 0x45, 0x2F, 0x48,
                0x61, 0x6D, 0x49, 0x6E, 0x2B, 0x78, 0x75, 0x77, 0x2F, 0x44, 0x66, 0x52, 0x41, 0x57, 0x71, 0x7A,
                0x67, 0x68, 0x73, 0x51, 0x37, 0x77, 0x7A, 0x44, 0x41, 0x0A, 0x4C, 0x39, 0x38, 0x6A, 0x57, 0x4E,
                0x6E, 0x64, 0x79, 0x63, 0x74, 0x70, 0x4B, 0x4B, 0x4F, 0x42, 0x61, 0x46, 0x44, 0x2F, 0x46, 0x49,
                0x65, 0x62, 0x61, 0x77, 0x4B, 0x42, 0x67, 0x51, 0x44, 0x44, 0x77, 0x34, 0x64, 0x76, 0x54, 0x38,
                0x56, 0x57, 0x55, 0x51, 0x74, 0x63, 0x44, 0x6A, 0x30, 0x30, 0x52, 0x63, 0x77, 0x73, 0x53, 0x79,
                0x53, 0x53, 0x69, 0x61, 0x61, 0x6C, 0x46, 0x32, 0x38, 0x5A, 0x0A, 0x63, 0x37, 0x6B, 0x4C, 0x31,
                0x6F, 0x45, 0x58, 0x79, 0x65, 0x70, 0x33, 0x6C, 0x6D, 0x4D, 0x4D, 0x64, 0x45, 0x76, 0x4B, 0x4A,
                0x5A, 0x2F, 0x56, 0x65, 0x57, 0x54, 0x52, 0x37, 0x41, 0x35, 0x2B, 0x6D, 0x77, 0x58, 0x44, 0x32,
                0x70, 0x49, 0x43, 0x2B, 0x39, 0x37, 0x66, 0x41, 0x62, 0x4E, 0x2B, 0x71, 0x7A, 0x37, 0x56, 0x2F,
                0x62, 0x77, 0x71, 0x56, 0x75, 0x49, 0x66, 0x79, 0x51, 0x58, 0x68, 0x0A, 0x69, 0x69, 0x6A, 0x30,
                0x6A, 0x30, 0x4D, 0x42, 0x37, 0x42, 0x51, 0x78, 0x46, 0x72, 0x45, 0x64, 0x51, 0x63, 0x43, 0x42,
                0x4F, 0x4D, 0x67, 0x44, 0x41, 0x6F, 0x48, 0x4D, 0x47, 0x32, 0x64, 0x50, 0x42, 0x37, 0x69, 0x33,
                0x7A, 0x49, 0x54, 0x47, 0x53, 0x71, 0x6B, 0x6A, 0x74, 0x74, 0x4B, 0x5A, 0x55, 0x73, 0x64, 0x61,
                0x73, 0x52, 0x37, 0x70, 0x4E, 0x76, 0x79, 0x36, 0x6E, 0x74, 0x6B, 0x50, 0x0A, 0x4B, 0x39, 0x4A,
                0x4B, 0x75, 0x50, 0x36, 0x38, 0x56, 0x51, 0x4B, 0x42, 0x67, 0x48, 0x72, 0x69, 0x6F, 0x74, 0x34,
                0x6C, 0x30, 0x4A, 0x44, 0x41, 0x57, 0x35, 0x6F, 0x79, 0x30, 0x68, 0x64, 0x4D, 0x73, 0x4D, 0x78,
                0x36, 0x62, 0x74, 0x54, 0x34, 0x4E, 0x2F, 0x70, 0x63, 0x6A, 0x74, 0x33, 0x63, 0x37, 0x49, 0x31,
                0x32, 0x39, 0x4E, 0x58, 0x77, 0x68, 0x63, 0x39, 0x66, 0x6D, 0x49, 0x38, 0x67, 0x0A, 0x53, 0x78,
                0x4A, 0x38, 0x49, 0x67, 0x6E, 0x63, 0x78, 0x50, 0x38, 0x4D, 0x78, 0x4F, 0x73, 0x2B, 0x70, 0x51,
                0x6C, 0x6F, 0x68, 0x63, 0x70, 0x69, 0x78, 0x4A, 0x54, 0x4D, 0x2F, 0x6D, 0x62, 0x33, 0x52, 0x2F,
                0x5A, 0x49, 0x4F, 0x50, 0x79, 0x41, 0x54, 0x64, 0x30, 0x68, 0x6A, 0x4F, 0x4B, 0x52, 0x7A, 0x79,
                0x4E, 0x58, 0x39, 0x4F, 0x38, 0x75, 0x62, 0x49, 0x4A, 0x69, 0x2F, 0x75, 0x7A, 0x57, 0x0A, 0x71,
                0x70, 0x64, 0x71, 0x33, 0x56, 0x51, 0x6A, 0x30, 0x4F, 0x4B, 0x2B, 0x69, 0x34, 0x31, 0x61, 0x30,
                0x39, 0x79, 0x50, 0x79, 0x69, 0x6E, 0x56, 0x49, 0x6D, 0x6F, 0x51, 0x30, 0x4B, 0x6D, 0x55, 0x4B,
                0x7A, 0x52, 0x39, 0x76, 0x73, 0x79, 0x6E, 0x6D, 0x53, 0x75, 0x6B, 0x6D, 0x56, 0x66, 0x79, 0x64,
                0x6C, 0x5A, 0x53, 0x59, 0x41, 0x4D, 0x62, 0x41, 0x6F, 0x47, 0x41, 0x4A, 0x44, 0x48, 0x42, 0x0A,
                0x6C, 0x36, 0x59, 0x44, 0x45, 0x56, 0x74, 0x34, 0x6F, 0x7A, 0x4F, 0x4B, 0x77, 0x32, 0x42, 0x6F,
                0x53, 0x49, 0x51, 0x34, 0x4A, 0x64, 0x50, 0x48, 0x65, 0x4E, 0x79, 0x6C, 0x55, 0x4F, 0x70, 0x68,
                0x65, 0x7A, 0x76, 0x55, 0x73, 0x47, 0x5A, 0x48, 0x73, 0x49, 0x61, 0x67, 0x6C, 0x34, 0x76, 0x6B,
                0x2B, 0x32, 0x49, 0x4C, 0x64, 0x44, 0x65, 0x32, 0x6D, 0x55, 0x31, 0x77, 0x76, 0x4A, 0x33, 0x43,
                0x0A, 0x36, 0x68, 0x77, 0x2B, 0x52, 0x4D, 0x6A, 0x32, 0x77, 0x76, 0x71, 0x59, 0x69, 0x6B, 0x48,
                0x31, 0x49, 0x7A, 0x46, 0x2F, 0x6D, 0x63, 0x42, 0x6F, 0x6E, 0x49, 0x48, 0x5A, 0x55, 0x59, 0x4C,
                0x75, 0x34, 0x5A, 0x43, 0x66, 0x54, 0x66, 0x71, 0x74, 0x48, 0x51, 0x70, 0x61, 0x41, 0x74, 0x50,
                0x47, 0x57, 0x4E, 0x33, 0x37, 0x74, 0x50, 0x72, 0x41, 0x72, 0x74, 0x45, 0x65, 0x50, 0x37, 0x46,
                0x6F, 0x0A, 0x79, 0x4B, 0x72, 0x4B, 0x6E, 0x62, 0x4C, 0x75, 0x4A, 0x4D, 0x6C, 0x48, 0x6C, 0x4D,
                0x75, 0x47, 0x53, 0x6C, 0x46, 0x58, 0x4B, 0x72, 0x31, 0x63, 0x36, 0x74, 0x41, 0x31, 0x78, 0x36,
                0x56, 0x4E, 0x67, 0x7A, 0x31, 0x43, 0x38, 0x2B, 0x55, 0x43, 0x67, 0x59, 0x45, 0x41, 0x30, 0x31,
                0x36, 0x39, 0x49, 0x55, 0x2F, 0x73, 0x54, 0x4C, 0x70, 0x72, 0x32, 0x64, 0x62, 0x73, 0x62, 0x33,
                0x34, 0x6F, 0x0A, 0x30, 0x6A, 0x50, 0x36, 0x32, 0x75, 0x6A, 0x38, 0x58, 0x41, 0x46, 0x54, 0x53,
                0x47, 0x6C, 0x38, 0x45, 0x48, 0x5A, 0x72, 0x71, 0x53, 0x31, 0x55, 0x51, 0x49, 0x61, 0x75, 0x73,
                0x39, 0x7A, 0x49, 0x42, 0x30, 0x62, 0x55, 0x62, 0x58, 0x58, 0x34, 0x7A, 0x2B, 0x34, 0x46, 0x54,
                0x2B, 0x59, 0x57, 0x35, 0x67, 0x72, 0x6B, 0x79, 0x73, 0x57, 0x4A, 0x34, 0x77, 0x55, 0x72, 0x59,
                0x32, 0x6B, 0x61, 0x0A, 0x38, 0x46, 0x36, 0x32, 0x43, 0x6D, 0x65, 0x6D, 0x44, 0x35, 0x42, 0x30,
                0x58, 0x66, 0x4E, 0x36, 0x63, 0x4A, 0x35, 0x6C, 0x48, 0x35, 0x78, 0x69, 0x43, 0x6D, 0x75, 0x4A,
                0x54, 0x64, 0x2B, 0x32, 0x79, 0x48, 0x79, 0x36, 0x66, 0x78, 0x49, 0x66, 0x42, 0x66, 0x55, 0x4F,
                0x7A, 0x39, 0x31, 0x48, 0x2F, 0x32, 0x67, 0x7A, 0x5A, 0x53, 0x4B, 0x50, 0x47, 0x65, 0x6F, 0x53,
                0x42, 0x63, 0x6C, 0x63, 0x0A, 0x47, 0x35, 0x69, 0x6F, 0x4A, 0x7A, 0x58, 0x2B, 0x76, 0x69, 0x72,
                0x78, 0x47, 0x73, 0x70, 0x37, 0x69, 0x4D, 0x50, 0x30, 0x67, 0x7A, 0x51, 0x3D, 0x0A, 0x2D, 0x2D,
                0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x50, 0x52, 0x49, 0x56, 0x41, 0x54, 0x45, 0x20, 0x4B,
                0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63, 0x74,
                0x83, 0x29, 0x00, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x4B, 0x65, 0x79, 0x53, 0x70, 0x65, 0x63, 0x00, 0x0B, 0x04,
                0x0C, 0x03, 0x52, 0x53, 0x41, 0x32, 0x30, 0x34, 0x38, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63,
                0x74, 0x83, 0xE7, 0x01, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x0A, 0x00, 0x00, 0x00, 0xC4,
                0x01, 0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x50, 0x75, 0x62, 0x6C, 0x69, 0x63, 0x4B, 0x65, 0x79,
                0x00, 0x0B, 0x04, 0x0C, 0x03, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x42, 0x45, 0x47, 0x49, 0x4E, 0x20,
                0x50, 0x55, 0x42, 0x4C, 0x49, 0x43, 0x20, 0x4B, 0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A,
                0x4D, 0x49, 0x49, 0x42, 0x49, 0x6A, 0x41, 0x4E, 0x42, 0x67, 0x6B, 0x71, 0x68, 0x6B, 0x69, 0x47,
                0x39, 0x77, 0x30, 0x42, 0x41, 0x51, 0x45, 0x46, 0x41, 0x41, 0x4F, 0x43, 0x41, 0x51, 0x38, 0x41,
                0x4D, 0x49, 0x49, 0x42, 0x43, 0x67, 0x4B, 0x43, 0x41, 0x51, 0x45, 0x41, 0x72, 0x48, 0x77, 0x42,
                0x39, 0x49, 0x37, 0x67, 0x54, 0x78, 0x49, 0x56, 0x4C, 0x69, 0x62, 0x4B, 0x72, 0x35, 0x46, 0x71,
                0x0A, 0x6C, 0x39, 0x4B, 0x46, 0x49, 0x64, 0x39, 0x58, 0x67, 0x4E, 0x7A, 0x42, 0x55, 0x35, 0x49,
                0x4A, 0x45, 0x79, 0x76, 0x54, 0x43, 0x31, 0x38, 0x49, 0x33, 0x33, 0x72, 0x4E, 0x47, 0x6E, 0x41,
                0x46, 0x67, 0x34, 0x72, 0x67, 0x71, 0x4D, 0x41, 0x4A, 0x62, 0x45, 0x67, 0x32, 0x52, 0x76, 0x37,
                0x58, 0x4F, 0x62, 0x35, 0x56, 0x35, 0x48, 0x63, 0x55, 0x50, 0x4D, 0x51, 0x47, 0x44, 0x54, 0x6E,
                0x72, 0x0A, 0x53, 0x5A, 0x31, 0x51, 0x75, 0x2B, 0x51, 0x69, 0x46, 0x34, 0x69, 0x48, 0x6C, 0x30,
                0x49, 0x43, 0x36, 0x63, 0x51, 0x36, 0x69, 0x53, 0x73, 0x57, 0x6C, 0x4E, 0x70, 0x75, 0x41, 0x56,
                0x55, 0x6F, 0x44, 0x46, 0x45, 0x4F, 0x43, 0x59, 0x77, 0x34, 0x57, 0x59, 0x51, 0x77, 0x59, 0x37,
                0x2B, 0x63, 0x41, 0x49, 0x32, 0x44, 0x66, 0x57, 0x4F, 0x31, 0x41, 0x70, 0x48, 0x35, 0x39, 0x47,
                0x43, 0x79, 0x0A, 0x4D, 0x52, 0x79, 0x58, 0x42, 0x6C, 0x70, 0x48, 0x37, 0x4B, 0x70, 0x74, 0x61,
                0x65, 0x7A, 0x31, 0x36, 0x78, 0x55, 0x74, 0x67, 0x4C, 0x77, 0x42, 0x78, 0x75, 0x4C, 0x6C, 0x63,
                0x64, 0x76, 0x51, 0x57, 0x2F, 0x39, 0x79, 0x6E, 0x43, 0x65, 0x30, 0x4E, 0x36, 0x61, 0x52, 0x63,
                0x66, 0x58, 0x43, 0x71, 0x78, 0x58, 0x58, 0x67, 0x7A, 0x4F, 0x39, 0x39, 0x6F, 0x41, 0x35, 0x46,
                0x75, 0x68, 0x58, 0x0A, 0x77, 0x58, 0x6A, 0x52, 0x4D, 0x63, 0x47, 0x69, 0x45, 0x4B, 0x50, 0x72,
                0x69, 0x71, 0x6B, 0x57, 0x38, 0x31, 0x45, 0x6E, 0x48, 0x64, 0x59, 0x30, 0x30, 0x63, 0x41, 0x77,
                0x33, 0x56, 0x30, 0x78, 0x76, 0x64, 0x33, 0x58, 0x6D, 0x47, 0x6E, 0x6D, 0x66, 0x37, 0x69, 0x71,
                0x4D, 0x47, 0x57, 0x2F, 0x4A, 0x45, 0x2B, 0x67, 0x73, 0x65, 0x66, 0x2F, 0x48, 0x72, 0x6B, 0x75,
                0x75, 0x75, 0x63, 0x69, 0x0A, 0x41, 0x63, 0x74, 0x55, 0x43, 0x5A, 0x75, 0x47, 0x51, 0x35, 0x62,
                0x65, 0x41, 0x52, 0x77, 0x6E, 0x4A, 0x78, 0x69, 0x45, 0x74, 0x41, 0x51, 0x49, 0x4E, 0x6D, 0x71,
                0x62, 0x41, 0x64, 0x32, 0x67, 0x70, 0x7A, 0x72, 0x6D, 0x58, 0x53, 0x43, 0x43, 0x5A, 0x52, 0x45,
                0x2B, 0x42, 0x76, 0x65, 0x66, 0x6C, 0x33, 0x74, 0x42, 0x78, 0x6A, 0x4C, 0x54, 0x6E, 0x38, 0x44,
                0x30, 0x67, 0x31, 0x4D, 0x75, 0x0A, 0x68, 0x77, 0x49, 0x44, 0x41, 0x51, 0x41, 0x42, 0x0A, 0x2D,
                0x2D, 0x2D, 0x2D, 0x2D, 0x45, 0x4E, 0x44, 0x20, 0x50, 0x55, 0x42, 0x4C, 0x49, 0x43, 0x20, 0x4B,
                0x45, 0x59, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x0A, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x5E, 0xC9, 0xBA,
                0xDC};

            std::ofstream oKeyFile("609A09889F32452393D8CDB97C4A41B4.key", std::ios::binary);
            oKeyFile.write((const char *)c_stlSignKeyFile.data(), c_stlSignKeyFile.size());
            oKeyFile.close();
        }

        // Sign the message
        CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();
        std::vector<Byte> stlSignature = oCryptographicEngine.GenerateDigitalSignature(oSignKeyGuid, c_oStructuredBufferRequest.GetBuffer("MessageDigest"));
        // Create digital signature blob
        StructuredBuffer oDigitalSignature;
        oDigitalSignature.PutString("KeyGuid", oSignKeyGuid.ToString(eHyphensAndCurlyBraces));
        oDigitalSignature.PutDword("AlgorithmUsed", (Dword) oCryptographicEngine.GetKeySpecification(oSignKeyGuid));
        oDigitalSignature.PutUnsignedInt64("SignedTime", ::GetEpochTimeInSeconds());
        // Get x509 public key certificate (PEM) of the public key used to verify this signature
        std::string strPublicKeyPem =  oCryptographicEngine.GetPublicKeyPEM(oSignKeyGuid);
        _ThrowBaseExceptionIf((0 > strPublicKeyPem.size()), "Error getting public key PEM.", nullptr);
        oDigitalSignature.PutString("PublicKeyPEM", strPublicKeyPem);
        oDigitalSignature.PutBuffer("DigitalSignature", stlSignature);

        // Send back the digital signature blob
        oResponse.PutStructuredBuffer("DSIG", oDigitalSignature);
        dwStatus = 200;
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

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetUserInfoAndUpdateEosb
 * @brief Takes in an Eosb, updates it and returns user information
 * @param[in] c_oRequest contains the request body
 * @throw BaseException on failure
 * @returns Serialized structuredBuffer containing user information and the updated Eosb
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetUserInfoAndUpdateEosb(
    _in const StructuredBuffer & c_oStructuredBufferRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 401;

    try
    {
        // Get the plain text Eosb. It will also have the account key.
        // Do not expose this function. It is meant for internal use only
        StructuredBuffer oPlainTextEosb = this->GetPlainTextSsbFromEosb(c_oStructuredBufferRequest.GetBuffer("Eosb"));
        StructuredBuffer oRegisterEosbResponse(this->RegisterEosb(c_oStructuredBufferRequest));
        if (201 == oRegisterEosbResponse.GetDword("Status"))
        {
            // Unregister or update the Eosb
            // UnregisterEosb encrypts the Eosb with the current keys
            StructuredBuffer oUnregisterEosbResponse(this->UnregisterEosb(oRegisterEosbResponse));
            if (201 == oUnregisterEosbResponse.GetDword("Status"))
            {
                oResponse.PutBuffer("UpdatedEosb", oUnregisterEosbResponse.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("UserInformation", oRegisterEosbResponse);
                oResponse.PutBuffer("AccountEncryptionKey", oPlainTextEosb.GetBuffer("AccountKey"));
                dwStatus = 201;
            }
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
