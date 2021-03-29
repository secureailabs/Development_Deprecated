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

#include <unistd.h>

/********************************************************************************************/

std::vector<Byte> FileToBytes(
    const std::string c_strFileName
)
{
    __DebugFunction();

    std::vector<Byte> stlFileData;

    std::ifstream stlFile(c_strFileName.c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        unsigned int unFileSizeInBytes = (unsigned int) stlFile.tellg();
        stlFileData.resize(unFileSizeInBytes);
        stlFile.seekg(0, std::ios::beg);
        stlFile.read((char *)stlFileData.data(), unFileSizeInBytes);
        stlFile.close();
    }
    else
    {
        _ThrowBaseException("Invalid File Path", nullptr);
    }
    return stlFileData;
}

// Singleton Object, can be declared anywhere, but only once
InitializerData InitializerData::m_oInitializerData;

/********************************************************************************************
 *
 * @function InitializerData
 * @brief Gets the singleton instance of the InitializerData object
 *
 ********************************************************************************************/

InitializerData & __thiscall InitializerData::Get(void)
{
    __DebugFunction();

    return m_oInitializerData;
}

/********************************************************************************************/

InitializerData::InitializerData(void)
{
    __DebugFunction();

}

/********************************************************************************************/

InitializerData::~InitializerData(void)
{
    __DebugFunction();

    if (nullptr != m_poAzure)
    {
        m_poAzure->Release();
    }
}

/********************************************************************************************/

bool __thiscall InitializerData::Login(
    _in const std::string c_strWebServiceIp,
    _in const std::string strUserEmail,
    _in const std::string strPassword
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        const std::string strLoginResponse = this->SendSaasRequest(c_strWebServiceIp, "POST", "AuthenticationManager/User/Login?Email="+ strUserEmail + "&Password="+ strPassword, "");
        StructuredBuffer oRestResponse = JsonValue::ParseDataToStructuredBuffer(strLoginResponse.substr(strLoginResponse.find_last_of("\r\n\r\n")).c_str());
        m_stlEncryptedOpaqueSessionBlob = oRestResponse.GetString("Eosb");
        fSuccess = true;
    }
    catch(const BaseException & oBaseException)
    {
        fSuccess = false;
    }

    // return fSuccess;
    return true;
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

std::string __thiscall InitializerData::GetEffectiveDigitalContractName(
    _in const unsigned int unDigitalContractIndex
) const throw()
{
    __DebugFunction();
    __DebugAssert(unDigitalContractIndex < this->GetNumberOfDatasets());

    return m_stlEffectiveDigitalContractName.at(unDigitalContractIndex);
}

/********************************************************************************************/

void __thiscall InitializerData::AddEffectiveDigitalContract(
    _in const std::string & c_strEffectiveDigitalContractName
    )
{
    __DebugFunction();

    m_stlEffectiveDigitalContractName.push_back(c_strEffectiveDigitalContractName);
}

/********************************************************************************************/

std::string __thiscall InitializerData::GetDatasetFilename(
    _in const unsigned int unFilenameIndex
) const throw()
{
    __DebugFunction();
    __DebugAssert(unFilenameIndex < this->GetNumberOfDatasets());

    return m_stlDatasetFilename.at(unFilenameIndex);
}

/********************************************************************************************/

bool __thiscall InitializerData::AddDatasetFilename(
    _in const std::string & c_strDatasetFilename
    )
{
    __DebugFunction();

    bool fSuccess = false;
    std::ifstream stlFile(c_strDatasetFilename.c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        m_stlDatasetFilename.push_back(c_strDatasetFilename);
        fSuccess = true;
    }

    return fSuccess;
}

/********************************************************************************************/

bool __thiscall InitializerData::InitializeNode(
    _in const std::string & c_strNodeAddress,
    _in unsigned int unDatasetIndex
    ) const
{
    __DebugFunction();

    std::vector<Byte> stlDataset;
    std::ifstream stlFile(m_stlDatasetFilename.at(unDatasetIndex).c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        unsigned int unFileSizeInBytes = (unsigned int) stlFile.tellg();
        stlDataset.resize(unFileSizeInBytes);
        stlFile.seekg(0, std::ios::beg);
        stlFile.read((char *) stlDataset.data(), unFileSizeInBytes);
        stlFile.close();
    }

    bool fSuccess;
    StructuredBuffer oInitializationParameters;
    // TODO: this is an empty buffer, we need to fill it in future
    oInitializationParameters.PutBuffer("DigitalContract", m_stlEffectiveDigitalContract);
    oInitializationParameters.PutBuffer("GlobalRootKeyCertificate", gc_abRootPublicKeyCertificate, gc_unRootPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutBuffer("ComputationalDomainRootKeyCertificate", gc_abResearcherPublicKeyCertificate, gc_unResearcherPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutBuffer("DataDomainRootKeyCertificate", gc_abDataOwnerPublicKeyCertificate, gc_unDataOwnerPublicKeyCertificateSizeInBytes);
    oInitializationParameters.PutString("DataOwnerAccessToken", m_stlEncryptedOpaqueSessionBlob);
    oInitializationParameters.PutGuid("ClusterUuid", m_oClusterIdentifier);
    oInitializationParameters.PutGuid("RootOfTrustDomainUuid", m_oRootOfTrustDomainIdentifier);
    oInitializationParameters.PutGuid("ComputationalDomainUuid", m_oComputationalDomainIdentifier);
    oInitializationParameters.PutGuid("DataDomainUuid", m_oDataDomainIdentifier);
    oInitializationParameters.PutBuffer("Dataset", stlDataset);

    // We try to establish connection every 2 second until it is successful.
    TlsNode * poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(c_strNodeAddress.c_str(), 6800, 60*1000, 2*1000);
    _ThrowIfNull(poTlsNode, "Couldn't establish connection with root of trust process.", nullptr);

    ::PutTlsTransaction(poTlsNode, oInitializationParameters);
    poTlsNode->Release();

    return true;
}

/********************************************************************************************/

std::string __thiscall InitializerData::SendSaasRequest(
    _in const std::string & c_strWebserviceIp,
    _in const std::string c_strVerb,
    _in const std::string c_strResource,
    _in const std::string & c_strBody
    ) const
{
    __DebugFunction();

    std::string strResponseString;
    std::string strRestRequestHeader = c_strVerb + " /SAIL/" + c_strResource + " HTTP/1.1\r\n"
                                       "Host: " + c_strWebserviceIp + ":6200" + "\r\n"
                                       "Content-Length: " + std::to_string(c_strBody.length()) + "\r\n\r\n";

    std::string strRestRequest = strRestRequestHeader + c_strBody + "\r\n\r\n";
    std::unique_ptr<TlsNode> poTlsNode(::TlsConnectToNetworkSocket(c_strWebserviceIp.c_str(), SERVER_PORT));
    poTlsNode->Write((const Byte *)strRestRequest.c_str(), strRestRequest.length());

    std::vector<Byte> oResponseByte = poTlsNode->Read(1, 5000);
    while(0 != oResponseByte.size())
    {
        strResponseString.push_back(oResponseByte.at(0));
        oResponseByte = poTlsNode->Read(1, 100);
    }

    return strResponseString;
}

/********************************************************************************************/

bool __thiscall InitializerData::AzureLogin(
    _in const std::string & c_strAppId,
    _in const std::string & c_strSecret,
    _in const std::string & c_strSubscriptionID,
    _in const std::string & c_strNetworkSecurityGroup,
    _in const std::string & c_strLocation,
    _in const std::string & c_strTenant
    )
{
    __DebugFunction();

    bool fAzureLoginSuccess = false;

    m_poAzure = new Azure(c_strAppId, c_strSecret, c_strSubscriptionID, c_strTenant, c_strNetworkSecurityGroup, c_strLocation);

    if (true == m_poAzure->Authenticate())
    {
        fAzureLoginSuccess = true;
    }
    else
    {
        m_poAzure->Release();
        m_poAzure = nullptr;
    }

    return fAzureLoginSuccess;
}

/********************************************************************************************/

void __thiscall InitializerData::SetNumberOfVirtualMachines(
    _in const unsigned int c_unNumberOfVirtualMachines
    )
{
    __DebugFunction();

    m_unNumberOfVirtualMachines = c_unNumberOfVirtualMachines;
}

/********************************************************************************************/

unsigned int __thiscall InitializerData::GetNumberOfVirtualMachines(void) const
{
    __DebugFunction();

    return m_unNumberOfVirtualMachines;
}

/********************************************************************************************/

unsigned int __thiscall InitializerData::GetNumberOfDatasets(void) const
{
    __DebugFunction();

    return m_stlDatasetFilename.size();
}

/********************************************************************************************/

unsigned int __thiscall InitializerData::CreateAndInitializeVirtualMachine(
    _in unsigned int unDatasetIndex
)
{
    __DebugFunction();

    std::vector<Byte> stlBinariesPayload = ::FileToBytes("SecureComputationalVirtualMachine.binaries");
    _ThrowBaseExceptionIf((0 == stlBinariesPayload.size()), "Unable to read SecureComputationalVirtualMachine.binaries file", nullptr);

    // In case of a failure to provision a Virtual Machine we will try for unVmCreationMaximumAttempts times
    // before we actually give up.
    unsigned int unVmCreationMaximumAttempts = 4;
    m_poAzure->SetResourceGroup(gc_strResourceGroup);
    m_poAzure->SetVirtualNetwork(gc_strVirtualNetwork);

    while (0 != unVmCreationMaximumAttempts)
    {
        // Provision a VM
        std::string strVmName;
        std::string strVirtualMachinePublicIp;
        std::string strVirtualMachineStatus;
        try
        {
            std::string strPassword = ::GeneratePassword(16);
            strVmName = m_poAzure->ProvisionVirtualMachine(gc_strImageName, gc_strVirtualMachineSize, "", strPassword);
            strVirtualMachineStatus = "|  " + strVmName + "  ";
            strVirtualMachineStatus += "|  " + strPassword + "  |";

            strVirtualMachinePublicIp = m_poAzure->GetVmIp(strVmName);
            strVirtualMachineStatus += "    " + strVirtualMachinePublicIp;
            unsigned int unSpaceCount = 17 - strVirtualMachinePublicIp.length();
            while (unSpaceCount--)
            {
                strVirtualMachineStatus += " ";
            }
            strVirtualMachineStatus += "|     " + m_poAzure->GetVmProvisioningState(strVmName) + "     |";

            // It makes sense to sleep for some time so that the VMs init process process can initialize
            // RootOfTrust process further communication.
            // TODO: This is a blocking call, make this non-blocking
            m_poAzure->WaitToRun(strVmName);

            // Establish a connection with a cron process running on the newly created VM
            // We are using a Connect call with a timeout so that in case the VM stub has not yet
            // initialized and opened a port for connection, we can try again every 2 second.
            TlsNode * oTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strVirtualMachinePublicIp.c_str(), 9090, 60*1000, 2*1000);
            _ThrowIfNull(oTlsNode, "Tls connection for package upload timed out", nullptr);

            // Send the Structured Buffer and wait 2 minutes for the initialization status
            std::vector<Byte> stlVirtualMachinePackageInstallResponse = ::PutTlsTransactionAndGetResponse(oTlsNode, stlBinariesPayload, 5*60*1000);
            _ThrowBaseExceptionIf((0 == stlVirtualMachinePackageInstallResponse.size()), "No respose from Virtual Machine about it's state", nullptr);

            StructuredBuffer oVmInitStatus(stlVirtualMachinePackageInstallResponse);
            if ("Success" == oVmInitStatus.GetString("Status"))
            {
                strVirtualMachineStatus += "      Success      |";
            }
            else
            {
                // In case the Platform Initialization fails on the VM, delete the VM.
                strVirtualMachineStatus += "     Failure     |";
                m_poAzure->DeleteVirtualMachine(strVmName);
                oTlsNode->Release();
                _ThrowBaseException("Platform initialization on VM failed with the error: %s. Deleting the VM..", oVmInitStatus.GetString("Error") ,nullptr);
            }
            oTlsNode->Release();

            // Once the VM is up and installed it is naked and waits for the initialization data
            // for the root of trust to configure the Virtual Machine
            // It makes sense to sleep for some time so that the RootOfTrust process can initialize
            // open socket for further communication.
            this->InitializeNode(strVirtualMachinePublicIp, unDatasetIndex);
            // _ThrowBaseException("to send initialization data to Root Of Trust. Error %s. Retrying.", oBaseException.GetExceptionMessage());

            // Since multiple threads are running and creating Virtual Machines, it is required
            // to acquire a lock before printing so that the Virtual Machine Status do not mix up.
            ::pthread_mutex_lock(&m_sMutex);
            std::cout << strVirtualMachineStatus;
            std::string strDatasetfile = this->GetDatasetFilename(unDatasetIndex);
            std::cout << "  " << strDatasetfile;
            unsigned int unSpacesToAdd = 29 - strDatasetfile.length();
            while(unSpacesToAdd--)
            {
                std::cout << " ";
            }
            std::cout << "|" << std::endl;

            ::pthread_mutex_unlock(&m_sMutex);
            unVmCreationMaximumAttempts = 0;
        }
        catch(BaseException & oBaseException)
        {
            std::cout << "\nFailed " << oBaseException.GetExceptionMessage() << std::endl;
        }
        catch (std::exception & oException)
        {
            std::cout << "Unexpected exception " << oException.what() << std::endl;
        }
    }
    std::cout << "+------------------------------------+--------------------+---------------------+-------------------+-------------------+-------------------------------+" << std::endl;

    return 0;
}
