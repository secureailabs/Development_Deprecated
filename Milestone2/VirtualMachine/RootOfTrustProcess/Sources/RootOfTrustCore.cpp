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
#include "DebugLibrary.h"
#include "Exceptions.h"
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
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
        // Print out the exception
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    
    catch (...)
    {
        // If there is an exception here, this means that the RootOfTrust process is
        // truly wrecked. We need to signal termination across the board
        StatusMonitor oStatusMonitor("void __thiscall RootOfTrustCore::RootOfTrustIpcListenerThread(void)");
        oStatusMonitor.SignalTermination("Unrecoverable exception");
        // Print out the exception
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
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
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    
    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    
    // No matter what happens in the transaction, we need to release the socket and then
    // free the RootOfTrustTransactionPacket block (in that specific order)
    
    poRootOfTrustTransactionPacket->poSocket->Release();
    gs_oMemoryAllocator.Deallocate(pParameter);
    
    return nullptr;
}

/********************************************************************************************/

RootOfTrustCore::RootOfTrustCore(
    _in const std::vector<Byte> & c_stlSerializedInitializationParameters
    )
    : m_fIsInitialized(false), m_fIsRunning(false)
{
    __DebugFunction();
    
    // This structure buffer should contain
    // 1. DigitalContract (buffer)
    // 2. SAIL root cert (buffer)
    // 3. Root cert for the Data Domain (buffer)
    // 4. Root cert for the Computational Domain (buffer)
    // 5. IESOB of the Data Owner (buffer)
    // 6. UUID of the cluster of nodes
    // 7. UUID of the RootOfTrust domain across all cluster nodes
    // 8. UUID of the Computational domain across all cluster nodes
    // 9. UUID of the Data domain across all cluster nodes
    // 10. Dataset (buffer)
    
    StructuredBuffer oInitializationData(c_stlSerializedInitializationParameters);
    
    m_stlSerializedDigitalContract = oInitializationData.GetBuffer("DigitalContract");
    m_stlGlobalRootKeyCertificate = oInitializationData.GetBuffer("GlobalRootKeyCertificate");
    m_stlComputationalDomainRootKeyCertificate = oInitializationData.GetBuffer("ComputationalDomainRootKeyCertificate");
    m_stlDataDomainRootKeyCertificate = oInitializationData.GetBuffer("DataDomainRootKeyCertificate");
    m_stlDataOwnerImpostorEncryptedOpaqueSessionBlob = oInitializationData.GetBuffer("DataOwnerAccessToken");
    m_oClusterInstanceIdentifier = oInitializationData.GetGuid("ClusterUuid");
    m_oRootOfTrustDomainIdentifier = oInitializationData.GetGuid("RootOfTrustDomainUuid");
    m_oComputationalDomainIdentifier = oInitializationData.GetGuid("ComputationalDomainUuid");
    m_oDataDomainIdentifier = oInitializationData.GetGuid("DataDomainUuid");
    m_stlDataSet = oInitializationData.GetBuffer("Dataset");
    
    m_strRootOfTrustIpcPath = Guid().ToString(eRaw);
    m_strComputationalDomainIpcPath = Guid().ToString(eRaw);
    m_strDataDomainIpcPath = Guid().ToString(eRaw);
        
    m_fIsInitialized = true;
}

/********************************************************************************************/

RootOfTrustCore::RootOfTrustCore(
    _in const RootOfTrustCore & c_oRootOfTrust
    )
{
    __DebugFunction();
    
    if (true == c_oRootOfTrust.m_fIsInitialized)
    {
        m_stlSerializedDigitalContract = c_oRootOfTrust.m_stlSerializedDigitalContract;
        m_stlGlobalRootKeyCertificate = c_oRootOfTrust.m_stlGlobalRootKeyCertificate;
        m_stlComputationalDomainRootKeyCertificate = c_oRootOfTrust.m_stlComputationalDomainRootKeyCertificate;
        m_stlDataDomainRootKeyCertificate = c_oRootOfTrust.m_stlDataDomainRootKeyCertificate;
        m_stlDataOwnerImpostorEncryptedOpaqueSessionBlob = c_oRootOfTrust.m_stlDataOwnerImpostorEncryptedOpaqueSessionBlob;
        m_oClusterInstanceIdentifier = c_oRootOfTrust.m_oClusterInstanceIdentifier;
        m_oRootOfTrustDomainIdentifier = c_oRootOfTrust.m_oRootOfTrustDomainIdentifier;
        m_oComputationalDomainIdentifier = c_oRootOfTrust.m_oComputationalDomainIdentifier;
        m_oDataDomainIdentifier = c_oRootOfTrust.m_oDataDomainIdentifier;
        m_stlDataSet = c_oRootOfTrust.m_stlDataSet;
        
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
    __DebugAssert(false == m_fIsRunning);
    
    // TODO: Securely zeroize buffers
}

/********************************************************************************************/

Guid __thiscall RootOfTrustCore::GetRootOfTrustDomainIdentifier(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return m_oRootOfTrustDomainIdentifier;
}

/********************************************************************************************/

Guid __thiscall RootOfTrustCore::GetComputationalDomainIdentifier(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return m_oComputationalDomainIdentifier;
}

/********************************************************************************************/

Guid __thiscall RootOfTrustCore::GetDataDomainIdentifier(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return m_oDataDomainIdentifier;
}

/********************************************************************************************/

std::string __thiscall RootOfTrustCore::GetRootOfTrustIpcPath(void) const throw()
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    return m_strRootOfTrustIpcPath;
}

/********************************************************************************************/

void __thiscall RootOfTrustCore::RunIpcListener(void)
{
    __DebugFunction();
    __DebugAssert(true == m_fIsInitialized);
    
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    
    if (false == m_fIsRunning)
    {
        m_fIsRunning = true;
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeListener", RootOfTrustIpcListenerThread, (void *) this)), "Failed to start the Root of Trust Listener Thread", nullptr);
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
            // We wait for a connection
            if (true == poIpcServer->WaitForConnection(1000))
            {
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
    
    StructuredBuffer oTransactionParameters(::GetIpcTransaction(poSocket));
    std::vector<Byte> stlSerializedResponse;
    
    // By default, ALL incoming transactions must have, at least, a DomainIdentifier
    // as well as a Transaction identifier
    Guid oOriginatingDomainIdentifier = oTransactionParameters.GetGuid("DomainIdentifier");
    Dword dwTransaction = oTransactionParameters.GetDword("Transaction");
    
    switch(dwTransaction)
    {
        case 0x00000001 //  "GetDataDomainIpcPath"
        :   stlSerializedResponse = this->TransactGetDataDomainIpcPath(oOriginatingDomainIdentifier);
            break;
        case 0x00000002 //  "GetDataDomainRootKeyCertificate"
        :   stlSerializedResponse = this->TransactGetDataDomainRootKeyCertificate(oOriginatingDomainIdentifier);
            break;    
        case 0x00000003 //  "GetComputationalDomainIpcPath"
        :   stlSerializedResponse = this->TransactGetComputationalDomainIpcPath(oOriginatingDomainIdentifier);
            break;
        case 0x00000004 //  "GetComputationalDomainRootKeyCertificate"
        :   stlSerializedResponse = this->TransactGetComputationalDomainRootKeyCertificate(oOriginatingDomainIdentifier);
            break;
        case 0x00000005 //  "GetDigitalContract"
        :   stlSerializedResponse = this->TransactGetDigitalContract(oOriginatingDomainIdentifier);
            break;
        case 0x00000006 //  "GetDataSet"
        :   stlSerializedResponse = this->TransactGetDataSet(oOriginatingDomainIdentifier);
            break;
        case 0x00000007 // "GetAllDomainCertificates"
        :   stlSerializedResponse = this->TransactGetAllDomainCertificates(oOriginatingDomainIdentifier);
            break;
        case 0x00000008 // "GenerateEphemeralTLSKeyPair"
        :   stlSerializedResponse = this->TransactGenerateEphemeralTlsKeyPair(oOriginatingDomainIdentifier);
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

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetDataDomainIpcPath(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();

    StructuredBuffer oResponseBuffer;
    
    if ((c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)||(c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier))
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutString("DataDomainIpcPath", m_strDataDomainIpcPath);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetDataDomainRootKeyCertificate(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if ((c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)||(c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier))
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("DataOwnerRootKeyCertificate", m_stlDataDomainRootKeyCertificate);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetComputationalDomainIpcPath(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if (c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier)
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutString("ComputationalDomainIpcPath", m_strComputationalDomainIpcPath);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetComputationalDomainRootKeyCertificate(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if ((c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)||(c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier))
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("ComputationalDomainRootKeyCertificate", m_stlComputationalDomainRootKeyCertificate);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetDigitalContract(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if ((c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)||(c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier))
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("DigitalContract", m_stlSerializedDigitalContract);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetDataSet(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if (c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("DataSet", m_stlDataSet);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGetAllDomainCertificates(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    oResponseBuffer.PutBoolean("Success", true);
    oResponseBuffer.PutBuffer("GlobalRootKeyCertificate", m_stlGlobalRootKeyCertificate);
    oResponseBuffer.PutBuffer("DataDomainRootKeyCertificate", m_stlDataDomainRootKeyCertificate);
    oResponseBuffer.PutBuffer("ComputationalDomainRootKeyCertificate", m_stlComputationalDomainRootKeyCertificate);
    
    //std::cout << "m_stlGlobalRootKeyCertificate.size()" << m_stlGlobalRootKeyCertificate.size() << std::endl;
    //std::cout << "m_stlDataDomainRootKeyCertificate.size()" << m_stlGlobalRootKeyCertificate.size() << std::endl;
    //std::cout << "m_stlComputationalDomainRootKeyCertificate.size()" << m_stlGlobalRootKeyCertificate.size() << std::endl;
    
    //std::cout << "oResponseBuffer.GetSerializedBufferRawDataSizeInBytes() = " << oResponseBuffer.GetSerializedBufferRawDataSizeInBytes() << std::endl;
    
    return oResponseBuffer.GetSerializedBuffer();
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustCore::TransactGenerateEphemeralTlsKeyPair(
    _in const Guid & c_oOriginatingDomainIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oResponseBuffer;
    
    if (c_oOriginatingDomainIdentifier == m_oDataDomainIdentifier)
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("PrivateKey", gc_abDataOwnerTlsPrivateKey, gc_unDataOwnerTlsPrivateKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKey", gc_abDataOwnerTlsPublicKey, gc_unDataOwnerTlsPublicKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKeyCertificate", gc_abDataOwnerTlsPublicKeyCertificate, gc_unDataOwnerTlsPublicKeyCertificateSizeInBytes);
    }
    else if (c_oOriginatingDomainIdentifier == m_oComputationalDomainIdentifier)
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("PrivateKey", gc_abResearcherTlsPrivateKey, gc_unResearcherTlsPrivateKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKey", gc_abResearcherTlsPublicKey, gc_unResearcherTlsPublicKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKeyCertificate", gc_abResearcherTlsPublicKeyCertificate, gc_unResearcherTlsPublicKeyCertificateSizeInBytes);
    }
    else if (c_oOriginatingDomainIdentifier == m_oInitializerDomainIdentifier)
    {
        oResponseBuffer.PutBoolean("Success", true);
        oResponseBuffer.PutBuffer("PrivateKey", gc_abInitializerTlsPrivateKey, gc_unInitializerTlsPrivateKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKey", gc_abInitializerTlsPublicKey, gc_unInitializerTlsPublicKeySizeInBytes);
        oResponseBuffer.PutBuffer("PublicKeyCertificate", gc_abInitializerTlsPublicKeyCertificate, gc_unInitializerTlsPublicKeyCertificateSizeInBytes);
    }
    
    return oResponseBuffer.GetSerializedBuffer();
}