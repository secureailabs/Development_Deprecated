/*********************************************************************************************
 *
 * @file SocketClient.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "Socket.h"
#include <string.h>

/********************************************************************************************/

extern Socket * __stdcall ConnectToUnixDomainSocket(
    _in const std::string & c_strSocketPath
    );

/********************************************************************************************/

extern Socket * __stdcall ConnectToNetworkSocket(
    _in const std::string & c_strTargetIpAddress,
    _in Word wSocket
    );