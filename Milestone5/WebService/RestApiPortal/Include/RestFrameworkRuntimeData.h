/*********************************************************************************************
 *
 * @file RestFrameworkRuntimeData.h
 * @author Shabana Akhtar Baig
 * @date 29 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "EntityTypes.h"
#include "Exceptions.h"
#include "PluginDictionaryManager.h"
#include "RestFramework.h"
#include "SmartMemoryAllocator.h"
#include "TlsServer.h"
#include "HttpRequestParser.h"

#include <pthread.h>
#include <string.h>

#include <algorithm>
#include <set>
#include <sstream>

/********************************************************************************************/

class RestFrameworkRuntimeData : public Object
{
    public:

        // Constructor and Destructor
        RestFrameworkRuntimeData(
            _in PluginDictionaryManager * m_poDictionaryManager
            );
        virtual ~RestFrameworkRuntimeData(void);

        // Method call by the RestFramework to handle an incoming connection.
        // A new thread is created to process the request and send back a response.
        void __thiscall HandleConnection(
            _in TlsNode * poTlsNode
            );

        // Methods used to manage the vector m_stlConnectionThreads. A thread id is added
        // when a new thread is created and the thread erases its id from the vector before exiting.
        // Mutex is used to control access.
        void __thiscall AddConnection(
            _in pthread_t connectionThread
            );
        void __thiscall DeleteConnection(void);

        // Parse an incoming connection and routes to the target plugin. Waits for a response from
        // the plugin. Upon receiving a response, sends it to the client.
        void __thiscall RunThread(
            _in TlsNode * poTlsNode
            );

        // Methods to fetch active connections and termination flag
        unsigned int __thiscall GetNumberOfActiveConnections(void) const throw();
        bool __thiscall IsTerminationSignalEncountered(void) const throw();

        SmartMemoryAllocator m_oSmartMemoryAllocator;

    private:

        // Wrapper that calls other functions to find the plugin with the best matching API and validates parameters
        int __thiscall ParseRequestContent(
            _in const HttpRequestParser & c_oParser,
            _in StructuredBuffer * c_poRequestParameters
            );

        // Find the latest version of the plugin with the best matching API
        // If the request specifies a plugin version then checks if the plugin's version can be rounded down to the requested version
        int __thiscall FindPlugin(
            _in const HttpRequestParser & c_oParser,
            _in Qword qwRequestedPluginVersion
            );

        // Validate and parses request parameters
        bool __thiscall ValidateRequestData(
            _in const std::vector<Byte> & c_stlSerializedRequest,
            _in unsigned int unMatchingPluginIndex,
            _in const std::string & c_strResourceString,
            _in StructuredBuffer * poRequestData
            );

        // Validate the requested parameters by comparing their values against the plugin defined range
        // Add the number values in a structured buffer according to the type required by the plugin;
        // Because all number types were added as float64 when parsing from json to structured buffer.
        bool __thiscall ValidateParameter(
            _in const char * c_szParameterName,
            _in const StructuredBuffer & c_oSerializedRequest,
            _in const StructuredBuffer & c_oPluginParameter,
            _in StructuredBuffer * poRequestStructuredBuffer
            );

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        PluginDictionaryManager * m_poDictionaryManager;
        std::set<pthread_t> m_stlConnectionThreads;
        Qword m_qwRequiredNumberOfUnixConnections;
        bool m_fTerminateSignalEncountered;
};
