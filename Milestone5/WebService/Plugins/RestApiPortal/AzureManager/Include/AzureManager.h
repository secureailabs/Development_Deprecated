/*********************************************************************************************
 *
 * @file AzureManager.h
 * @author Shabana Akhtar Baig
 * @date 21 Jul 2021
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

class AzureManager : public Object
{
    public:
        // Constructor and destructor
        AzureManager(void);
        AzureManager(
            _in const AzureManager & c_oAzureManager
          );
        virtual ~AzureManager(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call AzureManager's
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

        // Take in a full EOSB and send back a StructuredBuffer containing user metadata
        std::vector<Byte> __thiscall GetUserInfo(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of all available azure settings templates of an organization
        std::vector<Byte> __thiscall GetListOfAzureSettingsTemplates(
            _in const StructuredBuffer & c_oRequest
            );
        
        // Get azure settings template associated with the GUID
        std::vector<Byte> __thiscall PullAzureSettingsTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Register azure settings template
        std::vector<Byte> __thiscall RegisterAzureSettingsTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Update azure settings template
        std::vector<Byte> __thiscall UpdateAzureSettingsTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Delete azure settings template
        std::vector<Byte> __thiscall DeleteAzureSettingsTemplate(
            _in const StructuredBuffer & c_oRequest
            );

        // Create a Virtual Network and Network Security Group if they don't exist
        void __thiscall UpdateAzureTemplateResources(
            _in const StructuredBuffer c_oRequest,
            _in const std::string c_strTemplateGuid,
            _in const std::vector<Byte> c_stlEosb
            );

        std::vector<Byte> __thiscall UpdateAzureTemplateState(
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

extern AzureManager * __stdcall GetAzureManager(void);
extern void __stdcall ShutdownAzureManager(void);
