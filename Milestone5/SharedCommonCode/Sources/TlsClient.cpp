/*********************************************************************************************
 *
 * @file TLSClient.cpp
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "Chronometer.h"
#include "SocketClient.h"
#include "TlsClient.h"
#include "TlsNode.h"

#include <string>
#include <iostream>
#include <unistd.h>


/********************************************************************************************/

TlsNode * __stdcall TlsConnectToUnixDomainSocket(
    _in const char * c_szSocketPath
    )
{
    __DebugFunction();

    TlsNode * poTlsNode = nullptr;
    try
    {
        Socket * poSocket = ::ConnectToUnixDomainSocket(std::string(c_szSocketPath));

        poTlsNode = new TlsNode(poSocket, eSSLModeClient);
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return poTlsNode;
}

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToNetworkSocket(
    _in const char * c_szTargetIpAddress,
    _in Word wPortNumber
    )
{
    __DebugFunction();

    TlsNode * poTlsNode = nullptr;

    try
    {
        Socket * poSocket = ::ConnectToNetworkSocket(std::string(c_szTargetIpAddress), wPortNumber);

        poTlsNode = new TlsNode(poSocket, eSSLModeClient);
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return poTlsNode;
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

    TlsNode * poTlsNode = nullptr;

    try
    {
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

        poTlsNode = new TlsNode(poSocket, eSSLModeClient);
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return poTlsNode;
}

/********************************************************************************************/
