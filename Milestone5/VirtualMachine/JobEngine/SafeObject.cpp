/*********************************************************************************************
 *
 * @file SafeObject.cpp
 * @author Prawal Gangwar
 * @date 4 June 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "SafeObject.h"
#include "Exceptions.h"
#include "JobEngine.h"
#include "FileUtils.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <filesystem>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/inotify.h>
#include <limits.h>
#include <sys/epoll.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

/********************************************************************************************
 *
 * @class SafeObject
 * @function SafeObject
 * @brief Constructor to create a SafeObject object
 *
 ********************************************************************************************/

SafeObject::SafeObject(
    _in const std::string c_strSafeObjectUuid
    )
{
    __DebugFunction();

    // Get the safe object UUID
    m_strSafeObjectIdentifier = c_strSafeObjectUuid;
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

    std::cout << "SafeObject destructed" << std::endl;

    // Destructor will just delete the executable file containing the safeObject
    std::filesystem::remove(m_strSafeObjectIdentifier.c_str());
}

/********************************************************************************************
 *
 * @class SafeObject
 * @function SafeObject
 * @brief Constructor to create a SafeObject object
 *
 ********************************************************************************************/

void __thiscall SafeObject::Setup(
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();

    // Get the safe object UUID
    m_strSafeObjectIdentifier = c_oStructuredBuffer.GetString("SafeObjectUuid");

    // Write the executable file to file system to run
    ::WriteBytesAsFile(m_strSafeObjectIdentifier, c_oStructuredBuffer.GetBuffer("Payload"));

    // Make the file executable
    ::chmod(m_strSafeObjectIdentifier.c_str(), S_IRWXU);

    // Get List of parameters
    StructuredBuffer oStructuredBufferParameter = c_oStructuredBuffer.GetStructuredBuffer("ParameterList");
    std::vector<std::string> stlListOfParameters = oStructuredBufferParameter.GetNamesOfElements();
    for (std::string strParameterName : stlListOfParameters)
    {
        m_stlListOfParameters.push_back(strParameterName);
    }

    // Get the Output Parameters
    m_oStructuredBufferOutputParameter = c_oStructuredBuffer.GetStructuredBuffer("OutputParameters");

    // Add the command that is to be executed.
    m_strCommandToExecute = m_strSafeObjectIdentifier;
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

    int nProcessExitStatus = -1;

    try
    {
        // Get the JobEngine singleton object and send a job start signal to the remote orchestrator
        JobEngine & oJobEngine = JobEngine::Get();

        StructuredBuffer oStructruedBufferSignal;
        oStructruedBufferSignal.PutByte("SignalType", (Byte)JobStatusSignals::eJobStart);
        oStructruedBufferSignal.PutString("JobUuid", c_strJobUuid);
        oJobEngine.SendMessageToOrchestrator(oStructruedBufferSignal);

        pid_t nProcessIdentifier = ::fork();
        _ThrowBaseExceptionIf((-1 == nProcessIdentifier), "Fork has failed with errno = %d", errno);

        if (0 == nProcessIdentifier)
        {
            // This is the child process which will run the actual job and write the output to the screen
            // and create a file with the output
            ::execl("/usr/bin/python3", "/usr/bin/python3", m_strCommandToExecute.c_str(), c_strJobUuid.c_str(), "abcd", nullptr);
            ::exit(0);
        }
        else
        {
            // The parent process will wait for either the child to exit after completion or
            // wait for the halt all jobs signal from the Job Engine to kill the child process

            // Use the process file descriptor to wait for the child process to exit using epoll
            int nChildProcessFileDescriptor = ::syscall(SYS_pidfd_open, nProcessIdentifier, 0);
            _ThrowBaseExceptionIf((-1 == nChildProcessFileDescriptor), "Failed to get the file descriptor for the child process Errorno: %s.",  ::strerror(errno));

            // Also create a file descriptor to wait for the signal file that tells to halt all the jobs
            // running on the job engine. As soon as we receive that signal we kill the child process and cleanup.
            int nINotifyFd = ::inotify_init();
            _ThrowBaseExceptionIf((0 >= nINotifyFd), "Unable to create a inotify object. Errorno: %d", errno);

            // Add the directory we want to watch
            int nDirectoryToWatchFd = ::inotify_add_watch(nINotifyFd, gc_strJobsSignalFolderName.c_str(), IN_CREATE);
            _ThrowBaseExceptionIf((-1 == nDirectoryToWatchFd), "Cannot not watch : \"%s\"\n", gc_strJobsSignalFolderName.c_str());

            // Having got two file descriptors we wait for any one of them to wake up the process,
            // either job exit for the presence of the signal file to halt the running job
            struct epoll_event oEpollEvents[2];
            int nPollingFileDescriptor = ::epoll_create1(0);
            _ThrowBaseExceptionIf((-1 == nPollingFileDescriptor), "epoll_create1() failed with errno = %d", errno);

            // Add the Child Process wait event
            oEpollEvents[0].events = EPOLLIN;
            oEpollEvents[0].data.fd = nChildProcessFileDescriptor;
            int nReturnValue = ::epoll_ctl(nPollingFileDescriptor, EPOLL_CTL_ADD, nChildProcessFileDescriptor, &oEpollEvents[0]);
            _ThrowBaseExceptionIf((0 != nReturnValue), "epoll_ctl() failed with errno = %d", errno);

            // Add the Halt Process wait event
            oEpollEvents[1].events = EPOLLIN;
            oEpollEvents[1].data.fd = nINotifyFd;
            nReturnValue = ::epoll_ctl(nPollingFileDescriptor, EPOLL_CTL_ADD, nINotifyFd, &oEpollEvents[1]);
            _ThrowBaseExceptionIf((0 != nReturnValue), "epoll_ctl() failed with errno = %d", errno);

            // Before going to wait a check for the kill signal file is made, this is to ensure that
            // the kill signal was not created before the inotify_add_watch call. One could state that
            // the check could be performed just before the inotify_add_watch call but that would miss
            // out the time between that check and the creation of watch. Hence checking after watch
            // would cover all the time.
            bool fKillSignalPreExist = false;
            int nNumberOfEvents = 0;
            struct epoll_event asPollingEvents[1];
            if (false == std::filesystem::exists(gc_strJobsSignalFolderName + "/" + gc_strHaltAllJobsSignalFilename))
            {
                // This is a blocking call to infinitely wait for either event
                std::cout << "Waiting for epoll event to occur.. " << std::endl;
                nNumberOfEvents = ::epoll_wait(nPollingFileDescriptor, asPollingEvents, 1, -1);
            }
            else
            {
                fKillSignalPreExist = true;
            }

            if ((0 < nNumberOfEvents) || (true == fKillSignalPreExist))
            {
                // This is when the child process exits itself
                if (nChildProcessFileDescriptor == asPollingEvents->data.fd)
                {
                    std::cout << "Child died naturally. X-| \n";
                    ::waitpid(nProcessIdentifier, &nProcessExitStatus, 0);
                    // There is a chance that the job failed gracefully and in that case the
                    // output file was not written, that is a failure case and we send a jobfail
                    // signal to the remote orcehstrator
                    // TODO: check for all the output files present along with ProcessExitStatus
                    if (0 == nProcessExitStatus)
                    {
                        // This is already done in the SafeObject code
                        // std::ofstream output(gc_strSignalFolderName + "/" + strOutputFileName);

                        // Send a job success signal to the orchestrator
                        oStructruedBufferSignal.PutByte("SignalType", (Byte)JobStatusSignals::eJobDone);
                        oStructruedBufferSignal.PutString("JobUuid", c_strJobUuid);
                        oJobEngine.SendMessageToOrchestrator(oStructruedBufferSignal);
                    }
                    else if (123 == WEXITSTATUS(nProcessExitStatus))
                    {
                        // Send a job fail signal to the orchestrator
                        oStructruedBufferSignal.PutByte("SignalType", (Byte)JobStatusSignals::ePrivacyViolation);
                        oStructruedBufferSignal.PutString("JobUuid", c_strJobUuid);
                        oJobEngine.SendMessageToOrchestrator(oStructruedBufferSignal);

                        // We can potenitally add an audit log here.
                    }
                    else
                    {
                        // Send a job fail signal to the orchestrator
                        oStructruedBufferSignal.PutByte("SignalType", (Byte)JobStatusSignals::eJobFail);
                        oStructruedBufferSignal.PutString("JobUuid", c_strJobUuid);
                        oJobEngine.SendMessageToOrchestrator(oStructruedBufferSignal);
                    }
                }
                // This is when a kill signal is recevied from the JobEngine
                else if ((nINotifyFd == asPollingEvents->data.fd) || (true == fKillSignalPreExist))
                {
                    std::cout << "Child killed by orchestrator. X-< \n";
                    nProcessExitStatus = ::kill(nProcessIdentifier, SIGKILL);
                }
                else
                {
                    std::cout << "Potential Bug: Unknown event. Should have never hit this." << std::endl;
                }
            }
            else
            {
                _ThrowBaseException("Unexpected error while waiting for process to exit", nullptr);
            }

            // Cleanup the event listener
            ::inotify_rm_watch( nINotifyFd, nDirectoryToWatchFd);
            ::close(nINotifyFd);
            ::close(nChildProcessFileDescriptor);
            ::epoll_ctl(nPollingFileDescriptor, EPOLL_CTL_DEL, nINotifyFd, nullptr);
            ::epoll_ctl(nPollingFileDescriptor, EPOLL_CTL_DEL, nChildProcessFileDescriptor, nullptr);
            ::close(nPollingFileDescriptor);

            // Delete the inputs file if it exists
            if (true == std::filesystem::exists(c_strJobUuid + ".inputs"))
            {
                std::filesystem::remove(c_strJobUuid + ".inputs");
            }

            // Delete the output file if it exists
            if (true == std::filesystem::exists(c_strJobUuid + ".output"))
            {
                std::filesystem::remove(c_strJobUuid + ".output");
            }
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

    std::cout << "Process exit with status " << nProcessExitStatus << std::endl;
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

/********************************************************************************************
 *
 * @class SafeObject
 * @function GetSafeObjectIdentifier
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

void __thiscall SafeObject::AddJobUuidToQueue(
        _in const std::string & c_strJobUuid
    )
{
    __DebugFunction();

    m_stlListOfWaitingJobs.push_back(c_strJobUuid);
}

/********************************************************************************************
 *
 * @class SafeObject
 * @function GetSafeObjectIdentifier
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

const std::vector<std::string> & __thiscall SafeObject::GetQueuedJobsUuid(void) const
{
    __DebugFunction();

    return m_stlListOfWaitingJobs;
}

/********************************************************************************************
 *
 * @class SafeObject
 * @function GetSafeObjectIdentifier
 * @brief Constructor to create a Job object
 *
 ********************************************************************************************/

const std::vector<std::string> & __thiscall SafeObject::GetListOfParameters(void) const
{
    __DebugFunction();

    return m_stlListOfParameters;
}
