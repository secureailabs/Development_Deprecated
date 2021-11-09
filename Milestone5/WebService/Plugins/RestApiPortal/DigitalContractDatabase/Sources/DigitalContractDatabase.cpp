/*********************************************************************************************
 *
 * @file DigitalContractDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 03 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DigitalContractDatabase.h"
#include "DateAndTime.h"
#include "IpcTransactionHelperFunctions.h"
#include "SmartMemoryAllocator.h"
#include "SocketClient.h"
#include "ThreadManager.h"
#include "TlsClient.h"
#include "AzureHelper.h"
#include "Guid.h"
#include "JsonValue.h"
#include "CurlRest.h"
#include "TlsTransactionHelperFunctions.h"
#include "Base64Encoder.h"

#include <algorithm>
#include <thread>

#include <unistd.h>

static DigitalContractDatabase * gs_oDigitalContractDatabase = nullptr;

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
        DigitalContractDatabase * poDigitalContractDatabase = ::GetDigitalContractDatabase();
        poDigitalContractDatabase->RunIpcServer(poIpcServerParameters->poIpcServer, poIpcServerParameters->poThreadManager);
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
        DigitalContractDatabase * poDigitalContractDatabase = ::GetDigitalContractDatabase();
        poDigitalContractDatabase->HandleIpcRequest(poIpcSocket);
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
        gs_oDigitalContractDatabase->TerminateSignalEncountered();
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
    m_fTerminationSignalEncountered = false;
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

    // Wait for all threads in the group to terminate
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("DigitalContractPluginGroup");
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
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
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

    // Add parameters for registering a digital contract
    // Name, ElementType, and Range (if exists) are used by RestFrameworkRuntimeData::RunThread to vet request parameters.
    // Required parameters are marked by setting IsRequired to true
    // Otherwise the parameter is optional
    StructuredBuffer oRegisterDc;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oGuid;
    oGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oGuid.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("DataOwnerOrganization", oGuid);
    StructuredBuffer oTitle;
    oTitle.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oTitle.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("Title", oTitle);
    StructuredBuffer oDescription;
    oDescription.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDescription.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("Description", oDescription);
    StructuredBuffer oVersionNumber;
    oVersionNumber.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVersionNumber.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("VersionNumber", oVersionNumber);
    StructuredBuffer oSubscriptionDays;
    oSubscriptionDays.PutByte("ElementType", UINT64_VALUE_TYPE);
    oSubscriptionDays.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("SubscriptionDays", oSubscriptionDays);
    oRegisterDc.PutStructuredBuffer("DatasetGuid", oGuid);
    StructuredBuffer oLegalAgreement;
    oLegalAgreement.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oLegalAgreement.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("LegalAgreement", oLegalAgreement);
    StructuredBuffer oDatasetDRMMetadataSize;
    oDatasetDRMMetadataSize.PutByte("ElementType", UINT32_VALUE_TYPE);
    oDatasetDRMMetadataSize.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("DatasetDRMMetadataSize", oDatasetDRMMetadataSize);
    StructuredBuffer oDatasetDRMMetadata;
    oDatasetDRMMetadata.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oDatasetDRMMetadata.PutBoolean("IsRequired", true);
    oRegisterDc.PutStructuredBuffer("DatasetDRMMetadata", oDatasetDRMMetadata);

    // Add parameters for digital contract acceptance
    StructuredBuffer oDcAcceptance;
    oDcAcceptance.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oDcGuid;
    oDcGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDcGuid.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("DigitalContractGuid", oDcGuid);
    oDescription.PutBoolean("IsRequired", false);
    oDcAcceptance.PutStructuredBuffer("Description", oDescription);
    StructuredBuffer oRetentionTime;
    oRetentionTime.PutByte("ElementType", UINT64_VALUE_TYPE);
    oRetentionTime.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("RetentionTime", oRetentionTime);
    oDcAcceptance.PutStructuredBuffer("LegalAgreement", oLegalAgreement);
    StructuredBuffer oHostForVM;
    oHostForVM.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oHostForVM.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("HostForVirtualMachines", oHostForVM);
    StructuredBuffer oNumberOfVM;
    oNumberOfVM.PutByte("ElementType", UINT64_VALUE_TYPE);
    oNumberOfVM.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("NumberOfVirtualMachines", oNumberOfVM);
    StructuredBuffer oNumberOfVCPUs;
    oNumberOfVCPUs.PutByte("ElementType", UINT64_VALUE_TYPE);
    oNumberOfVCPUs.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("NumberOfVCPU", oNumberOfVCPUs);
    StructuredBuffer oHostRegion;
    oHostRegion.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oHostRegion.PutBoolean("IsRequired", true);
    oDcAcceptance.PutStructuredBuffer("HostRegion", oHostRegion);

    // Add parameters for digital contract activation
    StructuredBuffer oDcActivation;
    oDcActivation.PutStructuredBuffer("Eosb", oEosb);
    oDcActivation.PutStructuredBuffer("DigitalContractGuid", oDcGuid);
    oDcAcceptance.PutStructuredBuffer("Description", oDescription);

    // Add parameters for associating list of digital contract(s) with an Azure template
    StructuredBuffer oAssociateWithAzureTemplate;
    oAssociateWithAzureTemplate.PutStructuredBuffer("Eosb", oEosb);
    StructuredBuffer oListOfDc;
    oListOfDc.PutByte("ElementType", INDEXED_BUFFER_VALUE_TYPE);
    oListOfDc.PutBoolean("IsRequired", true);
    oAssociateWithAzureTemplate.PutStructuredBuffer("ListOfDigitalContracts", oListOfDc);
    StructuredBuffer oAzureTemplateGuid;
    oAzureTemplateGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oAzureTemplateGuid.PutBoolean("IsRequired", true);
    oAssociateWithAzureTemplate.PutStructuredBuffer("AzureTemplateGuid", oAzureTemplateGuid);

    // Add parameters for getting list of digital contracts
    StructuredBuffer oListDc;
    oListDc.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for getting information of a digital contract
    StructuredBuffer oPullDc;
    oPullDc.PutStructuredBuffer("Eosb", oEosb);
    oPullDc.PutStructuredBuffer("DigitalContractGuid", oDcGuid);

    // Add parameters for getting provisioning status of a digital contract
    StructuredBuffer oDcProvisioningStatus;
    oDcProvisioningStatus.PutStructuredBuffer("Eosb", oEosb);
    oDcProvisioningStatus.PutStructuredBuffer("DigitalContractGuid", oDcGuid);

    StructuredBuffer oUpdateProvisioningState;
    StructuredBuffer oProvisioningState;
    oProvisioningState.PutByte("ElementType", DWORD_VALUE_TYPE);
    oProvisioningState.PutBoolean("IsRequired", true);
    oUpdateProvisioningState.PutStructuredBuffer("ProvisioningStatus", oProvisioningState);
    oUpdateProvisioningState.PutStructuredBuffer("Eosb", oEosb);
    oUpdateProvisioningState.PutStructuredBuffer("DigitalContractGuid", oDcGuid);

    // Parameters to the Dictionary: Verb, Resource, Parameters, 0 or 1 to represent if the API uses any unix connections
    // Takes in an EOSB and create a digital contract for a chosen dataset
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DigitalContractManager/Applications", oRegisterDc, 1);
    // Update the digital contract when a data owner accepts the digital contract
    m_oDictionary.AddDictionaryEntry("PATCH", "/SAIL/DigitalContractManager/DataOwner/Accept", oDcAcceptance, 1);
    // Update the digital contract when a researcher accepts the DC terms from the Data owner organization
    m_oDictionary.AddDictionaryEntry("PATCH", "/SAIL/DigitalContractManager/Researcher/Activate", oDcActivation, 1);
    // Associate one or more digital contracts with one Azure template
    m_oDictionary.AddDictionaryEntry("PATCH", "/SAIL/DigitalContractManager/AssociateWithAzureTemplate", oAssociateWithAzureTemplate, 1);
    // Update the Provsioning status of the Virtual Machine
    m_oDictionary.AddDictionaryEntry("PATCH", "/SAIL/DigitalContractManager/UpdateProvisioningStatus", oUpdateProvisioningState, 1);
    // Get a list of digital contracts associated with a researcher or a data owner
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DigitalContractManager/DigitalContracts", oListDc, 1);
    // Get a digital contract's information
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DigitalContractManager/PullDigitalContract", oPullDc, 1);
    // Get a digital contract's provisioning status
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DigitalContractManager/GetProvisioningStatus", oDcProvisioningStatus, 1);
    // Deprovision a Digital Contract and delete it's corresponding resources
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DigitalContractManager/Deprovision", oDcProvisioningStatus, 1);

    // Provision a digital contract
    StructuredBuffer oDcProvision;
    oDcProvision.PutStructuredBuffer("Eosb", oEosb);
    oDcProvision.PutStructuredBuffer("DigitalContractGuid", oDcGuid);
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DigitalContractManager/Provision", oDcProvision, 1);

    StructuredBuffer oInitializeVm;
    oInitializeVm.PutStructuredBuffer("Eosb", oEosb);

    StructuredBuffer oDatasetGuid;
    oDatasetGuid.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oDatasetGuid.PutBoolean("IsRequired", true);
    oInitializeVm.PutStructuredBuffer("DatasetGuid", oDatasetGuid);

    StructuredBuffer oVirtualMachineIp;
    oVirtualMachineIp.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oVirtualMachineIp.PutBoolean("IsRequired", true);
    oInitializeVm.PutStructuredBuffer("VirtualMachineIp", oVirtualMachineIp);

    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DigitalContractManager/InitializeVm", oInitializeVm, 1);

    // Start the Ipc server
    // Start listening for Ipc connections
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{BC5AEAAF-E37E-4605-B074-F9DF2E82CD34}");
    IpcServerParameters * poIpcServerParameters = (IpcServerParameters *) gs_oMemoryAllocator.Allocate(sizeof(IpcServerParameters), true);
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);

    // Initialize IpcServerParameters struct
    poIpcServerParameters->poThreadManager = poThreadManager;
    poIpcServerParameters->poIpcServer = poIpcServer;
    poThreadManager->CreateThread("DigitalContractPluginGroup", StartIpcServerThread, (void *) poIpcServerParameters);
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function RunIpcServer
 * @brief Run Ipc server for incoming Ipc requests
 * @param[in] poIpcServer Pointer to Socket server
 * @param[in] poThreadManager Pointer to the thread manager object
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::RunIpcServer(
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
 * @class DigitalContractDatabase
 * @function HandleIpcRequest
 * @brief Handles an incoming Ipc request and call the relevant function based on the identifier
 * @param[in] poSocket Pointer to socket instance
 *
 ********************************************************************************************/
void __thiscall DigitalContractDatabase::HandleIpcRequest(
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
        case 0x00000001 // PullDigitalContract
        :
            stlResponse = this->PullDigitalContract(oRequestParameters);
        break;
        case 0x00000002 // ListDigitalContracts
        :
            stlResponse = this->ListDigitalContracts(oRequestParameters);
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
        if ("/SAIL/DigitalContractManager/DigitalContracts" == strResource)
        {
            stlResponseBuffer = this->ListDigitalContracts(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/PullDigitalContract" == strResource)
        {
            stlResponseBuffer = this->PullDigitalContract(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/GetProvisioningStatus" == strResource)
        {
            stlResponseBuffer = this->GetProvisioningStatus(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/DigitalContractManager/Applications" == strResource)
        {
            stlResponseBuffer = this->RegisterDigitalContract(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/Provision" == strResource)
        {
            stlResponseBuffer = this->ProvisionDigitalContract(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/InitializeVm" == strResource)
        {
            stlResponseBuffer = this->InitializeVirtualMachine(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/Deprovision" == strResource)
        {
            stlResponseBuffer = this->DeprovisionDigitalContract(c_oRequestStructuredBuffer);
        }
    }
    else if ("PATCH" == strVerb)
    {
        if ("/SAIL/DigitalContractManager/DataOwner/Accept" == strResource)
        {
            stlResponseBuffer = this->AcceptDigitalContract(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/Researcher/Activate" == strResource)
        {
            stlResponseBuffer = this->ActivateDigitalContract(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/AssociateWithAzureTemplate" == strResource)
        {
            stlResponseBuffer = this->AssociateWithAzureTemplate(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/DigitalContractManager/UpdateProvisioningStatus" == strResource)
        {
            stlResponseBuffer = this->UpdateDigitalContractProvisioningStatus(c_oRequestStructuredBuffer);
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
 * @function GetUserInfo
 * @brief Take in a full EOSB and send back a StructuredBuffer containing user metadata
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing user metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetUserInfo(
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

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetDigitalSignature
 * @brief Fetch digital signature blob for the given content
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the digital signature
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetDigitalSignature(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    std::vector<Byte> stlContent = c_oRequest.GetBuffer("Content");

    StructuredBuffer oDecryptEosbRequest;
    oDecryptEosbRequest.PutDword("TransactionType", 0x00000004);
    oDecryptEosbRequest.PutBuffer("MessageDigest", stlContent);

    // Call CryptographicManager plugin to get the digital signature blob
    Socket * poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
    StructuredBuffer oPluginResponse(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest, false));
    poIpcCryptographicManager->Release();
    poIpcCryptographicManager = nullptr;
    if ((0 < oPluginResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oPluginResponse.GetDword("Status")))
    {
        oResponse.PutStructuredBuffer("DSIG", oPluginResponse.GetStructuredBuffer("DSIG"));
    }
    else
    {
        _ThrowBaseException("Error getting digital signatures.", nullptr);
    }

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function SerializeDigitalContract
 * @brief Serialize a digital contract structured buffer
 * @param[in] c_oRequest contains the digital contract structured buffer
 * @returns Serialized digital contract blob
 *
 ********************************************************************************************/
 
 void __thiscall DigitalContractDatabase::SerializeDigitalContract(
    _in const StructuredBuffer & c_oDc,
    _in std::vector<Byte> & stlDigitalContractBlob
    )
{
    __DebugFunction();

    // The format of the digital contract blob is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xF62DE0021B48A123                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfDigitalContractStructuredBuffer                                    |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfDigitalContractStructuredBuffer] DigitalContractStructuredBuffer             |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xFFFFFFFFFFFFFFFF                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfDOORootKeyDSIG                                                     |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfRORootKeyDSIG                                                      |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfDOORootKeyDSIG] DOORootKeyDSIG                                               |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfRORootKeyDSIG] RORootKeyDSIG                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xFFFFFFFFFFFFFFFF                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfSailDSIG                                                           |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfSailDSIG] SailDSIG                                                           |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0x321A84B1200ED26F                                                                 |
    // +------------------------------------------------------------------------------------+

    uint32_t unSizeInBytesOfDigitalContractStructuredBuffer = c_oDc.GetSerializedBufferRawDataSizeInBytes();
    // Calculate size of the digital contract blob up to the first marker
    const unsigned int unSizeOfDcBlobFirstPart = sizeof(Qword) + sizeof(uint32_t) + unSizeInBytesOfDigitalContractStructuredBuffer + sizeof(Qword);
    stlDigitalContractBlob.resize(unSizeOfDcBlobFirstPart);
    Byte * pbSerializedBuffer = (Byte *) stlDigitalContractBlob.data();
    *((Qword *) pbSerializedBuffer) = 0xF62DE0021B48A123;
    pbSerializedBuffer += sizeof(Qword);
    *((uint32_t *) pbSerializedBuffer) =(uint32_t) unSizeInBytesOfDigitalContractStructuredBuffer;
    pbSerializedBuffer += sizeof(uint32_t);
    if (0 < unSizeInBytesOfDigitalContractStructuredBuffer)
    {
        ::memcpy((void *) pbSerializedBuffer, (const void *) c_oDc.GetSerializedBufferRawDataPtr(), unSizeInBytesOfDigitalContractStructuredBuffer);
        pbSerializedBuffer += unSizeInBytesOfDigitalContractStructuredBuffer;
    }
    *((Qword *) pbSerializedBuffer) = 0xFFFFFFFFFFFFFFFF;
    pbSerializedBuffer += sizeof(Qword);
    // TODO: calculate digital signatures for the real root keys of DOO, RO, and Sail
    StructuredBuffer oContent;
    oContent.PutBuffer("Content", stlDigitalContractBlob);
    // Get DOO root key digital signature
    StructuredBuffer oDOORootKeyDsig = StructuredBuffer(this->GetDigitalSignature(oContent)).GetStructuredBuffer("DSIG");
    StructuredBuffer oRORootKeyDsig = StructuredBuffer(this->GetDigitalSignature(oContent)).GetStructuredBuffer("DSIG");
    // Calculate size of the digital contract blob up to the second marker
    unsigned int unSizeInBytesOfDOORootKeyDSIG = oDOORootKeyDsig.GetSerializedBufferRawDataSizeInBytes();
    unsigned int unSizeInBytesOfRORootKeyDSIG = oRORootKeyDsig.GetSerializedBufferRawDataSizeInBytes();
    const unsigned int unSizeOfDcBlobSecondPart = sizeof(uint32_t) + sizeof(uint32_t) + unSizeInBytesOfDOORootKeyDSIG + unSizeInBytesOfRORootKeyDSIG + sizeof(Qword);
    stlDigitalContractBlob.resize(unSizeOfDcBlobFirstPart + unSizeOfDcBlobSecondPart);
    pbSerializedBuffer = (Byte *) (stlDigitalContractBlob.data() + unSizeOfDcBlobFirstPart);
    *((uint32_t *) pbSerializedBuffer) =(uint32_t) unSizeInBytesOfDOORootKeyDSIG;
    pbSerializedBuffer += sizeof(uint32_t);
    *((uint32_t *) pbSerializedBuffer) =(uint32_t) unSizeInBytesOfRORootKeyDSIG;
    pbSerializedBuffer += sizeof(uint32_t);
    if (0 < unSizeInBytesOfDOORootKeyDSIG)
    {
        ::memcpy((void *) pbSerializedBuffer, (const void *) oDOORootKeyDsig.GetSerializedBufferRawDataPtr(), unSizeInBytesOfDOORootKeyDSIG);
        pbSerializedBuffer += unSizeInBytesOfDOORootKeyDSIG;
    }
    if (0 < unSizeInBytesOfRORootKeyDSIG)
    {
        ::memcpy((void *) pbSerializedBuffer, (const void *) oRORootKeyDsig.GetSerializedBufferRawDataPtr(), unSizeInBytesOfRORootKeyDSIG);
        pbSerializedBuffer += unSizeInBytesOfRORootKeyDSIG;
    }
    *((Qword *) pbSerializedBuffer) = 0xFFFFFFFFFFFFFFFF;
    pbSerializedBuffer += sizeof(Qword);
    // Get Sail digital signature
    oContent.PutBuffer("Content", stlDigitalContractBlob);
    StructuredBuffer oSailDsig = StructuredBuffer(this->GetDigitalSignature(oContent)).GetStructuredBuffer("DSIG");
    // Calculate size of the digital contract blob up to the footer
    unsigned int unSizeInBytesOfSailDSIG = oSailDsig.GetSerializedBufferRawDataSizeInBytes();
    const unsigned int unSizeOfDcBlobThirdPart = sizeof(uint32_t) + unSizeInBytesOfSailDSIG + sizeof(Qword);
    stlDigitalContractBlob.resize(unSizeOfDcBlobFirstPart + unSizeOfDcBlobSecondPart + unSizeOfDcBlobThirdPart);
    pbSerializedBuffer = (Byte *) (stlDigitalContractBlob.data() + unSizeOfDcBlobFirstPart + unSizeOfDcBlobSecondPart);
    *((uint32_t *) pbSerializedBuffer) =(uint32_t) unSizeInBytesOfSailDSIG;
    pbSerializedBuffer += sizeof(uint32_t);
    if (0 < unSizeInBytesOfSailDSIG)
    {
        ::memcpy((void *) pbSerializedBuffer, (const void *) oSailDsig.GetSerializedBufferRawDataPtr(), unSizeInBytesOfSailDSIG);
        pbSerializedBuffer += unSizeInBytesOfSailDSIG;
    }
    *((Qword *) pbSerializedBuffer) = 0x321A84B1200ED26F;
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function DeserializeDigitalContract
 * @brief Deserialize a digital contract blob
 * @param[in] c_oRequest contains the serialized digital contract
 * @returns Deserialized digital contract structured buffer
 *
 ********************************************************************************************/
 
 StructuredBuffer __thiscall DigitalContractDatabase::DeserializeDigitalContract(
    _in const std::vector<Byte> c_stlDcBlob
    )
{
    __DebugFunction();

    // The format of the digital contract blob is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xF62DE0021B48A123                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfDigitalContractStructuredBuffer                                    |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfDigitalContractStructuredBuffer] DigitalContractStructuredBuffer             |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xFFFFFFFFFFFFFFFF                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfDOORootKeyDSIG                                                     |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfRORootKeyDSIG                                                      |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfDOORootKeyDSIG] DOORootKeyDSIG                                               |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfRORootKeyDSIG] RORootKeyDSIG                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0xFFFFFFFFFFFFFFFF                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfSailDSIG                                                           |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfSailDSIG] SailDSIG                                                           |
    // +------------------------------------------------------------------------------------+
    // | [Qword] 0x321A84B1200ED26F                                                                 |
    // +------------------------------------------------------------------------------------+
    std::vector<Byte> stlSsb;
    unsigned int unResizeSize = 0;
    const Byte * pbCurrentByte = (Byte *) c_stlDcBlob.data();
    _ThrowBaseExceptionIf((0xF62DE0021B48A123 != *((Qword *) pbCurrentByte)), "Invalid serialization format: EXPECTED [HEADER] = 0xF62DE0021B48A123 but got 0x%016X", *((Qword *) pbCurrentByte));
    pbCurrentByte += sizeof(Qword);
    unsigned int unSizeInBytesOfDigitalContractStructuredBuffer = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    unResizeSize = unSizeInBytesOfDigitalContractStructuredBuffer;
    if (0 < unSizeInBytesOfDigitalContractStructuredBuffer)
    {
        stlSsb.resize(unResizeSize);
        ::memcpy((void *) stlSsb.data(), (const void *) pbCurrentByte, unSizeInBytesOfDigitalContractStructuredBuffer);
        pbCurrentByte += unSizeInBytesOfDigitalContractStructuredBuffer;
    }
    _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF != *((Qword *) pbCurrentByte)), "Invalid serialization format: EXPECTED [MARKER] = 0xFFFFFFFFFFFFFFFF but got 0x%016X", *((Qword *) pbCurrentByte));
    pbCurrentByte += sizeof(Qword);
    unsigned int unSizeInBytesOfDOORootKeyDSIG = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    unsigned int unSizeInBytesOfRORootKeyDSIG = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    if (0 < unSizeInBytesOfDOORootKeyDSIG)
    {
        stlSsb.resize(unResizeSize + unSizeInBytesOfDOORootKeyDSIG);
        ::memcpy((void *) (stlSsb.data() + unResizeSize), (const void *) pbCurrentByte, unSizeInBytesOfDOORootKeyDSIG);
        pbCurrentByte += unSizeInBytesOfDOORootKeyDSIG;
    }
    unResizeSize += unSizeInBytesOfDOORootKeyDSIG;
    if (0 < unSizeInBytesOfRORootKeyDSIG)
    {
        stlSsb.resize(unResizeSize + unSizeInBytesOfRORootKeyDSIG);
        ::memcpy((void *) (stlSsb.data() + unResizeSize), (const void *) pbCurrentByte, unSizeInBytesOfRORootKeyDSIG);
        pbCurrentByte += unSizeInBytesOfRORootKeyDSIG;
    }
    unResizeSize += unSizeInBytesOfRORootKeyDSIG;
    _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF != *((Qword *) pbCurrentByte)), "Invalid serialization format: EXPECTED [MARKER] = 0xFFFFFFFFFFFFFFFF but got 0x%016X", *((Qword *) pbCurrentByte));
    pbCurrentByte += sizeof(Qword);
    unsigned int unSizeInBytesOfSailDSIG = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    if (0 < unSizeInBytesOfSailDSIG)
    {
        stlSsb.resize(unResizeSize + unSizeInBytesOfSailDSIG);
        ::memcpy((void *) (stlSsb.data() + unResizeSize), (const void *) pbCurrentByte, unSizeInBytesOfSailDSIG);
        pbCurrentByte += unSizeInBytesOfSailDSIG;
    }
    _ThrowBaseExceptionIf((0x321A84B1200ED26F != *((Qword *) pbCurrentByte)), "Invalid serialization format: EXPECTED [FOOTER] = 0x321A84B1200ED26F but got 0x%016X", *((Qword *) pbCurrentByte));

    return StructuredBuffer(stlSsb);
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ListDigitalContracts
 * @brief Fetch list of all digital contracts associated with the user's organization
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of digital contracts associated with the user's organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::ListDigitalContracts(
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
            // Create a request to list all the digital contracts for the user organization in the database
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/ListDigitalContracts");
            oRequest.PutString("UserOrganization", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
            std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

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
            if (404 != oDatabaseResponse.GetDword("Status"))
            {
                StructuredBuffer oDigitalContracts = oDatabaseResponse.GetStructuredBuffer("DigitalContracts");
                for (std::string strDcGuid : oDigitalContracts.GetNamesOfElements())
                {
                    StructuredBuffer oDigitalContractRecord = oDigitalContracts.GetStructuredBuffer(strDcGuid.c_str());
                    StructuredBuffer oDigitalContract = this->DeserializeDigitalContract(oDigitalContractRecord.GetStructuredBuffer("DigitalContract").GetBuffer("DigitalContractBlob"));
                    // Add digital contract, RO guid, and DOO guid to the response
                    oDigitalContract.PutString("ROName", oDigitalContractRecord.GetString("ROName"));
                    oDigitalContract.PutString("DOOName", oDigitalContractRecord.GetString("DOOName"));
                    oDigitalContract.PutString("ResearcherOrganization", oDigitalContractRecord.GetString("ResearcherOrganization"));
                    oDigitalContract.PutString("DataOwnerOrganization", oDigitalContractRecord.GetString("DataOwnerOrganization"));
                    oDigitalContracts.PutStructuredBuffer(strDcGuid.c_str(), oDigitalContract);
                }
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                oResponse.PutStructuredBuffer("DigitalContracts", oDigitalContracts);
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
 * @class DigitalContractDatabase
 * @function PullDigitalContract
 * @brief Fetch the digital contract information
 * @param[in] c_oRequest contains the digital contract guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the digital contract information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::PullDigitalContract(
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
            // TODO: Add a check if the api has to be restricted
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get the digital contract information
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/PullDigitalContract");
            oRequest.PutString("DigitalContractGuid", c_oRequest.GetString("DigitalContractGuid"));
            oRequest.PutString("UserOrganization", oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces));
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
                std::vector<Byte> stlDcBlob = oDatabaseResponse.GetStructuredBuffer("DigitalContract").GetBuffer("DigitalContractBlob");
                // Deserialize the Digital contract blob and get the DC information structured buffer
                StructuredBuffer oDigitalContract = this->DeserializeDigitalContract(stlDcBlob);
                // Add digital contract, RO guid, and DOO guid to the response
                oResponse.PutStructuredBuffer("DigitalContract", oDigitalContract);
                oResponse.PutString("ROName", oDatabaseResponse.GetString("ROName"));
                oResponse.PutString("DOOName", oDatabaseResponse.GetString("DOOName"));
                oResponse.PutString("ResearcherOrganization", oDatabaseResponse.GetString("ResearcherOrganization"));
                oResponse.PutString("DataOwnerOrganization", oDatabaseResponse.GetString("DataOwnerOrganization"));
                // Add the updated Eosb
                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
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
 * @class DigitalContractDatabase
 * @function RegisterDigitalContract
 * @brief Take in full EOSB and register a digital contract
 * @param[in] c_oRequest contains the digital contract information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::RegisterDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get User guid
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Create digital contract guid
            std::string strDcGuid = Guid(eDigitalContract).ToString(eHyphensAndCurlyBraces);
            // Get organization guid
            std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
            // Get dataset guid
            std::string strDsetGuid = c_oRequest.GetString("DatasetGuid");
            // Get data owner organization guid
            std::string strDooGuid = c_oRequest.GetString("DataOwnerOrganization");
            // Create Ssb containing Dc information
            StructuredBuffer oSsb;
            oSsb.PutString("Title", c_oRequest.GetString("Title"));
            oSsb.PutString("Description", c_oRequest.GetString("Description"));
            oSsb.PutString("VersionNumber", c_oRequest.GetString("VersionNumber"));
            oSsb.PutString("DigitalContractGuid", strDcGuid);
            oSsb.PutDword("ContractStage", ContractStage::eApplication);
            oSsb.PutUnsignedInt64("SubscriptionDays", c_oRequest.GetUnsignedInt64("SubscriptionDays"));
            oSsb.PutString("DatasetGuid", strDsetGuid);
            oSsb.PutString("Eula", SAIL_EULA);
            oSsb.PutString("LegalAgreement", c_oRequest.GetString("LegalAgreement"));
            oSsb.PutUnsignedInt32("DatasetDRMMetadataSize", c_oRequest.GetUnsignedInt32("DatasetDRMMetadataSize"));
            oSsb.PutStructuredBuffer("DatasetDRMMetadata", c_oRequest.GetStructuredBuffer("DatasetDRMMetadata"));
            oSsb.PutUnsignedInt64("LastActivity", ::GetEpochTimeInSeconds());
            oSsb.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eUnprovisioned);
            oSsb.PutString("Note", "...");
            // Get Dataset name from the database
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get the digital contract information
            StructuredBuffer oRequest;
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "GET");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/GetDatasetName");
            oRequest.PutString("DatasetGuid", strDsetGuid);
            oRequest.PutString("DataOwnerOrganization", strDooGuid);
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

            StructuredBuffer oDatasetName(stlResponse);
            _ThrowBaseExceptionIf((200 != oDatasetName.GetDword("Status")), "Dataset not found. Check dataset and data owner organization information.", nullptr);
            // Add dataset name to the response
            oSsb.PutString("DatasetName", oDatasetName.GetString("DatasetName"));

            // Get digital contract blob
            std::vector<Byte> stlDigitalContractBlob;
            this->SerializeDigitalContract(oSsb, stlDigitalContractBlob);

            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to add a digital contract to the database
            oRequest.PutString("PluginName", "DatabaseManager");
            oRequest.PutString("Verb", "POST");
            oRequest.PutString("Resource", "/SAIL/DatabaseManager/RegisterDigitalContract");
            oRequest.PutString("DigitalContractGuid", strDcGuid);
            oRequest.PutString("ResearcherOrganization", strOrganizationGuid);
            oRequest.PutString("DataOwnerOrganization", strDooGuid);
            oRequest.PutBuffer("DigitalContractBlob", stlDigitalContractBlob);
            stlRequest = ::CreateRequestPacket(oRequest);
            // Send request packet
            poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

            // Read header and body of the response
            stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 3000);
            _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
            unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
            stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
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
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
        oResponse.PutString("ErrorMessage: ", oException.GetExceptionMessage());
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
 * @class DigitalContractDatabase
 * @function AcceptDigitalContract
 * @brief Update the digital contract when a data owner accepts the digital contract
 * @param[in] c_oRequest contains user Eosb, retention time, and legal agreement
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::AcceptDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Get digital contract guid
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            Qword qwAccessRights = oUserInfo.GetQword("AccessRights");
            if ((AccessRights::eDatasetAdmin == qwAccessRights) || (AccessRights::eAdmin == qwAccessRights))
            {
                // Step 1: Get the digital contract blob and update the structure
                StructuredBuffer oDcBlob(this->PullDigitalContract(c_oRequest));
                if (200 == oDcBlob.GetDword("Status"))
                {
                    // Get user's organization guid
                    std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
                    if (oDcBlob.GetString("DataOwnerOrganization") == strOrganizationGuid)
                    {
                        StructuredBuffer oSsb(oDcBlob.GetStructuredBuffer("DigitalContract"));
                        if (ContractStage::eApplication == oSsb.GetDword("ContractStage"))
                        {
                            oSsb.PutDword("ContractStage", ContractStage::eApproval);
                            // Update the description of the digital contract if the data owner edited the description
                            if (true == c_oRequest.IsElementPresent("Description", ANSI_CHARACTER_STRING_VALUE_TYPE))
                            {
                                oSsb.PutString("Description", c_oRequest.GetString("Description"));
                            }
                            oSsb.PutUnsignedInt64("RetentionTime", c_oRequest.GetUnsignedInt64("RetentionTime"));
                            oSsb.PutString("EulaAcceptedByDOOAuthorizedUser", SAIL_EULA);
                            oSsb.PutString("LegalAgreement", c_oRequest.GetString("LegalAgreement"));
                            oSsb.PutUnsignedInt64("LastActivity", ::GetEpochTimeInSeconds());
                            oSsb.PutString("HostForVirtualMachines", c_oRequest.GetString("HostForVirtualMachines"));
                            oSsb.PutUnsignedInt64("NumberOfVirtualMachines", c_oRequest.GetUnsignedInt64("NumberOfVirtualMachines"));
                            oSsb.PutUnsignedInt64("NumberOfVCPU", c_oRequest.GetUnsignedInt64("NumberOfVCPU"));
                            oSsb.PutString("HostRegion", c_oRequest.GetString("HostRegion"));
                            oSsb.PutString("Note", "...");
                            // Serialize the update digital contract blob
                            std::vector<Byte> stlUpdatedSsb;
                            this->SerializeDigitalContract(oSsb, stlUpdatedSsb);
                            // Step 2: Call the database manager resource with the updated blob and update the database
                            // Make a Tls connection with the database portal
                            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                            // Create a request to update a digital contract to the database
                            StructuredBuffer oRequest;
                            oRequest.PutString("PluginName", "DatabaseManager");
                            oRequest.PutString("Verb", "PATCH");
                            oRequest.PutString("Resource", "/SAIL/DatabaseManager/Update/DigitalContract");
                            oRequest.PutString("DigitalContractGuid", strDcGuid);
                            oRequest.PutBuffer("DigitalContractBlob", stlUpdatedSsb);
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
            
                            // Check if DatabaseManager updated the digital contract or not
                            StructuredBuffer oDatabaseResponse(stlResponse);
                            if (204 != oDatabaseResponse.GetDword("Status"))
                            {
                                dwStatus = 200;
                                oResponse.PutString("Instructions", "Wait for activation.");
                                // Create a DC branch event for DOO
                                StructuredBuffer oEventInformation;
                                oEventInformation.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                                oEventInformation.PutString("DigitalContractGuid", strDcGuid);
                                oEventInformation.PutString("OrganizationGuid", strOrganizationGuid);
                                StructuredBuffer oDcEvent(this->RegisterDcAuditEvent(oEventInformation));
                                if ((0 < oDcEvent.GetSerializedBufferRawDataSizeInBytes())&&(201 == oDcEvent.GetDword("Status")))
                                {
                                    oResponse.PutDword("RootEventStatus", 201);
                                }
                                else
                                {
                                    oResponse.PutDword("RootEventStatus", 204);
                                }
                                // Add the updated Eosb
                                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                            }
                        }
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ActivateDigitalContract
 * @brief Update the digital contract when a researcher accepts the DC terms from the Data owner organization
 * @param[in] c_oRequest contains user Eosb, comments made by the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::ActivateDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get digital contract guid
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            Qword qwAccessRights = oUserInfo.GetQword("AccessRights");
            if ((AccessRights::eDigitalContractAdmin == qwAccessRights) || (AccessRights::eAdmin == qwAccessRights))
            {
                // Step 1: Get the digital contract blob and update the structure
                StructuredBuffer oDcBlob(this->PullDigitalContract(c_oRequest));
                if (200 == oDcBlob.GetDword("Status"))
                {
                    // Get user's organization guid
                    std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
                    if (oDcBlob.GetString("ResearcherOrganization") == strOrganizationGuid)
                    {
                        StructuredBuffer oSsb(oDcBlob.GetStructuredBuffer("DigitalContract"));
                        if (ContractStage::eApproval == oSsb.GetDword("ContractStage"))
                        {
                            uint64_t unActivationTime = ::GetEpochTimeInSeconds();
                            oSsb.PutUnsignedInt64("ActivationTime", unActivationTime);
                            // Calculate expiration time
                            uint64_t unSubscriptionDays = oSsb.GetUnsignedInt64("SubscriptionDays");
                            uint64_t unExpirationTime = unActivationTime + (unSubscriptionDays * 24 * 60 * 60);
                            oSsb.PutUnsignedInt64("ExpirationTime", unExpirationTime);
                            oSsb.PutDword("ContractStage", ContractStage::eActive);
                            if (false == oSsb.IsElementPresent("AzureTemplateGuid", ANSI_CHARACTER_STRING_VALUE_TYPE))
                            {
                                oSsb.PutString("Note", "Attach Azure Template of Hosting Organization.");
                            }
                            oSsb.PutString("EulaAcceptedByROAuthorizedUser", SAIL_EULA);
                            // Update the description of the digital contract if the researcher edited the description
                            if (true == c_oRequest.IsElementPresent("Description", ANSI_CHARACTER_STRING_VALUE_TYPE))
                            {
                                oSsb.PutString("Description", c_oRequest.GetString("Description"));
                            }
                            oSsb.PutUnsignedInt64("LastActivity", ::GetEpochTimeInSeconds());
                            // Serialize the update digital contract blob
                            std::vector<Byte> stlUpdatedSsb;
                            this->SerializeDigitalContract(oSsb, stlUpdatedSsb);
                            // Step 2: Call the database manager resource with the updated blob and update the database
                            // Make a Tls connection with the database portal
                            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                            // Create a request to update digital contract in the database
                            StructuredBuffer oRequest;
                            oRequest.PutString("PluginName", "DatabaseManager");
                            oRequest.PutString("Verb", "PATCH");
                            oRequest.PutString("Resource", "/SAIL/DatabaseManager/Update/DigitalContract");
                            oRequest.PutString("DigitalContractGuid", strDcGuid);
                            oRequest.PutBuffer("DigitalContractBlob", stlUpdatedSsb);
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

                            // Check if DatabaseManager updated the digital contract or not
                            StructuredBuffer oDatabaseResponse(stlResponse);
                            if (204 != oDatabaseResponse.GetDword("Status"))
                            {
                                dwStatus = 200;
                                oResponse.PutString("Instructions", "Wait for VM activation.");
                                // Create a DC branch event for RO
                                StructuredBuffer oEventInformation;
                                oEventInformation.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                                oEventInformation.PutString("DigitalContractGuid", strDcGuid);
                                oEventInformation.PutString("OrganizationGuid", strOrganizationGuid);
                                StructuredBuffer oDcEvent(this->RegisterDcAuditEvent(oEventInformation));
                                if ((0 < oDcEvent.GetSerializedBufferRawDataSizeInBytes())&&(201 == oDcEvent.GetDword("Status")))
                                {
                                    oResponse.PutDword("RootEventStatus", 201);
                                }
                                else
                                {
                                    oResponse.PutDword("RootEventStatus", 204);
                                }
                                // Add the updated Eosb
                                oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                            }
                        }
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

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function AssociateWithAzureTemplate
 * @brief Associate one or more digital contracts with one Azure template
 * @param[in] c_oRequest contains the list of DC and Azure template guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::AssociateWithAzureTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    std::string strErrorMessage = "Digital contracts that failed to be associated: ";
    TlsNode * poTlsNode = nullptr;
    Socket * poIpcAzureManager = nullptr;

    try
    {
        // Check if the user is an admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            if (AccessRights::eAdmin == oUserInfo.GetQword("AccessRights"))
            {
                // Get admin's organization guid
                std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
                // Get parameters
                std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
                std::string strAzureTemplateGuid = c_oRequest.GetString("AzureTemplateGuid");
                // Call AzureManager plugin to validate the Azure template
                StructuredBuffer oValidateTemplate;
                oValidateTemplate.PutDword("TransactionType", 0x00000001);
                oValidateTemplate.PutBuffer("Eosb", stlEosb);
                oValidateTemplate.PutString("TemplateGuid", strAzureTemplateGuid);
                poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4B56D0E0-7A38-40C1-839A-B9BBCDDFE521}");
                StructuredBuffer oValidateTemplateResponse(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oValidateTemplate, false));
                poIpcAzureManager->Release();
                poIpcAzureManager = nullptr;
                if ((0 < oValidateTemplateResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oValidateTemplateResponse.GetDword("Status")))
                {
                    StructuredBuffer oListOfDigitalContracts = c_oRequest.GetStructuredBuffer("ListOfDigitalContracts");
                    std::vector<std::string> stlDigitalContracts = oListOfDigitalContracts.GetNamesOfElements();
                    // Loop through the array of digital contract guids and add them to a vector
                    std::vector<std::string> stlDigitalContractGuids;
                    for (std::string strKey : stlDigitalContracts)
                    {
                        stlDigitalContractGuids.push_back(oListOfDigitalContracts.GetString(strKey.c_str()));
                    }
                    // Loop through the list of digital contracts
                    // For each digital contract, check if the admin's organization is responsible for hosting the virtual machines
                    // If yes, then associate the digital contract with the Azure template
                    // Return error otherwise
                    for (std::string strDcGuid : stlDigitalContractGuids)
                    {
                        StructuredBuffer oPullDcRequest;
                        oPullDcRequest.PutBuffer("Eosb", stlEosb);
                        oPullDcRequest.PutString("DigitalContractGuid", strDcGuid);
                        StructuredBuffer oDigitalContract = this->PullDigitalContract(oPullDcRequest);
                        if (200 == oDigitalContract.GetDword("Status"))
                        {
                            // Check if the admin's organization is responsible for hosting the virtual machines
                            bool fValid = false;
                            if (strOrganizationGuid == oDigitalContract.GetString("ResearcherOrganization"))
                            {
                                if ("Researcher" == oDigitalContract.GetStructuredBuffer("DigitalContract").GetString("HostForVirtualMachines"))
                                {
                                    fValid = true;
                                }
                            }
                            else if (strOrganizationGuid == oDigitalContract.GetString("DataOwnerOrganization"))
                            {
                                if ("Data Owner" == oDigitalContract.GetStructuredBuffer("DigitalContract").GetString("HostForVirtualMachines"))
                                {
                                    fValid = true;
                                }
                            }
                            if (true == fValid)
                            {
                                StructuredBuffer oSsb = oDigitalContract.GetStructuredBuffer("DigitalContract");
                                // Associate the Digital Contract with the Azure template guid
                                oSsb.PutString("AzureTemplateGuid", strAzureTemplateGuid);
                                // Remove the note to attach the azure template
                                oSsb.PutString("Note", "...");
                                // Serialize the update digital contract blob
                                std::vector<Byte> stlUpdatedSsb;
                                this->SerializeDigitalContract(oSsb, stlUpdatedSsb);
                                // Step 2: Call the database manager resource with the updated blob and update the database
                                // Make a Tls connection with the database portal
                                poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                                // Create a request to update a digital contract to the database
                                StructuredBuffer oRequest;
                                oRequest.PutString("PluginName", "DatabaseManager");
                                oRequest.PutString("Verb", "PATCH");
                                oRequest.PutString("Resource", "/SAIL/DatabaseManager/Update/DigitalContract");
                                oRequest.PutString("DigitalContractGuid", strDcGuid);
                                oRequest.PutBuffer("DigitalContractBlob", stlUpdatedSsb);
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

                                // Check if DatabaseManager updated the digital contract or not
                                StructuredBuffer oDatabaseResponse(stlResponse);
                                if (204 != oDatabaseResponse.GetDword("Status"))
                                {
                                    // Add the updated Eosb
                                    oResponse.PutBuffer("Eosb", oUserInfo.GetBuffer("Eosb"));
                                }
                            }
                            else
                            {
                                strErrorMessage += strDcGuid + " ";
                            }
                        }
                    }
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
    if (nullptr != poIpcAzureManager)
    {
        poIpcAzureManager->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);
    // Send back error message that will have digital contract guids that could not be associated to the template
    oResponse.PutString("ErrorMessage", strErrorMessage);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function GetProvisioningStatus
 * @brief Send back status of the digital contract provisioning
 * @param[in] c_oRequest contains the DC guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Transaction status and the provisioning status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::GetProvisioningStatus(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    Socket * poIpcVirtualMachineManager = nullptr;

    try
    {
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Get the digital contract
            StructuredBuffer oDigitalContract = this->PullDigitalContract(c_oRequest);
            if (200 == oDigitalContract.GetDword("Status"))
            {
                Dword dwProvisioningStatus = oDigitalContract.GetStructuredBuffer("DigitalContract").GetDword("ProvisioningStatus");
                oResponse.PutDword("ProvisioningStatus", dwProvisioningStatus);
                // Send back list of associated VMs and their IP addresses if the status is READY
                if ((Dword)DigitalContractProvisiongStatus::eReady == dwProvisioningStatus)
                {
                    StructuredBuffer oGetListOfVmsRequest;
                    oGetListOfVmsRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                    oGetListOfVmsRequest.PutDword("TransactionType", 0x00000001);
                    oGetListOfVmsRequest.PutString("DigitalContractGuid", c_oRequest.GetString("DigitalContractGuid"));
                    // Call VirtualMachine plugin to get the list of VMs associated with the digital contract
                    poIpcVirtualMachineManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
                    StructuredBuffer oVirtualMachines(::PutIpcTransactionAndGetResponse(poIpcVirtualMachineManager, oGetListOfVmsRequest, false));
                    poIpcVirtualMachineManager->Release();
                    poIpcVirtualMachineManager = nullptr;
                    if ((0 < oVirtualMachines.GetSerializedBufferRawDataSizeInBytes())&&(200 == oVirtualMachines.GetDword("Status")))
                    {
                        oResponse.PutStructuredBuffer("VirtualMachines", oVirtualMachines.GetStructuredBuffer("VirtualMachines"));
                    }
                }
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

    if (nullptr != poIpcVirtualMachineManager)
    {
        poIpcVirtualMachineManager->Release();
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function RegisterDcAuditEvent
 * @brief Add DC branch event to the database
 * @param[in] c_oRequest contains the DC information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::RegisterDcAuditEvent(
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
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");

        // Get the root event guid for the user organization
        StructuredBuffer oGetDcBranchEventRequest;
        oGetDcBranchEventRequest.PutDword("TransactionType", 0x00000002);
        oGetDcBranchEventRequest.PutBuffer("Eosb", stlEosb);
        oGetDcBranchEventRequest.PutString("ParentGuid", "{00000000-0000-0000-0000-000000000000}");
        oGetDcBranchEventRequest.PutString("OrganizationGuid", strOrganizationGuid);
        StructuredBuffer oFilters;
        oGetDcBranchEventRequest.PutStructuredBuffer("Filters", oFilters);
        // Call AuditLogManager plugin to get the guid of DC event log
        std::string strDcEventGuid;
        poIpcAuditLogManager = ::ConnectToUnixDomainSocket("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
        StructuredBuffer oAuditLogResponse(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oGetDcBranchEventRequest, false));
        poIpcAuditLogManager->Release();
        poIpcAuditLogManager = nullptr;
        if (200 == oAuditLogResponse.GetDword("Status"))
        {
            StructuredBuffer oListOfEvents = oAuditLogResponse.GetStructuredBuffer("ListOfEvents");
            if (0 < oListOfEvents.GetNamesOfElements().size())
            {
                std::string strRootEventGuid = oListOfEvents.GetStructuredBuffer(oListOfEvents.GetNamesOfElements()[0].c_str()).GetGuid("EventGuid").ToString(eHyphensAndCurlyBraces);
                // Create a DC branch event for DCGuid
                StructuredBuffer oDcBranchEvent;
                oDcBranchEvent.PutDword("TransactionType", 0x00000001);
                oDcBranchEvent.PutBuffer("Eosb", stlEosb);
                StructuredBuffer oDcMetadata;
                strDcEventGuid = Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces);
                oDcMetadata.PutString("EventGuid", strDcEventGuid);
                oDcMetadata.PutString("ParentGuid", strRootEventGuid);
                oDcMetadata.PutString("OrganizationGuid", strOrganizationGuid);
                oDcMetadata.PutQword("EventType", 2); // where 2 is for non root event type
                oDcMetadata.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
                StructuredBuffer oPlainTextMetadata;
                oPlainTextMetadata.PutDword("BranchType", 1); // where 1 is for for DC branch type
                oPlainTextMetadata.PutString("GuidOfDcOrVm", strDcGuid);
                oDcMetadata.PutStructuredBuffer("PlainTextEventData", oPlainTextMetadata);
                oDcBranchEvent.PutStructuredBuffer("NonLeafEvent", oDcMetadata);
                // Call AuditLogManager plugin to create a DC event log
                poIpcAuditLogManager = ::ConnectToUnixDomainSocket("/tmp/{F93879F1-7CFD-400B-BAC8-90162028FC8E}");
                StructuredBuffer oDcEventLog(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oDcBranchEvent, false));
                poIpcAuditLogManager->Release();
                poIpcAuditLogManager = nullptr;
                _ThrowBaseExceptionIf(((0 > oDcEventLog.GetSerializedBufferRawDataSizeInBytes())&&(201 != oDcEventLog.GetDword("Status"))), "Error creating DC branch event.", nullptr);
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
 * @class DigitalContractDatabase
 * @function ProvisionDigitalContract
 * @brief Create a Virutal Machine for the Digital Contract activation
 * @param[in] c_oRequest contains user Eosb for the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::ProvisionDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;
    std::string strErrorMessage = "";
    std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
    std::string strDcGuid = "";

    try
    {
        // Get digital contract guid
        strDcGuid = c_oRequest.GetString("DigitalContractGuid");

        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Get the digital contract blob
            StructuredBuffer oDcBlob(this->PullDigitalContract(c_oRequest));
            if (200 == oDcBlob.GetDword("Status"))
            {
                // Get user's organization guid
                std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
                if ((oDcBlob.GetString("ResearcherOrganization") == strOrganizationGuid) || (oDcBlob.GetString("DataOwnerOrganization") == strOrganizationGuid))
                {
                    StructuredBuffer oDigitialContract = oDcBlob.GetStructuredBuffer("DigitalContract");
                    if (((Dword)DigitalContractProvisiongStatus::eUnprovisioned == oDigitialContract.GetDword("ProvisioningStatus")) || ((Dword)DigitalContractProvisiongStatus::eProvisioningFailed == oDigitialContract.GetDword("ProvisioningStatus")))
                    {
                        if (eActive == oDigitialContract.GetDword("ContractStage"))
                        {
                            uint64_t unCurrentTime = ::GetEpochTimeInSeconds();
                            // Calculate expiration time and check if the contact is still active
                            uint64_t unExpirationTime = oDigitialContract.GetUnsignedInt64("ExpirationTime");
                            _ThrowBaseExceptionIf((unCurrentTime > unExpirationTime), "Digital Contract Expired", nullptr);

                            // TODO: Check if the dataset attached to the Digital Contract is registered
                            // and the Remote Data Connector sent a ping. Get the time of the
                            // latest ping and if the ping is older than the dataconnector_ping_duration
                            // the Remote DataConnector is deemed dead and this request will fail
                            std::string strRequiredDatasetGuid = oDigitialContract.GetString("DatasetGuid");

                            bool fIsRemoteDataConnectorActive = true;

                            std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
                            StructuredBuffer oRemoteDataconnectorRequest;
                            oRemoteDataconnectorRequest.PutDword("TransactionType", 0x00000001);
                            oRemoteDataconnectorRequest.PutBuffer("Eosb", stlEosb);
                            oRemoteDataconnectorRequest.PutString("OrganizationGuid", oDcBlob.GetString("DataOwnerOrganization"));
                            Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{9546C893-7F55-4FB7-BA63-B94B172105A0}");
                            StructuredBuffer oRemoteDataconnectorRequestResponse(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oRemoteDataconnectorRequest, false));
                            poIpcAzureManager->Release();
                            poIpcAzureManager = nullptr;

                            StructuredBuffer oRemoteDataConnectors = oRemoteDataconnectorRequestResponse.GetStructuredBuffer("Connectors");
                            for(auto strConnectorGuid : oRemoteDataConnectors.GetNamesOfElements())
                            {
                                StructuredBuffer oRemoteConnctor = oRemoteDataConnectors.GetStructuredBuffer(strConnectorGuid.c_str());
                                for (auto strDatasetsGuid : oRemoteConnctor.GetStructuredBuffer("Datasets").GetNamesOfElements())
                                {
                                    if (strRequiredDatasetGuid == strDatasetsGuid)
                                    {
                                        // TODO: Prawal add a time check for the remote dataconnector.. if the last heartbeat was older than 30seconds, discard the dataconnector
                                        fIsRemoteDataConnectorActive = true;
                                    }
                                }
                            }
                            // _ThrowBaseExceptionIf((false == fIsRemoteDataConnectorActive), "No remote data connector serving the requested dataset", nullptr);

                            // Get the Guid of the user who's azure template are to be used to create Virtual Machine
                            std::string strHostForVirtualMachine = oDigitialContract.GetString("HostForVirtualMachines");

                            // Get the Azure Template Uuid from the Digital Contract
                            std::string strAzureTemplateUuid = oDigitialContract.GetString("AzureTemplateGuid");
                            // Use the TemplateUuid to fetch the Azure Template
                            // Call AzureManager plugin to validate the Azure template
                            StructuredBuffer oAzureTemplate;
                            oAzureTemplate.PutDword("TransactionType", 0x00000001);
                            oAzureTemplate.PutBuffer("Eosb", stlEosb);
                            oAzureTemplate.PutString("TemplateGuid", strAzureTemplateUuid);
                            poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4B56D0E0-7A38-40C1-839A-B9BBCDDFE521}");
                            StructuredBuffer oAzureTemplateResponse(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oAzureTemplate, false));
                            poIpcAzureManager->Release();
                            poIpcAzureManager = nullptr;
                            if ((0 < oAzureTemplateResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oAzureTemplateResponse.GetDword("Status")))
                            {
                                StructuredBuffer oTemplateData = oAzureTemplateResponse.GetStructuredBuffer("Template");
                                std::string strName = oTemplateData.GetString("Name");
                                std::string strDescription = oTemplateData.GetString("Description");
                                std::string strSubscriptionID = oTemplateData.GetString("SubscriptionID");
                                std::string strSecret = oTemplateData.GetString("Secret");
                                std::string strTenantID = oTemplateData.GetString("TenantID");
                                std::string strApplicationID = oTemplateData.GetString("ApplicationID");
                                std::string strResourceGroup = oTemplateData.GetString("ResourceGroup");
                                std::string strVirtualMachineImageName = oTemplateData.GetString("VirtualMachineImage");
                                std::string strLocation = oTemplateData.GetString("HostRegion");
                                std::string strVirtualMachineImageId = ::CreateAzureResourceId(strSubscriptionID, strResourceGroup, "providers/Microsoft.Compute", "images", strVirtualMachineImageName);
                                std::string strVirtualNetwork = oTemplateData.GetString("VirtualNetwork");
                                std::string strVirtualNetworkId = ::CreateAzureResourceId(strSubscriptionID, strResourceGroup, "providers/Microsoft.Network", "virtualNetworks", strVirtualNetwork);
                                std::string strNetworkSecurityGroup = oTemplateData.GetString("NetworkSecurityGroup");
                                std::string strNetworkSecurityGroupId = ::CreateAzureResourceId(strSubscriptionID, strResourceGroup, "providers/Microsoft.Network", "networkSecurityGroups", strNetworkSecurityGroup);

                                auto unNumberOfVirtualMachines = oDigitialContract.GetUnsignedInt64("NumberOfVirtualMachines");

                                // If the DataConnector is active, start the VM provisioning in a different async thread
                                for (uint64_t unVmCounter = 0; unVmCounter < unNumberOfVirtualMachines; unVmCounter++)
                                {
                                    Guid oNewVmGuid;
                                    StructuredBuffer oVirtualMachineCreateParameter;
                                    oVirtualMachineCreateParameter.PutString("vmName", oNewVmGuid.ToString(eRaw));
                                    switch (oDigitialContract.GetUnsignedInt64("NumberOfVCPU"))
                                    {
                                        case 4
                                        :
                                            oVirtualMachineCreateParameter.PutString("vmSize", "Standard_B4ms");
                                            break;
                                        case 8
                                        :
                                            oVirtualMachineCreateParameter.PutString("vmSize", "Standard_B8ms");
                                            break;
                                        case 16
                                        :
                                            oVirtualMachineCreateParameter.PutString("vmSize", "Standard_B16ms");
                                            break;
                                        case 32
                                        :
                                            oVirtualMachineCreateParameter.PutString("vmSize", "Standard_D8_v4");
                                            break;
                                        case 48
                                        :
                                            oVirtualMachineCreateParameter.PutString("vmSize", "Standard_D48_v4");
                                            break;
                                        default
                                        :
                                            _ThrowBaseException("Number Of CPUs not supported", nullptr);
                                            break;
                                    }
                                    oVirtualMachineCreateParameter.PutString("vmImageId", strVirtualMachineImageId);
                                    oVirtualMachineCreateParameter.PutString("VirtualNetworkId", strVirtualNetworkId);
                                    oVirtualMachineCreateParameter.PutString("NetworkSecurityGroupId", strNetworkSecurityGroupId);
                                    oVirtualMachineCreateParameter.PutString("adminUsername", "saildeveloper");
                                    oVirtualMachineCreateParameter.PutString("adminPassword", "Iw2btin2AC+beRl&dir!");

                                    std::string strParamterJson = ::CreateAzureParamterJson("https://confidentialvmdeployment.blob.core.windows.net/deployemnttemplate/DeployVirtualMachineNoPorts.json?sp=r&st=2021-06-29T12:01:40Z&se=2022-02-28T20:01:40Z&spr=https&sv=2020-02-10&sr=b&sig=epPQ8kO62sj%2F0jA1aifQVq1VH1yN5woISBaqC2mRGfg%3D", oVirtualMachineCreateParameter);
                                    // Create a thread which will keep updating the VM status on the database as it proceeds
                                    // This API will return the response, stating that the VM creation has started
                                    std::thread oThreadCreateVirtualMachine(&DigitalContractDatabase::ProvisionVirtualMachine, this, oDigitialContract, oAzureTemplateResponse.GetBuffer("Eosb"), strApplicationID, strSecret, strTenantID, strSubscriptionID, strResourceGroup, oNewVmGuid.ToString(eRaw), strParamterJson, strLocation);
                                    oThreadCreateVirtualMachine.detach();
                                }

                                // Update the Digital ContractStatus to Provisioning
                                StructuredBuffer oUpdateProvisioningState;
                                oUpdateProvisioningState.PutBuffer("Eosb", oAzureTemplateResponse.GetBuffer("Eosb"));
                                oUpdateProvisioningState.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eProvisioning);
                                oUpdateProvisioningState.PutString("DigitalContractGuid", strDcGuid);
                                StructuredBuffer oUpdateProvisioningStateResponse(this->UpdateDigitalContractProvisioningStatus(oUpdateProvisioningState));
                                _ThrowBaseExceptionIf((200 != oUpdateProvisioningStateResponse.GetDword("Status")), "Update DC status fail", nullptr);

                                // Return a VM provisioning start success response here.
                                dwStatus = 200;
                            }
                        }
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
        strErrorMessage = oException.GetExceptionMessage();
        oResponse.PutString("Messsage: ", oException.GetExceptionMessage());
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

    try
    {
        if (200 != dwStatus)
        {
            // Update the Digital ContractStatus to Provisioning
            StructuredBuffer oUpdateProvisioningState;
            oUpdateProvisioningState.PutBuffer("Eosb", stlEosb);
            oUpdateProvisioningState.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eProvisioningFailed);
            oUpdateProvisioningState.PutString("DigitalContractGuid", strDcGuid);
            oUpdateProvisioningState.PutString("Note", "Error: "+strErrorMessage);
            StructuredBuffer oUpdateProvisioningStateResponse(this->UpdateDigitalContractProvisioningStatus(oUpdateProvisioningState));
            _ThrowBaseExceptionIf((200 != oUpdateProvisioningStateResponse.GetDword("Status")), "Update DC status fail", nullptr);
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ProvisionDigitalContract
 * @brief Create a Virutal Machine for the Digital Contract activation
 * @param[in] c_oRequest contains user Eosb for the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::ProvisionVirtualMachine(
    _in const StructuredBuffer c_oDigitalContract,
    _in const std::vector<Byte> c_stlEosb,
    _in const std::string c_szApplicationIdentifier,
    _in const std::string c_szSecret,
    _in const std::string c_szTenantIdentifier,
    _in const std::string c_szSubscriptionIdentifier,
    _in const std::string c_szResourceGroup,
    _in const std::string c_szVirtualMachineIdentifier,
    _in const std::string c_szVirtualMachineSpecification,
    _in const std::string c_szLocation
)
{
    __DebugFunction();

    bool fIsProvisioningSuccess = false;
    std::string strErrorMessage = "";
    try
    {
        // Register a Virtual Machine to the database.
        StructuredBuffer oVmRegisterRequest;
        oVmRegisterRequest.PutBuffer("Eosb", c_stlEosb);
        oVmRegisterRequest.PutDword("TransactionType", 0x00000003);
        oVmRegisterRequest.PutString("DigitalContractTitle", c_oDigitalContract.GetString("Title"));
        oVmRegisterRequest.PutString("DigitalContractGuid", c_oDigitalContract.GetString("DigitalContractGuid"));
        oVmRegisterRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
        oVmRegisterRequest.PutUnsignedInt64("HeartbeatBroadcastTime", ::GetEpochTimeInSeconds());
        oVmRegisterRequest.PutString("IPAddress", "0.0.0.0");
        oVmRegisterRequest.PutUnsignedInt64("NumberOfVCPU", c_oDigitalContract.GetUnsignedInt64("NumberOfVCPU"));
        oVmRegisterRequest.PutString("HostRegion", c_oDigitalContract.GetString("HostRegion"));
        oVmRegisterRequest.PutUnsignedInt64("StartTime", ::GetEpochTimeInSeconds());

        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
        StructuredBuffer oVmRegisterResponse(::PutIpcTransactionAndGetResponse(poSocket, oVmRegisterRequest, false));
        poSocket->Release();
        poSocket = nullptr;

        // Update the status of the Virutal Machine to Provisioning and add the IP Address there
        StructuredBuffer oUpdateVmStateRequest;
        oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
        oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
        oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
        oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eStarting);
        Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
        StructuredBuffer oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
        poIpcAzureManager->Release();
        poIpcAzureManager = nullptr;
        if ((0 < oUpdateVmStateResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oUpdateVmStateResponse.GetDword("Status")))
        {
            // Start the VM provisioning step. This step will be
            StructuredBuffer oDeployResponse = ::DeployVirtualMachineAndWait(c_szApplicationIdentifier, c_szSecret, c_szTenantIdentifier, c_szSubscriptionIdentifier, c_szResourceGroup, c_szVirtualMachineIdentifier, c_szVirtualMachineSpecification, c_szLocation);
            // TODO: Prawal add a check if the VM creation fails and mark the Digital contract as fail too possibly with an error message
            if("Success" != oDeployResponse.GetString("Status"))
            {
                // Delete the resources associated with the VM
                std::vector<std::string> stlListOfResourcesToDelete = ::AzureResourcesAssociatedWithVirtualMachine(c_szSubscriptionIdentifier, c_szResourceGroup, c_szVirtualMachineIdentifier);
                if (false == ::DeleteAzureResources(c_szApplicationIdentifier, c_szTenantIdentifier, c_szSecret, stlListOfResourcesToDelete))
                {
                    if (true == oDeployResponse.IsElementPresent("error", ANSI_CHARACTER_STRING_VALUE_TYPE))
                    {
                        _ThrowBaseException("Delete Azure resources failed. It may have to be done manually on the portal. %s", oDeployResponse.GetString("error").c_str());
                    }
                    else
                    {
                        _ThrowBaseException("Delete Azure resources failed. It may have to be done manually on the portal. Virtual Machine provisioning failed with unknown error.", nullptr);
                    }
                }

                if (true == oDeployResponse.IsElementPresent("error", ANSI_CHARACTER_STRING_VALUE_TYPE))
                {
                    _ThrowBaseException("%s", oDeployResponse.GetString("error").c_str());
                }
                else
                {
                    _ThrowBaseException("Virtual Machine provisioning failed with unknown error.", nullptr);
                }
            }
            std::string strIpAddress = oDeployResponse.GetString("IpAddress");

            // Update the IpAddress of the Virtual Machine
            StructuredBuffer oVmIpAddressRequest;
            oVmIpAddressRequest.PutBuffer("Eosb", c_stlEosb);
            oVmIpAddressRequest.PutDword("TransactionType", 0x00000004);
            oVmIpAddressRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
            oVmIpAddressRequest.PutString("IPAddress", strIpAddress);
            Socket * poIpcVirtualMachineManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
            StructuredBuffer oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcVirtualMachineManager, oVmIpAddressRequest, false));
            poIpcVirtualMachineManager->Release();
            poIpcVirtualMachineManager = nullptr;

            // Update the status of the Virutal Machine to Insstalling
            oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
            oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
            oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eConfiguring);
            Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
            oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
            poIpcAzureManager->Release();
            poIpcAzureManager = nullptr;

            // The installation package would be sent only to the Virutal Machines
            // that have been created with this process
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strIpAddress.c_str(), 9090, 10*60*1000, 30*1000);
            _ThrowIfNull(poTlsNode, "Failed to connect to the Virtual Machine", nullptr);

            StructuredBuffer oBinaryPackageInstructions;
            oBinaryPackageInstructions.PutString("Verb", "GET");
            oBinaryPackageInstructions.PutString("Content", "");
            oBinaryPackageInstructions.PutString("Uri", "/deployemnttemplate/SecureComputationalVirtualMachine.binaries?sp=r&st=2021-08-31T19:10:45Z&se=2025-03-02T03:10:45Z&sv=2020-08-04&sr=b&sig=Cax0Bm685X0UN8dVLrZYw55bj2gvRLNheiuRVzUtjBU%3D");
            oBinaryPackageInstructions.PutString("Host", "confidentialvmdeployment.blob.core.windows.net");

            // Send the url for the binary package and wait for a response on successful installation
            std::vector<Byte> stlSendPackageResponse = ::PutTlsTransactionAndGetResponse(poTlsNode, oBinaryPackageInstructions, 0);
            _ThrowBaseExceptionIf((0 >= stlSendPackageResponse.size()), "Invalid reponse to send package instructions.", nullptr);
            StructuredBuffer oSendPackageResponse(stlSendPackageResponse);
            _ThrowBaseExceptionIf(("Success" != oSendPackageResponse.GetString("Status")), "Sending package failed", nullptr);
            poTlsNode = nullptr;

            oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
            oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
            oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eInitializing);
            poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
            oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
            poIpcAzureManager->Release();
            poIpcAzureManager = nullptr;

            // Send the Virtual Machine Initialization data
            // Send the initialization data except for the dataset
            // First we need to build out the huge StructuredBuffer with all of the initialization parameters
            bool fSuccess = false;
            StructuredBuffer oInitializationParameters;
            oInitializationParameters.PutString("NameOfVirtualMachine", "Some nice name of Virtual machine");
            oInitializationParameters.PutString("IpAddressOfVirtualMachine", strIpAddress);
            oInitializationParameters.PutString("VirtualMachineIdentifier", c_szVirtualMachineIdentifier);
            oInitializationParameters.PutString("ClusterIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
            oInitializationParameters.PutString("DigitalContractIdentifier", c_oDigitalContract.GetString("DigitalContractGuid"));
            oInitializationParameters.PutString("RootOfTrustDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
            oInitializationParameters.PutString("ComputationalDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
            oInitializationParameters.PutString("DataConnectorDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
            oInitializationParameters.PutString("DatasetIdentifier", c_oDigitalContract.GetString("DatasetGuid"));
            auto stlVmEosb = oVmRegisterResponse.GetBuffer("VmEosb");
            oInitializationParameters.PutString("VmEosb", ::Base64Encode(stlVmEosb.data(), stlVmEosb.size()));

            // The installation package would be sent only to the Virutal Machines
            // that have been created with this process
            poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strIpAddress.c_str(), 6800, 10*60*1000, 30*1000);
            _ThrowIfNull(poTlsNode, "Failed to connect to the Virtual Machine to send initialization data", nullptr);
            // Send the url for the binary package and wait for a response on successful installation
            stlSendPackageResponse = ::PutTlsTransactionAndGetResponse(poTlsNode, oInitializationParameters, 0);
            _ThrowBaseExceptionIf((0 >= stlSendPackageResponse.size()), "Invalid reponse to send init parameters.", nullptr);
            oSendPackageResponse = StructuredBuffer(stlSendPackageResponse);
            _ThrowBaseExceptionIf(("Success" != oSendPackageResponse.GetString("Status")), "Sending package failed", nullptr);

            // Update status to waiting_for_data. This call wil also update the remote data connector
            // about the VM waiting for the Dataset
            oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
            oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
            oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eWaitingForData);
            poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
            oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
            poIpcAzureManager->Release();
            poIpcAzureManager = nullptr;

            fIsProvisioningSuccess = true;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        strErrorMessage = oException.GetExceptionMessage();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        strErrorMessage = "Internal Server Error. Contact SAIL.";
    }

    try
    {
        // Update the Digital ContractStatus to Provisioning
        StructuredBuffer oUpdateProvisioningState;
        oUpdateProvisioningState.PutBuffer("Eosb", c_stlEosb);
        if (true == fIsProvisioningSuccess)
        {
            oUpdateProvisioningState.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eReady);
        }
        else
        {
            oUpdateProvisioningState.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eProvisioningFailed);

            // Also mark the Virtual Machine Provisioning failed
            StructuredBuffer oUpdateVmStateRequest;
            oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
            oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
            oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_szVirtualMachineIdentifier);
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eProvisioningFailed);
            oUpdateVmStateRequest.PutString("Note", strErrorMessage);
            Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
            StructuredBuffer oUpdateVmStateResponse = StructuredBuffer(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
            poIpcAzureManager->Release();
            poIpcAzureManager = nullptr;
        }
        oUpdateProvisioningState.PutString("DigitalContractGuid", c_oDigitalContract.GetString("DigitalContractGuid"));
        StructuredBuffer oUpdateProvisioningStateResponse(this->UpdateDigitalContractProvisioningStatus(oUpdateProvisioningState));
        _ThrowBaseExceptionIf((200 != oUpdateProvisioningStateResponse.GetDword("Status")), "Update DC status fail", nullptr);
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ActivateDigitalContract
 * @brief Update the digital contract when a researcher accepts the DC terms from the Data owner organization
 * @param[in] c_oRequest contains user Eosb, comments made by the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::UpdateDigitalContractProvisioningStatus(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get digital contract guid
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Get user information to check if the user has admin access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            Qword qwAccessRights = oUserInfo.GetQword("AccessRights");
            if ((AccessRights::eDigitalContractAdmin == qwAccessRights) || (AccessRights::eAdmin == qwAccessRights))
            {
                // Step 1: Get the digital contract blob and update the structure
                StructuredBuffer oDcBlob(this->PullDigitalContract(c_oRequest));
                if (200 == oDcBlob.GetDword("Status"))
                {
                    StructuredBuffer oSsb(oDcBlob.GetStructuredBuffer("DigitalContract"));
                    if ((Dword)DigitalContractProvisiongStatus::eProvisioning == c_oRequest.GetDword("ProvisioningStatus"))
                    {
                        oSsb.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eProvisioning);
                    }
                    else if ((Dword)DigitalContractProvisiongStatus::eReady == c_oRequest.GetDword("ProvisioningStatus"))
                    {
                        if (true == oSsb.IsElementPresent("NumberOfVirtualMachinesReady", DWORD_VALUE_TYPE))
                        {
                            oSsb.PutDword("NumberOfVirtualMachinesReady", oSsb.GetDword("NumberOfVirtualMachinesReady") + 1);
                        }
                        else
                        {
                            oSsb.PutDword("NumberOfVirtualMachinesReady", 1);
                        }
                        // TODO: Prawal update this to provisioned when all the VMs are ready
                        oSsb.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eReady);
                    }
                    else
                    {
                        oSsb.PutDword("ProvisioningStatus", c_oRequest.GetDword("ProvisioningStatus"));
                    }

                    if (true == c_oRequest.IsElementPresent("Note", ANSI_CHARACTER_STRING_VALUE_TYPE))
                    {
                        oSsb.PutString("Note", c_oRequest.GetString("Note"));
                    }

                    // Serialize the update digital contract blob
                    std::vector<Byte> stlUpdatedSsb;
                    this->SerializeDigitalContract(oSsb, stlUpdatedSsb);
                    // Step 2: Call the database manager resource with the updated blob and update the database
                    // Make a Tls connection with the database portal
                    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
                    // Create a request to update digital contract in the database
                    StructuredBuffer oRequest;
                    oRequest.PutString("PluginName", "DatabaseManager");
                    oRequest.PutString("Verb", "PATCH");
                    oRequest.PutString("Resource", "/SAIL/DatabaseManager/Update/DigitalContract");
                    oRequest.PutString("DigitalContractGuid", strDcGuid);
                    oRequest.PutBuffer("DigitalContractBlob", stlUpdatedSsb);
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
 * @class DigitalContractDatabase
 * @function ProvisionDigitalContract
 * @brief Create a Virutal Machine for the Digital Contract activation
 * @param[in] c_oRequest contains user Eosb for the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::DeprovisionDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 400;
    TlsNode * poTlsNode = nullptr;

    try
    {
        // Get digital contract guid
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");

        // Get user information to check if the user has access rights
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Get the digital contract blob
            StructuredBuffer oDcBlob(this->PullDigitalContract(c_oRequest));
            if (200 == oDcBlob.GetDword("Status"))
            {
                // Get user's organization guid
                std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
                if ((oDcBlob.GetString("ResearcherOrganization") == strOrganizationGuid) || (oDcBlob.GetString("DataOwnerOrganization") == strOrganizationGuid))
                {
                    StructuredBuffer oDigitialContract = oDcBlob.GetStructuredBuffer("DigitalContract");
                    if ((Dword)DigitalContractProvisiongStatus::eReady == oDigitialContract.GetDword("ProvisioningStatus"))
                    {
                        // The location of Virtual Machines Provisioning is part of Digital Contract
                        std::string strLocation = oDigitialContract.GetString("HostRegion");

                        bool fIsRemoteDataConnectorActive = true;

                        // Get the Azure Template Uuid from the Digital Contract
                        std::string strAzureTemplateUuid = oDigitialContract.GetString("AzureTemplateGuid");
                        // Use the TemplateUuid to fetch the Azure Template
                        StructuredBuffer oAzureTemplate;
                        oAzureTemplate.PutDword("TransactionType", 0x00000001);
                        oAzureTemplate.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                        oAzureTemplate.PutString("TemplateGuid", strAzureTemplateUuid);
                        Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4B56D0E0-7A38-40C1-839A-B9BBCDDFE521}");
                        StructuredBuffer oAzureTemplateResponse(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oAzureTemplate, false));
                        poIpcAzureManager->Release();
                        poIpcAzureManager = nullptr;
                        if ((0 < oAzureTemplateResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oAzureTemplateResponse.GetDword("Status")))
                        {
                            StructuredBuffer oTemplateData = oAzureTemplateResponse.GetStructuredBuffer("Template");
                            // auto oListOfVirtualMachines = StructuredBuffer(this->GetProvisioningStatus(c_oRequest)).GetStructuredBuffer("VirtualMachines").GetNamesOfElements();
                            StructuredBuffer oListOfVirtualMachines;
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
                                oListOfVirtualMachines = oDatabaseResponse.GetStructuredBuffer("VirtualMachines");
                            }

                            // If the DataConnector is active, start the VM provisioning in a different async thread
                            for (auto strVirtualMachineName : oListOfVirtualMachines.GetNamesOfElements())
                            {
                                // Get the information about the Vitual Machine and deprovision
                                // it only if it was not already deleted or attempted to be deleted
                                StructuredBuffer oGetVmStateRequest;
                                oGetVmStateRequest.PutDword("TransactionType", 0x00000005);
                                oGetVmStateRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                                oGetVmStateRequest.PutString("VirtualMachineGuid", strVirtualMachineName);
                                Socket * poIpcVirtualMachineManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
                                StructuredBuffer oGetVmStateResponse(::PutIpcTransactionAndGetResponse(poIpcVirtualMachineManager, oGetVmStateRequest, false));
                                poIpcVirtualMachineManager->Release();
                                poIpcVirtualMachineManager = nullptr;

                                StructuredBuffer oVirtualMachine = oGetVmStateResponse.GetStructuredBuffer("VirtualMachine");
                                if (((Dword)VirtualMachineState::eDeleted != oVirtualMachine.GetDword("State")) && ((Dword)VirtualMachineState::eDeleteFailed != oVirtualMachine.GetDword("State")) && ((Dword)VirtualMachineState::eShuttingDown != oVirtualMachine.GetDword("State")))
                                {
                                    // Update the Virtual machine status to eShutingDown.
                                    StructuredBuffer oUpdateVmStateRequest;
                                    oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
                                    oUpdateVmStateRequest.PutBuffer("Eosb", c_oRequest.GetBuffer("Eosb"));
                                    oUpdateVmStateRequest.PutString("VirtualMachineGuid", strVirtualMachineName);
                                    oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eShuttingDown);
                                    poIpcVirtualMachineManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
                                    StructuredBuffer oUpdateVmStateResponse(::PutIpcTransactionAndGetResponse(poIpcVirtualMachineManager, oUpdateVmStateRequest, false));
                                    poIpcVirtualMachineManager->Release();
                                    poIpcVirtualMachineManager = nullptr;

                                    std::thread(&DigitalContractDatabase::DeleteVirtualMachineResources, this, oAzureTemplateResponse.GetBuffer("Eosb"), oTemplateData, strVirtualMachineName).detach();
                                }
                            }

                            // Update the Digital ContractStatus to UnProvisioned
                            StructuredBuffer oUpdateProvisioningState;
                            oUpdateProvisioningState.PutBuffer("Eosb", oAzureTemplateResponse.GetBuffer("Eosb"));
                            oUpdateProvisioningState.PutDword("ProvisioningStatus", (Dword)DigitalContractProvisiongStatus::eUnprovisioned);
                            oUpdateProvisioningState.PutString("DigitalContractGuid", strDcGuid);
                            StructuredBuffer oUpdateProvisioningStateResponse(this->UpdateDigitalContractProvisioningStatus(oUpdateProvisioningState));
                            _ThrowBaseExceptionIf((200 != oUpdateProvisioningStateResponse.GetDword("Status")), "Update DC status fail", nullptr);

                            // Return a VM provisioning start success response here.
                            dwStatus = 200;
                        }
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
        oResponse.PutString("Messsage: ", oException.GetExceptionMessage());
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
 * @class DigitalContractDatabase
 * @function DeleteVirtualMachineResources
 * @brief Delete all the resources belonging to a virtual machine
 * @param[in] c_stlEosb contains user Eosb for the researcher organization
 * @param[in] c_oTemplateData The azure template to use for azure credential
 * @param[in] c_strVirtualMachineName virutal machine which needs to be deleted
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

void __thiscall DigitalContractDatabase::DeleteVirtualMachineResources(
    _in const std::vector<Byte> c_stlEosb,
    _in const StructuredBuffer c_oTemplateData,
    _in const std::string c_strVirtualMachineName
    ) const throw()
{
    __DebugFunction();

    try
    {
        std::string strSubscriptionID = c_oTemplateData.GetString("SubscriptionID");
        std::string strSecret = c_oTemplateData.GetString("Secret");
        std::string strTenantID = c_oTemplateData.GetString("TenantID");
        std::string strApplicationID = c_oTemplateData.GetString("ApplicationID");
        std::string strResourceGroup = c_oTemplateData.GetString("ResourceGroup");

        std::vector<std::string> stlListOfResourcesToDelete = ::AzureResourcesAssociatedWithVirtualMachine(strSubscriptionID, strResourceGroup, c_strVirtualMachineName);

        // Update the Virtual machine status to eShutingDown.
        StructuredBuffer oUpdateVmStateRequest;
        oUpdateVmStateRequest.PutDword("TransactionType", 0x00000002);
        oUpdateVmStateRequest.PutBuffer("Eosb", c_stlEosb);
        oUpdateVmStateRequest.PutString("VirtualMachineGuid", c_strVirtualMachineName);
        if (true == ::DeleteAzureResources(strApplicationID, strTenantID, strSecret, stlListOfResourcesToDelete))
        {
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eDeleted);
        }
        else
        {
            oUpdateVmStateRequest.PutDword("State", VirtualMachineState::eDeleteFailed);
        }
        Socket * poIpcAzureManager = ::ConnectToUnixDomainSocket("/tmp/{4FBC17DA-81AF-449B-B842-E030E337720E}");
        StructuredBuffer oUpdateVmStateResponse(::PutIpcTransactionAndGetResponse(poIpcAzureManager, oUpdateVmStateRequest, false));
        poIpcAzureManager->Release();
        poIpcAzureManager = nullptr;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class DigitalContractDatabase
 * @function ProvisionDigitalContract
 * @brief Create a Virutal Machine for the Digital Contract activation
 * @param[in] c_oRequest contains user Eosb for the researcher organization
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DigitalContractDatabase::InitializeVirtualMachine(
    _in const StructuredBuffer & c_oRequest
)
{
    __DebugFunction();

    StructuredBuffer oResponse;
    Dword dwStatus = 400;

    std::cout << "It is here" << std::endl;
    std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
    std::string strDatasetGuid = c_oRequest.GetString("DatasetGuid");
    std::string strVirtualMachineIpAddress = c_oRequest.GetString("VirtualMachineIp");

    bool fIsProvisioningSuccess = false;
    std::string strErrorMessage = "";
    try
    {
        // Send the Virtual Machine Initialization data
        // Send the initialization data except for the dataset
        // First we need to build out the huge StructuredBuffer with all of the initialization parameters
        bool fSuccess = false;
        StructuredBuffer oInitializationParameters;
        oInitializationParameters.PutString("NameOfVirtualMachine", "Some nice name of Virtual machine");
        oInitializationParameters.PutString("IpAddressOfVirtualMachine", strVirtualMachineIpAddress);
        oInitializationParameters.PutString("VirtualMachineIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("ClusterIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("DigitalContractIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("RootOfTrustDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("ComputationalDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("DataConnectorDomainIdentifier", Guid().ToString(eHyphensAndCurlyBraces));
        oInitializationParameters.PutString("DatasetIdentifier", strDatasetGuid);
        oInitializationParameters.PutString("VmEosb", ::Base64Encode(stlEosb.data(), stlEosb.size()));

        // The installation package would be sent only to the Virutal Machines
        // that have been created with this process
        auto poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strVirtualMachineIpAddress.c_str(), 6800, 10*60*1000, 30*1000);
        _ThrowIfNull(poTlsNode, "Failed to connect to the Virtual Machine to send initialization data", nullptr);
        // Send the url for the binary package and wait for a response on successful installation
        auto stlSendPackageResponse = ::PutTlsTransactionAndGetResponse(poTlsNode, oInitializationParameters, 0);
        _ThrowBaseExceptionIf((0 >= stlSendPackageResponse.size()), "Invalid reponse to send init parameters.", nullptr);
        auto oSendPackageResponse = StructuredBuffer(stlSendPackageResponse);
        _ThrowBaseExceptionIf(("Success" != oSendPackageResponse.GetString("Status")), "Sending package failed", nullptr);

        dwStatus = 200;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    // Send back status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
