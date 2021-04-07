/*********************************************************************************************
 *
 * @file RootOfTrustNode.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief The RootOfTrustNode object is used by various processes within the secure virtual
 * machine in order to access the various facilities offered by the Root-of-Trust process. In
 * this current iteration, everything is hard coded and there is no actual connection made to
 * the root of trust. This allows developers to work on their individual components without
 * waiting for a functional Root-of-Trust process to be made available.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "CoreTypes.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "IpcTransactionHelperFunctions.h"
#include "RootOfTrustNode.h"
#include "SocketClient.h"

#include <iostream>

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function RootOfTrustNode
 * @brief Constructor
 * @param[in] c_szIpcPathForInitialization Path to the temporary Ipc channel used to initialize the root of trust
 *
 ********************************************************************************************/

RootOfTrustNode::RootOfTrustNode(
    _in const char * c_szIpcPathForInitialization
    )
{
    __DebugFunction();

    Socket * poSocket = ::ConnectToUnixDomainSocket(c_szIpcPathForInitialization);
    StructuredBuffer oInitializationData(::GetIpcTransaction(poSocket));
    
    // Get the information
    m_oDomainIdentifier = oInitializationData.GetGuid("YourDomainIdentifier");
    m_strRootOfTrustIpcPath = oInitializationData.GetString("RootOfTrustIpcPath");
    std::cout << "m_strRootOfTrustIpcPath.1 = " << m_strRootOfTrustIpcPath << std::endl;
    // Send a success response
    StructuredBuffer oResponse;
    oResponse.PutBoolean("Success", true);
    ::PutIpcTransaction(poSocket, oResponse);
    poSocket->Release();
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function RootOfTrustNode
 * @brief Copy constructor
 * @note
 *    All this copy constructor does is trigger an assertion failure if called. This is
 *    because this object should never get copied.
 *
 ********************************************************************************************/
 
RootOfTrustNode::RootOfTrustNode(
    _in const RootOfTrustNode & c_oRootOfTrustNode
    )
{
    __DebugFunction();
    //__DebugAssert(false);
    
    //UNREFERENCED_PARAMETER(c_oRootOfTrustNode);
    m_bProcessType = c_oRootOfTrustNode.m_bProcessType;
    m_oDomainIdentifier = c_oRootOfTrustNode.m_oDomainIdentifier;
    m_strRootOfTrustIpcPath = c_oRootOfTrustNode.m_strRootOfTrustIpcPath;
}

/********************************************************************************************
 *
 * @class RootOfTrustNode
 * @function ~RootOfTrustNode
 * @brief Destructor
 *
 ********************************************************************************************/
 
RootOfTrustNode::~RootOfTrustNode(void)
{
    __DebugFunction();
}

/********************************************************************************************/

Guid __thiscall RootOfTrustNode::GetDomainIdentifier(void) const
{
    __DebugFunction();
    
    return m_oDomainIdentifier;
}

/********************************************************************************************/

std::vector<Byte> __thiscall RootOfTrustNode::GetDataset(void) const
{
    __DebugFunction();
    
    Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
    StructuredBuffer oTransactionData;
    oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
    oTransactionData.PutDword("Transaction", 0x00000006);
    StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
    poSocket->Release();

    return oTransactionResponse.GetBuffer("Dataset");
}

/********************************************************************************************/

void __thiscall RootOfTrustNode::RecordAuditEvent(
    _in const char * c_szEventName,
    _in Word wTargetChannelsBitMask,
    _in Dword dwEventType,
    _in const StructuredBuffer & c_oEventData
    ) const
{
    __DebugFunction();
    try
    {
        // Construct the transaction packet
        StructuredBuffer oTransactionData;
        oTransactionData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
        oTransactionData.PutGuid("DomainIdentifier", m_oDomainIdentifier);
        oTransactionData.PutDword("Transaction", 0x00000009);
        oTransactionData.PutString("EventName", c_szEventName);
        oTransactionData.PutWord("TargetChannelsBitMask", wTargetChannelsBitMask);
        oTransactionData.PutDword("EventType", dwEventType);
        oTransactionData.PutStructuredBuffer("EventData", c_oEventData);
        // Send the transaction
        Socket * poSocket = ::ConnectToUnixDomainSocket(m_strRootOfTrustIpcPath.c_str());
        StructuredBuffer oTransactionResponse(::PutIpcTransactionAndGetResponse(poSocket, oTransactionData));
        poSocket->Release();
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
}
