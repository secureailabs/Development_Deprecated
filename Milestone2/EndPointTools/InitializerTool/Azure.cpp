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
#include "JsonValue.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>

std::string __stdcall GeneratePassword(
    _in const unsigned int c_unPasswordLength
)
{
    __DebugFunction();

    std::string strPasswordCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_-+={[}]|.?/";
    std::string strGeneratedPassword = "";

    for(unsigned int i = 0; i < c_unPasswordLength; i++)
    {
        strGeneratedPassword.push_back(strPasswordCharacters.at(::rand() % strPasswordCharacters.length()));
    }

    return strGeneratedPassword;
}

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
    try
    {
        size_t start_pos = 0;
        while((start_pos = strOriginalString.find(c_strChangeThis, start_pos)) != std::string::npos)
        {
            strOriginalString.replace(start_pos, c_strChangeThis.length(), c_strChangeTo);
            start_pos += c_strChangeTo.length();
        };
    }
    catch(const std::exception & oException)
    {
        std::cout << "std Exception in RepaceAll " << oException.what() << '\n';
    }
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

StructuredBuffer __thiscall GetHttpBodyJson(
    _in const std::string & strHttpResponse
)
{
    __DebugFunction();

    std::string strJsonString;
    std::size_t unHeaderEndPosition = strHttpResponse.find("\r\n\r\n") + 4;
    if (std::string::npos != unHeaderEndPosition)
    {
        strJsonString = strHttpResponse.substr(unHeaderEndPosition, (strHttpResponse.length() - unHeaderEndPosition));
    }

    StructuredBuffer oViewInstanceResponse(JsonValue::ParseDataToStructuredBuffer(strJsonString.c_str()));
    return oViewInstanceResponse;
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
    _in const std::string & c_strNetworkSecurityGroup,
    _in const std::string & c_strLocation
    )
    : m_strAppId(c_strAppId), m_strSecret(c_strSecret), m_strSubscriptionId(c_strSubcriptionID), m_strTenant(c_strTenant), m_strNetworkSecurityGroup(c_strNetworkSecurityGroup), m_strLocation(c_strLocation)
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
 * @function CreateResourceGroup
 * @brief Create a Resource Group for the Azure.
 * @param[in] c_strResourceGroupName Name of the Resource Group
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

bool __thiscall Azure::CreateResourceGroup(
    _in const std::string c_strResourceGroupName
)
{
    __DebugFunction();

    std::string strResponseString;

    if (0 == m_strAuthToken.length())
    {
        this->Authenticate();
    }

    const std::string c_strBody = this->CompleteTemplate("ResourceGroup.json");
    std::string strRestRequestHeader = "PUT /subscriptions/" + m_strSubscriptionId + "/resourcegroups/" + c_strResourceGroupName + "?api-version=2020-06-01" + " HTTP/1.1\r\n"
                                       "Host: management.azure.com\r\n"
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

    std::cout << strResponseString << std::endl;
    return true;
}

/********************************************************************************************
 *
 * @class Azure
 * @function SetResourceGroup
 * @brief Set the Resource Group for the class
 * @param[in] c_strResourceGroupName Name of the Resource Group
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

void __thiscall Azure::SetResourceGroup(
    _in const std::string c_strResourceGroupName
)
{
    m_strResourceGroup = c_strResourceGroupName;
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

std::string __thiscall Azure::ProvisionVirtualMachine(
    _in const std::string c_strBaseImageName,
    _in const std::string c_strVirtualMachineSize,
    _in const std::string c_strDnsLabel,
    _in const std::string c_strPassword
)
{
    __DebugFunction();

    // Create a name for the Virtual Machine which is just a Guid.
    Guid oGuidVirtualMachine;
    std::string strVirtualMachineName = oGuidVirtualMachine.ToString(eRaw);

    // Create an Public Ip Object
    std::string strPublicIpSpec = this->CompleteTemplate("PublicIpAddress.json");
    if ("" == c_strDnsLabel)
    {
        std::string strLowerCaseGuid = "sail"+strVirtualMachineName;
        std::transform(strLowerCaseGuid.begin(), strLowerCaseGuid.end(), strLowerCaseGuid.begin(), ::tolower);
        ::ReplaceAll(strPublicIpSpec, "{{DnsLabel}}", strLowerCaseGuid);
    }
    else
    {
        ::ReplaceAll(strPublicIpSpec, "{{DnsLabel}}", c_strDnsLabel);
    }
    std::string strRestResponse = this->MakeRestCall("PUT", "Microsoft.Network/publicIPAddresses/" + strVirtualMachineName + "-ip", "management.azure.com", strPublicIpSpec, "2020-07-01");
    // std::cout << "Ip response \n\n" << strRestResponse << "\n\n";
    _ThrowBaseExceptionIf((0 == strRestResponse.length()), "Request timed out", nullptr);

    // Create a network interface for the Virtual Machine
    std::string strNetworkInterfaceSpec = this->CompleteTemplate("NetworkInterface.json");
    ::ReplaceAll(strNetworkInterfaceSpec, "{{Name}}", strVirtualMachineName + "-nic");
    ::ReplaceAll(strNetworkInterfaceSpec, "{{IpAddressId}}", strVirtualMachineName + "-ip");
    strRestResponse = this->MakeRestCall("PUT", "Microsoft.Network/networkInterfaces/" + strVirtualMachineName + "-nic", "management.azure.com", strNetworkInterfaceSpec, "2020-07-01");
    // std::cout << "NIC response \n\n" << strRestResponse << "\n\n";
    _ThrowBaseExceptionIf((0 == strRestResponse.length()), "Request timed out", nullptr);

    // Create the Virtual Machine on the cloud
    std::string strVirtualMachineSpec = this->CompleteTemplate("VmFromImage.json");
    ::ReplaceAll(strVirtualMachineSpec, "{{OsDiskName}}", strVirtualMachineName + "-disk");
    ::ReplaceAll(strVirtualMachineSpec, "{{NetworkInterface}}", strVirtualMachineName + "-nic");
    ::ReplaceAll(strVirtualMachineSpec, "{{ImageName}}", c_strBaseImageName);
    ::ReplaceAll(strVirtualMachineSpec, "{{VmSize}}", c_strVirtualMachineSize);
    ::ReplaceAll(strVirtualMachineSpec, "{{Password}}", c_strPassword);
    ::ReplaceAll(strVirtualMachineSpec, "{{Username}}", strVirtualMachineName);
    ::ReplaceAll(strVirtualMachineSpec, "{{ComputerName}}", "SailVirtualMachine");
    strRestResponse = this->MakeRestCall("PUT", "Microsoft.Compute/virtualMachines/" + strVirtualMachineName, "management.azure.com", strVirtualMachineSpec, "2020-12-01");
    // std::cout << "VM response \n\n" << strRestResponse << "\n\n";
    _ThrowBaseExceptionIf((0 == strRestResponse.length()), "Request timed out", nullptr);

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

/********************************************************************************************
 *
 * @class Azure
 * @function CreateVirtualNetwork
 * @brief Creates a Virtual Network on the cloud
 * @param[in] c_strVirtualMachineName Name of the virtual Network
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

std::string __thiscall Azure::CreateVirtualNetwork(
    _in std::string c_strVirtualNetworkName
)
{
    __DebugFunction();

    std::string strVirtualNetworkSpec = this->CompleteTemplate("VirtualNetwork.json");
    std::string strVirtualNetworkStatusJson = this->MakeRestCall("PUT", "Microsoft.Network/virtualNetworks/" + c_strVirtualNetworkName, "management.azure.com", strVirtualNetworkSpec, "2020-07-01");

    StructuredBuffer oResponseJson = ::GetHttpBodyJson(strVirtualNetworkStatusJson);

    return oResponseJson.GetStructuredBuffer("properties").GetString("provisioningState");
}

/********************************************************************************************
 *
 * @class Azure
 * @function SetVirtualNetwork
 * @brief Creates a Virtual Network on the cloud
 * @param[in] c_strVirtualMachineName Name of the virtual Network
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

void __thiscall Azure::SetVirtualNetwork(
    _in const std::string c_strVirtualNetworkName
)
{
    __DebugFunction();

    m_strVirtualNetwork = c_strVirtualNetworkName;
}

/********************************************************************************************
 *
 * @class Azure
 * @function GetVmProvisioningState
 * @brief Returns the provisioning state of the VM
 * @param[in] c_strVirtualMachineName Name of the virtual machine
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

std::string __thiscall Azure::GetVmProvisioningState(
    _in std::string & c_strVirtualMachineName
)
{
    __DebugFunction();

    std::string strVmProvisioningStatusJson = this->MakeRestCall("GET", "Microsoft.Compute/virtualMachines/" + c_strVirtualMachineName, "management.azure.com", "", "2020-12-01");

    StructuredBuffer oResponseJson = ::GetHttpBodyJson(strVmProvisioningStatusJson);
    return oResponseJson.GetStructuredBuffer("properties").GetString("provisioningState");
}

/********************************************************************************************
 *
 * @class Azure
 * @function GetVmIp
 * @brief Returns the public IP of the VM
 * @param[in] c_strVirtualMachineName Name of the virtual machine
 * @return Authenticated Azure
 * @throw BaseException if the credentials fail
 *
 ********************************************************************************************/

std::string __thiscall Azure::GetVmIp(
    _in std::string & c_strVirtualMachineName
)
{
    __DebugFunction();

    std::string strPublicIpAddress;
    try
    {
        std::string strVmIpRestResponse = this->MakeRestCall("GET", "Microsoft.Network/publicIPAddresses/" + c_strVirtualMachineName + "-ip", "management.azure.com", "", "2020-07-01");
        strPublicIpAddress = ::GetJsonValue(strVmIpRestResponse, "\"ipAddress\"");
        _ThrowBaseExceptionIf((0 == strPublicIpAddress.length()), "Ip address length zero", nullptr);
    }
    catch(...)
    {
        _ThrowBaseException("Get Public Ip Address of Virtual Machine failed.", nullptr);
    }

    // TODO: Dont use this eventually
    return strPublicIpAddress;

    // StructuredBuffer oResponseJson = ::GetHttpBodyJson(strVmIpRestResponse);
    // return oResponseJson.GetStructuredBuffer("properties").GetString("ipAddress");
}

/********************************************************************************************
 *
 * @class Azure
 * @function DeleteVirtualMachine
 * @brief Deletes the VM instance
 * @param[in] c_strVirtualMachineName Name of the virtual machine
 * @return true on success, else false
 *
 ********************************************************************************************/

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

/********************************************************************************************
 *
 * @class Azure
 * @function MakeRestCall
 * @brief Helper function to make REST calls to Azure API
 * @return REST response from the server
 *
 ********************************************************************************************/

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
    std::string strRestRequestHeader = c_strVerb + " /subscriptions/" + m_strSubscriptionId + "/resourceGroups/" + m_strResourceGroup + "/providers/" + c_strResource + "?api-version=" + c_strApiVersionDate + " HTTP/1.1\r\n"
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
        std::vector<Byte> oResponseByte = poTlsNode->Read(1, 60*1000);
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
    catch(...)
    {
        std::cout << "Bad exception :( !!!!\n";
    }

    return strResponseString;
}

/********************************************************************************************
 *
 * @class Azure
 * @function CompleteTemplate
 * @brief Helper function to make to complete the json config template
 * @return The completed string
 *
 ********************************************************************************************/

std::string Azure::CompleteTemplate(
    _in std::string c_strFileName
)
{
    __DebugFunction();

    std::ifstream stlJsonFile(c_strFileName, std::ios::ate);
    std::streamsize nSizeOfJsonFile = stlJsonFile.tellg();
    stlJsonFile.seekg(0, std::ios::beg);
    std::string strRestRequestBody;
    strRestRequestBody.resize(nSizeOfJsonFile);
    stlJsonFile.read(strRestRequestBody.data(), nSizeOfJsonFile);

    ::ReplaceAll(strRestRequestBody, "{{SubscriptionId}}", m_strSubscriptionId);
    ::ReplaceAll(strRestRequestBody, "{{ResourceGroup}}", m_strResourceGroup);
    ::ReplaceAll(strRestRequestBody, "{{Location}}", m_strLocation);
    ::ReplaceAll(strRestRequestBody, "{{VirtualNetwork}}", m_strVirtualNetwork);
    ::ReplaceAll(strRestRequestBody, "{{NetworkSecurityGroup}}", m_strNetworkSecurityGroup);

    stlJsonFile.close();
    return strRestRequestBody;
}

/********************************************************************************************
 *
 * @class Azure
 * @function CompleteTemplate
 * @brief Wait for a VM to get into running state
 * @return The completed string
 *
 ********************************************************************************************/

void Azure::WaitToRun(
    _in const std::string & strVmName
    )
{
    __DebugFunction();
    std::string strRunStatus = "";

    while("VM running" != strRunStatus)
    {
        std::string strRunReponse = this->MakeRestCall("GET", "Microsoft.Compute/virtualMachines/" + strVmName + "/instanceView", "management.azure.com", "", "2020-12-01");

        StructuredBuffer oViewInstanceResponse = ::GetHttpBodyJson(strRunReponse);
        StructuredBuffer oStructuredBufferStatuses = oViewInstanceResponse.GetStructuredBuffer("statuses");
        StructuredBuffer oStructuredBufferStatuses1 = oStructuredBufferStatuses.GetStructuredBuffer("statuses1");

        strRunStatus = oStructuredBufferStatuses1.GetString("displayStatus");

        ::sleep(2);
    }
}