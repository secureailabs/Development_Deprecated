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
#include "DebugLibrary.h"
#include "SocketClient.h"
#include "StatusMonitor.h"
#include "StructuredBuffer.h"
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
    _in RootOfTrustNode& oRootOfTrustNode
    )
    : m_oEngine(nMaxProcess), m_oSocketServer(wPortIdentifier), m_fStop(false), m_oRootOfTrustNode(oRootOfTrustNode)
{
    m_strVMID = Guid().ToString(eRaw);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function InitializeVM
 * @brief initialize the computation VM, start the job engine and server on two different threads
 *
 ********************************************************************************************/

void __thiscall ComputationVM::InitializeVM(void)
{
	std::vector<std::thread> stlThreadPool;

	stlThreadPool.push_back(std::thread(&ComputationVM::SocketListen, this));
	stlThreadPool.push_back(std::thread(&JobEngine::Dispatch, &m_oEngine));

	for(auto& t: stlThreadPool)
    {
		t.join();
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
    
    StatusMonitor oStatusMonitor("void ComputationVM::SocketListen(void)");
    
    while (false == oStatusMonitor.IsTerminating())
    {
        if (true == m_oSocketServer.WaitForConnection(1000))
        {
            Socket * poNewConnection = m_oSocketServer.Accept();
            this->HandleConnection(poNewConnection);
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
    _in Socket* poSocket
    )
{
    __DebugFunction();
    
    bool bConnectionState = true;
    while(bConnectionState)
    {
        std::vector<Byte> stlContent = GetIpcTransaction(poSocket);
        if(0<stlContent.size())
        {
			StructuredBuffer oContent(stlContent);
			StructuredBuffer oResponse;
			
			std::string strReply;
			unsigned int nType = (unsigned int)oContent.GetInt8("Type");

			switch(nType)
			{
				//TODO: handle failure cases
				case (unsigned int)eQUIT:
					HandleQuit(oContent, oResponse);
					bConnectionState =false;
					Halt();
					break;
				case (unsigned int)eRUN:
					HandleRun(oContent, oResponse);
					break;
                case (unsigned int)eCONNECT:
					HandleConnect(oContent, oResponse);
					break;
				case (unsigned int)eINSPECT:
					HandleInspect(oContent, oResponse);
					break;
				case (unsigned int)eGETTABLE:
					HandleGetTable(oContent, oResponse);
					break;
				case (unsigned int)ePUSHDATA:
					HandlePushData(oContent, oResponse);
					break;
				case (unsigned int)ePULLDATA:
					HandlePullData(oContent, oResponse);
					break;
				case (unsigned int)eDELETEDATA:
					HandleDeleteData(oContent, oResponse);
					break;
				case (unsigned int)ePUSHFN:
					HandlePushFN(oContent, oResponse);
					break;
				default:
					oResponse.PutBoolean("Status", false);
					oResponse.PutString("Payload", "invalid request");
			}
			PutIpcTransaction(poSocket, oResponse);
        }
    }
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleQuit
 * @brief Handle the situation when recieving quit signal from researcher
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleQuit
(
    _in StructuredBuffer& oContent,
    _in StructuredBuffer& oResponse
)
{
    oResponse.PutBoolean("Status", true);
    m_oEngine.Halt();
}

void __thiscall ComputationVM::HandleConnect
(
    _in StructuredBuffer& oContent,
    _in StructuredBuffer& oResponse
)
{
    //__DebugFunction();
    std::string strEOSB = oContent.GetString("EOSB");
    SetEOSB(strEOSB);
    oResponse.PutString("VMID", m_strVMID);
}

void __thiscall ComputationVM::HandleRun
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    std::string strFunctionNode = oContent.GetString("FNID");
    std::string strJobID = oContent.GetString("JobID");
    std::vector<std::string> stlInput = m_stlFNMap[strFunctionNode]->GetInput();
    std::vector<std::string> stlOutput = m_stlFNMap[strFunctionNode]->GetOutput();
    std::vector<std::string> stlConfidentialInput = m_stlFNMap[strFunctionNode]->GetConfidentialInput();
    std::vector<std::string> stlConfidentialOutput = m_stlFNMap[strFunctionNode]->GetConfidentialOutput();

    //StructuredBuffer oAudit;
    //oAudit.PutString("Event", "Run");
    //oAudit.PutString("FNID", strFunctionNode);
    //oAudit.PutString("JobID", strJobID);
    //oAudit.PutString("ClientEOSB", m_strEOSB);

    //m_oRootOfTrustNode.RecordAuditEvent(m_strEOSB,1000,oAudit);

    std::unique_ptr<Job> stlNewJob = std::make_unique<PythonJob>(strFunctionNode, strJobID, stlInput, stlOutput, stlConfidentialInput, stlConfidentialOutput);
    m_oEngine.AddOneJob(std::move(stlNewJob));
    
    oResponse.PutBoolean("Status", true);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleRun
 * @brief Handle the situation when recieving run signal from researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleInspect
 * @brief Handle the inspect request from the researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleInspect
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    std::string strReply;

    strReply = m_oEngine.RetrieveJobs();
    oResponse.PutString("Payload", strReply);
}

void __thiscall ComputationVM::HandleGetTable
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    //ToDo
    std::string strReply;

    strReply = RetrieveDatasets();
    oResponse.PutString("Payload", strReply);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function RetrieveDatasets
 * @brief Get the list of all datasets from the data connector
 *
 ********************************************************************************************/

std::string __thiscall ComputationVM::RetrieveDatasets
(
    void
)
{
    Socket * poSocket =  ConnectToUnixDomainSocket("/tmp/{0bd8a254-49e4-4b86-b1b8-f353c18013c5}");
    StructuredBuffer oRequest;
    
    oRequest.PutInt8("RequestType",eGetTableMetadata);
    oRequest.PutUnsignedInt32("TableID", 0);

    std::vector<Byte> stlResponse = ::PutIpcTransactionAndGetResponse(poSocket, oRequest);
    std::string response;

    if (0 < stlResponse.size())
    {
        StructuredBuffer oResponse(stlResponse);
        StructuredBuffer temp = oResponse.GetStructuredBuffer("ResponseData");
        std::stringstream ss;
        ss<<"Table 0    "<<"Rows: "<<temp.GetInt32("NumberRows")<<"    Cols:"<<temp.GetInt32("NumberColumns")<<std::endl;
        response = ss.str();
    }
    else
    {
        std::cout << "Failed to read response" << std::endl;
    }

    return response;
}


/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleCheck
 * @brief Handle the situation when recieving check signal from researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleCheck
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    std::string strRequest = oContent.GetString("JobID");

    std::string strReply = m_oEngine.GetJobResult(strRequest);
    oResponse.PutString("Payload", strReply);
}

void __thiscall ComputationVM::HandlePushData
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{

    std::vector<std::string> stlVarIDs;
    std::vector<std::vector<Byte>> stlVars;
    std::vector<std::string> stlConfidentialInputIDs;
    std::string strJobID = oContent.GetString("JobID");
    std::string strFNID = oContent.GetString("FNID");
    StructuredBuffer oVarIDs = oContent.GetStructuredBuffer("VarIDs");
    StructuredBuffer oVars = oContent.GetStructuredBuffer("Vars");
    StructuredBuffer oConfidentialInputIDs = oContent.GetStructuredBuffer("ConfidentialInputIDs");

    std::cout<<"pushing data"<<std::endl;

    //StructuredBuffer oAudit;
    //oAudit.PutString("Event", "PushData");
    //oAudit.PutString("JobID", strJobID);
    //oAudit.PutString("ClientEOSB", m_strEOSB);
    
    //m_oRootOfTrustNode.RecordAuditEvent(m_strEOSB,2000, oAudit);

    BufToVec<std::vector<std::vector<Byte>>>(oVars, stlVars);
    BufToVec<std::vector<std::string>>(oVarIDs, stlVarIDs);
    BufToVec<std::vector<std::string>>(oConfidentialInputIDs, stlConfidentialInputIDs);

    std::cout<<"convert to vectors"<<std::endl;

    SaveBuffer(strJobID, stlVarIDs, stlVars);
    std::cout<<"saving data to buffer"<<std::endl;
    LinkPassID(strJobID, strFNID, stlConfidentialInputIDs);
    std::cout<<"linking data"<<std::endl;
    
    oResponse.PutBoolean("Status", true);
}

void __thiscall ComputationVM::SaveBuffer
(
    std::string& strJobID,
    std::vector<std::string>& stlVarIDs,
    std::vector<std::vector<Byte>>& stlVars 
)
{
    size_t nNumber = stlVars.size();
    for(size_t i =0; i<nNumber; i++)
    {
        std::ofstream stlVarFile;
        stlVarFile.open(std::string("/tmp/"+strJobID+stlVarIDs[i]).c_str(), std::ios::out | std::ios::binary);
        stlVarFile.write((char*)&stlVars[i][0], stlVars[i].size());
        stlVarFile.close();
    }
}

void __thiscall ComputationVM::LinkPassID
(
    _in std::string& strJobID,
    _in std::string& strFNID,
    _in std::vector<std::string>& stlPassIDs
)
{
    size_t nNumber = stlPassIDs.size();
    std::vector<std::string> stlConfidentialInputIDs =  m_stlFNMap[strFNID]->GetConfidentialInput();
    for(size_t i =0;i<nNumber;i++)
    {
        std::ofstream stlVarFile;
        std::string strTarget = std::string("/tmp/"+stlPassIDs[i]);
        std::string strLinkpath = std::string("/tmp/"+strJobID+stlConfidentialInputIDs[i]);
        const char* target = strTarget.c_str();
        const char* linkpath = strLinkpath.c_str();
        int res = symlink(target, linkpath);
    }
}

void __thiscall ComputationVM::HandlePullData
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    std::string strJobID = oContent.GetString("JobID");
    StructuredBuffer oVarIDs = oContent.GetStructuredBuffer("VarIDs");

    //StructuredBuffer oAudit;
    //oAudit.PutString("Event", "PullData");
    //oAudit.PutString("JobID", strJobID);
    //oAudit.PutString("ClientEOSB", m_strEOSB);

    //m_oRootOfTrustNode.RecordAuditEvent(m_strEOSB,3000, oAudit);

    std::vector<std::string> stlVarIDs;

    BufToVec<std::vector<std::string>>(oVarIDs, stlVarIDs);
    std::vector<std::vector<Byte>> stlOutputs;

    LoadDataToBuffer(strJobID, stlVarIDs, stlOutputs);

    StructuredBuffer oVarBuffer;
    VecToBuf<std::vector<std::vector<Byte>>>(stlOutputs, oVarBuffer);
    
    //std::cout<<oVarBuffer.GetInt16("size")<<std::endl;
    oResponse.PutStructuredBuffer("Vars", oVarBuffer);
}


void __thiscall ComputationVM::LoadDataToBuffer
(
    _in std::string& strJobID,
    _in std::vector<std::string>& stlVarIDs,
    _in std::vector<std::vector<Byte>>& stlVars
)
{
    size_t nNumber = stlVarIDs.size();
    for(size_t i=0; i<nNumber; i++)
    {
        while(access(std::string("/tmp/"+strJobID+stlVarIDs[i]).c_str(), F_OK)!=0||m_oEngine.PeekStatus(strJobID)!=eCompleted)   
        {
            //std::cout<<"filename:"<<"/tmp/"+strJobID+stlVarIDs[i]<<" waiting for file"<<std::endl;
        }
        
        std::ifstream stlVarFile;
        stlVarFile.open(std::string("/tmp/"+strJobID+stlVarIDs[i]).c_str(), std::ios::out | std::ios::binary);
        stlVarFile.unsetf(std::ios::skipws);
        
        std::vector<Byte> stlVec;
        stlVec.insert(stlVec.begin(),std::istream_iterator<Byte>(stlVarFile), std::istream_iterator<Byte>());
        stlVars.push_back(stlVec);
        
        std::cout<<"vector length: "<<stlVec.size()<<std::endl;
        
        stlVarFile.close();
    }
}

void __thiscall ComputationVM::HandleDeleteData
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    //TODO
    std::vector<std::string> stlVars;
    StructuredBuffer oVars = oContent.GetStructuredBuffer("Vars");

    BufToVec(oVars, stlVars);

    for(size_t i=0;i<stlVars.size();i++)
    {
        std::remove(std::string("/tmp/"+stlVars[i]).c_str());
    }
    
    oResponse.PutBoolean("Status", true);
}

void __thiscall ComputationVM::HandlePushFN
(
    _in StructuredBuffer& oContent, 
    _in StructuredBuffer& oResponse
)
{
    std::string strFNID = oContent.GetString("FNID");
    std::string strFNScript = oContent.GetString("FNScript");

    //StructuredBuffer oAudit;
    //std::string strType("PushFN");
    //oAudit.PutString("Event", strType);
    //oAudit.PutString("FNID", strFNID);
    //oAudit.PutString("ClientEOSB", m_strEOSB);
    
    //m_oRootOfTrustNode.RecordAuditEvent(m_strEOSB, 4000, oAudit);

    SaveFN(strFNID, strFNScript);

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

    std::unique_ptr<FunctionNode> stlFN = std::make_unique<FunctionNode>(stlInputIDs, stlOutputIDs, stlConfidentialInputIDs, stlConfidentialOutputIDs, strFNID);
    m_stlFNMap.insert({strFNID,std::move(stlFN)});
    
    oResponse.PutBoolean("Status", true);
}

void __thiscall ComputationVM::SaveFN
(
    std::string& strFNID,
    std::string& strFNScript
)
{
    std::ofstream stlFNFile;
    stlFNFile.open("/tmp/"+strFNID);
    stlFNFile<<strFNScript;
    stlFNFile.close();
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
    m_fStop = true;
}

void __thiscall ComputationVM::SetEOSB(std::string& strEOSB)
{
    __DebugFunction();
    m_strEOSB = strEOSB;
}
