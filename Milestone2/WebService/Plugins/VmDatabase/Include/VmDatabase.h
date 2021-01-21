/*********************************************************************************************
 *
 * @file VmDatabase.h
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
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
#include "VmInstance.h"
#include "UserAccount.h"

#include <pthread.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

/********************************************************************************************/

class VmDatabase : public Object
{
    public:
        // Constructor and destructor
        VmDatabase(void);
        VmDatabase(
            _in const VmDatabase & c_oVmDatabase
          );
        virtual ~VmDatabase(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call VmDatabase's
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

        // Fetch list of running Vm instances
        std::vector<Byte> __thiscall GetListOfRunningVmInstances(
            _in const StructuredBuffer & c_oRequest
            );

        // Report if a VmInstance is still running or not
        std::vector<Byte> __thiscall GetVmHeartBeat(
            _in const StructuredBuffer & c_oRequest
            );

        // Register a VmInstance
        std::vector<Byte> __thiscall RegisterVmInstance(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        std::vector<VmInstance *> m_stlVmInstances;
        std::vector<UserAccount *> m_stlUserAccounts;
};

/********************************************************************************************/

extern VmDatabase * __stdcall GetVmDatabase(void);
extern void __stdcall ShutdownVmDatabase(void);
