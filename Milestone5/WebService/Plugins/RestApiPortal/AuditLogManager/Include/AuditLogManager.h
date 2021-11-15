/*********************************************************************************************
 *
 * @file AuditLogManager.h
 * @author Shabana Akhtar Baig
 * @date 02 Feb 2021
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

class AuditLogManager : public Object
{
    public:
        // Constructor and destructor
        AuditLogManager(void);
        AuditLogManager(
            _in const AuditLogManager & c_oAuditLogManager
          );
        virtual ~AuditLogManager(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        // Property setter method
        void __thiscall TerminateSignalEncountered(void);

        // Method used to initializes data members including the plugin's dictionary
        void __thiscall InitializePlugin(const StructuredBuffer& oInitializationVectors);

        // RestFrameworkRuntimeData parses an incoming connection and calls the requested plugin's flat CallBack
        // functions, SubmitRequest and GetResponse. These functions then call AuditLogManager's
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

        // Given an EOSB and a non-leaf event, call Database portal to store in the database
        std::vector<Byte> __thiscall AddNonLeafEvent(
            _in const StructuredBuffer & c_oRequest
        );

        // Given an EOSB and leaf event(s), call Database portal to store in the database
        std::vector<Byte> __thiscall AddLeafEvent(
            _in const StructuredBuffer & c_oRequest
            );
        
        // Given an EOSB get list of audit log event(s) from the database
        std::vector<Byte> __thiscall GetListOfEvents(
            _in const StructuredBuffer & c_oRequest
            );

        // Given an organization guid fetch digital contract event guid, if exists
        std::vector<Byte> __thiscall DigitalContractBranchExists(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        bool m_fTerminationSignalEncountered;

        std::string m_strDatabaseServiceIpAddr;
        uint32_t m_unDatabaseServiceIpPort;
};

/********************************************************************************************/

extern AuditLogManager * __stdcall GetAuditLogManager(void);
extern void __stdcall ShutdownAuditLogManager(void);
