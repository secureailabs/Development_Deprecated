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
