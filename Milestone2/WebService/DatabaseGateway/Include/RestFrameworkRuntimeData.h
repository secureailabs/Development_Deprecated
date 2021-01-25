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

#include "Exceptions.h"
#include "PluginDictionaryManager.h"
#include "RestFramework.h"
#include "SmartMemoryAllocator.h"
#include "TlsServer.h"

#include <algorithm>
#include <pthread.h>
#include <string.h>

#include <set>

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

        // Parses an incoming connection and routes to the target plugin. Waits for a response from
        // the plugin. Upon receiving a response, sends it to the client.
        void __thiscall RunThread(
            _in TlsNode * poTlsNode
            );

        // Methods to fetch active connections and termination flag
        unsigned int __thiscall GetNumberOfActiveConnections(void) const throw();
        bool __thiscall IsTerminationSignalEncountered(void) const throw();

        SmartMemoryAllocator m_oSmartMemoryAllocator;

    private:

        // Private data members
        mutable pthread_mutex_t m_sMutex;
        PluginDictionaryManager * m_poDictionaryManager;
        std::set<pthread_t> m_stlConnectionThreads;
        bool m_fTerminateSignalEncountered;
};
