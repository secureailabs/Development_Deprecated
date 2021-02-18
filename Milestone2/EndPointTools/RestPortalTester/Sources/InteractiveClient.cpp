/*********************************************************************************************
 *
 * @file InteractiveClient.cpp
 * @author Shabana Akhtar Baig
 * @date 03 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ConsoleInputHelperFunctions.h"
#include "Exceptions.h"
#include "PluginDictionary.h"
#include "SmartMemoryAllocator.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "GuiTerminal.h"
#include "Utils.h"
#include "HttpRequestParser.h"
#include "JsonValue.h"

#include <string.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/********************************************************************************************
 *
 * @function ParseFirstLine
 * @brief Parse first line of the response to get the Status
 * @param[in] c_strRequestData response data
 * @return true if parsed successfully
 * @return false otherwise
 *
 ********************************************************************************************/

bool ParseFirstLine(
    _in const std::string & c_strRequestData
    )
{
    __DebugFunction();

    bool fSuccess = false;
    std::string strProtocol, strStatus;
    std::stringstream oFirstLineStream(c_strRequestData);

    // Get transaction status
    std::getline(oFirstLineStream, strProtocol, ' ');
    std::getline(oFirstLineStream, strStatus, ' ');
    if (!strStatus.empty())
    {
        fSuccess = true;
    }
    else
    {
        _ThrowBaseException("ERROR: Invalid request.", nullptr);
    }
    _ThrowBaseExceptionIf(("200" != strStatus), "Transaction returned with error code:", strStatus);

    return fSuccess;
}

/********************************************************************************************
 *
 * @function GetResponseBody
 * @brief Parse and return response body
 * @param[in] c_strRequestData response data
 * @return Serialized response body
 *
 ********************************************************************************************/

std::vector<Byte> GetResponseBody(
    _in const std::string & c_strRequestData,
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();

    std::vector<Byte> stlSerializedResponse;

    // Check http code
    bool fSuccess = ::ParseFirstLine(c_strRequestData);
    // Parse Header of the Rest Request
    HttpRequestParser oParser;
    fSuccess = oParser.ParseResponse(c_strRequestData);
    _ThrowBaseExceptionIf((false == fSuccess), "Error: Parsing response failed.", nullptr);

    if (true == oParser.HeaderExists("Content-Length"))
    {
        unsigned int unContentLength = std::stoi(oParser.GetHeaderValue("Content-Length"));
        if (0 < unContentLength)
        {
            // Read request content
            std::vector<Byte> stlBodyData = poTlsNode->Read(unContentLength, 100);
            _ThrowBaseExceptionIf((0 == stlBodyData.size()), "Dead Packet.", nullptr);
            std::string strRequestBody = std::string(stlBodyData.begin(), stlBodyData.end());

            // Check Content-Type
            _ThrowBaseExceptionIf((false == oParser.HeaderExists("Content-Type")), "Invalid request format.", nullptr);
            std::string strContentType = oParser.GetHeaderValue("Content-Type");
            if ("application/json" == strContentType)
            {
                // Parse Json
                std::string strUnEscapseJsonString = ::UnEscapeJsonString(strRequestBody);
                stlSerializedResponse = JsonValue::ParseDataToStructuredBuffer(strUnEscapseJsonString.c_str());
            }
            else
            {
                _ThrowBaseException("Content Type: %s not supported.", strContentType);
            }
        }
    }
    _ThrowBaseExceptionIf((0 == stlSerializedResponse.size()), "Error logging in.", nullptr);

    return stlSerializedResponse;
}

/********************************************************************************************/

std::string Login(
    _in const std::string & c_strEmail,
    _in const std::string & c_strUserPassword
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strEmail.length());
    __DebugAssert(0 != c_strUserPassword.length());

    std::string strEosb;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        std::string strHttpLoginRequest = "POST /SAIL/AuthenticationManager/User/Login?Email="+ c_strEmail +"&Password="+ c_strUserPassword +" HTTP/1.1\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strHttpLoginRequest.data(), (strHttpLoginRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error logging in.", nullptr);
        strEosb = oResponse.GetString("Eosb");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage("Login Failed!");
    }

    return strEosb;
}

/********************************************************************************************/

std::string GetBasicUserInformation(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    std::string strOrganizationGuid;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        std::string strHttpLoginRequest = "GET /SAIL/AuthenticationManager/GetBasicUserInformation?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strHttpLoginRequest.data(), (strHttpLoginRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error decrypting eosb.", nullptr);
        strOrganizationGuid = oResponse.GetString("OrganizationGuid");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }

    return strOrganizationGuid;
}

/********************************************************************************************/

std::string RegisterRootEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid
    )
{
    __DebugFunction();

    std::string strRootEventGuid = Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces);

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \""+ c_strEncodedEosb +"\","
                                "\n    \"NonLeafEvent\":"
                                "\n    {"
                                "\n        \"EventGuid\": \""+ strRootEventGuid +"\","
                                "\n        \"ParentGuid\": \"{00000000-0000-0000-0000-000000000000}\","
                                "\n        \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n        \"EventType\": 1,"
                                "\n        \"Timestamp\": 12345,"
                                "\n        \"SequenceNumber\": 0,"
                                "\n        \"PlainTextEventData\": "
                                "\n        {"
                                "\n        }"
                                "\n    }"
                                "\n}";

        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/Events HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: "+ std::to_string(strContent.size()) +"\r\n"
                                        "\r\n"
                                        + strContent;

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRegisterRequest.data(), strHttpRegisterRequest.size());

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }

        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        std::cout << "Root event added successfully!" << std::endl;
        std::cout << "Root event guid: " << strRootEventGuid << std::endl;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering root event.");
    }

    ::WaitForUserToContinue();

    return strRootEventGuid;
}

/********************************************************************************************/

bool RegisterBranchEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const std::string & c_strOrganizationGuid,
    _in const std::string & c_strDcGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \""+ c_strEncodedEosb +"\","
                                "\n    \"NonLeafEvent\":"
                                "\n    {"
                                "\n        \"EventGuid\": \""+ Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces) +"\","
                                "\n        \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n        \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n        \"EventType\": 2,"
                                "\n        \"Timestamp\": 12345,"
                                "\n        \"SequenceNumber\": 1,"
                                "\n        \"PlainTextEventData\": "
                                "\n        {"
                                "\n            \"BranchType\": 1,"
                                "\n            \"GuidOfDcOrVm\" : \""+ c_strDcGuid +"\""
                                "\n        }"
                                "\n    }"
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/Events HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: "+ std::to_string(strContent.size()) +"\r\n"
                                        "\r\n"
                                        + strContent;

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRegisterRequest.data(), strHttpRegisterRequest.size());

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }

        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        std::cout << "Branch event added successfully!" << std::endl;
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering branch event.");
    }

    ::WaitForUserToContinue();

    return fSuccess;
}

/********************************************************************************************/

bool RegisterLeafEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid,
    _in const std::string & c_strParentGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \""+ c_strEncodedEosb +"\","
                                "\n    \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n    \"LeafEvents\": ["
                                "\n        {"
                                "\n            \"EventGuid\": \""+ Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces) +"\","
                                "\n            \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n            \"EventType\": 6,"
                                "\n            \"Timestamp\": 12345,"
                                "\n            \"SequenceNumber\": 2,"
                                "\n            \"EncryptedEventData\": \"5iEQAhtloSMplcUnh1L0nooYrO0TKDJZSOAIzOgfaItg8i+EFrmGkDE7SNm6icKgskkBpoMBAABO6L5OOW6aS0pw3aMZjP2Q0KeKL2XtoeVCmW+2sN34h0LuFaPqN48Ku2WytWVHK2t0ilp50Xo7RHxcMPKkiUqSatRD42UjaqcpFCoy3plz+JfogTONcCDiRe+4tRcmg1zHAk2zsXZhwFg5tJioNIQdSoG1bOz4dPYqltRtMYlpbea85IH3pMkB4qduM5OK5zDNCxB0SdlyNpsREhRzeUCxAAiiop7PYgZb/8Vdsd67BTeSd73JkyFr301nqaa5+LCJpnSv19B6yUqQK7ZoSVsTwNsUqO+mtIRTEomvRspqu4hwQf++4I4rIyCwIlN2daJtxNI5RVujFTellgaWB0BudhxqIk72EMEkE9vOihEdaPcJJC2FkEBJTH9Dg3DBNRSck5ZXYmP2MBGAM474iwisvSpfiUSBNNkM3AL6y782K3vrhNDxvY2uxKmR3rfs8TJI9V7lAvIogLht2VYKJi1DWWLtMbccGScTYfyqZgjBw5m7R5LL1CkREZsV6kVymL1kYDkyGlaxKZESbg==\""
                                "\n        },"
                                "\n        {"
                                "\n            \"EventGuid\": \""+ Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces) +"\","
                                "\n            \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n            \"EventType\": 6,"
                                "\n            \"Timestamp\": 12345,"
                                "\n            \"SequenceNumber\": 3,"
                                "\n            \"EncryptedEventData\": \"5iEQAhtloSMplcUnh1L0nooYrO0TKDJZSOAIzOgfaItg8i+EFrmGkDE7SNm6icKgskkBpoMBAABO6L5OOW6aS0pw3aMZjP2Q0KeKL2XtoeVCmW+2sN34h0LuFaPqN48Ku2WytWVHK2t0ilp50Xo7RHxcMPKkiUqSatRD42UjaqcpFCoy3plz+JfogTONcCDiRe+4tRcmg1zHAk2zsXZhwFg5tJioNIQdSoG1bOz4dPYqltRtMYlpbea85IH3pMkB4qduM5OK5zDNCxB0SdlyNpsREhRzeUCxAAiiop7PYgZb/8Vdsd67BTeSd73JkyFr301nqaa5+LCJpnSv19B6yUqQK7ZoSVsTwNsUqO+mtIRTEomvRspqu4hwQf++4I4rIyCwIlN2daJtxNI5RVujFTellgaWB0BudhxqIk72EMEkE9vOihEdaPcJJC2FkEBJTH9Dg3DBNRSck5ZXYmP2MBGAM474iwisvSpfiUSBNNkM3AL6y782K3vrhNDxvY2uxKmR3rfs8TJI9V7lAvIogLht2VYKJi1DWWLtMbccGScTYfyqZgjBw5m7R5LL1CkREZsV6kVymL1kYDkyGlaxKZESbg==\""
                                "\n        }"
                                "\n    ]"
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/LeafEvents HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: "+ std::to_string(strContent.size()) +"\r\n"
                                        "\r\n"
                                        + strContent;

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRegisterRequest.data(), (strHttpRegisterRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }

        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        std::cout << "Leaf events added successfully!" << std::endl;
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering leaf events.");
    }

    ::WaitForUserToContinue();

    return fSuccess;
}

/********************************************************************************************/

std::string RegisterVirtualMachine(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    std::string strVmEventGuid;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \""+ c_strEncodedEosb +"\","
                                "\n    \"DCGuid\": \""+ c_strDcGuid +"\","
                                "\n    \"VMGuid\": \""+ c_strVmGuid +"\""
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/RegisterVM HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: "+ std::to_string(strContent.size()) +"\r\n"
                                        "\r\n"
                                        + strContent;

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRegisterRequest.data(), (strHttpRegisterRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }

        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        strVmEventGuid = oResponse.GetString("VmEventGuid");
        std::cout << "Vm registered successfully!" << std::endl;
        std::cout << "Vm event guid: " << strVmEventGuid << std::endl;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering virtual machine.");
    }

    ::WaitForUserToContinue();

    return strVmEventGuid;
}

/********************************************************************************************/

bool GetListOfEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const std::string & c_strOrganizationGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

        // Create rest request
        std::string strContent = "{\n    \"Eosb\": \""+ c_strEncodedEosb +"\","
                                "\n    \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n    \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n    \"Filters\":"
                                "\n    {"
                                "\n    }"
                                "\n}";
        std::string strHttpRequest = "GET /SAIL/AuditLogManager/GetListOfEvents HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: "+ std::to_string(strContent.size()) +"\r\n"
                                        "\r\n"
                                        + strContent;

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRequest.data(), (strHttpRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }

        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error logging in.", nullptr);
        std::cout << "List of Events:" << std::endl;
        for (std::string strEventUuid : oResponse.GetStructuredBuffer("ListOfEvents").GetNamesOfElements())
        {
            std::cout << strEventUuid << std::endl;
        }
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error getting list of events.");
    }

    ::WaitForUserToContinue();

    return fSuccess;
}

/********************************************************************************************/


int main()
{
    __DebugFunction();

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    try
    {
        ::ClearScreen();

        std::cout << "************************\n  SAIL LOGIN\n************************\n" << std::endl;
        std::string strEmail = ::GetStringInput("Enter email: ", 50, false, c_szValidInputCharacters);
        std::string strUserPassword = ::GetStringInput("Enter password: ", 50, true, c_szValidInputCharacters);

        std::string strEncodedEosb = Login(strEmail, strUserPassword);

        _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);

        // Get organization guid from eosb
        std::string strOrganizationGuid = ::GetBasicUserInformation(strEncodedEosb);
        // Hardcoded digital contract guid, and vm guid
        std::string strDcGuid = "{33DB1751-66AE-4EB5-BF7B-614CBC09BC4C}";
        std::string strVmGuid = Guid(eVirtualMachine).ToString(eHyphensAndCurlyBraces);

        // Get RootEventGuid after adding a root event
        std::string strRootEventGuid;
        // Get VmEventGuid after registering a VM and use it to get list of events
        std::string strVmEventGuid;

        bool fTerminatedSignalEncountered = false;

        while (false == fTerminatedSignalEncountered)
        {
            ::ShowTopMenu();

            std::string strUserInput = ::GetStringInput("Selection: ", 1, false, c_szValidInputCharacters);

            switch (stoi(strUserInput))
            {
                case 1:
                {
                    strRootEventGuid = ::RegisterRootEvent(strEncodedEosb, strOrganizationGuid);
                break;
               }
                case 2:
                {
                    if (0 < strRootEventGuid.size())
                    {
                        ::RegisterBranchEvent(strEncodedEosb, strRootEventGuid, strOrganizationGuid, strDcGuid);
                    }
                    else 
                    {
                        std::cout << "Register a root event first!" << std::endl;
                        ::WaitForUserToContinue();
                    }
                break;
                }
                case 3:
                {
                    if (0 < strVmEventGuid.size())
                    {
                        ::RegisterLeafEvents(strEncodedEosb, strOrganizationGuid, strVmEventGuid);
                    }
                    else 
                    {
                        std::cout << "Register a Vm event first!" << std::endl;
                        ::WaitForUserToContinue();
                    }
                break;
                }
                case 4:
                {
                    strVmEventGuid = ::RegisterVirtualMachine(strEncodedEosb, strDcGuid, strVmGuid);
                break;
                }
                case 5:
                {
                    std::string strParentGuid = ::GetStringInput("Enter hyphen and curly braces formatted parent guid: ", 38, true, c_szValidInputCharacters);
                    ::GetListOfEvents(strEncodedEosb, strParentGuid, strOrganizationGuid);
                break;
                }
                case 6:
                {
                    if (0 < strVmEventGuid.size())
                    {
                        ::GetListOfEvents(strEncodedEosb, strVmEventGuid, strOrganizationGuid);
                    }
                    else 
                    {
                        std::cout << "Register a Vm event first" << std::endl;
                        ::WaitForUserToContinue();
                    }
                break;
                }
                case 0:
                {
                    fTerminatedSignalEncountered = true;
                break;
                }
                default:
                {
                    std::cout << "Invalid option. Usage: [1-6]" << std::endl;
                break;
                }
            }
        }
    }
    catch(BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    
    return 0;
}
