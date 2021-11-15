/*********************************************************************************************
 *
 * @file DatasetFamilyManager.h
 * @author David Gascon
 * @date 28 October 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
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

class DatasetFamilyManager : public Object
{
    public:
        // Constructor and destructor
        DatasetFamilyManager(void);
        DatasetFamilyManager(
            _in const DatasetFamilyManager & c_oAuditLogManager
          );
        virtual ~DatasetFamilyManager(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        void __thiscall TerminateSignalEncountered(void);

        void __thiscall InitializePlugin(const StructuredBuffer& oInitializationVectors);

        uint64_t __thiscall SubmitRequest(
            _in const StructuredBuffer & c_oRequestStructuredBuffer,
            _out unsigned int * punSerializedResponseSizeInBytes
            );

        bool __thiscall GetResponse(
            _in uint64_t un64Identifier,
            _out Byte * pbSerializedResponseBuffer,
            _in unsigned int unSerializedResponseBufferSizeInBytes
            );

    private:

        // Register dataset family
        std::vector<Byte> __thiscall RegisterDatasetFamily(
            _in const StructuredBuffer & c_oRequest
            );

        std::vector<Byte> __thiscall ListDatasetFamilies(
            _in const StructuredBuffer & c_oRequest
            );

        // Update dataset family information
        std::vector<Byte> __thiscall EditDatasetFamilyInformation(
            _in const StructuredBuffer & c_oRequest
            );

        std::vector<Byte> __thiscall PullDatasetFamily(
            _in const StructuredBuffer & c_oRequest
            );

        std::vector<Byte> __thiscall DeleteDatasetFamily(
            _in const StructuredBuffer & c_oRequest
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        std::map<Qword, std::vector<Byte>> m_stlCachedResponse;
        uint64_t m_unNextAvailableIdentifier;
        PluginDictionary m_oDictionary;
        bool m_fTerminationSignalEncountered;
        uint64_t m_unNumHandledPosts{0};

        std::string m_strDatabaseServiceIpAddr;
        uint32_t m_unDatabaseServiceIpPort;
};

/********************************************************************************************/

extern DatasetFamilyManager * __stdcall GetDatasetFamilyManager(void);
extern void __stdcall ShutdownDatasetFamilyManager(void);
