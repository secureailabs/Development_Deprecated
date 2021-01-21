/*********************************************************************************************
 *
 * @file Utils.h
 * @author Shabana Akhtar Baig
 * @date 16 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

#include <string>

/********************************************************************************************/

extern "C" bool __stdcall IsNumber(
    _in const std::string & c_strValue
    );

extern "C" bool __stdcall IsBoolean(
    _in const std::string & c_strValue
    );

extern "C" std::string __stdcall EscapeJsonString(
    _in const std::string & c_strData
    );

extern "C" std::string __stdcall UnEscapeJsonString(
    _in const std::string & c_strData
    );

extern "C" std::string __stdcall Base64Encode(
    _in const std::string & c_strInputString
    );

extern "C" std::string __stdcall Base64Decode(
    _in const std::string & c_strInputString
    );
