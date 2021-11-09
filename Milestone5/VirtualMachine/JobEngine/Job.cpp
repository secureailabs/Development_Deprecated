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
#include "JobEngineHelper.h"
#include "JsonValue.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <future>
#include <filesystem>

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

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    std::cout << "Job destructor called\n";

}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall Job::SetSafeObject(
    _in std::shared_ptr<SafeObject> c_poSafeObjectId
    )
{
    __DebugFunction();

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    m_poSafeObject = c_poSafeObjectId;

    m_stlInputParameters = c_poSafeObjectId->GetListOfParameters();

    std::cout << "m_stlInputParameters.size() is " << m_stlInputParameters.size() << std::endl;

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
    int nProcessExitStatus;

    if (true == this->AreAllParametersSet())
    {
        std::cout << "All parameters set " << m_stlSetOfDependencies.size() << std::endl;
        if (0 == m_stlSetOfDependencies.size())
        {
            std::cout << "No dependencies" << std::endl;
            // If all dependencies are met, the next step is to write the Parameters required for
            // for the SafeObject to run to the file <JobId>.inputs which will be consumed by the
            // running Job and later deleted

            // TODO: this is what will go eventually, but due to some bugs in python implementation
            // StructuredBuffer, we are using the JSON
            // std::cout << m_oParameters.ToString() << std::endl;
            // ::WriteBytesAsFile(m_strJobUuid + ".inputs", m_oParameters.GetSerializedBuffer());
            auto oJsonBody = JsonValue::ParseStructuredBufferToJson(m_oParameters);
            std::string strInputsJson = oJsonBody->ToString();
            std::ofstream out(m_strJobUuid + ".inputs");
            out << strInputsJson;
            out.close();
            oJsonBody->Release();

            // The SafeObject just requires the JobId and BaseFolder name to run
            nProcessExitStatus = m_poSafeObject->Run(m_strJobUuid);
        }
    }

    // TODO: Get the JobEngine to perform a cleanup and remove this object
    // ensuring that no multi-threading memory corruption happens.
}

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

bool __thiscall Job::SetParameter(
    _in const std::string & c_strParameterIdentifier,
    _in const std::string & c_strValueIdentifier,
    _in unsigned int nExpectedParameters,
    _in unsigned int nParameterIndex
    )
{
    __DebugFunction();

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    // TODO: think of a better way to do this
    if (true == m_oParameters.IsElementPresent(c_strParameterIdentifier.c_str(), INDEXED_BUFFER_VALUE_TYPE))
    {
        StructuredBuffer oThisParameter = m_oParameters.GetStructuredBuffer(c_strParameterIdentifier.c_str());
        oThisParameter.PutString(std::to_string(nParameterIndex).c_str(), c_strValueIdentifier);

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
        oNewParameter.PutString(std::to_string(nParameterIndex).c_str(), c_strValueIdentifier);

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

    // If the parameter is a file and does not exist on the file system, we add it as
    // an dependency for the job and wait for it.
    std::string strParameterValueSignalFile =  gc_strSignalFolderName + "/" + c_strValueIdentifier;
    bool fIsSignalFilePresent = std::filesystem::exists(strParameterValueSignalFile.c_str());
    if (false == fIsSignalFilePresent)
    {
        // Add the file to the list of dependencies of the job
        std::cout << "Adding dependency " << c_strValueIdentifier << std::endl;
        this->AddDependency(c_strValueIdentifier);
    }
    else
    {
        // Once the parameter is set we try to run the job if all the paramets are set and good to go
        this->TryRunJob();
    }

    return fIsSignalFilePresent;
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
    )
{
    __DebugFunction();

    std::lock_guard<std::mutex> lock(m_oMutexJob);

    try
    {
        m_stlSetOfDependencies.erase(c_strDependencyName);
        std::cout << "m_stlSetOfDependencies.size() " << m_stlSetOfDependencies.size() << " " << c_strDependencyName << std::endl;
        if (0 == m_stlSetOfDependencies.size())
        {
            this->TryRunJob();
        }
    }
    catch(const BaseException & oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
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
const std::string & __thiscall Job::GetJobUuid(void) const throw()
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
bool __thiscall Job::AreAllParametersSet(void)
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
