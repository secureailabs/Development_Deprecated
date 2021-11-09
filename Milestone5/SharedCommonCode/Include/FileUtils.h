/*********************************************************************************************
 *
 * @file FileUtils.h
 * @author Prawal Gangwar
 * @date 16 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"

#include <vector>
#include <string>

std::vector<Byte> __stdcall ReadFileAsByteBuffer(
    const std::string c_strFileName
);

std::string __stdcall ReadFileAsString(
    _in const std::string & c_strFileName
);

void __stdcall WriteBytesAsFile(
    _in const std::string c_strFileName,
    _in const std::vector<Byte> c_stlFileData
);

void WriteStringAsFile(
    _in const std::string c_strFileName,
    _in const std::string c_strFileData
);
