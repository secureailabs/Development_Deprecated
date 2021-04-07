/*********************************************************************************************
 *
 * @file HttpRequestParser.h
 * @author Shabana Akhtar Baig
 * @date 30 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"

#include <iostream>
#include <sstream>
#include <map>
#include <string>

/********************************************************************************************/
class HttpRequestParser : public Object
{
    public:

        // Constructor and Destructor
        HttpRequestParser(void);
        virtual ~HttpRequestParser(void);

        // Parse request data
        bool __thiscall ParseRequest(
            _in const std::string & c_strRequestData
            );

        // Parse response data
        bool __thiscall ParseResponse(
            _in const std::string & c_strResponseData
            );

        // Parse http body if Content-Type is application/x-www-form-urlencoded
        void __thiscall ParseUrlEncodedString(
            _in const std::string & c_strRequestBody
        );

        // Fetch position of end of the http request header
        unsigned int __thiscall GetHeaderEndPosition(void) const;

        // Fetch keys in m_stlParameters
        std::vector<std::string> __thiscall GetParameterKeys(void) const;

        // Fetch parameter value associated with c_strKey from m_stlParameters
        std::string __thiscall GetParameterValue(
            _in const std::string & c_strKey
        ) const;
        // Fetch header value associated with c_strKey from m_stlHeaders
        std::string __thiscall GetHeaderValue(
            _in const std::string & c_strKey
        ) const;

        // Check if a parameter value exists for c_strKey in m_stlParameters
        bool __thiscall ParameterExists(
            _in const std::string & c_strKsey
            );
        // Check if a header value exists for c_strKey in m_stlHeaders
        bool __thiscall HeaderExists(
            _in const std::string & c_strKsey
            );

    private:

        // Parse first line of the request to extract Verb, Query, and Protocol
        bool __thiscall ParseFirstLine(
            _in const std::string & c_strRequestData
            );

        // Parse Query to extract the Resource and parameters(if any)
        void __thiscall ExtractQueryParameters(void);
        // Extract request headers
        void __thiscall ExtractHeaders(
            _in const std::string & c_strRequestData
            );

        // Private data members
        unsigned int m_unHeaderEndPosition;
        std::map<std::string,std::string> m_stlHeaders;
        std::map<std::string,std::string> m_stlParameters;
};
