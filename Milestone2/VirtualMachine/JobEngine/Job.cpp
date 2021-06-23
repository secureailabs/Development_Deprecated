/*********************************************************************************************
 *
 * @file Job.cpp
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Job.h"
#include "DebugLibrary.h"
#include "Exceptions.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <future>

#define cout cout << std::this_thread::get_id() << " "

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

    m_poSafeObject = nullptr;
    m_oParameters.PutBoolean("AllParametersSet", false);

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

void __thiscall Job::SetSafeObject(
    _in SafeObject const * c_poSafeObjectId
    )
{
    __DebugFunction();

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    m_poSafeObject = c_poSafeObjectId;

    // TODO: get a list of all the parameters
    m_stlInputParameters = c_poSafeObjectId->GetListOfParameters();

    this->TryRunJob();
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::TryRunJob(void)
{
    __DebugFunction();

    std::cout << "Trying to run job.." << std::endl;

    if (true == this->AreAllParametersSet())
    {
        std::cout << "All parameters set" << std::endl;
        if (0 == m_stlSetOfDependencies.size())
        {
            std::cout << "No dependencoes" << std::endl;
            int nProcessExitStatus = m_poSafeObject->Run(m_strJobUuid);
        }
    }
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

    // TODO: get a different lock for this
    std::lock_guard<std::mutex> lock(m_oMutexJob);

    // TODO: think of a better way to do this
    if (true == m_oParameters.IsElementPresent(c_strParameterIdentifier.c_str(), INDEXED_BUFFER_VALUE_TYPE))
    {
        StructuredBuffer oThisParameter = m_oParameters.GetStructuredBuffer(c_strParameterIdentifier.c_str());
        oThisParameter.PutString(std::to_string(nParameterIndex).c_str(), c_strParameterIdentifier);

        if (nExpectedParameters == (oThisParameter.GetNamesOfElements().size() - 1))
        {
            oThisParameter.PutBoolean("AllValueSet", true);
        }
        m_oParameters.PutStructuredBuffer(c_strParameterIdentifier.c_str(), oThisParameter);
    }
    else
    {
        StructuredBuffer oNewParameter;
        oNewParameter.PutBoolean("AllValueSet", false);
        oNewParameter.PutString(std::to_string(nParameterIndex).c_str(), c_strParameterIdentifier);

        if (nExpectedParameters == (oNewParameter.GetNamesOfElements().size() - 1))
        {
            oNewParameter.PutBoolean("AllValueSet", true);
        }
        else
        {
            oNewParameter.PutBoolean("AllValueSet", false);
        }
        m_oParameters.PutStructuredBuffer(c_strParameterIdentifier.c_str(), oNewParameter);
    }

    // Once the parameter is set we try to run the job if all the paramets are set and good to go
    this->TryRunJob();
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::RemoveAvailableDependency(
    _in const std::string & c_strDependencyName
    ) throw()
{
    __DebugFunction();

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    m_stlSetOfDependencies.erase(c_strDependencyName);

    if (0 == m_stlSetOfDependencies.size())
    {
        this->TryRunJob();
    }
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::AddDependency(
    _in const std::string & c_strDependencyName
    ) throw()
{
    __DebugFunction();

    m_stlSetOfDependencies.insert(c_strDependencyName);
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
    ) throw()
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
const std::string & __thiscall Job::GetOutputFileName(void) const throw()
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
const std::string & __thiscall Job::GetJobId(void) const throw()
{
    __DebugFunction();

    return m_strJobUuid;
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/
bool __thiscall Job::AreAllParametersSet(void) throw()
{
    __DebugFunction();

    bool fIsComplete = false;

    // Initial check if the StructuredBuffer is already marked complete
    if (nullptr == m_poSafeObject)
    {
        fIsComplete = false;
    }
    else if (true == m_oParameters.GetBoolean("AllParametersSet"))
    {
        fIsComplete = true;
    }
    else
    {
        // Check if all values are set, then mark the whole StructuredBuffer as set.
        auto stlListOfParameters = m_oParameters.GetNamesOfElements();
        if (m_stlInputParameters.size() == (stlListOfParameters.size() - 1))
        {
            bool fAllDone = true;
            for(std::string strParameterUuid : stlListOfParameters)
            {
                if ("AllParametersSet" != strParameterUuid)
                {
                    StructuredBuffer oStructuredBufferParameter = m_oParameters.GetStructuredBuffer(strParameterUuid.c_str());
                    if (false == oStructuredBufferParameter.GetBoolean("AllValueSet"))
                    {
                        fAllDone = false;
                        break;
                    }
                }
            }
            std::cout << "All paramter set status " << fAllDone << std::endl;
            m_oParameters.PutBoolean("AllParametersSet", fAllDone);
            fIsComplete = fAllDone;
        }
    }
    return fIsComplete;
}
