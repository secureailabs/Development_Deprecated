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

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "UserAccount.h"

#include <mongocxx/client.hpp>
#include <pthread.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

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

        // Fetch basic user record from the database
        std::vector<Byte> __thiscall GetBasicUserRecord(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch confidential user record from the database
        std::vector<Byte> __thiscall GetConfidentialUserRecord(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        std::vector<UserAccount *> m_stlUserAccounts;
        mongocxx::client * m_poMongoClient;
};

/********************************************************************************************/

extern DatabaseManager * __stdcall GetDatabaseManager(void);
extern void __stdcall ShutdownDatabaseManager(void);
