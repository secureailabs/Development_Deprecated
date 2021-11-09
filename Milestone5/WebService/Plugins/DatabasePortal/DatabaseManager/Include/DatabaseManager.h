/*********************************************************************************************
 *
 * @file DatabaseManager.h
 * @author Shabana Akhtar Baig
 * @date 22 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CryptoUtils.h"
#include "DateAndTime.h"
#include "64BitHashes.h"
#include "DebugLibrary.h"
#include "EntityTypes.h"
#include "ExceptionRegister.h"
#include "Exceptions.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "Utils.h"

#include <pthread.h>
#include <string.h>
#include <mongocxx/pool.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/client_session.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <iostream>
#include <map>
#include <vector>

using bsoncxx::type;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

/********************************************************************************************/

class DatabaseManager : public Object
{
    public:
        // Constructor and destructor
        DatabaseManager(void);
        DatabaseManager(
            _in const DatabaseManager & c_oDatabaseManager
          );
        virtual ~DatabaseManager(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        // Property setter method
        void __thiscall TerminateSignalEncountered(void);

        // Method used to initializes data members including the plugin's dictionary
        void __thiscall InitializePlugin(void);

        // RestFrameworkRuntimeData parses an incoming connection and calls the requested plugin's flat CallBack
        // functions, SubmitRequest and GetResponse. These functions then call DatabaseManager's
        // SubmitRequest and GetResponse functions.
        // This function calls the requested resource and sends back a uinque transaction identifier.
        uint64_t __thiscall SubmitRequest(
            _in const StructuredBuffer & c_oRequestStructuredBuffer,
            _out unsigned int * punSerializedResponseSizeInBytes
            );

        // This function sends back the response associated with un64Identifier
        bool __thiscall GetResponse(
            _in uint64_t un64Identifier,
            _out Byte * pbSerializedResponseBuffer,
            _in unsigned int unSerializedResponseBufferSizeInBytes
            );

    private:

        // Given an organization guid fetch digital contract event guid, if exists
        std::vector<Byte> __thiscall DigitalContractBranchExists(
            _in const StructuredBuffer & c_oRequest
            );
        // Fetch audit log records from the database
        std::vector<Byte> __thiscall GetListOfEvents(
            _in const StructuredBuffer & c_oRequest
            );
        // Apply filters and add logs to the listofevents
        void __thiscall GetEventObjectBlob(
            _in const mongocxx::database & c_oSailDatabase,
            _in const StructuredBuffer & c_oFilters,
            _in const bsoncxx::document::view & oDocumentView,
            _out StructuredBuffer * poListOfEvents
            );
        // Fetch next sequence number from the parent event and update the parent event's next sequence number
        uint32_t __thiscall GetNextSequenceNumber(
            _in const StructuredBuffer & c_oRequest
            );

        // Store root/branch event in the database
        std::vector<Byte> __thiscall AddNonLeafEvent(
            _in const StructuredBuffer & c_oRequest
        );
        // Store leaf event(s) in the database
        std::vector<Byte> __thiscall AddLeafEvent(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch the virtual machine information
        std::vector<Byte> __thiscall PullVirtualMachine(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch the virtual machines associated with a digital contract
        std::vector<Byte> __thiscall ListOfVmsAssociatedWithDc(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch the virtual machines' ip addresses associated with a digital contract
        std::vector<Byte> __thiscall ListOfVmIpAddressesAssociatedWithDc(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of VM ipaddressese that are waiting for the remote data connector's dataset(s)
        std::vector<Byte> __thiscall GetVmsWaitingForData(
            _in const StructuredBuffer & c_oRequest
            );

        // Add metadata of a new virtual machine to the database
        std::vector<Byte> __thiscall RegisterVirtualMachine(
            _in const StructuredBuffer & c_oRequest
            );

        // Add metadata of a virtual machine that is waiting for data to the database
        std::vector<Byte> __thiscall RegisterVmAsWaitingForData(
            _in const StructuredBuffer & c_oRequest
            );

        // Update metadata of virtual machine in the database
        std::vector<Byte> __thiscall UpdateVirtualMachine(
            _in const StructuredBuffer & c_oRequest
            );

        // Remove metadata of virtual machine as waiting for data in the database
        std::vector<Byte> __thiscall RemoveVmAsWaitingForData(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch user's organization information from the database
        std::vector<Byte> __thiscall GetOrganizationInformation(
            _in const StructuredBuffer & c_oRequest
            );
        // Fetch basic organization record from the database
        std::vector<Byte> __thiscall GetBasicOrganizationRecord(
            _in const StructuredBuffer & c_oRequest
            );
        // Get organization name associated to a guid
        std::vector<Byte> __thiscall GetOrganizationName(
            _in const std::string & c_strOrganizationGuid
            );
        // Fetch basic user record from the database
        std::vector<Byte> __thiscall GetBasicUserRecord(
            _in const StructuredBuffer & c_oRequest
            );
        // Fetch confidential user record from the database
        std::vector<Byte> __thiscall GetConfidentialUserRecord(
            _in const StructuredBuffer & c_oRequest
            );

        // Add a new organization to the database and call AddSuperUser() to add the organziation's super user
        std::vector<Byte> __thiscall RegisterOrganization(
            _in const StructuredBuffer & c_oRequest
            );
        // Add organziation's super user to the database
        std::vector<Byte> __thiscall AddSuperUser(
            _in const StructuredBuffer & c_oRequest,
            _in mongocxx::pool::entry & oClient,
            _in mongocxx::v_noabi::client_session ** poSession
            );
        // Add a new user to the database
        std::vector<Byte> __thiscall RegisterUser(
            _in const StructuredBuffer & c_oRequest
            );

        // Update user's access rights
        std::vector<Byte> __thiscall UpdateUserRights(
            _in const StructuredBuffer & c_oRequest
            );
        // Update organization information
        std::vector<Byte> __thiscall UpdateOrganizationInformation(
            _in const StructuredBuffer & c_oRequest
            );
        // Update user information, excluding access rights
        std::vector<Byte> __thiscall UpdateUserInformation(
            _in const StructuredBuffer & c_oRequest
            );

        // Update user password
        std::vector<Byte> __thiscall UpdatePassword(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch a list of all organizations
        std::vector<Byte> __thiscall ListOrganizations(
            _in const StructuredBuffer & c_oRequest
            );
        // Return a list of all users
        std::vector<Byte> __thiscall ListUsers(
            _in const StructuredBuffer & c_oRequest
            );
        // Return a list of users for an organization
        std::vector<Byte> __thiscall ListOrganizationUsers(
            _in const StructuredBuffer & c_oRequest
            );

        // Recover a deleted user in the database
        std::vector<Byte> __thiscall RecoverUser(
            _in const StructuredBuffer & c_oRequest
            );

        // Delete a user from the database
        std::vector<Byte> __thiscall DeleteUser(
            _in const StructuredBuffer & c_oRequest
            );
        // Delete an organization and its users from the database
        std::vector<Byte> __thiscall DeleteOrganization(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of all digital contracts associated with the user's organization
        std::vector<Byte> __thiscall ListDigitalContracts(
            _in const StructuredBuffer & c_oRequest
            );
        // Fetch the digital contract information
        std::vector<Byte> __thiscall PullDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );

        // Register digital contract
        std::vector<Byte> __thiscall RegisterDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );

        // Update the digital contract when a data owner accepts the digital contract or when a researcher accepts the DC terms from the Data owner organization
        std::vector<Byte> __thiscall UpdateDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );

        // Get dataset name associated to a guid
        std::vector<Byte> __thiscall GetDatasetName(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of all available datasets
        std::vector<Byte> __thiscall ListDatasets(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch the dataset information
        std::vector<Byte> __thiscall PullDataset(
            _in const StructuredBuffer & c_oRequest
            );

        // Register a dataset
        std::vector<Byte> __thiscall RegisterDataset(
            _in const StructuredBuffer & c_oRequest
            );

        // Delete a dataset 
        std::vector<Byte> __thiscall DeleteDataset(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of azure settings templates
        std::vector<Byte> __thiscall ListAzureTemplates(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch an azure settings template
        std::vector<Byte> __thiscall PullAzureTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Register an azure settings template
        std::vector<Byte> __thiscall RegisterAzureTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Update an azure settings template
        std::vector<Byte> __thiscall UpdateAzureTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Delete an azure settings template
        std::vector<Byte> __thiscall DeleteAzureTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of remote data connectors
        std::vector<Byte> __thiscall ListRemoteDataConnectors(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch a remote data connector's metadata
        std::vector<Byte> __thiscall PullRemoteDataConnector(
            _in const StructuredBuffer & c_oRequest
            );

        // Register a remote data connector
        std::vector<Byte> __thiscall RegisterRemoteDataConnector(
            _in const StructuredBuffer & c_oRequest
            );

        // Update a remote data connector
        std::vector<Byte> __thiscall UpdateRemoteDataConnector(
            _in const StructuredBuffer & c_oRequest
            );

        // Register a dataset family
        std::vector<Byte> __thiscall RegisterDatasetFamily(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of all digital contracts associated with the user's organization
        std::vector<Byte> __thiscall ListDatasetFamilies(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch a single dataset family's metadata
        std::vector<Byte> __thiscall PullDatasetFamily(
            _in const StructuredBuffer & c_oRequest
            );

        std::vector<Byte> __thiscall UpdateDatasetFamily(
            _in const StructuredBuffer &c_oRequest
            );

        std::vector<Byte> __thiscall DeleteDatasetFamily(
            _in const StructuredBuffer &c_oRequest
            );

        // Shutdown the server
        std::vector<Byte> __thiscall ShutdownPortal(
            _in const StructuredBuffer & c_oRequest
            );

        // Reset the database
        std::vector<Byte> __thiscall ResetDatabase(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        std::unique_ptr<mongocxx::pool> m_poMongoPool;
        bool m_fTerminationSignalEncountered;
};

/********************************************************************************************/

extern DatabaseManager * __stdcall GetDatabaseManager(void);
extern void __stdcall ShutdownDatabaseManager(void);
