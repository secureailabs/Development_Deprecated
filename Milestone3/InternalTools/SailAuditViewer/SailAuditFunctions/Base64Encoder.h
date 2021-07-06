/*********************************************************************************************
 *
 * @file Base64Encoding.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include <string>
#include <vector>

/********************************************************************************************/

extern std::string __stdcall Base64Encode(
    _in const Byte * c_pbRawBuffer,
    _in unsigned int unRawBufferSizeInBytes
    ) throw();

/********************************************************************************************/
    
extern std::vector<Byte> __stdcall Base64Decode(
    _in const char * c_szBase64String
    );