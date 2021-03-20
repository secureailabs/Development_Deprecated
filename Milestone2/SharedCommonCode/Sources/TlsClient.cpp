/*********************************************************************************************
 *
 * @file TLSClient.cpp
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "TlsClient.h"
#include "TlsNode.h"
#include "DebugLibrary.h"
#include "SocketClient.h"
#include "CoreTypes.h"
#include "Exceptions.h"
#include "Chronometer.h"

#include <unistd.h>
#include <iostream>

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToUnixDomainSocket(
    _in const char * c_strSocketPath
    )
{
    __DebugFunction();

    Socket * poSocket = ::ConnectToUnixDomainSocket(c_strSocketPath);

    return new TlsNode(poSocket, eSSLModeClient);
}

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToNetworkSocket(
    _in const char * c_strTargetIpAddress,
    _in Word wPortNumber
    )
{
    __DebugFunction();

    Socket * poSocket = ::ConnectToNetworkSocket(c_strTargetIpAddress, wPortNumber);

    return new TlsNode(poSocket, eSSLModeClient);
}

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToNetworkSocketWithTimeout(
    _in const char * c_strTargetIpAddress,
    _in Word wPortNumber,
    _in unsigned int unMillisecondTimeout,
    _in unsigned int unMillesecondStepTime
    )
{
    __DebugFunction();

    Chronometer oChronometer;
    oChronometer.Start();
    Socket * poSocket = nullptr;

    while ((nullptr == poSocket) && (unMillisecondTimeout > oChronometer.GetElapsedTimeWithPrecision(Millisecond)))
    {
        try
        {
            poSocket = ::ConnectToNetworkSocket(c_strTargetIpAddress, wPortNumber);
        }
        catch(const BaseException & oBaseException)
        {
            ::sleep(unMillesecondStepTime/1000);
        }
    }
    _ThrowIfNull(poSocket, "Connection request timed out", nullptr);

    return new TlsNode(poSocket, eSSLModeClient);
}

/********************************************************************************************/
