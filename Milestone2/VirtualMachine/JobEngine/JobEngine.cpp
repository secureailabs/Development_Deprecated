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

// TODO:
// 3. Add signals from jobs running.

#define cout cout << std::this_thread::get_id() << " "

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Creates a file with the content from the buffer
 *
 ********************************************************************************************/

void * __stdcall FileSystemWatcherThread(void * poThreadParameter)
{
    __DebugFunction();

    try
    {
        std::cout << "FileSystemWatcherThread" << std::endl;

        int nINotifyFd = ::inotify_init();
        _ThrowBaseExceptionIf((0 >= nINotifyFd), "Unable to create a inotify object", nullptr);

        // Add the directory we want to watch
        int nDirectoryToWatchFd = ::inotify_add_watch(nINotifyFd, gc_strSignalFolderName.c_str(), IN_CREATE);
        _ThrowBaseExceptionIf((-1 == nDirectoryToWatchFd), "Could not watch : \"./\"\n", nullptr);

        bool fKeepRunning = true;

        // Allocate a buffer for
        std::vector<Byte> stlNotifyEvent(sizeof(struct inotify_event) + NAME_MAX + 1);

        // Get the Job Engine Object which has the callback we'd need to call on
        // every new file creation
        JobEngine & oJobEngine = JobEngine::Get();

        while(true == fKeepRunning)
        {
            // This is a blocking call is only invoked when a file system related event is
            // is reported in the specified directory
            int nLengthOfData = ::read(nINotifyFd, stlNotifyEvent.data(), stlNotifyEvent.size());

            struct inotify_event * poInotifyEvent = (struct inotify_event *)stlNotifyEvent.data();
            if(poInotifyEvent->len && (poInotifyEvent->mask & IN_CREATE))
            {
                // For everyfile created we call a JobEngine callback function which should
                // find the most efficient way to handle such a file.
                std::cout << "FileCreateCallback for " << poInotifyEvent->name << std::endl;

                oJobEngine.FileCreateCallback(poInotifyEvent->name);

                if (gc_strHaltAllJobsSignalFilename == poInotifyEvent->name)
                {
                    fKeepRunning = false;
                }
            }
        }

        // Cleanup the event listener
        ::inotify_rm_watch( nINotifyFd, nDirectoryToWatchFd);
        ::close(nINotifyFd);
    }
    catch(const BaseException & oException)
    {
        std::cout << oException.GetExceptionMessage() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return nullptr;
}

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Creates a file with the content from the buffer
 *
 ********************************************************************************************/

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
        _ThrowBaseException("Invalid File Path", nullptr);
    }
    return stlFileData;
}

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

void JobEngine::StartServer(void)
{
    __DebugFunction();

    // Start listening to requests and fullfill them
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    // StatusMonitor oStatusMonitor("static void __cdecl InitDataConnector()");

    std::error_code oErrorCode;
    // Delete existing Directories
    if (std::filesystem::exists(gc_strSignalFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strSignalFolderName, oErrorCode)), "Could not create Signal Files Folder. %s", oErrorCode.message().c_str());
    }
    if (std::filesystem::exists(gc_strDataFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strDataFolderName, oErrorCode)), "Could not delete Data Files Folder", nullptr);
    }
    if (std::filesystem::exists(gc_strJobsSignalFolderName))
    {
        _ThrowBaseExceptionIf((false == std::filesystem::remove_all(gc_strJobsSignalFolderName, oErrorCode)), "Could not delete Data Files Folder", nullptr);
    }

    // Create a folder for data files and signal files.
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strSignalFolderName, oErrorCode)), "Could not create Signal Files Folder", nullptr);
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strDataFolderName, oErrorCode)), "Could not create Data Folder", nullptr);
    _ThrowBaseExceptionIf((false == std::filesystem::create_directory(gc_strJobsSignalFolderName, oErrorCode)), "Could not create Signal All Jobs Folder", nullptr);

    // Create a thread to listen to all the files being created and a callback to the JobEngine
    poThreadManager->CreateThread(nullptr, ::FileSystemWatcherThread, (void *)nullptr);

    std::cout << "Connecting to socket server on orchestrator..." << std::endl;

    m_poSocket = ::ConnectToUnixDomainSocket("{164085b7-ef20-4257-bc14-e1e08c908aaa}");
    if (nullptr != m_poSocket)
    {
        this->ListenToRequests(m_poSocket);
        m_poSocket->Release();
    }
    // while (false == oStatusMonitor.IsTerminating());
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ListenToRequests
 * @brief Destructor for the JobEngine
 *
 ********************************************************************************************/

void __thiscall JobEngine::ListenToRequests(
    _in Socket * poSocket
)
{
    __DebugFunction();

    bool fConnectionIsOpen = true;
    std::future<void> stlReturn;
    do
    {
        // This should be a blocking call, because we have a persistant connection
        std::cout << "Waiting for request..\n";
        auto stlSerializedBuffer = ::GetIpcTransaction(poSocket);
        StructuredBuffer oNewRequest(stlSerializedBuffer);

        // Get the type of request
        EngineRequest eRequestType = (EngineRequest)oNewRequest.GetByte("RequestType");

        if (EngineRequest::ePushSafeObject == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::PushSafeObject, this, oNewRequest);
        }
        else if (EngineRequest::ePushdata == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::PushData, this, oNewRequest);
        }
        else if (EngineRequest::ePullData == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::PullData, this, oNewRequest.GetString("Filename"));
        }
        else if (EngineRequest::eSubmitJob == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::SubmitJob, this, oNewRequest);
        }
        else if (EngineRequest::eSetParameters == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::SetJobParameter, this, oNewRequest);
        }
        else if (EngineRequest::eHaltAllJobs == eRequestType)
        {
            stlReturn = std::async(std::launch::async, &JobEngine::HaltAllJobs, this, oNewRequest);
        }
        else if (EngineRequest::eShutdown == eRequestType)
        {
            fConnectionIsOpen = false;
            // Send a signal of VM Shutdown to the orceshtrator
            StructuredBuffer oStructuredBufferShutdown;
            oStructuredBufferShutdown.PutByte("SignalType", (Byte)JobStatusSignals::eVmShutdown);
            this->SendSignal(oStructuredBufferShutdown);
        }
        else
        {

        }
    } while (true == fConnectionIsOpen);
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

        SafeObject * poSafeObject = nullptr;
        std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
        if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
        {
            // Create a safe object and add it to the map of Guid and Object
            std::cout << "Safe Object not found. Creating new " << strSafeObjectUuid << std::endl;
            poSafeObject = new SafeObject(strSafeObjectUuid);
            poSafeObject->Setup(c_oStructuredBuffer);
            // Push the safe object to the list of safeObjects in the engine
            m_stlMapOfSafeObjects.insert(std::make_pair(strSafeObjectUuid, poSafeObject));
        }
        else
        {
            poSafeObject = m_stlMapOfSafeObjects.at(strSafeObjectUuid);
            poSafeObject->Setup(c_oStructuredBuffer);

            // Once the safe object setup is complete we notify all the jobs waiting on it
            for (auto stlJobUuid : poSafeObject->GetQueuedJobsUuid())
            {
                auto poJob = m_stlMapOfJobs.at(stlJobUuid);
                poJob->SetSafeObject(poSafeObject);
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

            // TODO: write the file to the socket
            this->SendSignal(oResponse);

            // We delete the signal file and the data file after we have consumed it.
            ::remove(strSignalFile.c_str());
            ::remove(c_strFileNametoSend.c_str());
        }
        else
        {
            // Register the request for now and fulfill it as soon as it is available later
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
        SafeObject * poSafeObject = nullptr;
        std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
        if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
        {
            // Create and Add the SafeObject to the class database
            std::cout << "Safe Object not found. " << strSafeObjectUuid << std::endl;
            poSafeObject = new SafeObject(strSafeObjectUuid);
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
        Job * poJob = nullptr;
        if (m_stlMapOfJobs.end() == m_stlMapOfJobs.find(c_oStructuredBuffer.GetString("JobUuid")))
        {
            poJob = new Job(c_oStructuredBuffer.GetString("JobUuid"));

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
        Job * poJob = nullptr;
        if (m_stlMapOfJobs.end() == m_stlMapOfJobs.find(strJobUuid))
        {
            poJob = new Job(strJobUuid);
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

    try
    {
        if (gc_strHaltAllJobsSignalFilename == c_strFileCreatedName)
        {

        }
        else if (m_stlSetOfPullObjects.end() != m_stlSetOfPullObjects.find(c_strFileCreatedName))
        {
            // We need to send the file back to the orchestrator, should be done asynchronously
            auto stlUnused = std::async(std::launch::async, &JobEngine::PullData, this, c_strFileCreatedName);
            m_stlSetOfPullObjects.erase(c_strFileCreatedName);
        }
        else if (m_stlMapOfParameterValuesToJob.end() != m_stlMapOfParameterValuesToJob.find(c_strFileCreatedName))
        {
            Job * poJob = m_stlMapOfParameterValuesToJob.at(c_strFileCreatedName);
            poJob->RemoveAvailableDependency(c_strFileCreatedName);
            m_stlMapOfParameterValuesToJob.erase(c_strFileCreatedName);
        }
        else
        {
            std::cout << "Useless file " << c_strFileCreatedName << std::endl;
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

void __thiscall JobEngine::HaltAllJobs(
    _in const StructuredBuffer & c_strJobId
)
{
    __DebugFunction();

    std::cout << "Halt all jobs" << std::endl;

    try
    {
        // Ask the listener to stop listening for new Requests for now

        // Create a kill running job signal file that will inform all the running jobs to quit
        std::ofstream output(gc_strJobsSignalFolderName + "/" + gc_strHaltAllJobsSignalFilename);

        // Call destructor on all the job objects as soon as they have killed the running jobs
        // are are stable
        // while(0 < m_stlMapOfJobs.size())
        // {
        //     for (auto oJobMapElement : m_stlMapOfJobs)
        //     {
        //         JobState eJobState = oJobMapElement.second->GetJobState();
        //         if (JobState::eRunning != eJobState)
        //         {
        //             oJobMapElement.second->Release();
        //             m_stlMapOfJobs.erase(oJobMapElement.first);
        //         }
        //     }
        // }

        // Once all the jobs have been stopped and cleared from the Engine, it is safe to remove all
        // the SafeObjects associated with them
        // for (auto oSafeObjectMapElement : m_stlMapOfSafeObjects)
        // {
        //     oSafeObjectMapElement.second->Release();
        //     m_stlMapOfJobs.erase(oSafeObjectMapElement.first);
        // }


        // Ask the listener to resume listening for new Requests for now


        // Delete the signal file, before resume

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
