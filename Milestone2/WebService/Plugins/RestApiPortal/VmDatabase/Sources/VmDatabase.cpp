/*********************************************************************************************
 *
 * @file VmDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "VmDatabase.h"

static VmDatabase * gs_oVmDatabase = nullptr;

/********************************************************************************************
 *
 * @function GetVmDatabase
 * @brief Create a singleton object of VmDatabase class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of VmDatabase class
 *
 ********************************************************************************************/

VmDatabase * __stdcall GetVmDatabase(void)
{
    __DebugFunction();

    if (nullptr == gs_oVmDatabase)
    {
        gs_oVmDatabase = new VmDatabase();
        _ThrowOutOfMemoryExceptionIfNull(gs_oVmDatabase);
    }

    return gs_oVmDatabase;
}

/********************************************************************************************
 *
 * @function ShutdownVmDatabase
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownVmDatabase(void)
{
    __DebugFunction();

    if (nullptr != gs_oVmDatabase)
    {
        gs_oVmDatabase->Release();
        gs_oVmDatabase = nullptr;
    }
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function VmDatabase
 * @brief Constructor
 *
 ********************************************************************************************/

VmDatabase::VmDatabase(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

    this->InitializeUserAccounts();
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function VmDatabase
 * @brief Copy Constructor
 * @param[in] c_oVmDatabase Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

VmDatabase::VmDatabase(
    _in const VmDatabase & c_oVmDatabase
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function ~VmDatabase
 * @brief Destructor
 *
 ********************************************************************************************/

VmDatabase::~VmDatabase(void)
{
    __DebugFunction();

    for (VmInstance * oVmInstance : m_stlVmInstances)
    {
        delete oVmInstance;
    }

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }

}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall VmDatabase::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "VmDatabase";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall VmDatabase::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{FC48839C-AFA9-4D95-B83E-538A0B31CFD2}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall VmDatabase::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VmDatabase::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function InitializeUserAccounts
 * @brief Insert user data
 *
 ********************************************************************************************/

void __thiscall VmDatabase::InitializeUserAccounts(void)
{
    __DebugFunction();

    m_stlUserAccounts.push_back(new UserAccount("{FEB1CAE7-0F10-4185-A1F2-DE71B85DBD25}", "johnsnow", "John Snow", "jsnow@example.com", "HBO", "1234567890", 999, 0x7));
    m_stlUserAccounts.push_back(new UserAccount("{C1F45EF0-AB47-4799-9407-CA8A40CAC159}", "aryastark", "Arya Stark", "astark@example.com", "HBO", "1234567890", 888, 0x2));
    m_stlUserAccounts.push_back(new UserAccount("{0A83BCF5-2845-4437-AEBE-E02DFB349BAB}", "belle", "Belle", "belle@example.com", "Walt Disney", "1234567890", 777, 0x1));
    m_stlUserAccounts.push_back(new UserAccount("{64E4FAC3-63C9-4844-BF82-1581F9C750CE}", "gaston", "Gaston", "gaston@example.com", "Walt Disney", "1234567890", 666, 0x6));
    m_stlUserAccounts.push_back(new UserAccount("{F732CA9C-217E-4E3D-BF25-E2425B480556}", "hermoinegranger", "Hermoine Granger", "hgranger@example.com", "Universal Studios", "1234567890", 555, 0x5));
    m_stlUserAccounts.push_back(new UserAccount("{F3FBE722-1A42-4052-8815-0ABDDB3F2841}", "harrypotter", "Harry Potter", "hpotter@example.com", "Universal Studios", "1234567890", 444, 0x4));
    m_stlUserAccounts.push_back(new UserAccount("{2B9C3814-79D4-456B-B64A-ED79F69373D3}", "antman", "Ant man", "antman@example.com", "Marvel Cinematic Universe", "1234567890", 333, 0x7));
    m_stlUserAccounts.push_back(new UserAccount("{B40E1F9C-C100-46B3-BD7F-C80EB1351794}", "spiderman", "Spider man", "spiderman@example.com", "Marvel Cinematic Universe", "1234567890", 222, 0x6));
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall VmDatabase::InitializePlugin(void)
{
    __DebugFunction();

    // Takes in an EOSB and sends back list of all running VMs information associated with the smart contract
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VM/GetRunningVMs");

    // Sends back report of VM's status
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/VM/GetVMHeartBeat");

    // Adds a VM record in the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/VirtualMachineManager");

}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall VmDatabase::SubmitRequest(
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
        if ("/SAIL/VM/GetRunningVMs" == strResource)
        {
            stlResponseBuffer = this->GetListOfRunningVmInstances(c_oRequestStructuredBuffer);
        }

        else if ("/SAIL/VM/GetVMHeartBeat" == strResource)
        {
            stlResponseBuffer = this->GetVmHeartBeat(c_oRequestStructuredBuffer);
        }
    }
    else if ("POST" == strVerb)
    {
        if ("/SAIL/VirtualMachineManager" == strResource)
        {
            stlResponseBuffer = this->RegisterVmInstance(c_oRequestStructuredBuffer);
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
 * @class VmDatabase
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall VmDatabase::GetResponse(
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
 * @class VmDatabase
 * @function GetListOfRunningVmInstances
 * @brief Takes in IEOSB of Researcher or Dataset Owner and send back list of all running VMs information associated with the smart contract
 * @param[in] c_oRequest contains the IEOSB
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of running VMs information associated with the smart contract
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VmDatabase::GetListOfRunningVmInstances(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch VMInstances records from the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full IEOSB of Researcher or DatasetAdmin
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((false == fIsImposter), "Imposter EOSB needed to get the list of running VMs.", nullptr);

    // Verify that the user's TypeOfAccount is "Researcher or ""Dataset Admin"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if ((true == m_stlUserAccounts[unIndex]->IsResearcher()) || (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin()))
            {
                fFound = true;
            }
            else
            {
                _ThrowBaseException("Error: User is not authorized for this transaction", nullptr);
            }
        }
    }

    _ThrowBaseExceptionIf((false == fFound), "Error: User not found", nullptr);

    // Generate a StructuredBuffer containing all running VMs information associated with the smart contract
    VmInstance * oVmInstance;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; unIndex < m_stlVmInstances.size(); ++unIndex)
    {
        // TODO: Add an if statement and add check for VMs associated with the smart contract
        oVmInstance = m_stlVmInstances[unIndex];
        StructuredBuffer oVmInstanceMetadata;
        oVmInstanceMetadata.PutString("VmIpAddress", oVmInstance->GetVmInstanceIpAddress());
        oVmInstanceMetadata.PutWord("VmPortNumber", oVmInstance->GetVmInstancePortNumber());
        oVmInstanceMetadata.PutBoolean("VmStatus", oVmInstance->GetVmInstanceStatus());
        oVmInstanceMetadata.PutQword("VmRegistrationDate", oVmInstance->GetVmInstanceRegistrationDate());
        oVmInstanceMetadata.PutQword("VmUpTime", oVmInstance->GetVmInstanceUpTime());
        oResponse.PutStructuredBuffer(oVmInstance->GetVmInstanceUuid().c_str(), oVmInstanceMetadata);

    }
    ::pthread_mutex_unlock(&m_sMutex);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function GetVmHeartBeat
 * @brief Send back status of VM
 * @param[in] c_oRequest contains the IEOSB of the Dataset Admin and uuid of the VMInstance
 * @returns StructuredBuffer containing status of the VM
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VmDatabase::GetVmHeartBeat(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch VmInstances records from the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in full IEOSB of Researcher or DatasetAdmin
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((false == fIsImposter), "Imposter EOSB needed to get status of a VM.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
        if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
        {
            if (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())
            {
                fFound = true;
            }
            else
            {
                _ThrowBaseException("Error: User is not authorized for this transaction", nullptr);
            }
        }
    }

    _ThrowBaseExceptionIf((false == fFound), "Error: User not found", nullptr);

    // Get the VM UUID
    std::string strVmUuid = c_oRequest.GetString("VmInstanceUuid");
    // Find the VM and send back status of the VM
    fFound = false;
    bool fVmStatus = false;
    ::pthread_mutex_lock(&m_sMutex);
    for (unsigned int unIndex = 0 ; ((false == fFound) && (unIndex < m_stlVmInstances.size())); ++unIndex)
    {
        if (strVmUuid == m_stlVmInstances[unIndex]->GetVmInstanceUuid())
        {
            fVmStatus = m_stlVmInstances[unIndex]->GetVmInstanceStatus();
            fFound = true;
        }
    }
    ::pthread_mutex_unlock(&m_sMutex);

    _ThrowBaseExceptionIf((false == fFound), "ERROR: VmInstance not found.", nullptr);

    oResponse.PutBoolean("VmStatus", fVmStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class VmDatabase
 * @function RegisterVmInstance
 * @brief Take in IEOSB of a dataset admin and add the vm record to the database
 * @param[in] c_oRequest contains IEOSB of the dataset admin and the VM information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall VmDatabase::RegisterVmInstance(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();
    // TODO: Fetch vm instance records from the database
    // TODO: Replace call to abstract class UserAccount::IsDatasetAdmin() with call to AccountDatabase plugin
    //       and get TypeOfUser associated with strUserUuid

    StructuredBuffer oResponse;

    // Take in IEOSB of dataset admin
    StructuredBuffer oEosb(c_oRequest.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((false == fIsImposter), "Imposter EOSB needed to register a VM.", nullptr);

    // Verify that the user's TypeOfAccount is "Dataset Admin"
    bool fFound = false;
    for (unsigned int unIndex = 0; ((false == fFound) && (unIndex < m_stlUserAccounts.size())); ++unIndex)
    {
       if (strUserUuid == m_stlUserAccounts[unIndex]->GetUserUuid())
       {
           if (true == m_stlUserAccounts[unIndex]->IsDatasetAdmin())
           {
               fFound = true;
           }
           else
           {
               _ThrowBaseException("Error: User is not authorized for this transaction", nullptr);
           }
       }
    }
    _ThrowBaseExceptionIf((false == fFound), "Error: User not found", nullptr);

    // Get the new VmInstance information
    std::string strVmInstanceIpAddress = c_oRequest.GetString("VmIpAddress");
    Word wVmInstancePortNumber = c_oRequest.GetWord("VmPortNumber");
    bool fVmStatus = c_oRequest.GetBoolean("VmStatus");
    Qword qwVmInstanceRegistrationDate = c_oRequest.GetQword("VmRegistrationDate");
    Qword qwVmInstanceUpTime = c_oRequest.GetQword("VmUpTime");

    // Generate a UUID for the new VmInstance
    Guid oVmGuid;
    std::string strVmUuid = oVmGuid.ToString(eHyphensAndCurlyBraces);

    // Create a new VmInstance record
    VmInstance * oVmInstance = new VmInstance(strVmUuid, strVmInstanceIpAddress, wVmInstancePortNumber, fVmStatus, qwVmInstanceRegistrationDate, qwVmInstanceUpTime);

    // Add VmInstance to m_stlVmInstances
    ::pthread_mutex_lock(&m_sMutex);
    m_stlVmInstances.push_back(oVmInstance);
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back transaction status and vm uuid
    oResponse.PutWord("Status", 200);
    oResponse.PutString("VmInstanceUuid", strVmUuid);

    return oResponse.GetSerializedBuffer();
}
