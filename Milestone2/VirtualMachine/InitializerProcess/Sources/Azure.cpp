/*********************************************************************************************
 *
 * @file Azure.cpp
 * @author Prawal Gangwar
 * @date 03 Feb 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Azure class to interact with Azure Web APIs
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "Azure.h"
#include "DebugLibrary.h"
#include "TlsClient.h"
#include "Exceptions.h"

#include <netdb.h>
#include <memory>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <sstream>

/********************************************************************************************
 *
 * @function ReplaceAll
 * @brief Replace all the instances of c_strChangeThis to c_strChangeTo
 * @param[inout] strOriginalString Original string which needs to be modified
 * @param[in] c_strChangeThis Original string which needs to be replaced
 * @param[in] c_strChangeTo The string which is put at the replaced position
 *
 ********************************************************************************************/

void __thiscall ReplaceAll(
    _inout std::string & strOriginalString,
    _in const std::string & c_strChangeThis,
    _in const std::string & c_strChangeTo)
{
    size_t start_pos = 0;
    while((start_pos = strOriginalString.find(c_strChangeThis, start_pos)) != std::string::npos)
    {
        strOriginalString.replace(start_pos, c_strChangeThis.length(), c_strChangeTo);
        start_pos += c_strChangeTo.length();
    };
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

/********************************************************************************************
 *
 * @class Azure
 * @function Azure
 * @brief Constructor to initialize and authenticate with Azure and get an auth-token
 * @param[in] strSecret Name of the UNIX domain socket
 * @param[in] strSecret Name of the UNIX domain socket
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

Azure::Azure(
    _in const std::string & c_strAppId,
    _in const std::string & c_strSecret,
    _in const std::string & c_strSubcriptionID,
    _in const std::string & c_strTenant,
    _in const std::string & c_strResourceGroup,
    _in const std::string & c_strLocation
    )
    : m_strAppId(c_strAppId), m_strSecret(c_strSecret), m_strSubscriptionId(c_strSubcriptionID), m_strTenant(c_strTenant), m_strResourceGroup(c_strResourceGroup), m_strLocation(c_strLocation)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class Azure
 * @function ~Azure
 * @brief Destructor
 *
 ********************************************************************************************/

Azure::~Azure(void)
{
    __DebugFunction();

}


/********************************************************************************************
 *
 * @class Azure
 * @function Azure
 * @brief Initialize and authenticate with Azure and get an auth-token
 * @param[in] strSecret Name of the UNIX domain socket
 * @param[in] strSecret Name of the UNIX domain socket
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

std::string __thiscall Azure::ProvisionVirtualMachine(void)
{
    __DebugFunction();

    Guid oGuidVirtualMachine;
    std::string strVirtualMachineName = oGuidVirtualMachine.ToString(eRaw);

    // Create an Public Ip Object
    std::string strPublicIpSpec = this->CompleteTemplate("PublicIpAddress.json");
    this->MakeRestCall("PUT", "Microsoft.Network/publicIPAddresses/" + strVirtualMachineName + "-ip", "management.azure.com", strPublicIpSpec, "2020-07-01");

    // Create a network interface for the Virtual Machine
    std::string strNetworkInterfaceSpec = this->CompleteTemplate("NetworkInterface.json");
    ReplaceAll(strNetworkInterfaceSpec, "{{Name}}", strVirtualMachineName + "-nic");
    ReplaceAll(strNetworkInterfaceSpec, "{{IpAddressId}}", strVirtualMachineName + "-ip");
    this->MakeRestCall("PUT", "Microsoft.Network/networkInterfaces/" + strVirtualMachineName + "-nic", "management.azure.com", strNetworkInterfaceSpec, "2020-07-01");

    // Create the Virtual Machine on the cloud
    std::string strVirtualMachineSpec = this->CompleteTemplate("VmConfig.json");
    ReplaceAll(strVirtualMachineSpec, "{{OsDiskName}}", strVirtualMachineName + "-disk");
    ReplaceAll(strVirtualMachineSpec, "{{NetworkInterface}}", strVirtualMachineName + "-nic");
    this->MakeRestCall("PUT", "Microsoft.Compute/virtualMachines/" + strVirtualMachineName, "management.azure.com", strVirtualMachineSpec, "2020-12-01");

    // When the VM is created, the first provisioning status is "Creating" indicating that the
    // VM provisioning has started but at that time the final creation status is unknown
    // So every 10 seconds another request is made to check for the status and the call is
    // blocked until the final status changes to success or failure.
    std::string strVmProvisioningState =  this->GetVmProvisioningState(strVirtualMachineName);
    while ("Creating" == strVmProvisioningState)
    {
        ::sleep(10);
        strVmProvisioningState = this->GetVmProvisioningState(strVirtualMachineName);
    }

    _ThrowBaseExceptionIf(("Failed" == strVmProvisioningState), "Failed to Provision the VM.", nullptr);
    _ThrowBaseExceptionIf(("Cancelled" == strVmProvisioningState), "Cancelled provisioning VM.", nullptr);

    return strVirtualMachineName;
}

std::string __thiscall Azure::GetVmProvisioningState(
    _in std::string & c_strVirtualMachineName
)
{
    __DebugFunction();

    std::string strVmProvisioningStatusJson =  this->MakeRestCall("GET", "Microsoft.Compute/virtualMachines/" + c_strVirtualMachineName, "management.azure.com", "", "2020-12-01");

    return ::GetJsonValue(strVmProvisioningStatusJson, "\"provisioningState\"");
}

std::string __thiscall Azure::GetVmIp(
    _in std::string & c_strVmName
)
{
    __DebugFunction();

    std::string strVmIpRestResponse = this->MakeRestCall("GET", "Microsoft.Network/publicIPAddresses/" + c_strVmName + "-ip", "management.azure.com", "", "2020-07-01");

    return ::GetJsonValue(strVmIpRestResponse, "\"ipAddress\"");
}

bool __thiscall Azure::DeleteVirtualMachine(
    _in const std::string & c_strVirtualMachineName
)
{
    __DebugFunction();

    // Delete the Virtual Machine
    this->MakeRestCall("DELETE", "Microsoft.Compute/virtualMachines/" + c_strVirtualMachineName, "management.azure.com", "", "2020-12-01");

    // Delete the Network Interface
    this->MakeRestCall("DELETE", "Microsoft.Network/networkInterfaces/" + c_strVirtualMachineName + "-nic", "management.azure.com", "", "2020-07-01");

    // Delete the IP Address Object
    this->MakeRestCall("DELETE", "Microsoft.Network/publicIPAddresses/" + c_strVirtualMachineName + "-ip", "management.azure.com", "", "2020-07-01");

    return true;
}

/********************************************************************************************
 *
 * @class Azure
 * @function Authenticate
 * @brief Authenticate with Azure and get an auth-token
 * @return true if the authentication is successful, otherwise false
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

bool __thiscall Azure::Authenticate(void)
{
    __DebugFunction();

    bool fAuthenticationStatus = false;

    std::string strRestRequestBody = "grant_type=client_credentials&client_id=" + m_strAppId + "&client_secret=" + m_strSecret + "&resource=https://management.core.windows.net/";

    std::string strRestRequestHeader = "POST /" + m_strTenant + "/oauth2/token HTTP/1.1\r\n"
                                       "Host: login.microsoftonline.com\r\n"
                                       "Content-Type: application/x-www-form-urlencoded\r\n"
                                       "Content-Length: " + std::to_string(strRestRequestBody.length()) + "\r\n\r\n";

    std::string strRestRequest = strRestRequestHeader + strRestRequestBody + "\r\n\r\n";

    try
    {
        struct hostent * oHostent = ::gethostbyname("login.microsoftonline.com");
        _ThrowIfNull(oHostent, "No DNS mapping for login.microsoftonline.com", nullptr);

        std::unique_ptr<TlsNode> poTlsNode(::TlsConnectToNetworkSocket(inet_ntoa (*((struct in_addr *)oHostent->h_addr_list[0])), 443));
        poTlsNode->Write((const Byte *)strRestRequest.c_str(), strRestRequest.length());

        std::string strResponseString;
        std::vector<Byte> oResponseByte = poTlsNode->Read(1, 5000);
        while(0 != oResponseByte.size())
        {
            strResponseString.push_back(oResponseByte.at(0));
            oResponseByte = poTlsNode->Read(1, 100);
        }

        std::vector<Byte> strSerailizedStructuredBuffer = JsonValue::ParseDataToStructuredBuffer(strResponseString.substr(strResponseString.find_last_of("\r\n\r\n")).c_str());
        StructuredBuffer oStructuredBufferOfResponse(strSerailizedStructuredBuffer);
        m_strAuthToken = oStructuredBufferOfResponse.GetString("access_token");
        fAuthenticationStatus = true;
    }
    catch(BaseException & oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << '\n';
    }

    return fAuthenticationStatus;
}

std::string __thiscall Azure::MakeRestCall(
    _in const std::string c_strVerb,
    _in const std::string c_strResource,
    _in const std::string c_strHost,
    _in const std::string & c_strBody,
    _in const std::string c_strApiVersionDate
)
{
    __DebugFunction();

    std::string strResponseString;

    if (0 == m_strAuthToken.length())
    {
        this->Authenticate();
    }
    std::string strRestRequestHeader = c_strVerb + " /subscriptions/" + m_strSubscriptionId + "/resourceGroups/cloud-shell-storage-centralindia/providers/" + c_strResource + "?api-version=" + c_strApiVersionDate + " HTTP/1.1\r\n"
                                       "Host: "+ c_strHost + "\r\n"
                                       "Authorization: Bearer "+ m_strAuthToken + "\r\n"
                                       "Content-Type: application/json\r\n"
                                       "Content-Length: " + std::to_string(c_strBody.length()) + "\r\n\r\n";

    std::string strRestRequest = strRestRequestHeader + c_strBody + "\r\n\r\n";
    try
    {
        struct hostent * oHostent = ::gethostbyname("management.azure.com");
        _ThrowIfNull(oHostent, "No DNS mapping for management.azure.com", nullptr);

        std::unique_ptr<TlsNode> poTlsNode(::TlsConnectToNetworkSocket(inet_ntoa (*((struct in_addr *)oHostent->h_addr_list[0])), 443));
        poTlsNode->Write((const Byte *)strRestRequest.c_str(), strRestRequest.length());

        std::vector<Byte> oResponseByte = poTlsNode->Read(1, 5000);
        while(0 != oResponseByte.size())
        {
            strResponseString.push_back(oResponseByte.at(0));
            oResponseByte = poTlsNode->Read(1, 100);
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception caught: " << oBaseException.GetExceptionMessage() << '\n';
    }

    return strResponseString;
}

std::string Azure::CompleteTemplate(
    _in std::string c_strFileName
)
{
    __DebugFunction();

    std::ifstream file(c_strFileName, std::ios::ate);
    std::streamsize nSizeOfJsonFile = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string strRestRequestBody;
    strRestRequestBody.resize(nSizeOfJsonFile);
    file.read(strRestRequestBody.data(), nSizeOfJsonFile);

    ReplaceAll(strRestRequestBody, "{{SubscriptionId}}", m_strSubscriptionId);
    ReplaceAll(strRestRequestBody, "{{ResourceGroup}}", m_strResourceGroup);
    ReplaceAll(strRestRequestBody, "{{Location}}", m_strLocation);

    return strRestRequestBody;
}