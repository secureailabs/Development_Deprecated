/*********************************************************************************************
 *
 * @file DatasetDatabase.h
 * @author Shabana Akhtar Baig
 * @date 18 Nov 2020
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
#include <map>
#include <vector>

/********************************************************************************************/

class DatasetDatabase : public Object
{
    public:
        // Constructor and destructor
        DatasetDatabase(void);
        DatasetDatabase(
            _in const DatasetDatabase & c_oDatasetDatabase
          );
        virtual ~DatasetDatabase(void);

        // Property accessor methods
        const char * __thiscall GetName(void) const throw();
        const char * __thiscall GetUuid(void) const throw();
        Qword __thiscall GetVersion(void) const throw();
        std::vector<Byte> __thiscall GetDictionarySerializedBuffer(void) const throw();

        // Property setter method
        void __thiscall TerminateSignalEncountered(void);

        // Method used to initializes data members including the plugin's dictionary
        void __thiscall InitializePlugin(
            _in const StructuredBuffer& oInitializationVectors
            );

        // RestFrameworkRuntimeData parses an incoming connection and calls the requested plugin's flat CallBack
        // functions, SubmitRequest and GetResponse. These functions then call DatasetDatabase's
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

        // Take in a full EOSB and send back a StructuredBuffer containing user metadata
        std::vector<Byte> __thiscall GetUserInfo(
            _in const StructuredBuffer & c_oRequest
            );

        // Fetch list of all available datasets metadata
        std::vector<Byte> __thiscall GetListOfAvailableDatasets(
            _in const StructuredBuffer & c_oRequest
            );

        // Get metadata of the dataset associated with the GUID
        std::vector<Byte> __thiscall PullDataset(
            _in const StructuredBuffer & c_oRequest
            );

        // Register dataset
        std::vector<Byte> __thiscall RegisterDataset(
            _in const StructuredBuffer & c_oRequest
            );
        // Delete dataset
        std::vector<Byte> __thiscall DeleteDataset(
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

extern DatasetDatabase * __stdcall GetDatasetDatabase(void);
extern void __stdcall ShutdownDatasetDatabase(void);
