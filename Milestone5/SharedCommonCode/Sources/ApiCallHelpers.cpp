/*********************************************************************************************
 *
 * @file ApiCallHelpers.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/
 
#include "ApiCallHelpers.h"
#include "Base64Encoder.h"
#include "CurlRest.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "HttpRequestParser.h"
#include "JsonValue.h"
#include "SmartMemoryAllocator.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "Utils.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

static std::string gs_strIpAddressOfWebPortalGateway;
static unsigned int gs_unPortAddressOfWebPortalGateway;
    
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

/********************************************************************************************/

static bool __stdcall ParseFirstLine(
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
    if ((false == strStatus.empty())&&("200" == strStatus))
    {
        fSuccess = true;
    }

    return fSuccess;
}

/********************************************************************************************/

static std::vector<Byte> __stdcall GetResponseBody(
    _in const std::string & c_strRequestData,
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    std::vector<Byte> stlSerializedResponse;

    if (0 < c_strRequestData.size())
    {
        if (true == ::ParseFirstLine(c_strRequestData))
        {
            // Parse Header of the Rest Request
            HttpRequestParser oParser;
            if ((true == oParser.ParseResponse(c_strRequestData))&&(true == oParser.HeaderExists("Content-Length")))
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
                }
            }
        }
    }

    return stlSerializedResponse;
}

/*********************************************************************************************/

bool __stdcall SetIpAddressOfSailWebApiPortalGateway(
    _in const std::string & c_strIpAddressOfWebPortalGateway,
    _in Word wPortAddressOfWebPortalGateway
    ) throw()
{
    __DebugFunction();
    __DebugAssert(0 == gs_strIpAddressOfWebPortalGateway.size());
    __DebugAssert(0 < c_strIpAddressOfWebPortalGateway.size());

    gs_strIpAddressOfWebPortalGateway = c_strIpAddressOfWebPortalGateway;
    gs_unPortAddressOfWebPortalGateway = wPortAddressOfWebPortalGateway;

    return true;
}

/*********************************************************************************************/

std::string __stdcall LoginToSailWebApiPortal(
    _in const std::string & c_strUsername,
    _in const std::string & c_strPassword
    )
{
    __DebugFunction();
    __DebugAssert(0 < gs_strIpAddressOfWebPortalGateway.size());
    
    std::string strEosb;

    // There is no sense trying to login if the incoming parameters are invalid
    _ThrowBaseExceptionIf(((0 == c_strUsername.size())&&(0 == c_strPassword.size())), "Invalid parameters.", nullptr);
    // Build the HTTP request
    std::string strVerb = "POST";
    std::string strApiUrl = "/SAIL/AuthenticationManager/User/Login?Email="+ c_strUsername +"&Password="+ c_strPassword;
    std::string strJsonBody = "";
    // Make the API call and get REST response
    std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strJsonBody, true);
    std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
    StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
    _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
    strEosb = oResponse.GetString("Eosb");
    
    return strEosb;
}

/*********************************************************************************************/

StructuredBuffer __stdcall GetSailWebApiPortalBasicUserInformation(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    
    StructuredBuffer oBasicUserInformation;
    
    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "GET";
            std::string strApiUrl = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb="+ c_strEosb;
            std::string strJsonBody = "";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strJsonBody, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            oBasicUserInformation.PutString("OrganizationGuid", oResponse.GetString("OrganizationGuid"));
            oBasicUserInformation.PutString("UserGuid", oResponse.GetString("UserGuid"));
            oBasicUserInformation.PutQword("AccessRights", (Qword) oResponse.GetFloat64("AccessRights"));
            oBasicUserInformation.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oBasicUserInformation.Clear();
        oBasicUserInformation.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oBasicUserInformation.Clear();
        oBasicUserInformation.PutBoolean("Success", false);
    }
    
    return oBasicUserInformation;
}

/*********************************************************************************************/

bool __stdcall TransmitAuditEventsToSailWebApiPortal(
    _in const std::string & c_strEosb,
    _in const std::string & c_strParentBranchNodeIdentifier,
    _in const StructuredBuffer & c_oAuditEvents
    )
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    try
    {
        if ((0 < c_strEosb.size())&&(0 < c_strParentBranchNodeIdentifier.size()))
        {
            // Build the HTTP request string
            std::string strVerb = "POST";
            std::string strApiUrl = "/SAIL/AuditLogManager/LeafEvents?Eosb="+ c_strEosb;
            std::string strContent = "{\n    \"ParentGuid\": \""+ c_strParentBranchNodeIdentifier +"\","
                                     "\n    \"LeafEvents\": [";
            // Add leaf events to the rest request body
            std::vector<std::string> stlEvents = c_oAuditEvents.GetNamesOfElements();
            unsigned int unNumberOfEvents = stlEvents.size();
            for (unsigned int unIndex = 0; unIndex < unNumberOfEvents; ++unIndex)
            {
                StructuredBuffer oEvent(c_oAuditEvents.GetStructuredBuffer(stlEvents.at(unIndex).c_str()));
                strContent += "\n        {"
                              "\n            \"EventGuid\": \""+ oEvent.GetString("EventGuid") +"\","
                              "\n            \"EventType\": "+ std::to_string(oEvent.GetQword("EventType")) +","
                              "\n            \"Timestamp\": "+ std::to_string(oEvent.GetUnsignedInt64("Timestamp")) +","
                              "\n            \"SequenceNumber\": "+ std::to_string(oEvent.GetUnsignedInt32("SequenceNumber")) +","
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
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strContent, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    return fSuccess;
}

/*********************************************************************************************/

StructuredBuffer __stdcall GetListOfDigitalContracts(
    _in const std::string & c_strEosb
    )
{
    __DebugFunction();
    
    StructuredBuffer oListOfDigitalContracts;
    
    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "GET";
            std::string strApiUrl = "/SAIL/DigitalContractManager/DigitalContracts?Eosb="+ c_strEosb;
            std::string strJsonBody = "";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strJsonBody, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            oListOfDigitalContracts.PutStructuredBuffer("ListOfDigitalContracts", oResponse.GetStructuredBuffer("DigitalContracts"));
            oListOfDigitalContracts.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oListOfDigitalContracts.Clear();
        oListOfDigitalContracts.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oListOfDigitalContracts.Clear();
        oListOfDigitalContracts.PutBoolean("Success", false);
    }
    
    return oListOfDigitalContracts;
}

/*********************************************************************************************/

StructuredBuffer __stdcall GetDigitalContract(
    _in const std::string & c_strEosb,
    _in const Guid & c_oDigitalContractIdentifier
    )
{
    __DebugFunction();
    
    StructuredBuffer oDigitalContract;
    
    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "GET";
            std::string strApiUrl = "/SAIL/DigitalContractManager/PullDigitalContract?Eosb="+ c_strEosb;
            std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_oDigitalContractIdentifier.ToString(eHyphensAndCurlyBraces) +"\""
                                    "\n}";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strContent, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
            oDigitalContract.PutStructuredBuffer("DigitalContract", oResponse.GetStructuredBuffer("DigitalContract"));
            oDigitalContract.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oDigitalContract.Clear();
        oDigitalContract.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oDigitalContract.Clear();
        oDigitalContract.PutBoolean("Success", false);
    }
    
    return oDigitalContract;
}

/********************************************************************************************/

std::string __stdcall RegisterVirtualMachineWithSailWebApiPortal(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier,
    _in const std::string & c_strDigitalContractIdentifier,
    _in const std::string & c_strIpAddress
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());
    __DebugAssert(0 < c_strVirtualMachineIdentifier.size());

    std::string strVirtualMachineEosb;

    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "POST";
            std::string strApiUrl = "/SAIL/VirtualMachineManager/RegisterVM?IEosb="+ c_strEosb;
            std::string strContent = "{\n    \"DigitalContractGuid\": \""+ c_strDigitalContractIdentifier +"\","
                                    "\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\","
                                    "\n    \"HeartbeatBroadcastTime\": "+ std::to_string(::GetEpochTimeInSeconds()) +","
                                    "\n    \"IPAddress\": \""+ c_strIpAddress +"\""
                                    "\n}";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strContent, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            std::cout << oResponse.ToString() << std::endl;
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineEosb = oResponse.GetString("VmEosb");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return strVirtualMachineEosb;
}

/********************************************************************************************/

std::string __stdcall RegisterVirtualMachineDataOwner(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());
    __DebugAssert(0 < c_strVirtualMachineIdentifier.size());

    std::string strVirtualMachineAuditEventBranchNodeIdentifier;

    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "POST";
            std::string strApiUrl = "/SAIL/VirtualMachineManager/DataOwner/RegisterVM?Eosb="+ c_strEosb;
            std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\""
                                    "\n}";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strContent, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineAuditEventBranchNodeIdentifier = oResponse.GetString("VmEventGuid");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return strVirtualMachineAuditEventBranchNodeIdentifier;
}

/********************************************************************************************/

std::string RegisterVirtualMachineResearcher(
    _in const std::string & c_strEosb,
    _in const std::string & c_strVirtualMachineIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strEosb.size());
    __DebugAssert(0 < c_strVirtualMachineIdentifier.size());

    std::string strVirtualMachineAuditEventBranchNodeIdentifier;

    try
    {
        if (0 < c_strEosb.size())
        {
            // Build the HTTP request string
            std::string strVerb = "POST";
            std::string strApiUrl = "/SAIL/VirtualMachineManager/Researcher/RegisterVM?Eosb="+ c_strEosb;
            std::string strContent = "{\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\""
                                    "\n}";
            // Make the API call and get REST response
            std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfWebPortalGateway, (Word) gs_unPortAddressOfWebPortalGateway, strVerb, strApiUrl, strContent, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineAuditEventBranchNodeIdentifier = oResponse.GetString("VmEventGuid");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return strVirtualMachineAuditEventBranchNodeIdentifier;
}