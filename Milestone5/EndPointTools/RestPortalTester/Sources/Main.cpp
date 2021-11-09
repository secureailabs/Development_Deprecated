/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Shabana Akhtar Baig
 * @date 15 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "InteractiveClient.h"

/********************************************************************************************/

int main()
{
    __DebugFunction();

    const char * c_szValidInputCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#_$ \b{}-.,";

    try
    {
        ::ClearScreen();

        std::cout << "************************\n  REST PORTAL TESTER \n************************\n" << std::endl;
        std::string strIpAddress = ::GetStringInput("IP address: ", 50, false, c_szValidInputCharacters);
        unsigned int unPortNumber = std::stoul(::GetStringInput("Port number: ", 50, false, c_szValidInputCharacters));
        // Add webservices configuration
        ::AddWebPortalConfiguration(strIpAddress.c_str(), unPortNumber);

        // PING web services portal
        ::GetRemoteAttestationCertificate();

        bool fTerminatedSignalEncountered = false;
        while(false == fTerminatedSignalEncountered)
        {
            ::ShowLoginMenu();
            std::string strUserInput = ::GetStringInput("Selection: ", 1, false, c_szValidInputCharacters);
            switch (stoi(strUserInput))
            {
                case 1:
                {
                    ::ClearScreen();

                    std::cout << "************************\n  SAIL LOGIN\n************************\n" << std::endl;
                    std::string strEmail = ::GetStringInput("Enter email: ", 50, false, c_szValidInputCharacters);
                    std::string strUserPassword = ::GetStringInput("Enter password: ", 50, true, c_szValidInputCharacters);

                    // Login to the web services
                    std::string strEncodedEosb = Login(strEmail, strUserPassword);

                    _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
                    // Get user and organization guid from eosb
                    StructuredBuffer oUserInformation(::GetBasicUserInformation(strEncodedEosb));
                    std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
                    std::string strUserGuid = oUserInformation.GetString("UserGuid");
                    Qword qwAccessRights = oUserInformation.GetQword("AccessRights");
                    // Create a Virtual Machine Guid
                    std::string strVmGuid = Guid(eVirtualMachine).ToString(eHyphensAndCurlyBraces);
                    std::string strVmEosb;

                    bool fTerminatedSignalEncountered = false;

                    while (false == fTerminatedSignalEncountered)
                    {
                        ::ShowTopMenu();

                        std::string strUserInput = ::GetStringInput("Selection: ", 2, false, c_szValidInputCharacters);

                        switch (stoi(strUserInput))
                        {
                            case 0:
                            {
                                // log out
                                fTerminatedSignalEncountered = true;
                            break;
                            }
                            case 1:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::RegisterUser(strEncodedEosb, strOrganizationGuid);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("User added successfully!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 2:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::ListOrganizations(strEncodedEosb);
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 3:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::ListAllUsers(strEncodedEosb);
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 4:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::ListOrganizationUsers(strEncodedEosb);
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 5:
                            {
                                ::GetOrganizationInformation(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 6:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::UpdateOrganizationInformation(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("Organization information updated successfully!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 7:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::UpdateUserAccessRights(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("User access rights updated successfully!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 8:
                            {
                                bool fSuccess = ::UpdateUserInformation(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("User information updated successfully!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 9:
                            {
                                bool fSuccess = ::UpdatePassword(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Password changed!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 10:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::DeleteUser(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("User deleted!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 11:
                            {
                                if (1 == qwAccessRights) // Check if user is an admin
                                {
                                    bool fSuccess = ::RecoverUser(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("User recoverd!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 12:
                            {
                                // Register a Vm
                                std::string strIEosb = ::GetIEosb(strEncodedEosb);
                                strVmEosb = ::RegisterVirtualMachine(strIEosb, strVmGuid);
                            break;
                            }
                            case 13:
                            {
                                if (0 == strVmEosb.size())
                                {
                                    ::ShowErrorMessage("Get VmEosb first.");
                                }
                                else 
                                {
                                    std::string strVmEventGuid = ::RegisterVmAfterDataUpload(strEncodedEosb, strVmGuid);
                                    // Register Leaf events
                                    if (0 < strVmEventGuid.size())
                                    {
                                        ::RegisterLeafEvents(strEncodedEosb, strVmEventGuid);
                                    }
                                }

                                ::WaitForUserToContinue();
                            break;
                            }
                            case 14:
                            {
                                if (0 == strVmEosb.size())
                                {
                                    ::ShowErrorMessage("Get VmEosb first.");
                                }
                                else
                                {
                                    // Register Vm branch event for RO
                                    std::string strVmEventGuid = ::RegisterVmForComputation(strEncodedEosb, strVmGuid);
                                    // Register Leaf events
                                    if (0 < strVmEventGuid.size())
                                    {
                                        ::RegisterLeafEvents(strEncodedEosb, strVmEventGuid);
                                    }
                                }
                                ::WaitForUserToContinue();
                            break;
                            }
                            case 15:
                            {
                                bool fSuccess = ::UpdateVirtualMachineStatus(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Virtual machine's status updated!");
                                }
                                ::WaitForUserToContinue();
                            break;
                            }
                            case 16:
                            {
                                ::PullVirtualMachine(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break;
                            }
                            case 17:
                            {
                                ::ListVirtualMachines(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break;
                            }
                            case 18:
                            {
                                std::cout << "************************\n  Audit Logs \n************************\n" << std::endl;
                                // Get list of all events for the organization
                                ::GetListOfEvents(strEncodedEosb, "{00000000-0000-0000-0000-000000000000}", strOrganizationGuid, 0);

                                ::WaitForUserToContinue();
                            break;
                            }
                            case 19:
                            {
                                std::cout << "************************\n  Audit Logs \n************************\n" << std::endl;
                                std::string strParentGuid = ::GetStringInput("Enter hyphen and curly braces formatted parent guid: ", 38, true, c_szValidInputCharacters);
                                if (0 < strParentGuid.size())
                                {
                                    // Get list of events for the given parent guid
                                    ::GetListOfEvents(strEncodedEosb, strParentGuid, strOrganizationGuid, 0);
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Error no parent guid specified, try again.");
                                }

                                ::WaitForUserToContinue();
                            break;
                            }
                            case 20:
                            {
                                bool fSuccess = ::RegisterDigitalContract(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Digital contract registered successfully!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 21:
                            {
                                StructuredBuffer oDigitalContracts(::ListDigitalContracts(strEncodedEosb));
                                ::PrintDigitalContracts(oDigitalContracts);
                                WaitForUserToContinue();
                            break; 
                            }
                            case 22:
                            {
                                ::PullDigitalContract(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 23:
                            {
                                if (1 == qwAccessRights || 5 == qwAccessRights) // Check if user is a dataset admin
                                {
                                    bool fSuccess = ::AcceptDigitalContract(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("Digital contract approved!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 24:
                            {
                                if (1 == qwAccessRights || 4 == qwAccessRights) // Check if user is a digital contract admin
                                {
                                    bool fSuccess = ::ActivateDigitalContract(strEncodedEosb);
                                    if (true == fSuccess)
                                    {
                                        ::ShowSuccessMessage("Digital contract activated!");
                                    }
                                }
                                else 
                                {
                                    ::ShowErrorMessage("Transaction not authorized.");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 25:
                            {
                                bool fSuccess = ::GetDigitalContractProvisioningStatus(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 26:
                            {
                                bool fSuccess = ::RegisterDataset(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Dataset registered!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 27:
                            {
                                bool fSuccess = ::ListDatasets(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 28:
                            {
                                bool fSuccess = ::PullDataset(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 29:
                            {
                                bool fSuccess = ::DeleteDataset(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Dataset deleted!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 30:
                            {
                                bool fSuccess = ::RegisterAzureTemplate(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Azure template registered!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 31:
                            {
                                bool fSuccess = ::ListAzureTemplates(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 32:
                            {
                                bool fSuccess = ::PullAzureTemplate(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 33:
                            {
                                bool fSuccess = ::UpdateAzureTemplate(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Azure template updated!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 34:
                            {
                                bool fSuccess = ::DeleteAzureTemplate(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Azure template deleted!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 35:
                            {
                                bool fSuccess = ::AssociateDigitalContractWithAzureTemplate(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Digital contract(s) associated with the template!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 36:
                            {
                                bool fSuccess = ::RegisterRemoteDataConnector(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Remote data connector registered!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 37:
                            {
                                bool fSuccess = ::ListRemoteDataConnectors(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 38:
                            {
                                bool fSuccess = ::PullRemoteDataConnector(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 39:
                            {
                                bool fSuccess = ::UpdateRemoteDataConnector(strEncodedEosb);
                                if (true == fSuccess)
                                {
                                    ::ShowSuccessMessage("Remote data connector updated!");
                                }
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 40:
                            {
                                StructuredBuffer oUserInformation = ::GetBasicUserInformation(strEncodedEosb);
                                std::cout << "User Information : \n";
                                std::cout << "User name : " << oUserInformation.GetString("Username") << "\n";
                                std::cout << "User access rights : " << oUserInformation.GetQword("AccessRights") << "\n";
                                std::cout << "Title : " << oUserInformation.GetString("Title") << "\n";
                                std::cout << "Phone Number : " << oUserInformation.GetString("PhoneNumber") << "\n";
                                ::WaitForUserToContinue();
                            break;
                            }
                            case 41:
                            {
                                bool fSuccess = ::SendRemoteDataConnectoHeartBeat(strEncodedEosb);
                                ::WaitForUserToContinue();
                            break; 
                            }
                            case 42:
                            {
                                ::ShutdownPortal(strEncodedEosb);
                                fTerminatedSignalEncountered = true;
                            break;
                            }
                            default:
                            {
                                ::ShowErrorMessage("Invalid option. Usage: [0-42]");
                            break;
                            }
                        }
                    }
    
                break;
                }
                case 2:
                {
                    
                    bool fSuccess = ::RegisterOrganizationAndSuperUser();
                    if (true == fSuccess)
                    {
                        std::cout << "Registration successful!\nLog in to access your dashboard.\n";
                    }
                    ::WaitForUserToContinue();
                break;
                }
                case 0:
                {
                    fTerminatedSignalEncountered = true;
                break;
                }
                default:
                {
                    std::cout << "Invalid option. Usage: [0-2]" << std::endl;
                break;
                }
            }

        }
    }
    catch(BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }

    return 0;
}