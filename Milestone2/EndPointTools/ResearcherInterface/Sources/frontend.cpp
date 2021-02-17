/*********************************************************************************************
 *
 * @file frontend.cpp
 * @author Jingwei Zhang
 * @date 15 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used for single job related issues
 *
 ********************************************************************************************/

#include "frontend.h"
#include "utils.h"
#include "StructuredBuffer.h"
#include "SocketClient.h"
#include "IpcTransactionHelperFunctions.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>

__thiscall Frontend::Frontend(void):
    m_stlConnectionMap(),
    m_stlFNTable(),
    m_fStop(false)
{

}

/********************************************************************************************
 *
 * @class Frontend
 * @function Frontend
 * @param [in] strServerIP the IP address of the server to connect to
 * @param [in] wPort the port to connect to the server
 * @brief Create a frontendCLI object
 *
 ********************************************************************************************/

void __thiscall Frontend::SetFrontend
(
    _in std::string& strServerIP, 
    _in Word wPort,
    _in std::string& strVMID
)
{
    Socket* poSocket = ConnectToNetworkSocket(strServerIP, wPort);
    std::shared_ptr<Socket>stlSocket=std::shared_ptr<Socket>(poSocket);

    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eCONNECT);

    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(stlSocket.get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);

    strVMID = oResponse.GetString("VMID");

    m_stlConnectionMap.emplace(strVMID, stlSocket);
}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function HandleRun
 * @param [in] oBuffer Reference of the structured buffer going to be send
 * @param [in] argMap Reference of the user input argument map
 * @brief Prepare the structured buffer going to be send for running a job
 *
 ********************************************************************************************/

void __thiscall Frontend::HandleExecJob
(
    _in std::string& strVMID,
    _in std::string& strFNID,
    _in std::string& strJobID
)
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eRUN);
    oBuffer.PutString("FNID", strFNID);
    oBuffer.PutString("JobID", strJobID);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    //StructuredBuffer oResponse(stlResponse);
}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function HandleCheck
 * @param [in] oBuffer Reference of the structured buffer going to be send
 * @param [in] argMap Reference of the user input argument map
 * @brief Prepare the structured buffer going to be send for checking a job result
 *
 ********************************************************************************************/

void __thiscall Frontend::HandleInspect
(
    _in std::string& strVMID,
    _inout std::string& strJobs
)
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eINSPECT);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);
    
    strJobs = oResponse.GetString("Payload");
}

void __thiscall Frontend::HandleGetTable
(
    _in std::string& strVMID,
    _in std::string& strTables
)
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eGETTABLE);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);

    strTables = oResponse.GetString("Payload");

}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function HandleQuit
 * @param [in] oBuffer Reference of the structured buffer going to be send
 * @brief Prepare the structured buffer going to be send for quitting
 * 
 ********************************************************************************************/

void __thiscall Frontend::HandleQuit
(
    void
)
{

    StructuredBuffer oBuffer;

    oBuffer.PutInt8("Type", eQUIT);
    
    for(auto const& i : m_stlConnectionMap)
    {
        std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[i.first].get(), oBuffer);
        //StructuredBuffer oResponse(stlResponse);
    }

    m_fStop = true;
    std::cout<<"Quit success"<<std::endl;
}

void __thiscall Frontend::HandlePushData
(
    _in std::string& strVMID,
    _in std::string& strJobID,
    _in std::vector<std::string>& stlvarID,
    _in std::vector<std::vector<Byte>>& stlVars   
)
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePUSHDATA);
    oBuffer.PutString("JobID", strJobID);
    
    StructuredBuffer oVarIDs;
    StructuredBuffer oVars;
    VecToBuf<std::vector<std::string>>(stlvarID, oVarIDs);
    VecToBuf<std::vector<std::vector<Byte>>>(stlVars, oVars);
    oBuffer.PutStructuredBuffer("VarIDs", oVarIDs);
    oBuffer.PutStructuredBuffer("Vars", oVars);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    //StructuredBuffer oResponse(stlResponse);
}

void __thiscall Frontend::GetInputVec
(
    _in std::string& strFNID,
    _inout std::vector<std::string>& stlVarIDs
)
{
    stlVarIDs = m_stlFNTable[strFNID]->GetInput();
}

void __thiscall Frontend::GetOutputVec
(
    _in std::string& strFNID,
    _inout std::vector<std::string>& stlVarIDs
)
{
    stlVarIDs = m_stlFNTable[strFNID]->GetOutput();
}

void __thiscall Frontend::HandlePullData
(
    _in std::string& strVMID,
    _in std::string& strJobID,
    _in std::vector<std::string>& stlvarIDs,
    _inout std::vector<std::vector<Byte>>& stlVars   
)
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePULLDATA);
    oBuffer.PutString("JobID", strJobID);
    
    StructuredBuffer oVarIDs;
    VecToBuf<std::vector<std::string>>(stlvarIDs, oVarIDs);
    oBuffer.PutStructuredBuffer("VarIDs", oVarIDs);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);
    StructuredBuffer oVars = oResponse.GetStructuredBuffer("Vars");
    BufToVec<std::vector<std::vector<Byte>>>(oVars, stlVars);
}

void __thiscall Frontend::HandleDeleteData
(
    _in std::string& strVMID,
    _in std::vector<std::string>& stlvarID   
)
{
    //TODO
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", eDELETEDATA);
    oBuffer.PutString("VMID", strVMID);
    
    StructuredBuffer oVars;
    VecToBuf<std::vector<std::string>>(stlvarID, oVars);
    oBuffer.PutStructuredBuffer("Vars", oVars);
    
    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);
}

void __thiscall Frontend::HandlePushFN
(
    _in std::string& strVMID,
    _in std::string& strFNID
)
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePUSHFN);
    
    oBuffer.PutString("FNID", m_stlFNTable[strFNID]->GetFNID());
    oBuffer.PutString("FNScript", m_stlFNTable[strFNID]->GetScript());
    
    std::vector<std::string> stlInputIDs = m_stlFNTable[strFNID]->GetInput(); 
    std::vector<std::string> stlOutputIDs = m_stlFNTable[strFNID]->GetOutput(); 
    StructuredBuffer oInputBuffer;
    StructuredBuffer oOutputBuffer;

    VecToBuf<std::vector<std::string>>(stlInputIDs, oInputBuffer);
    VecToBuf<std::vector<std::string>>(stlOutputIDs, oOutputBuffer);
    
    oBuffer.PutStructuredBuffer("Input", oInputBuffer);
    oBuffer.PutStructuredBuffer("Output", oOutputBuffer);

    std::vector<Byte> stlResponse = PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
    StructuredBuffer oResponse(stlResponse);
}

void __thiscall Frontend::RegisterFN
(
    _in std::string strFilePath,
    _in int nInputNumber,
    _in int nOutputNumber,
    _inout std::string& strFNID
)
{
    std::unique_ptr stlFNPointer = std::make_unique<FunctionNode>(nInputNumber, nOutputNumber, strFilePath);
    strFNID = stlFNPointer->GetFNID();
    m_stlFNTable.emplace(strFNID, std::move(stlFNPointer));
}
