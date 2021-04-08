/*********************************************************************************************
 *
 * @file HttpRequestParser.cpp
 * @author Shabana Akhtar Baig
 * @date 30 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "HttpRequestParser.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function HttpRequestParser
 * @brief Constructor
 *
 ********************************************************************************************/

HttpRequestParser::HttpRequestParser(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ~HttpRequestParser
 * @brief Destructor
 *
 ********************************************************************************************/

HttpRequestParser::~HttpRequestParser(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ParseRequest
 * @brief Parse request data
 * @param[in] c_strRequestData request data
 * @return true if parsed successfully
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall HttpRequestParser::ParseRequest(
    _in const std::string & c_strRequestData
    )
{
    __DebugFunction();

    bool fSuccess = true;
    std::string strFirstLine, strHeaders;
    std::stringstream oParser(c_strRequestData);

    std::getline(oParser, strFirstLine, '\r');

    // Get Verb, Resource, Query parameters, and Protocol
    if ( false == this->ParseFirstLine(strFirstLine))
    {
        fSuccess = false;
    }
    else
    {
        // Get request Headers
        unsigned int unCurrentPosition = oParser.tellg();
        // Get position of end of http request headers
        m_unHeaderEndPosition = c_strRequestData.find("\r\n\r\n") + 4;
        if (std::string::npos != m_unHeaderEndPosition)
        {
            strHeaders = c_strRequestData.substr(unCurrentPosition, (m_unHeaderEndPosition - unCurrentPosition));
        }
        else
        {
            m_unHeaderEndPosition = c_strRequestData.find("\n\r\n\r");
            if (std::string::npos != m_unHeaderEndPosition)
            {
                strHeaders = c_strRequestData.substr(unCurrentPosition, (m_unHeaderEndPosition - unCurrentPosition));
            }
            else
            {
                strHeaders = c_strRequestData.substr(unCurrentPosition);
            }

        }

        // Parse Query parameters and Headers
        this->ExtractQueryParameters();
        this->ExtractHeaders(strHeaders);
    }

    return fSuccess;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ParseResponse
 * @brief Parse response data
 * @param[in] c_strResponseData response data
 * @return true if parsed successfully
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall HttpRequestParser::ParseResponse(
    _in const std::string & c_strResponseData
    )
{
    __DebugFunction();

    bool fSuccess = true;
    std::string strFirstLine, strHeaders;
    std::stringstream oParser(c_strResponseData);

    std::getline(oParser, strFirstLine, '\r');

    // Get response Headers
    unsigned int unCurrentPosition = oParser.tellg();
    // Get position of end of http response headers
    m_unHeaderEndPosition = c_strResponseData.find("\r\n\r\n") + 4;
    if (std::string::npos != m_unHeaderEndPosition)
    {
        strHeaders = c_strResponseData.substr(unCurrentPosition, (m_unHeaderEndPosition - unCurrentPosition));
    }
    else
    {
        m_unHeaderEndPosition = c_strResponseData.find("\n\r\n\r");
        if (std::string::npos != m_unHeaderEndPosition)
        {
            strHeaders = c_strResponseData.substr(unCurrentPosition, (m_unHeaderEndPosition - unCurrentPosition));
        }
        else
        {
            strHeaders = c_strResponseData.substr(unCurrentPosition);
        }

    }

    // Parse Headers
    this->ExtractHeaders(strHeaders);

    return fSuccess;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ParseUrlEncodedString
 * @brief Parse url encoded string to get the parameters(if any)
 * @param[in] c_strRequestBody request body
 *
 ********************************************************************************************/

void __thiscall HttpRequestParser::ParseUrlEncodedString(
    _in const std::string & c_strRequestBody
    )
{
    __DebugFunction();

    std::stringstream oTokenStream(c_strRequestBody);
    std::string strUrlEncoded, strKey, strValue;

    while (std::getline(oTokenStream, strUrlEncoded, '&'))
    {
        if (0 < strUrlEncoded.length())
        {
            std::stringstream oParameterEntry(strUrlEncoded);
            std::getline(oParameterEntry, strKey, '=');
            std::getline(oParameterEntry, strValue);
            if ((!strKey.empty()) && (!strValue.empty()))
            {
                m_stlParameters[strKey] = strValue;
            }
        }
    }
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function GetHeaderEndPosition
 * @brief Get position of end of header
 * @return Position of end of header
 *
 ********************************************************************************************/

unsigned int __thiscall HttpRequestParser::GetHeaderEndPosition(void) const
{
    __DebugFunction();

    return m_unHeaderEndPosition;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function GetParameterKeys
 * @brief Get m_stlParameters keys
 * @return m_stlParameters keys
 *
 ********************************************************************************************/

std::vector<std::string> __thiscall HttpRequestParser::GetParameterKeys(void) const
{
    __DebugFunction();

    std::vector<std::string> stlParameterKeys;

    for (std::map<std::string,std::string>::const_iterator itrParameterMap = m_stlParameters.begin(); itrParameterMap != m_stlParameters.end(); ++itrParameterMap)
    {
        stlParameterKeys.push_back(itrParameterMap->first);
    }

    return stlParameterKeys;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function GetParameterValue
 * @brief Fetch parameter value associated with c_strKey from m_stlParameters
 * @param[in] c_strKey parameter key
 * @return Value associated with c_strKey
 *
 ********************************************************************************************/

std::string __thiscall HttpRequestParser::GetParameterValue(
    _in const std::string & c_strKey
    ) const
{
    __DebugFunction();

    std::string strParameterValue;

    std::map<std::string,std::string>::const_iterator itrParameterMap = m_stlParameters.find(c_strKey);

    if (m_stlParameters.end() != itrParameterMap)
    {
        strParameterValue = itrParameterMap->second;
    }

    return strParameterValue;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function GetHeaderValue
 * @brief Fetch header value associated with c_strKey from m_stlHeaders
 * @param[in] c_strKey header key
 * @return Value associated with c_strKey
 *
 ********************************************************************************************/

std::string __thiscall HttpRequestParser::GetHeaderValue(
    _in const std::string & c_strKey
    ) const
{
    __DebugFunction();

    std::string strHeaderValue;

    std::map<std::string,std::string>::const_iterator itrHeaderMap = m_stlHeaders.find(c_strKey);

    if (m_stlHeaders.end() != itrHeaderMap)
    {
        strHeaderValue = itrHeaderMap->second;
    }

    return strHeaderValue;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ParameterExists
 * @brief Check if a parameter value exists for c_strKey in m_stlParameters
 * @param[in] c_strKey parameter key
 * @return true if parameter exists
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall HttpRequestParser::ParameterExists(
    _in const std::string & c_strKey
    )
{
    __DebugFunction();

    bool fFound = m_stlParameters.find(c_strKey) != m_stlParameters.end();

    return fFound;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function HeaderExists
 * @brief Check if a header value exists for c_strKey in m_stlHeaders
 * @param[in] c_strKey header key
 * @return true if parameter exists
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall HttpRequestParser::HeaderExists(
    _in const std::string & c_strKey
    )
{
    __DebugFunction();

    bool fFound = m_stlHeaders.find(c_strKey) != m_stlHeaders.end();

    return fFound;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ParseFirstLine
 * @brief Parse first line of the request to get Verb, Query, and Protocol
 * @param[in] c_strRequestData request data
 * @return true if parsed successfully
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall HttpRequestParser::ParseFirstLine(
    _in const std::string & c_strRequestData
    )
{
    __DebugFunction();

    bool fSuccess = false;
    std::string strQuery, strVerb, strProtocol;
    std::stringstream oFirstLineStream(c_strRequestData);

    // Get Verb, Query and Protocol strings
    std::getline(oFirstLineStream, strVerb, ' ');
    std::getline(oFirstLineStream, strQuery, ' ');
    std::getline(oFirstLineStream, strProtocol, '\r');
    if ((!strVerb.empty()) && (!strQuery.empty()) && (!strProtocol.empty()) && (0 == strProtocol.rfind("HTTP", 0)))
    {
        m_stlHeaders["Verb"] = strVerb;
        m_stlHeaders["Query"] = strQuery;
        m_stlHeaders["Protocol"] = strProtocol;

        fSuccess = true;
    }
    else
    {
        _ThrowBaseException("ERROR: Invalid request.", nullptr);
    }

    return fSuccess;
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ExtractQueryParameters
 * @brief Parse Query to get the Resource and parameters(if any)
 *
 ********************************************************************************************/

void __thiscall HttpRequestParser::ExtractQueryParameters(void)
{
    __DebugFunction();

    std::stringstream oTokenStream(m_stlHeaders["Query"]);
    std::string strQuery, strKey, strValue, strResource;

    // Get Resource string
    std::getline(oTokenStream, strResource, '?');
    m_stlHeaders["Resource"] = strResource;

    // Fetch Query parameters (if any)
    if (0 < strResource.length())
    {
        while (std::getline(oTokenStream, strQuery, '&'))
        {
            if (0 < strQuery.length())
            {
                std::stringstream oParameterEntry(strQuery);
                std::getline(oParameterEntry, strKey, '=');
                std::getline(oParameterEntry, strValue);
                if ((!strKey.empty()) && (!strValue.empty()))
                {
                    m_stlParameters[strKey] = strValue;
                }
            }
        }
    }
}

/********************************************************************************************
 *
 * @class HttpRequestParser
 * @function ExtractHeaders
 * @brief Parse request headers
 * @param[in] c_strRequestData request data
 *
 ********************************************************************************************/

 void __thiscall HttpRequestParser::ExtractHeaders(
    _in const std::string & c_strRequestData
    )
{
    __DebugFunction();

    std::stringstream oTokenStream(c_strRequestData);
    std::string strKey, strValue, strHeaders;

    // Parse request Headers
    while (std::getline(oTokenStream, strHeaders, '\n'))
    {
        if (0 < strHeaders.length())
        {
            std::stringstream oHeaderEntry(strHeaders);
            std::getline(oHeaderEntry, strKey, ':');
            oHeaderEntry >> std::ws;
            std::getline(oHeaderEntry, strValue, '\r');
            if ((!strKey.empty()) && (!strValue.empty()))
            {
                m_stlHeaders[strKey] = strValue;
            }
        }
    }

}
