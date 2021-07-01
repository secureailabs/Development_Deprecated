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
#include "ExceptionRegister.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "CryptographicEngine.h"
#include "Socket.h"
#include "SocketServer.h"
#include "ThreadManager.h"

#include <string.h>
#include <pthread.h>
#include <uuid/uuid.h>

#include <iostream>
#include <algorithm>
#include <condition_variable>
#include <functional>
#include <string>
#include <vector>

/********************************************************************************************/

class CryptographicKeyManagementPlugin : public Object
{
    public:

        // Constructors and Destructor
        ~CryptographicKeyManagementPlugin(void);
        CryptographicKeyManagementPlugin(
            _in const CryptographicKeyManagementPlugin & c_oCryptographicKeyManagementPlugin
            ) = delete;
        // The static function for the class that can get the reference to the
        // CryptographicEngine singleton object
        static CryptographicKeyManagementPlugin * __stdcall Get(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        // Property setter method
        void __thiscall TerminateSignalEncountered(void);

        // Method used to initializes data members including the plugin's dictionary
        void __thiscall InitializePlugin(void);

        // Start the Ipc server
        void __thiscall RunIpcServer(
            _in SocketServer * poIpcServer,
            _in ThreadManager * poThreadManager
        );

        // Handle an incoming Ipc request and call the relevant function based on the identifier
        void __thiscall HandleIpcRequest(
            _in Socket * poSocket
            );

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

        void __thiscall RotateEphemeralKeys(void);

    private:

        CryptographicKeyManagementPlugin(void);
        static CryptographicKeyManagementPlugin m_oCryptographicKeyManagementPlugin;

        std::vector<Byte> __thiscall GenerateEosb(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall RefreshEosb(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall RegisterEosb(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall UnregisterEosb(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall ModifyEosbAccessRights(
            _in const StructuredBuffer & c_oRequest
            );
        std::vector<Byte> __thiscall CheckEosbAccessRights(
            _in const StructuredBuffer & c_oRequest
            );
        std::vector<Byte> __thiscall CreateEosbFromPlainSsb(
            _in const std::vector<Byte> & c_stlPlainTextSsb
            );
        std::vector<Byte> __thiscall GetPlainTextSsbFromEosb(
            _in const std::vector<Byte> & c_stlPlainTextSsb
            );
        std::vector<Byte> __thiscall SignMessageDigest(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall VerifySignature(
            _in const StructuredBuffer & c_stlRequest
            );
        std::vector<Byte> __thiscall CreateDigitalSignature(
            _in const StructuredBuffer & c_oStructuredBufferRequest
            );
        std::vector<Byte> __thiscall GetUserInfoAndUpdateEosb(
            _in const StructuredBuffer & c_oStructuredBufferRequest
            );

        // private data members
        const Guid m_oPluginGuid;
        const std::string m_strPluginName;
        const std::vector<Byte> m_stlEosbHeader = {0xE6, 0x21, 0x10, 0x02, 0x1B, 0x65, 0xA1, 0x23};
        const std::vector<Byte> m_stlEosbFooter = {0x32, 0x1A, 0x56, 0xB1, 0x29, 0x91, 0x12, 0x6E};
        // Always use the following key Guid by taking the m_sEosbKeyMutex lock on them
        Guid m_oGuidEosbCurrentKey = (const char *)nullptr;
        Guid m_oGuidEosbPredecessorKey = (const char *)nullptr;
        uint64_t m_unKeyRotationThreadID;
        uint64_t m_unNextAvailableIdentifier;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unKeyRotateThreadID;
        PluginDictionary m_oDictionary;
        bool m_fTerminationSignalEncountered;
        pthread_mutex_t m_sMutex;
        pthread_mutex_t m_sEosbKeyMutex;
        pthread_mutex_t m_sEosbCacheMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedKeysForEosb;
        // Data members for managing the thread that rotates the keys
        std::condition_variable m_oRotateKeysConditionalVariable;
};

/********************************************************************************************/

extern void __stdcall ShutdownCryptographicKeyManagementPlugin(void);

