//////////////////////////////////////////////////////////////////////////////////////////////
//
// @file RestApiFunctions.h
// @author Prawal Gangwar
// @refactor Luis Miguel Huapaya
// @date 28 March 2021
// @License Private and Confidential. Internal Use Only.
// @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreTypes.h"

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////

extern std::vector<Byte> __stdcall RestApiCall(
    _in const std::string & c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string & c_strVerb,
    _in const std::string & c_strApiUri,
    _in const std::string & c_strJsonBody,
    _in bool fIsJsonResponse
    ) throw();

//////////////////////////////////////////////////////////////////////////////////////////////

extern std::vector<Byte> __stdcall RestApiCall(
    _in const std::string & c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string & c_strVerb,
    _in const std::string & c_strApiUri,
    _in const std::string & c_strContent,
    _in bool fDisableSslCertificateVerification,
    _in const std::vector<std::string> & c_stlListOfHeaders
    ) throw();