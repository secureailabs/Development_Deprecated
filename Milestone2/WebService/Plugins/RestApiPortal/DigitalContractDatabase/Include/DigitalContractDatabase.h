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
#include "Exceptions.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "DigitalContract.h"
#include "UserAccount.h"

#include <pthread.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

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

        // Initialize User Accounts
        void __thiscall InitializeUserAccounts(void);

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

    private:

        // Fetch list of all digital contracts whose organization is the same as the requesting user's organization
        std::vector<Byte> __thiscall GetListOfMyDigitalContracts(
            _in const StructuredBuffer & c_oRequest
            );
        // Fetch list of all digital contracts that are in flux
        std::vector<Byte> __thiscall GetListOfWaitingDigitalContracts(
            _in const StructuredBuffer & c_oRequest
            );

        // Register digital contract
        std::vector<Byte> __thiscall RegisterDigitalContract(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        std::vector<DigitalContract *> m_stlDigitalContracts;
        std::vector<UserAccount *> m_stlUserAccounts;
};

/********************************************************************************************/

extern DigitalContractDatabase * __stdcall GetDigitalContractDatabase(void);
extern void __stdcall ShutdownDigitalContractDatabase(void);
