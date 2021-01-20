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
#include "Guid.h"
#include <thread>
#include <vector>
#include <iostream>
#include <sstream>

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
    _in size_t nMaxProcess
    )
    : m_oEngine(nMaxProcess), m_oSocketServer(wPortIdentifier), m_fStop(false)
{
    
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
    bool bConnectionState = true;
    while(bConnectionState)
    {
        //message structure
        //|-------------------------------------|---------------|
        //|----Header:PayLoadLength(4 bytes)----|----PayLoad----|
        //|-------------------------------------|---------------|
    	std::vector<Byte> stlHeader;
    	while(0==stlHeader.size())
    	{
            stlHeader = poSocket->Read(HEADERLENGTH, 1000);
    	}

        size_t nPayloadLength = stlHeader[0]*256 + stlHeader[1];

        std::vector<Byte> stlContent = poSocket->Read(nPayloadLength, 1000);

        StructuredBuffer oContent(stlContent);
        StructuredBuffer oResponse;
        std::string strReply;

        switch((RequestType)(oContent.GetInt8("Type")))
        {
            //TODO: handle failure cases
            case eQUIT:
                HandleQuit();
                oResponse.PutBoolean("Status", true);
                bConnectionState =false;
                Halt();
                break;
            case eRUN:   
                strReply = HandleRun(oContent.GetString("FunctionNodeNumber"), oContent.GetString("InputNodeNumber"));
                oResponse.PutBoolean("Status", true);
                oResponse.PutString("Payload", strReply);
                break;
            case eCHECK: 
                strReply = HandleCheck(oContent.GetString("JobName"));
                oResponse.PutBoolean("Status", true);
                oResponse.PutString("Payload", strReply);
                break;
            case eINSPECT:
                strReply = HandleInspect(oContent.GetString("InspectTarget"));
                oResponse.PutBoolean("Status", true);
                oResponse.PutString("Payload", strReply);
                break;
            default:
                oResponse.PutBoolean("Status", false);
                oResponse.PutString("Payload", "invalid request");
        }

        std::vector<Byte> stlResponseHeader;
        stlResponseHeader.push_back(oResponse.GetSerializedBufferRawDataSizeInBytes()/256);
        stlResponseHeader.push_back(oResponse.GetSerializedBufferRawDataSizeInBytes()%256);
        poSocket->Write(stlResponseHeader.data(), stlResponseHeader.size());
        poSocket->Write(oResponse.GetSerializedBufferRawDataPtr(), oResponse.GetSerializedBufferRawDataSizeInBytes());
    }
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleQuit
 * @brief Handle the situation when recieving quit signal from researcher
 *
 ********************************************************************************************/

void __thiscall ComputationVM::HandleQuit(void)
{
    m_oEngine.Halt();
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleRun
 * @brief Handle the situation when recieving run signal from researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

std::string __thiscall ComputationVM::HandleRun(
    _in std::string strFunctionNodeNumber,
    _in std::string strInputNodeNumber
    )
{
    Guid oJobID;
    std::unique_ptr<Job> stlNewJob = std::make_unique<PythonJob>(Guid(strFunctionNodeNumber.c_str()), eScript, oJobID, Guid(strInputNodeNumber.c_str()));
    m_oEngine.AddOneJob(std::move(stlNewJob));
    return oJobID.ToString(eRaw);
}

/********************************************************************************************
 *
 * @class ComputationVM
 * @function HandleInspect
 * @brief Handle the inspect request from the researcher
 * @param[in] strRequest The request send by resercher containing the information of the job
 *
 ********************************************************************************************/

std::string __thiscall ComputationVM::HandleInspect
(
    _in std::string strRequest
)
{
    std::string strReply;
    if(!strRequest.compare("job"))
    {
        strReply = m_oEngine.RetrieveJobs();
    }
    else if(!strRequest.compare("dataset"))
    {
        strReply = RetrieveDatasets();
    }
    return strReply;
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

std::string __thiscall ComputationVM::HandleCheck(
    _in std::string strRequest
    )
{
    return m_oEngine.GetJobResult(strRequest);
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
