/*********************************************************************************************
 *
 * @file SailAuthentication.h
 * @author Shabana Akhtar Baig
 * @date 16 Nov 2020
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

#include <pthread.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

/********************************************************************************************/

class SailAuthentication : public Object
{
    public:

        // Constructors and Destructor
        SailAuthentication(void);
        SailAuthentication(
            _in const SailAuthentication & c_oSailAuthentication
          );
        virtual ~SailAuthentication(void);

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
        // functions, SubmitRequest and GetResponse. These functions then call SailAuthentication's
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

        // Validate a username/password/organization set of credentials
        std::vector<Byte> __thiscall AuthenticateUserCredentails(
            _in const StructuredBuffer & c_oRequest
            );

        // Take in a full EOSB, call Cryptographic plugin and fetches user guid and organization guid
        std::vector<Byte> __thiscall GetBasicUserInformation(
            _in const StructuredBuffer & c_oRequest
            );

        // Take in a nonce and send back a certificate and public key
        std::vector<Byte> __thiscall GetRemoteAttestationCertificate(
            _in const StructuredBuffer & c_oRequest
            );

        // Shutdown the servers
        std::vector<Byte> __thiscall ShutdownPortal(
            _in const StructuredBuffer & c_oRequest
            );

        // Reset the database
        std::vector<Byte> __thiscall ResetDatabase(
            _in const StructuredBuffer & c_oRequest
            );

        // private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        bool m_fTerminationSignalEncountered;
};

/********************************************************************************************/

extern SailAuthentication * __stdcall GetSailAuthentication(void);
extern void __stdcall ShutdownSailAuthentication(void);
