/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Prawal Gangwar
 * @date 14 July 2021
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ConsoleInputHelperFunctions.h"
#include "StructuredBuffer.h"
#include "FileUtils.h"

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>

static const char * gsc_szPrintableCharacters = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szNumericCharacters = "0123456789";
static const char * gsc_szIpAddressCharacters = "0123456789.";
static const char * gsc_szAddRemoveNodeInputCharacters = "aArRdD";
static const char * gsc_szYesNoInputCharacters = "yYnN";

void __thiscall ReplaceAll(
    _inout std::string & strOriginalString,
    _in const std::string & c_strChangeThis,
    _in const std::string & c_strChangeTo)
{
    try
    {
        size_t start_pos = 0;
        while((start_pos = strOriginalString.find(c_strChangeThis, start_pos)) != std::string::npos)
        {
            strOriginalString.replace(start_pos, c_strChangeThis.length(), c_strChangeTo);
            start_pos += c_strChangeTo.length();
        };
    }
    catch(const std::exception & oException)
    {
        std::cout << "std Exception in RepaceAll " << oException.what() << '\n';
    }
}

void PackageSafeObject(void)
{
    __DebugFunction();

    Guid oSafeObjectGuid;

    std::string strTitle = ::GetStringInput("Title : ", 64, false, gsc_szPrintableCharacters);
    std::string strDescription = ::GetStringInput("Description : ", 64, false, gsc_szPrintableCharacters);

    // The Python code in the file would not have indentation as the template. The file code is just a body
    // of a function with inputs and possible multiple or none outputs.
    std::string nPythonCode = ::GetStringInput("Python Code Filename: ", 128, false, gsc_szPrintableCharacters);
    std::string strPythonCode = ::ReadFileAsString(nPythonCode);
    ::ReplaceAll(strPythonCode, "\n", "\n        ");

    // Read the SafeObject Template and add the python code to it and make it specific to this SafeObject
    std::string strSafeObjectTemplate = ::ReadFileAsString("SafeObjectTemplate");
    ::ReplaceAll(strSafeObjectTemplate, "{{code}}", strPythonCode);
    ::ReplaceAll(strSafeObjectTemplate, "{{safeObjectId}}", oSafeObjectGuid.ToString(eRaw));

    StructuredBuffer oInputParameters;
    int nNumberOfInputs = std::stoi(::GetStringInput("Number of Inputs: ", 2, false, gsc_szNumericCharacters).c_str());
    for (int nInputIndex = 0; nInputIndex < nNumberOfInputs; nInputIndex++)
    {
        StructuredBuffer oParameter;
        Guid oGuid;
        std::string strParamGuid = oGuid.ToString(eRaw);
        std::string strToPrint = "Input "+ std::to_string(nInputIndex) + " Name : ";
        std::string strParameterName = ::GetStringInput(strToPrint.c_str(), 64, false, gsc_szPrintableCharacters);
        std::string strParameterDescription = ::GetStringInput("Description: ", 512, false, gsc_szPrintableCharacters);

        // Replace the file variables
        ::ReplaceAll(strSafeObjectTemplate, strParameterName, "self.m_"+strParamGuid);

        // TODO: use this, json is temporary
        // std::string strStringToGetParameterFromFile = "self.m_"+ strParamGuid + " = pickle.load(oInputParameters.GetStructuredBuffer(\""+ strParamGuid +"\").GetString(\"0\"))";

        std::string strStringToGetParameterFromFile = "self.m_"+ strParamGuid + " = pickle.load(open(oInputParameters[\""+ strParamGuid +"\"][\"0\"], 'rb'))";
        if (nInputIndex != (nNumberOfInputs-1))
        {
            strStringToGetParameterFromFile += "\n        {{ParamterSet}}\n";
        }
        ::ReplaceAll(strSafeObjectTemplate, "{{ParamterSet}}", strStringToGetParameterFromFile);

        oParameter.PutString("Uuid", oGuid.ToString(eRaw));
        oParameter.PutString("Description", strParameterDescription);

        // Put this strucuted Buffer in the list of StrucutredBuffers of Paramters
        oInputParameters.PutStructuredBuffer(std::to_string(nInputIndex).c_str(), oParameter);
    }

    StructuredBuffer oOutputParameters;
    int nNumberOfOutputs = std::stoi(::GetStringInput("Number of Outputs: ", 2, false, gsc_szNumericCharacters).c_str());
    for (int nOutputIndex = 0; nOutputIndex < nNumberOfOutputs; nOutputIndex++)
    {
        StructuredBuffer oParameter;

        Guid oOutputGuid;
        std::string strOutputParamterGuid = oOutputGuid.ToString(eRaw);

        std::string strToPrint = "Output "+ std::to_string(nOutputIndex) + " Name : ";
        std::string strParameterName = ::GetStringInput(strToPrint.c_str(), 64, false, gsc_szPrintableCharacters);
        std::string strParameterDescription = ::GetStringInput("Description: ", 512, false, gsc_szPrintableCharacters);

        ::ReplaceAll(strSafeObjectTemplate, strParameterName, "self.m_"+oOutputGuid.ToString(eRaw));

        std::string strStringToSetParameterFile = "OutputFileHandler = open(self.m_JobIdentifier+\"." + strOutputParamterGuid + "\",\"wb\")\n        pickle.dump(self.m_" + strOutputParamterGuid + ", OutputFileHandler)\n        with open(\"DataSignals/\" + self.m_JobIdentifier + \"." + strOutputParamterGuid + "\", 'w') as fp:\n            pass";
        if (nOutputIndex != (nNumberOfOutputs-1))
        {
            strStringToSetParameterFile += "\n        {{WriteOutputToFile}}\n";
        }
        ::ReplaceAll(strSafeObjectTemplate, "{{WriteOutputToFile}}", strStringToSetParameterFile);

        oParameter.PutString("Uuid", oOutputGuid.ToString(eRaw));
        oParameter.PutString("Description", strParameterDescription);

        // Put this strucuted Buffer in the list of StrucutredBuffers of Paramters
        oOutputParameters.PutStructuredBuffer(std::to_string(nOutputIndex).c_str(), oParameter);
    }

    // Now the python code is ready and in a state that it can run independently provided all the paramters
    // are present on the <JobId>.inputs file
    StructuredBuffer oStructuredBuffer;
    oStructuredBuffer.PutString("Title", strTitle);
    oStructuredBuffer.PutString("Uuid", oSafeObjectGuid.ToString(eRaw));
    oStructuredBuffer.PutString("Description", strDescription);
    oStructuredBuffer.PutStructuredBuffer("InputParameters", oInputParameters);
    oStructuredBuffer.PutStructuredBuffer("OutputParameters", oOutputParameters);
    oStructuredBuffer.PutString("Payload", strSafeObjectTemplate);

    std::cout << "The Strucutred Buffer is \n" << oStructuredBuffer.ToString();
    ::WriteBytesAsFile(oSafeObjectGuid.ToString(eRaw) + ".safe", oStructuredBuffer.GetSerializedBuffer());
}

/********************************************************************************************/

int __cdecl main(
    int nNumberOfArguments,
    char ** pszCommandLineArguments
    )
{
    __DebugFunction();

    try
    {
        ::PackageSafeObject();
    }
    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    catch (std::exception & oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.what() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    return 0;
}
