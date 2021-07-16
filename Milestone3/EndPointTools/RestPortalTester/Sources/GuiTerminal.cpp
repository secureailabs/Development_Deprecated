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
            oFontColor.strMagenta + "5- Update organization information - Admin privileges required!\n" +
            oFontColor.strMagenta + "6- Update user's access rights - Admin privileges required!\n" +
            oFontColor.strMagenta + "7- Update user information\n" +
            oFontColor.strMagenta + "8- Update password\n" +
            oFontColor.strMagenta + "9- Delete a user - Admin privileges required!\n" +
            oFontColor.strMagenta + "10- Recover a user - Admin privileges required!\n" +
            oFontColor.strCyan + "11- Register a virtual machine\n" +
            oFontColor.strCyan + "12- Register a virtual machine event for DOO and its leaf events\n" +
            oFontColor.strCyan + "13- Register a virtual machine event for RO and its leaf events\n" +
            oFontColor.strCyan + "14- Get list of all events of the organization\n" +
            oFontColor.strCyan + "15- Get list of events for a specified parent\n" +
            oFontColor.strBlue + "16- Register a digital contract\n" +
            oFontColor.strBlue + "17- Get list of all digital contracts of your organization\n" +
            oFontColor.strBlue + "18- Pull a digital contract\n" +
            oFontColor.strBlue + "19- Approve a digital contract - Dataset Admin privileges required\n" +
            oFontColor.strBlue + "10- Activate a digital contract - Digital Contract Admin privileges required\n" +
            oFontColor.strYellow + "21- Register a dataset\n" +
            oFontColor.strYellow + "22- Get list of all available datasets\n" +
            oFontColor.strYellow + "23- Get metadata of desired dataset\n" +
            oFontColor.strYellow + "24- Delete a dataset - Dataset Admin privileges required!\n" +
            oFontColor.strRed + "25- Shut down servers\n" +
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
