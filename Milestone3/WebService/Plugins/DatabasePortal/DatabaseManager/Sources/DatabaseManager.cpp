/*********************************************************************************************
 *
 * @file DatabaseManager.cpp
 * @author Shabana Akhtar Baig
 * @date 22 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

static DatabaseManager * gs_oDatabaseManager = nullptr;

/********************************************************************************************
 *
 * @function GetDatabaseManager
 * @brief Create a singleton object of DatabaseManager class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of DatabaseManager class
 *
 ********************************************************************************************/

DatabaseManager * __stdcall GetDatabaseManager(void)
{
    __DebugFunction();

    if (nullptr == gs_oDatabaseManager)
    {
        gs_oDatabaseManager = new DatabaseManager();
        _ThrowOutOfMemoryExceptionIfNull(gs_oDatabaseManager);
    }

    return gs_oDatabaseManager;
}

/********************************************************************************************
 *
 * @function ShutdownDatabaseManager
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownDatabaseManager(void)
{
    __DebugFunction();

    if (nullptr != gs_oDatabaseManager)
    {
        gs_oDatabaseManager->TerminateSignalEncountered();
        gs_oDatabaseManager->Release();
        gs_oDatabaseManager = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DatabaseManager
 * @brief Constructor
 *
 ********************************************************************************************/

DatabaseManager::DatabaseManager(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
    m_fTerminationSignalEncountered = false;
    // Create only one instance
    mongocxx::instance oMongoInstance{};
    mongocxx::uri oUri{"mongodb://localhost:27017/?replicaSet=rs0"};
    m_poMongoPool = std::unique_ptr<mongocxx::pool>(new mongocxx::pool(oUri));
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DatabaseManager
 * @brief Copy Constructor
 * @param[in] c_oDatabaseManager Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

DatabaseManager::DatabaseManager(
    _in const DatabaseManager & c_oDatabaseManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ~DatabaseManager
 * @brief Destructor
 *
 ********************************************************************************************/

DatabaseManager::~DatabaseManager(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatabaseManager::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "DatabaseManager";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall DatabaseManager::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{E14809BF-179A-498F-8C4B-57D9415A91F3}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall DatabaseManager::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall DatabaseManager::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function InitializePlugin
 * @brief Connects to the database and initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall DatabaseManager::InitializePlugin(void)
{
    __DebugFunction();

    // Get basic user record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/BasicUser");
    // Get Confidential User Record
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ConfidentialUser");
    // Given an organization guid fetch digital contract event guid, if exists
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/GetDCEvent");
    // Fetch audit log records from the database
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/Events");
    // Fetch a list of all organizations
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/Organizations");
    // Fetch a list of all users
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/Users");
    // Fetch a list of all users for an organizations
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/OrganizationUsers");
    // Get dataset name associated with the dataset guid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/GetDatasetName");
    // Fetch list of all datasets
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListDatasets");
    // Get a dataset's information
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/PullDataset");
    // Get a list of digital contracts associated with a user's organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListDigitalContracts");
    // Get a digital contract's information
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/PullDigitalContract");
    // Get a virtual machine's information
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/PullVirtualMachine");
    // Get list of VMs associated with a digital contract
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListOfVMsAssociatedWithDC");
    // Get list of VM ip addresses associated with a digital contract
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListOfVMIpAddressesAssociatedWithDC");
    // Get an azure settings template
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/PullAzureTemplate");
    // Get a list of azure settings templates for an organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListAzureTemplates");
    // Get a remote data connector
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/PullRemoteDataConnector");
    // Get a list of remote data connectors for an organization
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/ListRemoteDataConnectors");
    // Fetch list of VM ipaddressese that are waiting for the remote data connector's dataset(s)
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/GetVmsWaitingForData");
    // Add a non-leaf audit log event
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/NonLeafEvent");
    // Add a leaf audit log event
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/LeafEvent");
    // Register an organization and the new user as it's super user
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterOrganization");
    // Add a new user
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterUser");
    // Add metadata of a dataset to the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterDataset");
    // Add metadata of a new virtual machine to the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterVirtualMachine");
    // Takes in an EOSB and create a digital contract for a chosen dataset
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterDigitalContract");
    // Takes in an EOSB and registers an azure settings template
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterAzureTemplate");
    // Takes in an EOSB and registers a remote data connector
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterRemoteDataConnector");
    // Add metadata of a virtual machine that is waiting for data to the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterVmAsWaitingForData");
    // Shuts down the server
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/ShutdownPortal");
    // Update user's access rights
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/UpdateUserRights");
    // Update organization information
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/UpdateOrganizationInformation");
    // Update user information, excluding access rights
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/UpdateUserInformation");
    // Recover deleted user account
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/RecoverUser");
    // Update an azure settings template
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/UpdateAzureTemplate");
    // Update a remote data connector
    m_oDictionary.AddDictionaryEntry("PUT", "/SAIL/DatabaseManager/UpdateRemoteDataConnector");
    // Update the digital contract when a data owner accepts the digital contract or when a researcher accepts the DC terms from the Data owner organization
    m_oDictionary.AddDictionaryEntry("PATCH", "/SAIL/DatabaseManager/Update/DigitalContract");
    // Delete a user from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/DeleteUser");
    // Delete an organization and its users from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/DeleteOrganization");
    // Delete a dataset record from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/DeleteDataset");
    // Delete an Azure template record from the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/DeleteAzureTemplate");
    // Remove VM information as waiting for data
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/RemoveVmAsWaitingForData");
    // Add metadata of a dataset family to the database
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/DatabaseManager/RegisterDatasetFamily");
    // Add metadata of a dataset family to the database
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/DatabaseManager/DatasetFamilies");
    // Reset the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/DatabaseManager/ResetDatabase");
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall DatabaseManager::SubmitRequest(
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
    try
    {
        if ("GET" == strVerb)
        {
            if ("/SAIL/DatabaseManager/BasicUser" == strResource)
            {
                stlResponseBuffer = this->GetBasicUserRecord(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ConfidentialUser" == strResource)
            {
                stlResponseBuffer = this->GetConfidentialUserRecord(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/GetDCEvent" == strResource)
            {
                stlResponseBuffer = this->DigitalContractBranchExists(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/Events" == strResource)
            {
                stlResponseBuffer = this->GetListOfEvents(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/Organizations" == strResource)
            {
                stlResponseBuffer = this->ListOrganizations(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/Users" == strResource)
            {
                stlResponseBuffer = this->ListUsers(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/OrganizationUsers" == strResource)
            {
                stlResponseBuffer = this->ListOrganizationUsers(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/GetDatasetName" == strResource)
            {
                stlResponseBuffer = this->GetDatasetName(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListDatasets" == strResource)
            {
                stlResponseBuffer = this->ListDatasets(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullDataset" == strResource)
            {
                stlResponseBuffer = this->PullDataset(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListDigitalContracts" == strResource)
            {
                stlResponseBuffer = this->ListDigitalContracts(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullDigitalContract" == strResource)
            {
                stlResponseBuffer = this->PullDigitalContract(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullVirtualMachine" == strResource)
            {
                stlResponseBuffer = this->PullVirtualMachine(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListOfVMsAssociatedWithDC" == strResource)
            {
                stlResponseBuffer = this->ListOfVmsAssociatedWithDc(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListOfVMIpAddressesAssociatedWithDC" == strResource)
            {
                stlResponseBuffer = this->ListOfVmIpAddressesAssociatedWithDc(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/Organization/Information" == strResource)
            {
                stlResponseBuffer = this->GetOrganizationInformation(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullAzureTemplate" == strResource)
            {
                stlResponseBuffer = this->PullAzureTemplate(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListAzureTemplates" == strResource)
            {
                stlResponseBuffer = this->ListAzureTemplates(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullRemoteDataConnector" == strResource)
            {
                stlResponseBuffer = this->PullRemoteDataConnector(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ListRemoteDataConnectors" == strResource)
            {
                stlResponseBuffer = this->ListRemoteDataConnectors(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/GetVmsWaitingForData" == strResource)
            {
                stlResponseBuffer = this->GetVmsWaitingForData(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/DatasetFamilies" == strResource)
            {
                stlResponseBuffer = this->ListDatasetFamilies(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/PullDatatsetFamily" == strResource)
            {
                stlResponseBuffer = this->PullDatasetFamily(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else if ("POST" == strVerb)
        {
            if ("/SAIL/DatabaseManager/NonLeafEvent" == strResource)
            {
                stlResponseBuffer = this->AddNonLeafEvent(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/LeafEvent" == strResource)
            {
                stlResponseBuffer = this->AddLeafEvent(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterOrganization" == strResource)
            {
                stlResponseBuffer = this->RegisterOrganization(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterUser" == strResource)
            {
                stlResponseBuffer = this->RegisterUser(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterDataset" == strResource)
            {
                stlResponseBuffer = this->RegisterDataset(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterVirtualMachine" == strResource)
            {
                stlResponseBuffer = this->RegisterVirtualMachine(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterDigitalContract" == strResource)
            {
                stlResponseBuffer = this->RegisterDigitalContract(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterAzureTemplate" == strResource)
            {
                stlResponseBuffer = this->RegisterAzureTemplate(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterRemoteDataConnector" == strResource)
            {
                stlResponseBuffer = this->RegisterRemoteDataConnector(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RegisterVmAsWaitingForData" == strResource)
            {
                stlResponseBuffer = this->RegisterVmAsWaitingForData(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ShutdownPortal" == strResource)
            {
                stlResponseBuffer = this->ShutdownPortal(c_oRequestStructuredBuffer);
            }
            else if("/SAIL/DatabaseManager/RegisterDatasetFamily" == strResource)
            {
                stlResponseBuffer = this->RegisterDatasetFamily(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else if ("PUT" == strVerb)
        {
            if ("/SAIL/DatabaseManager/UpdateUserRights" == strResource)
            {
                stlResponseBuffer = this->UpdateUserRights(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateOrganizationInformation" == strResource)
            {
                stlResponseBuffer = this->UpdateOrganizationInformation(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateUserInformation" == strResource)
            {
                stlResponseBuffer = this->UpdateUserInformation(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RecoverUser" == strResource)
            {
                stlResponseBuffer = this->RecoverUser(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateAzureTemplate" == strResource)
            {
                stlResponseBuffer = this->UpdateAzureTemplate(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateVirtualMachine" == strResource)
            {
                stlResponseBuffer = this->UpdateVirtualMachine(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateRemoteDataConnector" == strResource)
            {
                stlResponseBuffer = this->UpdateRemoteDataConnector(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/UpdateDatasetFamily" == strResource)
            {
                stlResponseBuffer = this->UpdateDatasetFamily(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else if ("PATCH" == strVerb)
        {
            if ("/SAIL/DatabaseManager/Update/DigitalContract" == strResource)
            {
                stlResponseBuffer = this->UpdateDigitalContract(c_oRequestStructuredBuffer);
            }
            if ("/SAIL/DatabaseManager/User/Password" == strResource)
            {
                stlResponseBuffer = this->UpdatePassword(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else if ("DELETE" == strVerb)
        {
            if ("/SAIL/DatabaseManager/DeleteUser" == strResource)
            {
                stlResponseBuffer = this->DeleteUser(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/DeleteOrganization" == strResource)
            {
                stlResponseBuffer = this->DeleteOrganization(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/DeleteDataset" == strResource)
            {
                stlResponseBuffer = this->DeleteDataset(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/DeleteDatasetFamily" == strResource)
            {
                stlResponseBuffer = this->DeleteDatasetFamily(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/DeleteAzureTemplate" == strResource)
            {
                stlResponseBuffer = this->DeleteAzureTemplate(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/RemoveVmAsWaitingForData" == strResource)
            {
                stlResponseBuffer = this->RemoveVmAsWaitingForData(c_oRequestStructuredBuffer);
            }
            else if ("/SAIL/DatabaseManager/ResetDatabase" == strResource)
            {
                stlResponseBuffer = this->ResetDatabase(c_oRequestStructuredBuffer);
            }
            else
            {
                _ThrowBaseException("Invalid resource.", nullptr);
            }
        }
        else
        {
            _ThrowBaseException("Invalid resource.", nullptr);
        }
    }
    catch(...)
    {
        StructuredBuffer oError;
        oError.PutDword("Status", 404);
        stlResponseBuffer = oError.GetSerializedBuffer();
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
 * @class DatabaseManager
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
********************************************************************************************/

bool __thiscall DatabaseManager::GetResponse(
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
 * @class DatabaseManager
 * @function ShutdownPortal
 * @brief Shut down the server
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns termination signal and status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ShutdownPortal(
    _in const StructuredBuffer & c_oRequest
    )
{
    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Send termination signal
        oResponse.PutBoolean("TerminateSignalEncountered", true);
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

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ResetDatabase
 * @brief Reset the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ResetDatabase(
    _in const StructuredBuffer & c_oRequest
    )
{
    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Drop the database
            oSailDatabase.drop();
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
            dwStatus = 200;
        }
        catch (mongocxx::exception& e) 
        {
            std::cout << "Collection transaction exception: " << e.what() << std::endl;
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
