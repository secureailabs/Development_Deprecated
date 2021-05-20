#pragma once

#include "CoreTypes.h"

#include <string>
#include <vector>

/// <summary>
/// 
/// </summary>
/// <param name="c_strHostIpAddress"></param>
/// <param name="nPort"></param>
/// <param name="c_strVerb"></param>
/// <param name="c_strApiUri"></param>
/// <param name="c_strJsonBody"></param>
/// <param name="fEnsureJsonNullTerminate"></param>
/// <returns></returns>
extern std::vector<Byte> __stdcall RestApiCall(
    _in const std::string & c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string & c_strVerb,
    _in const std::string & c_strApiUri,
    _in const std::string & c_strJsonBody,
    _in bool fIsJsonResponse
    ) throw();