/*********************************************************************************************
 *
 * @file RemoteDataConnector.cpp
 * @author Prawal Gangwar
 * @date 31 July 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "RemoteDataConnector.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "StructuredBuffer.h"
#include "CommandLine.h"
#include "ConsoleInputHelperFunctions.h"

#include <iostream>
#include <filesystem>

static const char * gsc_szPrintableCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szIpAddressCharacters = "0123456789.";

RemoteDataConnector RemoteDataConnector::m_oRemoteDataConnector;

void PrintHelp(void)
{
    __DebugFunction();

    std::cout << "Usage: RemoteDataConnector [--Folder=<FolderName>] [--PortalIp=<RestPortalIp> --Port=<RestPortalIp>] [--User=<username> --Password=<password>]" << std::endl;
    ::exit(1);
}

int main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();


    try
    {
        // Parse the command line
        StructuredBuffer oCommandLineArguments = ::ParseCommandLineParameters((unsigned int) nNumberOfArguments, (const char **) pszCommandLineArguments);

        std::string strDatasetFolderName;
        if (nNumberOfArguments < 2)
        {
            // In case the users does not provide an existing folder with dataset,
            // a new folder is created where the user can later drop the datasets
            // and they will be picked up by the DataConnector and if valid, dataset
            // will be pinged to the SAIL Rest Portal.
            strDatasetFolderName = gc_strDataConnectorDefaultFolder;

            // Delete any existing folder
            std::error_code oErrorCode;
            if (std::filesystem::exists(strDatasetFolderName))
            {
                _ThrowBaseExceptionIf((false == std::filesystem::remove_all(strDatasetFolderName, oErrorCode)), "Could not delete Dataset Folder. %s", oErrorCode.message().c_str());
            }

            // Create a folder for data files
            _ThrowBaseExceptionIf((false == std::filesystem::create_directory(strDatasetFolderName, oErrorCode)), "Could not create Dataset Folder. %s", oErrorCode.message().c_str());
        }
        else
        {
            // The other option is when the dataset is already located some folder in
            // the filesystem and can be used directly by the DataConnector

            std::cout << oCommandLineArguments.ToString() << std::endl;
            if (true == oCommandLineArguments.IsElementPresent("Folder", ANSI_CHARACTER_STRING_VALUE_TYPE))
            {
                strDatasetFolderName = oCommandLineArguments.GetString("Folder");
            }
            else
            {
                ::PrintHelp();
            }
        }

        // Get the IP Address of the SAIL WebPortal. It could either be present
        // as a commnad line argument or be provided separately
        std::string strRestPortalApi = "";
        Dword dwPort = 0;
        if (true == oCommandLineArguments.IsElementPresent("PortalIp", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            strRestPortalApi = oCommandLineArguments.GetString("PortalIp");
            if (true == oCommandLineArguments.IsElementPresent("Port", ANSI_CHARACTER_STRING_VALUE_TYPE))
            {
                dwPort = std::stoi(oCommandLineArguments.GetString("Port"));
            }
            else
            {
                ::PrintHelp();
            }
        }
        else
        {
            strRestPortalApi = ::GetStringInput("PortalIpAddress: ", 64, false, gsc_szIpAddressCharacters);
            dwPort = std::stoi(::GetStringInput("Port: ", 64, false, gsc_szIpAddressCharacters));
        }

        // Login to the SAIL WebPortal. The credential could either be present as
        // Command line parameters or could be passed as
        std::string strUserEmail = "";
        std::string strPassword = "";
        if (true == oCommandLineArguments.IsElementPresent("User", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            strUserEmail = oCommandLineArguments.GetString("User");
            if (true == oCommandLineArguments.IsElementPresent("Password", ANSI_CHARACTER_STRING_VALUE_TYPE))
            {
                strPassword = oCommandLineArguments.GetString("Password");
            }
            else
            {
                ::PrintHelp();
            }
        }
        else
        {
            strUserEmail = ::GetStringInput("Email : ", 64, false, gsc_szPrintableCharacters);
            strPassword = ::GetStringInput("Password : ", 64, true, gsc_szPrintableCharacters);
        }

        // RemoteDataConnector is a singleton global object.
        RemoteDataConnector & oRemoteDataConnector = RemoteDataConnector::Get();

        // Keep trying to Login until successful
        bool fIsLoggedIn = false;
        do
        {
            if (false == oRemoteDataConnector.UserLogin(strRestPortalApi, dwPort, strUserEmail, strPassword))
            {
                ::system("clear");
                std::cout << "Login Failed. Try Again.." << std::endl;
                strUserEmail = ::GetStringInput("Email : ", 64, false, gsc_szPrintableCharacters);
                strPassword = ::GetStringInput("Password : ", 64, true, gsc_szPrintableCharacters);
            }
            else
            {
                fIsLoggedIn = true;
            }
        } while(false == fIsLoggedIn);

        // Start the DataConnector for all the datasets in the given folder
        if (true == oCommandLineArguments.IsElementPresent("VirtualMachineIp", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            std::string strVmIp = oCommandLineArguments.GetString("VirtualMachineIp");
            std::string strDatasetGuid = oCommandLineArguments.GetString("DatasetGuid");
            std::string strFileName = oCommandLineArguments.GetString("FileName");

            oRemoteDataConnector.ManualUploadDataSetToVirtualMachine(strVmIp, strDatasetGuid, strFileName);
        }
        else if (0 != strDatasetFolderName.size())
        {
            oRemoteDataConnector.SetDatasetFolderAndRun(strDatasetFolderName);
        }
    }
    catch (BaseException & oException)
    {
        std::cout << "RemoteDataConnector" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m" << oException.GetExceptionMessage() << "\033[0m" << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    catch (std::exception & oException)
    {
        std::cout << "RemoteDataConnector" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m" << oException.what() << "\033[0m" << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    catch (...)
    {
        std::cout << "RemoteDataConnector" << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}