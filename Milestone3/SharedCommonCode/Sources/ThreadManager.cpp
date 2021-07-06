/*********************************************************************************************
 *
 * @file ThreadManager.cpp
 * @author Luis Miguel Huapaya
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TlsServer class that is used to create TLS Servers
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "ThreadManager.h"

#include <pthread.h>

#include <signal.h>
#include <map>
#include <queue>

static ThreadManager * gs_poThreadManager = nullptr;

/********************************************************************************************/

ThreadManager * __stdcall ThreadManager::GetInstance(void) throw()
{
    __DebugFunction();
   
    if (nullptr == gs_poThreadManager)
    {
        ThreadManager * poThreadManager = new ThreadManager();
        if (nullptr != poThreadManager)
        {
            if (false == ::__sync_bool_compare_and_swap(&gs_poThreadManager, nullptr, poThreadManager))
            {
                delete poThreadManager;
            }
        }
    }
    
    return gs_poThreadManager;
}

/********************************************************************************************/

ThreadManager::ThreadManager(void)
{
    __DebugFunction();
    //__DebugAssert(this == gs_poThreadManager);
}

/********************************************************************************************/

ThreadManager::ThreadManager(
    _in const ThreadManager & c_oThreadManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
    
    UNREFERENCED_PARAMETER(c_oThreadManager);
}
            
/********************************************************************************************/

ThreadManager::~ThreadManager(void)
{
    __DebugFunction();

    // If there are ANY threads left running, we have to 'hard kill' them all, since obviously
    // the code using ThreadManager wasn't nice enough to deal with things 'cleanly'
    
    this->TerminateAllThreads();
    
    // Interestingly enough, we will NOT reset m_fIsInitialized to false. This is because the
    // ThreadManager is a singleton instance. It should never be possible to create a new
    // instance of this class apart from the original implicitly allocated g_oThreadManager
}

/********************************************************************************************/

ThreadManager & ThreadManager::operator=(
    _in const ThreadManager & c_oThreadManager
    )
{
    __DebugFunction();
    __DebugAssert(false);
    
    UNREFERENCED_PARAMETER(c_oThreadManager);
}
            
/********************************************************************************************/

uint64_t __thiscall ThreadManager::CreateThread(
    _in const char * c_szTreadGroup,
    _in void * (* fnThreadRoutine) (void *),
    _in void * pParameter
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    __DebugAssert(nullptr != fnThreadRoutine);
    
    uint64_t un64ThreadIdentifier = 0xFFFFFFFFFFFFFFFF;
    pthread_t * psNewThread = (pthread_t *) m_oMemoryAllocator.Allocate(sizeof(pthread_t), true);
    _ThrowOutOfMemoryExceptionIfNull(psNewThread);
    
    // Create the thread
    if (0 == ::pthread_create(psNewThread, nullptr, fnThreadRoutine, pParameter))
    {
        // If the thread was successfully created, register it and give it a unique identifier
        m_stlLock.lock();
        un64ThreadIdentifier = m_un64NextAvailableThreadIdentifier++;
        m_stlRunningThreads.insert(std::pair<uint64_t, pthread_t *>(un64ThreadIdentifier, psNewThread));
        m_stlThreadGroupings.insert(std::pair<Qword, uint64_t>(::Get64BitHashOfNullTerminatedString(c_szTreadGroup, false), un64ThreadIdentifier));
        m_stlLock.unlock();
    }
    else
    {
        // If the pthread_create() failed, always make sure to deallocate psNewThread since
        // it's useless and won't be recorded anywhere
        m_oMemoryAllocator.Deallocate(psNewThread);
    }

    return un64ThreadIdentifier;
}
                          
/********************************************************************************************/

void __thiscall ThreadManager::TerminateThread(
    _in uint64_t un64ThreadIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    m_stlLock.lock();
    if (m_stlRunningThreads.end() != m_stlRunningThreads.find(un64ThreadIdentifier))
    {
        pthread_t * psTargetThread = m_stlRunningThreads.at(un64ThreadIdentifier);
        __DebugAssert(nullptr != psTargetThread);
        // Send a signal to the target thread. This should never fail, but we will track
        // the return value anyway, just in case. However, we will not check to see if
        // nReturnValue is 0 (for success) until AFTER we exit the spin lock, since an
        // error will cause an exception to be thrown
        (void) ::pthread_kill(*psTargetThread, 0);
        // Make sure to deallocate the heap allocated psTargetThread structure. This will
        // happen no matter if pthread_kill succeeded or not
        m_oMemoryAllocator.Deallocate((void *) psTargetThread);
        // Make sure to remove the qwThreadIdentifier entry from m_stlRunningThreads and
        // m_stlThreadGroupings.
        this->RemoveThreadInformation(un64ThreadIdentifier);
    }
    m_stlLock.unlock();    
}

/********************************************************************************************/

void __thiscall ThreadManager::TerminateThreadGroup(
    _in const char * c_szThreadGroup
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    Qword qwThreadGroupIdentifier = ::Get64BitHashOfNullTerminatedString(c_szThreadGroup, false);
    std::queue<uint64_t> stlTargetThreads;
    
    // First we need to make a list of all the running threads since we cannot
    // loop through m_stlRunningThreads and m_stlThreadGroupings and then call TerminateThread
    // which modifieds m_stlThreadGroupings and m_stlRunningThreads . We would end up with
    // unpredictable results
    m_stlLock.lock();
    for (std::multimap<Qword, uint64_t>::iterator iterator = m_stlThreadGroupings.begin(); iterator != m_stlThreadGroupings.end(); iterator++)
    {
        if (qwThreadGroupIdentifier == iterator->first)
        {
            stlTargetThreads.push(iterator->second);
        }
    }
    m_stlLock.unlock();
    
    // For each item in the queue, call TerminateThread()
    while (!stlTargetThreads.empty())
    {
        this->TerminateThread(stlTargetThreads.front());
        stlTargetThreads.pop();
    }
}
            
/********************************************************************************************/

void __thiscall ThreadManager::TerminateAllThreads(void)
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    std::queue<uint64_t> stlTargetThreads;
    
    // First we need to make a list of all the running threads since we cannot
    // loop through m_stlRunningThreads and m_stlThreadGroupings and then call TerminateThread
    // which modifieds m_stlThreadGroupings and m_stlRunningThreads . We would end up with
    // unpredictable results
    
    m_stlLock.lock();
    {
        for (auto const & element : m_stlRunningThreads)
        {
            stlTargetThreads.push(element.first);
        }
    }
    m_stlLock.unlock();
    
    // For each item in the queue, call TerminateThread()
    while (!stlTargetThreads.empty())
    {
        this->TerminateThread(stlTargetThreads.front());
        stlTargetThreads.pop();
    }
}

/********************************************************************************************/

void __thiscall ThreadManager::JoinThread(
    _in uint64_t un64ThreadIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    pthread_t * psTargetThread = nullptr;
    
    m_stlLock.lock();
    if (m_stlRunningThreads.end() != m_stlRunningThreads.find(un64ThreadIdentifier))
    {
        // Unlike the other methods in this class, we need to retrieve and remove the
        // m_stlRunningThreads entry BEFORE we call pthread_join. This is because we want
        // to make sure we cann pthread_join when not owning the spinlock or else we will
        // easily end up with a deadlock scenario. DO NOT CALL free() on psTargetThread
        // until pthread_join() returns
        
        psTargetThread = m_stlRunningThreads.at(un64ThreadIdentifier);
    }
    m_stlLock.unlock();
    
    // Now that we are outside the spin lock, we can call pthread_join() without causing a
    // deadlock.
    if (nullptr != psTargetThread)
    {
        ::pthread_join(*psTargetThread, nullptr);
        // Do NOT delete the thread information until AFTER pthread_join is called, since doing
        // so deallocates the underlying pthread_t structure
        this->RemoveThreadInformation(un64ThreadIdentifier);
    }    
}

/********************************************************************************************/
void __thiscall ThreadManager::JoinThreadGroup(
    _in const char * c_szThreadGrounp
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    Qword qwThreadGroupIdentifier = ::Get64BitHashOfNullTerminatedString(c_szThreadGrounp, false);
    std::queue<uint64_t> stlTargetThreads;
    
    // First we need to make a list of all the running threads since we cannot
    // loop through m_stlRunningThreads and m_stlThreadGroupings and then call TerminateThread
    // which modifieds m_stlThreadGroupings and m_stlRunningThreads . We would end up with
    // unpredictable results
    m_stlLock.lock();
    for (std::multimap<Qword, uint64_t>::iterator iterator = m_stlThreadGroupings.begin(); iterator != m_stlThreadGroupings.end(); iterator++)
    {
        if (qwThreadGroupIdentifier == iterator->first)
        {
            stlTargetThreads.push(iterator->second);
        }
    }
    m_stlLock.unlock();
    
    // For each item in the queue, call TerminateThread()
    while (!stlTargetThreads.empty())
    {
        this->JoinThread(stlTargetThreads.front());
        stlTargetThreads.pop();
    }
}
            
/********************************************************************************************/      

void __thiscall ThreadManager::JoinAllThreads(void)
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    std::queue<Qword> stlTargetThreads;
    
    // First we need to make a list of all the running threads. This is required since we cannot
    // loop through m_stlRunningThreads and then call JoinThread which modifieds
    // m_stlRunningThreads. We would end up with unpredictable results
    m_stlLock.lock();
    for (auto const & element : m_stlRunningThreads)
    {
        stlTargetThreads.push(element.first);
    }
    m_stlLock.unlock();
    
    // For each item in the queue, call TerminateThread()
    while (!stlTargetThreads.empty())
    {
        this->JoinThread(stlTargetThreads.front());
        stlTargetThreads.pop();
    }
}

/********************************************************************************************/      

void __thiscall ThreadManager::RemoveThreadInformation(
    _in uint64_t un64ThreadIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(this == gs_poThreadManager);
    
    bool fDone = false;
    
    m_stlLock.lock();
    if (m_stlRunningThreads.end() != m_stlRunningThreads.find(un64ThreadIdentifier))
    {
        m_oMemoryAllocator.Deallocate(m_stlRunningThreads.at(un64ThreadIdentifier));
        m_stlRunningThreads.erase(un64ThreadIdentifier);
        // Also make sure to remove the thread entry from the m_stlThreadGroupings
        for (std::multimap<Qword, uint64_t>::iterator iterator = m_stlThreadGroupings.begin(); ((false == fDone)&&(iterator != m_stlThreadGroupings.end())); iterator++)
        {
            if (un64ThreadIdentifier == iterator->second)
            {
                m_stlThreadGroupings.erase(iterator);
                fDone = true;
            }
        }
    }
    m_stlLock.unlock();
}
