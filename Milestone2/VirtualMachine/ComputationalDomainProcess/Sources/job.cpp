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

#include "job.h"
#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <filesystem>

/********************************************************************************************
 *
 * @class Job
 * @function Job
 * @brief Constructor to create a Job object
 * @param[in] c_strIn input string for containing the information needed for job engine to run
 * @param[in] oFormat the job format: script or string
 * @note
 *     The contructor is only designed to run python-like job
 *
 ********************************************************************************************/

Job::Job
    (
        _in std::string& strFunctionNode,
        _in std::string& strJobID,
        _in std::vector<std::string>& stlInput,
        _in std::vector<std::string>& stlOutput,
        _in std::vector<std::string>& stlConfidentialInput,
        _in std::vector<std::string>& stlConfidentialOutput
    )
    : m_oStatus(eIdle),
      m_strFunctionNodeNumber(strFunctionNode),  
      m_strJobID(strJobID),
      m_stlInput(stlInput),
      m_stlOutput(stlOutput),
      m_stlConfidentialInput(stlConfidentialInput),
      m_stlConfidentialOutput(stlConfidentialOutput)

{
    
}

/********************************************************************************************
 *
 * @class Job
 * @function GetID
 * @brief get the string output of a job ID
 *
 ********************************************************************************************/

std::string __thiscall Job::GetJobID(void) const
{
    return m_strJobID;
}


/********************************************************************************************
 *
 * @class Job
 * @function GetOutput
 * @brief get the output of a job if it is finished, otherwise indicate it is running
 *
 ********************************************************************************************/

const std::string __thiscall Job::GetOutput(void)
{
//    if(m_oStatus == eIdle){
//        return "The job has not started running yet\n";
//    }

    std::ifstream stlIn, stlError;
    std::ostringstream stlBuf;
    stlIn.open(m_strOutputFile);
    stlError.open(m_strErrFile);
    stlBuf<<"OUTPUT: "<<stlIn.rdbuf()<<std::endl;
    stlBuf<<"ERROR: "<<stlError.rdbuf()<<std::endl;

    stlIn.close();
    stlError.close();

    return stlBuf.str();
}

/********************************************************************************************
 *
 * @class Job
 * @function GetStatus
 * @brief get the job status
 *
 ********************************************************************************************/

JobStatus& __thiscall Job::GetStatus(void)
{
    return m_oStatus;
}

void __thiscall Job::SetStatus(JobStatus oNewStatus)
{
     m_oStatus = oNewStatus;
}

/********************************************************************************************
 *
 * @class Job
 * @function SetOutputFile
 * @brief set the output file for a job to write its results.
 *
 ********************************************************************************************/

void __thiscall Job::SetOutputAndErrFile(void)
{
    m_strOutputFile = "/tmp/" + m_strJobID + ".output";
    m_strErrFile = "/tmp/" + m_strJobID + ".error";
}

/********************************************************************************************
 *
 * @class PythonJob
 * @function JobRun
 * @brief run a python job
 *
 ********************************************************************************************/

// void __thiscall PythonJob::JobRunString(void)
// {

//     //the cpython internal function to init, run and finalize a python interpreter
//     Py_Initialize();

//     PyObject *pPySys = PyImport_ImportModule("sys");
//     FILE* pPyOutFile = fopen(m_strOutputFile.c_str(), "w+");
//     FILE* pPyErrFile = fopen(m_strErrFile.c_str(), "w+");

//     PyObject *pPyOut = PyFile_FromFd(fileno(pPyOutFile), (char*)(m_strOutputFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
//     PyObject *pPyErr = PyFile_FromFd(fileno(pPyErrFile), (char*)(m_strErrFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
//     PyObject_SetAttrString(pPySys, "stdout", pPyOut);
//     PyObject_SetAttrString(pPySys, "stderr", pPyErr);

//     rewind(pPyOutFile);
//     rewind(pPyErrFile);

//     int nRes = PyRun_SimpleString(m_c_strInput.c_str());

//     fclose(pPyOutFile);
//     fclose(pPyErrFile);

//     Py_Finalize();
// }

/********************************************************************************************
 *
 * @class PythonJob
 * @function JobRunScript
 * @brief run a python job
 *
 ********************************************************************************************/

// void __thiscall PythonJob::JobRunScript(void)
// {

//     FILE* pFile;
//     int nArgc;
//     const char* pArgv[2];
//     //int nStartPos = 0;

//     nArgc = 2; //std::atoi(m_c_strArgumentCount.c_str());
//     pArgv[0] = PYENTRANCE;//(m_c_strJobScript + ".py").c_str();
//     pArgv[1] = m_c_strInput.c_str();//m_c_strJobArguments.c_str();
//     // for(size_t i = 1; i<nArgc; i++){
//     //     size_t nTempPos = strArg.find(" ");
//     //     pArgv[i] = strArg.substr(nStartPos, nTempPos).c_str();
//     //     nStartPos = nTempPos+1;
//     // }
//     wchar_t** wszpArgv = (wchar_t**)PyMem_Malloc(sizeof(wchar_t*)*nArgc);
//     for (int i=0; i<nArgc; i++) {
//         wchar_t* wszArg = Py_DecodeLocale(pArgv[i], NULL);
//         wszpArgv[i] = wszArg;
//     }

//     Py_SetProgramName(wszpArgv[0]);
    
//     Py_Initialize();

//     PyObject *pPySys = PyImport_ImportModule("sys");
//     FILE* pPyOutFile = fopen(m_strOutputFile.c_str(), "w+");
//     FILE* pPyErrFile = fopen(m_strErrFile.c_str(), "w+");

//     PyObject *pPyOut = PyFile_FromFd(fileno(pPyOutFile), (char*)(m_strOutputFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
//     PyObject *pPyErr = PyFile_FromFd(fileno(pPyErrFile), (char*)(m_strErrFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
//     PyObject_SetAttrString(pPySys, "stdout", pPyOut);
//     PyObject_SetAttrString(pPySys, "stderr", pPyErr);

//     rewind(pPyOutFile);
//     rewind(pPyErrFile);

//     //PySys_SetArgv(nArgc, (char**)pArgv);
//     PySys_SetArgv(nArgc, wszpArgv);
//     pFile = fopen(pArgv[0],"r+");

//     int nRes = PyRun_SimpleFile(pFile, pArgv[0]);
//     //the cpython internal function to init, run and finalize a python interpreter

//     fclose(pPyOutFile);
//     fclose(pPyErrFile);

//     Py_Finalize();
// }

void __thiscall PythonJob::JobRunFunctionNode(void)
{
    size_t nInArgCount = m_stlInput.size();
    size_t nOutArgCount = m_stlOutput.size();
    size_t nConfidentialInputCount = m_stlConfidentialInput.size();
    size_t nConfidentialOutputCount = m_stlConfidentialOutput.size();
    size_t nArgLen = nInArgCount+nOutArgCount+nConfidentialInputCount +nConfidentialOutputCount;
    const char* pArgv[nArgLen];
    wchar_t* wszArgv[nArgLen];
    FILE* pFile;

    size_t i;
    size_t j=0;
    for(i=0;i<nInArgCount;i++)
    {
    	std::string strTmp("/tmp/"+m_strJobID+m_stlInput[i]);
        pArgv[j] = strTmp.c_str();
        wszArgv[j] = Py_DecodeLocale(pArgv[j], NULL);
        j++;
    }
    for(i=0;i<nConfidentialInputCount;i++)
    {
    	std::string strTmp("/tmp/"+m_strJobID+m_stlConfidentialInput[i]);
        pArgv[j]= strTmp.c_str();
        wszArgv[j]= Py_DecodeLocale(pArgv[j], NULL);
        j++;
    }
    for(i=0;i<nOutArgCount;i++)
    {
    	std::string strTmp("/tmp/"+m_strJobID+m_stlOutput[i]);
        pArgv[j]= strTmp.c_str();
        wszArgv[j]= Py_DecodeLocale(pArgv[j], NULL);
        j++;
    }
    for(i=0;i<nConfidentialOutputCount;i++)
    {
    	std::string strTmp("/tmp/"+m_strJobID+m_stlConfidentialOutput[i]);
        pArgv[j]= strTmp.c_str();
        wszArgv[j]= Py_DecodeLocale(pArgv[j], NULL);
        j++;
    }

    std::string pCodeFileName = "/tmp/"+m_strFunctionNodeNumber;

    Py_Initialize();

    PyObject *pPySys = PyImport_ImportModule("sys");
    FILE* pPyOutFile = fopen(m_strOutputFile.c_str(), "w+");
    FILE* pPyErrFile = fopen(m_strErrFile.c_str(), "w+");

    PyObject *pPyOut = PyFile_FromFd(fileno(pPyOutFile), (char*)(m_strOutputFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
    PyObject *pPyErr = PyFile_FromFd(fileno(pPyErrFile), (char*)(m_strErrFile.c_str()), (char*)"w+", -1, NULL, NULL, NULL, 1);
    PyObject_SetAttrString(pPySys, "stdout", pPyOut);
    PyObject_SetAttrString(pPySys, "stderr", pPyErr);

    rewind(pPyOutFile);
    rewind(pPyErrFile);

    PySys_SetArgv(nArgLen, wszArgv);
    pFile = fopen(pCodeFileName.c_str(),"r+");

    PyRun_SimpleFile(pFile, pCodeFileName.c_str());
    //the cpython internal function to init, run and finalize a python interpreter

    fclose(pPyOutFile);
    fclose(pPyErrFile);
    fclose(pFile);

    Py_Finalize();
}
