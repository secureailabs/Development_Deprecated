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

std::vector<Byte> __stdcall RestApiCall(
    _in const std::string& c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string& c_strVerb,
    _in const std::string& c_strApiUri,
    _in const std::string& c_strContent,
    _in bool fDisableSslCertificateVerification,
    _in const std::vector<std::string>& c_stlListOfHeaders,
    _in long * c_nResponseCode
) throw();

std::vector<Byte> __stdcall RestApiCall(
    _in const std::string& c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string& c_strVerb,
    _in const std::string& c_strApiUri,
    _in const std::string& c_strContent,
    _in bool fDisableSslCertificateVerification,
    _in const std::vector<std::string>& c_stlListOfHeaders,
    _in const std::map<std::string, std::string> * stlMapOfHeaders,
    _inout long * nResponseCode
) throw();
