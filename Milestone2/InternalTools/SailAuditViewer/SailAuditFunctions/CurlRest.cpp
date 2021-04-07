/*********************************************************************************************
 *
 * @file CurlRest.cpp
 * @author Prawal Gangwar
 * @date 28 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "JsonValue.h"
#include "StructuredBuffer.h"
#include "Utils.h"

#include <curl/curl.h>

#include <iostream>
#include <string>
#include <vector>

/********************************************************************************************
 *
 * @function CurlWriteCallback
 * @brief Callback function used by psCurl to write data to the vector
 * @param[in] c_pszData pointer to the data pointer
 * @return Keys in JsonObject
 *
 ********************************************************************************************/

size_t __stdcall CurlWriteCallback(
    _in const Byte * c_pbAdditionalData,
    _in size_t unSizeOfElements,
    _in size_t unNumberOfElements,
    _in std::vector<Byte> * pstlResponseBuffer
    ) throw()
{
    __DebugFunction();

    unsigned int unNewSizeInBytes = 0;

    try
    {
        if (nullptr != pstlResponseBuffer)
        {
            // Only append additional data if the incoming parameters are valid
            if ((nullptr != c_pbAdditionalData) && (0 < unSizeOfElements) && (0 < unNumberOfElements))
            {
                // Figure out how many additional bytes will be we appending to the existing response buffer
                unsigned int unAdditionalSizeInBytes = (unsigned int) (unSizeOfElements * unNumberOfElements);
                pstlResponseBuffer->insert(pstlResponseBuffer->end(), c_pbAdditionalData, (c_pbAdditionalData + unAdditionalSizeInBytes));                
            }

            // Make sure to return the new size of the response buffer now that new bytes have
            // been appended to it
            unNewSizeInBytes = (unsigned int) pstlResponseBuffer->size();
        }
    }

    catch (...)
    {

    }

    return unNewSizeInBytes;
}


/********************************************************************************************
 *
 * @function RestApiCall
 * @brief Make Rest API calls using psCurl
 * @param[in] c_pszData pointer to the data pointer
 * @return Rest Response
 *
 ********************************************************************************************/

std::vector<Byte> RestApiCall(
    _in const std::string & c_strHostIpAddress,
    _in const Word nPort,
    _in const std::string & c_strVerb,
    _in const std::string & c_strApiUri,
    _in const std::string & c_strJsonBody,
    _in bool fIsJsonResponse
    ) throw()
{
    __DebugFunction();

    std::vector<Byte> stlResponse;

    try
    {
        CURL * psCurl = nullptr;
        ::curl_global_init(CURL_GLOBAL_DEFAULT);
        psCurl = ::curl_easy_init();
        if (nullptr != psCurl)
        {
            CURLcode eCurlResponse;

            // Disable diagnostic messages
            ::curl_easy_setopt(psCurl, CURLOPT_VERBOSE, 0L);
            std::string strUrl = "https://" + c_strHostIpAddress + ":" + std::to_string(nPort) + c_strApiUri;
            ::curl_easy_setopt(psCurl, CURLOPT_CUSTOMREQUEST, c_strVerb.c_str());
            ::curl_easy_setopt(psCurl, CURLOPT_URL, strUrl.c_str());
            ::curl_easy_setopt(psCurl, CURLOPT_POSTFIELDS, c_strJsonBody.c_str());
            // Disbaling the SSL certificate verification
            ::curl_easy_setopt(psCurl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
            ::curl_easy_setopt(psCurl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
            ::curl_easy_setopt(psCurl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
            ::curl_easy_setopt(psCurl, CURLOPT_WRITEDATA, &stlResponse);
            if (0 < c_strJsonBody.length())
            {
                struct curl_slist* headers = NULL;
                headers = ::curl_slist_append(headers, "Content-Type: application/json");
                headers = ::curl_slist_append(headers, "Accept: */*");
                headers = ::curl_slist_append(headers, "Host: localhost:6200");
                ::curl_easy_setopt(psCurl, CURLOPT_HTTPHEADER, headers);
            }

            eCurlResponse = ::curl_easy_perform(psCurl);
            ::curl_easy_cleanup(psCurl);

            if (true == fIsJsonResponse)
            {
                // Append the new incoming bytes to the existing response buffer
                std::string strResponse;
                strResponse.insert(strResponse.end(), stlResponse.begin(), stlResponse.end());
                // Now we need to unescape the string to clean it up
                strResponse = ::UnEscapeJsonString(strResponse);
                // It's possible that the call to RestApiCall returns a JSON buffer that is NOT null terminated.
                // So even though the std::vector is the correct size, because it's not terminated, the JSON
                // string basically has crap at the end. We make sure it's null terminated here
                stlResponse.clear();
                stlResponse.insert(stlResponse.end(), strResponse.begin(), strResponse.end());
                // Because of how std::vectors work, we need to add a null terminator here since JSON is
                // effectively a string and needs to be properly null terminated for some functions to
                // work properly
                stlResponse.insert(stlResponse.end(), 0);
            }
        }
    }

    catch (...)
    {

    }

    return stlResponse;
}
