/*********************************************************************************************
 *
 * @file GuiTerminal.h
 * @author Shabana Akhtar Baig
 * @date 05 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "StructuredBuffer.h"

#include <iostream>
#include <string>

/********************************************************************************************/

typedef struct color
{
    std::string strBlack = "\u001b[30m";
    std::string strRed = "\u001b[31m";
    std::string strGreen = "\u001b[32m";
    std::string strYellow = "\u001b[33m";
    std::string strBlue = "\u001b[34m";
    std::string strMagenta = "\u001b[35m";
    std::string strCyan = "\u001b[36m";
    std::string strWhite = "\u001b[37m";
    std::string strReset = "\u001b[0m";
}
FontColor;

extern std::string strTopMenu;

extern void __stdcall ClearScreen(void) throw();

extern void __stdcall ShowErrorMessage(
  _in std::string strErrorMessage
  ) throw();

extern void __stdcall ShowSuccessMessage(
  _in std::string strSuccessMessage
  ) throw();

extern void __stdcall ShowLoginMenu(void) throw();

extern void __stdcall ShowTopMenu(void) throw();

extern void __stdcall WaitForUserToContinue(void) throw();
