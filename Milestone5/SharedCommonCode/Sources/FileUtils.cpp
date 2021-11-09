/*********************************************************************************************
 *
 * @file FileUtils.cpp
 * @author Prawal Gangwar
 * @date 16 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "FileUtils.h"
#include "Exceptions.h"

#include <iostream>
#include <fstream>

/********************************************************************************************
 *
 * @function ReadFileAsByteBuffer
 * @brief Read the file as a byte buffer
 * @param[in] c_strFileName File to be read
 * @return Byte vector containing file data
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall ReadFileAsByteBuffer(
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
        _ThrowBaseException("Invalid File Path", nullptr);
    }
    return stlFileData;
}

/********************************************************************************************
 *
 * @function ReadFileAsString
 * @brief Read the file as a std::string
 * @param[in] c_strFileName File to be read
 * @return string containing file data
 *
 ********************************************************************************************/

std::string ReadFileAsString(
    _in const std::string & c_strFileName
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


/********************************************************************************************
 *
 * @function WriteBytesAsFile
 * @brief Create a new binary file with the vector data
 * @param[in] c_strFileName File to be created
 * @param[in] c_stlFileData Byte vector with file data
 *
 ********************************************************************************************/

void WriteBytesAsFile(
    _in const std::string c_strFileName,
    _in const std::vector<Byte> c_stlFileData
)
{
    __DebugFunction();

    std::ofstream stlFileToWrite(c_strFileName, std::ios::out | std::ofstream::binary);
    std::copy(c_stlFileData.begin(), c_stlFileData.end(), std::ostreambuf_iterator<char>(stlFileToWrite));
    stlFileToWrite.close();
}

/********************************************************************************************
 *
 * @function WriteStringAsFile
 * @brief Create a new file with the string data
 * @param[in] c_strFileName File to be created
 * @param[in] c_strFileData String to Write
 *
 ********************************************************************************************/

void WriteStringAsFile(
    _in const std::string c_strFileName,
    _in const std::string c_strFileData
)
{
    __DebugFunction();

    std::ofstream stlOutFile(c_strFileName);
    stlOutFile << c_strFileData;
    stlOutFile.close();
}

