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
            oFontColor.strRed + "0- Shut down servers\n" +
            oFontColor.strMagenta + "1- Add a new user - Admin privileges required!\n" +
            oFontColor.strMagenta + "2- Get list of organizations - Admin privileges required!\n" +
            oFontColor.strMagenta + "3- Update organization information - Admin privileges required!\n" +
            oFontColor.strMagenta + "4- Delete an organization - Admin privileges required!\n" +
            oFontColor.strMagenta + "5- Delete a user - Admin privileges required!\n" +
            oFontColor.strCyan + "6- Register a virtual machine\n" +
            oFontColor.strCyan + "7- Register a virtual machine event for DOO and its leaf events\n" +
            oFontColor.strCyan + "8- Register a virtual machine event for RO and its leaf events\n" +
            oFontColor.strCyan + "9- Get list of all events of the organization\n" +
            oFontColor.strCyan + "10- Get list of events for a specified parent\n" +
            oFontColor.strBlue + "11- Register a digital contract\n" +
            oFontColor.strBlue + "12- Get list of all digital contracts of your organization\n" +
            oFontColor.strBlue + "13- Pull a digital contract\n" +
            oFontColor.strBlue + "14- Approve a digital contract - Dataset Admin privileges required\n" +
            oFontColor.strBlue + "15- Activate a digital contract - Digital Contract Admin privileges required\n" +
            oFontColor.strYellow + "16- Register a dataset\n" +
            oFontColor.strYellow + "17- Get list of all available datasets\n" +
            oFontColor.strYellow + "18- Get metadata of desired dataset\n" +
            oFontColor.strYellow + "19- Delete a dataset - Dataset Admin privileges required!\n" +
            oFontColor.strYellow + "20- Logout\n";

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
