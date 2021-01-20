/*********************************************************************************************
 *
 * @file job.h
 * @author Jingwei Zhang
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"

#include <sstream>
#include <string>
#include <sys/types.h>
#include <thread>


#define PYENTRANCE "/usr/local/lib/python3.8/site-packages/scripts/main.py"

/********************************************************************************************/

enum JobStatus
{
    eIdle,
    eRunning,
    eCompleted,
    eFail,
    eUnknown
};

enum JobFormat
{
    eString,
    eScript
};
/********************************************************************************************/

class Job
{
    public:
        Job
        (
        	_in Guid oFunctionNodeNumber,
            _in JobFormat oFormat,
            _in Guid oJobID,
            _in Guid oInputNodeNumber
		);

	    JobStatus& __thiscall GetStatus(void);
        const JobFormat& __thiscall GetFormat(void) const { return m_oFormat; }
        const std::string& __thiscall GetJobName(void) const { return m_strJobName; }
        const std::string& __thiscall GetOutputFile(void) const { return m_strOutputFile; }
        std::string __thiscall GetJobID(void) const;

        const std::string __thiscall GetOutput(void);
        void __thiscall SetOutputAndErrFile(void);

        // virtual void __thiscall JobRunString(void) {}
        // virtual void __thiscall JobRunScript(void) {}
        virtual void __thiscall JobRunFunctionNode(void) {}

    protected:
        const Guid m_oFunctionNodeNumber;
        const std::string m_c_strJobScript;
	    JobStatus m_oStatus;
        const JobFormat m_oFormat;
        const std::string m_strJobName;
        const Guid m_oJobID;
        const Guid m_oInputNodeNumber;
        std::string m_strOutputFile;
        std::string m_strErrFile;
	    std::thread::id m_stlJobId;
};

/********************************************************************************************/

class PythonJob : public Job
{
    public:
        PythonJob
        (
       	    _in Guid oFunctionNodeNumber,
            _in JobFormat oFormat,
            _in Guid oJobID,
            _in Guid oInputNodeNumber
		): Job(oFunctionNodeNumber, oFormat, oJobID, oInputNodeNumber) {}

        // virtual void __thiscall JobRunString(void);
        // virtual void __thiscall JobRunScript(void);
        virtual void __thiscall JobRunFunctionNode(void);
};