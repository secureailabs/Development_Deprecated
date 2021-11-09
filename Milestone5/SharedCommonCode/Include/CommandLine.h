/*********************************************************************************************
 *
 * @file CommandLine.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "StructuredBuffer.h"

/********************************************************************************************/

extern StructuredBuffer __stdcall ParseCommandLineParameters(
    _in unsigned int unNumberOfCommandLineArguments,
    _in const char ** c_pszCommandLineArguments
    );