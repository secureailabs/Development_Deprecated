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

    // Add Parameters for registering audit event for DOO/RO
    StructuredBuffer oRegisterEvent;
    oRegisterEvent.PutStructuredBuffer("Eosb", oEosb);
    oRegisterEvent.PutStructuredBuffer("VirtualMachineGuid", oVmGuid);

    // // Takes in an EOSB and sends back list of all running VMs information associated with the smart contract
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VirtualMachineManager/GetRunningVMs", 0);

    // Sends back report of VM's status
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VirtualMachineManager/GetVMHeartBeat", 0);

    // Register a Virtual Machine
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/RegisterVM", oRegisterVmParameters, 2);

    // Register a Virtual Machine event for DOO 
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/DataOwner/RegisterVM", oRegisterEvent, 4);

    // Register a Virtual Machine event for RO 
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager/Researcher/RegisterVM", oRegisterEvent, 4);

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
        if ("/SAIL/VirtualMachineManager/GetRunningVMs" == strResource)
        {
            stlResponseBuffer = this->GetListOfRunningVmInstances(c_oRequestStructuredBuffer);
        }

        else if ("/SAIL/VirtualMachineManager/GetVMHeartBeat" == strResource)
        {
            stlResponseBuffer = this->GetVmHeartBeat(c_oRequestStructuredBuffer);
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
 * @function GetListOfRunningVmInstances
 * @brief Takes in IEOSB of Researcher or Dataset Owner and send back list of all running VMs information associated with the smart contract
 * @param[in] c_oRequest contains the IEOSB
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of running VMs information associated with the smart contract
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetListOfRunningVmInstances(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch VMInstances records from the database
    // TODO: Check if user is a DatasetAdmin

    StructuredBuffer oResponse;

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VirtualMachineManager
 * @function GetVmHeartBeat
 * @brief Send back status of VM
 * @param[in] c_oRequest contains the IEOSB of the Dataset Admin and uuid of the VMInstance
 * @returns StructuredBuffer containing status of the VM
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::GetVmHeartBeat(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch VmInstances records from the database

    StructuredBuffer oResponse;

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
        StructuredBuffer oDecryptedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest));
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
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(c_oRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
            // Make a Tls connection with the database portal
            poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
            // Create a request to get the digital contract information
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
                oResponse.PutStructuredBuffer("VirtualMachine", oDatabaseResponse.GetStructuredBuffer("VirtualMachine"));
                dwStatus = 200;
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
 * @class VirtualMachineManager
 * @function VerifyDigitalContract
 * @brief Check if IEosb is of the data owner of the digital contract associated with the strDcGuid
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the comparison status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VirtualMachineManager::VerifyDigitalContract(
    _in const StructuredBuffer & c_oRequest,
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
        StructuredBuffer oDigitalContractResponse(::PutIpcTransactionAndGetResponse(poIpcDigitalContractManager, oDigitalContractRequest));
        poIpcDigitalContractManager->Release();
        poIpcDigitalContractManager = nullptr;
        if ((0 < oDigitalContractResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oDigitalContractResponse.GetDword("Status")))
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
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("IEosb");
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");

        // Get user information 
        StructuredBuffer oEosbRequest;
        oEosbRequest.PutBuffer("Eosb", stlEosb);
        // Get user information to check if the user is a digital contract admin or database admin
        StructuredBuffer oUserInfo(this->GetUserInfo(oEosbRequest));
        if (200 == oUserInfo.GetDword("Status"))
        {
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
            oRequest.PutString("DigitalContractGuid", strDcGuid);
            oRequest.PutUnsignedInt64("RegistrationTime", ::GetEpochTimeInSeconds());
            oRequest.PutUnsignedInt64("HeartbeatBroadcastTime", c_oRequest.GetUnsignedInt64("HeartbeatBroadcastTime"));
            oRequest.PutString("IPAddress", c_oRequest.GetString("IPAddress"));
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
                oUpdateEosbRequest.PutQword("AccessRights", eVmEosb); 
                poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
                StructuredBuffer oUpdatedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oUpdateEosbRequest));
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
        if (eEosb == oUserInfo.GetQword("AccessRights"))
        {
            // Get the organization guid
            std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
            // Check if Eosb is of the data owner of the digital contract associated with the strDcGuid
            StructuredBuffer oVerifyDcRequest;
            oVerifyDcRequest.PutString("DigitalContractGuid", strDcGuid);
            oVerifyDcRequest.PutString("OrganizationGuid", strOrganizationGuid);
            oVerifyDcRequest.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVerificationStatus(this->VerifyDigitalContract(oVerifyDcRequest, false));
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
        if (eEosb == oUserInfo.GetQword("AccessRights"))
        {
            // Get the organization guid
            std::string strOrganizationGuid = oUserInfo.GetGuid("OrganizationGuid").ToString(eHyphensAndCurlyBraces);
            // Check if Eosb is a user within the RO of the digital contract associated with the strDcGuid
            StructuredBuffer oVerifyDcRequest;
            oVerifyDcRequest.PutString("DigitalContractGuid", strDcGuid);
            oVerifyDcRequest.PutString("OrganizationGuid", strOrganizationGuid);
            oVerifyDcRequest.PutBuffer("Eosb", stlEosb);
            StructuredBuffer oVerificationStatus(this->VerifyDigitalContract(oVerifyDcRequest, true));
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
        ::RegisterException(oException, __func__, __LINE__);
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
        StructuredBuffer oDCEventLog(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oGetDcBranchEventRequest));
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
            StructuredBuffer oVmEventLog(::PutIpcTransactionAndGetResponse(poIpcAuditLogManager, oVmBranchEvent));
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
        ::RegisterException(oException, __func__, __LINE__);
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
