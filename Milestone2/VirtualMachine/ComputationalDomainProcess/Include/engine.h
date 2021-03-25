/*********************************************************************************************
 *
 * @file engine.h
 * @author Jingwei Zhang
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "job.h"
#include "CoreTypes.h"
#include <deque>
#include <map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <map>
#include <queue>
#include <memory>
#include <sys/types.h>

/********************************************************************************************/

class JobEngine
{
    private:
        std::deque<std::unique_ptr<Job>> m_stlJobQueue;
        std::mutex m_stlQueueMutex;
        std::condition_variable m_stlQueueConditionVariable;

        std::map<std::string, std::unique_ptr<Job>> m_stlJobMap;
        std::mutex m_stlMapMutex;
        std::condition_variable m_stlMapConditionVariable;

        size_t m_nMaxWorkers;
        bool m_fStop;

    public:
        JobEngine
        (
            _in size_t nMaxProcesses
		): 
            m_nMaxWorkers(nMaxProcesses) 
            {}

        void __thiscall AddOneJob
        (
        	_in std::unique_ptr<Job>&& stlJobPtr
		);

        void __thiscall ProcessOneJob(void);
        std::unique_ptr<Job> __thiscall GetOneJob(void);

        void __thiscall RemoveDone
        (
        	std::string& stlId
		);

        void __thiscall Dispatch(void);
        void __thiscall Halt(void);

        std::string __thiscall GetJobResult
        (
            std::string& strID
        );

        JobStatus& __thiscall PeekStatus
        (
            std::string& strJobID
        );

        std::string __thiscall RetrieveJobs(void);
};

