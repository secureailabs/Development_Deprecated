/*********************************************************************************************
 *
 * @file job.cpp
 * @author Jingwei Zhang
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used for single job related issues
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "engine.h"
#include "job.h"
#include "DebugLibrary.h"
#include "StatusMonitor.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

/********************************************************************************************
 *
 * @class JobEngine
 * @function AddOneJob
 * @param [in]pJobPtr
 * @brief add one job pointer to the job queue of the engine.
 *
 ********************************************************************************************/

void __thiscall JobEngine::AddOneJob(std::unique_ptr<Job>&& stlJobPtr)
{
    std::unique_lock<std::mutex> stlQueueLock(m_stlQueueMutex);
    m_stlJobQueue.push_back(std::move(stlJobPtr));
    stlQueueLock.unlock();
    m_stlQueueConditionVariable.notify_all();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function ProcessOneJob
 * @brief get a job from the job queue, put it into the job map and process it.
 *
 ********************************************************************************************/

void __thiscall JobEngine::ProcessOneJob(void)
{
    __DebugFunction();
    
    StatusMonitor oStatusMonitor("void __thiscall JobEngine::ProcessOneJob(void)");
    bool fIsTerminating = false;
    
	//m_fStop determines if the thread will stop or not, controlled by the Halt function.
	while (false == fIsTerminating)
	{
        //get a job from the job queue
        std::unique_lock<std::mutex> stlQueueLock(m_stlQueueMutex);
        while ((false == fIsTerminating)&&(m_stlJobQueue.empty()))
        {
            if (true == m_fStop)
            {
                oStatusMonitor.SignalTermination("User termination received.");
                fIsTerminating = true;
                stlQueueLock.unlock();
            }
            else if (true == oStatusMonitor.IsTerminating())
            {
                fIsTerminating = true;
                m_fStop = true;
                stlQueueLock.unlock();
            }
            else
            {
                m_stlQueueConditionVariable.wait(stlQueueLock);
            }
        }
        
        if (false == fIsTerminating)
        {
            std::unique_ptr<Job> stlTempJobPtr = GetOneJob();
            stlQueueLock.unlock();

            //put the job to the job map
            std::string strJob = stlTempJobPtr->GetJobID();
            std::unique_lock<std::mutex> stlMapLock(m_stlMapMutex);
            m_stlJobMap.insert({strJob, std::move(stlTempJobPtr)});
            stlMapLock.unlock();

            //set job id and output file
            //stlTempJobPtr->SetID(std::this_thread::get_id());
            m_stlJobMap[strJob]->SetOutputAndErrFile();

            //if(eString == pTempJobPtr->GetFormat())
            //pTempJobPtr->JobRunString();
            //else if(eScript == pTempJobPtr->GetFormat())
            //m_stlJobMap[strJob]->JobRunScript();

            //create a process to run the job

            pid_t nPid = fork();

            if(0==nPid)
            {
                m_stlJobMap[strJob]->JobRunFunctionNode();
                std::string strMarker = "/tmp/" + strJob + ".marker";
                const char* fname = strMarker.c_str();
                std::cout<<"create: "<<fname<<std::endl;
                std::ofstream outfile (fname);
                outfile << "marker" << std::endl;
                outfile.close();
                std::cout<<"create done "<<std::endl;
                exit(0);
            }

            //the main process will create a thread to wait for the job to finish
            else
            {
            	// int nStatus;
            	// waitpid(nPid, &nStatus, 0);
            	// std::cout<<"job done"<<std::endl;
            	// if(nStatus==0)
            	// {
            	//     m_stlJobMap[strJob]->SetStatus(eCompleted);
            	//     std::cout<<"job success"<<std::endl;
            	// }
            	// else
            	// {
            	//     //m_stlJobMap[strJob]->SetStatus(eCompleted);
            	//     std::cout<<"job fail"<<std::endl;
            	// }
            }
        }
    }
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function GetOneJob
 * @return [out]Job* the job pointer popped from the front of the queue
 * @brief get a job from the job queue.
 *
 ********************************************************************************************/

std::unique_ptr<Job> __thiscall JobEngine::GetOneJob()
{

    std::unique_ptr<Job> stlTempJobPtr(std::move(m_stlJobQueue.front()));
    m_stlJobQueue.pop_front();

    return stlTempJobPtr;
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function WaitOnChild
 * @param [in]nPid the pid of the child process
 * @brief wait for the child process to finish and reduced the concurrent running thread number
 *  , notify all waiting threads
 *
 ********************************************************************************************/
//void __thiscall JobEngine::WaitOnChild(pid_t nPid){
//
//}

/********************************************************************************************
 *
 * @class JobEngine
 * @function Dispatch
 * @brief dispatch a job thread when there are jobs in the job queue
 *
 ********************************************************************************************/

void __thiscall JobEngine::Dispatch()
{
    m_fStop = false;

    std::vector<std::thread> stlWorkersThreadPool(m_nMaxWorkers);
    for (size_t i=0; i<m_nMaxWorkers; ++i)
    {
    	stlWorkersThreadPool[i] = std::thread(&JobEngine::ProcessOneJob, this);
    }

    //std::thread stlHaltThread(&JobEngine::Halt, this);
    //stlHaltThread.join();

    for(auto& t: stlWorkersThreadPool)
    	t.join();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function RemoveDone
 * @brief remove a completed job from the job map
 *
 ********************************************************************************************/

void __thiscall JobEngine::RemoveDone(std::string& strJobName)
{
    m_stlJobMap.erase(strJobName);
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function Halt
 * @brief signal all threads to stop (keyboard, by pressing "q"), release them from waiting
 * on empty job queue.
 *
 ********************************************************************************************/

void __thiscall JobEngine::Halt()
{

	m_fStop=true;
	m_stlQueueConditionVariable.notify_all();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function GetJobResult
 * @brief Get the job result with jobID
 *
 ********************************************************************************************/

std::string __thiscall JobEngine::GetJobResult(std::string& strJobName)
{
	return m_stlJobMap[strJobName]->GetOutput();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function RetrieveJobs
 * @brief Get the information of all jobs from the engine
 *
 ********************************************************************************************/

std::string __thiscall JobEngine::RetrieveJobs(void)
{
	std::stringstream stlStream;
	stlStream<<"Job ID                                       "<<"Job Status            "<<std::endl;
	for(auto it = m_stlJobQueue.begin(); it != m_stlJobQueue.end(); ++it)
	{
        stlStream<<it->get()->GetJobID()<<"         "<<"WAITING"<<std::endl;
	}	
//	for(auto it = m_stlJobMap.begin(); it != m_stlJobMap.end(); ++it)
//	{
//        stlStream<<it->first<<"         "<<PeekStatus(it->get()->GetJobID())<<std::endl;
//	}
	return stlStream.str();
}

/********************************************************************************************
 *
 * @class JobEngine
 * @function TranslateStatus
 * @brief Translate the enum status type to string type
 *
 ********************************************************************************************/

JobStatus& __thiscall JobEngine::PeekStatus(std::string& strJobID)
{
    return m_stlJobMap[strJobID]->GetStatus();    
}
