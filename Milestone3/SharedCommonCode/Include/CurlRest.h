#pragma once

#include "CoreTypes.h"

#include <string>
#include <vector>

extern std::vector<Byte> __stdcall RestApiCall(
    _in const std::string & c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string & c_strVerb,
    _in const std::string & c_strApiUri,
    _in const std::string & c_strJsonBody,
    _in bool fIsJsonResponse
    ) throw();

extern std::vector<Byte> __stdcall RestApiCall(
    _in const std::string& c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string& c_strVerb,
    _in const std::string& c_strApiUri,
    _in const std::string& c_strContent,
    _in bool fDisableSslCertificateVerification,
    _in const std::vector<std::string>& c_stlListOfHeaders
) throw();
