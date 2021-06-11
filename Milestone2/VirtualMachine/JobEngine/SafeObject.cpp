/*********************************************************************************************
 *
 * @file SafeObject.cpp
 * @author Prawal Gangwar
 * @date 4 June 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "JobEngine.h"
#include "DebugLibrary.h"

#include <iostream>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Creates a file with the content from the buffer
 *
 ********************************************************************************************/

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

/********************************************************************************************
 *
 * @class SafeObject
 * @function SafeObject
 * @brief Constructor to create a SafeObject object
 *
 ********************************************************************************************/

SafeObject::SafeObject(
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();

    // Get the safe object UUID
    m_strSafeObjectIdentifier = c_oStructuredBuffer.GetString("SafeObjectUuid");

    // Write the executable file to file system to run
    ::BytesToFile(m_strSafeObjectIdentifier, c_oStructuredBuffer.GetBuffer("Payload"));

    // Make the file executable
    ::chmod(m_strSafeObjectIdentifier.c_str(), S_IRWXU);

    // Add the command that is to be executed.
    m_strCommandToExecute = "python3 " + m_strSafeObjectIdentifier;
}

/********************************************************************************************
 *
 * @class SafeObject
 * @function ~SafeObject
 * @brief Destructor for the SafeObject
 *
 ********************************************************************************************/

SafeObject::~SafeObject(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class SafeObject
 * @function SafeObject
 * @brief Constructor to create a SafeObject object
 *
 ********************************************************************************************/

int __thiscall SafeObject::Run(
    _in const std::string & c_strJobUuid
) const
{
    __DebugFunction();

    FILE * in = ::popen(m_strCommandToExecute.c_str(), "r");

    std::string strOutputString;
    strOutputString.resize(1024);
    ::fread((void *)strOutputString.c_str(), 1, 1024, in);
    std::cout << "Run Job Out:\n " << strOutputString << std::endl;
    int nProcessExitStatus = ::pclose(in);
    std::cout << "Process exit with status: " << nProcessExitStatus << std::endl;

    return nProcessExitStatus;
}

/********************************************************************************************
 *
 * @class SafeObject
 * @function GetSafeObjectIdentifier
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

const std::string & __thiscall SafeObject::GetSafeObjectIdentifier(void) const
{
    __DebugFunction();

    return m_strSafeObjectIdentifier;
}


/********************************************************************************************
 *
 * @class SafeObject
 * @function GetSafeObjectIdentifier
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

const std::string & __thiscall SafeObject::GetCommandToExecute(void) const
{
    __DebugFunction();

    return m_strCommandToExecute;
}
