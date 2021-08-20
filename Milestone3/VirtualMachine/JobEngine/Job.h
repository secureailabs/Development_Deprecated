/*********************************************************************************************
 *
 * @file Job.h
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Job class to create and run jobs on the Virtual Machine
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "RootOfTrustNode.h"
#include "Socket.h"
#include "StructuredBuffer.h"
#include "SafeObject.h"

#include <vector>
#include <string>
#include <fstream>
#include <future>
#include <unordered_map>
#include <unordered_set>

/********************************************************************************************/

enum class JobState
{
    eWaiting,
    eHalted,
    eFinished,
    eRunning
};

/********************************************************************************************/

class Job : public Object
{
    public:

        Job(
            _in std::string strJobUuid
            );
        // We only want one instance of the job to exist, so not copy constructor
        Job(
            _in const Job & c_oJob
            ) = delete;
        ~Job(void);

        void __thiscall SetSafeObject(
            _in std::shared_ptr<SafeObject> c_poSafeObjectId
            );
        bool __thiscall SetParameter(
            _in const std::string & c_strParameterIdentifier,
            _in const std::string & c_strValueIdentifier,
            _in unsigned int nExpectedParameters,
            _in unsigned int nValueIdentifier
            );
        void __thiscall RemoveAvailableDependency(
            _in const std::string & strOutFileName
            );
        const std::string & __thiscall GetJobUuid(void) const throw();

    private:

        // Private member methods
        bool __thiscall AreAllParametersSet(void);
        void __thiscall TryRunJob(void);
        void __thiscall AddDependency(
            _in const std::string & strDepedencyName
            ) throw();

        // Private data members
        StructuredBuffer m_oParameters;
        std::string m_strJobUuid;
        std::shared_ptr<SafeObject> m_poSafeObject;
        std::vector<std::string> m_stlInputParameters;
        std::unordered_set<std::string> m_stlSetOfDependencies;
        std::mutex m_oMutexJob;
};
