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
#include "TlsTransactionHelperFunctions.h"
#include "TlsServer.h"

#include <vector>
#include <string>
#include <fstream>
#include <future>
#include <unordered_map>
#include <unordered_set>

/********************************************************************************************/

const std::string gc_strHaltAllJobsSignalFilename = "StopAllJobs";

/********************************************************************************************/

enum class EngineRequest
{
    eShutdown = 0,
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

class SafeObject
{
    public:

        SafeObject(
            _in const StructuredBuffer & c_oStructuredBuffer
            );
        SafeObject(
            _in const SafeObject & c_oSafeObject
            );
        ~SafeObject(void);

        int __thiscall Run(
            _in const std::string & c_strJobUuid
            ) const;
        const std::string & __thiscall GetSafeObjectIdentifier(void) const;
        const std::string & __thiscall GetCommandToExecute(void) const;

    private:

        // Private member methods

        // Private data members
        std::string m_strSafeObjectIdentifier;
        std::string m_strCommandToExecute;
};

/********************************************************************************************/

class Job
{
    public:

        Job(
            _in std::string strJobId
            );
        Job(
            _in const std::string & c_strJobId,
            _in const SafeObject * const c_poSafeObject
            );
        Job(
            _in const Job & c_oJob
            );
        ~Job(void);

        void __thiscall SetSafeObject(
            _in const SafeObject * const c_poSafeObjectId
            );
        int __thiscall TryRunJob(
            _in const std::string & c_strParameterFile
            );
        void __thiscall SetParameter(
            _in const std::string & c_strParameterIdentifier,
            _in const std::string & c_strValueIdentifier,
            _in unsigned int nExpectedParameters,
            _in unsigned int nValueIdentifier
            );
        const std::vector<std::string> & __thiscall GetInputParameters(void) const;
        void __thiscall SetOutputFileName(
            _in const std::string & c_strOutFileName
            );
        const std::string & __thiscall GetOutputFileName(void) const;
        const std::string & __thiscall GetJobId(void) const;

    private:

        // Private member methods

        // Private data members
        std::string m_strSafeObjectUuid;
        std::string m_strJobUuid;
        const SafeObject * m_poSafeObject;
        std::unordered_map<unsigned int, std::string> m_stlInputParameters;
        int m_nNumberOfParameters = 0;
        std::string m_stlOutputFileName;
};

/********************************************************************************************/

class JobEngine
{
    public:

        // Singleton objects cannot be copied so we delete the copy constructor and = operator
        JobEngine(
            _in const JobEngine & c_oJobEngine
            ) = delete;
        void operator=(
            _in JobEngine const & c_oJobEngine
            ) = delete;
        ~JobEngine(void);

        static JobEngine & Get(void);

        void __thiscall StartServer(void);
        void __thiscall ListenToRequests(
            _in TlsNode * c_poTlsNode
            );
        void __thiscall FileCreateCallback(
            _in const std::string & c_strFileCreatedFilename
        );

    private:

        // Private member methods
        JobEngine(void);
        SafeObject * __thiscall PushSafeObject(
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
        void __thiscall HaltAllJobs(
            _in const StructuredBuffer & oStructuredBuffer
            );
        void __thiscall SendSignal(
            _in const std::string & JobId,
            _in JobStatusSignals eJobStatus
            );

        // Private data members
        static JobEngine m_oJobEngine;
        TlsServer * m_poTlsServer;
        TlsNode * m_poTlsNode;
        std::unordered_map<uint64_t, Job *> m_stlMapOfJobs;
        std::unordered_map<uint64_t, std::future<SafeObject *>> m_stlMapOfSafeObjects;
        std::unordered_map<std::string, Job *> m_stlMapOfParameterToJob;
        std::unordered_set<std::string> m_stlSetOfPullObjects;
        std::mutex m_stlMutexPullFiles;
        std::mutex m_stlMutexParamtereFiles;
};
