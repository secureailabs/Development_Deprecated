/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 17 March 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "TlsClient.h"
#include "TlsTransactionHelperFunctions.h"
#include "ConsoleInputHelperFunctions.h"
#include "Azure.h"

#include <iostream>

#include <fstream>
#include <unistd.h>
#include <string.h>

/********************************************************************************************/

static const char * gsc_szPrintableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szNumericCharacters = "0123456789";
static const char * gsc_szAddRemoveNodeInputCharacters = "aArRdD";
static const char * gsc_szYesNoInputCharacters = "yYnN";

/********************************************************************************************/

const std::string gc_strSubscriptionId = "20c11edd-abb4-4bc0-a6d5-c44d6d2524be";
const std::string gc_strLocation = "eastus2";
const std::string gc_strResourceGroup = "WebServiceTemplate0.2_group";
const std::string gc_strImageName = "WebServiceTemplate0.2Image";
const std::string gc_strVirtualNetwork = "WebServiceTemplate0.2_group-vnet";
const std::string gc_strNetworkSecurityGroup = "WebServiceTemplate0.2-nsg";
const std::string gc_strVirtualMachineSize = "Standard_A8_v2";

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
        _ThrowBaseException("Invalid File Path %s\n", c_strFileName.c_str(), nullptr);
    }
    return stlFileData;
}

/********************************************************************************************/

static void __stdcall CreateVirtualMachine(void)
{
    __DebugFunction();

    bool fDone = false;
    bool fFailedOnce = false;
    do
    {
        // Clear the screen
        ::system("clear");

        // Print the instructions relating to credentials
        std::cout << "The initialization of WebService requires your Azure credentials." << std::endl
                  << "---------------------------------------------------------------------------------" << std::endl;
        if (true == fFailedOnce)
        {
            std::cout << "\r\033[1;31mInvalid credentials. Try again.\033[0m" << std::endl;
        }

        std::string strApplicationId = ::GetStringInput("Application ID : ", 64, false, gsc_szPrintableCharacters);
        std::string strSecret = ::GetStringInput("Secret/Password : ", 64, true, gsc_szPrintableCharacters);
        std::string strTenantId = ::GetStringInput("Tenant Id: ", 64, false, gsc_szPrintableCharacters);

        // Attempt a login, if the login fails try again
        Azure oAzure(strApplicationId, strSecret, gc_strSubscriptionId, strTenantId, gc_strNetworkSecurityGroup, gc_strLocation);
        if (true == oAzure.Authenticate())
        {
            fDone = true;

            std::vector<Byte> stlBinariesPayload = ::FileToBytes("WebServicesPortal.binaries");
            _ThrowBaseExceptionIf((0 == stlBinariesPayload.size()), "Unable to read WebServicesPortal.binaries file", nullptr);

            std::cout << "---------------------------------------------------------------------------------\n";
            std::cout << "Attempting to run WebService on Virtual Machine\n";
            std::cout << "---------------------------------------------------------------------------------\n";

            unsigned int unVmCreationCounter = 0;
            oAzure.SetResourceGroup(gc_strResourceGroup);
            oAzure.SetVirtualNetwork(gc_strVirtualNetwork);
        
            std::string strPassword = ::GeneratePassword(16);
            std::string strVmName = oAzure.ProvisionVirtualMachine(gc_strImageName, gc_strVirtualMachineSize, "WebService", strPassword);
            std::string strVirtualMachinePublicIp = oAzure.GetVmIp(strVmName);
            std::cout << "Virtual Machine Provisioning status: Success\n";
            std::cout << "Virtual Machine Public IP Address: " << strVirtualMachinePublicIp << std::endl;
            std::cout << "Virtual Machine Public username: " << strVmName << std::endl;
            std::cout << "Virtual Machine Public password: " << strPassword << std::endl;
            // It makes sense to sleep for some time so that the VMs init process process can initialize
            // RootOfTrust process further communication.
            // TODO: This is a blocking call, make this non-blocking
            oAzure.WaitToRun(strVmName);

            // Establish a connection with a cron process running on the newly created VM
            // We are using a Connect call with a timeout so that in case the VM stub has not yet
            // initialized and opened a port for connection, we can try again every 2 second.
            TlsNode * oTlsNode = ::TlsConnectToNetworkSocketWithTimeout(strVirtualMachinePublicIp.c_str(), 9090, 60*1000, 2*1000);
            _ThrowIfNull(oTlsNode, "Tls connection timed out", nullptr);

            std::cout << "Establishing connection with the Virtual Machine...\n";
            fflush(stdout);
            // Send the Structured Buffer and wait 2 minutes for the initialization status
            StructuredBuffer oVmInitStatus(::PutTlsTransactionAndGetResponse(oTlsNode, stlBinariesPayload, 2*60*1000));
            if ("Success" == oVmInitStatus.GetString("Status"))
            {
                std::cout << "Success" << std::endl;
            }
            else
            {
                // In case the Platform Initialization fails on the VM, delete the VM.
                std::cout << "Failed to setup WebServices on Azure." << std::endl;
                oAzure.DeleteVirtualMachine(strVmName);
                oTlsNode->Release();
                _ThrowBaseException("Platform initialization on VM failed with the error: %s. Deleting the VM..", oVmInitStatus.GetString("Error") ,nullptr);
            }
            ::fflush(stdout);
        }
        else
        {
            fFailedOnce = true;
        }
    }
    while (false == fDone);
}

/********************************************************************************************/

int __cdecl main(
    int nNumberOfArguments,
    char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    try
    {
        // Allow the user to specific the composition of the cluster of nodes
        ::CreateVirtualMachine();

        std::cout << "************************************************************************" << std::endl;
        std::cout << "!! Kindly delete the Virtual Machines after use from the Azure Portal !!" << std::endl;
        std::cout << "************************************************************************" << std::endl;
    }

    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}
