/*********************************************************************************************
 *
 * @file GlobalMonitor.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "CoreTypes.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "GlobalMonitor.h"
#include "IpcTransactionHelperFunctions.h"
#include "SocketServer.h"
#include "ThreadManager.h"

#include <sys/ioctl.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <queue>
#include <string>
#include <thread>

// Singleton object for GlobalMonitor. There can be only one!!!
static GlobalMonitor gs_oGlobalMonitor;

/********************************************************************************************/

GlobalMonitor * __stdcall GetGlobalMonitor(void) throw()
{
    __DebugFunction();
    
    return &gs_oGlobalMonitor;
}

/********************************************************************************************/

static void * GlobalMonitorRunThread(
    _in void * pParameter
    ) throw()
{
    __DebugFunction();
    
    try
    {
        gs_oGlobalMonitor.Run();
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    return nullptr;
}

/********************************************************************************************/

static void * GlobalMonitorHandleTransactionThread(
    _in void * pParameter
    ) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != pParameter);
    
    Socket * poSocket = (Socket *) pParameter;
    
    try
    {
        gs_oGlobalMonitor.HandleTransaction(poSocket);
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    // No matter what happens in the transaction, we need to release the socket and then
    // free the RootOfTrustTransactionPacket block (in that specific order)
    
    poSocket->Release();
    
    return nullptr;
}

/********************************************************************************************/

GlobalMonitor::GlobalMonitor(void)
{
    __DebugFunction();
}

/********************************************************************************************/

GlobalMonitor::GlobalMonitor(
    _in const GlobalMonitor & c_oGlobalMonitor
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************/

GlobalMonitor::~GlobalMonitor(void)
{
    __DebugFunction();
}

/********************************************************************************************/

void __thiscall GlobalMonitor::Run(
    _in bool fDisplayStatus
    )
{
    __DebugFunction();
    
    // Figure out if we should be displaying status or not
    m_stlLock.lock();
    m_fDisplayStatus = fDisplayStatus;
    m_stlLock.unlock();
    // Spin up a thread which will process the GlobalMonitor handling
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    __DebugAssert(nullptr != poThreadManager);
    _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("GlobalMonitorListener", GlobalMonitorRunThread, nullptr)), "Failed to start the global monitoring thread", nullptr);
}

/********************************************************************************************/

void __thiscall GlobalMonitor::Run(void)
{
    __DebugFunction();
    
    bool fIsTerminating = false;
    unsigned int unNumberOfSubscribers = 0;
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    SocketServer * poIpcServer = new SocketServer("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);
    
    // Spin around in this thread until we are signalled for termination.
    while ((false == fIsTerminating)||(0 < unNumberOfSubscribers))
    {
        try
        {
            // We wait for a connection
            if (true == poIpcServer->WaitForConnection(1000))
            {
                // Get the socket for the waiting connection
                Socket * poSocket = poIpcServer->Accept();
                if (nullptr != poSocket)
                {
                    poThreadManager->CreateThread("GlobalMonitorHandlers", GlobalMonitorHandleTransactionThread, (void *) poSocket);
                }
            }
            
            // Regarless of whether or not we have processed an incoming transaction, we check
            // to see if the termination signal is on
            m_stlLock.lock();
            // Check to see if we have to clear the screen as we wait for everything to shutdown
            if (m_fIsTerminating != fIsTerminating)
            {
                std::cout <<"\x1B[2J\x1B[0;0f";
            }
            fIsTerminating = m_fIsTerminating;
            unNumberOfSubscribers = m_stlListOfProcessNamesByIdentifierHashes.size();
            __DebugAssert(unNumberOfSubscribers == m_stlListOfDescriptionsByIdentifierHashes.size());
            __DebugAssert(unNumberOfSubscribers == m_stlListOfProcessIdentifiersByIdentifierHashes.size());
            __DebugAssert(unNumberOfSubscribers == m_stlListOfThreadIdentifiersByIdentifierHashes.size());
            __DebugAssert(unNumberOfSubscribers == m_stlListOfThreadTimestampsByIdentifierHashes.size());
            __DebugAssert(unNumberOfSubscribers == m_stlTotalNumberOfEventsByIdentifierHashes.size());
            m_stlLock.unlock();
            
            // If we are NOT terminating, do some processing, like possibly displaying information
            if (false == fIsTerminating)
            {
                // Make sure to display our status if enabled
                this->DisplayStatus();
            }
        }
        
        catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    }

    poThreadManager->JoinThreadGroup("GlobalMonitorHandlers");
}

/********************************************************************************************/

void __thiscall GlobalMonitor::HandleTransaction(
    _in Socket * poSocket
    )
{
    __DebugFunction();
    
    StructuredBuffer oTransactionParameters(::GetIpcTransaction(poSocket));
    std::vector<Byte> stlSerializedResponse;
    
    // By default, ALL incoming transactions must have, at least, a DomainIdentifier
    // as well as a Transaction identifier
    Dword dwTransaction = oTransactionParameters.GetDword("Transaction");
    switch(dwTransaction)
    {
        case 0x10000000 // "RegisterMonitor"
        :   stlSerializedResponse = this->TransactRegisterMonitor(oTransactionParameters);
            break;
        case 0x10000001 // "MonitorHeartbeat"
        :   stlSerializedResponse = this->TransactMonitorHeartbeat(oTransactionParameters);
            break;
        case 0x10000002 // "MonitorStatusUpdate"
        :   stlSerializedResponse = this->TransactMonitorStatusUpdate(oTransactionParameters);
            break;
        case 0x10000003 // "UnregisterMonitor"
        :   stlSerializedResponse = this->TransactUnregisterMonitor(oTransactionParameters);
            break;
        case 0x10000004 // "SignalTermination"
        :   stlSerializedResponse = this->TransactSignalTermination(oTransactionParameters);
            break;
    }
    // Send out the response to the transaction if there is a response available only
    if ((0 < stlSerializedResponse.size())&&(false == ::PutIpcTransaction(poSocket, stlSerializedResponse)))
    {
        //TODO: do some logging into the future
    }
}
            
/********************************************************************************************/

void __thiscall GlobalMonitor::DisplayStatus(void) throw()
{
    __DebugFunction();
    
    if (true == m_fDisplayStatus)
    {
        // First we need to figure out what the console window dimensions are
        struct winsize sWindowSize;
        ::ioctl(fileno(stdout), TIOCGWINSZ, &sWindowSize);
        // Now we need to create a queue of elements to process
        std::queue<std::string> stlQueueOfMonitoredThreads;
        std::string strSeparator(sWindowSize.ws_col, '-');
        std::string strBlankLine(sWindowSize.ws_col, ' ');
        //stlQueueOfMonitoredThreads.push(strSeparator);
        m_stlLock.lock();
        for (auto const & monitoredThread: m_stlListOfProcessNamesByIdentifierHashes)
        {
            // Quick reality check using some asserts
            __DebugAssert(m_stlListOfDescriptionsByIdentifierHashes.end() != m_stlListOfDescriptionsByIdentifierHashes.find(monitoredThread.first));
            __DebugAssert(m_stlListOfProcessNamesByIdentifierHashes.end() != m_stlListOfProcessNamesByIdentifierHashes.find(monitoredThread.first));
            __DebugAssert(m_stlListOfProcessIdentifiersByIdentifierHashes.end() != m_stlListOfProcessIdentifiersByIdentifierHashes.find(monitoredThread.first));
            __DebugAssert(m_stlListOfThreadIdentifiersByIdentifierHashes.end() != m_stlListOfThreadIdentifiersByIdentifierHashes.find(monitoredThread.first));
            __DebugAssert(m_stlListOfThreadTimestampsByIdentifierHashes.end() != m_stlListOfThreadTimestampsByIdentifierHashes.find(monitoredThread.first));
            __DebugAssert(m_stlTotalNumberOfEventsByIdentifierHashes.end() != m_stlTotalNumberOfEventsByIdentifierHashes.find(monitoredThread.first));
            // Format the string to print out
            
            std::string strElement((int) sWindowSize.ws_col, ' ');
            std::snprintf(strElement.data(), strElement.size(), "[0x%08X%08X] %s (Pid=0x%08X, Tid=0x%08X, Time=%lu, Event Count=%d)", HIDWORD(monitoredThread.first), LODWORD(monitoredThread.first), m_stlListOfDescriptionsByIdentifierHashes[monitoredThread.first].c_str(), m_stlListOfProcessIdentifiersByIdentifierHashes[monitoredThread.first], m_stlListOfThreadIdentifiersByIdentifierHashes[monitoredThread.first], m_stlListOfThreadTimestampsByIdentifierHashes[monitoredThread.first], m_stlTotalNumberOfEventsByIdentifierHashes[monitoredThread.first]);
            strElement.resize(sWindowSize.ws_col, ' ');
            stlQueueOfMonitoredThreads.push(strElement);
        }
        m_stlLock.unlock();
        //stlQueueOfMonitoredThreads.push(strSeparator);
        // Now that we know how many elements to print, we can start printing them
        for (unsigned int unIndex = 1; unIndex < sWindowSize.ws_row; ++unIndex)
        {
            if (!stlQueueOfMonitoredThreads.empty())
            {
                //std::cout << stlQueueOfMonitoredThreads.front() << std::endl;
                ::printf("\33[%d;%dH%s", unIndex, 0, stlQueueOfMonitoredThreads.front().c_str());
                stlQueueOfMonitoredThreads.pop();
            }
            else
            {
                ::printf("\33[%d;%dH%s", unIndex , 0, strBlankLine.c_str());
            }
        }        
        
        m_stlLock.unlock();
    }
}

/********************************************************************************************/

std::vector<Byte> __thiscall GlobalMonitor::TransactRegisterMonitor(
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    Guid oMonitoringIdentifier = c_oTransactionParameters.GetGuid("MonitoringIdentifier");
    std::string strProcessFilename = c_oTransactionParameters.GetString("ProcessFilename");
    std::string strDescription = c_oTransactionParameters.GetString("Description");
    int nProcessIdentifier = (int) c_oTransactionParameters.GetInt32("ProcessIdentifier");
    int nThreadIdentifier = (int) c_oTransactionParameters.GetInt32("ThreadIdentifier");
    
    Qword qwHashOfMonitoringIdentifier = ::Get64BitHashOfByteArray((const Byte *) oMonitoringIdentifier.GetRawDataPtr(), 16);
    
    m_stlLock.lock();
    m_stlListOfProcessNamesByIdentifierHashes.insert(std::pair<Qword, std::string>(qwHashOfMonitoringIdentifier, strProcessFilename));
    m_stlListOfDescriptionsByIdentifierHashes.insert(std::pair<Qword, std::string>(qwHashOfMonitoringIdentifier, strDescription));
    m_stlListOfProcessIdentifiersByIdentifierHashes.insert(std::pair<Qword, int>(qwHashOfMonitoringIdentifier, nProcessIdentifier));
    m_stlListOfThreadIdentifiersByIdentifierHashes.insert(std::pair<Qword, int>(qwHashOfMonitoringIdentifier, nThreadIdentifier));
    m_stlListOfThreadTimestampsByIdentifierHashes.insert(std::pair<Qword, uint64_t>(qwHashOfMonitoringIdentifier, ::GetEpochTimeInSeconds()));
    m_stlTotalNumberOfEventsByIdentifierHashes.insert(std::pair<Qword, unsigned int>(qwHashOfMonitoringIdentifier, 0));
    m_stlLock.unlock();
    
    StructuredBuffer oResponse;
    oResponse.PutBoolean("Success", true);
    
    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall GlobalMonitor::TransactMonitorHeartbeat(
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    Guid oMonitoringIdentifier = c_oTransactionParameters.GetGuid("MonitoringIdentifier");
    Qword qwHashOfMonitoringIdentifier = ::Get64BitHashOfByteArray((const Byte *) oMonitoringIdentifier.GetRawDataPtr(), 16);
    
    m_stlLock.lock();
    if (m_stlListOfProcessNamesByIdentifierHashes.end() != m_stlListOfProcessNamesByIdentifierHashes.find(qwHashOfMonitoringIdentifier))
    {
        // Quick reality check using some asserts
        __DebugAssert(m_stlListOfDescriptionsByIdentifierHashes.end() != m_stlListOfDescriptionsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfProcessIdentifiersByIdentifierHashes.end() != m_stlListOfProcessIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadIdentifiersByIdentifierHashes.end() != m_stlListOfThreadIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadTimestampsByIdentifierHashes.end() != m_stlListOfThreadTimestampsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlTotalNumberOfEventsByIdentifierHashes.end() != m_stlTotalNumberOfEventsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        // Insert the new timestamp
        m_stlListOfThreadTimestampsByIdentifierHashes[qwHashOfMonitoringIdentifier] = ::GetEpochTimeInSeconds();
        // Prepare the response
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBoolean("TerminationSignal", m_fIsTerminating);
    }
    else
    {
        // Prepare the response
        oResponseBuffer.PutBoolean("Success", false);
    }
    m_stlLock.unlock();
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall GlobalMonitor::TransactMonitorStatusUpdate(
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    Guid oMonitoringIdentifier = c_oTransactionParameters.GetGuid("MonitoringIdentifier");
    std::string strStatus = c_oTransactionParameters.GetString("Status");
    
    Qword qwHashOfMonitoringIdentifier = ::Get64BitHashOfByteArray((const Byte *) oMonitoringIdentifier.GetRawDataPtr(), 16);
    
    m_stlLock.lock();
    if (m_stlListOfProcessNamesByIdentifierHashes.end() != m_stlListOfProcessNamesByIdentifierHashes.find(qwHashOfMonitoringIdentifier))
    {
        // Quick reality check using some asserts
        __DebugAssert(m_stlListOfDescriptionsByIdentifierHashes.end() != m_stlListOfDescriptionsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfProcessIdentifiersByIdentifierHashes.end() != m_stlListOfProcessIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadIdentifiersByIdentifierHashes.end() != m_stlListOfThreadIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadTimestampsByIdentifierHashes.end() != m_stlListOfThreadTimestampsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlTotalNumberOfEventsByIdentifierHashes.end() != m_stlTotalNumberOfEventsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        // Insert the new timestamp and status
        m_stlListOfThreadTimestampsByIdentifierHashes[qwHashOfMonitoringIdentifier] = ::GetEpochTimeInSeconds();
        m_stlTotalNumberOfEventsByIdentifierHashes[qwHashOfMonitoringIdentifier]++;
    }
    m_stlLock.unlock();
    
    // Prepare the response
    StructuredBuffer oResponseBuffer;
    oResponseBuffer.PutBoolean("Success", true);
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall GlobalMonitor::TransactUnregisterMonitor(
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    Guid oMonitoringIdentifier = c_oTransactionParameters.GetGuid("MonitoringIdentifier");
    Qword qwHashOfMonitoringIdentifier = ::Get64BitHashOfByteArray((const Byte *) oMonitoringIdentifier.GetRawDataPtr(), 16);
    
    m_stlLock.lock();
    if (m_stlListOfProcessNamesByIdentifierHashes.end() != m_stlListOfProcessNamesByIdentifierHashes.find(qwHashOfMonitoringIdentifier))
    {
        // Quick reality check using some asserts
        __DebugAssert(m_stlListOfDescriptionsByIdentifierHashes.end() != m_stlListOfDescriptionsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfProcessIdentifiersByIdentifierHashes.end() != m_stlListOfProcessIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadIdentifiersByIdentifierHashes.end() != m_stlListOfThreadIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadTimestampsByIdentifierHashes.end() != m_stlListOfThreadTimestampsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlTotalNumberOfEventsByIdentifierHashes.end() != m_stlTotalNumberOfEventsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        // Now delete all of the entries
        m_stlListOfProcessNamesByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
        m_stlListOfDescriptionsByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
        m_stlListOfProcessIdentifiersByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
        m_stlListOfThreadIdentifiersByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
        m_stlListOfThreadTimestampsByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
        m_stlTotalNumberOfEventsByIdentifierHashes.erase(qwHashOfMonitoringIdentifier);
    }
    m_stlLock.unlock();
    
    StructuredBuffer oResponse;
    oResponse.PutBoolean("Success", true);
    
    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall GlobalMonitor::TransactSignalTermination(
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    Guid oMonitoringIdentifier = c_oTransactionParameters.GetGuid("MonitoringIdentifier");
    std::string strReason = c_oTransactionParameters.GetString("Reason");
    
    Qword qwHashOfMonitoringIdentifier = ::Get64BitHashOfByteArray((const Byte *) oMonitoringIdentifier.GetRawDataPtr(), 16);
    
    m_stlLock.lock();
    if (m_stlListOfProcessNamesByIdentifierHashes.end() != m_stlListOfProcessNamesByIdentifierHashes.find(qwHashOfMonitoringIdentifier))
    {
        // Quick reality check using some asserts
        __DebugAssert(m_stlListOfDescriptionsByIdentifierHashes.end() != m_stlListOfDescriptionsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfProcessIdentifiersByIdentifierHashes.end() != m_stlListOfProcessIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadIdentifiersByIdentifierHashes.end() != m_stlListOfThreadIdentifiersByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlListOfThreadTimestampsByIdentifierHashes.end() != m_stlListOfThreadTimestampsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        __DebugAssert(m_stlTotalNumberOfEventsByIdentifierHashes.end() != m_stlTotalNumberOfEventsByIdentifierHashes.find(qwHashOfMonitoringIdentifier));
        // Now signal termination
        m_fIsTerminating = true;
        m_strTerminationReason = strReason;
    }
    m_stlLock.unlock();
    
    StructuredBuffer oResponse;
    oResponse.PutBoolean("Success", true);
    
    return oResponse.GetSerializedBuffer();
}