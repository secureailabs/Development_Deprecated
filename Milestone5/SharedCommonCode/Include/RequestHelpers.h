/*********************************************************************************************
 *
 * @file RequestHelpers.h
 * @author David Gascon
 * @date 1 Nov 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include <vector>
#include "StructuredBuffer.h"

#pragma once

/********************************************************************************************/

std::vector<Byte> __stdcall CreateRequestPacketFromStructuredBuffer(
    _in const StructuredBuffer & c_oRequest
    );

std::vector<Byte> __stdcall GetUserInfoFromEosb(
    _in const StructuredBuffer & c_oRequest
    );