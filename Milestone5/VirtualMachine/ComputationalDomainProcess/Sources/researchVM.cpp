/*********************************************************************************************
 *
 * @file researchVM.cpp
 * @author Jingwei Zhang
 * @date 29 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used as the backend of job engine and handle resercher requests
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "researchVM.h"
#include "DataConnector.h"
#include "SocketClient.h"
#include "StatusMonitor.h"
#include "StructuredBuffer.h"
#include "TlsTransactionHelperFunctions.h"
#include "IpcTransactionHelperFunctions.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "Guid.h"
#include "utils.h"
#include <thread>
#include <vector>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <stdio.h>

//Audit:
//1000: ExecJob
//2000: PushData
//3000: PullData
//4000: PushFN

/********************************************************************************************
 *
 * @class ComputationVM
 * @function ComputationVM
 * @brief Constructor to create a ComputationVM object
 * @param[in] wPortIdentifier the port of computation server will listen on
 * @param[in] nMaxProcess number of max worker threads in job engine
 *
 ********************************************************************************************/

ComputationVM::ComputationVM(
    _in Word wPortIdentifier,
    _in size_t nMaxProcess,
    _in RootOfTrustNode & oRootOfTrustNode
    )
    : m_oEngine(nMaxProcess), m_oTlsServer(wPortIdentifier), m_oRootOfTrustNode(oRootOfTrustNode)
{
    __DebugFunction();

    m_strVirtualMachineIdentifier = Guid().ToString(eRaw);
    
    std::cout<<"init vm"<<std::endl;
    std::cout<<m_strVirtualMachineIdentifier<<std::endl;
    
    StructuredBuffer oEventData;
    oEventData.PutBoolean("Success", true);
    oEventData.PutGuid("ComputationalDomainIdentifier", m_oRootOfTrustNode.GetDomainIdentifier());
    oEventData.PutUnsignedInt32("CPU cores", 16);
    oEventData.PutString("Python Version", "python3.8");
    oEventData.PutString("OS Version", "Ubuntu 20.04");
    m_oRootOfTrustNode.RecordAuditEvent("COMPUTATIONAL_PROCESS_START", 0x1111, 0x05, oEventData);
    std::cout<<"init vm done"<<std::endl;
}

/********************************************************************************************/

ComputationVM::~ComputationVM(void)
{
    __DebugFunction();
    
    StructuredBuffer oEventData;
    oEventData.PutBoolean("Success", true);
    oEventData.PutGuid("ComputationalDomainIdentifier", m_oRootOfTrustNode.GetDomainIdentifier());
    m_oRootOfTrustNode.RecordAuditEvent("COMPUTATIONAL_PROCESS_STOP", 0x1111, 0x05, oEventData);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function InitializeVM
 * @brief initialize the computation VM, start the job engine and server on two different threads
 *
 ********************************************************************************************/

void __thiscall ComputationVM::Initialize(void)
{
    __DebugFunction();

    std::vector<std::thread> stlThreadPool;

	stlThreadPool.push_back(std::thread(&ComputationVM::SocketListen, this));
	stlThreadPool.push_back(std::thread(&JobEngine::Dispatch, &m_oEngine));

	for (auto & stlThread: stlThreadPool)
    {
		stlThread.join();
    }
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function SocketListen
 * @brief Listen on port assigned to receive resercher commands
 *
 ********************************************************************************************/

void ComputationVM::SocketListen(void)
{
    __DebugFunction();
    TlsNode* poNewConnection = nullptr;

    StatusMonitor oStatusMonitor("void ComputationVM::SocketListen(void)");

    while (false == oStatusMonitor.IsTerminating())
    {
        try 
        {
            if (true == m_oTlsServer.WaitForConnection(1000))
            {
                poNewConnection = m_oTlsServer.Accept();
                _ThrowIfNull(poNewConnection, "Handle computationVM connection request retures a null pointer.", nullptr);
                this->HandleConnection(poNewConnection);
                poNewConnection->Release();
                poNewConnection = nullptr;
            }
        }
        catch(const BaseException & oBaseException)
        {
            StructuredBuffer oResponseStructuredBuffer;
            std::cout<<oBaseException.GetExceptionMessage()<<std::endl;
            oResponseStructuredBuffer.PutString("Status", "Fail");
            oResponseStructuredBuffer.PutString("Error", oBaseException.GetExceptionMessage());
        }
        if (nullptr != poNewConnection)
        {
            poNewConnection->Release();
            poNewConnection = nullptr;
        }
    }
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleConnection
 * @brief Proccess connection requests from researcher
 * @param[in] poSocket The pointer to Socket object of the server
 *
 ********************************************************************************************/

void ComputationVM::HandleConnection(
    _in TlsNode *  poSocket
    )
{
    __DebugFunction();

    bool bConnectionState = true;
    StructuredBuffer oResponseStructuredBuffer;

    std::vector<Byte> stlContent = GetTlsTransaction(poSocket, 10*1000);
    std::cout<<"Get tls transaction request: "<<stlContent.size()<<std::endl;
    if(0<stlContent.size())
    {
        StructuredBuffer oContent(stlContent);
        StructuredBuffer oResponse;
        
        std::string strReply;
        unsigned int nType = (unsigned int)oContent.GetInt8("Type");
        std::cout<<"check type: "<<nType<<std::endl;

        switch(nType)
        {
            //TODO: handle failure cases
            case (unsigned int)eQUIT:
                this->HandleQuit(oContent, oResponse);
                bConnectionState =false;
                Halt();
                break;
            case (unsigned int)eRUN:
                this->HandleRun(oContent, oResponse);
                break;
            case (unsigned int)eCONNECT:
                std::cout<<"handle vm connection"<<std::endl;
                this->HandleConnect(oContent, oResponse);
                break;
            case (unsigned int)eINSPECT:
                this->HandleInspect(oContent, oResponse);
                break;
            case (unsigned int)eGETTABLE:
                this->HandleGetTable(oContent, oResponse);
                break;
            case (unsigned int)ePUSHDATA:
                this->HandlePushData(oContent, oResponse);
                break;
            case (unsigned int)ePULLDATA:
                this->HandlePullData(oContent, oResponse);
                break;
            case (unsigned int)eDELETEDATA:
                this->HandleDeleteData(oContent, oResponse);
                break;
            case (unsigned int)ePUSHFN:
                this->HandlePushFN(oContent, oResponse);
                break;
            default:
                oResponse.PutBoolean("Status", false);
                oResponse.PutString("Payload", "invalid request");
        }
        PutTlsTransaction(poSocket, oResponse);
    }
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleConnect
 * @brief Handle incoming connection request
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleConnect(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();

    bool fSuccess = true;
    
    std::cout<<"connect vm"<<std::endl;
    __DebugAssert(true == oContent.IsElementPresent("EOSB", ANSI_CHARACTER_STRING_VALUE_TYPE));
    std::cout<<"present eosb"<<std::endl;
    std::cout<<oContent.GetString("EOSB")<<std::endl;
    
    oResponse.PutString("VMID", m_strVirtualMachineIdentifier);
    oResponse.PutBoolean("Success", fSuccess);
    
    StructuredBuffer oEventData;
    oEventData.PutBoolean("Success", true);
    oEventData.PutString("Username", "lbart@igr.com");
    oEventData.PutString("OrchestratorIpAddress", "132.34.4.23");
    oEventData.PutString("OrchestratorVersion", "1.0.0");
    oEventData.PutString("EOSB", oContent.GetString("EOSB"));
    m_oRootOfTrustNode.RecordAuditEvent("CONNECT_SUCCESS", 0x1111, 0x04, oEventData);
    std::cout<<"connect vm done"<<std::endl;
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleQuit
 * @brief Handle the situation when recieving quit signal from researcher
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleQuit(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();

    m_oEngine.Halt();
    oResponse.PutBoolean("Success", true);
    
    oContent.PutBoolean("Success", true);
    oContent.PutString("Username", "lbart@igr.com");
    m_oRootOfTrustNode.RecordAuditEvent("LOGOFF", 0x1111, 0x04, oContent);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleRun
 * @brief Handle the situation when recieving run signal from researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleRun(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();

    std::string strFunctionNode = oContent.GetString("FNID");
    std::string strJobIdentifier = oContent.GetString("JobID");
    std::vector<std::string> stlInput = m_stlFNMap[strFunctionNode]->GetInput();
    std::vector<std::string> stlOutput = m_stlFNMap[strFunctionNode]->GetOutput();
    std::vector<std::string> stlConfidentialInput = m_stlFNMap[strFunctionNode]->GetConfidentialInput();
    std::vector<std::string> stlConfidentialOutput = m_stlFNMap[strFunctionNode]->GetConfidentialOutput();

    std::unique_ptr<Job> stlNewJob = std::make_unique<PythonJob>(strFunctionNode, strJobIdentifier, stlInput, stlOutput, stlConfidentialInput, stlConfidentialOutput);
    m_oEngine.AddOneJob(std::move(stlNewJob));

    oResponse.PutBoolean("Success", true);
    
    oContent.PutBoolean("Success", true);
    m_oRootOfTrustNode.RecordAuditEvent("RUN_FN", 0x1010, 0x01, oContent);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleInspect
 * @brief Handle the inspect request from the researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleInspect(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::string strReply = m_oEngine.RetrieveJobs();
    
    oResponse.PutBoolean("Success", true);
    oResponse.PutString("Payload", strReply);
    
    oContent.PutBoolean("Success", true);
    oContent.PutString("Jobs", strReply);
    m_oRootOfTrustNode.RecordAuditEvent("INSPECT", 0x1010, 0x01, oContent);
}

/********************************************************************************************/

void __thiscall ComputationVM::HandleGetTable(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::string strReply = this->RetrieveDatasets();
    
    oResponse.PutBoolean("Success", true);
    oResponse.PutString("Payload", strReply);
    
    oContent.PutBoolean("Success", true);
    oContent.PutString("Tables", strReply);
    m_oRootOfTrustNode.RecordAuditEvent("GET_TABLE", 0x1110, 0x01, oContent);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function RetrieveDatasets
 * @brief Get the list of all datasets from the data connector
 *
 ********************************************************************************************/

std::string __thiscall ComputationVM::RetrieveDatasets(void)
{
    __DebugFunction();
    
    Socket * poSocket =  ConnectToUnixDomainSocket("/tmp/{0bd8a254-49e4-4b86-b1b8-f353c18013c5}");
    StructuredBuffer oRequest;

    oRequest.PutInt8("RequestType",eGetTableMetadata);
    oRequest.PutUnsignedInt32("TableID", 0);

    std::vector<Byte> stlResponse = ::PutIpcTransactionAndGetResponse(poSocket, oRequest, false);
    // Release the socket
    poSocket->Release();
    std::string strResponse;

    if (0 < stlResponse.size())
    {
        StructuredBuffer oResponse(stlResponse);
        StructuredBuffer temp = oResponse.GetStructuredBuffer("ResponseData");
        std::stringstream stlStream;
        stlStream << "Table 0    " << "Rows: " << temp.GetInt32("NumberRows") << "    Cols:" << temp.GetInt32("NumberColumns") << std::endl;
        strResponse = stlStream.str();
        
        m_oRootOfTrustNode.RecordAuditEvent("RETRIEVE_DATASET", 0x1110, 0x01, oRequest);
    }
    else
    {
        std::cout << "Failed to read response" << std::endl;
    }

    return strResponse;
}


/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleCheck
 * @brief Handle the situation when recieving check signal from researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleCheck(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::string strJobIdentifierentifier = oContent.GetString("JobID");
    std::string strReply = m_oEngine.GetJobResult(strJobIdentifierentifier);
    
    oResponse.PutBoolean("Success", true);
    oResponse.PutString("Payload", strReply);
    
    oContent.PutBoolean("Success", true);
    oContent.PutString("Result", strReply);
    m_oRootOfTrustNode.RecordAuditEvent("CHECK_JOB", 0x1010, 0x01, oContent);
}

/********************************************************************************************/

void __thiscall ComputationVM::HandlePushData(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::vector<std::string> stlVarIDs;
    std::vector<std::vector<Byte>> stlVars;
    std::vector<std::string> stlConfidentialInputIDs;
    std::string strJobIdentifier = oContent.GetString("JobID");
    std::string strFunctionNodeIdentifier = oContent.GetString("FNID");
    StructuredBuffer oVariableIdentifiers = oContent.GetStructuredBuffer("VarIDs");
    StructuredBuffer oVariables = oContent.GetStructuredBuffer("Vars");
    StructuredBuffer oConfidentialInputIdentifiers = oContent.GetStructuredBuffer("ConfidentialInputIDs");

    std::cout << "pushing data" << std::endl;

    BufToVec<std::vector<std::vector<Byte>>>(oVariables, stlVars);
    BufToVec<std::vector<std::string>>(oVariableIdentifiers, stlVarIDs);
    BufToVec<std::vector<std::string>>(oConfidentialInputIdentifiers, stlConfidentialInputIDs);

    std::cout << "convert to vectors" << std::endl;

    this->SaveBuffer(strJobIdentifier, stlVarIDs, stlVars);
    std::cout << "saving data to buffer" << std::endl;
    this->LinkPassID(strJobIdentifier, strFunctionNodeIdentifier, stlConfidentialInputIDs);
    std::cout << "linking data" << std::endl;
    
    oResponse.PutBoolean("Success", true);
    
    oContent.PutBoolean("Success", true);
    m_oRootOfTrustNode.RecordAuditEvent("PUSH_DATA", 0x1010, 0x01, oContent);
}

/********************************************************************************************/

void __thiscall ComputationVM::SaveBuffer(
    std::string & strJobIdentifier,
    std::vector<std::string> & stlVarIDs,
    std::vector<std::vector<Byte>> & stlVars
    )
{
    __DebugFunction();

    size_t nNumber = stlVars.size();
    for(size_t i =0; i<nNumber; i++)
    {
        std::ofstream stlVarFile;
        stlVarFile.open(std::string("/tmp/"+strJobIdentifier+stlVarIDs[i]).c_str(), std::ios::out | std::ios::binary);
        stlVarFile.write((char*)&stlVars[i][0], stlVars[i].size());
        stlVarFile.close();
    }
}

/********************************************************************************************/

void __thiscall ComputationVM::LinkPassID(
    _in std::string & strJobIdentifier,
    _in std::string & strFunctionNodeIdentifier,
    _in std::vector<std::string> & stlPassIDs
    )
{
    __DebugFunction();

    size_t nNumber = stlPassIDs.size();

    std::vector<std::string> stlConfidentialInputIDs =  m_stlFNMap[strFunctionNodeIdentifier]->GetConfidentialInput();

    try{
        for(size_t i =0;i<nNumber;i++)
        {
            std::string strTarget = std::string("/tmp/"+stlPassIDs[i]);
            std::string strLinkpath = std::string("/tmp/"+strJobIdentifier+stlConfidentialInputIDs[i]);
            const char* target = strTarget.c_str();
            const char* linkpath = strLinkpath.c_str();

            while(access(target, F_OK)!=0)   
            {
                //std::cout<<"filename:"<<target<<" linkdata waiting for file"<<std::endl;
            }

            int res = symlink(target, linkpath);
            if(-1 == res)
                _ThrowBaseException("Can not establish symbolic link", nullptr);
        }
    }
    catch(const BaseException & oBaseException)
    {
        std::cout<<oBaseException.GetExceptionMessage()<<std::endl;
    }
}

void __thiscall ComputationVM::HandlePullData(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::string strJobIdentifier = oContent.GetString("JobID");
    StructuredBuffer oVariableIdentifiers = oContent.GetStructuredBuffer("VarIDs");

    std::vector<std::string> stlVarIDs;

    BufToVec<std::vector<std::string>>(oVariableIdentifiers, stlVarIDs);
    std::vector<std::vector<Byte>> stlOutputs;

    this->LoadDataToBuffer(strJobIdentifier, stlVarIDs, stlOutputs);

    StructuredBuffer oVarBuffer;
    VecToBuf<std::vector<std::vector<Byte>>>(stlOutputs, oVarBuffer);
    
    oResponse.PutBoolean("Success", true);
    oResponse.PutStructuredBuffer("Vars", oVarBuffer);
    
    oContent.PutBoolean("Success", true);
    oContent.PutStructuredBuffer("Vars", oVarBuffer);
    m_oRootOfTrustNode.RecordAuditEvent("PULL_DATA", 0x1010, 0x01, oContent);
}

/********************************************************************************************/

void __thiscall ComputationVM::LoadDataToBuffer(
    _in std::string & strJobIdentifier,
    _in std::vector<std::string> & stlVarIDs,
    _in std::vector<std::vector<Byte>> & stlVars
    )
{
    __DebugFunction();
    size_t nNumber = stlVarIDs.size();
    for (size_t i = 0; i < nNumber; i++)
    {
        // while ((0 != access(std::string("/tmp/"+strJobIdentifier+stlVarIDs[i]).c_str(), F_OK))||(eCompleted != m_oEngine.PeekStatus(strJobIdentifier)))
        // {
        //     //std::cout<<"filename:"<<"/tmp/"+strJobIdentifier+stlVarIDs[i]<<" waiting for file"<<std::endl;
        // }
        std::cout<<"filename:"<<"/tmp/"+strJobIdentifier+stlVarIDs[i]<<" waiting for file"<<std::endl;
        std::string strMarker = "/tmp/" + strJobIdentifier + ".marker";
        const char* fname = strMarker.c_str();
        while (0 != access(fname, F_OK))
        {
            //std::cout<<"filename:"<<"/tmp/"+strJobID+stlVarIDs[i]<<" waiting for file"<<std::endl;
        }
        std::cout<<"file obtained"<<std::endl;

        std::ifstream stlVarFile;
        stlVarFile.open(std::string("/tmp/"+strJobIdentifier+stlVarIDs[i]).c_str(), std::ios::out | std::ios::binary);
        stlVarFile.unsetf(std::ios::skipws);

        std::vector<Byte> stlVec;
        stlVec.insert(stlVec.begin(),std::istream_iterator<Byte>(stlVarFile), std::istream_iterator<Byte>());
        stlVars.push_back(stlVec);

        std::cout<<"vector length: "<<stlVec.size()<<std::endl;

        stlVarFile.close();
    }
}

/********************************************************************************************/

void __thiscall ComputationVM::HandleDeleteData(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::vector<std::string> stlVars;
    StructuredBuffer oVariables = oContent.GetStructuredBuffer("Vars");

    BufToVec(oVariables, stlVars);

    for (size_t i = 0; i < stlVars.size(); i++)
    {
        std::remove(std::string("/tmp/"+stlVars[i]).c_str());
    }
    
    oResponse.PutBoolean("Success", true);
    
    oContent.PutBoolean("Success", true);
    m_oRootOfTrustNode.RecordAuditEvent("DELETE_DATA", 0x1010, 0x01, oContent);
}

/********************************************************************************************/

void __thiscall ComputationVM::HandlePushFN(
    _in StructuredBuffer & oContent,
    _in StructuredBuffer & oResponse
    )
{
    __DebugFunction();
    
    std::string strFunctionNodeIdentifier = oContent.GetString("FNID");
    std::string strFNScript = oContent.GetString("FNScript");

    this->SaveFN(strFunctionNodeIdentifier, strFNScript);

    StructuredBuffer oInput = oContent.GetStructuredBuffer("Input");
    StructuredBuffer oOutput = oContent.GetStructuredBuffer("Output");
    StructuredBuffer oConfidentialInput = oContent.GetStructuredBuffer("ConfidentialInput");
    StructuredBuffer oConfidentialOutput = oContent.GetStructuredBuffer("ConfidentialOutput");

    std::vector<std::string> stlOutputIDs;
    std::vector<std::string> stlInputIDs;
    std::vector<std::string> stlConfidentialInputIDs;
    std::vector<std::string> stlConfidentialOutputIDs;

    BufToVec(oInput, stlInputIDs);
    BufToVec(oOutput, stlOutputIDs);
    BufToVec(oConfidentialInput, stlConfidentialInputIDs);
    BufToVec(oConfidentialOutput, stlConfidentialOutputIDs);

    std::unique_ptr<FunctionNode> stlFN = std::make_unique<FunctionNode>(stlInputIDs, stlOutputIDs, stlConfidentialInputIDs, stlConfidentialOutputIDs, strFunctionNodeIdentifier);
    m_stlFNMap.insert({strFunctionNodeIdentifier,std::move(stlFN)});
    
    oResponse.PutBoolean("Status", true);
    
    oContent.PutBoolean("Status", true);
    m_oRootOfTrustNode.RecordAuditEvent("PUSH_FN", 0x1010, 0x01, oContent);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function Halt
 * @brief Shutdown the server
 *
 ********************************************************************************************/

void __thiscall ComputationVM::Halt(void)
{
    __DebugFunction();
    //m_fStop = true;
}

void __thiscall ComputationVM::SaveFN(
    std::string & strFunctionNodeIdentifier,
    std::string & strFNScript
    )
{
    __DebugFunction();
    
    std::ofstream stlFNFile;
    stlFNFile.open("/tmp/"+strFunctionNodeIdentifier);
    stlFNFile << strFNScript;
    stlFNFile.close();
}
