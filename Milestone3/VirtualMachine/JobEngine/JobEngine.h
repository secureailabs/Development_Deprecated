/*********************************************************************************************
 *
 * @file JobEngine.h
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the JobEngine class to create and run jobs on the Virtual Machine
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "RootOfTrustNode.h"
#include "Socket.h"
#include "StructuredBuffer.h"
#include "SafeObject.h"
#include "Job.h"
#include "JobEngineHelper.h"

#include <vector>
#include <string>
#include <fstream>
#include <future>
#include <unordered_map>
#include <unordered_set>


enum class EngineRequest
{
    eVmShutdown = 0,
    ePushSafeObject = 1,
    eSubmitJob = 2,
    ePullData = 3,
    ePushdata = 4,
    eSetParameters = 5,
    eHaltAllJobs = 6,
    eJobStatusSignal = 7
};

enum class JobStatusSignals
{
    eJobStart = 0,
    eJobDone = 1,
    eJobFail = 2,
    ePostValue = 3,
    eVmShutdown = 4
};

/********************************************************************************************/

class JobEngine : public Object
{
    public:

        // Singleton objects cannot be copied, the copy constructor and = operator are deleted
        JobEngine(
            _in const JobEngine & c_oJobEngine
            ) = delete;
        void operator=(
            _in JobEngine const & c_oJobEngine
            ) = delete;
        ~JobEngine(void);

        static JobEngine & Get(void);

        void __thiscall StartServer(
            _in Socket * poSocket
        );
        void __thiscall ListenToRequests(void);
        void __thiscall FileCreateCallback(
            _in const std::string & c_strFileCreatedFilename
        );
        void __thiscall SendSignal(
            _in const StructuredBuffer & oStructuredBuffer
            );

    private:

        // Private member methods
        JobEngine(void);
        void __thiscall PushSafeObject(
            _in const StructuredBuffer & oStructuredBuffer
            );
        void __thiscall PushData(
            _in const StructuredBuffer & oStructuredBuffer
            );
        void __thiscall PullData(
            _in const std::string & c_strFileNameOfData
            );
        void __thiscall SetJobParameter(
            _in const StructuredBuffer & oStructuredBuffer
            );
        void __thiscall SubmitJob(
            _in const StructuredBuffer & oStructuredBuffer
            );
        void __thiscall ResetJobEngine(void);

        // Private data members
        std::mutex m_oMutexjobEngine;
        uint64_t m_FileListenerId = 0;
        bool m_fIsEngineRunning;
        static JobEngine m_oJobEngine;
        Socket * m_poSocket;
        std::unordered_map<std::string, std::shared_ptr<Job>> m_stlMapOfJobs;
        std::mutex m_oMutexOnJobsMap;
        std::unordered_map<std::string, std::shared_ptr<SafeObject>> m_stlMapOfSafeObjects;
        std::mutex m_oMutexOnSafeObjectMap;
        std::unordered_map<std::string, std::shared_ptr<Job>> m_stlMapOfParameterValuesToJob;
        std::mutex m_oMutexOnParameterValuesToJobMap;
        std::unordered_set<std::string> m_stlSetOfPullObjects;
        std::mutex m_oMutexOnSetOfPullObjects;
};
