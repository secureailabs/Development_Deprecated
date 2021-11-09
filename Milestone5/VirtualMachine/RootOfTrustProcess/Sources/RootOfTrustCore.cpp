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
#include "CurlRest.h"
#include "Utils.h"
#include "JsonValue.h"

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
static std::mutex gs_stlAuditEventsMutex;
static bool gs_fIsInitialized;
static bool gs_fIsRunning;

static std::string gs_strDataOwnerAccessToken;
static std::string gs_strResearcherEosb;
static std::string gs_strVirtualMachineEosb;

static std::string gs_strNameOfVirtualMachine;
static std::string gs_strIpAddressOfVirtualMachine;
static std::string gs_strVirtualMachineIdentifier;
static std::string gs_strClusterIdentifier;
static std::string gs_strDigitalContractIdentifier;
static std::string gs_strDatasetIdentifier;
static std::string gs_strRootOfTrustDomainIdentifier;
static std::string gs_strComputationalDomainIdentifier;
static std::string gs_strDataConnectorDomainIdentifier;
static std::string gs_strSailWebApiPortalIpAddress;
static std::string gs_strDataOwnerOrganizationIdentifier;
static std::string gs_strDataOwnerUserIdentifier;
static std::vector<Byte> gs_stlDataset;

static std::string gs_strRootOfTrustIpcPath;
static std::string gs_strComputationalDomainIpcPath;
static std::string gs_strDataDomainIpcPath;
        
static std::string gs_strDataOrganizationAuditEventParentBranchNodeIdentifier;
static std::string gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
static std::queue<std::string> gs_stlResearchOrganizationAuditEventQueue;
static std::queue<std::string> gs_stlDataOrganizationAuditEventQueue;
static std::queue<std::string> gs_stlIndependentAuditorOrganizationAuditEventQueue;
static std::queue<std::string> gs_stlSailOrganizationAuditEventQueue;

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
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
{
    __DebugFunction();

    StructuredBuffer oInitializationData(c_stlSerializedInitializationParameters);
    gs_fIsInitialized = false;
    gs_fIsRunning = false;
    gs_strNameOfVirtualMachine = oInitializationData.GetString("NameOfVirtualMachine");
    gs_strIpAddressOfVirtualMachine = oInitializationData.GetString("IpAddressOfVirtualMachine");
    gs_strVirtualMachineIdentifier = oInitializationData.GetString("VirtualMachineIdentifier");
    gs_strClusterIdentifier = oInitializationData.GetString("ClusterIdentifier");
    gs_strDigitalContractIdentifier = oInitializationData.GetString("DigitalContractIdentifier");
    gs_strDatasetIdentifier = oInitializationData.GetString("DatasetIdentifier");
    gs_strRootOfTrustDomainIdentifier = oInitializationData.GetString("RootOfTrustDomainIdentifier");
    gs_strComputationalDomainIdentifier = oInitializationData.GetString("ComputationalDomainIdentifier");
    gs_strDataConnectorDomainIdentifier = oInitializationData.GetString("DataConnectorDomainIdentifier");
    gs_strSailWebApiPortalIpAddress = oInitializationData.GetString("SailWebApiPortalIpAddress");
    gs_strDataOwnerAccessToken = oInitializationData.GetString("DataOwnerAccessToken");
    gs_strDataOwnerOrganizationIdentifier = oInitializationData.GetString("DataOwnerOrganizationIdentifier");
    gs_strDataOwnerUserIdentifier = oInitializationData.GetString("DataOwnerUserIdentifier");
    std::string strBase64EncodedSerializedDataset = oInitializationData.GetString("Base64EncodedDataset");
    gs_stlDataset = ::Base64Decode(strBase64EncodedSerializedDataset.c_str());
    gs_strVirtualMachineEosb = oInitializationData.GetString("VmEosb");

    gs_strRootOfTrustIpcPath = Guid().ToString(eRaw);
    gs_strComputationalDomainIpcPath = Guid().ToString(eRaw);
    gs_strDataDomainIpcPath = Guid().ToString(eRaw);

    gs_fIsInitialized = true;

    ::SetIpAddressOfSailWebApiPortalGateway(gs_strSailWebApiPortalIpAddress, 6200);

    this->InitializeVirtualMachine();
    this->RegisterDataOwnerEosb();
}

/********************************************************************************************/

RootOfTrustCore::RootOfTrustCore(
    _in const RootOfTrustCore & c_oRootOfTrust
    )
{
    __DebugFunction();
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
    __DebugAssert(true == gs_fIsInitialized);
    
    return Guid(gs_strDataConnectorDomainIdentifier.c_str());
}

/********************************************************************************************/

Guid __thiscall RootOfTrustCore::GetComputationalDomainIdentifier(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == gs_fIsInitialized);

    return Guid(gs_strComputationalDomainIdentifier.c_str());
}

/********************************************************************************************/

std::string __thiscall RootOfTrustCore::GetRootOfTrustIpcPath(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == gs_fIsInitialized);
    
    return gs_strRootOfTrustIpcPath;
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
            const std::lock_guard<std::mutex> lock(gs_stlAuditEventsMutex);
            
            // Handle lingering events within the data organization audit event queue
            if ((0 < gs_strDataOwnerAccessToken.size())&&(0 < gs_strDataOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                bool fTransmitAuditEvents = false;
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < gs_stlDataOrganizationAuditEventQueue.size())
                {
                    fTransmitAuditEvents = true;
                    std::cout << "Transmitting an audit event (DOO)" << std::endl;
                    std::cout << gs_stlDataOrganizationAuditEventQueue.front() << std::endl;
                    StructuredBuffer oNewAuditEvent(gs_stlDataOrganizationAuditEventQueue.front().c_str());
                    gs_stlDataOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                    
                }
                
                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < gs_strDataOwnerAccessToken.size());
                    __DebugAssert(0 < gs_strDataOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(gs_strDataOwnerAccessToken, gs_strDataOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
            
            if ((0 < gs_strResearcherEosb.size())&&(0 < gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                bool fTransmitAuditEvents = false;
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < gs_stlResearchOrganizationAuditEventQueue.size())
                {
                    std::cout << "Transmitting an audit event (RO)" << std::endl;
                    fTransmitAuditEvents = true;
                    std::cout << gs_stlResearchOrganizationAuditEventQueue.front() << std::endl;
                    StructuredBuffer oNewAuditEvent(gs_stlResearchOrganizationAuditEventQueue.front().c_str());
                    gs_stlResearchOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                }

                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < gs_strResearcherEosb.size());
                    __DebugAssert(0 < gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(gs_strResearcherEosb, gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
        }
        
        catch (BaseException oException)
        {
            ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
            oEventData.PutString("IpAddressOfSecureVirtualMachine", gs_strIpAddressOfVirtualMachine);
            oEventData.PutString("VirtualMachineIdentifier", gs_strClusterIdentifier);
            oEventData.PutString("ClusterIdentifier", gs_strClusterIdentifier);
            this->RecordInternalAuditEvent("VM_SHUTDOWN", 0x1111, 0x05, oEventData);
        }
        
        // Put this thread to sleep for 5 seconds
        ::sleep(30);
    }
}

/********************************************************************************************/

void __thiscall RootOfTrustCore::RunIpcListener(void)
{
    __DebugFunction();
    __DebugAssert(true == gs_fIsInitialized);
    
    std::cout << __func__ << std::endl;
    
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    if (false == gs_fIsRunning)
    {
        gs_fIsRunning = true;
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", RootOfTrustAuditEventDispatcherThread, (void *) this)), "Failed to start the audit event dispacher thread", nullptr);
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", RootOfTrustIpcListenerThread, (void *) this)), "Failed to start the Root of Trust Listener Thread", nullptr);
    }
    else
    {
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RunIpcListener(void)");
        SocketServer * poIpcServer = new SocketServer(gs_strRootOfTrustIpcPath.c_str());
        _ThrowOutOfMemoryExceptionIfNull(poIpcServer);
        unsigned int unNumberOfLoops = 0;
        unsigned int unNumberOfSuccessfulTransactions = 0;
        unsigned int unNumberOfFailedTransactions = 0;
        while (false == oStatusMonitor.IsTerminating())
        {
            // We wait for a connection
            if (true == poIpcServer->WaitForConnection(1000))
            {
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
        gs_fIsRunning = false;
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
    
    StructuredBuffer oTransactionParameters(::GetIpcTransaction(poSocket, false));
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
        if (c_oOriginatingDomainIdentifier == Guid(gs_strDataConnectorDomainIdentifier.c_str()))
        {
            oResponseBuffer.PutBoolean("Success", true);
            oResponseBuffer.PutBuffer("Dataset", gs_stlDataset);
        }
        else
        {
            oResponseBuffer.PutBoolean("Success", false);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        Guid computationalDomainIdentifier(gs_strComputationalDomainIdentifier.c_str());
        std::cout << "c_oOriginatingDomainIdentifier = " << c_oOriginatingDomainIdentifier.ToString(eHyphensAndCurlyBraces) << std::endl;
        std::cout << "computationalDomainIdentifier = " << computationalDomainIdentifier.ToString(eHyphensAndCurlyBraces) << std::endl;
        if (c_oOriginatingDomainIdentifier == computationalDomainIdentifier)
        {
            StructuredBuffer oEncryptedData(oCopyOfTransactionParameters.GetString("EncryptedEventData").c_str());
            std::string eventName = oEncryptedData.GetString("EventName");
            if (eventName == "CONNECT_SUCCESS")
            {
                gs_strResearcherEosb = oEncryptedData.GetString("Eosb");
                this->RegisterResearcherEosb();
            }
        }
        
        const std::lock_guard<std::mutex> lock(gs_stlAuditEventsMutex);
        if (0x1000 == (0x1000 & wTargetChannelsBitMask)) // Third Party Auditor Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unThirdPartyAuditorAuditEventSequenceNumber, 1);
            //oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            //gs_stlIndependentAuditorOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0100 == (0x0100 & wTargetChannelsBitMask)) // Data Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unDataOwnerAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            gs_stlDataOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0010 == (0x0010 & wTargetChannelsBitMask)) // Research Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unResearcherAuditEventSequenceNumber, 1);
            oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            gs_stlResearchOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
        if (0x0001 == (0x0001 & wTargetChannelsBitMask)) // Sail Organization Channel
        {
            unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &s_unSailAuditEventSequenceNumber, 1);
            //oCopyOfTransactionParameters.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
            //gs_stlSailOrganizationAuditEventQueue.push(oCopyOfTransactionParameters.GetBase64SerializedBuffer());
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        oEventData.PutString("IpAddressOfSecureVirtualMachine", gs_strIpAddressOfVirtualMachine);
        oEventData.PutString("VirtualMachineIdentifier", gs_strClusterIdentifier);
        oEventData.PutString("ClusterIdentifier", gs_strClusterIdentifier);
        this->RecordInternalAuditEvent("VM_INITIALIZE", 0x1111, 0x05, oEventData);
        {
            // Make sure we are thread safe
            const std::lock_guard<std::mutex> lock(gs_stlAuditEventsMutex);
            // Make sure all of the parameters are proper
            if ((0 < gs_strDataOwnerAccessToken.size())&&(0 < gs_strVirtualMachineIdentifier.size())&&(0 < gs_strDigitalContractIdentifier.size())&&(0 < gs_strIpAddressOfVirtualMachine.size()))
            {
                // gs_strVirtualMachineEosb = ::RegisterVirtualMachineWithSailWebApiPortal(gs_strDataOwnerAccessToken, gs_strVirtualMachineIdentifier, gs_strDigitalContractIdentifier, gs_strIpAddressOfVirtualMachine);
                std::string strVerb = "PUT";
                // TODO: Prawal make this an enum or string.. VirtualMachineState::eWaitingForData is 5
                std::string strApiUrl = "/SAIL/VirtualMachineManager/UpdateStatus?Eosb="+ gs_strDataOwnerAccessToken;
                StructuredBuffer oStateUpdateRequest;
                oStateUpdateRequest.PutString("VirtualMachineGuid", gs_strVirtualMachineIdentifier);
                oStateUpdateRequest.PutDword("State", 5);
                std::string strContent = JsonValue::ParseStructuredBufferToJson(oStateUpdateRequest)->ToString();
                std::cout << "/SAIL/VirtualMachineManager/UpdateStatus " << strContent << std::endl;

                // Make the API call and get REST response
                std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strSailWebApiPortalIpAddress, (Word)6200, strVerb, strApiUrl, strContent, true);
                std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
                StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
                _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error updating the virtual machine status.", nullptr);

                fSuccess = true;
            }
        }
        // Now, we generate an event to recognize the digital contract initialization
        oEventData.Clear();
        oEventData.PutString("DigitalContractIdentifier", gs_strDigitalContractIdentifier);
        oEventData.PutString("VirtualMachineIdentifier", gs_strClusterIdentifier);
        oEventData.PutString("ClusterIdentifier", gs_strClusterIdentifier);
        this->RecordInternalAuditEvent("DC_INITIALIZE", 0x1111, 0x05, oEventData);
    }

    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        const std::lock_guard<std::mutex> lock(gs_stlAuditEventsMutex);
        // Make sure all of the parameters are proper
        if ((0 < gs_strDataOwnerAccessToken.size())&&(0 < gs_strVirtualMachineIdentifier.size()))
        {
            gs_strDataOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineDataOwner(gs_strDataOwnerAccessToken, gs_strVirtualMachineIdentifier);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        const std::lock_guard<std::mutex> lock(gs_stlAuditEventsMutex);
        // Make sure all of the parameters are proper
        if ((0 < gs_strResearcherEosb.size())&&(0 < gs_strVirtualMachineIdentifier.size()))
        {
            gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineResearcher(gs_strResearcherEosb, gs_strVirtualMachineIdentifier);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
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
        Guid oRootOfTrustDomainIdentifier(gs_strRootOfTrustDomainIdentifier.c_str());
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