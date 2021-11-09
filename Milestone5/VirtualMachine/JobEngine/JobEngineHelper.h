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

StructuredBuffer DataConnectorGetFetchableUuid(void);

std::string DataConnectorGetTable(
    _in std::string strTableUuid,
    _in std::string strTableName
);
