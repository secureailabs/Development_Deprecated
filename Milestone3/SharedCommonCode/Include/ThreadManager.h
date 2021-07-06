/*********************************************************************************************
 *
 * @file ThreadManager.h
 * @author Luis Miguel Huapaya
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TlsServer class that is used to create TLS Servers
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "SmartMemoryAllocator.h"

#include <mutex>
#include <map>

/********************************************************************************************/
 
class ThreadManager
{
    public:
    
        static ThreadManager * __stdcall GetInstance(void) throw();
        
        uint64_t __thiscall CreateThread(
            _in const char * c_szTreadGroup,
            _in void *(*ThreadRoutine) (void *),
            _in void * pParameter
            );
            
        void __thiscall TerminateThread(
            _in uint64_t un64ThreadIdentifier
            );
        void __thiscall TerminateThreadGroup(
            _in const char * c_szThreadGroup
            );
        void __thiscall TerminateAllThreads(void);
        
        void __thiscall JoinThread(
            _in uint64_t un64ThreadIdentifier
            );
        void __thiscall JoinThreadGroup(
            _in const char * c_szThreadGrounp
            );
        void __thiscall JoinAllThreads(void);
        
    private:
    
        // Singleton class means constructors, copy constructors, etc... are all private
        ThreadManager(void);
        ThreadManager(
            _in const ThreadManager & c_oThreadManager
            );
        ~ThreadManager(void);

        ThreadManager & operator=(
            _in const ThreadManager & c_oThreadManager
            );
        
        // Private methods
        void __thiscall RemoveThreadInformation(
            _in uint64_t un64ThreadIdentifier
            );
            
        // Privata data members
        std::recursive_mutex m_stlLock;
        SmartMemoryAllocator m_oMemoryAllocator;
        uint64_t m_un64NextAvailableThreadIdentifier;
        std::map<uint64_t, pthread_t *> m_stlRunningThreads;
        std::multimap<Qword, uint64_t> m_stlThreadGroupings;
};

/********************************************************************************************/

// ThreadManager is a singleton instance. Access to the ThreadManager facilities is entirely
// done through GetThreadManager() which alreadys returns the singleton instance of
// ThreadManager
extern ThreadManager * __thiscall GetThreadManager(void) throw();