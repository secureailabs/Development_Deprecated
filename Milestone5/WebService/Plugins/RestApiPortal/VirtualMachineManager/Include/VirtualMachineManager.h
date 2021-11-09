/*********************************************************************************************
 *
 * @file VirtualMachineManager.h
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
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

/********************************************************************************************/

class VirtualMachineManager : public Object
{
    public:
        // Constructor and destructor
        VirtualMachineManager(void);
        VirtualMachineManager(
            _in const VirtualMachineManager & c_oVirtualMachineManager
          );
        virtual ~VirtualMachineManager(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call VirtualMachineManager's
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

        // Fetch list of running VMs associated with the digital contract
        std::vector<Byte> __thiscall GetListOfOrganizationVMs(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of running VMs' ip addresses associated with the digital contract
        std::vector<Byte> __thiscall GetListOfVmIpAddressesAssociatedWithDc(
            _in const StructuredBuffer & c_oRequest
            );

        // Take in a full IEOSB and send back a StructuredBuffer containing user metadata
        std::vector<Byte> __thiscall GetUserInfo(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch the virtual machine information
        std::vector<Byte> __thiscall GetVmInformation(
            _in const StructuredBuffer & c_oRequest
            );

        // Check if IEosb is of the data owner of the digital contract associated with the strDcGuid
        std::vector<Byte> __thiscall VerifyDigitalContract(
            _in const StructuredBuffer & c_oRequest,
            _in bool fIsEitherRoOrDoo,
            _in bool fIsResearcher
            );

        // Add the vm record to the database
        std::vector<Byte> __thiscall RegisterVmInstance(
            _in const StructuredBuffer & c_oRequest
            );

        // Take in VmEOSB of the data owner and add a VM branch event
        std::vector<Byte> __thiscall RegisterVmAfterDataUpload(
            _in const StructuredBuffer & c_oRequest
            );

        // Take in VmEOSB of the researcher and add a VM branch event
        std::vector<Byte> __thiscall RegisterVmForComputation(
            _in const StructuredBuffer & c_oRequest
            );

        // Register VM audit event
        std::vector<Byte> __thiscall RegisterVmAuditEvent(
            _in const StructuredBuffer & c_oRequest
            );

        // Update a virtual machine's status
        std::vector<Byte> __thiscall UpdateVirtualMachineStatus(
            _in const StructuredBuffer & c_oRequest
            );

        std::vector<Byte> __thiscall AddVirtualMachineIpAddress(
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

extern VirtualMachineManager * __stdcall GetVirtualMachineManager(void);
extern void __stdcall ShutdownVirtualMachineManager(void);
