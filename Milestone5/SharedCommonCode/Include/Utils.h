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
#include "StructuredBuffer.h"

#include <string>

/********************************************************************************************/

extern "C" bool __stdcall IsNumber(
    _in const std::string & c_strValue
    );

extern "C" bool __stdcall IsBoolean(
    _in const std::string & c_strValue
    );

extern "C" std::string __stdcall EscapeJsonString(
    _in const std::string & c_strData,
    _in bool fPutInQuotes = true
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

extern "C" void PutJsonNumberToStructuredBuffer(
    _in const char * c_szRequireParameterName,
    _in Byte bElementType,
    _in float64_t fl64ParameterValue,
    _out StructuredBuffer * poRequestStructuredBuffer
    );

extern "C" bool ValidateUnsignedNumber(
    _in Qword qwParameterValue,
    _in const std::string & c_strRange,
    _in Byte bRangeType
    );

extern "C" bool ValidateSignedNumber(
    _in float64_t fl64ParameterValue,
    _in const std::string & c_strRange,
    _in Byte bRangeType
    );

void __thiscall ReplaceAll(
    _inout std::string & strOriginalString,
    _in const std::string & c_strChangeThis,
    _in const std::string & c_strChangeTo
    );
