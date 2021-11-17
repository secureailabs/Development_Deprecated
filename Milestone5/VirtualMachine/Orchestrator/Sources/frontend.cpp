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
#include "StructuredBuffer.h"
#include "SocketClient.h"
#include "TlsClient.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "TlsTransactionHelperFunctions.h"
#include "HttpRequestParser.h"
#include "CurlRest.h"
#include "JsonValue.h"
#include <exception>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <filesystem>
#include <chrono>
#include <thread>

#ifndef SERVER_IP_ADDRESS
    #define SERVER_PORT 6200
    #define SERVER_IP_ADDRESS "20.185.6.111"
#endif

Frontend::Frontend(void):
    m_stlConnectionMap(),
    m_stlJobStatusMap(),
    m_stlDataTableMap(),
    m_stlFNTable(),
    m_stlResultSet(),
    m_fStop(false)
{
    __DebugFunction();
}

Frontend::~Frontend(void)
{
    __DebugFunction();

}

/********************************************************************************************/

std::string Frontend::Login(
    _in const std::string & c_strEmail,
    _in const std::string & c_strUserPassword
    )
{
    __DebugFunction();
    //__DebugAssert(0 < strlen(g_szServerIpAddress));
    //__DebugAssert(0 != g_unPortNumber);
    __DebugAssert(0 < c_strEmail.size());
    __DebugAssert(0 < c_strUserPassword.size());

    std::string strEosb;

    try
    {
        std::string strVerb = "POST";
        std::string strApiUrl = "/SAIL/AuthenticationManager/User/Login?Email="+ c_strEmail +"&Password="+ c_strUserPassword;
        std::string strJsonBody = "";
        // Make the API call and get REST response
        std::vector<Byte> stlRestResponse = ::RestApiCall(SERVER_IP_ADDRESS, SERVER_PORT, strVerb, strApiUrl, strJsonBody, true);
        std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
        StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error logging in.", nullptr);
        strEosb = oResponse.GetString("Eosb");
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    m_strEOSB = strEosb;
    m_strUsername = c_strEmail; 
    return strEosb;
}

void __thiscall Frontend::Listener(
    _in std::string strVMID
)
{
    __DebugFunction();

    while (!m_fStop)
    {
        try
        {
            std::cout << "Waiting for data" << std::endl;
            fflush(stdout);
            std::vector<Byte> stlResponseBuffer = ::GetTlsTransaction(m_stlConnectionMap[strVMID].get(), 0);
            _ThrowBaseExceptionIf((0 == stlResponseBuffer.size()), "No data received.", nullptr);

            StructuredBuffer oResponse(stlResponseBuffer);
            JobStatusSignals eStatusSignalType = (JobStatusSignals)oResponse.GetByte("SignalType");

            switch(eStatusSignalType)
            {
                case JobStatusSignals::eJobStart:
                {
                    std::string strJobID = oResponse.GetString("JobUuid");
                    std::lock_guard<std::mutex> lock(m_stlJobStatusMapMutex);
                    //m_stlJobStatusMap.emplace(strJobID, JobStatusSignals::eJobStart);
                    m_stlJobStatusMap[strJobID] = JobStatusSignals::eJobStart;
                    break;
                }
                case JobStatusSignals::eJobDone:
                {
                    std::string strJobID = oResponse.GetString("JobUuid");
                    std::lock_guard<std::mutex> lock(m_stlJobStatusMapMutex);
                    m_stlJobStatusMap[strJobID] = JobStatusSignals::eJobDone;
                    std::cout<<"Job status of: "<<strJobID<<" set to: "<<(int)m_stlJobStatusMap[strJobID]<<std::endl;
                    break;
                }
                case JobStatusSignals::eJobFail:
                {
                    std::string strJobID = oResponse.GetString("JobUuid");
                    std::lock_guard<std::mutex> lock(m_stlJobStatusMapMutex);
                    m_stlJobStatusMap[strJobID] = JobStatusSignals::eJobFail;
                    break;
                }
                case JobStatusSignals::ePostValue:
                {
                    std::vector<Byte> stlData = oResponse.GetBuffer("FileData");
                    std::string strDataID = oResponse.GetString("ValueName");
                    std::cout<<"Get result posted: "<<strDataID<<" Data length: "<<stlData.size()<<std::endl;
                    //std::lock_guard<std::mutex> lock(m_stlResultMapMutex);
                    //m_stlResultMap.emplace(strDataID, stlData);
                    //m_stlResultMap[strDataID] = stlData;
                    SaveBuffer(strDataID, stlData);
                    m_stlResultSet.emplace(strDataID);
                    break;
                }
                case JobStatusSignals::eHeartBeatPing:
                {
                    // Send a response to the JobEngine to keep the connection alive
                    StructuredBuffer oResponse;
                    oResponse.PutString("EndPoint", "JobEngine");
                    oResponse.PutByte("RequestType", (Byte)EngineRequest::eHeartBeatPong);
                    this->SendDataToJobEngine(strVMID, oResponse);
                    break;
                }
                case JobStatusSignals::eVmShutdown: break;
                default: break;
            }
        }

        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        }
    }

    m_stlConnectionMap[strVMID].get()->Release();
    // for(auto const& i: m_stlConnectionMap)
    // {
    //     i.second->Release();
    // }
}

void __thiscall Frontend::SaveBuffer(
    std::string& strDataID,
    std::vector<Byte>& stlVars
    )
{
    std::ofstream stlVarFile;
    std::string strFilename = std::string("/tmp/"+strDataID);
    stlVarFile.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    stlVarFile.write((char*)&stlVars[0], stlVars.size());
    stlVarFile.close();
    std::cout<<"writing data file: "<<strDataID<<std::endl;
}


void __thiscall Frontend::SetFrontend(
    _in std::string & strServerIP,
    _in Word wPort,
    _in std::string & strVMID
    )
{
    //if(m_strEOSB.empty())
    //{
    //    m_strEOSB = Login(strEmail, strPassword);
    //    std::cout<<"Login done"<<std::endl;
    //    std::cout<<"EOSB:"<<m_strEOSB<<std::endl;
    //}
    
    StructuredBuffer oBuffer;
    oBuffer.PutByte("RequestType", (Byte)EngineRequest::eConnectVirtualMachine);
    oBuffer.PutString("Eosb", m_strEOSB);
    oBuffer.PutString("Username", m_strUsername);

    TlsNode * poSocket = nullptr;

    try
    {
        if(m_stlConnectionMap.end()==m_stlConnectionMap.find(strVMID)){
            poSocket = ::TlsConnectToNetworkSocket(strServerIP.c_str(), wPort);
            _ThrowIfNull(poSocket, "Tls connection error for connectVM", nullptr);
            std::cout<<"connect to "<<strServerIP<<" successful"<<std::endl;

            oBuffer.PutString("EndPoint", "JobEngine");
            PutTlsTransaction(poSocket, oBuffer.GetSerializedBuffer());
            //std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket, oBuffer, 2*60*1000);
            //if(0==stlResponse.size())
            //    _ThrowBaseException("No response for connectVM request", nullptr);

            auto stlResponse = GetTlsTransaction(poSocket, 0);
            _ThrowBaseExceptionIf((0 == stlResponse.size()), "No data received.", nullptr);

            StructuredBuffer oResponse(stlResponse);
            strVMID = oResponse.GetString("VirtualMachineUuid");

            std::map<std::string, std::string> stlDataMap;
            StructuredBuffer oDataset = oResponse.GetStructuredBuffer("Dataset");
            StructuredBuffer oTables = oDataset.GetStructuredBuffer("Tables");
            std::vector<std::string> stlNameList = oTables.GetNamesOfElements();
            
            for(size_t i = 0 ; i<stlNameList.size(); i++)
            {
                //stlDataGuid.push_back(oDataset.GetStructuredBuffer("Tables").GetString(stlNameList[i].c_str()));
                stlDataMap.emplace(stlNameList[i], oDataset.GetStructuredBuffer("Tables").GetString(stlNameList[i].c_str()));
            }

            m_stlDataTableMap.emplace(strVMID, stlDataMap);

            std::shared_ptr<TlsNode> stlSocket(poSocket);
            m_stlConnectionMap.emplace(strVMID, stlSocket);

            // Create and insert a mutex to m_stlConnectionMutexMap for this strVmID
            std::shared_ptr<std::mutex> stlMutex(new std::mutex);
            m_stlConnectionMutexMap.insert(std::make_pair(strVMID, stlMutex));
        }
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    std::thread(&Frontend::Listener, this, strVMID).detach();
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

void __thiscall Frontend::HandleSubmitJob(
    _in std::string & strVMID,
    _in std::string & strFNID,
    _in std::string & strJobID
    )
{
    std::cout<<"submit job: "<<strJobID<<std::endl;
    StructuredBuffer oBuffer;
    oBuffer.PutString("EndPoint", "JobEngine");
    oBuffer.PutByte("RequestType", (Byte)EngineRequest::eSubmitJob);
    oBuffer.PutString("SafeObjectUuid", strFNID);
    oBuffer.PutString("JobUuid", strJobID);

    try
    {
        this->SendDataToJobEngine(strVMID, oBuffer);
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
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

// void __thiscall Frontend::HandleInspect(
//     _in std::string & strVMID,
//     _inout std::string & strJobs
//     )
// {
//     StructuredBuffer oBuffer;
//     oBuffer.PutInt8("Type", eINSPECT);

//     TlsNode * poSocket =nullptr;
//     try
//     {
//         poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
//         _ThrowIfNull(poSocket, "Tls connection error for inspect request", nullptr);
//         std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
//         if(0==stlResponse.size())
//             _ThrowBaseException("No response for inspect request", nullptr);
        
//         StructuredBuffer oResponse(stlResponse);
//         strJobs = oResponse.GetString("Payload");
//         poSocket->Release();
//         poSocket = nullptr;
//     }
    
//     catch(BaseException oBaseException)
//     {
//         ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __FILE__, __LINE__);
//     }
    
//     if (nullptr != poSocket)
//     {
//         poSocket->Release();
//         poSocket = nullptr;
//     }
// }

// void __thiscall Frontend::HandleGetTable(
//     _in std::string & strVMID,
//     _in std::string & strTables
//     )
// {
//     StructuredBuffer oBuffer;
//     oBuffer.PutInt8("Type", eGETTABLE);
    
//     TlsNode * poSocket = nullptr;
    
//     try
//     {
//         poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
//         _ThrowIfNull(poSocket, "Tls connection error for getTable", nullptr);
//         std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
//         if(0==stlResponse.size())
//             _ThrowBaseException("No response for getTable request", nullptr);

//         StructuredBuffer oResponse(stlResponse);
//         strTables = oResponse.GetString("Payload");
//         poSocket->Release();
//         poSocket = nullptr;
//     }
    
//     catch(BaseException oBaseException)
//     {
//         ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __FILE__, __LINE__);
//     }
    
//     if (nullptr != poSocket)
//     {
//         poSocket->Release();
//         poSocket = nullptr;
//     }
// }

/********************************************************************************************
 *
 * @class EngineCLI
 * @function HandleQuit
 * @param [in] oBuffer Reference of the structured buffer going to be send
 * @brief Prepare the structured buffer going to be send for quitting
 *
 ********************************************************************************************/

void __thiscall Frontend::HandleQuit(void)
{
    StructuredBuffer oBuffer;

    oBuffer.PutString("EndPoint", "JobEngine");
    oBuffer.PutByte("RequestType", (Byte)EngineRequest::eVmShutdown);
    
    for(auto const& i : m_stlConnectionMap)
    {
        try
        {
            this->SendDataToJobEngine(i.first, oBuffer);
        }

        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        }
    }

    std::lock_guard<std::mutex> lock(m_stlFlagMutex);
    m_fStop = true;
}

void __thiscall Frontend::HandlePushData(
    _in std::string & strVMID,
    _in std::vector<std::string>& stlInputIds,
    _in std::vector<std::vector<Byte>> & stlInputVars
    )
{
    std::cout<<"push data: "<<std::endl;
    for(size_t i=0; i<stlInputIds.size(); i++)
    {
        StructuredBuffer oBuffer;
    
        oBuffer.PutByte("RequestType", (Byte)EngineRequest::ePushdata);
        oBuffer.PutString("EndPoint", "JobEngine");
        oBuffer.PutString("DataId", stlInputIds[i]);
        std::cout<<"dataid: "<<stlInputIds[i]<<"length: "<<stlInputVars[i].size()<<std::endl;
        oBuffer.PutBuffer("Data", stlInputVars[i]);

        try
        {
            this->SendDataToJobEngine(strVMID, oBuffer);
        }

        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        }
    }  
}

//do not know the purpose of "ValuesExpected" and "ValueIndex", disable?
void __thiscall Frontend::HandleSetParameters(
    _in std::string& strVMID, 
    _in std::string& strFNID, 
    _in std::string& strJobID, 
    _in std::vector<std::string>stlParams
    )
{
    std::vector<std::string> stlInputIds = m_stlFNTable[strFNID]->GetInput();
    std::cout<<"set parameter: "<<strJobID<<std::endl;

    for(size_t i=0; i<stlParams.size(); i++)
    {
        StructuredBuffer oBuffer;
        oBuffer.PutByte("RequestType", (Byte)EngineRequest::eSetParameters);
        oBuffer.PutString("EndPoint", "JobEngine");
        oBuffer.PutString("JobUuid", strJobID);
        oBuffer.PutString("ParameterUuid", stlInputIds[i]);
        oBuffer.PutString("ValueUuid", stlParams[i]);
        oBuffer.PutUnsignedInt32("ValuesExpected", 1);
        oBuffer.PutUnsignedInt32("ValueIndex", 0);
        
        try
        {
            this->SendDataToJobEngine(strVMID, oBuffer);
        }

        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        }

    }
}

void __thiscall Frontend::HandlePullData(
    _in std::string & strVMID,
    _in std::string & strJobID,
    _in std::string & strFNID
    )
{
    std::cout<<"pull data: "<<strJobID<<std::endl;
    std::vector<std::string> stlOutputIDs = m_stlFNTable[strFNID]->GetOutput();
    std::vector<std::string> stlOutputConf = m_stlFNTable[strFNID]->GetOutputConfidential();

    for(size_t i=0; i<stlOutputIDs.size(); i++)
    {
        if(stlOutputConf[i].compare("0")==0)
        {
            StructuredBuffer oBuffer;
            std::string strOutputFilename = strJobID + "." + stlOutputIDs[i];
        
            oBuffer.PutByte("RequestType", (Byte)EngineRequest::ePullData);
            oBuffer.PutString("EndPoint", "JobEngine");
            oBuffer.PutString("Filename", strOutputFilename);
            
            try
            {
                this->SendDataToJobEngine(strVMID, oBuffer);
                std::cout<<"pull data for: "<<strOutputFilename<<std::endl;
            }
            
            catch(BaseException oBaseException)
            {
                ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
            }

            catch(...)
            {
                ::RegisterUnknownException(__func__, __FILE__, __LINE__);
            }
        }
    }
}

void __thiscall Frontend::QueryResult(
    _in std::string& strJobID,
    _in std::string& strFNID,
    _inout std::map<std::string, int>& stlOutput
)
{
    std::cout<<"query result: "<<strJobID<<std::endl;
    std::vector<std::string> stlOutputID = m_stlFNTable[strFNID]->GetOutput();
    std::vector<std::string> stlOutputConf = m_stlFNTable[strFNID]->GetOutputConfidential();

    for(size_t i =0; i<stlOutputID.size(); i++)
    {
    
        std::string strDataID = strJobID + "." + stlOutputID[i];

        std::cout<<"query result data id: "<<strDataID<<std::endl;

        if(stlOutputConf[i].compare("0")==0)
        {
            while(m_stlResultSet.end()==m_stlResultSet.find(strDataID))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                std::cout<<"waiting for result: "<<strDataID<<std::endl;
            }
            stlOutput.emplace(strDataID, 0);
        }
        else
        {
            stlOutput.emplace(strDataID, 1);
        }
    }
}

JobStatusSignals __thiscall Frontend::QueryJobStatus(
    _in std::string& strJobID
)
{
    //std::cout<<"query job status: "<<strJobID<<" : "<<m_stlJobStatusMap[strJobID]<<std::endl;
    std::lock_guard<std::mutex> lock(m_stlJobStatusMapMutex);
    return m_stlJobStatusMap[strJobID];
}

std::map<std::string, std::string> __thiscall Frontend::QueryDataset(
    _in std::string& strVMID
)
{
    return m_stlDataTableMap[strVMID];
}

// void __thiscall Frontend::HandleDeleteData(
//     _in std::string & strVMID,
//     _in std::vector<std::string> & stlvarID   
//     )
// {
//     //TODO
//     StructuredBuffer oBuffer;
    
//     oBuffer.PutInt8("Type", eDELETEDATA);
//     oBuffer.PutString("VMID", strVMID);
    
//     StructuredBuffer oVars;
//     VecToBuf<std::vector<std::string>>(stlvarID, oVars);
//     oBuffer.PutStructuredBuffer("Vars", oVars);
    
//     TlsNode * poSocket = nullptr;
    
//     try
//     {
//         poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
//         _ThrowIfNull(poSocket, "Tls connection error for DeleteData", nullptr);
//         std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
//         // Make sure to release the socket
//         poSocket->Release();
//         poSocket = nullptr;
//     }
    
//     catch(BaseException oBaseException)
//     {
//         ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __FILE__, __LINE__);
//     }

//     if (nullptr != poSocket)
//     {
//         poSocket->Release();
//         poSocket = nullptr;
//     }
// }

//msg field
//"SafeObjectUuid"
//"Payload"
//"InputParameterList"
//"OutputParameterList"
//The last two to substitue "ParameterList", need to have a module in structuredbuffer to handle vector of string.

void __thiscall Frontend::HandlePushSafeObject(
    _in std::string & strVMID,
    _in std::string & strFNID
    )
{
    std::cout<<"push safe object: "<<strFNID<<std::endl;
    StructuredBuffer oBuffer;

    oBuffer.PutByte("RequestType", (Byte)EngineRequest::ePushSafeObject);
    oBuffer.PutString("EndPoint", "JobEngine");

    oBuffer.PutString("SafeObjectUuid", strFNID);
    oBuffer.PutString("Title", m_stlFNTable[strFNID]->GetTitle());
    oBuffer.PutString("Description", m_stlFNTable[strFNID]->GetDescription());

    std::string strCode =  m_stlFNTable[strFNID]->GetScript();
    oBuffer.PutBuffer("Payload", (Byte*)strCode.c_str(), strCode.size());

    std::vector<std::string> stlInputIDs = m_stlFNTable[strFNID]->GetInput();
    std::vector<std::string> stlOutputIDs = m_stlFNTable[strFNID]->GetOutput();

    StructuredBuffer oInputParams;
    for(size_t i = 0; i<stlInputIDs.size(); i++)
    {
        StructuredBuffer oInputElement;
        oInputElement.PutString("Uuid", stlInputIDs[i]);
        //oInputElement.PutString("Description", "");
        oInputParams.PutStructuredBuffer(std::to_string(i).c_str(), oInputElement);
    }
    oBuffer.PutStructuredBuffer("ParameterList", oInputParams);

    StructuredBuffer oOutputParams;
    for(size_t i = 0; i<stlOutputIDs.size(); i++)
    {
        StructuredBuffer oOutputElement;
        oOutputElement.PutString("Uuid", stlOutputIDs[i]);
        //oInputElement.PutString("Description", "");
        oOutputParams.PutStructuredBuffer(std::to_string(i).c_str(), oOutputElement);
    }
    oBuffer.PutStructuredBuffer("OutputParameters", oOutputParams);

    try
    {
        this->SendDataToJobEngine(strVMID, oBuffer);
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

void __thiscall Frontend::RegisterSafeObject(
    _in std::string& strFilePath
    )
{
    for (const auto & entry : std::filesystem::directory_iterator(strFilePath))
    {
        std::cout<<"start"<<std::endl;
        std::string strFilename = entry.path().string();
        std::cout<<strFilename<<std::endl;
        std::ifstream stlFNFile;
        stlFNFile.open(strFilename, (std::ios::in | std::ios::binary | std::ios::ate));
        unsigned int unFileSizeInBytes = (unsigned int)stlFNFile.tellg();
        std::cout<<unFileSizeInBytes<<std::endl;
        std::vector<Byte> stlBuffer;
        stlBuffer.resize(unFileSizeInBytes);
        //stlFNFile.unsetf(std::ios::skipws);
        stlFNFile.seekg(0, std::ios::beg);
        stlFNFile.read((char*)stlBuffer.data(), unFileSizeInBytes);
        //stlBuffer.insert(stlBuffer.begin(),std::istream_iterator<Byte>(stlFNFile), std::istream_iterator<Byte>());
        stlFNFile.close();

        std::cout<<"file read done"<<std::endl;
        StructuredBuffer oSafeObject(stlBuffer);
        std::cout<<"safe obj created"<<std::endl;
        std::unique_ptr<SafeObject> poFN = std::make_unique<SafeObject>(oSafeObject);
        std::string strFNID = poFN->GetSafeObjectID();
        std::cout<<strFNID<<" created"<<std::endl;
        m_stlFNTable.emplace(strFNID, std::move(poFN));
        std::cout<<"end"<<std::endl;
    }
}


/********************************************************************************************
 *
 * @class Frontend
 * @function SendDataToJobEngine
 * @param [in] strVMID Id of the VM to send the data to to
 * @param [in] c_oStructuredBuffer StructuredBUffer to send
 * @brief Prepare the structured buffer going to be send for quitting
 *
 ********************************************************************************************/
void __thiscall Frontend::SendDataToJobEngine(
    _in const std::string& strVMID,
    _in StructuredBuffer & c_oStructuredBuffer
)
{
    __DebugFunction();

    try
    {
        std::lock_guard<std::mutex> lock(*m_stlConnectionMutexMap.at(strVMID).get());
        ::PutTlsTransaction(m_stlConnectionMap[strVMID].get(), c_oStructuredBuffer);
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(std::exception & e)
    {
        std::cout << "Exception: " << e.what() << '\n';
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}

