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
#include "IpcTransactionHelperFunctions.h"
#include "StructuredBuffer.h"
#include "SocketClient.h"
#include "TlsClient.h"

#include <iostream>
#include <fstream>

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
    _in const std::string strUsername,
    _in const std::string strPassword
    )
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    // Create rest request
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "SailAuthentication");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Authentication/Login");
    oRestRequestStructuredBuffer.PutString("Username", strUsername);
    oRestRequestStructuredBuffer.PutString("Password", strPassword);
    oRestRequestStructuredBuffer.PutString("Organization", strOrganization);

    m_stlEncryptedOpaqueSessionBlob = this->SendSaasRequest(oRestRequestStructuredBuffer);
    if (0 < m_stlEncryptedOpaqueSessionBlob.size())
    {
        fSuccess = this->GetImporterEncryptedOpaqueSessionBlob();
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
    std::string strAddressOne = "127.0.0.1";
    
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
    oInitializationParameters.PutBuffer("DataOwnerAccessToken", m_stlImposterEncryptedOpaqueSessionBlob);
    oInitializationParameters.PutGuid("ClusterUuid", m_oClusterIdentifier);
    oInitializationParameters.PutGuid("RootOfTrustDomainUuid", m_oRootOfTrustDomainIdentifier);
    oInitializationParameters.PutGuid("ComputationalDomainUuid", m_oComputationalDomainIdentifier);
    oInitializationParameters.PutGuid("DataDomainUuid", m_oDataDomainIdentifier);
    oInitializationParameters.PutBuffer("Dataset", m_stlDataset);

    Socket * poSocket = ::ConnectToUnixDomainSocket("{b675be6d-b092-4b37-9e07-f1d645fc0f32}");
    ::PutIpcTransaction(poSocket, oInitializationParameters);
    poSocket->Release();
    
    return true;
}

/********************************************************************************************/

bool __thiscall InitializerData::GetImporterEncryptedOpaqueSessionBlob(void)
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "SailAuthentication");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Authentication/ImposterEOSB");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", m_stlEncryptedOpaqueSessionBlob);
    
    m_stlImposterEncryptedOpaqueSessionBlob = this->SendSaasRequest(oRestRequestStructuredBuffer);
    if (0 < m_stlImposterEncryptedOpaqueSessionBlob.size())
    {
        fSuccess = true;
    }
    
    return fSuccess;
}

/********************************************************************************************/

std::vector<Byte> __thiscall InitializerData::SendSaasRequest(
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();
    
    std::vector<Byte> stlRestResponse;
    // Establish the connection to the SaaS server
    TlsNode * poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);
    _ThrowBaseExceptionIf((nullptr == poTlsNode), "TlsConnectToNetworkSocket() has failed...", nullptr);
    // Format the request packet
    std::vector<Byte> stlFormattedRequestPacket = this->FormatRequestPacket(c_oStructuredBuffer);
    // Send request packet
    poTlsNode->Write(stlFormattedRequestPacket.data(), (stlFormattedRequestPacket.size()));
    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 1000);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 1000);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> __thiscall InitializerData::FormatRequestPacket(
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();
    
    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(uint32_t) + c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes() + sizeof(Dword);
    std::vector<Byte> stlSerializedBuffer(unSerializedBufferSizeInBytes);
    Byte * pbSerializedBuffer = (Byte *) stlSerializedBuffer.data();

    // The format of the request data is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x436f6e74                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfRestRequestStructuredBuffer                                |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfRestRequestStructuredBuffer] RestRequestStructuredBuffer             |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x656e6420                                                                 |
    // +------------------------------------------------------------------------------------+

    *((Dword *) pbSerializedBuffer) = 0x436f6e74;
    pbSerializedBuffer += sizeof(Dword);
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes();
    pbSerializedBuffer += sizeof(uint32_t);
    ::memcpy((void *) pbSerializedBuffer, (const void *) c_oStructuredBuffer.GetSerializedBufferRawDataPtr(), c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
    pbSerializedBuffer += c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes();
    *((Dword *) pbSerializedBuffer) = 0x656e6420;

    return stlSerializedBuffer;
}