/*********************************************************************************************
 *
 * @file AzureHelper.cpp
 * @author Prawal Gangwar
 * @date 28 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "AzureHelper.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "CurlRest.h"
#include "StructuredBuffer.h"
#include "JsonValue.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <vector>

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

std::string CreateAzureParamterJson(
    _in const std::string & c_strTemplateUrl,
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    StructuredBuffer oProperties;

    StructuredBuffer oTemplateLink;
    oTemplateLink.PutString("uri", c_strTemplateUrl);
    oProperties.PutStructuredBuffer("templateLink", oTemplateLink);

    StructuredBuffer oParameters;
    for (auto strElement : c_oStructuredBuffer.GetNamesOfElements())
    {
        StructuredBuffer oElement;
        oElement.PutString("value", c_oStructuredBuffer.GetString(strElement.c_str()));
        oParameters.PutStructuredBuffer(strElement.c_str(), oElement);
    }

    oProperties.PutStructuredBuffer("parameters", oParameters);
    oProperties.PutString("mode", "Incremental");

    StructuredBuffer oJson;
    oJson.PutStructuredBuffer("properties", oProperties);

    JsonValue * oOut = JsonValue::ParseStructuredBufferToJson(oJson);

    return oOut->ToString();
}


/********************************************************************************************
 *
 * @function GetJsonValue
 * @brief Function to get small json values which exist in the same line as the key
 * @param[in] strFullJsonString Json string to read the value from
 * @param[in] strKey Key for which the value is needed
 * @return Value corresponding to that key
 * @note This is not a perfect function to get a value form the Json object.
 *      This function would just find the first line with the key value in the format:
 *      "key" : "value"
 *      and return the value. This is the most that was needed in the Azure class.
 *      For other operation we would need a full-fledged Json Module.
 *
 ********************************************************************************************/

std::string __thiscall GetJsonValue(
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
        if (strTempLine.find(strKey) != std::string::npos)
        {
            strLineWithKey = strTempLine;
            break;
        }
    }

    std::string strStartOfValue = strLineWithKey.substr(strLineWithKey.find(": \"")+3);
    return strStartOfValue.substr(0, strStartOfValue.find("\""));
}


std::string __stdcall LoginToMicrosoftAzureApiPortal(
    _in const std::string & c_szApplicationIdentifier,
    _in const std::string & c_szSecret,
    _in const std::string & c_szTenantIdentifier
    )
{
    __DebugFunction();

    std::string strAccessToken = "";

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

    strAccessToken = oMicrosoftAzureResponse.GetString("access_token");

    return strAccessToken;
}


/********************************************************************************************
 *
 * @function GetJsonValue
 * @brief Function to get small json values which exist in the same line as the key
 * @param[in] strFullJsonString Json string to read the value from
 * @param[in] strKey Key for which the value is needed
 * @return Value corresponding to that key
 * @note This is not a perfect function to get a value form the Json object.
 *
 ********************************************************************************************/

static std::vector<Byte> __cdecl MakeMicrosoftAzureApiCall(
    _in const std::string & c_strMicrosoftAzureAccessToken,
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
    _ThrowBaseExceptionIf((0 == c_strMicrosoftAzureAccessToken.size()), "Authentication is required...", nullptr);

    // Build out the API call components
    std::string strApiUri = "/subscriptions/" + c_strSubscriptionIdentifier + "/resourceGroups/" + c_strResourceGroup + "/providers/" + c_strResource + "?api-version=" + c_strApiVersionDate;
    std::vector<std::string> stlHeader;
    stlHeader.push_back("Host: " + c_strHost);
    stlHeader.push_back("Authorization: Bearer " + c_strMicrosoftAzureAccessToken);
    if (0 != c_strContent.length())
    {
        stlHeader.push_back("Content-Type: application/json");
    }
    stlHeader.push_back("Content-Length: " + std::to_string(c_strContent.size()));

    return ::RestApiCall(c_strHost, 443, c_strVerb, strApiUri, c_strContent, false, stlHeader);
}


std::string __stdcall CreateResourceGroup(
    _in const std::string & c_strMicrosoftAzureAccessToken,
    _in const std::string & c_szSubscriptionIdentifier,
    _in const std::string & c_szResourceGroup,
    _in const std::string & c_szResourceGroupSpecification
)
{
    __DebugFunction();

    // Make sure we are not currently logged on
    _ThrowBaseExceptionIf((0 == c_strMicrosoftAzureAccessToken.size()), "Authentication required...", nullptr);

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

    // Build out the API call components
    std::string strApiUri = "/subscriptions/" + strSubscription + "/resourceGroups/" + strResourceGroup + "?api-version=" + strApiVersionDate;
    std::vector<std::string> stlHeader;
    stlHeader.push_back("Host: " + strHost);
    stlHeader.push_back("Authorization: Bearer " + c_strMicrosoftAzureAccessToken);
    if (0 != strContent.length())
    {
        stlHeader.push_back("Content-Type: application/json");
    }
    stlHeader.push_back("Content-Length: " + std::to_string(strContent.size()));

    std::vector<Byte> stlResponse = ::RestApiCall(strHost, 443, strVerb, strApiUri, strContent, false, stlHeader);
    std::string strResponse = (const char*)stlResponse.data();
    _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to create a Microsoft Azure Resource Group", nullptr);
    _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to create a Microsoft Azure Resource Group with error %s", strResponse.c_str());

    return "Success";
}

/********************************************************************************************
 *
 * @function DeployVirtualMachineAndWait
 * @brief Function to get small json values which exist in the same line as the key
 * @param[in] c_szApplicationIdentifier
 * @param[in] c_szSecret
 * @param[in] c_szTenantIdentifier
 * @param[in] c_szSubscriptionIdentifier
 * @param[in] c_szResourceGroup
 * @param[in] c_szVirtualMachineIdentifier
 * @param[in] c_szconfidentialVirtualMachineSpecification
 * @param[in] c_szLocation
 * @return Ip Address of the newly created Virtual Machine
 * @note This is a blocking call
 *
 ********************************************************************************************/
std::string DeployVirtualMachineAndWait(
    _in const std::string & c_szApplicationIdentifier,
    _in const std::string & c_szSecret,
    _in const std::string & c_szTenantIdentifier,
    _in const std::string & c_szSubscriptionIdentifier,
    _in const std::string & c_szResourceGroup,
    _in const std::string & c_szVirtualMachineIdentifier,
    _in const std::string & c_szconfidentialVirtualMachineSpecification,
    _in const std::string & c_szLocation
)
{
    __DebugFunction();

    std::string strVirtualMachineIpAddress = "0.0.0.0";

    // Login to the Microsoft Azure API Portal
    const std::string c_strMicrosoftAzureAccessToken = ::LoginToMicrosoftAzureApiPortal(c_szApplicationIdentifier, c_szSecret, c_szTenantIdentifier);
    _ThrowBaseExceptionIf((0 == c_strMicrosoftAzureAccessToken.length()), "Authentication failed...", nullptr);

    // TODO: Prawal use the CreateAzureDeployment function here
    // Create resource group
    std::string resourceGroupSpec = std::string("{\"location\": \"") + c_szLocation + "\"}";
    ::CreateResourceGroup(c_strMicrosoftAzureAccessToken, c_szSubscriptionIdentifier, c_szResourceGroup, resourceGroupSpec.c_str());

    // Create a Virtual Machine with a pre-existing Azure Template
    std::string strVerb = "PUT";
    std::string strResource = "Microsoft.Resources/deployments/" + c_szVirtualMachineIdentifier + "-deploy";
    std::string strHost = "management.azure.com";
    std::string strContent = c_szconfidentialVirtualMachineSpecification;
    std::string strApiVersionDate = "2020-10-01";
    std::vector<Byte> stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, strContent, strApiVersionDate, c_szSubscriptionIdentifier, c_szResourceGroup);
    stlResponse.push_back(0);
    std::string strResponse = (const char*)stlResponse.data();
    _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to create a Microsoft Azure public IP address", nullptr);
    _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to create a Microsoft Azure public IP address with error %s", strResponse.c_str());

    // Wait until the deployment is running
    bool fIsRunning = false;
    do
    {
        strVerb = "GET";
        strResource = "Microsoft.Resources/deployments/" + c_szVirtualMachineIdentifier + "-deploy";
        strHost = "management.azure.com";
        strContent = "";
        strApiVersionDate = "2021-04-01";
        stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, strContent, strApiVersionDate, c_szSubscriptionIdentifier, c_szResourceGroup);
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

        // Put the thread to sleep while we wait
        if (false == fIsRunning)
        {
            ::sleep(5);
        }
    } while (false == fIsRunning);

    // Wait until the virtual machine is running
    fIsRunning = false;
    do
    {
        strVerb = "GET";
        strResource = "Microsoft.Compute/virtualMachines/" + c_szVirtualMachineIdentifier;
        strHost = "management.azure.com";
        strContent = "";
        strApiVersionDate = "2020-12-01";
        stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, strContent, strApiVersionDate, c_szSubscriptionIdentifier, c_szResourceGroup);
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
            ::sleep(5);
        }
    } while (false == fIsRunning);

    // Now that the virtual machine is running, we go ahead and effectively get the IP address of the
    // virtual machine. This is the last step in the provisioning of a Microsoft Azure virtual machine
    strVerb = "GET";
    strResource = "Microsoft.Network/publicIPAddresses/" + c_szVirtualMachineIdentifier + "-ip";
    strHost = "management.azure.com";
    strContent = "";
    strApiVersionDate = "2020-07-01";
    stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, strContent, strApiVersionDate, c_szSubscriptionIdentifier, c_szResourceGroup);
    stlResponse.push_back(0);
    strResponse = (const char*)stlResponse.data();
    _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to get the ip address of a Microsoft Azure virtual machine", nullptr);
    _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to get the ip address of a Microsoft Azure virtual machine with error %s", strResponse.c_str());

    // ParseDataToStructuredBuffer cannot handle the escaped strings. The workaround is to manually get
    // the Ip Address from the string response.
    strVirtualMachineIpAddress = ::GetJsonValue(std::string((char*)stlResponse.data(), stlResponse.size()), "\"ipAddress\"");

    return strVirtualMachineIpAddress;
}

/********************************************************************************************
 *
 * @function CreateAzureDeployment
 * @brief Deploy a Azure template with parameters
 * @param[in] c_strApplicationIdentifier
 * @param[in] c_strSecret
 * @param[in] c_strTenantIdentifier
 * @param[in] c_strSubscriptionIdentifier
 * @param[in] c_strResourceGroup
 * @param[in] c_strVirtualNetworkIdentifier
 * @param[in] c_strLocation
 * @return Id of the created resource
 * @note This is a blocking call
 *
 ********************************************************************************************/

std::string CreateAzureDeployment(
    _in const std::string & c_strDeploymentParameters,
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strLocation
)
{
    __DebugFunction();

    // TODO: Prawal put this properly
    std::string strResourceId = "abcd";

    // Login to the Microsoft Azure API Portal
    const std::string c_strMicrosoftAzureAccessToken = ::LoginToMicrosoftAzureApiPortal(c_strApplicationIdentifier, c_strSecret, c_strTenantIdentifier);
    _ThrowBaseExceptionIf((0 == c_strMicrosoftAzureAccessToken.length()), "Authentication failed...", nullptr);

    // Create resource group
    std::string resourceGroupSpec = std::string("{\"location\": \"") + c_strLocation + "\"}";
    ::CreateResourceGroup(c_strMicrosoftAzureAccessToken, c_strSubscriptionIdentifier, c_strResourceGroup, resourceGroupSpec.c_str());

    // Create a Virtual Machine with a pre-existing Azure Template
    std::string strVerb = "PUT";
    std::string strResource = "Microsoft.Resources/deployments/sail-" + Guid().ToString(eRaw) + "-deploy";
    std::string strHost = "management.azure.com";

    std::string strApiVersionDate = "2020-10-01";
    std::vector<Byte> stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, c_strDeploymentParameters, strApiVersionDate, c_strSubscriptionIdentifier, c_strResourceGroup);
    stlResponse.push_back(0);
    std::string strResponse = (const char*)stlResponse.data();
    _ThrowBaseExceptionIf((0 == stlResponse.size()), "Failed to create a Microsoft Azure public IP address", nullptr);
    _ThrowBaseExceptionIf((std::string::npos != strResponse.find("error")), "Failed to create a Microsoft Azure public IP address with error %s", strResponse.c_str());

    // Wait until the deployment is running
    bool fIsRunning = false;
    do
    {
        strVerb = "GET";
        strApiVersionDate = "2021-04-01";
        stlResponse = ::MakeMicrosoftAzureApiCall(c_strMicrosoftAzureAccessToken, strVerb, strResource, strHost, "", strApiVersionDate, c_strSubscriptionIdentifier, c_strResourceGroup);
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

        // Put the thread to sleep while we wait
        if (false == fIsRunning)
        {
            ::sleep(5);
        }
    } while (false == fIsRunning);

    // TODO: Prawal: get the resourceId

    return strResourceId;
}

/********************************************************************************************
 *
 * @function DoesAzureResourceExist
 * @brief Check if an Azure resource exists
 * @param[in] c_strApplicationIdentifier
 * @param[in] c_strSecret
 * @param[in] c_strTenantIdentifier
 * @param[in] c_strResourceId
 * @return true if exits, false otherwise
 *
 ********************************************************************************************/

bool DoesAzureResourceExist(
    _in const std::string & c_strApplicationIdentifier,
    _in const std::string & c_strSecret,
    _in const std::string & c_strTenantIdentifier,
    _in const std::string & c_strResourceId
)
{
    __DebugFunction();

    bool fResourceExist = false;

    // Login to the Microsoft Azure API Portal
    const std::string c_strMicrosoftAzureAccessToken = ::LoginToMicrosoftAzureApiPortal(c_strApplicationIdentifier, c_strSecret, c_strTenantIdentifier);
    _ThrowBaseExceptionIf((0 == c_strMicrosoftAzureAccessToken.length()), "Azure Authentication failed...", nullptr);

    std::string strVerb = "GET";
    std::string strContent = "";
    std::string strApiUri = c_strResourceId + "?api-version=2014-12-01-preview";
    std::string strHost = "management.azure.com";

    std::vector<std::string> stlHeader;
    stlHeader.push_back("Host: " + strHost);
    stlHeader.push_back("Authorization: Bearer " + c_strMicrosoftAzureAccessToken);
    stlHeader.push_back("Content-Length: " + std::to_string(strContent.length()));

    long nResponseCode = 0;
    std::vector<Byte> stlResponse = ::RestApiCall(strHost, 443, strVerb, strApiUri, "", false, stlHeader, &nResponseCode);

    std::cout << "nResponseCode " << nResponseCode;
    if (200 == nResponseCode)
    {
        fResourceExist = true;
    }
    else if (404 == nResponseCode)
    {
        fResourceExist = false;
    }
    else
    {
        _ThrowBaseException("Invalid Response for the resource check. Response code: %d", nResponseCode);
    }

    return fResourceExist;
}

std::string CreateAzureResourceId(
    _in const std::string & c_strSubscriptionIdentifier,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strResourceProviderNamespace,
    _in const std::string & c_strResourceType,
    _in const std::string & c_strResourceName
)
{
    __DebugFunction();

    std::string gc_strAzureIdFormat = "/subscriptions/{{SubscriptionUuid}}/resourceGroups/{{ResourceGroup}}/{{ResourceProvidernamespcae}}/{{ResourceType}}/{{ResourceName}}";

    ::ReplaceAll(gc_strAzureIdFormat, "{{SubscriptionUuid}}", c_strSubscriptionIdentifier);
    ::ReplaceAll(gc_strAzureIdFormat, "{{ResourceGroup}}", c_strResourceGroup);
    ::ReplaceAll(gc_strAzureIdFormat, "{{ResourceProvidernamespcae}}", c_strResourceProviderNamespace);
    ::ReplaceAll(gc_strAzureIdFormat, "{{ResourceType}}", c_strResourceType);
    ::ReplaceAll(gc_strAzureIdFormat, "{{ResourceName}}", c_strResourceName);

    return gc_strAzureIdFormat;
}
