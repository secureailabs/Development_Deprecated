/*********************************************************************************************
 *
 * @file DigitalContractDatabase.h
 * @author Shabana Akhtar Baig
 * @date 23 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "DebugLibrary.h"
#include "EntityTypes.h"
#include "ExceptionRegister.h"
#include "Exceptions.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "Socket.h"
#include "SocketServer.h"
#include "ThreadManager.h"

#include <pthread.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

#define SAIL_EULA "Company grants You a revocable, non-exclusive, non-transferable, limited right to use the dataset on a single machine."

/********************************************************************************************/

class DigitalContractDatabase : public Object
{
    public:
        // Constructor and destructor
        DigitalContractDatabase(void);
        DigitalContractDatabase(
            _in const DigitalContractDatabase & c_oDigitalContractDatabase
            );
        virtual ~DigitalContractDatabase(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call DigitalContractDatabase's
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

        // Start the Ipc server
        void __thiscall RunIpcServer(
            _in SocketServer * poIpcServer,
            _in ThreadManager * poThreadManager
        );

        // Handle an incoming Ipc request and call the relevant function based on the identifier
        void __thiscall HandleIpcRequest(
            _in Socket * poSocket
            );

    private:

        // Given an EOSB, return a StructuredBuffer containing user metadata
        std::vector<Byte> __thiscall GetUserInfo(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch digital signature blob for the given content
        std::vector<Byte> __thiscall GetDigitalSignature(
            _in const StructuredBuffer & c_oRequest
            );

        // Serialize a digital contract structured buffer
        void __thiscall SerializeDigitalContract(
            _in const StructuredBuffer & c_oDc,
            _in std::vector<Byte> & stlDigitalContractBlob
            );
        // Deserialize a digital contract blob
        StructuredBuffer __thiscall DeserializeDigitalContract(
            _in const std::vector<Byte> c_stlDcBlob
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

        // Update the digital contract when a data owner accepts the digital contract
        std::vector<Byte> __thiscall AcceptDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );
        // Update the digital contract when a researcher accepts the DC terms from the Data owner organization
        std::vector<Byte> __thiscall ActivateDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );
        // Activate a Digital Contract when the Researcher wants to create a Virtual Machine
        std::vector<Byte> __thiscall ProvisionDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );
        void __thiscall DeleteVirtualMachineResources(
            _in const std::vector<Byte> c_stlEosb,
            _in const StructuredBuffer c_oTemplate,
            _in const std::string c_strVirtualMachineName
            )  const throw();
        std::vector<Byte> __thiscall DeprovisionDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );

        // Provision a Virtual Machine
        void __thiscall ProvisionVirtualMachine(
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
            );

        std::vector<Byte> __thiscall InitializeVirtualMachine(
            _in const StructuredBuffer & c_oRequest
        );

        // Update the Provisioning Status of Digital Contract to
        std::vector<Byte> __thiscall UpdateDigitalContractProvisioningStatus(
            _in const StructuredBuffer & c_oRequest
            );

        // Associate one or more digital contracts with one Azure template
        std::vector<Byte> __thiscall AssociateWithAzureTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Send back status of the digital contract provisioning
        std::vector<Byte> __thiscall GetProvisioningStatus(
            _in const StructuredBuffer & c_oRequest
            );

        // Add DC branch event to the database
        std::vector<Byte> __thiscall RegisterDcAuditEvent(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        bool m_fTerminationSignalEncountered;
};

/********************************************************************************************/

extern DigitalContractDatabase * __stdcall GetDigitalContractDatabase(void);
extern void __stdcall ShutdownDigitalContractDatabase(void);
