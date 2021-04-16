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
    // Connect to the Web API Portal Gateway
    TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
    // Build the HTTP request string
    std::string strHttpLoginRequest = "POST /SAIL/AuthenticationManager/User/Login?Email="+ c_strUsername +"&Password="+ c_strPassword +" HTTP/1.1\r\n"
                                      "Accept: */*\r\n"
                                      "Host: localhost:6200\r\n"
                                      "Connection: keep-alive\r\n"
                                      "Content-Length: 0\r\n"
                                      "\r\n";
    // Send the HTTP request string
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
    _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Invalid Packet.", nullptr);
    std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
    std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
    // Release poTlsNode
    poTlsNode->Release();
    StructuredBuffer oResponse(stlSerializedResponse);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strHttpLoginRequest = "GET /SAIL/AuthenticationManager/GetBasicUserInformation?Eosb="+ c_strEosb +" HTTP/1.1\r\n"
                                              "Accept: */*\r\n"
                                              "Host: localhost:6200\r\n"
                                              "Connection: keep-alive\r\n"
                                              "Content-Length: 0\r\n"
                                              "\r\n";
            // Send the HTTP request string
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
            _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Invalid Packet.", nullptr);
            std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
            std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
            _ThrowBaseExceptionIf((0 == stlSerializedResponse.size()), "Invalid Packet.", nullptr);
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            oBasicUserInformation.PutString("OrganizationGuid", oResponse.GetString("OrganizationGuid"));
            oBasicUserInformation.PutString("UserGuid", oResponse.GetString("UserGuid"));
            oBasicUserInformation.PutQword("AccessRights", (Qword) oResponse.GetFloat64("AccessRights"));
            oBasicUserInformation.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oBasicUserInformation.Clear();
        oBasicUserInformation.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"Eosb\": \""+ c_strEosb +"\","
                                     "\n    \"ParentGuid\": \""+ c_strParentBranchNodeIdentifier +"\","
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
            
            _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Invalid Packet.", nullptr);
            std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
            std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"Eosb\": \""+ c_strEosb +"\""
                                    "\n}";
            std::string strHttpRequest = "GET /SAIL/DigitalContractManager/DigitalContracts HTTP/1.1\r\n"
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
            _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Invalid Packet.", nullptr);
            std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
            std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);
            oListOfDigitalContracts.PutStructuredBuffer("ListOfDigitalContracts", oResponse.GetStructuredBuffer("DigitalContracts"));
            oListOfDigitalContracts.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oListOfDigitalContracts.Clear();
        oListOfDigitalContracts.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"Eosb\": \""+ c_strEosb +"\","
                                    "\n    \"DigitalContractGuid\": \""+ c_oDigitalContractIdentifier.ToString(eHyphensAndCurlyBraces) +"\""
                                    "\n}";
            std::string strHttpRequest = "GET /SAIL/DigitalContractManager/PullDigitalContract HTTP/1.1\r\n"
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
            
            _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Invalid Packet.", nullptr);
            std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
            std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Error getting list of digital contracts.", nullptr);
            oDigitalContract.PutStructuredBuffer("DigitalContract", oResponse.GetStructuredBuffer("DigitalContract"));
            oDigitalContract.PutBoolean("Success", true);
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oDigitalContract.Clear();
        oDigitalContract.PutBoolean("Success", false);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"IEosb\": \""+ c_strEosb +"\","
                                    "\n    \"DigitalContractGuid\": \""+ c_strDigitalContractIdentifier +"\","
                                    "\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\","
                                    "\n    \"HeartbeatBroadcastTime\": "+ std::to_string(::GetEpochTimeInSeconds()) +","
                                    "\n    \"IPAddress\": \""+ c_strIpAddress +"\""
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
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            std::cout << oResponse.ToString() << std::endl;
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineEosb = oResponse.GetString("VmEosb");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"Eosb\": \""+ c_strEosb +"\","
                                    "\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\""
                                    "\n}";
            std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/DataOwner/RegisterVM HTTP/1.1\r\n"
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
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineAuditEventBranchNodeIdentifier = oResponse.GetString("VmEventGuid");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
            // Connect to the Web API Portal Gateway
            TlsNode * poTlsNode = ::TlsConnectToNetworkSocket(gs_strIpAddressOfWebPortalGateway.c_str(), gs_unPortAddressOfWebPortalGateway);
            // Build the HTTP request string
            std::string strContent = "{\n    \"Eosb\": \""+ c_strEosb +"\","
                                    "\n    \"VirtualMachineGuid\": \""+ c_strVirtualMachineIdentifier +"\""
                                    "\n}";
            std::string strHttpRegisterRequest = "POST /SAIL/VirtualMachineManager/Researcher/RegisterVM HTTP/1.1\r\n"
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
            // Release poTlsNode
            poTlsNode->Release();
            StructuredBuffer oResponse(stlSerializedResponse);
            _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error while processing the transaction.", nullptr);
            strVirtualMachineAuditEventBranchNodeIdentifier = oResponse.GetString("VmEventGuid");
        }
    }
    
    catch(BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return strVirtualMachineAuditEventBranchNodeIdentifier;
}