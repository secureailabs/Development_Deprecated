/*********************************************************************************************
 *
 * @file ConsoleInputHelperFunctions.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/
 
#pragma once
 
#include "CoreTypes.h"
#include <string>
 
/********************************************************************************************/

extern char __stdcall GetCharacterInput(
    _in const char * c_szPrompt,
    _in bool fEcho,
    _in const char * c_szValidInputCharacters
    ) throw();

extern std::string __stdcall GetStringInput(
    _in const char * c_szPrompt,
    _in unsigned int unMaximumLengthInCharacters,
    _in bool fIsPassword,
    _in const char * c_szValidInputCharacters
    ) throw();