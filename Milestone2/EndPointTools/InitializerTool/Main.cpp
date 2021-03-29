/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ConsoleInputHelperFunctions.h"
#include "CommandLine.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "InitializerData.h"
#include "TlsClient.h"
#include "TlsTransactionHelperFunctions.h"
#include "ThreadManager.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <iterator>

static const char * gsc_szPrintableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szNumericCharacters = "0123456789";
static const char * gsc_szIpAddressCharacters = "0123456789.";
static const char * gsc_szAddRemoveNodeInputCharacters = "aArRdD";
static const char * gsc_szYesNoInputCharacters = "yYnN";

typedef struct
{
    unsigned int unIndex;
}
CreateVirtualMachineParamaters;


/********************************************************************************************/

static void __stdcall PrintHeader(
    _in unsigned int unProgress
    )
{
    __DebugFunction();
    __DebugAssert((0 <= unProgress)&&(4 >= unProgress));

    // Print the generic header
    std::cout << "+-------------------------------------------------------------------------------+" << std::endl
              << "| Secure Payload Initialization Tool                                            |" << std::endl
              << "| Copyright (C) 2020 Secure AI Labs, Inc., All Rights Reserved.                 |" << std::endl
              << "| Confidential Technical Preview                                                |" << std::endl
              << "+-------------------------------------------------------------------------------+" << std::endl
              << "| PROGRESS " << unProgress << "/4                                                                  |" << std::endl
              << "+-------------------+-------------------+-------------------+-------------------+" << std::endl
              << "|    Credentials    |    Azure Nodes    | Digital Contract  |      Dataset      |" << std::endl
              << "+-------------------+-------------------+-------------------+-------------------+" << std::endl;
    switch(unProgress)
    {
        case 0
        :   std::cout << "|                   |                   |                   |                   |" << std::endl;
            break;
        case 1
        :   std::cout << "|     [Success]     |                   |                   |                   |" << std::endl;
            break;
        case 2
        :   std::cout << "|     [Success]     |     [Success]     |                   |                   |" << std::endl;
            break;
        case 3
        :   std::cout << "|     [Success]     |     [Success]     |     [Success]     |                   |" << std::endl;
            break;
        case 4
        :   std::cout << "|     [Success]     |     [Success]     |     [Success]     |     [Success]     |" << std::endl;
            break;
    }

    std::cout << "+-------------------+-------------------+-------------------+-------------------+" << std::endl;
}

/********************************************************************************************/

static void __stdcall PromptForCredentials(
    _inout InitializerData & oInitializerData,
    _in const std::string & c_strWebServiceIpAddress
    )
{
    __DebugFunction();

    try
    {
        // Clear the screen
        ::system("clear");
        // Print the generic header
        ::PrintHeader(0);
        // Print the instructions relating to credentials
        std::cout << "The initialization of the cluster of nodes requires your SAIL  credentials to" << std::endl
                  << "logon." << std::endl
                  << "---------------------------------------------------------------------------------" << std::endl;
        // Repeatedly try to login until oInitializerData.Login() throw an exception. This happens
        // after too many login attempts have failed.
        bool fSuccess = false;
        do
        {
            // Fetch user credentials
            std::string strUserEmail = ::GetStringInput("Email : ", 64, false, gsc_szPrintableCharacters);
            std::string strPassword = ::GetStringInput("Password : ", 64, true, gsc_szPrintableCharacters);
            fSuccess = oInitializerData.Login(c_strWebServiceIpAddress, strUserEmail, strPassword);
            if (false == fSuccess)
            {
                std::cout << "\r\033[1;31mInvalid credentials. Try again.\033[0m" << std::endl;
            }
        }
        while (false == fSuccess);
    }

    catch(...)
    {

    }
}

/********************************************************************************************/

static void __stdcall SelectDigitalContract(
    _inout InitializerData & oInitializerData
    )
{
    __DebugFunction();

    bool fDone = false;
    // Clear the screen
    ::system("clear");
    // Print the generic header
    ::PrintHeader(2);
    // Print the instructions relating to credentials
    std::cout << "Select which digital contract to use." << std::endl
              << "---------------------------------------------------------------------------------" << std::endl;
    // First we fetch the list of all digital contracts.
    std::vector<std::string> stlListOfDigitalContracts = oInitializerData.GetListOfDigitalContracts();
    // Now, let's print the list out, giving each entry a proper number
    for (unsigned int unIndex = 0; unIndex < stlListOfDigitalContracts.size(); ++unIndex)
    {
        std::cout << "    " << (unIndex + 1) << ". " << stlListOfDigitalContracts[unIndex].c_str() << std::endl;
    }
    // Add a white space between the list and the input prompt
    std::cout << std::endl;
    // Ask for a number. Make sure it's valid. Continue looking until a valid number is entered.
    do
    {
        unsigned int unNumber = std::stoi(::GetStringInput("Enter a number : ", 3, false, gsc_szNumericCharacters));
        if ((0 < unNumber)&&(unNumber <= stlListOfDigitalContracts.size()))
        {
            // Set the digital contract that will be used to drive the cluster of nodes
            oInitializerData.AddEffectiveDigitalContract(stlListOfDigitalContracts[unNumber - 1]);
            // We are done
            fDone = true;
        }
    }
    while (false == fDone);

    // Asking for the number of Virtual Machines to create, all of which will have same dataset
    std::string strNumberOfVm = ::GetStringInput("Number of Virtual Machine: ", 2, false, gsc_szNumericCharacters);

    // Set the number of Virtual Machine to create post authentication
    oInitializerData.SetNumberOfVirtualMachines(std::stoi(strNumberOfVm));
}

/********************************************************************************************/

static void __stdcall InputAzureCredentials(
    _inout InitializerData & oInitializerData
    )
{
    __DebugFunction();

    bool fDone = false;
    bool fFailedOnce = false;
    do
    {
        // Clear the screen
        ::system("clear");
        // Print the generic header
        ::PrintHeader(1);
        // Print the instructions relating to credentials
        std::cout << "The initialization of the cluster of nodes requires your Azure credentials." << std::endl
                  << "---------------------------------------------------------------------------------" << std::endl;
        if (true == fFailedOnce)
        {
            std::cout << "\r\033[1;31mInvalid credentials. Try again.\033[0m" << std::endl;
        }

        std::string strApplicationId = ::GetStringInput("Application ID : ", 64, false, gsc_szPrintableCharacters);
        std::string strSecret = ::GetStringInput("Secret/Password : ", 64, true, gsc_szPrintableCharacters);
        std::string strTenantId = ::GetStringInput("Tenant Id: ", 64, false, gsc_szPrintableCharacters);

        // Attempt a login, if the login fails try again
        if (true == oInitializerData.AzureLogin(strApplicationId, strSecret, gc_strSubscriptionId, gc_strNetworkSecurityGroup, gc_strLocation, strTenantId))
        {
            fDone = true;
        }
        else
        {
            fFailedOnce = true;
        }
    }
    while (false == fDone);
}

/********************************************************************************************/

static void __stdcall InputDataset(
    _inout InitializerData & oInitializerData
    )
{
    __DebugFunction();

    bool fDone = false;

    // Clear the screen
    ::system("clear");
    // Print the generic header
    ::PrintHeader(3);
    // Print the instructions relating to credentials
    std::cout << "Enter the filename of the dataset you wish to use in the cluster. Note that this" << std::endl
              << "dataset needs to match the digital contract that you have selected." << std::endl
              << "---------------------------------------------------------------------------------" << std::endl;
    do
    {
        std::string strFilename = ::GetStringInput("(Filename) >> ", 256, false, gsc_szPrintableCharacters);
        fDone = oInitializerData.AddDatasetFilename(strFilename.c_str());
        if (false == fDone)
        {
            std::cout << "Invalid filename, invalid file or dataset file specified doesn't match the digital contract." << std::endl;
        }
    }
    while (false == fDone);
}

/********************************************************************************************/

static bool __stdcall VerifyParametersAndProceed(
    _in const InitializerData & c_oInitializerData
    )
{
    __DebugFunction();

    bool fProceed = false;

    // Clear the screen
    ::system("clear");
    // Print the generic header
    ::PrintHeader(4);
    // Print the instructions relating to credentials
    std::cout << "This is the information and data that will be used to initialize the cluster of" << std::endl
              << "nodes." << std::endl;
    // Print the name of the digital contract
    std::cout << "+------------------------------------------------+-------------------------------+" << std::endl
              << "|               Digital Contract                 |           Dataset             |" << std::endl
              << "+------------------------------------------------+-------------------------------+" << std::endl;

    for(unsigned int unPrintCounter = 0; unPrintCounter < c_oInitializerData.GetNumberOfDatasets(); unPrintCounter++)
    {
        std::string strContractName = c_oInitializerData.GetEffectiveDigitalContractName(unPrintCounter);
        std::string strDatasetfile = c_oInitializerData.GetDatasetFilename(unPrintCounter);
        std::cout << "| " << strContractName;
        unsigned int unSpacesToAdd = 47 - strContractName.length();
        while(unSpacesToAdd--)
        {
            std::cout << " ";
        }
        std::cout << "|  " << strDatasetfile;
        unSpacesToAdd = 29 - strDatasetfile.length();
        while(unSpacesToAdd--)
        {
            std::cout << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "+------------------------------------------------+-------------------------------+" << std::endl;

    // Print out the number of Virtual Machines
    std::cout << "\nNumber of Virtual Machines to provision  = " << c_oInitializerData.GetNumberOfVirtualMachines() << std::endl;

    // Ask if this you want to proceed with these settings
    char cCharacterInput = ::GetCharacterInput("Proceed to initialize the cluster of nodes using these parameters (Y/N) ? ", false, gsc_szYesNoInputCharacters);
    if (('y' == cCharacterInput)||('Y' == cCharacterInput))
    {
        fProceed = true;
    }

    return fProceed;
}

/********************************************************************************************/

void * __thiscall CreateVirtualMachines(
    _in void * poParameter
    )
{
    __DebugFunction();

    InitializerData & oInitializerData = InitializerData::Get();

    CreateVirtualMachineParamaters * poCreateVirtualMachineParamaters = (CreateVirtualMachineParamaters *)poParameter;
    unsigned int unIndex = poCreateVirtualMachineParamaters->unIndex;
    oInitializerData.CreateAndInitializeVirtualMachine(unIndex);

    return 0;
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

        // Get the IP address of the webserices REST API server to login
        std::string strWebServiceIpAddress = ::GetStringInput("Enter WebServices Public Ip Address: ", 64, false, gsc_szIpAddressCharacters);

        // Using a singleton class for initializerData
        InitializerData & oInitializerData = InitializerData::Get();

        // Parse the command line
        StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);

        // Know if we are using the config file for Contract and Dataset or we
        // take them as an input during the initialization process
        bool fUsingConfigFile = false;

        // First we initialize the RootOfTrustNode. Without that, nothing else matters
        if (oCommandLineArguments.IsElementPresent("configFile", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            std::ifstream stlConfigFile(oCommandLineArguments.GetString("configFile"));
            std::string strLine;
            unsigned int unNumberOfVirtualMachines = 0;
            while (std::getline(stlConfigFile, strLine))
            {
                if (0 == strLine.length())
                {
                    break;
                }

                std::istringstream stlPerVirtualMachineData(strLine);
                std::string strDatasetFile;
                std::string strGuidDataset;

                if (!(stlPerVirtualMachineData >> strGuidDataset >> strDatasetFile))
                {
                    _ThrowBaseException("Invalid config format.", nullptr);
                }
                oInitializerData.AddDatasetFilename(strDatasetFile);
                oInitializerData.AddEffectiveDigitalContract(strGuidDataset);
                unNumberOfVirtualMachines++;
            }
            oInitializerData.SetNumberOfVirtualMachines(unNumberOfVirtualMachines);
            fUsingConfigFile = true;
        }

        // The first step is to fetch and verify the login credentials. This first step
        // will ensure that the oInitializerData object is initialized with all the
        // required credentials to continue processing
        ::PromptForCredentials(oInitializerData, strWebServiceIpAddress);
        // Now we fetch all of the SaaS data required to fill in the oInitializerData object. This
        // will download things like the list of digital contracts, digital certificates, the
        // imposter encrypted opaque session blob, etc...
        ::InputAzureCredentials(oInitializerData);
        // Now we need to enter the name of the dataset. This function will also ensure the file
        // exists and is ready to be uploaded

        do
        {
            // When using a confog file for data we skip the Select Digital Contract and
            // dataset as they have already been provided in the config file
            if (false == fUsingConfigFile)
            {
                // Allow the user to select which digital contract to use
                ::SelectDigitalContract(oInitializerData);
                // Allow the user to specific the composition of the cluster of nodes
                ::InputDataset(oInitializerData);
                // Okay, now that we have everything, we need to send the information to each and every nodes
                // This includes the IEOSB, the digital contract and the dataset
            }
        }
        while (false == ::VerifyParametersAndProceed(oInitializerData));

        ThreadManager * poThreadManager = ThreadManager::GetInstance();

        std::cout << "+------------------------------------+--------------------+---------------------+-------------------+-------------------+-------------------------------+" << std::endl
                  << "|        Virtual Machine Name        |      Password      |  Public IP Address  |   Provisioning    |  Initialization   |           Dataset             |" << std::endl
                  << "+------------------------------------+--------------------+---------------------+-------------------+-------------------+-------------------------------+" << std::endl;

        unsigned int unDatasetIndex = 0;
        for (unsigned int unVirtualMachineLoopCounter = 0; unVirtualMachineLoopCounter < oInitializerData.GetNumberOfVirtualMachines(); unVirtualMachineLoopCounter++)
        {

            CreateVirtualMachineParamaters * poCreateVirtualMachineParamaters = new CreateVirtualMachineParamaters();
            poCreateVirtualMachineParamaters->unIndex = unDatasetIndex;

            poThreadManager->CreateThread("VirtualMachineCreation", CreateVirtualMachines, (void *)poCreateVirtualMachineParamaters);
            // ::CreateVirtualMachines((void *)poCreateVirtualMachineParamaters);
            if (true == fUsingConfigFile)
            {
                unDatasetIndex++;
            }
        }

        // Wait for all of the threads to gracefull exit
        poThreadManager->JoinThreadGroup("VirtualMachineCreation");

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
