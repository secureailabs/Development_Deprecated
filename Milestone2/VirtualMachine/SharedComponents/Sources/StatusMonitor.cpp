/*********************************************************************************************
 *
 * @file StatusMonitor.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "IpcTransactionHelperFunctions.h"
#include "SocketClient.h"
#include "StatusMonitor.h"
#include "StructuredBuffer.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>       

#include <iostream>

/********************************************************************************************/
extern char *__progname;

StatusMonitor::StatusMonitor(
    _in const char * c_szDescription
    )
{
    __DebugFunction();
    
    try
    {
        // Build the request packet
        StructuredBuffer oRequest;
        oRequest.PutDword("Transaction", 0x10000000);
        oRequest.PutGuid("MonitoringIdentifier", m_oStatusMonitorIdentifier);
        oRequest.PutString("ProcessFilename", (const char *)__progname);
        oRequest.PutString("Description", c_szDescription);
        oRequest.PutInt32("ProcessIdentifier", (int) ::getpid());
        oRequest.PutInt32("ThreadIdentifier", (int) ::pthread_self());
        // Send the request
        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
        (void) ::PutIpcTransactionAndGetResponse(poSocket, oRequest.GetSerializedBuffer());
        poSocket->Release();
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

/********************************************************************************************/

StatusMonitor::~StatusMonitor(void)
{
    __DebugFunction();
    
    try
    {
        // Build the request packet
        StructuredBuffer oRequest;
        oRequest.PutDword("Transaction", 0x10000003);
        oRequest.PutGuid("MonitoringIdentifier", m_oStatusMonitorIdentifier);
        // Send the request
        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
        (void) ::PutIpcTransactionAndGetResponse(poSocket, oRequest.GetSerializedBuffer());
        poSocket->Release();
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

/********************************************************************************************/

bool __thiscall StatusMonitor::IsTerminating(void) const throw()
{
    __DebugFunction();
    
    bool fIsTerminating = false;
    
    try
    {
        // Build the request packet
        StructuredBuffer oRequest;
        oRequest.PutDword("Transaction", 0x10000001);
        oRequest.PutGuid("MonitoringIdentifier", m_oStatusMonitorIdentifier);
        // Send the request and wait for a response
        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
        StructuredBuffer oResponse(::PutIpcTransactionAndGetResponse(poSocket, oRequest.GetSerializedBuffer()));
        poSocket->Release();
        // Parse the response and extract the terminating signal state
        if ((true == oResponse.IsElementPresent("Success", BOOLEAN_VALUE_TYPE))&&(true == oResponse.IsElementPresent("TerminationSignal", BOOLEAN_VALUE_TYPE)))
        {
            if (true == oResponse.GetBoolean("Success"))
            {
                fIsTerminating = oResponse.GetBoolean("TerminationSignal");
            }
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
    
    return fIsTerminating;
}

/********************************************************************************************/

void __thiscall StatusMonitor::SignalTermination(
    _in const char * c_szReason
    ) const throw()
{
    __DebugFunction();
    
    try
    {
        // Build the request packet
        StructuredBuffer oRequest;
        oRequest.PutDword("Transaction", 0x10000004);
        oRequest.PutGuid("MonitoringIdentifier", m_oStatusMonitorIdentifier);
        (nullptr != c_szReason) ? oRequest.PutString("Reason", c_szReason) : oRequest.PutString("Reason", "Unspecified");
        // Send the request
        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
        (void) ::PutIpcTransactionAndGetResponse(poSocket, oRequest.GetSerializedBuffer());
        poSocket->Release();
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
            
/********************************************************************************************/

void __thiscall StatusMonitor::UpdateStatus(
    _in const char * c_szFormatString,
    ...
    ) const throw()
{
    __DebugFunction();
    
    char * szResultingString = nullptr;
    
    try
    {
        // Generate our status string
        if (nullptr != c_szFormatString)
        {
            va_list pListOfArguments;
            va_start(pListOfArguments, c_szFormatString);
            ::vasprintf(&szResultingString, c_szFormatString, pListOfArguments);
            _ThrowOutOfMemoryExceptionIfNull(szResultingString);
            va_end(pListOfArguments);
        }
        // Build the request packet
        StructuredBuffer oRequest;
        oRequest.PutDword("Transaction", 0x10000002);
        oRequest.PutGuid("MonitoringIdentifier", m_oStatusMonitorIdentifier);
        oRequest.PutString("Status", szResultingString);
        // Send the request packet
        Socket * poSocket = ::ConnectToUnixDomainSocket("/tmp/{597722ad-5085-435b-81d7-6af3e0510432}");
        (void) ::PutIpcTransactionAndGetResponse(poSocket, oRequest);
        poSocket->Release();
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    // No matter what happens, we must free the szResultingString. This is done outside the
    // try...catch block since it's possible some exceptions can get thrown AFTER the string
    // has been allocated.
    if (nullptr != szResultingString)
    {
        ::free((void *) szResultingString);
    }
}