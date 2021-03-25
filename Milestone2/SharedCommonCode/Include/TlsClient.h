/*********************************************************************************************
 *
 * @file TLSClient.h
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TLSClient connect functions used to create TLS Connections
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "TlsNode.h"

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToUnixDomainSocket(
    _in const char * c_strSocketPath
    );

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToNetworkSocket(
    _in const char * c_strTargetIpAddress,
    _in Word wPortNumber
    );

/********************************************************************************************/

TlsNode * __stdcall TlsConnectToNetworkSocketWithTimeout(
    _in const char * c_strTargetIpAddress,
    _in Word wPortNumber,
    _in unsigned int unMillisecondTimeout,
    _in unsigned int unStepTime
    );

/********************************************************************************************/
