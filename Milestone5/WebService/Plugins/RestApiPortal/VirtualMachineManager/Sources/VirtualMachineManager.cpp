/*********************************************************************************************
 *
 * @file VirtualMachineManager.cpp
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "VirtualMachineManager.h"
#include "DateAndTime.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"

static VirtualMachineManager * gs_oVirtualMachineManager = nullptr;

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
        VirtualMachineManager * poVirtualMachineManager = ::GetVirtualMachineManager();
        poVirtualMachineManager->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
        VirtualMachineManager * poVirtualMachineManager = ::GetVirtualMachineManager();
        poVirtualMachineManager->HandleIpcRequest(poIpcSocket);
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
 * @function GetVirtualMachineManager
 * @brief Create a singleton object of VirtualMachineManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of VirtualMachineManager class
 *
 ********************************************************************************************/

VirtualMachineManager * __stdcall GetVirtualMachineManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oVirtualMachineManager)
    {
        gs_oVirtualMachineManager = new VirtualMachineManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oVirtualMachineManager);
    }

    return gs_oVirtualMachineManager;
}

/********************************************************************************************
 *
 * @function ShutdownVirtualMachineManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownVirtualMachineManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oVirtualMachineManager)
    {
        gs_oVirtualMachineManager->TerminateSignalEncountered();
        gs_oVirtualMachineManager->Release();
        gs_oVirtualMachineManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function VirtualMachineManager
 * @brief Constructor
 *
 ********************************************************************************************/

VirtualMachineManager::VirtualMachineManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function VirtualMachineManager
 * @brief Copy Constructor
 * @param[in] c_oVirtualMachineManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

VirtualMachineManager::VirtualMachineManager(
    _in const VirtualMachineManager & c_oVirtualMachineManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function ~VirtualMachineManager
 * @brief Destructor
 *
 ********************************************************************************************/

VirtualMachineManager::~VirtualMachineManager(void)
{
    __DebugFunction();

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("VirtualMachineManagerPluginGroup");
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall VirtualMachineManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "VirtualMachineManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall VirtualMachineManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{FC48839C-AFA9-4D95-B83E-538A0B31CFD2}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall VirtualMachineManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall VirtualMachineManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall VirtualMachineManager::InitializePlugin(void)
{
    __DebugFunction();

    // Add parameters for RegisterVM resource in a StructuredBuffer.
    // Name, ElementType, and Range (if exists) are used by RestFrameworkRuntimeData::RunThread to vet request parameters.
    // Required parameters are marked by setting IsRequired to true
    // Otherwise the parameter is optional
    StructuredBuffer oRegisterVmParameters;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("IEosb", oEosb);
    StructuredBuffer oDcGuid;
    oDcGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDcGuid.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("DigitalContractGuid", oDcGuid);
    StructuredBuffer oVmGuid;
    oVmGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVmGuid.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("VirtualMachineGuid", oVmGuid);
    StructuredBuffer oHeartBeatTime;
    oHeartBeatTime.PutByte("ElementType", UINT64_VALUE_TYPE);
    oHeartBeatTime.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("HeartbeatBroadcastTime", oHeartBeatTime);
    StructuredBuffer oIpAddress;
    oIpAddress.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oIpAddress.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("IPAddress", oIpAddress);
    StructuredBuffer oNumberOfVCPUs;
    oNumberOfVCPUs.PutByte("ElementType", UINT64_VALUE_TYPE);
    oNumberOfVCPUs.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("NumberOfVCPU", oNumberOfVCPUs);
    StructuredBuffer oHostRegion;
    oHostRegion.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oHostRegion.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("HostRegion", oHostRegion);
    StructuredBuffer oStartTime;
    oStartTime.PutByte("ElementType", UINT64_VALUE_TYPE);
    oStartTime.PutBoolean("IsRequired", true);
    oRegisterVmParameters.PutStructuredBuffer("StartTime", oStartTime);

    // Add Parameters for registering audit event for DOO/RO
    StructuredBuffer oRegisterEvent;
    oRegisterEvent.PutStructuredBuffer("Eosb", oEosb);
    oRegisterEvent.PutStructuredBuffer("VirtualMachineGuid", oVmGuid);

    // Add Parameters for getting list of running VMs
    StructuredBuffer oListRunningVMs;
    oListRunningVMs.PutStructuredBuffer("Eosb", oEosb);
    oListRunningVMs.PutStructuredBuffer("DigitalContractGuid", oDcGuid);

    // Add Parameters for getting full status of a virtual machine
    StructuredBuffer oPullVM;
    oPullVM.PutStructuredBuffer("Eosb", oEosb);
    oPullVM.PutStructuredBuffer("VirtualMachineGuid", oVmGuid);

    // Add Parameters for getting list of VMs associated with an organization
    StructuredBuffer oListOrganizationVMs;
    oListOrganizationVMs.PutStructuredBuffer("Eosb", oEosb);

    // Add Parameters for updating status of a virtual machine
    StructuredBuffer oUpdateStatus;
    oUpdateStatus.PutStructuredBuffer("Eosb", oEosb);
    oUpdateStatus.PutStructuredBuffer("VirtualMachineGuid", oVmGuid);
    StructuredBuffer oState;
    oState.PutByte("ElementType", DWORD_VALUE_TYPE);
    oState.PutBoolean("IsRequired", true);
    oUpdateStatus.PutStructuredBuffer("State", oState);
    // Make VMLoggedInUser optional for UpdateStatus API but the API will send back an error if the State is "InUse" and this field is missing
    // TODO: Prawal add some utility to this
    // StructuredBuffer oVmLoggedInUser;
    // oVmLoggedInUser.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    // oVmLoggedInUser.PutBoolean("IsRequired", false);
    // oUpdateStatus.PutStructuredBuffer("VMLoggedInUser", oVmLoggedInUser);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Takes in an EOSB and sends back list of all running VM ip addresses associated with the digiatl contract
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses", oListRunningVMs, 1);

    // Takes in an EOSB and sends full status of a VM
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VirtualMachineManager/PullVirtualMachine", oPullVM, 1);

    // Takes in an EOSB and get a list of VMs associated with an organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VirtualMachineManager/ListVirtualMachines", oListOrganizationVMs, 1);

    // Sends back report of VM's status
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/VirtualMachineManager/UpdateStatus", oUpdateStatus, 1);

    // Register a Virtual Machine
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/RegisterVM", oRegisterVmParameters, 1);

    // Register a Virtual Machine event for DOO
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/DataOwner/RegisterVM", oRegisterEvent, 1);

    // Register a Virtual Machine event for RO
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/Researcher/RegisterVM", oRegisterEvent, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("VirtualMachineManagerPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall VirtualMachineManager::RunIpcServer(
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
 * @class VirtualMachineManager
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall VirtualMachineManager::HandleIpcRequest(
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
        case 0x00000001 // GetListOfVmIpAddressesAssociatedWithDc
        :
            stlResponse = this->GetListOfVmIpAddressesAssociatedWithDc(oRequestParameters);
            break;
        case 0x00000002 // UpdateVirtualMachineStatus
        :
            stlResponse = this->UpdateVirtualMachineStatus(oRequestParameters);
            break;
        case 0x00000003 // RegisterNewVirtualMachine
        :
            stlResponse = this->RegisterVmInstance(oRequestParameters);
            break;
        case 0x00000004 // AddVirtualMachineIpAddress
        :
            stlResponse = this->AddVirtualMachineIpAddress(oRequestParameters);
            break;
        case 0x00000005 // PullVirtualMachine
        :
            stlResponse = this->GetVmInformation(oRequestParameters);
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
 * @class VirtualMachineManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall VirtualMachineManager::SubmitRequest(
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
        if ("/SAIL/VirtualMachineManager/GetRunningVMsIpAdresses" == strResource)
        {
            stlResponseBuffer = this->GetListOfVmIpAddressesAssociatedWithDc(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/VirtualMachineManager/PullVirtualMachine" == strResource)
        {
            stlResponseBuffer = this->GetVmInformation(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/VirtualMachineManager/ListVirtualMachines" == strResource)
        {
            stlResponseBuffer = this->GetListOfOrganizationVMs(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/VirtualMachineManager/RegisterVM" == strResource)
        {
            stlResponseBuffer = this->RegisterVmInstance(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/VirtualMachineManager/DataOwner/RegisterVM" == strResource)
        {
            stlResponseBuffer = this->RegisterVmAfterDataUpload(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/VirtualMachineManager/Researcher/RegisterVM" == strResource)
        {
            stlResponseBuffer = this->RegisterVmForComputation(c_oRequestStructuredBuffer);
        }
    }
    else if ("PUT" == strVerb)
    {
        if ("/SAIL/VirtualMachineManager/UpdateStatus" == strResource)
        {
            stlResponseBuffer = this->UpdateVirtualMachineStatus(c_oRequestStructuredBuffer);
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
 * @class VirtualMachineManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall VirtualMachineManager::GetResponse(
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
 * @class VirtualMachineManager
 * @function GetListOfOrganizationVMs
 * @brief Send back list of all running VMs associated with an organization guid
 * @param[in] c_oRequest contains the IEOSB
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of VMs
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetListOfOrganizationVMs(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    Socket * poIpcDigitalContractManager = nullptr;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Call DigitalContractDatabase plugin to get the digital contracts associated with the organization
            StructuredBuffer oDigitalContractRequest;
            oDigitalContractRequest.PutDword("TransactionType", 0x00000002);
            oDigitalContractRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
            poIpcDigitalContractManager = ::ConnectToUnixDomainSocket("/tmp/{BC5AEAAF-E37E-4605-B074-F9DF2E82CD34}");
            StructuredBuffer oDigitalContractResponse(::PutIpcTransactionAndGetResponse(poIpcDigitalContractManager, oDigitalContractRequest, false));
            poIpcDigitalContractManager->Release();
            poIpcDigitalContractManager = nullptr;
            if ((0 < oDigitalContractResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oDigitalContractResponse.GetDword("Status")))
            {
                StructuredBuffer oDigitalContracts = oDigitalContractResponse.GetStructuredBuffer("DigitalContracts");
                // Get list of VMs associated with a digital contract
                StructuredBuffer oListOfVMs;
                for (std::string strDcGuid : oDigitalContracts.GetNamesOfElements())
                {
                    StructuredBuffer oDigitalContract = oDigitalContracts.GetStructuredBuffer(strDcGuid.c_str());
                    // Make a Tls connection with the database portal
                    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                    // Create a request to get the list of VMs
                    StructuredBuffer oRequest;
                    oRequest.PutString("PluginName", "DatabaseManager");
                    oRequest.PutString("Verb", "GET");
                    oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListOfVMsAssociatedWithDC");
                    oRequest.PutString("DigitalContractGuid", strDcGuid);
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

                    StructuredBuffer oDatabaseResponse(stlResponse);
                    if (404 != oDatabaseResponse.GetDword("Status"))
                    {
                        StructuredBuffer oVmsAssociatedWithDc;
                        oVmsAssociatedWithDc.PutString("HostForVirtualMachines", oDigitalContract.GetString("HostForVirtualMachines"));
                        oVmsAssociatedWithDc.PutString("ResearcherOrganization", oDigitalContract.GetString("ResearcherOrganization"));
                        oVmsAssociatedWithDc.PutString("DataOwnerOrganization", oDigitalContract.GetString("DataOwnerOrganization"));
                        oVmsAssociatedWithDc.PutStructuredBuffer("VirtualMachinesAssociatedWithDc", oDatabaseResponse.GetStructuredBuffer("VirtualMachines"));
                        oListOfVMs.PutStructuredBuffer(strDcGuid.c_str(), oVmsAssociatedWithDc);
                    }
                }
                oResponse.PutStructuredBuffer("VirtualMachines", oListOfVMs);
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

    if (nullptr != poIpcDigitalContractManager)
    {
        poIpcDigitalContractManager->Release();
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
 * @class VirtualMachineManager
 * @function GetListOfVmIpAddressesAssociatedWithDc
 * @brief Send back list of all running VMs' ip addresses associated with the digital contract
 * @param[in] c_oRequest contains the IEOSB
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of running VMs information associated with the digital contract
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetListOfVmIpAddressesAssociatedWithDc(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get the list of VMs
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListOfVMIpAddressesAssociatedWithDC");
            oRequest.PutString("DigitalContractGuid", c_oRequest.GetString("DigitalContractGuid"));
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
                
            StructuredBuffer oDatabaseResponse(stlResponse);
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                oResponse.PutStructuredBuffer("VirtualMachines", oDatabaseResponse.GetStructuredBuffer("VirtualMachines"));
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
 * @class VirtualMachineManager
 * @function GetUserInfo
 * @brief Take in a full IEOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetUserInfo(
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
            oResponse.PutQword("AccessRights", oEosb.GetQword("AccessRights"));
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

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetVmInformation
 * @brief Fetch the virtual machine information
 * @param[in] c_oRequest contains the virtual machine guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the virtual machine information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetVmInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get user information
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get the virtual machine information
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullVirtualMachine");
            oRequest.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
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

            StructuredBuffer oDatabaseResponse(stlResponse);
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                StructuredBuffer oVirtualMachine = oDatabaseResponse.GetStructuredBuffer("VirtualMachine");
                if (true == oVirtualMachine.IsElementPresent("State", DWORD_VALUE_TYPE))
                {
                    if (VirtualMachineState::eInUse != oVirtualMachine.GetDword("State"))
                    {
                        oVirtualMachine.RemoveElement("VMLoggedInUser");
                    }
                }
                oResponse.PutStructuredBuffer("VirtualMachine", oVirtualMachine);
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
 * @class VirtualMachineManager
 * @function VerifyDigitalContract
 * @brief Check if IEosb is of the data owner of the digital contract associated with the strDcGuid
 * @param[in] c_oRequest contains the request body
 * @param[in] fIsEitherRoOrDoo checks if the user's organization is either RO or DOO
 * @param[in] fIsResearcher checks if the user's organization is RO or DOO
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the comparison status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::VerifyDigitalContract(
    _in const StructuredBuffer & c_oRequest,
    _in bool fIsEitherRoOrDoo,
    _in bool fIsResearcher
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    Socket * poIpcDigitalContractManager = nullptr;

    try
    {
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Call DigitalContractDatabase plugin to get the digital contract associated with the digital contract guid
        StructuredBuffer oDigitalContractRequest;
        oDigitalContractRequest.PutDword("TransactionType", 0x00000001);
        oDigitalContractRequest.PutString("DigitalContractGuid", c_oRequest.GetString("DigitalContractGuid"));
        oDigitalContractRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
        poIpcDigitalContractManager = ::ConnectToUnixDomainSocket("/tmp/{BC5AEAAF-E37E-4605-B074-F9DF2E82CD34}");
        StructuredBuffer oDigitalContractResponse(::PutIpcTransactionAndGetResponse(poIpcDigitalContractManager, oDigitalContractRequest, false));
        poIpcDigitalContractManager->Release();
        poIpcDigitalContractManager = nullptr;
        if ((0 < oDigitalContractResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oDigitalContractResponse.GetDword("Status")))
        {   
            if (true == fIsEitherRoOrDoo)
            {
                // Check if Eosb is of user within RO or DOO of the digital contract associated with the strDcGuid
                if ((strOrganizationGuid == oDigitalContractResponse.GetString("ResearcherOrganization")) || (strOrganizationGuid == oDigitalContractResponse.GetString("DataOwnerOrganization")))
                {
                    oResponse.PutBoolean("IsEqual", true);
                } 
            }
            else 
            {
                if (true == fIsResearcher)
                {
                    // Check if Eosb is of user within RO of the digital contract associated with the strDcGuid
                    if (strOrganizationGuid == oDigitalContractResponse.GetString("ResearcherOrganization"))
                    {
                        oResponse.PutBoolean("IsEqual", true);
                    } 
                }
                else 
                {
                    // Check if Eosb is of the data owner of the digital contract associated with the strDcGuid
                    if (strOrganizationGuid == oDigitalContractResponse.GetString("DataOwnerOrganization"))
                    {
                        oResponse.PutBoolean("IsEqual", true);
                    }
                }
            }
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

    if (nullptr != poIpcDigitalContractManager)
    {
        poIpcDigitalContractManager->Release();
    }
    
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function RegisterVmInstance
 * @brief Take in IEOSB of the Initializer and add the vm record to the database
 * @param[in] c_oRequest contains IEOSB of the Initializer and the VM information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 * @note
 *      This is step 1 of the register virtual machine feature. Once a data owner accepts a digital
 *      contract, a virtual machine is initialized and it registers itself with the webservices.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::RegisterVmInstance(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    // TODO: Implement the two steps described in the spec document.
    // The step involves registration of a VM and proof of remote attestation.
    // VMEosb should be returned after the remote attestation

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;
    Socket * poIpcCryptographicManager = nullptr;

    try
    {
        // Take in IEOSB, DigitalContractGuid, VMGuid, and VM information
        // TODO: Prawal
        // std::vector<Byte> stlEosb = c_oRequest.GetBuffer("IEosb");
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");

        // Get user information
        StructuredBuffer oEosbRequest;
        oEosbRequest.PutBuffer("Eosb", stlEosb);
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(oEosbRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Verify the Digital Contract and check if the user's organization is an associated party
            StructuredBuffer oVerifyDcRequest;
            oVerifyDcRequest.PutString("DigitalContractGuid", strDcGuid);
            oVerifyDcRequest.PutString("OrganizationGuid", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
            oVerifyDcRequest.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVerificationStatus(this->VerifyDigitalContract(oVerifyDcRequest, true, false));
            _ThrowBaseExceptionIf((200 != oVerificationStatus.GetDword("Status")), "Digital contract could not be verified.", nullptr);
            _ThrowBaseExceptionIf((true != oVerificationStatus.GetBoolean("IsEqual")), "Digital contract is not associated to the user's organization.", nullptr);

            // TODO: Check if the Eosb is an imposter Eosb
            // TODO: Add a check and Register Vm only if it is an imposter Eosb
            // Register the Virtual Machine
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to register the virtual machine information in the database
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "POST");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterVirtualMachine");
            oRequest.PutString("VirtualMachineGuid", strVmGuid);
            oRequest.PutString("DigitalContractTitle", c_oRequest.GetString("DigitalContractTitle"));
            oRequest.PutString("DigitalContractGuid", strDcGuid);
            oRequest.PutUnsignedInt64("RegistrationTime", ::GetEpochTimeInSeconds());
            oRequest.PutUnsignedInt64("HeartbeatBroadcastTime", c_oRequest.GetUnsignedInt64("HeartbeatBroadcastTime"));
            oRequest.PutString("IPAddress", c_oRequest.GetString("IPAddress"));
            oRequest.PutUnsignedInt64("NumberOfVCPU", c_oRequest.GetUnsignedInt64("NumberOfVCPU"));
            oRequest.PutString("HostRegion", c_oRequest.GetString("HostRegion"));
            oRequest.PutUnsignedInt64("StartTime", c_oRequest.GetUnsignedInt64("StartTime"));
            oRequest.PutString("Note", "...");
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

            // Check if DatabaseManager registered the virtual machine or not
            StructuredBuffer oDatabaseResponse(stlResponse);
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                // Call CryptographicManager plugin to get the VMEOSB
                StructuredBuffer oUpdateEosbRequest;
                oUpdateEosbRequest.PutDword("TransactionType", 0x00000005);
                oUpdateEosbRequest.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oUpdateEosbRequest.PutQword("AccessRights", EosbAccessRights::eVmEosb);
                poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
                StructuredBuffer oUpdatedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oUpdateEosbRequest, false));
                poIpcCryptographicManager->Release();
                poIpcCryptographicManager = nullptr;
                // Throw base exception if transaction was unsuccessful
                _ThrowBaseExceptionIf(((0 == oUpdatedEosb.GetSerializedBufferRawDataSizeInBytes())&&(200 != oUpdatedEosb.GetDword("Status"))), "Error updating the Eosb", nullptr);
                oResponse.PutBuffer("VmEosb", oUpdatedEosb.GetBuffer("UpdatedEosb"));
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
    if (nullptr != poIpcCryptographicManager)
    {
        poIpcCryptographicManager->Release();
    }


    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function RegisterVmAfterDataUpload
 * @brief Take in a VmEosb of a dataset admin and add a Vm branch event under DOO's root event
 * @param[in] c_oRequest contains EOSB of the dataset admin and the VM information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 * @note
 *      This is step 2 of the register virtual machine feature. Once a data owner accepts a digital
 *      contract and a virtual machine is initialized, they upload the requested dataset to the 
 *      virtual machine and an audit event gets added to DOO's audit log tree.
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::RegisterVmAfterDataUpload(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Take in EOSB, DigitalContractGuid, VMGuid, and VM information
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");

        // Get Digital contract guid
        std::string strDcGuid = StructuredBuffer(this->GetVmInformation(c_oRequest)).GetStructuredBuffer("VirtualMachine").GetString("DigitalContractGuid");

        // Get user information 
        StructuredBuffer oEosbRequest;
        oEosbRequest.PutBuffer("Eosb", stlEosb);
        StructuredBuffer oUserInfo(this->GetUserInfo(oEosbRequest));
        // Check if the Eosb is a Vm Eosb
        // Register Vm event if it is a Vm Eosb
        // Check if the user is a dataset admin
        // if ((eVmEosb == oUserInfo.GetQword("AccessRights"))&&(eDatasetAdmin == oUserInfo.GetQword("UserAccessRights")))
        if (EosbAccessRights::eEosb == oUserInfo.GetQword("AccessRights"))
        {
            // Get the organization guid
            std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
            // Check if Eosb is of the data owner of the digital contract associated with the strDcGuid
            StructuredBuffer oVerifyDcRequest;
            oVerifyDcRequest.PutString("DigitalContractGuid", strDcGuid);
            oVerifyDcRequest.PutString("OrganizationGuid", strOrganizationGuid);
            oVerifyDcRequest.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVerificationStatus(this->VerifyDigitalContract(oVerifyDcRequest, false, false));
            if (200 == oVerificationStatus.GetDword("Status"))
            {
                if (true == oVerificationStatus.GetBoolean("IsEqual"))
                {
                    // Check whether DC branch event log exists for DCGuid in the database or not
                    // If it does then, create a Vm branch event log
                    StructuredBuffer oEventInformation;
                    oEventInformation.PutBuffer("Eosb", stlEosb);
                    oEventInformation.PutString("DigitalContractGuid", strDcGuid);
                    oEventInformation.PutString("VirtualMachineGuid", strVmGuid);
                    oEventInformation.PutString("OrganizationGuid", strOrganizationGuid);
                    // Get VmEventGuid
                    StructuredBuffer oVm(this->RegisterVmAuditEvent(oEventInformation));
                    if (201 == oVm.GetDword("Status"))
                    {
                        oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                        oResponse.PutString("VmEventGuid", oVm.GetGuid("VmEventGuid").ToString(eHyphensAndCurlyBraces));
                        dwStatus = 201;
                    }
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

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function RegisterVmForComputation
 * @brief Take in a VmEosb of a researcher and add a Vm branch event under RO's root event
 * @param[in] c_oRequest contains EOSB of a researcher and the VM information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 * @note
 *      This is step 3 of the register virtual machine feature. Once a data owner uploads the 
 *      requested dataset to the virtual machine, the researcher can start the computation process.
 *      An audit event gets added to RO's audit log tree.
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::RegisterVmForComputation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Take in EOSB, DigitalContractGuid, VMGuid, and VM information
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");

        // Get Digital contract guid
        std::string strDcGuid = StructuredBuffer(this->GetVmInformation(c_oRequest)).GetStructuredBuffer("VirtualMachine").GetString("DigitalContractGuid");

        // Get user information 
        StructuredBuffer oEosbRequest;
        oEosbRequest.PutBuffer("Eosb", stlEosb);
        StructuredBuffer oUserInfo(this->GetUserInfo(oEosbRequest));

        // TODO: Who can call this api? Add a check accordingly
        // Check if the Eosb is a Vm Eosb
        // Register Vm if it is a Vm Eosb
        if (EosbAccessRights::eEosb == oUserInfo.GetQword("AccessRights"))
        {
            // Get the organization guid
            std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
            // Check if Eosb is a user within the RO of the digital contract associated with the strDcGuid
            StructuredBuffer oVerifyDcRequest;
            oVerifyDcRequest.PutString("DigitalContractGuid", strDcGuid);
            oVerifyDcRequest.PutString("OrganizationGuid", strOrganizationGuid);
            oVerifyDcRequest.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVerificationStatus(this->VerifyDigitalContract(oVerifyDcRequest, false, true));
            if (200 == oVerificationStatus.GetDword("Status"))
            {
                if (true == oVerificationStatus.GetBoolean("IsEqual"))
                {
                    // Check whether DC branch event log exists for DCGuid in the database or not
                    // If it does then, create a Vm branch event log
                    StructuredBuffer oEventInformation;
                    oEventInformation.PutBuffer("Eosb", stlEosb);
                    oEventInformation.PutString("DigitalContractGuid", strDcGuid);
                    oEventInformation.PutString("VirtualMachineGuid", strVmGuid);
                    oEventInformation.PutString("OrganizationGuid", strOrganizationGuid);
                    // Get VmEventGuid
                    StructuredBuffer oVm(this->RegisterVmAuditEvent(oEventInformation));
                    if (201 == oVm.GetDword("Status"))
                    {
                        oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                        oResponse.PutString("VmEventGuid", oVm.GetGuid("VmEventGuid").ToString(eHyphensAndCurlyBraces));
                        dwStatus = 201;
                    }
                }
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

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function RegisterVmAuditEvent
 * @brief Add VM branch event to the database
 * @param[in] c_oRequest contains the VM information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::RegisterVmAuditEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    Socket * poIpcAuditLogManager = nullptr;

    try 
    {
        // Take in EOSB, DigitalContractGuid, VMGuid, and VM information
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");

        // Check whether DC branch event log exists for DCGuid in the database or not
        StructuredBuffer oGetDcBranchEventRequest;
        oGetDcBranchEventRequest.PutBuffer("Eosb", stlEosb);
        oGetDcBranchEventRequest.PutDword("TransactionType", 0x00000003);
        oGetDcBranchEventRequest.PutString("OrganizationGuid", strOrganizationGuid);
        StructuredBuffer oFilters;
        oFilters.PutString("DCGuid", strDcGuid);
        oGetDcBranchEventRequest.PutStructuredBuffer("Filters", oFilters);

        // Call AuditLogManager plugin to get the guid of DC event log
        std::string strDcEventGuid;
        poIpcAuditLogManager = ::ConnectToUnixDomainSocket("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
        StructuredBuffer oDCEventLog(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oGetDcBranchEventRequest, false));
        poIpcAuditLogManager->Release();
        poIpcAuditLogManager = nullptr;
        _ThrowBaseExceptionIf((0 > oDCEventLog.GetSerializedBufferRawDataSizeInBytes()), "Error checking for DC event", nullptr);
        
        if (200 == oDCEventLog.GetDword("Status"))
        {
            strDcEventGuid = oDCEventLog.GetString("DCEventGuid");
            // Create a Vm branch event log
            StructuredBuffer oVmBranchEvent;
            oVmBranchEvent.PutDword("TransactionType", 0x00000001);
            oVmBranchEvent.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVmMetadata;
            Guid oVmEventGuid(eAuditEventBranchNode);
            oVmMetadata.PutString("EventGuid", oVmEventGuid.ToString(eHyphensAndCurlyBraces));
            oVmMetadata.PutString("ParentGuid", strDcEventGuid);
            oVmMetadata.PutString("OrganizationGuid", strOrganizationGuid);
            oVmMetadata.PutQword("EventType", 2); // where 2 is for non root event type
            oVmMetadata.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
            StructuredBuffer oPlainTextMetadata;
            oPlainTextMetadata.PutDword("BranchType", 2); // where 2 is for for Vm branch type
            oPlainTextMetadata.PutString("GuidOfDcOrVm", strVmGuid);
            oVmMetadata.PutStructuredBuffer("PlainTextEventData", oPlainTextMetadata);
            oVmBranchEvent.PutStructuredBuffer("NonLeafEvent", oVmMetadata);
            // Call AuditLogManager plugin to create a Vm event log
            poIpcAuditLogManager = ::ConnectToUnixDomainSocket("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
            StructuredBuffer oVmEventLog(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oVmBranchEvent, false));
            poIpcAuditLogManager->Release();
            poIpcAuditLogManager = nullptr;
            if ((0 < oVmEventLog.GetSerializedBufferRawDataSizeInBytes())&&(201 == oVmEventLog.GetDword("Status")))
            {
                // Add Vm branch log event guid to the response
                oResponse.PutGuid("VmEventGuid", oVmEventGuid);
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

    if (nullptr != poIpcAuditLogManager)
    {
        poIpcAuditLogManager->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function UpdateVirtualMachineStatus
 * @brief Update the status of the VM
 * @param[in] c_oRequest contains the Eosb and VM information
 * @returns StructuredBuffer containing status of the VM
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::UpdateVirtualMachineStatus(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;
    Socket * poIpcDigitalContractManager = nullptr;

    try
    {
        // Get user information
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Pull virtual machine and then update the structured buffer
            StructuredBuffer oVmInformation = this->GetVmInformation(c_oRequest);
            if (200 == oVmInformation.GetDword("Status"))
            {
                StructuredBuffer oVmBlob = oVmInformation.GetStructuredBuffer("VirtualMachine");
                // Add new VM information from the request
                Dword dwState = c_oRequest.GetDword("State");
                oVmBlob.PutDword("State", dwState);
                if (VirtualMachineState::eInUse == dwState)
                {
                    oVmBlob.PutString("VMLoggedInUser", c_oRequest.GetString("VMLoggedInUser"));
                }
                oVmBlob.PutUnsignedInt64("HeartbeatBroadcastTime", ::GetEpochTimeInSeconds());
                // Add a note if it is there
                if (true == c_oRequest.IsElementPresent("Note", ANSI_CHARACTER_STRING_VALUE_TYPE))
                {
                    oVmBlob.PutString("Note", c_oRequest.GetString("Note"));
                }
                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                // Create a request to get the list of VMs
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateVirtualMachine");
                oRequest.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
                oRequest.PutStructuredBuffer("VirtualMachineData", oVmBlob);
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

                StructuredBuffer oDatabaseResponse(stlResponse);
                if (200 == oDatabaseResponse.GetDword("Status"))
                {
                    // If the virtual machine is waiting for data, then add its information
                    // to VirtualMachinesWaitingForData collection. When the relevant remote data connector
                    // sends its heart beat, the ip address of the VM will be sent to the connector and it
                    // will upload the dataset
                    if (VirtualMachineState::eWaitingForData == dwState)
                    {
                        // Call DigitalContractDatabase plugin to get the digital contract associated with the digital contract guid
                        StructuredBuffer oDigitalContractRequest;
                        oDigitalContractRequest.PutDword("TransactionType", 0x00000001);
                        oDigitalContractRequest.PutString("DigitalContractGuid", oVmBlob.GetString("DigitalContractGuid"));
                        oDigitalContractRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                        poIpcDigitalContractManager = ::ConnectToUnixDomainSocket("/tmp/{BC5AEAAF-E37E-4605-B074-F9DF2E82CD34}");
                        StructuredBuffer oDigitalContractResponse(::PutIpcTransactionAndGetResponse(poIpcDigitalContractManager, oDigitalContractRequest, false));
                        poIpcDigitalContractManager->Release();
                        poIpcDigitalContractManager = nullptr;
                        if ((0 < oDigitalContractResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oDigitalContractResponse.GetDword("Status")))
                        {
                            // Make a Tls connection with the database portal
                            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                            // Create a request to get the list of VMs
                            oRequest.PutString("PluginName", "DatabaseManager");
                            oRequest.PutString("Verb", "POST");
                            oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterVmAsWaitingForData");
                            oRequest.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
                            oRequest.PutString("IPAddress", oVmBlob.GetString("IPAddress"));
                            oRequest.PutString("DatasetGuid", oDigitalContractResponse.GetStructuredBuffer("DigitalContract").GetString("DatasetGuid"));
                            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                            // Send request packet
                            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                            // Read header and body of the response
                            stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
                            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                            unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                            stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
                            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                            // Make sure to release the poTlsNode
                            poTlsNode->Release();
                            poTlsNode = nullptr;

                            oDatabaseResponse = stlResponse;
                            if (201 == oDatabaseResponse.GetDword("Status"))
                            {
                                dwStatus = 200;
                            }
                        }
                    }
                    else if (VirtualMachineState::eReadyForComputation == dwState)
                    {
                        // If the virtual machine has the datasetand is ready for computation, then delete the document from the VirtualMachinesWaitingForData collection
                        // Make a Tls connection with the database portal
                        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                        // Create a request to get the list of VMs
                        oRequest.PutString("PluginName", "DatabaseManager");
                        oRequest.PutString("Verb", "DELETE");
                        oRequest.PutString("Resource", "/SAIL/DatabaseManager/RemoveVmAsWaitingForData");
                        oRequest.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
                        std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
                        // Send request packet
                        poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

                        // Read header and body of the response
                        stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 2000);
                        _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
                        unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
                        stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 2000);
                        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
                        // Make sure to release the poTlsNode
                        poTlsNode->Release();
                        poTlsNode = nullptr;

                        oDatabaseResponse = stlResponse;
                        if (200 == oDatabaseResponse.GetDword("Status"))
                        {
                            dwStatus = 200;
                        }
                    }
                    else
                    {
                        dwStatus = 200;
                    }
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
    if (nullptr != poIpcDigitalContractManager)
    {
        poIpcDigitalContractManager->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}


/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function AddVirtualMachineIpAddress
 * @brief Update the status of the VM
 * @param[in] c_oRequest contains the Eosb and VM information
 * @returns StructuredBuffer containing status of the VM
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::AddVirtualMachineIpAddress(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;
    Socket * poIpcDigitalContractManager = nullptr;

    try
    {
        // Get user information
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Pull virtual machine and then update the structured buffer
            StructuredBuffer oVmInformation = this->GetVmInformation(c_oRequest);
            if (200 == oVmInformation.GetDword("Status"))
            {
                StructuredBuffer oVmBlob = oVmInformation.GetStructuredBuffer("VirtualMachine");
                // Add new VM information from the request
                oVmBlob.PutString("IPAddress", c_oRequest.GetString("IPAddress"));

                // Make a Tls connection with the database portal
                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                StructuredBuffer oRequest;
                oRequest.PutString("PluginName", "DatabaseManager");
                oRequest.PutString("Verb", "PUT");
                oRequest.PutString("Resource", "/SAIL/DatabaseManager/UpdateVirtualMachine");
                oRequest.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
                oRequest.PutStructuredBuffer("VirtualMachineData", oVmBlob);
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
    if (nullptr != poIpcDigitalContractManager)
    {
        poIpcDigitalContractManager->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}