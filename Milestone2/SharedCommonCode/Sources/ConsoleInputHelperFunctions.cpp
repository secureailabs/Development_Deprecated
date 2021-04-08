/*********************************************************************************************
 *
 * @file ConsoleInputHelperFunctions.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ConsoleInputHelperFunctions.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>

/********************************************************************************************/

static char GetCharacter(
    _in bool fEcho
    )
{
    __DebugFunction();
    
    struct termios sOldTerminalSettings, sNewTerminalSettings;
    int nCharacter;;

    ::tcgetattr( STDIN_FILENO, &sOldTerminalSettings);
    sNewTerminalSettings = sOldTerminalSettings;
    sNewTerminalSettings.c_lflag &= ~( ICANON | ECHO );
    ::tcsetattr( STDIN_FILENO, TCSANOW, &sNewTerminalSettings );

    nCharacter = ::getchar();
    
    ::tcsetattr( STDIN_FILENO, TCSANOW, &sOldTerminalSettings );

    return (char) nCharacter;
}

/********************************************************************************************/

char __stdcall GetCharacterInput(
    _in const char * c_szPrompt,
    _in bool fEcho,
    _in const char * c_szValidInputCharacters
    ) throw()
{
    __DebugFunction();
    
    char cInputCharacter;
    bool fDone = false;
    struct termios sOldTerminalSettings, sNewTerminalSettings;
    
    // Print out the prompt
    std::cout << c_szPrompt << std::endl;
    
    do
    {
        cInputCharacter = ::GetCharacter(fEcho);
        if (nullptr != ::strchr(c_szValidInputCharacters, (int) cInputCharacter))
        {
            // We have entered a valid character
            fDone = true;
        }
        else
        {
            // Make a beep sound to show that the attempt to input an invalid
            // character failed
            std::cout << '\a';
        }
    }
    while (false == fDone);
    
    return cInputCharacter;
}

/********************************************************************************************/

std::string __stdcall GetStringInput(
    _in const char * c_szPrompt,
    _in unsigned int unMaximumLengthInCharacters,
    _in bool fIsPassword,
    _in const char * c_szValidInputCharacters
    ) throw()
{
    __DebugFunction();

    std::string strInputString;
    bool fDone = false;
    struct termios sOldTerminalSettings, sNewTerminalSettings;
    
    // Print out the prompt
    std::cout << c_szPrompt;
    
    do
    {
        char cInputCharacter = ::GetCharacter(fIsPassword);
        if (0x0A == cInputCharacter) // Check to see if the enter key has been pressed
        {
            // If we were entering a password, we need to delete all of the '*' characters
            // we were printing since anyone looking at the stdout history of the console
            // would be able to determine the size of the password by counting the number
            // of '*' characters
            for (unsigned int unIndex = 0; unIndex < strInputString.size(); ++unIndex)
            {
                std::cout << " \b \b";
            }
            // Make sure to print out a carriage return
            std::cout << std::endl;
            // Make sure to break out of the while loop since we are done
            fDone = true;
        }
        else if (0x08 == cInputCharacter) // Check to see if the backspace key has been pressed
        {
            // We can only delete the last character in the string if the string has a size
            // greater than 0
            if (0 < strInputString.size())
            {
                // Erase the last character printed on cout
                std::cout << " \b \b";
                // Delete the last character in the string
                strInputString.resize(strInputString.size() - 1);
            }
        }
        else if (unMaximumLengthInCharacters > strInputString.size())
        {
            // Make sure this is a valid input character
            if (nullptr != ::strchr(c_szValidInputCharacters, (int) cInputCharacter))
            {
                // Append the new character to the string
                strInputString += cInputCharacter;
                if (true == fIsPassword)
                {
                    // Print out the password character
                    std::cout << '*';
                }
                else
                {
                    // Print out the actual character
                    std::cout << cInputCharacter;
                }
            }
            else
            {
                // Make a beep sound to show that the attempt to input a non-printable
                // character failed
                std::cout << '\a';
            }
        }
        else
        {
            // Make a beep sound to show that we have reached the maximum string length
            // and no other characters are being accepted as input. Only the RETURN or
            // BACKSPACE will work 
            std::cout << '\a';
        }
    }
    while (false == fDone);
    
    return strInputString;
}