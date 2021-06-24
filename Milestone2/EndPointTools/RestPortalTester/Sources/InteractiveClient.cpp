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
#include "ExceptionRegister.h"

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
    __DebugAssert(0 < strlen(g_szServerIpAddress));
    __DebugAssert(0 != g_unPortNumber);
    __DebugAssert(0 < c_strEmail.size());
    __DebugAssert(0 < c_strUserPassword.size());

    std::string strEosb;

    try
    {
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AuthenticationManager/User/Login?Email="+ c_strEmail +"&Password="+ c_strUserPassword;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error logging in.", nullptr);
        strEosb = oResponse.GetString("Eosb");
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb="+ c_strEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error decrypting eosb.", nullptr);
        oUserInformation.PutString("OrganizationGuid", oResponse.GetString("OrganizationGuid"));
        oUserInformation.PutString("UserGuid", oResponse.GetString("UserGuid"));
        oUserInformation.PutQword("AccessRights", (Qword) oResponse.GetFloat64("AccessRights"));
        oUserInformation.PutString("Username", oResponse.GetString("Username"));
        oUserInformation.PutString("Title", oResponse.GetString("Title"));
        oUserInformation.PutString("Email", oResponse.GetString("Email"));
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/CryptographicManager/User/GetIEosb?Eosb="+ c_strEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting imposter eosb.", nullptr);
        strIEosb = oResponse.GetString("UpdatedEosb");
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return strIEosb;
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
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AuditLogManager/LeafEvents?Eosb="+ c_strEncodedEosb;
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

        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering leaf events.", nullptr);
        std::cout << "Leaf events added successfully!" << std::endl;
        fSuccess = true;
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/VirtualMachineManager/RegisterVM?IEosb="+ c_strEncodedIEosb;
        std::string strContent = "{\n   \"DigitalContractGuid\": \""+ c_oVmInformation.GetString("DigitalContractGuid") +"\","
                                "\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\","
                                "\n    \"HeartbeatBroadcastTime\": "+ std::to_string(::GetEpochTimeInSeconds()) +","
                                "\n    \"IPAddress\": \""+ c_oVmInformation.GetString("IPAddress") +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering the virtual machine.", nullptr);
        strVmEosb = oResponse.GetString("VmEosb");
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/VirtualMachineManager/DataOwner/RegisterVM?Eosb="+ c_strEncodedVmEosb;
        std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering the virtual machine.", nullptr);
        strVmEventGuid = oResponse.GetString("VmEventGuid");
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/VirtualMachineManager/Researcher/RegisterVM?Eosb="+ c_strEncodedVmEosb;
        std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVmGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering the virtual machine.", nullptr);
        strVmEventGuid = oResponse.GetString("VmEventGuid");
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/AuditLogManager/GetListOfEvents?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n    \"ParentGuid\": \""+ c_strParentGuid +"\","
                                "\n    \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\","
                                "\n    \"Filters\":"
                                "\n    {"
                                "\n         \"SequenceNumber\": 0"
                                "\n    }"
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
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
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AccountManager/RegisterUser";
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
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new organization and super user.", nullptr);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("RootEventStatus")), "Error registering root event for the organization.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AccountManager/Admin/RegisterUser?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n   \"Email\": \""+ c_oUserInformation.GetString("Email") +"\","
                                "\n    \"Password\": \""+ c_oUserInformation.GetString("Password") +"\","
                                "\n    \"Name\": \""+ c_oUserInformation.GetString("Name") +"\","
                                "\n    \"PhoneNumber\": \""+ c_oUserInformation.GetString("PhoneNumber") +"\","
                                "\n    \"Title\": \""+ c_oUserInformation.GetString("Title") +"\","
                                "\n    \"AccessRights\": "+ std::to_string(c_oUserInformation.GetQword("AccessRights")) +","
                                "\n    \"OrganizationGuid\": \""+ c_strOrganizationGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new user.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool UpdateOrganizationInformation(
    _in const std::string & c_strEncodedEosb
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
        // Create rest request
        std::string strVerb = "PUT";
        std::string strApiUrl = "/SAIL/AccountManager/Update/Organization?Eosb="+ c_strEncodedEosb;
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
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error updating organization information.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool ListOrganizations(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try 
    {
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/AccountManager/Organizations?Eosb="+ c_strEncodedEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of organizations.", nullptr);
        fSuccess = true;
        std::cout << "************************\n List of Organizations \n************************\n" << std::endl;
        StructuredBuffer oOrganizations(oResponse.GetStructuredBuffer("Organizations"));
        for (std::string strElement : oOrganizations.GetNamesOfElements())
        {
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
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool DeleteUser(
    _in const std::string & c_strEncodedEosb
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
        // Create rest request
        std::string strVerb = "DELETE";
        std::string strApiUrl = "/SAIL/AccountManager/Remove/User?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n   \"UserGuid\": \""+ strUserGuid +"\","
                                "\n    \"IsHardDelete\": true"
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error deleting user.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool DeleteOrganization(
    _in const std::string & c_strEncodedEosb
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
        // Create rest request
        std::string strVerb = "DELETE";
        std::string strApiUrl = "/SAIL/AccountManager/Remove/Organization?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n   \"OrganizationGuid\": \""+ strOrganizationGuid +"\","
                                "\n    \"IsHardDelete\": true"
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error deleting organization.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
    std::string strDescription = ::GetStringInput("Enter description for the digital contract: ", 200, false, c_szValidInputCharacters);
    std::string strVersionNumber = "0x0000000100000001";
    uint64_t unSubscriptionDays = std::stoull(::GetStringInput("Enter your requested subscription period (in days): ", 50, false, c_szValidInputCharacters));
    std::string strDatasetGuid = Guid(eDataset).ToString(eHyphensAndCurlyBraces);
    std::string strLegalAgreement = ::GetStringInput("Enter the legal agreement: ", 200, false, c_szValidInputCharacters);

    __DebugAssert(38 == strDooGuid.size());
    __DebugAssert(0 < strDescription.size());
    __DebugAssert(0 < unSubscriptionDays);
    __DebugAssert(0 < strLegalAgreement.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("Title", strTitle);
    oDcInformation.PutString("DOOGuid", strDooGuid);
    oDcInformation.PutString("Description", strDescription);
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
        if (true == c_oDcInformation.IsElementPresent("DatasetGuid", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            strDatasetGuid = c_oDcInformation.GetString("DatasetGuid");
        }
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/DigitalContractManager/Applications?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n   \"DataOwnerOrganization\": \""+ c_oDcInformation.GetString("DOOGuid") +"\","
                                "\n    \"Title\": \""+ c_oDcInformation.GetString("Title") +"\","
                                "\n    \"Description\": \""+ c_oDcInformation.GetString("Description") +"\","
                                "\n    \"VersionNumber\": \""+ strVersionNumber +"\","
                                "\n    \"SubscriptionDays\": "+ std::to_string(c_oDcInformation.GetUnsignedInt64("SubscriptionDays")) +","
                                "\n    \"DatasetGuid\": \""+ strDatasetGuid +"\","
                                "\n    \"LegalAgreement\": \""+ c_oDcInformation.GetString("LegalAgreement") +"\","
                                "\n    \"DatasetDRMMetadataSize\": "+ std::to_string(0) +","
                                "\n    \"DatasetDRMMetadata\":{}"
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering new digital contract.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
    std::string strDescription = ::GetStringInput("Add your comment on the digital contract (if any): ", 200, false, c_szValidInputCharacters);
    uint64_t unRetentionTime = std::stoull(::GetStringInput("Enter the retention time: ", 50, false, c_szValidInputCharacters));
    std::string strLegalAgreement = ::GetStringInput("Enter the legal agreement: ", 200, false, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());
    __DebugAssert(0 < unRetentionTime);
    __DebugAssert(0 < strLegalAgreement.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DigitalContractGuid", strDcGuid);
    oDcInformation.PutString("Description", strDescription);
    oDcInformation.PutUnsignedInt64("RetentionTime", unRetentionTime);
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
        // Create rest request
        std::string strVerb = "PATCH";
        std::string strApiUrl = "/SAIL/DigitalContractManager/DataOwner/Accept?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDcInformation.GetString("DigitalContractGuid") +"\",";
        if (true == c_oDcInformation.IsElementPresent("Description", ANSI_CHARACTER_STRING_VALUE_TYPE)) {
            strContent += "\n    \"Description\": \""+ c_oDcInformation.GetString("Description") +"\",";
        }
        strContent +=   "\n    \"RetentionTime\": "+ std::to_string(c_oDcInformation.GetUnsignedInt64("RetentionTime")) +","
                        "\n    \"LegalAgreement\": \""+ c_oDcInformation.GetString("LegalAgreement") +"\""
                        "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error approving the digital contract.", nullptr);
        std::cout << "Instructions: " << oResponse.GetString("Instructions") << std::endl;
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
    std::string strDescription = ::GetStringInput("Add your comment on the digital contract (if any): ", 200, false, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DigitalContractGuid", strDcGuid);
    oDcInformation.PutString("Description", strDescription);

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
        // Create rest request
        std::string strVerb = "PATCH";
        std::string strApiUrl = "/SAIL/DigitalContractManager/Researcher/Activate?Eosb="+ c_strEncodedEosb;
        std::string strContent = "";
        if (true == c_oDcInformation.IsElementPresent("Description", ANSI_CHARACTER_STRING_VALUE_TYPE)) {
            strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDcInformation.GetString("DigitalContractGuid") +"\","
                        "\n    \"Description\": \""+ c_oDcInformation.GetString("Description") +"\""
                        "\n}";
        }
        else 
        {
            strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDcInformation.GetString("DigitalContractGuid") +"\""
                        "\n}";
        }
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error activating the digital contract.", nullptr);
        std::cout << "Instructions: " << oResponse.GetString("Instructions") << std::endl;
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

std::vector<Byte> ListDigitalContracts(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlDigitalContracts;

    try 
    {
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/DigitalContractManager/DigitalContracts?Eosb="+ c_strEncodedEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
        stlDigitalContracts = oResponse.GetStructuredBuffer("DigitalContracts").GetSerializedBuffer();
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        StructuredBuffer oElement = c_oDigitalContracts.GetStructuredBuffer(strElement.c_str());
        std::cout << "Digital contract guid: " << strElement << std::endl;
        std::cout << "Title: " << oElement.GetString("Title") << std::endl;
        std::cout << "Description: " << oElement.GetString("Description") << std::endl;
        std::cout << "Data owner organization guid: " << oElement.GetString("DataOwnerOrganization") << std::endl;
        std::cout << "Researcher organization guid: " << oElement.GetString("ResearcherOrganization") << std::endl;
        std::cout << "Version number: " << oElement.GetString("VersionNumber") << std::endl;
        std::cout << "Contract stage: " << (Dword) oElement.GetFloat64("ContractStage") << std::endl;
        std::cout << "Subscription days: " << (uint64_t) oElement.GetFloat64("SubscriptionDays") << std::endl;
        std::cout << "Dataset guid: " << oElement.GetString("DatasetGuid") << std::endl;
        std::cout << "Eula: " << oElement.GetString("Eula") << std::endl;
        std::cout << "Legal agreement: " << oElement.GetString("LegalAgreement") << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
    }
}

/********************************************************************************************/

std::vector<Byte> PullDigitalContract(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get digital contract information
    std::cout << "************************\n Pull Digital Contract \n************************\n" << std::endl;
    std::string strDcGuid = ::GetStringInput("Enter hyphen and curly braces formatted digital contract guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(0 < strDcGuid.size());

    return ::PullDigitalContract(c_strEosb, strDcGuid);
}

/********************************************************************************************/

std::vector<Byte> PullDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid
    )
{
    __DebugFunction();

    std::vector<Byte> stlSerializedDigitalContract;

    try 
    {
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+ c_strEncodedEosb;
        std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_strDcGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
        StructuredBuffer oDigitalContract(oResponse.GetStructuredBuffer("DigitalContract"));
        std::cout << "Digital contract guid: " << oDigitalContract.GetString("DigitalContractGuid") << std::endl;
        std::cout << "Title: " << oDigitalContract.GetString("Title") << std::endl;
        std::cout << "Description: " << oDigitalContract.GetString("Description") << std::endl;
        std::cout << "Version number: " << oDigitalContract.GetString("VersionNumber") << std::endl;
        std::cout << "Contract stage: " << (Dword) oDigitalContract.GetFloat64("ContractStage") << std::endl;
        std::cout << "Subscription days: " << (uint64_t) oDigitalContract.GetFloat64("SubscriptionDays") << std::endl;
        std::cout << "Dataset guid: " << oDigitalContract.GetString("DatasetGuid") << std::endl;
        std::cout << "Legal agreement: " << oDigitalContract.GetString("LegalAgreement") << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;

        stlSerializedDigitalContract.resize(oDigitalContract.GetSerializedBufferRawDataSizeInBytes());
        ::memcpy(stlSerializedDigitalContract.data(), oDigitalContract.GetSerializedBufferRawDataPtr(), oDigitalContract.GetSerializedBufferRawDataSizeInBytes());
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return stlSerializedDigitalContract;
}

/********************************************************************************************/

bool RegisterDataset(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get dataset information
    std::cout << "************************\n Register Dataset \n************************\n" << std::endl;
    std::string strDsetGuid = Guid(eDataset).ToString(eHyphensAndCurlyBraces);
    std::string strVersionNumber = ::GetStringInput("Enter dataset version number: ", 16, false, c_szValidInputCharacters);
    std::string strName = ::GetStringInput("Enter dataset name: ", 50, false, c_szValidInputCharacters);
    std::string strDescription = ::GetStringInput("Enter dataset description: ", 100, false, c_szValidInputCharacters);
    std::string strKeywords = ::GetStringInput("Enter comma seperated keywords: ", 100, false, c_szValidInputCharacters);
    uint64_t un64PublishTime = std::stoull(::GetStringInput("Enter datetime (in seconds) of publish date: ", 64, false, c_szValidInputCharacters));
    Byte bPrivacyLevel = std::stoi(::GetStringInput("Enter privacy level (0-10): ", 2, false, c_szValidInputCharacters));
    std::string strLimitations = ::GetStringInput("Enter country codes where dataset can be used: ", 100, false, c_szValidInputCharacters);

    StructuredBuffer oDsetInformation;
    oDsetInformation.PutString("DatasetGuid", strDsetGuid);
    oDsetInformation.PutString("VersionNumber", strVersionNumber);
    oDsetInformation.PutString("DatasetName", strName);
    oDsetInformation.PutString("Description", strDescription);
    oDsetInformation.PutString("Keywords", strKeywords);
    oDsetInformation.PutUnsignedInt64("PublishDate", un64PublishTime);
    oDsetInformation.PutByte("PrivacyLevel", bPrivacyLevel);
    oDsetInformation.PutString("JurisdictionalLimitations", strLimitations);

    __DebugAssert(38 == strDsetGuid.size());

    fSuccess = ::RegisterDataset(c_strEosb, oDsetInformation);

    return fSuccess;
}

/********************************************************************************************/

bool RegisterDataset(
    _in const std::string & c_strEosb,
    _in const StructuredBuffer & c_oDsetInformation
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    bool fSuccess = false;

    try
    {
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/DatasetManager/RegisterDataset?Eosb="+ c_strEosb;
        std::string strContent = "{\n   \"DatasetGuid\": \""+ c_oDsetInformation.GetString("DatasetGuid") +"\","
                                "\n   \"DatasetData\": {"
                                "\n   \"VersionNumber\": \""+ c_oDsetInformation.GetString("VersionNumber") +"\","
                                "\n   \"DatasetName\": \""+ c_oDsetInformation.GetString("DatasetName") +"\","
                                "\n   \"Description\": \""+ c_oDsetInformation.GetString("Description") +"\","
                                "\n   \"Keywords\": \""+ c_oDsetInformation.GetString("Keywords") +"\","
                                "\n   \"PublishDate\": "+ std::to_string(c_oDsetInformation.GetUnsignedInt64("PublishDate")) +","
                                "\n   \"PrivacyLevel\": "+ std::to_string(c_oDsetInformation.GetByte("PrivacyLevel")) +","
                                "\n   \"JurisdictionalLimitations\": \""+ c_oDsetInformation.GetString("JurisdictionalLimitations") +"\""
                                "\n   }"
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error registering the dataset.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool ListDatasets(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    bool fSuccess = false;

    try
    {
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/DatasetManager/ListDatasets?Eosb="+ c_strEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error fetching available datasets.", nullptr);
        fSuccess = true;
        std::cout << "************************\n List of Datasets \n************************\n" << std::endl;
        StructuredBuffer oDatasets(oResponse.GetStructuredBuffer("Datasets"));
        for (std::string strElement : oDatasets.GetNamesOfElements())
        {
            StructuredBuffer oElement(oDatasets.GetStructuredBuffer(strElement.c_str()));
            std::cout << "Guid: " << strElement << std::endl;
            std::cout << "Name: " << oElement.GetString("DatasetName") << std::endl;
            std::cout << "Version number: " << oElement.GetString("VersionNumber") << std::endl;
            std::cout << "Data owner organization guid: " << oElement.GetString("DataOwnerGuid") << std::endl;
            std::cout << "Description: " << oElement.GetString("Description") << std::endl;
            std::cout << "Keywords: " << oElement.GetString("Keywords") << std::endl;
            std::cout << "Publish date: " << oElement.GetFloat64("PublishDate") << std::endl;
            std::cout << "Privacy level: " << oElement.GetFloat64("PrivacyLevel") << std::endl;
            std::cout << "Jurisdictional limitations: " << oElement.GetString("JurisdictionalLimitations") << std::endl;
            std::cout << "------------------------------------------------------" << std::endl;
        }
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool PullDataset(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get dataset information
    std::cout << "************************\n Pull Dataset \n************************\n" << std::endl;
    std::string strDsetGuid = ::GetStringInput("Enter hyphen and curly braces formatted dataset guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(38 == strDsetGuid.size());

    try
    {
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/DatasetManager/PullDataset?Eosb="+ c_strEosb;
        std::string strContent = "{\n   \"DatasetGuid\": \""+ strDsetGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting the dataset information.", nullptr);
        fSuccess = true;
        StructuredBuffer oDataset(oResponse.GetStructuredBuffer("Dataset"));
        std::cout << "Guid: " << oDataset.GetString("DatasetGuid") << std::endl;
        std::cout << "Name: " << oDataset.GetString("DatasetName") << std::endl;
        std::cout << "Version number: " << oDataset.GetString("VersionNumber") << std::endl;
        std::cout << "Data owner organization guid: " << oDataset.GetString("DataOwnerGuid") << std::endl;
        std::cout << "Description: " << oDataset.GetString("Description") << std::endl;
        std::cout << "Keywords: " << oDataset.GetString("Keywords") << std::endl;
        std::cout << "Publish date: " << oDataset.GetFloat64("PublishDate") << std::endl;
        std::cout << "Privacy level: " << oDataset.GetFloat64("PrivacyLevel") << std::endl;
        std::cout << "Jurisdictional limitations: " << oDataset.GetString("JurisdictionalLimitations") << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool DeleteDataset(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());

    bool fSuccess = false;

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    // Get dataset information
    std::cout << "************************\n Delete Dataset \n************************\n" << std::endl;
    std::string strDsetGuid = ::GetStringInput("Enter hyphen and curly braces formatted dataset guid: ", 38, true, c_szValidInputCharacters);

    __DebugAssert(38 == strDsetGuid.size());

    try
    {
        // Create rest request
        std::string strVerb = "DELETE";
        std::string strApiUrl = "/SAIL/DatasetManager/DeleteDataset?Eosb="+ c_strEosb;
        std::string strContent = "{\n   \"DatasetGuid\": \""+ strDsetGuid +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error deleting the dataset record.", nullptr);
        fSuccess = true;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
        // Create rest request
        std::string strVerb = "GET";
        std::string strApiUrl = "/SAIL/AuthenticationManager/RemoteAttestationCertificate";
        std::string strContent = "{\n    \"Nonce\": \""+ strNonce +"\""
                                "\n}";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strContent, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting remote attestation certificate.", nullptr);
        // The following is the response structure
        std::vector<Byte> stlRemoteAttestationCert = ::Base64Decode(oResponse.GetString("RemoteAttestationCertificatePem").c_str());
        std::string strPulicKeyPem = oResponse.GetString("PublicKeyCertificate");
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return stlSerializedResponse;
}

/********************************************************************************************/

void ShutdownPortal(
    _in const std::string & c_strEncodedEosb
    )
{
    __DebugFunction();

    try 
    {
        // Create rest request
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AuthenticationManager/ShutdownPortal?Eosb="+ c_strEncodedEosb;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(g_szServerIpAddress, (Word) g_unPortNumber, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((204 == oResponse.GetFloat64("Status")), "Error shutting down the server.", nullptr);
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}