/*********************************************************************************************
 *
 * @file InteractiveClient.cpp
 * @author Shabana Akhtar Baig
 * @date 03 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "InteractiveClient.h"
#include "Base64Encoder.h"

#include <openssl/rand.h>

const char * g_szServerIpAddress;
unsigned int g_unPortNumber;

/********************************************************************************************/

void AddWebPortalConfiguration(
    _in const char * c_szIpAddress, 
    _in unsigned int unPortNumber
    )
{
    __DebugFunction();

    g_szServerIpAddress = c_szIpAddress;  // define the global variables
    g_unPortNumber = unPortNumber;
}

/********************************************************************************************/

static std::string __stdcall _GetEpochTimeInMilliseconds(void)
{
    __DebugFunction();
    
    std::string strEpochTimeInMilliseconds;
    uint64_t un64EpochTimeInMilliseconds = ::GetEpochTimeInMilliseconds();
    char szString[64];
    
    ::sprintf(szString, "%ld", un64EpochTimeInMilliseconds);
    strEpochTimeInMilliseconds = szString;
    
    return strEpochTimeInMilliseconds;
}

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
    _ThrowBaseExceptionIf(("200" != strStatus), "Transaction returned with error code.", nullptr);

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
            std::vector<Byte> stlBodyData = poTlsNode->Read(unContentLength, 2000);
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
    __DebugAssert(0 < c_strEmail.size());
    __DebugAssert(0 < c_strUserPassword.size());

    std::string strEosb;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 20000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
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

std::vector<Byte> GetBasicUserInformation(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    StructuredBuffer oUserInformation;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        _ThrowBaseExceptionIf((0 == stlSerializedResponse.size()), "Dead Packet.", nullptr);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error decrypting eosb.", nullptr);
        oUserInformation.PutString("OrganizationGuid", oResponse.GetString("OrganizationGuid"));
        oUserInformation.PutString("UserGuid", oResponse.GetString("UserGuid"));
        oUserInformation.PutQword("AccessRights", (Qword) oResponse.GetFloat64("AccessRights"));
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }

    return oUserInformation.GetSerializedBuffer();
}

/********************************************************************************************/

std::string GetIEosb(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

   std::string strIEosb;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        std::string strRequest = "GET /SAIL/CryptographicManager/User/GetIEosb?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strRequest.data(), (strRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {   
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        _ThrowBaseExceptionIf((0 == stlSerializedResponse.size()), "Dead Packet.", nullptr);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error gettign imposter eosb.", nullptr);
        strIEosb = oResponse.GetString("UpdatedEosb");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }

    return strIEosb;
}

/********************************************************************************************/

std::string RegisterRootEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());

    std::string strRootEventGuid = Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces);

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"NonLeafEvent\":"
                                "\n    {"
                                "\n        \"EventGuid\": \""+ strRootEventGuid +"\","
                                "\n        \"ParentGuid\": \"{00000000-0000-0000-0000-000000000000}\","
                                "\n        \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n        \"EventType\": 1,"
                                "\n        \"Timestamp\": "+ ::_GetEpochTimeInMilliseconds() +","
                                "\n        \"PlainTextEventData\": "
                                "\n        {"
                                "\n        }"
                                "\n    }"
                                "\n}";

        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/Events?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
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
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strParentGuid.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());
    __DebugAssert(0 < c_strDcGuid.size());

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"NonLeafEvent\":"
                                "\n    {"
                                "\n        \"EventGuid\": \""+ Guid(eAuditEventBranchNode).ToString(eHyphensAndCurlyBraces) +"\","
                                "\n        \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n        \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n        \"EventType\": 2,"
                                "\n        \"Timestamp\": "+ ::_GetEpochTimeInMilliseconds() +","
                                "\n        \"PlainTextEventData\": "
                                "\n        {"
                                "\n            \"BranchType\": 1,"
                                "\n            \"GuidOfDcOrVm\" : \""+ c_strDcGuid +"\""
                                "\n        }"
                                "\n    }"
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/Events?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
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
    _in const std::string & c_strParentGuid
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strParentGuid.size());

    bool fSuccess = false;

    StructuredBuffer oLeafEvents;
    // Add first leaf event
    StructuredBuffer oEvent1;
    oEvent1.PutString("EventGuid", Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces));
    oEvent1.PutQword("EventType", 6);
    oEvent1.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    StructuredBuffer oEncryptedEventData;
    oEncryptedEventData.PutString("EventName", "VMAdded");
    oEncryptedEventData.PutByte("EventType", 1);
    StructuredBuffer oEventData;
    oEventData.PutUnsignedInt64("VersionNumber", 0x0000000100000001);
    oEventData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    oEncryptedEventData.PutStructuredBuffer("EventData", oEventData);
    oEvent1.PutString("EncryptedEventData", ::Base64Encode(oEncryptedEventData.GetSerializedBufferRawDataPtr(), oEncryptedEventData.GetSerializedBufferRawDataSizeInBytes()));
    oLeafEvents.PutStructuredBuffer("0", oEvent1);
    // Add second leaf event
    StructuredBuffer oEvent2;
    oEvent2.PutString("EventGuid", Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces));
    oEvent2.PutQword("EventType", 6);
    oEvent2.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    oEncryptedEventData.PutString("EventName", "VMDeleted");
    oEncryptedEventData.PutByte("EventType", 2);
    oEventData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    oEncryptedEventData.PutStructuredBuffer("EventData", oEventData);
    oEvent2.PutString("EncryptedEventData", ::Base64Encode(oEncryptedEventData.GetSerializedBufferRawDataPtr(), oEncryptedEventData.GetSerializedBufferRawDataSizeInBytes()));
    oLeafEvents.PutStructuredBuffer("1", oEvent2);

    fSuccess = ::RegisterLeafEvents(c_strEncodedEosb, c_strParentGuid, oLeafEvents);

    return fSuccess;
}

/********************************************************************************************/

bool RegisterLeafEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const StructuredBuffer & c_oLeafEvents
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strParentGuid.size());

    bool fSuccess = false;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n    \"LeafEvents\": [";

        // Add leaf events to the rest request body
        std::vector<std::string> stlEvents = c_oLeafEvents.GetNamesOfElements();
        unsigned int unNumberOfEvents = stlEvents.size();
        for (unsigned int unIndex = 0; unIndex < unNumberOfEvents; ++unIndex)
        {
            StructuredBuffer oEvent(c_oLeafEvents.GetStructuredBuffer(stlEvents.at(unIndex).c_str()));
            strContent += "\n        {"
                        "\n            \"EventGuid\": \""+ oEvent.GetString("EventGuid") +"\","
                        "\n            \"EventType\": "+ std::to_string(oEvent.GetQword("EventType")) +","
                        "\n            \"Timestamp\": "+ std::to_string(oEvent.GetUnsignedInt64("Timestamp")) +","
                        "\n            \"SequenceNumber\": "+ stlEvents.at(unIndex) +","
                        "\n            \"EncryptedEventData\": \""+ oEvent.GetString("EncryptedEventData") +"\"";
            if ((unNumberOfEvents - 1) != unIndex)
            {   
                strContent += "\n        },";
            }
            else 
            {
                strContent += "\n        }";
            }
        }
        
        strContent += "\n    ]"
                    "\n}";

        std::string strHttpRegisterRequest = "POST /SAIL/AuditLogManager/LeafEvents?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 5000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
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

    return fSuccess;
}

/********************************************************************************************/

std::string RegisterVirtualMachine(
    _in const std::string & c_strEncodedIEosb,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedIEosb.size());
    __DebugAssert(0 < c_strVmGuid.size());

    std::string strVmEosb;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Register Virtual Machine \n************************\n" << std::endl;
    std::string strDcGuid = ::GetStringInput("Enter hyphen and curly braces formatted digital contract guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(38 == strDcGuid.size());

    StructuredBuffer oVmInformation;
    oVmInformation.PutString("DigitalContractGuid", strDcGuid);
    oVmInformation.PutString("IPAddress", "127.0.0.1");

    strVmEosb = ::RegisterVirtualMachine(c_strEncodedIEosb, c_strVmGuid, oVmInformation);

    return strVmEosb;
}

/********************************************************************************************/

std::string RegisterVirtualMachine(
    _in const std::string & c_strEncodedIEosb,
    _in const std::string & c_strVmGuid,
    _in const StructuredBuffer & c_oVmInformation
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedIEosb.size());
    __DebugAssert(0 < c_strVmGuid.size());

    std::string strVmEosb;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n   \"DigitalContractGuid\": \""+ c_oVmInformation.GetString("DigitalContractGuid") +"\","
                                "\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\","
                                "\n    \"HeartbeatBroadcastTime\": "+ std::to_string(::GetEpochTimeInSeconds()) +","
                                "\n    \"IPAddress\": \""+ c_oVmInformation.GetString("IPAddress") +"\""
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/RegisterVM?IEosb="+ c_strEncodedIEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        strVmEosb = oResponse.GetString("VmEosb");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering virtual machine.");
    }

    return strVmEosb;
}

/********************************************************************************************/

std::string RegisterVmAfterDataUpload(
    _in const std::string & c_strEncodedVmEosb,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedVmEosb.size());
    __DebugAssert(0 < c_strVmGuid.size());

    std::string strVmEventGuid;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\""
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/DataOwner/RegisterVM?Eosb="+ c_strEncodedVmEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        strVmEventGuid = oResponse.GetString("VmEventGuid");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering virtual machine.");
    }

    return strVmEventGuid;
}

/********************************************************************************************/

std::string RegisterVmForComputation(
    _in const std::string & c_strEncodedVmEosb,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedVmEosb.size());
    __DebugAssert(0 < c_strVmGuid.size());

    std::string strVmEventGuid;

    try
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\""
                                "\n}";
        std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/Researcher/RegisterVM?Eosb="+ c_strEncodedVmEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
        strVmEventGuid = oResponse.GetString("VmEventGuid");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering virtual machine.");
    }

    return strVmEventGuid;
}

/********************************************************************************************/

bool GetListOfEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const std::string & c_strOrganizationGuid,
    _in unsigned int unIndentDepth
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strParentGuid.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n    \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n    \"Filters\":"
                                "\n    {"
                                "\n         \"SequenceNumber\": 0"
                                "\n    }"
                                "\n}";
        std::string strHttpRequest = "GET /SAIL/AuditLogManager/GetListOfEvents?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of events.", nullptr);
        StructuredBuffer oListOfEvents(oResponse.GetStructuredBuffer("ListOfEvents"));
        std::string strIndentString((unIndentDepth++) * 4, ' ');
        for (std::string strSequenceNumber : oListOfEvents.GetNamesOfElements())
        {
            StructuredBuffer oEvent(oListOfEvents.GetStructuredBuffer(strSequenceNumber.c_str()));
            StructuredBuffer oEventObject(oEvent.GetStructuredBuffer("ObjectBlob"));
            std::string strEventUuid = oEvent.GetString("EventGuid");
            std::cout << strIndentString << "EventGuid: " << strEventUuid << std::endl;
            std::cout << strIndentString << "ParentGuid: " << oEventObject.GetString("ParentGuid") << std::endl;
            std::cout << strIndentString << "OrganizationGuid: " << oEvent.GetString("OrganizationGuid") << std::endl;
            std::cout << strIndentString << "Timestamp: " << oEventObject.GetFloat64("Timestamp") << std::endl;
            std::cout << strIndentString << "Sequence Number: " << strSequenceNumber << std::endl;
            ::GetListOfEvents(c_strEncodedEosb, strEventUuid, c_strOrganizationGuid, unIndentDepth);
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

    return fSuccess;
}

/********************************************************************************************/

bool RegisterOrganizationAndSuperUser(void)
{
    __DebugFunction();

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get user and organization information
    std::cout << "************************\n New User Information \n************************\n" << std::endl;
    std::string strEmail = ::GetStringInput("Enter your email: ", 50, false, c_szValidInputCharacters);
    std::string strPassword = ::GetStringInput("Enter your new password: ", 50, true, c_szValidInputCharacters);
    std::string strName = ::GetStringInput("Enter your full name: ", 50, false, c_szValidInputCharacters);
    std::string strPhoneNumber = ::GetStringInput("Enter your phone number: ", 12, false, c_szValidInputCharacters);
    std::string strTitle = ::GetStringInput("Enter your title within your organization: ", 50, false, c_szValidInputCharacters);
    std::cout << "************************\n  New Organization Information \n************************\n" << std::endl;
    std::string strOrganizationName = ::GetStringInput("Enter your organization name: ", 50, false, c_szValidInputCharacters);
    std::string strOrganizationAddress = ::GetStringInput("Enter your organization address: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactName = ::GetStringInput("Enter primary contact name for the organization: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactTitle = ::GetStringInput("Enter primary contact title within the organization: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactEmail = ::GetStringInput("Enter primary contact email: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactPhoneNumber = ::GetStringInput("Enter primary contact phone number: ", 12, false, c_szValidInputCharacters);
    std::string strSecondaryContactName = ::GetStringInput("Enter secondary contact name for the organization: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactTitle= ::GetStringInput("Enter secondary contact title within the organization: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactEmail = ::GetStringInput("Enter secondary contact email: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactPhoneNumber = ::GetStringInput("Enter secondary contact phone number: ", 12, false, c_szValidInputCharacters);

    __DebugAssert(0 < strEmail.size());
    __DebugAssert(0 < strPassword.size());
    __DebugAssert(0 < strName.size());
    __DebugAssert(0 < strPhoneNumber.size());
    __DebugAssert(0 < strTitle.size());
    __DebugAssert(0 < strOrganizationName.size());
    __DebugAssert(0 < strOrganizationAddress.size());
    __DebugAssert(0 < strPrimaryContactName.size());
    __DebugAssert(0 < strPrimaryContactTitle.size());
    __DebugAssert(0 < strPrimaryContactEmail.size());
    __DebugAssert(0 < strPrimaryContactPhoneNumber.size());
    __DebugAssert(0 < strSecondaryContactName.size());
    __DebugAssert(0 < strSecondaryContactTitle.size());
    __DebugAssert(0 < strSecondaryContactEmail.size());
    __DebugAssert(0 < strSecondaryContactPhoneNumber.size());

    StructuredBuffer oOrganizationInformation;
    oOrganizationInformation.PutString("Email", strEmail);
    oOrganizationInformation.PutString("Password", strPassword);
    oOrganizationInformation.PutString("Name", strName);
    oOrganizationInformation.PutString("PhoneNumber", strPhoneNumber);
    oOrganizationInformation.PutString("Title", strTitle);
    oOrganizationInformation.PutString("OrganizationName", strOrganizationName);
    oOrganizationInformation.PutString("OrganizationAddress", strOrganizationAddress);
    oOrganizationInformation.PutString("PrimaryContactName", strPrimaryContactName);
    oOrganizationInformation.PutString("PrimaryContactTitle", strPrimaryContactTitle);
    oOrganizationInformation.PutString("PrimaryContactEmail", strPrimaryContactEmail);
    oOrganizationInformation.PutString("PrimaryContactPhoneNumber", strPrimaryContactPhoneNumber);
    oOrganizationInformation.PutString("SecondaryContactName", strSecondaryContactName);
    oOrganizationInformation.PutString("SecondaryContactTitle", strSecondaryContactTitle);
    oOrganizationInformation.PutString("SecondaryContactEmail", strSecondaryContactEmail);
    oOrganizationInformation.PutString("SecondaryContactPhoneNumber", strSecondaryContactPhoneNumber);

    fSuccess = ::RegisterOrganizationAndSuperUser(oOrganizationInformation);

    return fSuccess;
}

/********************************************************************************************/

bool RegisterOrganizationAndSuperUser(
    _in const StructuredBuffer & c_oOrganizationInformation
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"Email\": \""+ c_oOrganizationInformation.GetString("Email") +"\","
                                "\n    \"Password\": \""+ c_oOrganizationInformation.GetString("Password") +"\","
                                "\n    \"Name\": \""+ c_oOrganizationInformation.GetString("Name") +"\","
                                "\n    \"PhoneNumber\": \""+ c_oOrganizationInformation.GetString("PhoneNumber") +"\","
                                "\n    \"Title\": \""+ c_oOrganizationInformation.GetString("Title") +"\","
                                "\n    \"OrganizationName\": \""+ c_oOrganizationInformation.GetString("OrganizationName") +"\","
                                "\n    \"OrganizationAddress\": \""+ c_oOrganizationInformation.GetString("OrganizationAddress") +"\","
                                "\n    \"PrimaryContactName\": \""+ c_oOrganizationInformation.GetString("PrimaryContactName") +"\","
                                "\n    \"PrimaryContactTitle\": \""+ c_oOrganizationInformation.GetString("PrimaryContactTitle") +"\","
                                "\n    \"PrimaryContactEmail\": \""+ c_oOrganizationInformation.GetString("PrimaryContactEmail") +"\","
                                "\n    \"PrimaryContactPhoneNumber\": \""+ c_oOrganizationInformation.GetString("PrimaryContactPhoneNumber") +"\","
                                "\n    \"SecondaryContactName\": \""+ c_oOrganizationInformation.GetString("SecondaryContactName") +"\","
                                "\n    \"SecondaryContactTitle\": \""+ c_oOrganizationInformation.GetString("SecondaryContactTitle") +"\","
                                "\n    \"SecondaryContactEmail\": \""+ c_oOrganizationInformation.GetString("SecondaryContactEmail") +"\","
                                "\n    \"SecondaryContactPhoneNumber\": \""+ c_oOrganizationInformation.GetString("SecondaryContactPhoneNumber") +"\""
                                "\n}";
        std::string strHttpRequest = "POST /SAIL/AccountManager/RegisterUser HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new organization and super user.", nullptr);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("RootEventStatus")), "Error registering root event for the organization.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering new organization and super user.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool RegisterUser(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid
    )
{
    __DebugFunction();

    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get user information
    std::cout << "************************\n New User Information \n************************\n" << std::endl;
    std::string strEmail = ::GetStringInput("Enter email: ", 50, false, c_szValidInputCharacters);
    std::string strPassword = ::GetStringInput("Enter new password: ", 50, true, c_szValidInputCharacters);
    std::string strName = ::GetStringInput("Enter full name: ", 50, false, c_szValidInputCharacters);
    std::string strPhoneNumber = ::GetStringInput("Enter phone number: ", 12, false, c_szValidInputCharacters);
    std::string strTitle = ::GetStringInput("Enter title within the organization: ", 50, false, c_szValidInputCharacters);
    Qword qwAccessRights = std::stoull(::GetStringInput("Enter user's access rights: ", 2, false, c_szValidInputCharacters));

    __DebugAssert(0 < strEmail.size());
    __DebugAssert(0 < strPassword.size());
    __DebugAssert(0 < strName.size());
    __DebugAssert(0 < strPhoneNumber.size());
    __DebugAssert(0 < strTitle.size());

    StructuredBuffer oUserInformation;
    oUserInformation.PutString("Email", strEmail);
    oUserInformation.PutString("Password", strPassword);
    oUserInformation.PutString("Name", strName);
    oUserInformation.PutString("PhoneNumber", strPhoneNumber);
    oUserInformation.PutString("Title", strTitle);
    oUserInformation.PutQword("AccessRights", qwAccessRights);

    fSuccess = ::RegisterUser(c_strEncodedEosb, c_strOrganizationGuid, oUserInformation);

    return fSuccess;
}

/********************************************************************************************/

bool RegisterUser(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid,
    _in const StructuredBuffer & c_oUserInformation
    )
{
    __DebugFunction();

    __DebugAssert(0 < c_strEncodedEosb.size());
    __DebugAssert(0 < c_strOrganizationGuid.size());

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n   \"Email\": \""+ c_oUserInformation.GetString("Email") +"\","
                                "\n    \"Password\": \""+ c_oUserInformation.GetString("Password") +"\","
                                "\n    \"Name\": \""+ c_oUserInformation.GetString("Name") +"\","
                                "\n    \"PhoneNumber\": \""+ c_oUserInformation.GetString("PhoneNumber") +"\","
                                "\n    \"Title\": \""+ c_oUserInformation.GetString("Title") +"\","
                                "\n    \"AccessRights\": "+ std::to_string(c_oUserInformation.GetQword("AccessRights")) +","
                                "\n    \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\""
                                "\n}";
        std::string strHttpRequest = "POST /SAIL/AccountManager/Admin/RegisterUser?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new user.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering new user.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool UpdateOrganizationInformation(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get organization information
    std::cout << "************************\n Update Organization Information \n************************\n" << std::endl;
    std::string strOrganizationGuid = ::GetStringInput("Enter hyphen and curly braces formatted organization guid: ", 38, true, c_szValidInputCharacters);
    std::string strOrganizationName = ::GetStringInput("Enter organization name: ", 50, false, c_szValidInputCharacters);
    std::string strOrganizationAddress = ::GetStringInput("Enter organization address: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactName = ::GetStringInput("Enter primary contact name for the organization: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactTitle = ::GetStringInput("Enter primary contact title within the organization: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactEmail = ::GetStringInput("Enter primary contact email: ", 50, false, c_szValidInputCharacters);
    std::string strPrimaryContactPhoneNumber = ::GetStringInput("Enter primary contact phone number: ", 12, false, c_szValidInputCharacters);
    std::string strSecondaryContactName = ::GetStringInput("Enter secondary contact name for the organization: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactTitle= ::GetStringInput("Enter secondary contact title within the organization: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactEmail = ::GetStringInput("Enter secondary contact email: ", 50, false, c_szValidInputCharacters);
    std::string strSecondaryContactPhoneNumber = ::GetStringInput("Enter secondary contact phone number: ", 12, false, c_szValidInputCharacters);

    __DebugAssert(38 == strOrganizationGuid.size())
    __DebugAssert(0 < strOrganizationName.size());
    __DebugAssert(0 < strOrganizationAddress.size());
    __DebugAssert(0 < strPrimaryContactName.size());
    __DebugAssert(0 < strPrimaryContactTitle.size());
    __DebugAssert(0 < strPrimaryContactEmail.size());
    __DebugAssert(0 < strPrimaryContactPhoneNumber.size());
    __DebugAssert(0 < strSecondaryContactName.size());
    __DebugAssert(0 < strSecondaryContactTitle.size());
    __DebugAssert(0 < strSecondaryContactEmail.size());
    __DebugAssert(0 < strSecondaryContactPhoneNumber.size());

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"OrganizationGuid\": \""+ strOrganizationGuid +"\","
                                "\n    \"OrganizationInformation\": "
                                "\n    {"
                                "\n        \"OrganizationName\": \""+ strOrganizationName +"\","
                                "\n        \"OrganizationAddress\" : \""+ strOrganizationAddress +"\","
                                "\n        \"PrimaryContactName\": \""+ strPrimaryContactName +"\","
                                "\n        \"PrimaryContactTitle\" : \""+ strPrimaryContactTitle +"\","
                                "\n        \"PrimaryContactEmail\": \""+ strPrimaryContactEmail +"\","
                                "\n        \"PrimaryContactPhoneNumber\" : \""+ strPrimaryContactPhoneNumber +"\","
                                "\n        \"SecondaryContactName\": \""+ strSecondaryContactName +"\","
                                "\n        \"SecondaryContactTitle\" : \""+ strSecondaryContactTitle +"\","
                                "\n        \"SecondaryContactEmail\": \""+ strSecondaryContactEmail +"\","
                                "\n        \"SecondaryContactPhoneNumber\" : \""+ strSecondaryContactPhoneNumber +"\""
                                "\n     }"
                                "\n}";
        std::string strHttpRequest = "PUT /SAIL/AccountManager/Update/Organization?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error updating organization information.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error updating organization information.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool ListOrganizations(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strHttpRequest = "GET /SAIL/AccountManager/Organizations?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRequest.data(), (strHttpRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {   
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of organizations.", nullptr);
        fSuccess = true;
        std::cout << "************************\n List of Organizations \n************************\n" << std::endl;
        StructuredBuffer oOrganizations(oResponse.GetStructuredBuffer("Organizations"));
        for (std::string strElement : oOrganizations.GetNamesOfElements())
        {;
            StructuredBuffer oElement(oOrganizations.GetStructuredBuffer(strElement.c_str()));
            std::cout << "Organization guid: " << strElement << std::endl;
            std::cout << "Organization name: " << oElement.GetString("OrganizationName") << std::endl;
            std::cout << "Organization address: " << oElement.GetString("OrganizationAddress") << std::endl;
            std::cout << "Primary contact name: " << oElement.GetString("PrimaryContactName") << std::endl;
            std::cout << "Primary contact title: " << oElement.GetString("PrimaryContactTitle") << std::endl;
            std::cout << "Primary contact email: " << oElement.GetString("PrimaryContactEmail") << std::endl;
            std::cout << "Primary contact phone #: " << oElement.GetString("PrimaryContactPhoneNumber") << std::endl;
            std::cout << "Secondary contact name: " << oElement.GetString("SecondaryContactName") << std::endl;
            std::cout << "Secondary contact title: " << oElement.GetString("SecondaryContactTitle") << std::endl;
            std::cout << "Secondary contact email: " << oElement.GetString("SecondaryContactEmail") << std::endl;
            std::cout << "Secondary contact phone #: " << oElement.GetString("SecondaryContactPhoneNumber") << std::endl;
            std::cout << "------------------------------------------------------" << std::endl;
        }
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error getting list of organizations.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool DeleteUser(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get user guid
    std::cout << "************************\n Delete User \n************************\n" << std::endl;
    std::string strUserGuid = ::GetStringInput("Enter hyphen and curly braces formatted user guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(38 == strUserGuid.size())

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n   \"UserGuid\": \""+ strUserGuid +"\","
                                "\n    \"IsHardDelete\": true"
                                "\n}";
        std::string strHttpRequest = "DELETE /SAIL/AccountManager/Remove/User?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error deleting user.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error deleting user.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool DeleteOrganization(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get organization guid
    std::cout << "************************\n Delete Organization \n************************\n" << std::endl;
    std::string strOrganizationGuid = ::GetStringInput("Enter hyphen and curly braces formatted organization guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(38 == strOrganizationGuid.size())

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n   \"OrganizationGuid\": \""+ strOrganizationGuid +"\","
                                "\n    \"IsHardDelete\": true"
                                "\n}";
        std::string strHttpRequest = "DELETE /SAIL/AccountManager/Remove/Organization?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error deleting organization.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error deleting organization.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool RegisterDigitalContract(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Register Digital Contract \n************************\n" << std::endl;
    std::string strTitle = ::GetStringInput("Enter digital contract title: ", 50, false, c_szValidInputCharacters);
    std::string strDooGuid = ::GetStringInput("Enter hyphen and curly braces formatted data owner organization guid: ", 38, false, c_szValidInputCharacters);
    std::string strVersionNumber = "0x0000000100000001";
    uint64_t unSubscriptionDays = std::stoull(::GetStringInput("Enter your requested subscription period (in days): ", 50, false, c_szValidInputCharacters));
    std::string strDatasetGuid = Guid(eDataset).ToString(eHyphensAndCurlyBraces);
    std::string strLegalAgreement = ::GetStringInput("Enter the legal agreement: ", 200, false, c_szValidInputCharacters);

    __DebugAssert(38 == strDooGuid.size());
    __DebugAssert(0 < unSubscriptionDays);
    __DebugAssert(0 < strLegalAgreement.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("Title", strTitle);
    oDcInformation.PutString("DOOGuid", strDooGuid);
    oDcInformation.PutUnsignedInt64("SubscriptionDays", unSubscriptionDays);
    oDcInformation.PutString("LegalAgreement", strLegalAgreement);

    fSuccess = ::RegisterDigitalContract(c_strEncodedEosb, oDcInformation);

    return fSuccess;
}

/********************************************************************************************/

bool RegisterDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const StructuredBuffer & c_oDcInformation
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;
    // TODO: change hard coded values
    std::string strVersionNumber = "0x0000000100000001";
    std::string strDatasetGuid = Guid(eDataset).ToString(eHyphensAndCurlyBraces);

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n   \"DataOwnerOrganization\": \""+ c_oDcInformation.GetString("DOOGuid") +"\","
                                "\n    \"Title\": \""+ c_oDcInformation.GetString("Title") +"\","
                                "\n    \"VersionNumber\": \""+ strVersionNumber +"\","
                                "\n    \"SubscriptionDays\": "+ std::to_string(c_oDcInformation.GetUnsignedInt64("SubscriptionDays")) +","
                                "\n    \"DatasetGuid\": \""+ strDatasetGuid +"\","
                                "\n    \"LegalAgreement\": \""+ c_oDcInformation.GetString("LegalAgreement") +"\","
                                "\n    \"DatasetDRMMetadataSize\": "+ std::to_string(0) +","
                                "\n    \"DatasetDRMMetadata\":{}"
                                "\n}";
        std::string strHttpRequest = "POST /SAIL/DigitalContractManager/Applications?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new digital contract.", nullptr);
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error registering new digital contract.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool AcceptDigitalContract(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Approve Digital Contract \n************************\n" << std::endl;
    std::string strDcGuid = ::GetStringInput("Enter hyphen and curly braces formatted digital contract guid: ", 38, true, c_szValidInputCharacters);
    uint64_t unRetentionTime = std::stoull(::GetStringInput("Enter the retention time: ", 50, false, c_szValidInputCharacters));
    std::string strEula = ::GetStringInput("Enter the accepted EULA: ", 200, false, c_szValidInputCharacters);
    std::string strLegalAgreement = ::GetStringInput("Enter the legal agreement: ", 200, false, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());
    __DebugAssert(0 < unRetentionTime);
    __DebugAssert(0 < strEula.size());
    __DebugAssert(0 < strLegalAgreement.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DigitalContractGuid", strDcGuid);
    oDcInformation.PutUnsignedInt64("RetentionTime", unRetentionTime);
    oDcInformation.PutString("EULA", strEula);
    oDcInformation.PutString("LegalAgreement", strLegalAgreement);

    fSuccess = ::AcceptDigitalContract(c_strEncodedEosb, oDcInformation);

    return fSuccess;
}

/********************************************************************************************/

bool AcceptDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const StructuredBuffer & c_oDcInformation
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDcInformation.GetString("DigitalContractGuid") +"\","
                                "\n    \"RetentionTime\": "+ std::to_string(c_oDcInformation.GetUnsignedInt64("RetentionTime")) +","
                                "\n    \"EULA\": \""+ c_oDcInformation.GetString("EULA") +"\","
                                "\n    \"LegalAgreement\": \""+ c_oDcInformation.GetString("LegalAgreement") +"\""
                                "\n}";
        std::string strHttpRequest = "PATCH /SAIL/DigitalContractManager/DataOwner/Accept?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error approving the digital contract.", nullptr);
        std::cout << "Instructions: " << oResponse.GetString("Instructions") << std::endl;
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error approving the digital contract.");
    }

    return fSuccess;
}

/********************************************************************************************/

bool ActivateDigitalContract(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Activate Digital Contract \n************************\n" << std::endl;
    std::string strDcGuid = ::GetStringInput("Enter hyphen and curly braces formatted digital contract guid: ", 38, true, c_szValidInputCharacters);
    std::string strEula = ::GetStringInput("Enter the accepted EULA: ", 200, false, c_szValidInputCharacters);
    std::string strLegalAgreement = ::GetStringInput("Enter the legal agreement: ", 200, false, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());
    __DebugAssert(0 < strEula.size());
    __DebugAssert(0 < strLegalAgreement.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DigitalContractGuid", strDcGuid);
    oDcInformation.PutString("EULA", strEula);
    oDcInformation.PutString("LegalAgreement", strLegalAgreement);

    fSuccess = ::ActivateDigitalContract(c_strEncodedEosb, oDcInformation);

    return fSuccess;
}

/********************************************************************************************/

bool ActivateDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const StructuredBuffer & c_oDcInformation
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEncodedEosb.size());

    bool fSuccess = false;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDcInformation.GetString("DigitalContractGuid") +"\","
                                "\n    \"EULA\": \""+ c_oDcInformation.GetString("EULA") +"\","
                                "\n    \"LegalAgreement\": \""+ c_oDcInformation.GetString("LegalAgreement") +"\""
                                "\n}";
        std::string strHttpRequest = "PATCH /SAIL/DigitalContractManager/Researcher/Activate?Eosb="+ c_strEncodedEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error activating the digital contract.", nullptr);
        std::cout << "Instructions: " << oResponse.GetString("Instructions") << std::endl;
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error activating the digital contract.");
    }

    return fSuccess;
}

/********************************************************************************************/

std::vector<Byte> ListDigitalContracts(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlDigitalContracts;

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strHttpRequest = "GET /SAIL/DigitalContractManager/DigitalContracts?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strHttpRequest.data(), (strHttpRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {   
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
        stlDigitalContracts = oResponse.GetStructuredBuffer("DigitalContracts").GetSerializedBuffer();
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error getting list of digital contracts.");
    }

    return stlDigitalContracts;
}

/********************************************************************************************/

void PrintDigitalContracts(
    _in const StructuredBuffer & c_oDigitalContracts
    )
{
    __DebugFunction();

    // Print digital contract information
    std::cout << "************************\n List of Digital Contracts \n************************\n" << std::endl;
    for (std::string strElement : c_oDigitalContracts.GetNamesOfElements())
    {
        StructuredBuffer oElement(c_oDigitalContracts.GetStructuredBuffer(strElement.c_str()));
        std::cout << "Digital contract guid: " << strElement << std::endl;
        std::cout << "Title: " << oElement.GetString("Title") << std::endl;
        std::cout << "Version number: " << oElement.GetString("VersionNumber") << std::endl;
        std::cout << "Contract stage: " << (Dword) oElement.GetFloat64("ContractStage") << std::endl;
        std::cout << "Subscription days: " << (uint64_t) oElement.GetFloat64("SubscriptionDays") << std::endl;
        std::cout << "Dataset guid: " << oElement.GetString("DatasetGuid") << std::endl;
        std::cout << "Activation date: " << oElement.GetFloat64("ActivationTime") << std::endl;
        std::cout << "Expiration date: " << oElement.GetFloat64("ExpirationTime") << std::endl;
        std::cout << "Eula accepted by data owner organization: " << oElement.GetString("EulaAcceptedByDOOAuthorizedUser") << std::endl;
        std::cout << "Eula accepted by researcher organization: " << oElement.GetString("EulaAcceptedByROAuthorizedUser") << std::endl;
        std::cout << "Legal agreement: " << oElement.GetString("LegalAgreement") << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
    }
}

/********************************************************************************************/

bool PullDigitalContract(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Pull Digital Contract \n************************\n" << std::endl;
    std::string strDcGuid = ::GetStringInput("Enter hyphen and curly braces formatted digital contract guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"DigitalContractGuid\": \""+ strDcGuid +"\""
                                "\n}";
        std::string strHttpRequest = "GET /SAIL/DigitalContractManager/PullDigitalContract?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
        fSuccess = true;
        StructuredBuffer oDigitalContract(oResponse.GetStructuredBuffer("DigitalContract"));
        std::cout << "Digital contract guid: " << oDigitalContract.GetString("DigitalContractGuid") << std::endl;
        std::cout << "Title: " << oDigitalContract.GetString("Title") << std::endl;
        std::cout << "Version number: " << oDigitalContract.GetString("VersionNumber") << std::endl;
        std::cout << "Contract stage: " << (Dword) oDigitalContract.GetFloat64("ContractStage") << std::endl;
        std::cout << "Subscription days: " << (uint64_t) oDigitalContract.GetFloat64("SubscriptionDays") << std::endl;
        std::cout << "Dataset guid: " << oDigitalContract.GetString("DatasetGuid") << std::endl;
        std::cout << "Legal agreement: " << oDigitalContract.GetString("LegalAgreement") << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error pulling the digital contract.");
    }

    return fSuccess;
}

/********************************************************************************************/

std::vector<Byte> GetRemoteAttestationCertificate(void)
{
    __DebugFunction();

    std::vector<Byte> stlSerializedResponse;

    // Generate a random nonce using OpenSSL
    std::vector<Byte> stlNonce(256);
    int nStatus = ::RAND_bytes(stlNonce.data(), stlNonce.size());
    _ThrowBaseExceptionIf((1 != nStatus), "Failed to generate a random nonce.", nullptr);
    // Base64 encode buffer
    std::string strNonce = ::Base64Encode(stlNonce.data(), stlNonce.size());

    try 
    {
        std::vector<Byte> stlRestResponse;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(g_szServerIpAddress, g_unPortNumber);

        // Create rest request
        std::string strContent = "{\n    \"Nonce\": \""+ strNonce +"\""
                                "\n}";
        std::string strHttpRequest = "GET /SAIL/AuthenticationManager/RemoteAttestationCertificate HTTP/1.1\r\n"
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
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);
        
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting remote attestation certificate.", nullptr);
        // The following is the response structure
        std::vector<Byte> stlRemoteAttestationCert = ::Base64Decode(oResponse.GetString("RemoteAttestationCertificatePem").c_str());
        std::string strPulicKeyPem = oResponse.GetString("PublicKeyCertificate");
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage(oBaseException.GetExceptionMessage());
    }
    catch(...)
    {
        ::ShowErrorMessage("Error getting remote attestation certificate.");
    }

    return stlSerializedResponse;
}

