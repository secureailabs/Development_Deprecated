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
#include "FileUtils.h"
#include "ExceptionRegister.h"

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
#include <optional>

//TODO:
// 1. There is one case where the job can run even after the JobEngine reset has been called
// Last paramter set was called.
// 2. Put all the additional files in the Data folder instead of the cwd, need to delete that
// during reset, need to know how the SafeObect works.
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

    std::cout << "\n\n\nJobEngine constructor called.\n\n\n";
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

    // Start the thread to send heartbeat messages to the orchestrator
    std::thread(&JobEngine::Heartbeat, this).detach();
    std::thread(&FileSystemWatcherThread, (void *)nullptr).detach();

    // Start listening to requests and fullfill them
    this->ListenToRequests();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ~JobEngine
 * @brief Destructor for the JobEngine
 *
 ********************************************************************************************/

void __thiscall JobEngine::SetRootOfTrustNode(
    _in RootOfTrustNode * poRootOfTrust
)
{
    __DebugFunction();

    _ThrowIfNull(poRootOfTrust, "Root of Trust Node invalid", nullptr);
    m_poRootOfTrustNode = poRootOfTrust;
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

    bool fIsEngineRunning = true;
    do
    {
        // This should be a blocking call, because we have a persistant connection
        std::cout << "Waiting for request..\n";
        StructuredBuffer oNewRequest(::GetIpcTransaction(m_poSocket, true));
        m_oTimeOfLastOrchestratorMessageArrival = std::time(nullptr);

        // Get the type of request
        EngineRequest eRequestType = (EngineRequest)oNewRequest.GetByte("RequestType");
        switch (eRequestType)
        {
            case EngineRequest::eConnectVirtualMachine
            :
                this->ConnectVirtualMachine(oNewRequest);
                break;
            case EngineRequest::ePushSafeObject
            :
                std::thread(&JobEngine::PushSafeObject, this, oNewRequest).detach();
                break;
            case EngineRequest::ePushdata
            :
                std::thread(&JobEngine::PushData, this, oNewRequest).detach();
                break;
            case EngineRequest::ePullData
            :
                std::thread(&JobEngine::PullData, this, oNewRequest.GetString("Filename")).detach();
                break;
            case EngineRequest::eSubmitJob
            :
                std::thread(&JobEngine::SubmitJob, this, oNewRequest).detach();
                break;
            case EngineRequest::eSetParameters
            :
                std::thread(&JobEngine::SetJobParameter, this, oNewRequest).detach();
                break;
            case EngineRequest::eHaltAllJobs
            :
                // This job does not need to be async, it will block for the cleanup to be performed
                // before new requests can be accpeted and fulfilled
                this->ResetJobEngine();
                break;
            case EngineRequest::eHeartBeatPong
            :
                // We do nothing for this as the time is already recorded above
                break;
            case EngineRequest::eVmShutdown
            :
                {
                    fIsEngineRunning = false;
                    // Send a signal of VM Shutdown to the orceshtrator
                    StructuredBuffer oStructuredBufferShutdown;
                    oStructuredBufferShutdown.PutByte("SignalType", (Byte)JobStatusSignals::eVmShutdown);
                    this->SendMessageToOrchestrator(oStructuredBufferShutdown);
                }
                break;
            default
            :
                break;
        }
    } while (true == fIsEngineRunning);
}


/********************************************************************************************
 *
 * @class JobEngine
 * @function ConnectVirtualMachine
 * @brief Register the Eosb of the Orchestrator user for login
 * @param[in] c_oStructuredBuffer
 *
 ********************************************************************************************/

void __thiscall JobEngine::ConnectVirtualMachine(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    if (false == m_fIsInitialized)
    {
        std::cout << "The Virtual Machine Uuid is " << m_GuidVmId.ToString(eHyphensAndCurlyBraces);

        // Get the Set of available Guids from the DataConnector
        m_oDataConnectorAvailableGuids = ::DataConnectorGetFetchableUuid();
        StructuredBuffer oStructuredBufferOfGuids = m_oDataConnectorAvailableGuids.GetStructuredBuffer("Tables");
        auto oListOfGuids = oStructuredBufferOfGuids.GetNamesOfElements();
        for (auto strName : oListOfGuids)
        {
            m_stlMapOfDataConnectorGuidsToName.insert(std::make_pair(oStructuredBufferOfGuids.GetString(strName.c_str()), strName));
        }
        m_fIsInitialized = true;
    }
    else
    {
        if (true == m_fIsConnected)
        {
            // If a connection with the orchestrator already exists, we will reset the JobEngine and send a message to
            // the thread that was handling the connection, otherwise treat it as if
            StructuredBuffer oKillThread;
            oKillThread.PutBoolean("KeepAlive", false);
            this->SendMessageToOrchestrator(oKillThread);
        }

        this->ResetJobEngine();
    }

    StructuredBuffer oStructuredBufferLoginResponse;
    oStructuredBufferLoginResponse.PutString("VirtualMachineUuid", m_GuidVmId.ToString(eHyphensAndCurlyBraces));
    oStructuredBufferLoginResponse.PutBoolean("Success", true);
    oStructuredBufferLoginResponse.PutStructuredBuffer("Dataset", m_oDataConnectorAvailableGuids);
    this->SendMessageToOrchestrator(oStructuredBufferLoginResponse);

    // Mark the JobEngine as connected
    m_fIsConnected = true;

    // TODO: check if this audit log needs to be generated at every connect
    StructuredBuffer oEventData;
    oEventData.PutBoolean("Success", true);
    oEventData.PutString("Username", c_oStructuredBuffer.GetString("Username"));
    oEventData.PutString("OrchestratorVersion", "2.0.0");
    oEventData.PutString("Eosb", c_oStructuredBuffer.GetString("Eosb"));

    m_poRootOfTrustNode->RecordAuditEvent("CONNECT_SUCCESS", 0x1111, 0x04, oEventData);
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushSafeObject
 * @brief Push the details of the SafeObject needed by the job to run
 * @param[in] c_oStructuredBuffer StrucutredBuffer containing
 *
 ********************************************************************************************/

void __thiscall JobEngine::PushSafeObject(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        bool fIsSafeObjectNew = true;
        std::cout << "Submitting a safe object" << std::endl;
        std::string strSafeObjectUuid = c_oStructuredBuffer.GetString("SafeObjectUuid");
        std::shared_ptr<SafeObject> poSafeObject = nullptr;

        // Defining the scope of the lock only for where it is needed. Free it up as
        // soon as it's purpose is finished. Also, there is chance that a job could run
        // after this which maybe blocking and and must not acquire a mutex potentially
        // leading to a deadlock.
        {
            std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
            // It may happen that a reference was already made to the safeObject by it's
            // Uuid possibly in SubmitJob, if that is the case, an empty SafeObject object
            // already exists with no additional data except for it's Uuid, so the setup
            // function would just do that.
            if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
            {
                // Create a safe object and add it to the map of Guid and Object
                poSafeObject = std::make_shared<SafeObject>(strSafeObjectUuid);
                poSafeObject->Setup(c_oStructuredBuffer);

                // Push the safe object to the list of safeObjects in the engine
                m_stlMapOfSafeObjects.insert(std::make_pair(strSafeObjectUuid, poSafeObject));
            }
            else
            {
                poSafeObject = m_stlMapOfSafeObjects.at(strSafeObjectUuid);
                poSafeObject->Setup(c_oStructuredBuffer);

                fIsSafeObjectNew = false;
            }
        }
        // Once the safe object setup is complete, notify all the jobs waiting on it
        if (false == fIsSafeObjectNew)
        {
            for (auto strJobUuid : poSafeObject->GetQueuedJobsUuid())
            {
                // A check if the JobUuid is present is not performed here
                // because it should be present. If it is not present, there is
                // something else wrong somewhere else
                auto poJob = m_stlMapOfJobs.at(strJobUuid);
                // TODO: if multiple jobs are waiting on the safeObject, it is possible that
                // this call will be blocked as the job may start to run. Fix this, to run multiple
                // jobs with the same SafeObject run in parallel.
                poJob->SetSafeObject(poSafeObject);
            }
        }
    }

    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PushData
 * @brief Create a file on the filesystem with the DataId and Data provided
 * @param[in] c_oStructuredBuffer Strucutred Buffer containing data and an ID attched to it
 *
 ********************************************************************************************/

void __thiscall JobEngine::PushData(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::cout << "Pushing " << c_oStructuredBuffer.GetBuffer("Data").size() << " bytes to file " << c_oStructuredBuffer.GetString("DataId") << std::endl;
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
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function PullData
 * @brief Register a request from the Orchestrator and send the requested
 *      file as soon as is available
 * @param[in] c_strFileNametoSend File name requested by the Orchestrator
 *
 ********************************************************************************************/

void __thiscall JobEngine::PullData(
    _in const std::string & c_strFileNametoSend
)
{
    __DebugFunction();

    try
    {
        std::cout << "Pull Data request " << c_strFileNametoSend << std::endl;
        fflush(stdout);

        std::string strSignalFile = gc_strSignalFolderName + "/" + c_strFileNametoSend;

        // Check if the signal file already exists, if the file exists already exists
        // push it to the orchestrator otherwise just register the request and
        // wait for the file to be created in future.

        {
            std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
            m_stlSetOfPullObjects.insert(c_strFileNametoSend);
        }

        if (true == std::filesystem::exists(strSignalFile.c_str()))
        {
            // As soon as the file we requested for is found, we return it to the
            // orchestrator who is waiting asychronously waiting for it.
            StructuredBuffer oResponse;
            oResponse.PutByte("SignalType", (Byte)JobStatusSignals::ePostValue);
            oResponse.PutString("ValueName", c_strFileNametoSend);
            oResponse.PutBuffer("FileData", ::ReadFileAsByteBuffer(c_strFileNametoSend));

            // The Pull data response to the orchestrator is sent as a signal with data
            this->SendMessageToOrchestrator(oResponse);

            {
                std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
                // Remove the file from the set
                m_stlSetOfPullObjects.erase(c_strFileNametoSend);
            }

            // We delete the signal file and the data file after we have consumed it.
            ::remove(strSignalFile.c_str());
            //::remove(c_strFileNametoSend.c_str());
        }
    }

    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function SubmitJob
 * @brief Submit a Job and the corresponding SafeObjecctId to run the Job.
 * @param[in] c_oStructuredBuffer StructuredBuffer consting the JobUuid and SafeObjectUuid
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

        // We find the safeObject if it exists or create an empty object if it does not
        std::string strSafeObjectUuid = c_oStructuredBuffer.GetString("SafeObjectUuid");
        bool fIsSafeObjectNew = true;

        // If the safeObject is already present in the JobEngine database, use that one
        // but if the safeObject was not pushed already, a new empty safeObject is created
        std::shared_ptr<SafeObject> poSafeObject = nullptr;

        // There is a forced scope defined for this lock on the mutex to avoid taking up
        // two locks at the same time which can cause a possible deadlock
        {
            std::lock_guard<std::mutex> lock(m_oMutexOnSafeObjectMap);
            if (m_stlMapOfSafeObjects.end() == m_stlMapOfSafeObjects.find(strSafeObjectUuid))
            {
                // Create and Add the SafeObject to the class database
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
        }

        // check if the job object already exists, if it does just add the SafeObject to it
        std::shared_ptr<Job> poJob = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
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
        }

        // Do not link the job to the safeObject before it is fully created. Since, the incomplete
        // safe object has the list of unlinked jobs it will call the SetSafeObject for all the jobs it has
        // on the queue
        if (false == fIsSafeObjectNew)
        {
            poJob->SetSafeObject(poSafeObject);
        }
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Submit Job Exception: " << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function SetJobParameter
 * @brief Inform the job about the ValueId where it can find the value for a designated
 *      parameter
 * @param[in] c_oStructuredBuffer StructuredBuffer consting the JobUuid, ParamterUuid and a
 *          corresponding ValueUuid along with index and number of values for that paramter
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

        // check if the job object already exists, if it does add the parameter
        std::string strJobUuid = c_oStructuredBuffer.GetString("JobUuid");
        std::shared_ptr<Job> poJob = nullptr;

        // To avoid locking two mutex simultaneously in the same thread, a scope is forced
        // so that the mutex is unlocked at the end of the scope and the chances of deadlock
        // are mitigated
        {
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
        }

        // Check if the requested data is a dataset from the DataConnector.
        if (m_stlMapOfDataConnectorGuidsToName.end() != m_stlMapOfDataConnectorGuidsToName.find(c_oStructuredBuffer.GetString("ValueUuid")))
        {
            // This means that the valueId is to be fetched from the DataConnector
            // and written to the file system
            ::DataConnectorGetTable(c_oStructuredBuffer.GetString("ValueUuid"), m_stlMapOfDataConnectorGuidsToName.at(c_oStructuredBuffer.GetString("ValueUuid")));

            // Assuming the file was successfully written to the file system, create a signal file for the same
            std::ofstream output(gc_strSignalFolderName + "/" + c_oStructuredBuffer.GetString("ValueUuid"));
        }

        // If the parameter is a file and does not exist on the file system, we add it as
        // an dependency for the job and wait for it.
        {
            // Register the request for now and fulfill it as soon as it is available later
            std::string strParameterValueFile = c_oStructuredBuffer.GetString("ValueUuid");
            std::lock_guard<std::mutex> lock(m_oMutexOnParameterValuesToJobMap);
            bool fIsValueFilePresent = poJob->SetParameter(c_oStructuredBuffer.GetString("ParameterUuid"), strParameterValueFile, c_oStructuredBuffer.GetUnsignedInt32("ValuesExpected"), c_oStructuredBuffer.GetUnsignedInt32("ValueIndex"));
            if (false == fIsValueFilePresent)
            {
                m_stlMapOfParameterValuesToJob.insert(std::make_pair(strParameterValueFile, poJob));
            }
        }
    }
    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function FileCreateCallback
 * @brief A callback function for every new file signal that is created in the Signals folder
 * @param[in] c_strFileCreatedName Name of the signal file that is created
 *
 ********************************************************************************************/

void __thiscall JobEngine::FileCreateCallback(
    _in const std::string & c_strFileCreatedName
    )
{
    __DebugFunction();

    bool fIsParameterValue = false;
    bool fPullData = false;

    try
    {
        std::shared_ptr<Job> poJob = nullptr;

        // The scope is defined for the lock to free up as soon as it is not needed anymore.
        // Taking up two locks at the same time is risky and should be avoided to prevent deadlocks.
        // The mutex needed to be locked before the start of the if condition, hence a forced scope
        {
            std::lock_guard<std::mutex> lockParameterValuesToJobMap(m_oMutexOnParameterValuesToJobMap);
            if(m_stlMapOfParameterValuesToJob.end() != m_stlMapOfParameterValuesToJob.find(c_strFileCreatedName))
            {
                poJob = m_stlMapOfParameterValuesToJob.at(c_strFileCreatedName);
                m_stlMapOfParameterValuesToJob.erase(c_strFileCreatedName);
                fIsParameterValue = true;
            }
        }

        if (true == fIsParameterValue)
        {
            std::thread(&Job::RemoveAvailableDependency, poJob, c_strFileCreatedName).detach();
        }
        else
        {
            // Since the new file could either be a pull object or a valueId needed by a job,
            // there is an additional check to ensure that it was not a job parameter value
            {
                std::lock_guard<std::mutex> lockSetOfPullObjects(m_oMutexOnSetOfPullObjects);
                if (m_stlSetOfPullObjects.end() != m_stlSetOfPullObjects.find(c_strFileCreatedName))
                {
                    fPullData = true;
                }
            }

            if (true == fPullData)
            {
                // Send the file back to the orchestrator asynchronously
                // The mutex locked up here will not block the next PullData call because it is async
                // and the mutex will be unlocked just after the call.
                std::thread(&JobEngine::PullData, this, c_strFileCreatedName).detach();
            }
        }
    }

    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function SendMessageToOrchestrator
 * @brief Send StructuredBuffer encoded messages to the Orchestrator
 * @param[in] c_oStructuredBuffer StrucutredBuffer message to send
 * @note This is the only function that must be used to send messages to the remote
 *      Orchestrator. It ensure that the IpcSocket write is locked before being used and
 *      does not end up in a race condition when being used by multiple threads.
 *
 ********************************************************************************************/

void __thiscall JobEngine::SendMessageToOrchestrator(
    _in const StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        // The message to send could be a signal or a simple packet like the response to
        // the connect call made. This is the only function on the JobEngine which is supposed
        // to send Ipc Messages to the Communication Module. It takes a lock so that the IPC does not
        // run into race conditions.
        if (c_oStructuredBuffer.IsElementPresent("SignalType", BYTE_VALUE_TYPE))
        {
            // If the job ended and resulted in a failure or success the entry for that job is cleared
            // from the local map, the safeObject is not killed as it could be reused eventually.
            JobStatusSignals eSignalType = (JobStatusSignals)c_oStructuredBuffer.GetByte("SignalType");
            if ((JobStatusSignals::eJobFail == eSignalType) || (JobStatusSignals::eJobDone == eSignalType))
            {
                std::lock_guard<std::mutex> lock(m_oMutexOnJobsMap);
                std::cout << "m_stlMapOfJobs.size " << m_stlMapOfJobs.size() << std::endl;
                m_stlMapOfJobs.erase(c_oStructuredBuffer.GetString("JobUuid"));
            }
            else if (JobStatusSignals::ePostValue == eSignalType)
            {
                std::cout << "Writing " << c_oStructuredBuffer.GetString("ValueName") << " of size " << c_oStructuredBuffer.GetBuffer("FileData").size() << std::endl;
                fflush(stdout);
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_oMutexOnIpcSocket);
            ::PutIpcTransaction(m_poSocket, c_oStructuredBuffer);
        }
        std::string strListOfNames;
        for (auto strElement : c_oStructuredBuffer.GetNamesOfElements())
        {
            strListOfNames += " " + strElement;
        }
        std::cout << "Sent to orchestrator " << strListOfNames << std::endl;
        fflush(stdout);
    }

    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ResetJobEngine
 * @brief Reset the Job Engine to it's original fresh state with no jobs and safe objects
 *      in the queue
 *
 ********************************************************************************************/

void __thiscall JobEngine::ResetJobEngine(void)
{
    __DebugFunction();

    // Create a kill running job signal file that will inform all the running jobs to quit
    std::ofstream output(gc_strJobsSignalFolderName + "/" + gc_strHaltAllJobsSignalFilename);

    // TODO: Prawal. Stop all running threads including the FileListener

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

    std::cout << "..Job Engine reset..\n";
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function Heartbeat
 * @brief Send a heartbeat to the Orchestrator if no message is received for a certain time
 *
 ********************************************************************************************/
void __thiscall JobEngine::Heartbeat(void)
{
    __DebugFunction();

    try
    {
        while (true)
        {
            // Sleep for 30 seconds
            std::this_thread::sleep_for(std::chrono::seconds(30));

            if (true == m_fIsConnected)
            {
                if(60 < std::difftime(std::time(nullptr), m_oTimeOfLastOrchestratorMessageArrival))
                {
                    StructuredBuffer oStructuredBuffer;
                    oStructuredBuffer.PutBoolean("Timeout", true);
                    this->SendMessageToOrchestrator(oStructuredBuffer);

                    // Mark the JobEngine as not connected
                    m_fIsConnected = false;
                }
                else if(30 < std::difftime(std::time(nullptr), m_oTimeOfLastOrchestratorMessageArrival))
                {
                    // If there was no message from the Orchestrator for the past 30 seconds, send a eHeartBeatPing
                    // Send the message to the Orchestrator
                    StructuredBuffer oStructuredBuffer;
                    oStructuredBuffer.PutByte("SignalType", (Byte)JobStatusSignals::eHeartBeatPing);
                    this->SendMessageToOrchestrator(oStructuredBuffer);
                }
                else
                {
                    // All good. We are alive
                }
            }
        }
    }

    catch(BaseException & oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}
