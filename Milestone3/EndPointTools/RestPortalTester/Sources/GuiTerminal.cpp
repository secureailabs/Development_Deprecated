/*********************************************************************************************
 *
 * @file GuiTerminal.cpp
 * @author Shabana Akhtar Baig
 * @date 05 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "GuiTerminal.h"
#include "DebugLibrary.h"

#include <string>
#include <vector>

/********************************************************************************************/

FontColor oFontColor;

std::string strLoginMenu = oFontColor.strWhite + "1- Login \n" +
            oFontColor.strWhite + "2- Sign up \n\n" +
            oFontColor.strRed + "0- Exit\n";

std::string strTopMenu = oFontColor.strWhite + "Menu: \n\n" +
            oFontColor.strMagenta + "1- Add a new user - Admin privileges required!\n" +
            oFontColor.strMagenta + "2- Get list of organizations - Sail Admin privileges required!\n" +
            oFontColor.strMagenta + "3- Get list of all users - Sail Admin privileges required!\n" +
            oFontColor.strMagenta + "4- Get list of organization users - Admin privileges required!\n" +
            oFontColor.strMagenta + "5- Get organization information\n" +
            oFontColor.strMagenta + "6- Update organization information - Admin privileges required!\n" +
            oFontColor.strMagenta + "7- Update user's access rights - Admin privileges required!\n" +
            oFontColor.strMagenta + "8- Update user information\n" +
            oFontColor.strMagenta + "9- Update password\n" +
            oFontColor.strMagenta + "10- Delete a user - Admin privileges required!\n" +
            oFontColor.strMagenta + "11- Recover a user - Admin privileges required!\n" +
            oFontColor.strCyan + "12- Register a virtual machine\n" +
            oFontColor.strCyan + "13- Register a virtual machine event for DOO and its leaf events\n" +
            oFontColor.strCyan + "14- Register a virtual machine event for RO and its leaf events\n" +
            oFontColor.strCyan + "15- Update virtual machine status\n" +
            oFontColor.strCyan + "16- Get virtual machine status\n" +
            oFontColor.strCyan + "17- Get list of virtual machines associated with an organization\n" +
            oFontColor.strCyan + "18- Get list of all events of the organization\n" +
            oFontColor.strCyan + "19- Get list of events for a specified parent\n" +
            oFontColor.strBlue + "20- Register a digital contract\n" +
            oFontColor.strBlue + "21- Get list of all digital contracts of your organization\n" +
            oFontColor.strBlue + "22- Pull a digital contract\n" +
            oFontColor.strBlue + "23- Approve a digital contract - Dataset Admin privileges required\n" +
            oFontColor.strBlue + "24- Activate a digital contract - Digital Contract Admin privileges required\n" +
            oFontColor.strBlue + "25- Get digital contract provisioning status\n" +
            oFontColor.strYellow + "26- Register a dataset\n" +
            oFontColor.strYellow + "27- Get list of all available datasets\n" +
            oFontColor.strYellow + "28- Get metadata of desired dataset\n" +
            oFontColor.strYellow + "29- Delete a dataset - Dataset Admin privileges required!\n" +
            oFontColor.strBlue + "30- Register an Azure settings template - Admin privileges required!\n" +
            oFontColor.strBlue + "31- Get list of Azure settings template - Admin privileges required!\n" +
            oFontColor.strBlue + "32- Get an Azure settings template - Admin privileges required!\n" +
            oFontColor.strBlue + "33- Update Azure settings template - Admin privileges required!\n" +
            oFontColor.strBlue + "34- Delete Azure settings template - Admin privileges required!\n" +
            oFontColor.strBlue + "35- Associate Digital Contract(s) with an Azure settings template - Admin privileges required!\n" +
            oFontColor.strYellow + "36- Register a remote data connector\n" +
            oFontColor.strYellow + "37- Get list of remote data connectors\n" +
            oFontColor.strYellow + "38- Pull a remote data connector's information\n" +
            oFontColor.strYellow + "39- Update a remote data connector\n" +
            oFontColor.strYellow + "40- Update a remote data connector\n" +
            oFontColor.strMagenta + "41- Send remote data connector's heartbeat\n" +
            oFontColor.strRed + "42- Shut down servers\n" +
            oFontColor.strRed + "0- Logout\n";

/********************************************************************************************/

void __stdcall ClearScreen(void) throw()
{
    __DebugFunction();

    std::cout << u8"\033[2J\033[1;1H";
}

/********************************************************************************************/

void __stdcall ShowErrorMessage(
  _in std::string strErrorMessage
  ) throw()
{
    __DebugFunction();

    std::cout << oFontColor.strRed << strErrorMessage << oFontColor.strWhite << std::endl;
}

/********************************************************************************************/

void __stdcall ShowSuccessMessage(
  _in std::string strSuccessMessage
  ) throw()
{
    __DebugFunction();

    std::cout << oFontColor.strGreen << strSuccessMessage << oFontColor.strWhite << std::endl;
}

/********************************************************************************************/

void __stdcall ShowLoginMenu(void) throw()
{
    __DebugFunction();

    ::ClearScreen();
    std::cout << "************************\n  SAIL DEMO\n************************\n" << std::endl;
    std::cout << strLoginMenu << std::endl;
    std::cout << oFontColor.strWhite << std::endl;
}

/********************************************************************************************/

void __stdcall ShowTopMenu(void) throw()
{
    __DebugFunction();

    ::ClearScreen();
    std::cout << strTopMenu << std::endl;
    std::cout << oFontColor.strWhite << std::endl;
}

/********************************************************************************************/

void __stdcall WaitForUserToContinue(void) throw()
{
    __DebugFunction();

    std::cout << oFontColor.strWhite + "\nPress Enter to Continue ... ";
    while('\n' != std::cin.get());
}
