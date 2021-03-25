/*********************************************************************************************
 *
 * @file InstallerData.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "HardCodedCryptographicKeys.h"
#include "InitializerData.h"
#include "TlsTransactionHelperFunctions.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "JsonValue.h"

#include <iostream>
#include <fstream>
#include <memory>

/********************************************************************************************/

InitializerData::InitializerData(void)
{
    __DebugFunction();
    
    m_stlEncryptedOpaqueSessionBlob.assign(gc_abRootPrivateKey, gc_abRootPrivateKey + gc_unRootPrivateKeySizeInBytes);
    m_stlImposterEncryptedOpaqueSessionBlob.assign(gc_abRootPrivateKey, gc_abRootPrivateKey + gc_unRootPrivateKeySizeInBytes);
}

/********************************************************************************************/

InitializerData::~InitializerData(void)
{
    __DebugFunction();
    

}

/********************************************************************************************/

bool __thiscall InitializerData::Login(
    _in const std::string strOrganization,
    _in const std::string strUserEmail,
    _in const std::string strPassword
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        const std::string strLoginResponse = this->SendSaasRequest("POST", "AuthenticationManager/User/Login?Email="+ strUserEmail + "&Password="+ strPassword, "");
        StructuredBuffer oRestResponse = JsonValue::ParseDataToStructuredBuffer(strLoginResponse.substr(strLoginResponse.find_last_of("\r\n\r\n")).c_str());
        m_stlEncryptedOpaqueSessionBlob = oRestResponse.GetString("Eosb");
        fSuccess = true;
    }
    catch(const BaseException & oBaseException)
    {
        fSuccess = false;
    }

    return fSuccess;
}

/********************************************************************************************/

std::vector<std::string> __thiscall InitializerData::GetListOfDigitalContracts(void) const
{
    __DebugFunction();

    std::vector<std::string> stlListOfDigitalContracts;
    std::string strContractOne = "ACME <--> Wile E. Coyote Contract";
    std::string strContractTwo = "Devil <--> Trump's Soul Contract";
    std::string strContractThree = "SAIL <--> KPMG Contract";

    stlListOfDigitalContracts.push_back(strContractOne);
    stlListOfDigitalContracts.push_back(strContractTwo);
    stlListOfDigitalContracts.push_back(strContractThree);

    return stlListOfDigitalContracts;
}

/********************************************************************************************/

std::string __thiscall InitializerData::GetEffectiveDigitalContractName(void) const throw()
{
    __DebugFunction();

    return m_strEffectiveDigitalContractName;
}

/********************************************************************************************/

void __thiscall InitializerData::SetEffectiveDigitalContract(
    _in const std::string & c_strEffectiveDigitalContractName
    )
{
    __DebugFunction();
    
    m_strEffectiveDigitalContractName = c_strEffectiveDigitalContractName;
}

/********************************************************************************************/

std::vector<std::string> __thiscall InitializerData::GetUninitializedNodeAddresses(void) const
{
    __DebugFunction();
    
    std::vector<std::string> strListOfUninitializedNodeAddresses;
    std::string strAddressOne = "172.17.0.1";

    strListOfUninitializedNodeAddresses.push_back(strAddressOne);
    
    return strListOfUninitializedNodeAddresses;
}

/********************************************************************************************/

std::vector<std::string> __thiscall InitializerData::GetClusterNodeAddresses(void) const
{
    __DebugFunction();
    
    std::vector<std::string> strListOfClusterNodeAddresses;
    for (auto const & element: m_stlClusterNodes)
    {
        strListOfClusterNodeAddresses.push_back(element.second);
    }
    
    return strListOfClusterNodeAddresses;
}

/********************************************************************************************/

bool __thiscall InitializerData::AddNodeToCluster(
    _in const std::string & c_strNodeAddress
    )
{
    __DebugFunction();

    Qword qwNodeAddressHash = ::Get64BitHashOfNullTerminatedString(c_strNodeAddress.c_str(), false);
    
    m_stlClusterNodes.insert(std::pair<Qword, std::string>(qwNodeAddressHash, c_strNodeAddress));
    
    return true;
}

/********************************************************************************************/

void __thiscall InitializerData::RemoveNodeFromCluster(
    _in const std::string & c_strNodeAddress
    )
{
    __DebugFunction();
    
    Qword qwNodeAddressHash = ::Get64BitHashOfNullTerminatedString(c_strNodeAddress.c_str(), false);
    
    if (m_stlClusterNodes.end() != m_stlClusterNodes.find(qwNodeAddressHash))
    {
        m_stlClusterNodes.erase(m_stlClusterNodes.find(qwNodeAddressHash));
    }
}

/********************************************************************************************/

std::string __thiscall InitializerData::GetDatasetFilename(void) const throw()
{
    __DebugFunction();
    
    return m_strDatasetFilename;
}

/********************************************************************************************/

bool __thiscall InitializerData::SetDatasetFilename(
    _in const std::string & c_strDatasetFilename
    )
{
    __DebugFunction();
    
    bool fSuccess = false;
    std::ifstream stlFile(c_strDatasetFilename.c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        unsigned int unFileSizeInBytes = (unsigned int) stlFile.tellg();
        m_stlDataset.resize(unFileSizeInBytes);
        stlFile.seekg(0, std::ios::beg);
        stlFile.read((char *) m_stlDataset.data(), unFileSizeInBytes);
        stlFile.close();
        m_strDatasetFilename = c_strDatasetFilename;
        fSuccess = true;
    }
    
    return fSuccess;
}

/********************************************************************************************/

bool __thiscall InitializerData::InitializeNode(
    _in const std::string & c_strNodeAddress
    ) const
{
    __DebugFunction();

    bool fSuccess;
    StructuredBuffer oInitializationParameters;
    oInitializationParameters.PutBuffer("DigitalContract", m_stlEffectiveDigitalContract);
    oInitializationParameters.PutBuffer("GlobalRootKeyCertificate", gc_abRootPublicKeyCertificate, gc_unRootPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutBuffer("ComputationalDomainRootKeyCertificate", gc_abResearcherPublicKeyCertificate, gc_unResearcherPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutBuffer("DataDomainRootKeyCertificate", gc_abDataOwnerPublicKeyCertificate, gc_unDataOwnerPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutString("DataOwnerAccessToken", m_stlEncryptedOpaqueSessionBlob);
    oInitializationParameters.PutGuid("ClusterUuid", m_oClusterIdentifier);
    oInitializationParameters.PutGuid("RootOfTrustDomainUuid", m_oRootOfTrustDomainIdentifier);
    oInitializationParameters.PutGuid("ComputationalDomainUuid", m_oComputationalDomainIdentifier);
    oInitializationParameters.PutGuid("DataDomainUuid", m_oDataDomainIdentifier);
    oInitializationParameters.PutBuffer("Dataset", m_stlDataset);

    TlsNode * poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6800);
    ::PutTlsTransaction(poTlsNode, oInitializationParameters);
    poTlsNode->Release();

    return true;
}

/********************************************************************************************/

bool __thiscall InitializerData::GetImposterEncryptedOpaqueSessionBlob(void)
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        const std::string stlLoginResponse = this->SendSaasRequest("POST", "Authentication/ImposterEOSB", "");
        StructuredBuffer oRestResponse = JsonValue::ParseDataToStructuredBuffer(stlLoginResponse.substr(stlLoginResponse.find_last_of("\r\n\r\n")).c_str());
        m_stlImposterEncryptedOpaqueSessionBlob = oRestResponse.GetString("Eosb");
        fSuccess = true;
    }
    catch(const BaseException & oBaseException)
    {
        fSuccess = false;
    }

    return fSuccess;
}

/********************************************************************************************/

std::string __thiscall InitializerData::SendSaasRequest(
    _in const std::string c_strVerb,
    _in const std::string c_strResource,
    _in const std::string & c_strBody
    )
{
    __DebugFunction();

    std::string strResponseString;
    std::string strRestRequestHeader = c_strVerb + " /SAIL/" + c_strResource + " HTTP/1.1\r\n"
                                       "Host: 127.0.0.1:6200" + "\r\n"
                                       "Content-Length: " + std::to_string(c_strBody.length()) + "\r\n\r\n";

    std::string strRestRequest = strRestRequestHeader + c_strBody + "\r\n\r\n";
    std::unique_ptr<TlsNode> poTlsNode(::TlsConnectToNetworkSocket("172.17.0.1", 6200));
    poTlsNode->Write((const Byte *)strRestRequest.c_str(), strRestRequest.length());

    std::vector<Byte> oResponseByte = poTlsNode->Read(1, 5000);
    while(0 != oResponseByte.size())
    {
        strResponseString.push_back(oResponseByte.at(0));
        oResponseByte = poTlsNode->Read(1, 100);
    }

    return strResponseString;
}
