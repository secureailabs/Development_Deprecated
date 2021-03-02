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
            oFontColor.strGreen + "2- Register a root event\n" +
            oFontColor.strGreen + "3- Register a virtual machine and its leaf events\n" +
            oFontColor.strCyan + "4- Get list of all events of the organization\n" +
            oFontColor.strCyan + "5- Get list of events for a specified parent\n" +
            oFontColor.strYellow + "0- Logout\n";

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

    std::cout << oFontColor.strRed << strErrorMessage << std::endl;
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
