/*********************************************************************************************
 *
 * @file SailWebApiFunctions.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Guid class that handles 128 bit (16 bytes) GUID/UUID values.
 *
 ********************************************************************************************/

#include <winsock2.h>   // Always include winsock2.g BEFORE Windows.h
#include <ws2tcpip.h>
#include <Windows.h>

#include "64BitHashes.h"
#include "Base64Encoder.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "JsonValue.h"
#include "RestApiHelperFunctions.h"
#include "StructuredBuffer.h"
#include "SmartMemoryAllocator.h"

#include <mutex>
#include <sstream>
#include <string>
#include <iostream>

// Microsoft Azure global data
static bool gs_fWinsockInitializer = false;
static std::string gs_strMicrosoftAzureAccessToken;

/// <summary>
/// We need this special function coupled with the SmartMemoryAllocator to make sure that we
/// are properly freeing the memory relating to the internal wszUnicodeString variable
/// </summary>
/// <param name="c_strAnsiString"></param>
/// <returns></returns>
static BSTR __cdecl ConvertToBSTR(
    _in const std::string & c_strAnsiString
    ) throw()
{
    __DebugFunction();

    BSTR pszString = nullptr;

    try
    {
        SmartMemoryAllocator oSmartMemoryAllocator;

        // Convert the char * string to wchar * string 
        size_t unCharacterCountIncludingNullTerminatingCharacter = (size_t) (c_strAnsiString.size() + 1);
        size_t unConvertedChars = 0;
        wchar_t* wszUnicodeString = (wchar_t*) oSmartMemoryAllocator.Allocate((unsigned int) unCharacterCountIncludingNullTerminatingCharacter * sizeof(wchar_t));
        ::mbstowcs_s(&unConvertedChars, wszUnicodeString, unCharacterCountIncludingNullTerminatingCharacter, c_strAnsiString.c_str(), c_strAnsiString.size());
        pszString = ::SysAllocString(wszUnicodeString);
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return pszString;
}

/// <summary>
/// Set the SAIL Web API portal IP address
/// </summary>
/// <param name="c_szIpAddressOfSailWebApiPortal"></param>
/// <returns></returns>
std::string __cdecl GetIpAddressOfHostName(
    _in const char * c_szHostName
    ) throw()
{
    __DebugFunction();

    std::string strIpAddress = "";

    try
    {
        // First we make sure that Winsock is initialized or else we cannot do ANY socket
        // operations
        if (false == gs_fWinsockInitializer)
        {
            WSADATA sWindowsSocketData;

            int nResult = ::WSAStartup(MAKEWORD(2, 2), &sWindowsSocketData);
            _ThrowBaseExceptionIf((0 != nResult), "WSAStartup() has failed with %d", nResult);
            gs_fWinsockInitializer = true;
        }
        // Now we go and get the IP address for the Microsoft Azure Api portal
        struct hostent * psRemoteHost;
        psRemoteHost = ::gethostbyname(c_szHostName);
        _ThrowBaseExceptionIf((nullptr == psRemoteHost), "gethostbyname() has failed with error = 0x%08X", ::WSAGetLastError());
        // Extract the text based IP address from psRemoteHost
        strIpAddress = ::inet_ntoa(*((struct in_addr *) psRemoteHost->h_addr_list[0]));
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return strIpAddress;
}

/// <summary>
/// 
/// </summary>
/// <param name="strFullJsonString"></param>
/// <param name="strKey"></param>
/// <returns></returns>
static std::string __cdecl GetJsonValue(
    _in const std::string & strFullJsonString,
    _in const std::string & strKey
    )
{
    __DebugFunction();

    std::string strLineWithKey;
    std::string strTempLine;
    std::istringstream oStringStream(strFullJsonString);
    while (std::getline(oStringStream, strTempLine))
    {
        if (std::string::npos != strTempLine.find(strKey))
        {
            strLineWithKey = strTempLine;
            break;
        }
    }
    std::string strStartOfValue = strLineWithKey.substr(strLineWithKey.find(": \"") + 3);

    return strStartOfValue.substr(0, strStartOfValue.find("\""));
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szVerb"></param>
/// <param name="c_szResource"></param>
/// <param name="c_szHost"></param>
/// <param name="c_szContent"></param>
/// <param name="c_szApiVersionDate"></param>
/// <param name="c_szSubscriptionIdentifier"></param>
/// <param name="c_szResourceGroup"></param>
/// <returns></returns>
static std::vector<Byte> __cdecl MakeMicrosoftAzureApiCall(
    _in const std::string & c_strVerb,
    _in const std::string & c_strResource,
    _in const std::string & c_strHost,
    _in const std::string & c_strContent,
    _in const std::string & c_strApiVersionDate,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup
    )
{
    __DebugFunction();

    // Make sure we are not currently logged on
    _ThrowBaseExceptionIf((0 == gs_strMicrosoftAzureAccessToken.size()), "Authentication is required...", nullptr);

    // Build out the API call components
    std::string strApiUri = "/subscriptions/" + c_strSubscriptionIdentifier + "/resourceGroups/" + c_strResourceGroup + "/providers/" + c_strResource + "?api-version=" + c_strApiVersionDate;
    std::vector<std::string> stlHeader;
    stlHeader.push_back("Host: " + c_strHost);
    stlHeader.push_back("Authorization: Bearer " + gs_strMicrosoftAzureAccessToken);
    if (0 != c_strContent.length())
    {
        stlHeader.push_back("Content-Type: application/json");
    }
    stlHeader.push_back("Content-Length: " + std::to_string(c_strContent.size()));

    return ::RestApiCall(c_strHost, 443, c_strVerb, strApiUri, c_strContent, false, stlHeader);
}

/// <summary>
/// Logs into SAIL web services and gets an EOSB string. This is cached internally for future use
/// </summary>
/// <param name="c_szUsername"></param>
/// <param name="c_szPAssword"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl LoginToMicrosoftAzureApiPortal(
    _in const char * c_szApplicationIdentifier,
    _in const char * c_szSecret,
    _in const char * c_szTenantIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool LoginToMicrosoftAzureApiPortal(string applicationIdentifier, string secret, string tenantIdentifier);

    bool fSuccess = false;

    try
    {
        // Make sure we are not currently logged on
        _ThrowBaseExceptionIf((false == gs_strMicrosoftAzureAccessToken.empty()), "Already logged in. Make sure to logout first.", nullptr);

        // Build out the API call components
        std::string strVerb = "POST";
        std::string strApiUri = "/" + std::string(c_szTenantIdentifier) + "/oauth2/token";
        std::string strApiContentBody = "grant_type=client_credentials&client_id=" + std::string(c_szApplicationIdentifier) + "&client_secret=" + std::string(c_szSecret) + "&resource=https://management.core.windows.net/";
        std::vector<std::string> stlHeader;
        stlHeader.push_back("Content-Type: application/x-www-form-urlencoded");
        stlHeader.push_back("Accept: */*");
        std::vector<Byte> stlResponse = ::RestApiCall("login.microsoftonline.com", 443, strVerb, strApiUri, strApiContentBody, false, stlHeader);
        StructuredBuffer oMicrosoftAzureResponse = JsonValue::ParseDataToStructuredBuffer((const char*) stlResponse.data());
        _ThrowBaseExceptionIf((false == oMicrosoftAzureResponse.IsElementPresent("access_token", ANSI_CHARACTER_STRING_VALUE_TYPE)), "Microsoft Azure authentication failed. Response JSON is:\r\n%s", (const char *) oMicrosoftAzureResponse.ToString().c_str());
        gs_strMicrosoftAzureAccessToken = oMicrosoftAzureResponse.GetString("access_token");
        // If we get here, no exceptions were thrown, so the transaction was successfull!!!
        fSuccess = true;
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szSubscriptionIdentifier"></param>
/// <param name="c_szResourceGroup"></param>
/// <param name="c_szVirtualMachineIdentifier"></param>
/// <param name="c_szPublicIpSpecification"></param>
/// <param name="c_szNetworkInterfaceSpecification"></param>
/// <param name="c_szVirtualMachineSpecification"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl CreateResourceGroup(
    _in const char* c_szSubscriptionIdentifier,
    _in const char* c_szResourceGroup,
    _in const char* c_szResourceGroupSpecification
)
{
    __DebugFunction();

    // [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public string ProvisionVirtualMachineAndWait(string subscriptionIdentifier, string resourceGroup, string virtualMachineIdentifier, string publicIpSpecification, string networkInterfaceSpecification, string virtualMachineSpecification);

    try
    {
        // Make sure we are not currently logged on
        _ThrowBaseExceptionIf((0 == gs_strMicrosoftAzureAccessToken.size()), "Authentication required...", nullptr);

        // General settings
        std::string strSubscription = c_szSubscriptionIdentifier;
        std::string strResourceGroup = c_szResourceGroup;
        std::string strResourceGroupSpecification = c_szResourceGroupSpecification;
        // Create a Microsoft Azure public IP address
        std::string strVerb = "PUT";
        std::string strResource = "Microsoft.Network/resourcegroups/" + strResourceGroup;
        std::string strHost = "management.azure.com";
        std::string strContent = c_szResourceGroupSpecification;
        std::string strApiVersionDate = "2020-10-01";

        // Make sure we are currently logged on
        _ThrowBaseExceptionIf((0 == gs_strMicrosoftAzureAccessToken.size()), "Authentication is required...", nullptr);

        // Build out the API call components
        std::string strApiUri = "/subscriptions/" + strSubscription + "/resourceGroups/" + strResourceGroup + "?api-version=" + strApiVersionDate;
        std::vector<std::string> stlHeader;
        stlHeader.push_back("Host: " + strHost);
        stlHeader.push_back("Authorization: Bearer " + gs_strMicrosoftAzureAccessToken);
        if (0 != strContent.length())
        {
            stlHeader.push_back("Content-Type: application/json");
        }
        stlHeader.push_back("Content-Length: " + std::to_string(strContent.size()));

        std::vector<Byte> stlResponse = ::RestApiCall(strHost, 443, strVerb, strApiUri, strContent, false, stlHeader);
        std::string strResponse = (const char*)stlResponse.data();
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to create a Microsoft Azure Resource Group", nullptr);
        _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to create a Microsoft Azure Resource Group with error %s", strResponse.c_str());
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR("Success");
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szSubscriptionIdentifier"></param>
/// <param name="c_szResourceGroup"></param>
/// <param name="c_szVirtualMachineIdentifier"></param>
/// <param name="c_szPublicIpSpecification"></param>
/// <param name="c_szNetworkInterfaceSpecification"></param>
/// <param name="c_szVirtualMachineSpecification"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl DeployVirtualMachineAndWait(
    _in const char* c_szSubscriptionIdentifier,
    _in const char* c_szResourceGroup,
    _in const char* c_szVirtualMachineIdentifier,
    _in const char* c_szconfidentialVirtualMachineSpecification,
    _in const char* c_szLocation
)
{
    __DebugFunction();

    // [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public string ProvisionVirtualMachineAndWait(string subscriptionIdentifier, string resourceGroup, string virtualMachineIdentifier, string publicIpSpecification, string networkInterfaceSpecification, string virtualMachineSpecification);

    std::string strVirtualMachineIpAddress = "0.0.0.0";

    try
    {
        // Make sure we are not currently logged on
        _ThrowBaseExceptionIf((0 == gs_strMicrosoftAzureAccessToken.size()), "Authentication required...", nullptr);

        // General settings
        std::string strSubscription = c_szSubscriptionIdentifier;
        std::string strResourceGroup = c_szResourceGroup;
        std::string strVirtualMachineIdentifier = c_szVirtualMachineIdentifier;
        std::string strLocation = c_szLocation;

        // Create resource group
        std::string resourceGroupSpec = std::string("{\"location\": \"") + strLocation + "\"}";
        ::CreateResourceGroup(c_szSubscriptionIdentifier, c_szResourceGroup, resourceGroupSpec.c_str());

        // Create a Microsoft Azure public IP address
        std::string strVerb = "PUT";
        std::string strResource = "Microsoft.Resources/deployments/" + strVirtualMachineIdentifier + "-deploy";
        std::string strHost = "management.azure.com";
        std::string strContent = c_szconfidentialVirtualMachineSpecification;
        std::string strApiVersionDate = "2020-10-01";
        std::vector<Byte> stlResponse = ::MakeMicrosoftAzureApiCall(strVerb, strResource, strHost, strContent, strApiVersionDate, strSubscription, strResourceGroup);
        stlResponse.push_back(0);
        std::string strResponse = (const char*)stlResponse.data();
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to create a Microsoft Azure public IP address", nullptr);
        _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to create a Microsoft Azure public IP address with error %s", strResponse.c_str());

        // Wait until the deployment is running
        bool fIsRunning = false;
        do
        {
            strVerb = "GET";
            strResource = "Microsoft.Resources/deployments/" + strVirtualMachineIdentifier + "-deploy";
            strHost = "management.azure.com";
            strContent = "";
            strApiVersionDate = "2021-04-01";
            stlResponse = ::MakeMicrosoftAzureApiCall(strVerb, strResource, strHost, strContent, strApiVersionDate, strSubscription, strResourceGroup);
            stlResponse.push_back(0);
            strResponse = (const char*)stlResponse.data();
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to get the status of a virtual machine being provisioned", nullptr);
            _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to get the status of a virtual machine being provisioned with error %s", strResponse.c_str());
            StructuredBuffer oResponse = JsonValue::ParseDataToStructuredBuffer((const char*)stlResponse.data());
            if (true == oResponse.IsElementPresent("properties", INDEXED_BUFFER_VALUE_TYPE))
            {
                StructuredBuffer oProperties(oResponse.GetStructuredBuffer("properties").GetBase64SerializedBuffer().c_str());
                if (true == oProperties.IsElementPresent("provisioningState", ANSI_CHARACTER_STRING_VALUE_TYPE))
                {
                    std::string strProvisioningState = oProperties.GetString("provisioningState");
                    if (strProvisioningState == "Succeeded")
                    {
                        fIsRunning = true;
                    }
                }
            }

            // Should we put the thread to sleep while we wait?
            if (false == fIsRunning)
            {
                ::Sleep(5000);
            }
        } while (false == fIsRunning);

        // Wait until the virtual machine is running
        fIsRunning = false;
        do
        {
            strVerb = "GET";
            strResource = "Microsoft.Compute/virtualMachines/" + strVirtualMachineIdentifier;
            strHost = "management.azure.com";
            strContent = "";
            strApiVersionDate = "2020-12-01";
            stlResponse = ::MakeMicrosoftAzureApiCall(strVerb, strResource, strHost, strContent, strApiVersionDate, strSubscription, strResourceGroup);
            stlResponse.push_back(0);
            strResponse = (const char*)stlResponse.data();
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to get the status of a virtual machine being provisioned", nullptr);
            _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to get the status of a virtual machine being provisioned with error %s", strResponse.c_str());
            StructuredBuffer oResponse = JsonValue::ParseDataToStructuredBuffer((const char*)stlResponse.data());
            if (true == oResponse.IsElementPresent("properties", INDEXED_BUFFER_VALUE_TYPE))
            {
                StructuredBuffer oProperties(oResponse.GetStructuredBuffer("properties").GetBase64SerializedBuffer().c_str());
                if (true == oProperties.IsElementPresent("provisioningState", ANSI_CHARACTER_STRING_VALUE_TYPE))
                {
                    std::string strProvisioningState = oProperties.GetString("provisioningState");
                    if (strProvisioningState == "Succeeded")
                    {
                        fIsRunning = true;
                    }
                }
            }

            // Should we put the thread to sleep while we wait?
            if (false == fIsRunning)
            {
                ::Sleep(5000);
            }
        } while (false == fIsRunning);

        // Now that the virtual machine is running, we go ahead and effectively get the IP address of the
        // virtual machine. This is the last step in the provisioning of a Microsoft Azure virtual machine
        strVerb = "GET";
        strResource = "Microsoft.Network/publicIPAddresses/" + strVirtualMachineIdentifier + "-ip";
        strHost = "management.azure.com";
        strContent = "";
        strApiVersionDate = "2020-07-01";
        stlResponse = ::MakeMicrosoftAzureApiCall(strVerb, strResource, strHost, strContent, strApiVersionDate, strSubscription, strResourceGroup);
        stlResponse.push_back(0);
        strResponse = (const char*)stlResponse.data();
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to get the ip address of a Microsoft Azure virtual machine", nullptr);
        _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to get the ip address of a Microsoft Azure virtual machine with error %s", strResponse.c_str());
        // This won't work as the ParseDataToStructuredBuffer cannot handle the escaped strings. So the workaround is to manually get
        // the Ip Address from the string response.
        strVirtualMachineIpAddress = ::GetJsonValue(std::string((char*)stlResponse.data(), stlResponse.size()), "\"ipAddress\"");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strVirtualMachineIpAddress);
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetExceptionCount(void)
{
    __DebugFunction();

    // [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetExceptionCount();

    unsigned int unNumberOfExceptions = 0;

    try
    {
        unNumberOfExceptions = ::GetRegisteredExceptionCount();
    }

    catch (...)
    {

    }

    return unNumberOfExceptions;
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetNextException(void)
{
    __DebugFunction();

    // [DllImport("MicrosoftAzureApiFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetNextException();

    std::string strException = "";

    try
    {
        strException = ::GetNextRegisteredException();
    }

    catch (...)
    {

    }

    return ::ConvertToBSTR(strException);
}