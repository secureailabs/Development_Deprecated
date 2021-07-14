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

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>

static const char * gsc_szPrintableCharacters = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789`~!@#$%^&*()_-+={[}]|\\:;\"'<,>.?/";
static const char * gsc_szNumericCharacters = "0123456789";
static const char * gsc_szIpAddressCharacters = "0123456789.";
static const char * gsc_szAddRemoveNodeInputCharacters = "aArRdD";
static const char * gsc_szYesNoInputCharacters = "yYnN";

void BytesToFile(
    _in const std::string c_strFileName,
    _in const std::vector<Byte> c_stlFileData
)
{
    __DebugFunction();

    std::ofstream stlFileToWrite(c_strFileName, std::ios::out | std::ofstream::binary);
    std::copy(c_stlFileData.begin(), c_stlFileData.end(), std::ostreambuf_iterator<char>(stlFileToWrite));
    stlFileToWrite.close();
}

std::vector<Byte> FileToBytes(
    const std::string c_strFileName
)
{
    __DebugFunction();

    std::vector<Byte> stlFileData;

    std::ifstream stlFile(c_strFileName.c_str(), (std::ios::in | std::ios::binary | std::ios::ate));
    if (true == stlFile.good())
    {
        unsigned int unFileSizeInBytes = (unsigned int) stlFile.tellg();
        stlFileData.resize(unFileSizeInBytes);
        stlFile.seekg(0, std::ios::beg);
        stlFile.read((char *)stlFileData.data(), unFileSizeInBytes);
        stlFile.close();
    }
    else
    {
        _ThrowBaseException("Invalid File Path: %s", nullptr, c_strFileName.c_str());
    }
    return stlFileData;
}

std::string ReadFileAsString(
    _in std::string c_strFileName
)
{
    __DebugFunction();

    std::ifstream stlFile(c_strFileName, std::ios::ate);
    std::streamsize nSizeOfFile = stlFile.tellg();
    stlFile.seekg(0, std::ios::beg);
    std::string strFileContent;
    strFileContent.resize(nSizeOfFile);
    stlFile.read(strFileContent.data(), nSizeOfFile);

    stlFile.close();
    return strFileContent;
}

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

    Guid oGuid;
    StructuredBuffer oStructuredBuffer;

    std::string strTitle = ::GetStringInput("Title : ", 64, false, gsc_szPrintableCharacters);
    oStructuredBuffer.PutString("Title", strTitle);
    oStructuredBuffer.PutString("Uuid", oGuid.ToString(eRaw));

    std::string strDescription = ::GetStringInput("Description : ", 64, false, gsc_szPrintableCharacters);
    oStructuredBuffer.PutString("Description", strDescription);

    std::string nPythonCode = ::GetStringInput("Python Code Filename: ", 128, false, gsc_szPrintableCharacters);
    std::string strPythonCode = ::ReadFileAsString(nPythonCode);
    ::ReplaceAll(strPythonCode, "\n", "\n        ");

    int nNumberOfInputs = std::stoi(::GetStringInput("Number of Inputs: ", 2, false, gsc_szNumericCharacters).c_str());
    oStructuredBuffer.PutWord("NumberOfInputs", nNumberOfInputs);

    std::string strSafeObjectTemplate = ::ReadFileAsString("SafeObjectTemplate");
    ::ReplaceAll(strSafeObjectTemplate, "{{code}}", strPythonCode);
    ::ReplaceAll(strSafeObjectTemplate, "{{safeObjectId}}", oGuid.ToString(eRaw));

    StructuredBuffer oInputParameters;
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
    oStructuredBuffer.PutStructuredBuffer("InputParamters", oInputParameters);

    int nNumberOfOutputs = std::stoi(::GetStringInput("Number of Outputs: ", 2, false, gsc_szNumericCharacters).c_str());

    StructuredBuffer oOutputParamter;
    for (int nOutputIndex = 0; nOutputIndex < nNumberOfOutputs; nOutputIndex++)
    {
        Guid oOutputGuid;
        ::ReplaceAll(strSafeObjectTemplate, "{{OutputUuid}}", oOutputGuid.ToString(eRaw));

        std::string strToPrint = "Output "+ std::to_string(nOutputIndex) + " Name : ";
        std::string strParameterName = ::GetStringInput(strToPrint.c_str(), 64, false, gsc_szPrintableCharacters);

        ::ReplaceAll(strSafeObjectTemplate, strParameterName, "self.m_"+oOutputGuid.ToString(eRaw));

        oOutputParamter.PutString("Uuid", oOutputGuid.ToString(eRaw));
    }
    oStructuredBuffer.PutStructuredBuffer("OutputParamter", oOutputParamter);
    oStructuredBuffer.PutString("Payload", strSafeObjectTemplate);

    std::cout << "The Strucutred BUffer is \n" << oStructuredBuffer.ToString();
    ::BytesToFile(oGuid.ToString(eRaw) + ".safe", oStructuredBuffer.GetSerializedBuffer());
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
