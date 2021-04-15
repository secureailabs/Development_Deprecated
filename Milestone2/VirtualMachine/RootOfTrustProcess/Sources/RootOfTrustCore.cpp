/*********************************************************************************************
 *
 * @file RootOfTrustCore.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "ApiCallHelpers.h"
#include "Base64Encoder.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "HardCodedCryptographicKeys.h"
#include "IpcTransactionHelperFunctions.h"
#include "RootOfTrustCore.h"
#include "SmartMemoryAllocator.h"
#include "SocketServer.h"
#include "StatusMonitor.h"
#include "ThreadManager.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

// Allocated structure that is used to carry TWO parameters to the RootOfTrustTransactionHandlerThread
// function

typedef struct
{
    RootOfTrustCore * poRootOfTrustCore;
    Socket * poSocket;
}
RootOfTrustTransactionPacket;

// Global smart memory allocator that is used to allocate and deallocate RootOfTrustTransactionPacket
// memory blocks
static SmartMemoryAllocator gs_oMemoryAllocator;

/********************************************************************************************/


static void * RootOfTrustIpcListenerThread(
    _in void * pParameter
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != pParameter);
    
    try
    {
        ((RootOfTrustCore *) pParameter)->RunIpcListener();
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
    }
    
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
    }
    
    return nullptr;
}

/********************************************************************************************/

static void * RootOfTrustTransactionHandlerThread(
    _in void * pParameter
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != pParameter);
    
    RootOfTrustTransactionPacket * poRootOfTrustTransactionPacket = (RootOfTrustTransactionPacket *) pParameter;
    __DebugAssert(nullptr != poRootOfTrustTransactionPacket->poSocket);
    __DebugAssert(nullptr != poRootOfTrustTransactionPacket->poRootOfTrustCore);
    
    try
    {
        poRootOfTrustTransactionPacket->poRootOfTrustCore->HandleIncomingTransaction(poRootOfTrustTransactionPacket->poSocket);
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    // No matter what happens in the transaction, we need to release the socket and then
    // free the RootOfTrustTransactionPacket block (in that specific order)
    
    poRootOfTrustTransactionPacket->poSocket->Release();
    gs_oMemoryAllocator.Deallocate(pParameter);
    
    return nullptr;
}

/********************************************************************************************/

static void * RootOfTrustAuditEventDispatcherThread(
    _in void * pParameter
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != pParameter);
    
    std::cout << __func__ << std::endl;
    
    try
    {
        ((RootOfTrustCore *) pParameter)->AuditEventDispatcher();
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    return nullptr;
}

/********************************************************************************************/

RootOfTrustCore::RootOfTrustCore(
    _in const std::vector<Byte> & c_stlSerializedInitializationParameters
    )
    : m_fIsInitialized(false), m_fIsRunning(false)
{
    __DebugFunction();
    
    StructuredBuffer oInitializationData(c_stlSerializedInitializationParameters);
    
    m_strNameOfVirtualMachine = oInitializationData.GetString("NameOfVirtualMachine");
    m_strIpAddressOfVirtualMachine = oInitializationData.GetString("IpAddressOfVirtualMachine");
    m_strVirtualMachineIdentifier = oInitializationData.GetString("VirtualMachineIdentifier");
    m_strClusterIdentifier = oInitializationData.GetString("ClusterIdentifier");
    m_strDigitalContractIdentifier = oInitializationData.GetString("DigitalContractIdentifier");
    m_strDatasetIdentifier = oInitializationData.GetString("DatasetIdentifier");
    m_strRootOfTrustDomainIdentifier = oInitializationData.GetString("RootOfTrustDomainIdentifier");
    m_strComputationalDomainIdentifier = oInitializationData.GetString("ComputationalDomainIdentifier");
    m_strDataConnectorDomainIdentifier = oInitializationData.GetString("DataConnectorDomainIdentifier");
    m_strSailWebApiPortalIpAddress = oInitializationData.GetString("SailWebApiPortalIpAddress");
    m_strDataOwnerAccessToken = oInitializationData.GetString("DataOwnerAccessToken");
    m_strDataOwnerOrganizationIdentifier = oInitializationData.GetString("DataOwnerOrganizationIdentifier");
    m_strDataOwnerUserIdentifier = oInitializationData.GetString("DataOwnerUserIdentifier");
    std::string strBase64EncodedSerializedDataset = oInitializationData.GetString("Base64EncodedDataset");    
    m_stlDataset = ::Base64Decode(strBase64EncodedSerializedDataset.c_str());
    
    m_strRootOfTrustIpcPath = Guid().ToString(eRaw);
    m_strComputationalDomainIpcPath = Guid().ToString(eRaw);
    m_strDataDomainIpcPath = Guid().ToString(eRaw);

    m_fIsInitialized = true;
    
    ::SetIpAddressOfSailWebApiPortalGateway(m_strSailWebApiPortalIpAddress, 6200);
    
    this->InitializeVirtualMachine();
    this->RegisterDataOwnerEosb();
}

/********************************************************************************************/

RootOfTrustCore::RootOfTrustCore(
    _in const RootOfTrustCore & c_oRootOfTrust
    )
{
    __DebugFunction();
    
    if (true == c_oRootOfTrust.m_fIsInitialized)
    {
        m_strDataOwnerAccessToken = c_oRootOfTrust.m_strDataOwnerAccessToken;;
        m_strResearcherEosb = c_oRootOfTrust.m_strResearcherEosb;
        
        m_strDataOrganizationAuditEventParentBranchNodeIdentifier = c_oRootOfTrust.m_strDataOrganizationAuditEventParentBranchNodeIdentifier;
        m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier = c_oRootOfTrust.m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
        
        m_strNameOfVirtualMachine = c_oRootOfTrust.m_strNameOfVirtualMachine;
        m_strIpAddressOfVirtualMachine = c_oRootOfTrust.m_strIpAddressOfVirtualMachine;
        m_strVirtualMachineIdentifier = c_oRootOfTrust.m_strVirtualMachineIdentifier;
        m_strClusterIdentifier = c_oRootOfTrust.m_strClusterIdentifier;
        m_strDigitalContractIdentifier = c_oRootOfTrust.m_strDigitalContractIdentifier;
        m_strDatasetIdentifier = c_oRootOfTrust.m_strDatasetIdentifier;
        m_strRootOfTrustDomainIdentifier = c_oRootOfTrust.m_strRootOfTrustDomainIdentifier;
        m_strComputationalDomainIdentifier = c_oRootOfTrust.m_strComputationalDomainIdentifier;
        m_strDataConnectorDomainIdentifier = c_oRootOfTrust.m_strDataConnectorDomainIdentifier;
        m_strSailWebApiPortalIpAddress = c_oRootOfTrust.m_strSailWebApiPortalIpAddress;
        m_strDataOwnerOrganizationIdentifier = c_oRootOfTrust.m_strDataOwnerOrganizationIdentifier;
        m_strDataOwnerUserIdentifier = c_oRootOfTrust.m_strDataOwnerUserIdentifier;
        m_stlDataset = c_oRootOfTrust.m_stlDataset;    
        
        m_strRootOfTrustIpcPath = c_oRootOfTrust.m_strRootOfTrustIpcPath;
        m_strComputationalDomainIpcPath = c_oRootOfTrust.m_strComputationalDomainIpcPath;
        m_strDataDomainIpcPath = c_oRootOfTrust.m_strDataDomainIpcPath;

        m_fIsInitialized = true;
        // By default, this new instance of RootOfTrustCore is NOT running it's own thread
        m_fIsRunning = false;
    }
}        
        
/********************************************************************************************/

RootOfTrustCore::~RootOfTrustCore(void)
{
    __DebugFunction();
    
    // TODO: Securely zeroize buffers
}

/********************************************************************************************/

Guid __thiscall RootOfTrustCore::GetDataDomainIdentifier(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return Guid(m_strDataConnectorDomainIdentifier.c_str());
}

/********************************************************************************************/

std::string __thiscall RootOfTrustCore::GetRootOfTrustIpcPath(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return m_strRootOfTrustIpcPath;
}

/********************************************************************************************/

void __thiscall RootOfTrustCore::AuditEventDispatcher(void)
{
    __DebugFunction();
    
    std::cout << __func__ << std::endl;
    
    bool fIsShutdown = false;
    bool fIsShuttingDown = false;
    StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::AuditEventDispatcher(void)");
    while (false == fIsShutdown)
    {
        try
        {
            const std::lock_guard<std::mutex> lock(m_stlAuditEventsMutex);
            // We use a intermediate StructuredBuffer to store all of the new audit events
            bool fTransmitAuditEvents = false;
            
            // Handle lingering events within the data organization audit event queue
            if ((0 < m_strDataOwnerAccessToken.size())&&(0 < m_strDataOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < m_stlDataOrganizationAuditEventQueue.size())
                {
                    fTransmitAuditEvents = true;
                    StructuredBuffer oNewAuditEvent(m_stlDataOrganizationAuditEventQueue.front().c_str());
                    m_stlDataOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                    std::cout << "Transmitting an audit event (DOO)" << std::endl;
                }
                
                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < m_strDataOwnerAccessToken.size());
                    __DebugAssert(0 < m_strDataOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(m_strDataOwnerAccessToken, m_strDataOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
            
            if ((0 < m_strResearcherEosb.size())&&(0 < m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < m_stlResearchOrganizationAuditEventQueue.size())
                {
                    fTransmitAuditEvents = true;
                    StructuredBuffer oNewAuditEvent(m_stlResearchOrganizationAuditEventQueue.front().c_str());
                    m_stlResearchOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                    std::cout << "Transmitting an audit event (RO)" << std::endl;
                }

                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < m_strResearcherEosb.size());
                    __DebugAssert(0 < m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(m_strResearcherEosb, m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
        }
        
        catch (BaseException oException)
        {
            ::RegisterException(oException, __func__, __LINE__);
        }
        
        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
        }
        
        // This must appear before the check to oStatusMonitor.IsTerminating()
        if (true == fIsShuttingDown)
        {
            fIsShutdown = true;
        }
        
        // Check to see if we are terminating
        if ((false == fIsShuttingDown)&&(true == oStatusMonitor.IsTerminating()))
        {
            fIsShuttingDown = true;
            StructuredBuffer oEventData;
            oEventData.PutString("IpAddressOfSecureVirtualMachine", m_strIpAddressOfVirtualMachine);
            oEventData.PutString("VirtualMachineIdentifier", m_strClusterIdentifier);
            oEventData.PutString("ClusterIdentifier", m_strClusterIdentifier);
            this->RecordInternalAuditEvent("VM_SHUTDOWN", 0x1111, 0x05, oEventData);
        }
        
        // Put this thread to sleep for 5 seconds
        ::sleep(5);
    }
    
    // On the way out, the AuditEventDispatcher will send out one last event, the VM_SHUTDOWN event
    
}

/********************************************************************************************/

void __thiscall RootOfTrustCore::RunIpcListener(void)
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    std::cout << __func__ << std::endl;
    
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    if (false == m_fIsRunning)
    {
        m_fIsRunning = true;
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", RootOfTrustAuditEventDispatcherThread, (void *) this)), "Failed to start the audit event dispacher thread", nullptr);
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", RootOfTrustIpcListenerThread, (void *) this)), "Failed to start the Root of Trust Listener Thread", nullptr);
    }
    else
    {
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RunIpcListener(void)");
        SocketServer * poIpcServer = new SocketServer(m_strRootOfTrustIpcPath.c_str());
        _ThrowOutOfMemoryExceptionIfNull(poIpcServer);
        unsigned int unNumberOfLoops = 0;
        unsigned int unNumberOfSuccessfulTransactions = 0;
        unsigned int unNumberOfFailedTransactions = 0;
        while (false == oStatusMonitor.IsTerminating())
        {
            std::string strDataOwnerAccessToken = m_strDataOwnerAccessToken;;
            std::string strResearcherEosb = m_strResearcherEosb;
            std::string strDataOrganizationAuditEventParentBranchNodeIdentifier = m_strDataOrganizationAuditEventParentBranchNodeIdentifier;
            std::string strResearcherOrganizationAuditEventParentBranchNodeIdentifier = m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
            std::string strNameOfVirtualMachine = m_strNameOfVirtualMachine;
            std::string strIpAddressOfVirtualMachine = m_strIpAddressOfVirtualMachine;
            std::string strVirtualMachineIdentifier = m_strVirtualMachineIdentifier;
            std::string strClusterIdentifier = m_strClusterIdentifier;
            std::string strDigitalContractIdentifier = m_strDigitalContractIdentifier;
            std::string strDatasetIdentifier = m_strDatasetIdentifier;
            std::string strRootOfTrustDomainIdentifier = m_strRootOfTrustDomainIdentifier;
            std::string strComputationalDomainIdentifier = m_strComputationalDomainIdentifier;
            std::string strDataConnectorDomainIdentifier = m_strDataConnectorDomainIdentifier;
            std::string strSailWebApiPortalIpAddress = m_strSailWebApiPortalIpAddress;
            std::string strDataOwnerOrganizationIdentifier = m_strDataOwnerOrganizationIdentifier;
            std::string strDataOwnerUserIdentifier = m_strDataOwnerUserIdentifier;
            std::string strRootOfTrustIpcPath = m_strRootOfTrustIpcPath;
            std::string strComputationalDomainIpcPath = m_strComputationalDomainIpcPath;
            std::string strDataDomainIpcPath = m_strDataDomainIpcPath;
            bool fIsInitialized = m_fIsInitialized;
            bool fIsRunning = m_fIsRunning;
            // We wait for a connection
            if (true == poIpcServer->WaitForConnection(1000))
            {
                // BUGBUG: We are cheating here because there is a memory corruption issue when WaitForConnection()
                // get calls, the m_strComputationalDomainIdentifier gets corrupted.
                m_strDataOwnerAccessToken = strDataOwnerAccessToken;;
                m_strResearcherEosb = strResearcherEosb;
                m_strDataOrganizationAuditEventParentBranchNodeIdentifier = strDataOrganizationAuditEventParentBranchNodeIdentifier;
                m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier = strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
                m_strNameOfVirtualMachine = strNameOfVirtualMachine;
                m_strIpAddressOfVirtualMachine = strIpAddressOfVirtualMachine;
                m_strVirtualMachineIdentifier = strVirtualMachineIdentifier;
                m_strClusterIdentifier = strClusterIdentifier;
                m_strDigitalContractIdentifier = strDigitalContractIdentifier;
                m_strDatasetIdentifier = strDatasetIdentifier;
                m_strRootOfTrustDomainIdentifier = strRootOfTrustDomainIdentifier;
                m_strComputationalDomainIdentifier = strComputationalDomainIdentifier;
                m_strDataConnectorDomainIdentifier = strDataConnectorDomainIdentifier;
                m_strSailWebApiPortalIpAddress = strSailWebApiPortalIpAddress;
                m_strDataOwnerOrganizationIdentifier = strDataOwnerOrganizationIdentifier;
                m_strDataOwnerUserIdentifier = strDataOwnerUserIdentifier;
                m_strRootOfTrustIpcPath = strRootOfTrustIpcPath;
                m_strComputationalDomainIpcPath = strComputationalDomainIpcPath;
                m_strDataDomainIpcPath = strDataDomainIpcPath;
                m_fIsInitialized = fIsInitialized;
                m_fIsRunning = fIsRunning;
                // Get the socket for the waiting connection
                Socket * poSocket = poIpcServer->Accept();
                if (nullptr != poSocket)
                {
                    // Allocate the memory block for RootOfTrustTransactionPacket in preparation
                    // of spawning a new thread
                    RootOfTrustTransactionPacket * poRootOfTrustTransactionPacket = (RootOfTrustTransactionPacket *) gs_oMemoryAllocator.Allocate(sizeof(RootOfTrustTransactionPacket), true);
                    _ThrowOutOfMemoryExceptionIfNull(poIpcServer);
                    // Initialize the RootOfTrustTransactionPacket structure
                    poRootOfTrustTransactionPacket->poRootOfTrustCore = this;
                    poRootOfTrustTransactionPacket->poSocket = poSocket;
                    // If we fail to create the thread, then we fail to process the transaction
                    // and this is impossibly BAD. We need to throw an exception. This will
                    // terminate the RunIpcListener thread and exit.
                    if (0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", RootOfTrustTransactionHandlerThread, (void *) poRootOfTrustTransactionPacket))
                    {
                        unNumberOfFailedTransactions++;
                        // TODO: In the future, return an error packet to the caller.
                    }
                    else
                    {
                        unNumberOfSuccessfulTransactions++;
                    }
                }
            }
            unNumberOfLoops++;
            //oStatusMonitor.UpdateStatus("unNumberOfLoops = %d, unNumberOfSuccessfulTransactions = %d, unNumberOfFailedTransactions = %d", unNumberOfLoops, unNumberOfSuccessfulTransactions, unNumberOfFailedTransactions);
        }
        // Close out the IPC server for the RootOfTrust
        poIpcServer->Release();
        // Wait for all of the slave threads to terminate before exiting the dedicated Ipc
        // listener thread
        poThreadManager->JoinThreadGroup("RootOfTrustCodeGroup");
        m_fIsRunning = false;
    }
}
/********************************************************************************************/

void __thiscall RootOfTrustCore::HandleIncomingTransaction(
    _in Socket * poSocket
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    std::cout << __func__ << std::endl;
    
    StructuredBuffer oTransactionParameters(::GetIpcTransaction(poSocket));
    std::vector<Byte> stlSerializedResponse;

    // By default, ALL incoming transactions must have, at least, a DomainIdentifier
    // as well as a Transaction identifier
    Guid oOriginatingDomainIdentifier = oTransactionParameters.GetGuid("DomainIdentifier");
    Dword dwTransaction = oTransactionParameters.GetDword("Transaction");
    switch(dwTransaction)
    {
        case 0x00000006 //  "GetDataSet"
        :   stlSerializedResponse = this->TransactGetDataSet(oOriginatingDomainIdentifier);
            break;
        case 0x00000009 // "RecordAuditEvent"
        :   stlSerializedResponse = this->TransactRecordAuditEvent(oOriginatingDomainIdentifier, oTransactionParameters);
            break;
        
    }
    
    // Send out the response to the transaction if there is a response available only
    if ((0 < stlSerializedResponse.size())&&(false == ::PutIpcTransaction(poSocket, stlSerializedResponse)))
    {
        //TODO: do some logging into the future
    }
}
            
/********************************************************************************************/

void __thiscall RootOfTrustCore::WaitForTermination(void) throw()
{
    __DebugFunction();
    // Wait for ALL threads started by the RootOfTrustCore object
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinThreadGroup("RootOfTrustCodeListener");
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetDataSet(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    try
    {
        if (c_oOriginatingDomainIdentifier == Guid(m_strDataConnectorDomainIdentifier.c_str()))
        {
            oResponseBuffer.PutBoolean("Success", true);
            oResponseBuffer.PutBuffer("Dataset", m_stlDataset);
        }
        else
        {
            oResponseBuffer.PutBoolean("Success", false);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponseBuffer.PutBoolean("Success", false);
    }
        
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseBuffer.PutBoolean("Success", false);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactRecordAuditEvent(
    _in const Guid & c_oOriginatingDomainIdentifier,
    _in const StructuredBuffer & c_oTransactionParameters
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    try
    {
        std::cout << __func__ << std::endl;
        static unsigned int s_unResearcherAuditEventSequenceNumber = 0;
        static unsigned int s_unDataOwnerAuditEventSequenceNumber = 0;
        static unsigned int s_unThirdPartyAuditorAuditEventSequenceNumber = 0;
        static unsigned int s_unSailAuditEventSequenceNumber = 0;
        StructuredBuffer oCopyOfTransactionParameters(c_oTransactionParameters);
        // Make a copy of the target channels
        Word wTargetChannelsBitMask = oCopyOfTransactionParameters.GetWord("TargetChannelsBitMask");
        // First we make sure to shave off elements that are within the oCopyOfTransactionParameters
        // but should not actually be persisted within the audit event
        oCopyOfTransactionParameters.RemoveElement("Transaction");
        oCopyOfTransactionParameters.RemoveElement("DomainIdentifier");
        oCopyOfTransactionParameters.RemoveElement("TargetChannelsBitMask");
        // There is ONE special case where we need to detect CONNECT_SUCCESS so that we
        // can register the EOSB of the researcher
        if (c_oOriginatingDomainIdentifier == m_strComputationalDomainIdentifier.c_str())
        {
            StructuredBuffer oEncryptedData(oCopyOfTransactionParameters.GetString("EncryptedEventData").c_str());
            std::string eventName = oEncryptedData.GetString("EventName");
            if (eventName == "CONNECT_SUCCESS")
            {
                m_strResearcherEosb = oEncryptedData.GetString("EOSB");
                // Make sure that element is removed from the audit record
                oEncryptedData.RemoveElement("EOSB");
                // Update the EventData
                oCopyOfTransactionParameters.PutString("EncryptedEventData", oEncryptedData.GetBase64SerializedBuffer());
                this->RegisterResearcherEosb();
            }
        }
        
        const std::lock_guard<std::mutex> lock(m_stlAuditEventsMutex);
        if (0x1000 == (0x1000 & wTargetChannelsBitMask)) // Third Party Auditor Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unThirdPartyAuditorAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            m_stlIndependentAuditorOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0100 == (0x0100 & wTargetChannelsBitMask)) // Data Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unDataOwnerAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            m_stlDataOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0010 == (0x0010 & wTargetChannelsBitMask)) // Research Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unResearcherAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            m_stlResearchOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0001 == (0x0001 & wTargetChannelsBitMask)) // Sail Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unSailAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            m_stlSailOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponseBuffer.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponseBuffer.PutBoolean("Success", false);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

bool __thiscall RootOfTrustCore::InitializeVirtualMachine(void)
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        std::cout << __func__ << std::endl;
        // First we generate an audit event which marks the start of the virtual machine
        StructuredBuffer oEventData;
        oEventData.PutString("PythonIntepreterVersion", "v3.8");
        oEventData.PutString("SecureVirtualMachineVersion", "0.1.37");
        oEventData.PutString("IpAddressOfSecureVirtualMachine", m_strIpAddressOfVirtualMachine);
        oEventData.PutString("VirtualMachineIdentifier", m_strClusterIdentifier);
        oEventData.PutString("ClusterIdentifier", m_strClusterIdentifier);
        this->RecordInternalAuditEvent("VM_INITIALIZE", 0x1111, 0x05, oEventData);
        {
            // Make sure we are thread safe
            const std::lock_guard<std::mutex> lock(m_stlAuditEventsMutex);
            // Make sure all of the parameters are proper
            if ((0 < m_strDataOwnerAccessToken.size())&&(0 < m_strVirtualMachineIdentifier.size())&&(0 < m_strDigitalContractIdentifier.size())&&(0 < m_strIpAddressOfVirtualMachine.size()))
            {
                m_strVirtualMachineEosb = ::RegisterVirtualMachineWithSailWebApiPortal(m_strDataOwnerAccessToken, m_strVirtualMachineIdentifier, m_strDigitalContractIdentifier, m_strIpAddressOfVirtualMachine);
                fSuccess = true;
            }
        }
        // Now, we generate an event to recognize the digital contract initialization
        oEventData.Clear();
        oEventData.PutString("DigitalContractIdentifier", m_strDigitalContractIdentifier);
        oEventData.PutString("VirtualMachineIdentifier", m_strClusterIdentifier);
        oEventData.PutString("ClusterIdentifier", m_strClusterIdentifier);
        this->RecordInternalAuditEvent("DC_INITIALIZE", 0x1111, 0x05, oEventData);
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool __thiscall RootOfTrustCore::RegisterDataOwnerEosb(void)
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    try
    {
        std::cout << __func__ << std::endl;
        // Make sure we are thread safe
        const std::lock_guard<std::mutex> lock(m_stlAuditEventsMutex);
        // Make sure all of the parameters are proper
        if ((0 < m_strDataOwnerAccessToken.size())&&(0 < m_strVirtualMachineIdentifier.size()))
        {
            m_strDataOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineDataOwner(m_strDataOwnerAccessToken, m_strVirtualMachineIdentifier);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool __thiscall RootOfTrustCore::RegisterResearcherEosb(void)
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    try
    {
        std::cout << __func__ << std::endl;
        // Make sure we are thread safe
        const std::lock_guard<std::mutex> lock(m_stlAuditEventsMutex);
        // Make sure all of the parameters are proper
        if ((0 < m_strResearcherEosb.size())&&(0 < m_strVirtualMachineIdentifier.size()))
        {
            m_strResearcherOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineResearcher(m_strResearcherEosb, m_strVirtualMachineIdentifier);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

void __thiscall RootOfTrustCore::RecordInternalAuditEvent(
    _in const char * c_szEventName,
    _in Word wTargetChannelsBitMask,
    _in Dword dwEventType,
    _in const StructuredBuffer & c_oEventData
    )
{
    __DebugFunction();
    
    try
    {   
        std::cout << __func__ << std::endl;
        // Construct the transaction packet
        Guid oRootOfTrustDomainIdentifier(m_strRootOfTrustDomainIdentifier.c_str());
        Guid oEventGuid;
        StructuredBuffer oTransactionData;
        StructuredBuffer oEncryptedEventData = c_oEventData;
        // Internal elements only. These will be deleted before to transmitting the audit event
        oTransactionData.PutGuid("DomainIdentifier", oRootOfTrustDomainIdentifier);
        oTransactionData.PutDword("Transaction", 0x00000009);
        oTransactionData.PutWord("TargetChannelsBitMask", wTargetChannelsBitMask);
        // Persistent properties of audit event
        oTransactionData.PutString("EventGuid", oEventGuid.ToString(eHyphensAndCurlyBraces));
        oTransactionData.PutQword("EventType", dwEventType);
        oTransactionData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
        // Make sure that the encrypted data contains the EventName property
        oEncryptedEventData.PutString("EventName", c_szEventName);
        // Add the encrypted event data to the audit event
        oTransactionData.PutString("EncryptedEventData", oEncryptedEventData.GetBase64SerializedBuffer());
        // Send the transaction
        
        this->TransactRecordAuditEvent(oRootOfTrustDomainIdentifier, oTransactionData);
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