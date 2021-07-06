/*********************************************************************************************
 *
 * @file JobEngineHelper.h
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the JobEngine class to create and run jobs on the Virtual Machine
 *
 ********************************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <iostream>

/********************************************************************************************/

const std::string gc_strJobsSignalFolderName = "SignalAllJobs";
const std::string gc_strHaltAllJobsSignalFilename = "StopAllJobs";
const std::string gc_strSignalFolderName = "DataSignals";
const std::string gc_strDataFolderName = "DataFiles";

/********************************************************************************************/

void * __stdcall FileSystemWatcherThread(void * poThreadParameter);

std::vector<Byte> FileToBytes(
    const std::string c_strFileName
);

void BytesToFile(
    _in const std::string c_strFileName,
    _in const std::vector<Byte> c_stlFileData
);
