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

    // We try to establish connection every 2 second until it is successful.
    TlsNode * poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(c_strNodeAddress.c_str(), 6800, 60*1000, 2*1000);
    ::PutTlsTransaction(poTlsNode, oInitializationParameters);
    poTlsNode->Release();

    return true;
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
    std::unique_ptr<TlsNode> poTlsNode(::TlsConnectToNetworkSocket(SERVER_IP_ADDRESS, SERVER_PORT));
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

unsigned int __thiscall InitializerData::CreateVirtualMachines(void)
{
    __DebugFunction();

    unsigned int unNumberOfVmCreated = 0;

    std::cout << "+------------------------------------+---------------------+-------------------+-------------------+" << std::endl
              << "|        Virtual Machine Name        |  Public IP Address  |   Provisioning    |  Initialization   |" << std::endl
              << "+------------------------------------+---------------------+-------------------+-------------------+" << std::endl;

    // Prepare the payload to send to the VM
    StructuredBuffer oPayloadToVm;
    // The instruction to execute after all the files are uploaded on the VM
    oPayloadToVm.PutString("Entrypoint", "./RootOfTrustProcess");

    // Set the type of VM, either it runs Computation or WebService
    oPayloadToVm.PutString("VirtualMachineType", "Computation");

    // A nested Structured Buffer containing all the executable files
    StructuredBuffer oFilesToPut;
    oFilesToPut.PutBuffer("RootOfTrustProcess", ::FileToBytes("RootOfTrustProcess"));
    oFilesToPut.PutBuffer("InitializerProcess", ::FileToBytes("InitializerProcess"));
    oFilesToPut.PutBuffer("SignalTerminationProcess", ::FileToBytes("SignalTerminationProcess"));
    oFilesToPut.PutBuffer("DataDomainProcess", ::FileToBytes("DataDomainProcess"));
    oFilesToPut.PutBuffer("ComputationalDomainProcess", ::FileToBytes("ComputationalDomainProcess"));
    oFilesToPut.PutBuffer("/usr/local/lib/python3.8/dist-packages/_DataConnector.so", ::FileToBytes("../VirtualMachine/DataConnectorPythonModule/libDataConnector.so"));

    oPayloadToVm.PutStructuredBuffer("ExecutableFiles", oFilesToPut);

    unsigned int unVmCreationCounter = 0;
    m_poAzure->SetResourceGroup(gc_strResourceGroup);
    m_poAzure->SetVirtualNetwork(gc_strVirtualNetwork);

    for (unVmCreationCounter = 0; unVmCreationCounter < m_unNumberOfVirtualMachines; unVmCreationCounter++)
    {
        // Provision a VM
        std::string strVmName;
        std::string strVirtualMachinePublicIp;
        try
        {
            strVmName = m_poAzure->ProvisionVirtualMachine(gc_strImageName, gc_strVirtualMachineSize, "");
            std::cout << "|  " << strVmName << "  |";
            ::fflush(stdout);
            strVirtualMachinePublicIp = m_poAzure->GetVmIp(strVmName);
            std::cout << "    " << strVirtualMachinePublicIp << "    |";
            ::fflush(stdout);
            std::cout << "     " << m_poAzure->GetVmProvisioningState(strVmName) << "     |";
            ::fflush(stdout);

            // It makes sense to sleep for some time so that the VMs init process process can initialize
            // RootOfTrust process further communication.
            // TODO: This is a blocking call, make this non-blocking
            m_poAzure->WaitToRun(strVmName);

            // Establish a connection with a cron process running on the newly created VM
            // We are using a Connect call with a timeout so that in case the VM stub has not yet
            // initialized and opened a port for connection, we can try again every 2 second.
            TlsNode * oTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strVirtualMachinePublicIp.c_str(), 9090, 60*1000, 2*1000);
            _ThrowIfNull(oTlsNode, "Tls connection timed out", nullptr);

            // Send the Structured Buffer and wait 2 minutes for the initialization status
            StructuredBuffer oVmInitStatus(::PutTlsTransactionAndGetResponse(oTlsNode, oPayloadToVm.GetSerializedBuffer(), 2*60*1000));
            if ("Success" == oVmInitStatus.GetString("Status"))
            {
                std::cout << "     " << "Success" << "     |" << std::endl;
            }
            else
            {
                // In case the Platform Initialization fails on the VM, delete the VM.
                std::cout << "     " << "Failure" << "     |" << std::endl;
                m_poAzure->DeleteVirtualMachine(strVmName);
                oTlsNode->Release();
                _ThrowBaseException("Platform initialization on VM failed with the error: %s. Deleting the VM..", oVmInitStatus.GetString("Error") ,nullptr);
            }
            ::fflush(stdout);
            // Once the VM is up and installed it is naked and waits for the initialization data
            // for the root of trust to configure the Virtual Machine
            // It makes sense to sleep for some time so that the RootOfTrust process can initialize
            // open socket for further communication.
            this->InitializeNode(strVirtualMachinePublicIp);

            unNumberOfVmCreated++;

            oTlsNode->Release();
        }
        catch(BaseException & oBaseException)
        {
            std::cout << "VM Initialization failed. Error: " << oBaseException.GetExceptionMessage() << std::endl;
        }
        catch (std::exception & oException)
        {
            std::cout << "Unexpected exception " << oException.what() << std::endl;
        }
    }
    std::cout << "+------------------------------------+---------------------+-------------------+-------------------+" << std::endl;

    return unNumberOfVmCreated;
}