/*********************************************************************************************
 *
 * @file InteractiveClient.cpp
 * @author Shabana Akhtar Baig
 * @date 03 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ConsoleInputHelperFunctions.h"
#include "Exceptions.h"
#include "PluginDictionary.h"
#include "SmartMemoryAllocator.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "GuiTerminal.h"

#include <string.h>

#include <iostream>
#include <string>
#include <vector>

/********************************************************************************************/

std::vector<Byte> __stdcall CreateRequestPacket(
    _in const StructuredBuffer & c_oRequestPacket
    )
{
    __DebugFunction();

    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(uint32_t) + c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes() + sizeof(Dword);

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
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes();
    pbSerializedBuffer += sizeof(uint32_t);
    ::memcpy((void *) pbSerializedBuffer, (const void *) c_oRequestPacket.GetSerializedBufferRawDataPtr(), c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes());
    pbSerializedBuffer += c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes();
    *((Dword *) pbSerializedBuffer) = 0x656e6420;

    return stlSerializedBuffer;
}

/********************************************************************************************/

bool Login(
    _out std::vector<Byte> & stlEosb,
    _in const std::string & c_strUserName,
    _in const std::string & c_strUserPassword,
    _in const std::string & c_strUserOrganization
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strUserName.length());
    __DebugAssert(0 != c_strUserPassword.length());
    __DebugAssert(0 != c_strUserOrganization.length());

    bool fSuccess = false;

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "SailAuthentication");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Authentication/Login");
    oRestRequestStructuredBuffer.PutString("Username", c_strUserName);
    oRestRequestStructuredBuffer.PutString("Password", c_strUserPassword);
    oRestRequestStructuredBuffer.PutString("Organization", c_strUserOrganization);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        stlEosb.resize(stlRestResponse.size());
        ::memcpy((void *) stlEosb.data(), (const void *) stlRestResponse.data(), stlRestResponse.size());
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage("\nLogin Failed");
    }

    return fSuccess;
}

/********************************************************************************************/

bool GetImposterEosb(
    _out std::vector<Byte> & stlIEosb,
    _in const std::vector<Byte> & c_stlEosb
    )
{
    __DebugFunction();

    bool fSuccess = false;

    std::vector<Byte> stlRestResponse;

    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "SailAuthentication");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Authentication/ImposterEOSB");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", c_stlEosb);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        stlIEosb.resize(stlRestResponse.size());
        ::memcpy((void *) stlIEosb.data(), (const void *) stlRestResponse.data(), stlRestResponse.size());
        fSuccess = true;
    }
    catch(BaseException oBaseException)
    {
        ::ShowErrorMessage("\nError getting imposter EOSB");
    }

    return fSuccess;
}

/********************************************************************************************/

std::vector<Byte> RegisterDataset(
    _in const std::vector<Byte> & c_stlEosb,
    _in const std::string & c_strDatasetName,
    _in const std::string & c_strDatasetOrganization,
    _in Qword qwDatasetSubmissionDate
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Add dataset information
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DatasetDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Dataset/RegisterDataset");
    oRestRequestStructuredBuffer.PutString("DatasetName", c_strDatasetName);
    oRestRequestStructuredBuffer.PutString("DatasetOrganization", c_strDatasetOrganization);
    oRestRequestStructuredBuffer.PutQword("DatasetSubmissionDate", qwDatasetSubmissionDate);
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        std::cout << "Dataset registered with UUID: " << oResponse.GetString("DatasetUuid") << std::endl;
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> DeleteDataset(
    _in const std::vector<Byte> & c_stlEosb,
    _in const std::string & c_strDatasetUuid
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Add dataset information
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DatasetDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Dataset/DeleteDataset");
    oRestRequestStructuredBuffer.PutString("DatasetUuid", c_strDatasetUuid);
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        if (200 == oResponse.GetWord("Status"))
        {
          std::cout << "Dataset with UUID " << c_strDatasetUuid << " deleted successfully."<< std::endl;
        }
        else
        {
          ::ShowErrorMessage("Error deleteing the dataset with UUID " + c_strDatasetUuid + ". Dataset not found.");
        }
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> GetListOfSubmittedDatasets(
    _in const std::vector<Byte> & c_stlEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DatasetDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "GET");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Dataset/GetSubmittedDataset");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        if (200 == oResponse.GetWord("Status"))
        {
            std::cout << "List of Submitted Datsets: \n\n";

            oResponse.RemoveElement("Status");
            std::vector<std::string> stlDatasetUuids = oResponse.GetNamesOfElements();
            for (std::string strUuid : stlDatasetUuids)
            {
                StructuredBuffer oDataset(oResponse.GetStructuredBuffer(strUuid.c_str()));
                std::cout << "Name: " << oDataset.GetString("DatasetName") << std::endl;
                std::cout << "Submitted By: " << oDataset.GetString("UserUuid") << std::endl;
                Qword qwSubmissionDate = oDataset.GetQword("DatasetSubmissionDate");
                std::cout << "Submission Date: " << ((qwSubmissionDate / 100) % 100) << "/" << (qwSubmissionDate % 100) << "/" << (qwSubmissionDate / 10000) << std::endl << std::endl;
            }
        }
        else
        {
          ::ShowErrorMessage("No datasets submitted.");
        }
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> GetListOfAvailableDatasets(
    _in const std::vector<Byte> & c_stlEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DatasetDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "GET");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Dataset/GetAvailableDataset");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        if (200 == oResponse.GetWord("Status"))
        {
            std::cout << "List of Available Datsets: \n\n";

            oResponse.RemoveElement("Status");
            std::vector<std::string> stlDatasetUuids = oResponse.GetNamesOfElements();
            for (std::string strUuid : stlDatasetUuids)
            {
                StructuredBuffer oDataset(oResponse.GetStructuredBuffer(strUuid.c_str()));
                std::cout << "Name: " << oDataset.GetString("DatasetName") << std::endl;
                std::cout << "Owner Organization: " << oDataset.GetString("DatasetOrganization") << std::endl;
                Qword qwSubmissionDate = oDataset.GetQword("DatasetSubmissionDate");
                std::cout << "Submission Date: " << ((qwSubmissionDate / 100) % 100) << "/" << (qwSubmissionDate % 100) << "/" << (qwSubmissionDate / 10000) << std::endl << std::endl;
            }
        }
        else
        {
          ::ShowErrorMessage("No datasets available.");
        }
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> RegisterDigitalContract(
    _in const std::vector<Byte> & c_stlEosb,
    _in const std::string & c_strDataOwnerOrganization,
    _in const std::string & c_strResearcherOrganization,
    _in const std::string & c_strInvolvedDatasets,
    _in Qword qwCreationDate,
    _in Qword qwExpirationDate
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Add plugin information
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DigitalContractDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/DigitalContract/RegisterDigitalContract");

    // Add Digital contract information
    oRestRequestStructuredBuffer.PutString("DataOwnerOrganization", c_strDataOwnerOrganization);
    oRestRequestStructuredBuffer.PutString("ResearcherOrganization", c_strResearcherOrganization);
    oRestRequestStructuredBuffer.PutString("InvolvedDatasets", c_strInvolvedDatasets);
    oRestRequestStructuredBuffer.PutQword("CreationDate", qwCreationDate);
    oRestRequestStructuredBuffer.PutQword("ExpirationDate", qwExpirationDate);
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        std::cout << "Digital contract registered with UUID: " << oResponse.GetString("DigitalContractUuid") << std::endl;
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> GetListOfDigitalContracts(
    _in const std::vector<Byte> & c_stlEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DigitalContractDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "GET");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/DigitalContract/MyDigitalContracts");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        if (200 == oResponse.GetWord("Status"))
        {
            std::cout << "List of Digital Contracts: \n\n";

            oResponse.RemoveElement("Status");
            std::vector<std::string> stlDigitalContractUuids = oResponse.GetNamesOfElements();
            for (std::string strUuid : stlDigitalContractUuids)
            {
                StructuredBuffer oDigitalContract(oResponse.GetStructuredBuffer(strUuid.c_str()));
                std::cout << "Owner Organization: " << oDigitalContract.GetString("DataOwnerOrganization") << std::endl;
                std::cout << "Researcher Organization: " << oDigitalContract.GetString("ResearcherOrganization") << std::endl;
                std::cout << "Involved Datasets: " << oDigitalContract.GetString("InvolvedDatasets") << std::endl;
                Qword qwCreationDate = oDigitalContract.GetQword("CreationDate");
                Qword qwExpirationDate = oDigitalContract.GetQword("ExpirationDate");
                std::cout << "Creation Date: " << ((qwCreationDate / 100) % 100) << "/" << (qwCreationDate % 100) << "/" << (qwCreationDate / 10000) << std::endl;
                std::cout << "Expiration Date: " << ((qwExpirationDate / 100) % 100) << "/" << (qwExpirationDate % 100) << "/" << (qwExpirationDate / 10000) << std::endl << std::endl;
            }
        }
        else
        {
          ::ShowErrorMessage("No digital contracts found.");
        }
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> GetListOfWaitingDigitalContracts(
    _in const std::vector<Byte> & c_stlEosb
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;

    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "DigitalContractDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "GET");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/DigitalContract/WaitingDigitalContracts");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        if (200 == oResponse.GetWord("Status"))
        {
            std::cout << "List of Waiting Digital Contracts: \n\n";

            oResponse.RemoveElement("Status");
            std::vector<std::string> stlDigitalContractUuids = oResponse.GetNamesOfElements();
            for (std::string strUuid : stlDigitalContractUuids)
            {
                StructuredBuffer oDigitalContract(oResponse.GetStructuredBuffer(strUuid.c_str()));
                std::cout << "Owner Organization: " << oDigitalContract.GetString("DataOwnerOrganization") << std::endl;
                std::cout << "Researcher Organization: " << oDigitalContract.GetString("ResearcherOrganization") << std::endl;
                std::cout << "Involved Datasets: " << oDigitalContract.GetString("InvolvedDatasets") << std::endl;
                Qword qwCreationDate = oDigitalContract.GetQword("CreationDate");
                Qword qwExpirationDate = oDigitalContract.GetQword("ExpirationDate");
                std::cout << "Creation Date: " << ((qwCreationDate / 100) % 100) << "/" << (qwCreationDate % 100) << "/" << (qwCreationDate / 10000) << std::endl;
                std::cout << "Expiration Date: " << ((qwExpirationDate / 100) % 100) << "/" << (qwExpirationDate % 100) << "/" << (qwExpirationDate / 10000) << std::endl << std::endl;
            }
        }
        else
        {
          ::ShowErrorMessage("No digital contracts are in flux.");
        }
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> RegisterVmInstance(
  _in const std::vector<Byte> & c_stlEosb,
  _in const std::string & c_strVmIPAddress,
  _in Word wVmPortNumber,
  _in bool fVmStatus,
  _in Qword qwRegistrationDate,
  _in Qword qwUpTime
  )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;

    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Add plugin information
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "VmDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/VM/RegisterVM");

    // Add Vm information
    oRestRequestStructuredBuffer.PutString("VmIpAddress", c_strVmIPAddress);
    oRestRequestStructuredBuffer.PutWord("VmPortNumber", wVmPortNumber);
    oRestRequestStructuredBuffer.PutBoolean("VmStatus", fVmStatus);
    oRestRequestStructuredBuffer.PutQword("VmRegistrationDate", qwRegistrationDate);
    oRestRequestStructuredBuffer.PutQword("VmUpTime", qwUpTime);
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        std::cout << "VM Instance registered with UUID: " << oResponse.GetString("VmInstanceUuid") << std::endl;
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/

std::vector<Byte> GetVMHeartBeat(
    _in const std::vector<Byte> & c_stlEosb,
    _in const std::string & c_strVmUuid
    )
{
    __DebugFunction();

    std::vector<Byte> stlRestResponse;

    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    // Add data owner's Eosb
    StructuredBuffer oEosb(c_stlEosb);

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "VmDatabase");
    oRestRequestStructuredBuffer.PutString("Verb", "GET");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/VM/GetVMHeartBeat");
    oRestRequestStructuredBuffer.PutBuffer("Eosb", oEosb.GetSerializedBuffer());

    // Add Vm's uuid
    oRestRequestStructuredBuffer.PutString("VmInstanceUuid", c_strVmUuid);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    try
    {
        StructuredBuffer oResponse(stlRestResponse);
        std::string strStatus = oResponse.GetBoolean("VmStatus") ? "Active" : "InActive";
        std::cout << "VM intance with UUID " << c_strVmUuid << " is: "<< strStatus << std::endl;
    }
    catch(...)
    {
        ::ShowErrorMessage(std::string(stlRestResponse.begin(), stlRestResponse.end()));
    }

    ::WaitForUserToContinue();

    return stlRestResponse;
}

/********************************************************************************************/


int main()
{
    __DebugFunction();

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    try
    {
        ::ClearScreen();

        std::cout << "************************\n  SAIL LOGIN\n************************\n" << std::endl;
        std::string strUserName = ::GetStringInput("Enter username: ", 20, false, c_szValidInputCharacters);
        std::string strUserPassword = ::GetStringInput("Enter password: ", 50, true, c_szValidInputCharacters);
        std::string strUserOrganization = ::GetStringInput("Enter organization name: ", 50, false, c_szValidInputCharacters);

        std::vector<Byte> stlEosb;
        bool fSuccess = Login(stlEosb, strUserName, strUserPassword, strUserOrganization);
        _ThrowBaseExceptionIf((false == fSuccess), "Exiting\n", nullptr);

        std::vector<Byte> stlIEosb;
        fSuccess = ::GetImposterEosb(stlIEosb, stlEosb);
        _ThrowBaseExceptionIf((false == fSuccess), "Exiting\n", nullptr);

        bool fTerminatedSignalEncountered = false;

        while (false == fTerminatedSignalEncountered)
        {
            ::ShowTopMenu();

            std::string strUserInput = ::GetStringInput("Selection: ", 1, false, c_szValidInputCharacters);

            switch (stoi(strUserInput))
            {
                case 1:
                {
                    std::string strDatasetName = ::GetStringInput("Enter dataset name: ", 20, false, c_szValidInputCharacters);
                    std::string strDatasetOrganization = ::GetStringInput("Enter dataset organization: ", 50, false, c_szValidInputCharacters);
                    Qword qwSubmissionDate = std::stoul(::GetStringInput("Enter dataset submission date in YYYYMMDD format (Example: 20201204): ", 50, false, "0123456789"));
                    ::RegisterDataset(stlEosb, strDatasetName, strDatasetOrganization, qwSubmissionDate);
                break;
               }
                case 2:
                {
                    std::string strDatasetUuid = ::GetStringInput("Enter dataset uuid: ", 38, false, c_szValidInputCharacters);
                    ::DeleteDataset(stlEosb, strDatasetUuid);
                break;
                }
                case 3:
                {
                    ::GetListOfAvailableDatasets(stlEosb);
                break;
                }
                case 4:
                {
                    ::GetListOfSubmittedDatasets(stlEosb);
                break;
                }
                case 5:
                {
                    std::string strDataOwnerOrganization = ::GetStringInput("Enter data owner's organization name: ", 20, false, c_szValidInputCharacters);
                    std::string strResearcherOrganization = ::GetStringInput("Enter researcher's  organization name: ", 50, false, c_szValidInputCharacters);
                    std::string strInvolvedDatasets = ::GetStringInput("Enter comma delimitted datasets involved in the contract: ", 50, false, c_szValidInputCharacters);
                    Qword qwCreationDate = std::stoul(::GetStringInput("Enter digital contract creation date in YYYYMMDD format (Example: 20201204): ", 50, false, "0123456789"));
                    Qword qwExpirationDate = std::stoul(::GetStringInput("Enter digital contract expiration date in YYYYMMDD format (Example: 20201204): ", 50, false, "0123456789"));
                    ::RegisterDigitalContract(stlEosb, strDataOwnerOrganization, strResearcherOrganization, strInvolvedDatasets, qwCreationDate, qwExpirationDate);
                break;
                }
                case 6:
                {
                    ::GetListOfDigitalContracts(stlEosb);
                break;
                }
                case 7:
                {
                    ::GetListOfWaitingDigitalContracts(stlEosb);
                break;
                }
                case 8:
                {
                    std::string strIpAddress = ::GetStringInput("Enter VM's IP Address: ", 12, false, c_szValidInputCharacters);
                    Word wPort = std::stoul(::GetStringInput("Enter port number: ", 5, false, c_szValidInputCharacters));
                    bool fIsActive = stoul(::GetStringInput("Enter Vm's active status, 0 for inactive and 1 for active: ", 1, false, "01"));
                    Qword qwRegistrationDate = std::stoul(::GetStringInput("Enter Vm's registration date in YYYYMMDD format (Example: 20201204): ", 50, false, "0123456789"));
                    Qword qwUpTime = std::stoul(::GetStringInput("Enter Vm's uptime : ", 50, false, "0123456789"));
                    ::RegisterVmInstance(stlIEosb, strIpAddress, wPort, fIsActive, qwRegistrationDate, qwUpTime);
                break;
                }
                case 9:
                {
                    std::string strVmUuid = ::GetStringInput("Enter VM UUID: ", 38, false, c_szValidInputCharacters);
                    ::GetVMHeartBeat(stlIEosb, strVmUuid);
                break;
                }
                case 0:
                {
                    fTerminatedSignalEncountered = true;
                break;
                }
                default:
                {
                    std::cout << "Invalid option. Usage: [1-9]" << std::endl;
                break;
                }
            }
        }
    }
    catch(BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {

    }
    return 0;
}
