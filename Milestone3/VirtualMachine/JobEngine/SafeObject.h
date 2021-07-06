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

#include <vector>

class SafeObject : public Object
{
    public:

        SafeObject(
            _in const std::string c_strSafeObjectUuid
            );
        SafeObject(
            _in const SafeObject & c_oSafeObject
            );
        ~SafeObject(void);

        void __thiscall Setup(
            _in const StructuredBuffer & c_oStructuredBuffer
            );
        int __thiscall Run(
            _in const std::string & c_strJobUuid,
            _in const std::string & c_strOutFileName
            ) const;
        const std::string & __thiscall GetSafeObjectIdentifier(void) const;
        const std::string & __thiscall GetCommandToExecute(void) const;
        void __thiscall AddJobUuidToQueue(
                _in const std::string & c_strJobUuid
            );
        const std::vector<std::string> & __thiscall GetQueuedJobsUuid(void) const;
        const std::vector<std::string> & __thiscall GetListOfParameters(void) const;

    private:

        // Private member methods

        // Private data members
        std::string m_strSafeObjectIdentifier;
        std::string m_strCommandToExecute;
        std::vector<std::string> m_stlListOfWaitingJobs;
        std::vector<std::string> m_stlListOfParameters;
};
