/*********************************************************************************************
 *
 * @file RestFramework.h
 * @author Shabana Akhtar Baig
 * @date 20 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 * The REST Framework interfaces with the TLS Library and provides an interface to REST API plugins.
 * It provides Initialization/Shutdown interfaces.
 * Once initialized, it will be ready to register plugins.
 * When one or more plugins are registered, the framework will parse incoming requests.
 * It will find the best matching plugin and call it's callback function.
 * It will send back the response received from the plugin to the client.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "Guid.h"
#include "RestFrameworkRuntimeData.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"
#include "TlsServer.h"

#include <unistd.h>
#include <uuid/uuid.h>
#include <dlfcn.h>

#include <iostream>
#include <experimental/filesystem>
#include <list>
#include <map>
#include <string>
#include <thread>
#include <vector>

/********************************************************************************************/

class RestFramework : public Object
{
    public:

        // Constructors and Destructor
        RestFramework(
            _in Word wPortNumber,
            // _in CryptographicTrustStore & oCryptographicTrustStore,
            _in const std::string & c_strPluginFolderPath,
            _in const StructuredBuffer & c_oPluginInitializationVectors
            );
        RestFramework (
            _in const char * c_szUnixSocketAddress,
            // _in CryptographicTrustStore & oCryptographicTrustStore,
            _in const std::string & c_strPluginFolderPath
            );
        RestFramework(
            _in const RestFramework & c_oRestFramework
            );
        virtual ~RestFramework(void);

        // Method for plugins to register themselves
        bool __thiscall RegisterPlugin(
            _in const char * c_szPluginName,
            _in const char * c_szIdentifier,
            _in Qword qwPluginVersion,
            _in SubmitRequestFn fnSubmitRequest,
            _in GetResponseFn fnGetResponse,
            _in const Byte * c_pbSerializedDictionary,
            _in unsigned int unSerializedDictionarySizeInBytes
            );

        // Method to start and run the server
        void __thiscall RunServer(void);

    private:

        // Method that loads all plugins and stores their handles in a vector
        void __thiscall LoadPlugins(
            _in const std::string & c_strPluginFolderPath
            );

        // Private data members
        std::vector<void *> m_stlPluginHandles;
        TlsServer * m_poTlsServer;
        PluginDictionaryManager * m_poDictionaryManager;
        StructuredBuffer m_oPluginInitializationVectors;
};
