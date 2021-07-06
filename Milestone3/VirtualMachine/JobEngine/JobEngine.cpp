/*********************************************************************************************
 *
 * @file JobEngine.cpp
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "JobEngine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ThreadManager.h"
#include "StatusMonitor.h"
#include "SocketClient.h"
#include "IpcTransactionHelperFunctions.h"
#include "JobEngineHelper.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <iostream>
#include <algorithm>
#include <future>
#include <filesystem>
#include <memory>

#define cout cout << std::this_thread::get_id() << " "

//TODO:
// 1. There is one case where the job can run even after the JobEngine reset has been called
// Last paramter set was called.
// 2. Put all the additional files in the Data folder instead of the cwd, need to delete that
// during reset.
// 3. The SafeObject is not deleted on Job finish/fail. Should it be ? It could be re-used
// by other jobs as well

/********************************************************************************************
 *
 * @function GetJobEngine
 * @brief Gets the singleton instance reference of the JobEngine object
 *
 ********************************************************************************************/

JobEngine & __stdcall JobEngine::Get(void)
{
    __DebugFunction();

    return m_oJobEngine;
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function JobEngine
 * @brief Constructor to create a JobEngine object
 *
 ********************************************************************************************/

JobEngine::JobEngine(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ~JobEngine
 * @brief Destructor for the JobEngine
 *
 ********************************************************************************************/

JobEngine::~JobEngine(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ~JobEngine
 * @brief Destructor for the JobEngine
 *
 ********************************************************************************************/

void JobEngine::StartServer(
    _in Socket * poSocket
)
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    m_poSocket = poSocket;

    // We reset the job engine, it will create all the necessary files and folders necessary
    this->ResetJobEngine();

    // Start listening to requests and fullfill them
    this->ListenToRequests();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ListenToRequests
 * @brief Destructor for the JobEngine
 *
 ********************************************************************************************/

void __thiscall JobEngine::ListenToRequests(void)
{
    __DebugFunction();

    m_fIsEngineRunning = true;
    std::future<void> stlReturn;
    do
    {
        // This should be a blocking call, because we have a persistant connection
        std::cout << "Waiting for request..\n";
        auto stlSerializedBuffer = ::GetIpcTransaction(m_poSocket);
        StructuredBuffer oNewRequest(stlSerializedBuffer);

        // Get the type of request
        EngineRequest eRequestType = (EngineRequest)oNewRequest.GetByte("RequestType");
        switch (eRequestType)
        {
            case EngineRequest::ePushSafeObject
            :
                stlReturn = std::async(std::launch::async, &JobEngine::PushSafeObject, this, oNewRequest);
                break;
            case EngineRequest::ePushdata
            :
                stlReturn = std::async(std::launch::async, &JobEngine::PushData, this, oNewRequest);
                break;
            case EngineRequest::ePullData
            :
                stlReturn = std::async(std::launch::async, &JobEngine::PullData, this, oNewRequest.GetString("Filename"));
                break;
            case EngineRequest::eSubmitJob
            :
                stlReturn = std::async(std::launch::async, &JobEngine::SubmitJob, this, oNewRequest);
                break;
            case EngineRequest::eSetParameters
            :
                stlReturn = std::async(std::launch::async, &JobEngine::SetJobParameter, this, oNewRequest);
                break;
            case EngineRequest::eHaltAllJobs
            :
                // This job does not need to be async, it will block for the cleanup to be performed
                // before new requests can be accpeted and fulfilled
                this->ResetJobEngine();
                break;
            case EngineRequest::eVmShutdown
            :
                {
                    m_fIsEngineRunning = false;
                    // Send a signal of VM Shutdown to the orceshtrator
                    StructuredBuffer oStructuredBufferShutdown;
                    oStructuredBufferShutdown.PutByte("SignalType", (Byte)JobStatusSignals::eVmShutdown);
                    this->SendSignal(oStructuredBufferShutdown);
                }
                break;
            default
            :
                break;
        }
    } while (true == m_fIsEngineRunning);
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushSafeObject
 * @brief
 * @param[in] c_oStructuredBuffer Inout Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::PushSafeObject(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::cout << "Submitting a safe object" << std::endl;

        std::string strSafeObjectUuid = c_oStructuredBuffer.GetString("SafeObjectUuid");

        std::shared_ptr<SafeObject> poSafeObject = nullptr;
        std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
        if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
        {
            // Create a safe object and add it to the map of Guid and Object
            std::cout << "Safe Object not found. Creating new " << strSafeObjectUuid << std::endl;
            poSafeObject = std::make_shared<SafeObject>(strSafeObjectUuid);
            poSafeObject->Setup(c_oStructuredBuffer);
            // Push the safe object to the list of safeObjects in the engine
            m_stlMapOfSafeObjects.insert(std::make_pair(strSafeObjectUuid, poSafeObject));
        }
        else
        {
            poSafeObject = m_stlMapOfSafeObjects.at(strSafeObjectUuid);
            poSafeObject->Setup(c_oStructuredBuffer);

            // Once the safe object setup is complete we notify all the jobs waiting on it
            std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
            for (auto stlJobUuid : poSafeObject->GetQueuedJobsUuid())
            {
                if (m_stlMapOfJobs.end() != m_stlMapOfJobs.find(stlJobUuid))
                {
                    auto poJob = m_stlMapOfJobs.at(stlJobUuid);
                    poJob->SetSafeObject(poSafeObject);
                }
            }
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushData
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::PushData(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::cout << "Pushing data " << c_oStructuredBuffer.GetBuffer("Data").data() << " to file " << c_oStructuredBuffer.GetString("DataId") << std::endl;
        fflush(stdout);

        // Create a new file and put the buffer data
        std::ofstream stlFileToWrite(c_oStructuredBuffer.GetString("DataId"), std::ios::out | std::ofstream::binary);
        auto stlFileData = c_oStructuredBuffer.GetBuffer("Data");
        std::copy(stlFileData.begin(), stlFileData.end(), std::ostreambuf_iterator<char>(stlFileToWrite));
        stlFileToWrite.close();

        // Once the file write is complete we create a signal file for the same
        std::ofstream output(gc_strSignalFolderName + "/" + c_oStructuredBuffer.GetString("DataId"));
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function RunJob
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::PullData(
    _in const std::string & c_strFileNametoSend
)
{
    __DebugFunction();

    try
    {
        std::cout << "Pull Data request" << std::endl;

        // We check if the signal file already exists, if the file exists already
        // we push it to the orchestrator otherwise we will just register the request and
        // wait for the file to be created in future.
        std::string strSignalFile = gc_strSignalFolderName + "/" + c_strFileNametoSend;

        if (true == std::filesystem::exists(strSignalFile.c_str()))
        {
            // As soon as the file we requested for is found, we return it to the
            // orchestrator who is waiting asychronously waiting for it.
            StructuredBuffer oResponse;
            oResponse.PutByte("SignalType", (Byte)JobStatusSignals::ePostValue);
            oResponse.PutString("ValueName", c_strFileNametoSend);
            oResponse.PutBuffer("FileData", ::FileToBytes(c_strFileNametoSend));

            // The Pull data response to the orchestrator is sent as a signal with data
            this->SendSignal(oResponse);

            // We delete the signal file and the data file after we have consumed it.
            ::remove(strSignalFile.c_str());
            ::remove(c_strFileNametoSend.c_str());
        }
        else
        {
            // Register the request for now and fulfill it as soon as it is available later
            std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
            m_stlSetOfPullObjects.insert(c_strFileNametoSend);
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushData
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::SubmitJob(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::cout << "Submitting Job " << c_oStructuredBuffer.GetString("JobUuid") << "with SafeObject " << c_oStructuredBuffer.GetString("SafeObjectUuid") << std::endl;
        fflush(stdout);

        // We find the safeObject if it exists or create an empty object if it does not
        std::string strSafeObjectUuid = c_oStructuredBuffer.GetString("SafeObjectUuid");
        bool fIsSafeObjectNew = true;
        std::shared_ptr<SafeObject> poSafeObject = nullptr;
        std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
        if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
        {
            // Create and Add the SafeObject to the class database
            std::cout << "Safe Object not found. " << strSafeObjectUuid << std::endl;
            poSafeObject = std::make_shared<SafeObject>(strSafeObjectUuid);
            m_stlMapOfSafeObjects.insert(std::make_pair(strSafeObjectUuid, poSafeObject));

            // If the SafeObject is not complete yet. i.e. PushSafeObjet has not been called, we add
            // the Job to the list that waits on SafeObject Creation.
            poSafeObject->AddJobUuidToQueue(c_oStructuredBuffer.GetString("JobUuid"));
        }
        else
        {
            poSafeObject = m_stlMapOfSafeObjects.at(strSafeObjectUuid);
            fIsSafeObjectNew = false;
        }

        // check if the job object already exists, if it does just add the SafeObject to it
        std::shared_ptr<Job> poJob = nullptr;
        std::lock_guard<std::mutex> lockJobMap(m_oMutexOnJobsMap);
        if (m_stlMapOfJobs.end() == m_stlMapOfJobs.find(c_oStructuredBuffer.GetString("JobUuid")))
        {
            poJob = std::make_shared<Job>(c_oStructuredBuffer.GetString("JobUuid"));

            // Add the job the class database
            m_stlMapOfJobs.insert(std::make_pair(c_oStructuredBuffer.GetString("JobUuid"), poJob));
        }
        else
        {
            poJob = m_stlMapOfJobs.at(c_oStructuredBuffer.GetString("JobUuid"));
        }

        // We do not link the job to the safeObject before it is fully created. Since, the incomplete
        // safe object has the list of unlinked jobs it will call the SetSafeObject for all the jobs it has
        // on the queue
        if (false == fIsSafeObjectNew)
        {
            std::cout << "Setting up safe object for the job" << std::endl;
            poJob->SetSafeObject(poSafeObject);
        }

        // Set the name of the output file that the job has to produce
        poJob->SetOutputFileName(c_oStructuredBuffer.GetString("OutFileName"));
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Submit Job Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushData
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::SetJobParameter(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::cout << "Setting a Job Parameter " << c_oStructuredBuffer.GetString("JobUuid") << std::endl;
        fflush(stdout);

        // check if the job object already exists, if it does add the parameter
        std::string strJobUuid = c_oStructuredBuffer.GetString("JobUuid");
        std::shared_ptr<Job> poJob = nullptr;
        std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
        if (m_stlMapOfJobs.end() == m_stlMapOfJobs.find(strJobUuid))
        {
            poJob = std::make_shared<Job>(strJobUuid);
            _ThrowIfNull(poJob, "Failed to create a job.", nullptr);
            // Add the job the class database
            m_stlMapOfJobs.insert(std::make_pair(strJobUuid, poJob));
        }
        else
        {
            // Get the safe object from the stored list
            poJob = m_stlMapOfJobs.at(strJobUuid);
            _ThrowIfNull(poJob, "Invalid job Uuid.", nullptr);
        }

        // If the parameter is a file and does not exist on the file system, we add it as
        // an dependency for the job and wait for it.
        bool fIsValueFilePresent = poJob->SetParameter(c_oStructuredBuffer.GetString("ParameterUuid"), c_oStructuredBuffer.GetString("ValueUuid"), c_oStructuredBuffer.GetUnsignedInt32("ValuesExpected"), c_oStructuredBuffer.GetUnsignedInt32("ValueIndex"));
        std::string strParameterValueFile = c_oStructuredBuffer.GetString("ValueUuid");
        if (false == fIsValueFilePresent)
        {
            // Register the request for now and fulfill it as soon as it is available later
            std::lock_guard<std::mutex> lockParameterValuesToJobMap(m_oMutexOnParameterValuesToJobMap);
            m_stlMapOfParameterValuesToJob.insert(std::make_pair(strParameterValueFile, poJob));
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushData
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::FileCreateCallback(
    _in const std::string & c_strFileCreatedName
    )
{
    __DebugFunction();

    std::future<void> stlUnused;
    try
    {
        std::lock_guard<std::mutex> lockParameterValuesToJobMap(m_oMutexOnParameterValuesToJobMap);
        std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
        if (m_stlSetOfPullObjects.end() != m_stlSetOfPullObjects.find(c_strFileCreatedName))
        {
            // We need to send the file back to the orchestrator, should be done asynchronously
            stlUnused = std::async(std::launch::async, &JobEngine::PullData, this, c_strFileCreatedName);
            m_stlSetOfPullObjects.erase(c_strFileCreatedName);
        }
        else if (m_stlMapOfParameterValuesToJob.end() != m_stlMapOfParameterValuesToJob.find(c_strFileCreatedName))
        {
            std::shared_ptr<Job> poJob = m_stlMapOfParameterValuesToJob.at(c_strFileCreatedName);
            stlUnused = std::async(std::launch::async, &Job::RemoveAvailableDependency, poJob, c_strFileCreatedName);
            m_stlMapOfParameterValuesToJob.erase(c_strFileCreatedName);
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function RunJob
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::SendSignal(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        // If the job ended and resulted in a failure or success the entry for that job is cleared
        // from the local map, the safeObject is not killed as it could be reused eventually.
        JobStatusSignals eSignalType = (JobStatusSignals)c_oStructuredBuffer.GetByte("SignalType");
        if ((JobStatusSignals::eJobFail == eSignalType) || (JobStatusSignals::eJobDone == eSignalType))
        {
            std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
            m_stlMapOfJobs.erase(c_oStructuredBuffer.GetString("JobUuid"));
        }
        // As soon as the file we requested for is found, we return it to the
        // orchestrator who is waiting asychronously waiting for it.
        ::PutIpcTransaction(m_poSocket, c_oStructuredBuffer);
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exceptional error in " << __func__ << std::endl;
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function RunJob
 * @brief
 * @param[in] c_oStructuredBuffer Input Request Params
 *
 ********************************************************************************************/

void __thiscall JobEngine::ResetJobEngine(void)
{
    __DebugFunction();

    // Kill all the threads created that belong to the JobEngine thread group
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    if (0 != m_FileListenerId)
    {
        poThreadManager->TerminateThread(m_FileListenerId);
    }

    // Create a kill running job signal file that will inform all the running jobs to quit
    std::ofstream output(gc_strJobsSignalFolderName + "/" + gc_strHaltAllJobsSignalFilename);

    // Call destructor on all the job objects as soon as they have killed the running jobs
    // are are stable
    std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
    m_stlMapOfJobs.clear();

    // Once all the jobs have been stopped and cleared from the Engine, it is safe to remove all
    // the SafeObjects associated with them
    std::lock_guard<std::mutex> lockSafeObjectMap(m_oMutexOnSafeObjectMap);
    m_stlMapOfSafeObjects.clear();

    // Erase the list of files that were queued for as part of pull data registration
    std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
    m_stlSetOfPullObjects.clear();

    // Erase the list of files that were queued for jobs to wait on as parameters
    std::lock_guard<std::mutex> lockParameterValuesToJobMap(m_oMutexOnParameterValuesToJobMap);
    m_stlMapOfParameterValuesToJob.clear();

    // Delete existing Directories
    std::error_code oErrorCode;
    if (std::filesystem::exists(gc_strSignalFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strSignalFolderName, oErrorCode)), "Could not create Signal Files Folder. %s", oErrorCode.message().c_str());
    }
    if (std::filesystem::exists(gc_strDataFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strDataFolderName, oErrorCode)), "Could not delete Data Files Folder. %s", oErrorCode.message().c_str());
    }
    if (std::filesystem::exists(gc_strJobsSignalFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strJobsSignalFolderName, oErrorCode)), "Could not delete Data Files Folder. %s", oErrorCode.message().c_str());
    }

    // Create a folder for data files and signal files.
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strSignalFolderName, oErrorCode)), "Could not create Signal Files Folder. %s", oErrorCode.message().c_str());
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strDataFolderName, oErrorCode)), "Could not create Data Folder. %s", oErrorCode.message().c_str());
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strJobsSignalFolderName, oErrorCode)), "Could not create Signal All Jobs Folder. %s", oErrorCode.message().c_str());

    // Create a new thread to listen to all the files being created and a callback to the JobEngine
    m_FileListenerId = poThreadManager->CreateThread("JobEngineThreads", ::FileSystemWatcherThread, (void *)nullptr);

    std::cout << "..Job Engine reset..\n";
}
