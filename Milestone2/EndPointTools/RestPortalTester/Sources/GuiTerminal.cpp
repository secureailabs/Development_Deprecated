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

std::string strTopMenu = oFontColor.strWhite + "Menu: \n\n" +
            oFontColor.strGreen + "1- Register Dataset\n" +
            oFontColor.strGreen + "2- Delete Dataset\n" +
            oFontColor.strGreen + "3- Get list of all Available Datasets\n" +
            oFontColor.strGreen + "4- Get list of Datasets Submitted within your Organization\n" +
            oFontColor.strBlue + "5- Register Digital Contract\n" +
            oFontColor.strBlue + "6- Get list of Digital Contracts associated with your Organization\n" +
            oFontColor.strBlue + "7- Get list of Waiting Digital Contracts associated with your Organization\n" +
            oFontColor.strYellow + "8- Register VM Instance\n" +
            oFontColor.strYellow + "9- Get VM Instance's Status\n" +
            oFontColor.strYellow + "0- Quit\n";

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
