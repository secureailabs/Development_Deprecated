#include <iostream>
#include <curl/curl.h>
#include <string>
#include <vector>
#include "StructuredBuffer.h"
#include "JsonValue.h"
#include "DebugLibrary.h"
#include "Exceptions.h"

size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::vector<Byte>* s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->resize(newLength + 1);
        s->assign((Byte*)contents, (Byte*)contents + newLength);
       // s->at(newLength) = 0;
    }
    catch (...)
    {
        std::cout << "Something is failiing here \n";
        return newLength;
    }
    return newLength;
}

std::vector<Byte> RestApiCall(
    const std::string& c_strhostIp,
    const Word nPort,
    const std::string& c_strVerb,
    const std::string& c_strApiUri,
    const std::string& c_strJsonBody
)
{
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    std::vector<Byte> stlResponse;
    if (curl)
    {
        // Disbale Diagnostic messages
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        std::string strUrl = "https://" + c_strhostIp + ":" + std::to_string(nPort) + c_strApiUri;
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, c_strVerb.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, c_strJsonBody.c_str());
        // Disbaling the SSL certificate verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stlResponse);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output
        if (0 < c_strJsonBody.length())
        {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: */*");
            headers = curl_slist_append(headers, "Host: localhost:6200");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }

    return stlResponse;
}

StructuredBuffer GetListOfEvents(
    _in const std::string& c_strEncodedEosb,
    _in const std::string& c_strParentGuid,
    _in const std::string& c_strOrganizationGuid,
    _in const unsigned int c_unSequenceNumber
)
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strParentGuid.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());

    StructuredBuffer oResponseStructuredBuffer;

    try
    {
        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \"" + c_strEncodedEosb + "\","
            "\n    \"ParentGuid\": \"" + c_strParentGuid + "\","
            "\n    \"OrganizationGuid\": \"" + c_strOrganizationGuid + "\","
            "\n    \"Filters\":"
            "\n    {"
            "\n         \"SequenceNumber\": " + std::to_string(c_unSequenceNumber) + 
            "\n    }"
            "\n}";

        std::string strHostIp = "localhost";
        Word nPort = 6200;
        std::string strVerb = "GET";
        std::string strApiUri = "/SAIL/AuditLogManager/GetListOfEvents";
        std::vector<Byte> stlRestResponse = ::RestApiCall(strHostIp, nPort, strVerb, strApiUri, strContent);
        std::cout << stlRestResponse.data() << std::endl;
        StructuredBuffer oResponse = JsonValue::ParseDataToStructuredBuffer((const char*)stlRestResponse.data());
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of events.", nullptr);

        StructuredBuffer oListOfEvents(oResponse.GetStructuredBuffer("ListOfEvents"));
    }
    catch (BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage();
    }
    catch (...)
    {
        std::cout << "Error getting list of events.";
    }

    return oResponseStructuredBuffer;
}

int main()
{
    try
    {
        // LOGIN
        std::string strHostIp = "localhost";
        Word nPort = 6200;
        std::string strVerb = "POST";
        std::string strApiUri = "/SAIL/AuthenticationManager/User/Login?Email=overlord@zerg.com&Password=sailpassword";
        std::string strJsonBody = "";
        std::vector<byte> stlRestResponse = ::RestApiCall(strHostIp, nPort, strVerb, strApiUri, strJsonBody);
        StructuredBuffer oStructuredBuffer = JsonValue::ParseDataToStructuredBuffer((const char*)stlRestResponse.data());

        // GET USER BASIC INFORMATION
        std::string strEncodedEosb = ::UnEscapeJsonString(oStructuredBuffer.GetString("Eosb"));
        strVerb = "GET";
        strApiUri = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb=" + strEncodedEosb;
        stlRestResponse = ::RestApiCall(strHostIp, nPort, strVerb, strApiUri, strJsonBody);
        StructuredBuffer oBasicUserInfoStructuredBuffer = JsonValue::ParseDataToStructuredBuffer((const char*)stlRestResponse.data());

        // GET LIST OF EVENTS
        const std::string c_strParentGuid = "{00000000-0000-0000-0000-000000000000}";
        const std::string c_strOrganizationGuid = oBasicUserInfoStructuredBuffer.GetString("OrganizationGuid");
        const unsigned int unSequenceNumber = 0;
        StructuredBuffer oAuditEventsStructuredBuffer = ::GetListOfEvents(strEncodedEosb, c_strParentGuid, c_strOrganizationGuid, 0);
    }
    catch (const std::exception& oException)
    {
        std::cout << oException.what();
    }
    catch (const BaseException & oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage();
    }

    return 0;
}
