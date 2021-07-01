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
#include "64BitHashes.h"
#include "DateAndTime.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"

static AccountDatabase * gs_oAccountDatabase = nullptr;

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
        AccountDatabase * poAccountDatabase = ::GetAccountDatabase();
        poAccountDatabase->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
        AccountDatabase * poAccountDatabase = ::GetAccountDatabase();
        poAccountDatabase->HandleIpcRequest(poIpcSocket);
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
        gs_oAccountDatabase->TerminateSignalEncountered();
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
    m_fTerminationSignalEncountered = false;
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

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("AccountManagerPluginGroup");
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
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall AccountDatabase::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
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

    // Add parameters to fetch a list of all organizations
    // Name, ElementType, and Range (if exists) are used by RestFrameworkRuntimeData::RunThread to vet request parameters.
    // Required parameters are marked by setting IsRequired to true
    // Otherwise the parameter is optional
    StructuredBuffer oListOrganizations;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oListOrganizations.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters to fetch a list of all users
    StructuredBuffer oListUsers;
    oListUsers.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters to fetch a list of users in an organization
    StructuredBuffer oListOrganizationUsers;
    oListOrganizationUsers.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oOrganizationGuid;
    oOrganizationGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oOrganizationGuid.PutBoolean("IsRequired", true);
    oListOrganizationUsers.PutStructuredBuffer("OrganizationGuid", oOrganizationGuid);

    // Add parameters for registering an organization and it's super user
    StructuredBuffer oRegisterOrganizationAndUser;
    StructuredBuffer oOrganizationName;
    oOrganizationName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oOrganizationName.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("OrganizationName", oOrganizationName);
    StructuredBuffer oOrganizationAddress;
    oOrganizationAddress.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oOrganizationAddress.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("OrganizationAddress", oOrganizationAddress);
    StructuredBuffer oPrimaryContactName;
    oPrimaryContactName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPrimaryContactName.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("PrimaryContactName", oPrimaryContactName);
    StructuredBuffer oPrimaryContactTitle;
    oPrimaryContactTitle.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPrimaryContactTitle.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("PrimaryContactTitle", oPrimaryContactTitle);
    StructuredBuffer oPrimaryContactEmail;
    oPrimaryContactEmail.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPrimaryContactEmail.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("PrimaryContactEmail", oPrimaryContactEmail);
    StructuredBuffer oPrimaryContactPhoneNumber;
    oPrimaryContactPhoneNumber.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPrimaryContactPhoneNumber.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("PrimaryContactPhoneNumber", oPrimaryContactPhoneNumber);
    StructuredBuffer oSecondaryContactName;
    oSecondaryContactName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSecondaryContactName.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("SecondaryContactName", oSecondaryContactName);
    StructuredBuffer oSecondaryContactTitle;
    oSecondaryContactTitle.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSecondaryContactTitle.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("SecondaryContactTitle", oSecondaryContactTitle);
    StructuredBuffer oSecondaryContactEmail;
    oSecondaryContactEmail.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSecondaryContactEmail.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("SecondaryContactEmail", oSecondaryContactEmail);
    StructuredBuffer oSecondaryContactPhoneNumber;
    oSecondaryContactPhoneNumber.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oSecondaryContactPhoneNumber.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("SecondaryContactPhoneNumber", oSecondaryContactPhoneNumber);
    StructuredBuffer oEmail;
    oEmail.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oEmail.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("Email", oEmail);
    StructuredBuffer oPassword;
    oPassword.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPassword.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("Password", oPassword);
    StructuredBuffer oName;
    oName.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oName.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("Name", oName);
    StructuredBuffer oTitle;
    oTitle.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oTitle.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("Title", oTitle);
    StructuredBuffer oPhoneNumber;
    oPhoneNumber.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPhoneNumber.PutBoolean("IsRequired", true);
    oRegisterOrganizationAndUser.PutStructuredBuffer("PhoneNumber", oPhoneNumber);

    // Add parameters for registering a new user
    StructuredBuffer oRegisterUser;
    oRegisterUser.PutStructuredBuffer("Eosb", oEosb);
    oRegisterUser.PutStructuredBuffer("Email", oEmail);
    oRegisterUser.PutStructuredBuffer("Password", oPassword);
    oRegisterUser.PutStructuredBuffer("Name", oName);
    oRegisterUser.PutStructuredBuffer("Title", oTitle);
    oRegisterUser.PutStructuredBuffer("PhoneNumber", oPhoneNumber);
    StructuredBuffer oAccessRights;
    oAccessRights.PutByte("ElementType", QWORD_VALUE_TYPE);
    oAccessRights.PutBoolean("IsRequired", true);
    oRegisterUser.PutStructuredBuffer("AccessRights", oAccessRights);

    // Add parameters for updating user's access rights
    StructuredBuffer oUpdateAccessRight;
    oUpdateAccessRight.PutStructuredBuffer("Eosb", oEosb);
    oUpdateAccessRight.PutStructuredBuffer("AccessRights", oAccessRights);
    StructuredBuffer oUserGuid;
    oUserGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oUserGuid.PutBoolean("IsRequired", true);
    oUpdateAccessRight.PutStructuredBuffer("UserGuid", oUserGuid);

    // Add parameters for updating organization information
    StructuredBuffer oUpdateOrganization;
    oUpdateOrganization.PutStructuredBuffer("Eosb", oEosb);
    oUpdateOrganization.PutStructuredBuffer("OrganizationGuid", oOrganizationGuid);
    StructuredBuffer oOrganizationInformation;
    oOrganizationInformation.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oOrganizationInformation.PutBoolean("IsRequired", true);
    oOrganizationInformation.PutStructuredBuffer("OrganizationName", oOrganizationName);
    oOrganizationInformation.PutStructuredBuffer("OrganizationAddress", oOrganizationAddress);
    oOrganizationInformation.PutStructuredBuffer("PrimaryContactName", oPrimaryContactName);
    oOrganizationInformation.PutStructuredBuffer("PrimaryContactTitle", oPrimaryContactTitle);
    oOrganizationInformation.PutStructuredBuffer("PrimaryContactEmail", oPrimaryContactEmail);
    oOrganizationInformation.PutStructuredBuffer("PrimaryContactPhoneNumber", oPrimaryContactPhoneNumber);
    oOrganizationInformation.PutStructuredBuffer("SecondaryContactName", oSecondaryContactName);
    oOrganizationInformation.PutStructuredBuffer("SecondaryContactTitle", oSecondaryContactTitle);
    oOrganizationInformation.PutStructuredBuffer("SecondaryContactEmail", oSecondaryContactEmail);
    oOrganizationInformation.PutStructuredBuffer("SecondaryContactPhoneNumber", oSecondaryContactPhoneNumber);
    oUpdateOrganization.PutStructuredBuffer("OrganizationInformation", oOrganizationInformation);

    // Add parameters for updating user information
    StructuredBuffer oUpdateUser;
    oUpdateUser.PutStructuredBuffer("Eosb", oEosb);
    oUpdateUser.PutStructuredBuffer("UserGuid", oUserGuid);
    oUpdateUser.PutStructuredBuffer("Name", oName);
    oUpdateUser.PutStructuredBuffer("Title", oTitle);
    oUpdateUser.PutStructuredBuffer("Email", oEmail);
    oUpdateUser.PutStructuredBuffer("PhoneNumber", oPhoneNumber);

    // Add parameters for deleting a user
    StructuredBuffer oDeleteUser;
    oDeleteUser.PutStructuredBuffer("Eosb", oEosb);
    oDeleteUser.PutStructuredBuffer("UserGuid", oUserGuid);
    StructuredBuffer oIsHardDelete;
    oIsHardDelete.PutByte("ElementType", BOOLEAN_VALUE_TYPE);
    oIsHardDelete.PutBoolean("IsRequired", true);
    oDeleteUser.PutStructuredBuffer("IsHardDelete", oIsHardDelete);

    // Takes in an EOSB and returns a list of all organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AccountManager/Organizations", oListOrganizations, 1);
    // Takes in an EOSB and returns a list of all users
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AccountManager/Users", oListUsers, 1);
    // Takes in an EOSB and returns a list of users associated with specified organization guid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AccountManager/Organization/Users", oListOrganizationUsers, 1);
    // TODO: take in Sail Eosb
    // Takes in user information and registers a user
    // Register the organziation first and register the user as a super admin afterwards
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AccountManager/RegisterUser", oRegisterOrganizationAndUser, 1);
    // Accessed by an admin user to register a new user
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AccountManager/Admin/RegisterUser", oRegisterUser, 1);
    // Updates user's access rights
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/AccountManager/Update/AccessRight", oUpdateAccessRight, 1);
    // Updates organization information
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/AccountManager/Update/Organization", oUpdateOrganization, 1);
    // Updates user information, excluding access rights
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/AccountManager/Update/User", oUpdateUser, 1);
    // Delete a user from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/AccountManager/Remove/User", oDeleteUser, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{0BE996BF-6966-41EB-B211-2D63C9908289}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("AccountManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall AccountDatabase::RunIpcServer(
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
 * @class AccountDatabase
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall AccountDatabase::HandleIpcRequest(
    _in Socket * poSocket
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    std::vector<Byte> stlResponse;

    StructuredBuffer oRequestParameters(::GetIpcTransaction(poSocket));

    Dword dwTransactionType = oRequestParameters.GetDword("TransactionType");

    switch (dwTransactionType)
    {
        case 0x00000001 // GetUserRecords
        :
            stlResponse = this->GetUserRecords(oRequestParameters);
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
        if ("/SAIL/AccountManager/Organizations" == strResource)
        {
            stlResponseBuffer = this->ListOrganizations(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AccountManager/Users" == strResource)
        {
            stlResponseBuffer = this->ListUsers(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AccountManager/Organization/Users" == strResource)
        {
            stlResponseBuffer = this->ListOrganizationUsers(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/AccountManager/RegisterUser" == strResource)
        {
            stlResponseBuffer = this->RegisterOrganizationAndSuperUser(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AccountManager/Admin/RegisterUser" == strResource)
        {
            stlResponseBuffer = this->RegisterUser(c_oRequestStructuredBuffer);
        }
    }
    else if ("PUT" == strVerb)
    {
        if ("/SAIL/AccountManager/Update/AccessRight" == strResource)
        {
            stlResponseBuffer = this->UpdateUserRights(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AccountManager/Update/Organization" == strResource)
        {
            stlResponseBuffer = this->UpdateOrganizationInformation(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AccountManager/Update/User" == strResource)
        {
            stlResponseBuffer = this->UpdateUserInformation(c_oRequestStructuredBuffer);
        }
    }
    else if ("DELETE" == strVerb)
    {
        if ("/SAIL/AccountManager/Remove/User" == strResource)
        {
            stlResponseBuffer = this->DeleteUser(c_oRequestStructuredBuffer);
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
 * @function GetUserRecords
 * @brief Given a email/password string, fetches Basic and Confidential records from the database
 * @param[in] c_oRequest contains the request body
 * @returns StructuredBuffer containing user basic and confidential records
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::GetUserRecords(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oAccountRecords;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Calculate 64BitHash of the passphrase
        std::string strPassphrase = c_oRequest.GetString("Passphrase");
        Qword qw64BitHashPassphrase = ::Get64BitHashOfNullTerminatedString(strPassphrase.c_str(), false);

        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Create a request to fetch Basic user record
        StructuredBuffer oBasicRecordRequest;
        oBasicRecordRequest.PutString("PluginName", "DatabaseManager");
        oBasicRecordRequest.PutString("Verb", "GET");
        oBasicRecordRequest.PutString("Resource", "/SAIL/DatabaseManager/BasicUser");
        oBasicRecordRequest.PutQword("Passphrase", qw64BitHashPassphrase);
        std::vector<Byte> stlBasicRecordRequest = ::CreateRequestPacket(oBasicRecordRequest);
        // Send request packet
        poTlsNode->Write(stlBasicRecordRequest.data(), (stlBasicRecordRequest.size()));

        // Read header and body of the response
        std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
        _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
        unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
        std::vector<Byte> stlBasicUser = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
        _ThrowBaseExceptionIf((0 == stlBasicUser.size()), "Dead Packet.", nullptr);
        // Make sure to release the poTlsNode
        poTlsNode->Release();
        poTlsNode = nullptr;
        
        StructuredBuffer oBasicRecord(stlBasicUser);
        if (404 != oBasicRecord.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to fetch Confidential user record
            StructuredBuffer oConfidentialRecordRequest;
            oConfidentialRecordRequest.PutString("PluginName", "DatabaseManager");
            oConfidentialRecordRequest.PutString("Verb", "GET");
            oConfidentialRecordRequest.PutString("Resource", "/SAIL/DatabaseManager/ConfidentialUser");
            oConfidentialRecordRequest.PutString("UserUuid", oBasicRecord.GetStructuredBuffer("BasicUserRecord").GetGuid("UserGuid").ToString(eHyphensAndCurlyBraces));
            std::vector<Byte> stlConfidentialRecordRequest = ::CreateRequestPacket(oConfidentialRecordRequest);
            // Send request packet
            poTlsNode->Write(stlConfidentialRecordRequest.data(), (stlConfidentialRecordRequest.size()));

            // Read header and body of the response
            stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            std::vector<Byte> stlConfidentialUser = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
            _ThrowBaseExceptionIf((0 == stlConfidentialUser.size()), "Dead Packet.", nullptr);
            // Make sure to release the poTlsNode
            poTlsNode->Release();
            poTlsNode = nullptr;
        
            StructuredBuffer oConfidentialRecord(stlConfidentialUser);
            if (404 != oConfidentialRecord.GetDword("Status"))
            {
                // Add BasicUserRecord and ConfidentialOrganizationOrUserRecord
                oAccountRecords.PutStructuredBuffer("BasicUserRecord", oBasicRecord.GetStructuredBuffer("BasicUserRecord"));
                oAccountRecords.PutStructuredBuffer("ConfidentialOrganizationOrUserRecord", oConfidentialRecord.GetStructuredBuffer("ConfidentialOrganizationOrUserRecord"));
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oAccountRecords.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oAccountRecords.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    // Add transaction status to the response
    oAccountRecords.PutDword("Status", dwStatus);
    
    return oAccountRecords.GetSerializedBuffer();
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
        StructuredBuffer oDecryptedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest));
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
        ::RegisterException(oException, __func__, __LINE__);
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

    // Add status code for the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function RegisterOrganizationAndSuperUser
 * @brief Register an organization and the new user as it's super user
 * @param[in] c_oRequest contains the organization and user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 * @note
 *  Register the organziation first and register the user as a super admin afterwards.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::RegisterOrganizationAndSuperUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;
    Socket * poIpcAuditLogManager = nullptr;

    try
    {
        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Create a request to add a user to the database
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "POST");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterOrganization");
        oRequest.PutStructuredBuffer("Request", c_oRequest);
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
        
        // Check if DatabaseManager registered the user or not
        StructuredBuffer oDatabaseResponse(stlResponse);
        if (204 != oDatabaseResponse.GetDword("Status"))
        {
            dwStatus = 201;
            // Create request to register a root node event for the organization
            StructuredBuffer oRootEvent;
            oRootEvent.PutDword("TransactionType", 0x00000001);
            StructuredBuffer oMetadata;
            oMetadata.PutString("EventGuid", Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces));
            oMetadata.PutString("ParentGuid", "{00000000-0000-0000-0000-000000000000}");
            oMetadata.PutString("OrganizationGuid", oDatabaseResponse.GetString("OrganizationGuid"));
            oMetadata.PutQword("EventType", 1);    // 1 for Root event and 2 for Branch event type
            oMetadata.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
            oMetadata.PutStructuredBuffer("PlainTextEventData", StructuredBuffer());
            oRootEvent.PutStructuredBuffer("NonLeafEvent", oMetadata);
            // Call AuditLog plugin to register root node event
            poIpcAuditLogManager = ::ConnectToUnixDomainSocket("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
            StructuredBuffer oStatus(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oRootEvent));
            poIpcAuditLogManager->Release();
            poIpcAuditLogManager = nullptr;
            if ((0 < oStatus.GetSerializedBufferRawDataSizeInBytes())&&(201 == oStatus.GetDword("Status")))
            {
                oResponse.PutDword("RootEventStatus", 201);
            }
            else
            {
                oResponse.PutDword("RootEventStatus", 204);
            }
        }
        else
        {
            if (true == oDatabaseResponse.IsElementPresent("ErrorMessage", ANSI_CHARACTER_STRING_VALUE_TYPE))
            {
                oResponse.PutString("ErrorMessage", oDatabaseResponse.GetString("ErrorMessage"));
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
    if (nullptr != poIpcAuditLogManager)
    {
        poIpcAuditLogManager->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class AccountDatabase
 * @function RegisterUser
 * @brief Add a user account to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 * @note
 *  If user's organziation not found then, register the organziation first and register 
 *  the user as a super admin afterwards.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::RegisterUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "POST");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterUser");
                oRequest.PutStructuredBuffer("Request", c_oRequest);
                oRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
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
                
                // Check if DatabaseManager registered the user or not
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
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function UpdateUserRights
 * @brief Update user's access rights
 * @param[in] c_oRequest contains the admin's Eosb, user guid, and new rights bitmap
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::UpdateUserRights(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateUserRights");
                oRequest.PutString("UserGuid", c_oRequest.GetString("UserGuid"));
                oRequest.PutQword("AccessRights", c_oRequest.GetQword("AccessRights"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;
                
                // Check if DatabaseManager updated the user rights or not
                StructuredBuffer oDatabaseResponse(stlResponse);
                if (204 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function UpdateOrganizationInformation
 * @brief Update organization information
 * @param[in] c_oRequest contains the admin's Eosb, organization guid, and new information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::UpdateOrganizationInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateOrganizationInformation");
                oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
                oRequest.PutStructuredBuffer("OrganizationInformation", c_oRequest.GetStructuredBuffer("OrganizationInformation"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;
        
                // Check if DatabaseManager updated the organization information or not
                StructuredBuffer oDatabaseResponse(stlResponse);
                if (204 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function UpdateUserInformation
 * @brief Update user information, excluding access rights
 * @param[in] c_oRequest contains the admin's Eosb or the user's eosb, user guid, and new user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::UpdateUserInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information to check if the user has admin access rights or if the Eosb is associated with the userguid
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // TODO: add an or in the if statement to check if user is eAdmin
            // For now, an admin can't change a user's information
            if (strUserGuid == oUserInfo.GetGuid("UserGuid").ToString(eHyphensAndCurlyBraces))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateUserInformation");
                oRequest.PutString("UserGuid", c_oRequest.GetString("UserGuid"));
                oRequest.PutStructuredBuffer("UserInformation", c_oRequest.GetStructuredBuffer("UserInformation"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                // Read header and body of the response
                std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
                _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
                _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                // Make sure to release the poTlsNode
                poTlsNode->Release();
                poTlsNode = nullptr;
                
                // Check if DatabaseManager updated the user information, excluding the access rights, or not
                StructuredBuffer oDatabaseResponse(stlResponse);
                if (204 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function ListOrganizations
 * @brief Return a list of organizations
 * @param[in] c_oRequest contains the user's eosb
 * @throw BaseException Error StructuredBuffer element not found
 * @returns structuredbuffer containing list of organizations
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::ListOrganizations(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // TODO: Should be Sail admin
        // For now: Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/Organizations");
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
                if (404 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    oResponse.PutStructuredBuffer("Organizations", oDatabaseResponse.GetStructuredBuffer("Organizations"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function ListUsers
 * @brief Return a list of all users
 * @param[in] c_oRequest contains the user's eosb
 * @throw BaseException Error StructuredBuffer element not found
 * @returns structuredbuffer containing list of users
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::ListUsers(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // TODO: Should be Sail admin
        // For now: Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/Users");
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
                if (404 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    oResponse.PutStructuredBuffer("Users", oDatabaseResponse.GetStructuredBuffer("Users"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function ListOrganizationUsers
 * @brief Return a list of users for an organization
 * @param[in] c_oRequest contains the user's eosb and organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns structuredbuffer containing list of users for an organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::ListOrganizationUsers(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user has admin access rights
        // Todo: add an or statement and check if its a sail admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "GET");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/OrganizationUsers");
                oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
                if (404 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    oResponse.PutStructuredBuffer("OrganizationUsers", oDatabaseResponse.GetStructuredBuffer("OrganizationUsers"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function DeleteUser
 * @brief Delete a user from the database
 * @param[in] c_oRequest contains the user's eosb and user guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::DeleteUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user has admin access rights
        // Todo: add an or statement and check if its a sail admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "DELETE");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/DeleteUser");
                oRequest.PutString("UserGuid", c_oRequest.GetString("UserGuid"));
                oRequest.PutBoolean("IsHardDelete", c_oRequest.GetBoolean("IsHardDelete"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
                if (404 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @class AccountDatabase
 * @function DeleteOrganization
 * @brief Delete an organization and its users from the database
 * @param[in] c_oRequest contains the user's eosb and organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall AccountDatabase::DeleteOrganization(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get user information to check if the user has admin access rights
        // Todo: add an or statement and check if its a sail admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to add a user to the database
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "DELETE");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/DeleteOrganization");
                oRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
                oRequest.PutBoolean("IsHardDelete", c_oRequest.GetBoolean("IsHardDelete"));
                std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                // Send request packet
                poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
                if (404 != oDatabaseResponse.GetDword("Status"))
                {
                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                    dwStatus = 200;
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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


