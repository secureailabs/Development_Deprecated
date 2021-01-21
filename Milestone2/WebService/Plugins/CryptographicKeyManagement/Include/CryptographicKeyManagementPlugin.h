/*********************************************************************************************
 *
 * @file CryptographicKeyManagementPlugin.h
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
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

#include <string.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <uuid/uuid.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

/********************************************************************************************/

class CryptographicKeyManagementPlugin : public Object
{
    public:

        // Constructors and Destructor
        CryptographicKeyManagementPlugin(void);
        CryptographicKeyManagementPlugin(
            _in const CryptographicKeyManagementPlugin & c_oCryptographicKeyManagementPlugin
          );
        virtual ~CryptographicKeyManagementPlugin(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        // Method used to initializes data members including the plugin's dictionary
        void __thiscall InitializePlugin(void);

        // RestFrameworkRuntimeData parses an incoming connection and calls the requested plugin's flat CallBack
        // functions, SubmitRequest and GetResponse. These functions then call CryptographicKeyManagementPlugin's
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

        // Generates a Nonce to be sent back to the Initializer
        std::vector<Byte> __thiscall GetChallengeNonceForInitializer(
            _in const StructuredBuffer & c_oRequest,
            _in uint64_t un64BitIdentifier
            );
        // Checks if the Proof is the same as the Nonce generated to the Initializer to avoid replay attacks.
        // Sends back signed certificate.
        std::vector<Byte> __thiscall GetKeyPairAndCertificateForInitializer(
            _in const StructuredBuffer & c_oRequest
            );
        // Takes in the SHA2 hash of the dataset and signs it with the private key of the dataOwner
        // And returns a signed hash with the public key certificate
        std::vector<Byte> __thiscall GetSignedHashAndCertificate(
            _in const StructuredBuffer & c_oRequest
            ) throw();

        // private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlChallengeNonce;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
};

/********************************************************************************************/

extern CryptographicKeyManagementPlugin * __stdcall GetCryptographicKeyManagementPlugin(void);
extern void __stdcall ShutdownCryptographicKeyManagementPlugin(void);
