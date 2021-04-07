/*********************************************************************************************
 *
 * @file 64BitHashes.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

/********************************************************************************************/

extern "C" Qword __cdecl Get64BitHashOfNullTerminatedString(
    _in const char * c_szString,
    _in bool fCaseSensitive
    );

/********************************************************************************************/
    
extern "C" Qword __cdecl Get64BitHashOfByteArray(
    _in const Byte * c_pbBuffer,
    _in unsigned int unBufferSizeInBytes
    );