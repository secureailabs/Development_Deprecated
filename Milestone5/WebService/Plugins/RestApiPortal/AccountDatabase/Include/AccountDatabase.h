/*********************************************************************************************
 *
 * @file AccountDatabase.h
 * @author Shabana Akhtar Baig
 * @date 17 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "DebugLibrary.h"
#include "EntityTypes.h"
#include "Exceptions.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "ExceptionRegister.h"
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

class AccountDatabase : public Object
{
    public:
        // Constructor and destructor
        AccountDatabase(void);
        AccountDatabase(
            _in const AccountDatabase & c_oAccountDatabase
          );
        virtual ~AccountDatabase(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call AccountDatabase's
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

        // Given a email/password string, fetch Basic and Confidential records from the database
        std::vector<Byte> __thiscall GetUserRecords(
            _in const StructuredBuffer & c_oRequest
        );

        // Given an EOSB, return a StructuredBuffer containing user metadata
        std::vector<Byte> __thiscall GetUserInfo(
            _in const StructuredBuffer & c_oRequest
            );

        // Given an Eosb, return the user's organization information
        std::vector<Byte> __thiscall GetOrganizationInformation(
            _in const StructuredBuffer & c_oRequest
        );

        // Register an organization and the new user as it's super user
        std::vector<Byte> __thiscall RegisterOrganizationAndSuperUser(
            _in const StructuredBuffer & c_oRequest
            );

        // Add a UserAccount record
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

extern AccountDatabase * __stdcall GetAccountDatabase(void);
extern void __stdcall ShutdownAccountDatabase(void);
