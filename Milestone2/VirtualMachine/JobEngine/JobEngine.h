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
#include <string>
#include <fstream>

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
        void StartServer(void);

    private:

        // Private member methods
        JobEngine(void);

        // Private data members
        static JobEngine m_oJobEngine;
};

JobEngine JobEngine::m_oJobEngine;

/********************************************************************************************/
