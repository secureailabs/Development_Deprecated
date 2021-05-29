/*********************************************************************************************
 *
 * @file Job.cpp
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "JobEngine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <future>

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

Job::Job(
    _in const std::string c_strJobUuid
    ) :m_strJobUuid(c_strJobUuid)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

Job::Job(
    _in const std::string & c_strJobUuid,
    _in const SafeObject * const c_poSafeObject
    ) :m_strJobUuid(c_strJobUuid), m_poSafeObject(c_poSafeObject)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class Job
 * @function ~Job
 * @brief Destructor for the Job
 *
 ********************************************************************************************/

Job::~Job(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

int __thiscall Job::TryRunJob(
    _in const std::string & c_strParameterFileName
    )
{
    __DebugFunction();

    int nProcessExitStatus = m_poSafeObject->Run(m_strJobUuid);

    return nProcessExitStatus;
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::SetParameter(
    _in const std::string & c_strParameterIdentifier,
    _in const std::string & c_strValueIdentifier,
    _in unsigned int nExpectedParameters,
    _in unsigned int nParameterIndex
    )
{
    __DebugFunction();

    if (0 == m_nNumberOfParameters)
    {
        m_nNumberOfParameters = nExpectedParameters;
    }
    else if (nExpectedParameters != m_nNumberOfParameters)
    {
        _ThrowBaseException("Number of parameters mismatch", nullptr);
    }

    m_stlInputParameters.insert(std::make_pair(nParameterIndex, c_strParameterIdentifier));
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

const std::vector<std::string> & __thiscall Job::GetInputParameters(void) const
{
    __DebugFunction();

    return m_stlInputParameters;
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::SetOutputFileName(
    _in const std::string & strOutFileName
    )
{
    __DebugFunction();

    m_stlOutputFileName = strOutFileName;
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/
const std::string & __thiscall Job::GetOutputFileName(void) const
{
    __DebugFunction();

    return m_stlOutputFileName;
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/
const std::string & __thiscall Job::GetJobId(void) const
{
    __DebugFunction();

    return m_strJobUuid;
}
