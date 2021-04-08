/*********************************************************************************************
 *
 * @file CommandLine.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 * @brief Implementation of a command line parsing function ParseCommandLineParameters()
 *
 * The ParseCommandLineParameters() functions returns a StructuredBuffer of elements which
 * are structured such that:
 *
 *    1. The StructuredBuffer.GetString() gets the value of a binary parameter. For example, if
 *       we have the command line parameter /file "/home/LuisMiguelHuapaya/File.txt", then the
 *       way to retrieve the 'file' parameter is to call StructuredBuffer.GetString("file")
 *    2. The StructuredBuffer.GetBoolean() checks to see if a unary parameter exists. For
 *       example, if we have the command line parameter /debug, then the way to check to see if
 *       the parameter exists is to call (true == StructuredBuffer.GetBoolean("debug"))
 *
 ********************************************************************************************/

#include "CommandLine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

#include <string.h>         // For memcmp()

#include <algorithm>        // For std::replace
#include <string>           // For std::string
#include <sstream>          // For std::istringstream
#include <vector>           // For std:vector


/********************************************************************************************
 *
 * @function SplitString
 * @param[in] c_oStringToSplit String that needs to be split up using the '=' separator
 * @brief Functions splits an incoming string into an array of strings using the '=' separator
 * @return An array of strings
 *
 ********************************************************************************************/

static std::vector<std::string> __stdcall SplitString(
    _in const std::string & c_oStringToSplit
    ) throw()
{
    __DebugFunction();
    
    std::vector<std::string> stlSplitElements;
    std::istringstream stlStringStream(c_oStringToSplit);
    std::string strElement;
    
    while (std::getline(stlStringStream, strElement, '='))
    {
        stlSplitElements.push_back(strElement);
    }
    
    return stlSplitElements;
}

/********************************************************************************************
 *
 * @function ParseCommandLineParameters
 * @param[in] unNumberOfCommandLineArguments Number of command line arguments
 * @param[in] c_pszCommandLineArguments Array of pointer to character strings
 * @brief Functions parses the incoming array of strings and converts them into a StructuredBuffer of parameters
 * @return A structured buffer of parameters.
 * @note
 *
 *   The ParseCommandLineParameters() functions returns a StructuredBuffer of elements which
 *   are structured such that:
 *
 *      1. The StructuredBuffer.GetString() gets the value of a binary parameter. For example, if
 *         we have the command line parameter /file "/home/LuisMiguelHuapaya/File.txt", then the
 *         way to retrieve the 'file' parameter is to call StructuredBuffer.GetString("file")
 *      2. The StructuredBuffer.GetBoolean() checks to see if a unary parameter exists. For
 *         example, if we have the command line parameter /debug, then the way to check to see if
 *         the parameter exists is to call (true == StructuredBuffer.GetBoolean("debug"))
 *
 ********************************************************************************************/

StructuredBuffer __stdcall ParseCommandLineParameters(
    _in unsigned int unNumberOfCommandLineArguments,
    _in const char ** c_pszCommandLineArguments
    )
{
    __DebugFunction();
    
    StructuredBuffer oParsedCommandLineParameters;
    
    // Linux does a pretty good job of separating parameters, but for each parameter, we need
    // to figure out if this parameter is a unary parameter or a binary parameter. This is where
    // things get tricky. This command line parser considers -, -- and / to be parameter
    // delimiters. If there is a command line like:
    //
    //      command --t crap
    //
    // Then t is a binary parameter with the value crap. But if the the command line is:
    //
    //      command --t --crap
    //
    // Then t and crap are both separate unary parameters. Other example of binary parameters are
    //
    //      command --t=crap
    //      command --t:crap
    //      command --t "A lot of crap in a sentence with some blank spaces, all of it is one value"
    //
    // Thankfully, Linux handles the "" parameters above correctly, so we do not have to deal with
    // that parsing.
    //
    // In order to be efficient at this, we will take in the parameters from last to first. This
    // allows us to efficiently detect binary parameters
    
    bool fIsBinaryParameter = false;
    for (unsigned int unIndex = (unNumberOfCommandLineArguments - 1); unIndex > 0 ; --unIndex)
    {
        std::string strLeftSideParameter;
        // Detect whether the parameter starts with a -, a -- or a /
        if (0 == ::memcmp((const void *) "--", (const void *) c_pszCommandLineArguments[unIndex], 2))
        {
            strLeftSideParameter = &(c_pszCommandLineArguments[unIndex][2]);
        }
        else if (('-' == c_pszCommandLineArguments[unIndex][0])||('/' == c_pszCommandLineArguments[unIndex][0]))
        {
            strLeftSideParameter = &(c_pszCommandLineArguments[unIndex][1]);
        }
        
        if (0 < strLeftSideParameter.size())
        {
            if (true == fIsBinaryParameter)
            {
                __DebugAssert((unIndex + 1) < unNumberOfCommandLineArguments);
                
                // If we think we are dealing with a binary parameter and we get to this point
                // in the logic of the code, then the srtLeftSideParameter is obviously the
                // name of the parameter
                oParsedCommandLineParameters.PutString(strLeftSideParameter.c_str(), c_pszCommandLineArguments[unIndex + 1]);
                // Make sure to reset fIsBinaryParameter to false before proceeding with
                // parsing more parameters
                fIsBinaryParameter = false;
            }
            else
            {
                // We are going to attempt to split the string to test whether or not
                // we are dealing with a binary operator of the "a:b" or "a=b" form, which
                // somewhat masquerades as a left side parameter when parsed by Linux. To
                // simplify out lives, we do a quick replace call so that any occurrences of
                // : are converted to =
                std::replace(strLeftSideParameter.begin(), strLeftSideParameter.end(), ':', '=');
                std::vector<std::string> stlSplitElements = ::SplitString(strLeftSideParameter);
                
                if (1 == stlSplitElements.size())
                {
                    // We are dealing with a unary parameter since SplitString() didnt' split
                    // the original string. So we are dealing with a parameter like
                    // '/paramname'
                    oParsedCommandLineParameters.PutBoolean(strLeftSideParameter.c_str(), true);
                }
                else if (2 == stlSplitElements.size())
                {
                    // We are dealing with a binary parameter since the SplitString() function
                    // has returned an array of two strings (i.e. the original parameter was
                    // something like '/paramname=paramvalue' and the SplitString() function
                    // has split the string into the two string "paramname" and "paramvalue"
                    oParsedCommandLineParameters.PutString(stlSplitElements.at(0).c_str(), stlSplitElements.at(1).c_str());
                }
                else
                {
                    // We are dealing with an invalid parameter is the SplitString() function
                    // returns an array of strings that has more than two elements
                    _ThrowBaseException("Invalid command line format.", nullptr);
                }
            }
        }
        else
        {
            // If we think we already are dealing with a binary parameter, then the command
            // line is improperly formatted. This happens when we get two values like 'one two'
            // where neither of the values get the -, -- or / prefix and the two values are not
            // safeguarded as one using string quotes such as "one two"
            _ThrowBaseExceptionIf((true == fIsBinaryParameter), "Invalid command line format.", nullptr);
            // If the code didn't throw an exception, then we should be dealing with a binary
            // parameters of the form '--paramname paramvalue', whereupon the current element that
            // we just parsed/detected is the paramvalue portion
            fIsBinaryParameter = true;
        }
    }
    
    return oParsedCommandLineParameters;
}