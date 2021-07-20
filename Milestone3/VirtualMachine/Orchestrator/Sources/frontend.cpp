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

#ifndef SERVER_IP_ADDRESS
    #define SERVER_PORT 6200
    #define SERVER_IP_ADDRESS "137.116.90.145"
#endif

Frontend::Frontend(void):
    m_stlConnectionMap(),
    m_stlJobStatusMap(),
    m_stlDataTableMap(),
    m_stlFNTable(),
    m_stlResultMap(),
    m_fStop(false)
{
    __DebugFunction();
}

Frontend::~Frontend(void)
{
    __DebugFunction();

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

bool ParseFirstLine(
    _in const std::string & c_strRequestData
    )
{
    __DebugFunction();

    bool fSuccess = false;
    std::string strProtocol, strStatus;
    std::stringstream oFirstLineStream(c_strRequestData);

    // Get transaction status
    std::getline(oFirstLineStream, strProtocol, ' ');
    std::getline(oFirstLineStream, strStatus, ' ');

    if (!strStatus.empty())
    {
        fSuccess = true;
    }
    else
    {
        _ThrowBaseException("ERROR: Invalid request.", nullptr);
    }
    _ThrowBaseExceptionIf(("200" != strStatus), "Transaction returned with error code.", nullptr);

    return fSuccess;
}

/********************************************************************************************
 *
 * @function GetResponseBody
 * @brief Parse and return response body
 * @param[in] c_strRequestData response data
 * @return Serialized response body
 *
 ********************************************************************************************/

std::vector<Byte> GetResponseBody(
    _in const std::string & c_strRequestData,
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();

    std::vector<Byte> stlSerializedResponse;

    // Check http code
    bool fSuccess = ::ParseFirstLine(c_strRequestData);
    // Parse Header of the Rest Request
    HttpRequestParser oParser;
    fSuccess = oParser.ParseResponse(c_strRequestData);
    _ThrowBaseExceptionIf((false == fSuccess), "Error: Parsing response failed.", nullptr);

    if (true == oParser.HeaderExists("Content-Length"))
    {
        unsigned int unContentLength = std::stoi(oParser.GetHeaderValue("Content-Length"));
        if (0 < unContentLength)
        {
            // Read request content
            std::vector<Byte> stlBodyData = poTlsNode->Read(unContentLength, 2000);
            _ThrowBaseExceptionIf((0 == stlBodyData.size()), "Dead Packet.", nullptr);
            std::string strRequestBody = std::string(stlBodyData.begin(), stlBodyData.end());

            // Check Content-Type
            _ThrowBaseExceptionIf((false == oParser.HeaderExists("Content-Type")), "Invalid request format.", nullptr);
            std::string strContentType = oParser.GetHeaderValue("Content-Type");
            if ("application/json" == strContentType)
            {
                // Parse Json
                std::string strUnEscapseJsonString = ::UnEscapeJsonString(strRequestBody);
                stlSerializedResponse = JsonValue::ParseDataToStructuredBuffer(strUnEscapseJsonString.c_str());
            }
            else
            {
                _ThrowBaseException("Content Type: %s not supported.", strContentType);
            }
        }
    }
    _ThrowBaseExceptionIf((0 == stlSerializedResponse.size()), "Error logging in.", nullptr);

    return stlSerializedResponse;
}

/********************************************************************************************/

std::string Login(
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
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return strEosb;
}

void __thiscall Frontend::Listener(
    _in TlsNode* poSocket
)
{
    __DebugFunction();
    while(!m_fStop)
    {
        std::vector<Byte> stlResponseBuffer = GetTlsTransaction(poSocket, 0);
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
                std::lock_guard<std::mutex> lock(m_stlResultMapMutex);
                //m_stlResultMap.emplace(strDataID, stlData);
                m_stlResultMap[strDataID] = stlData;
                break;
            }
            case JobStatusSignals::eVmShutdown: break;
            default: break;
        }
    }

    poSocket->Release();
    // for(auto const& i: m_stlConnectionMap)
    // {
    //     i.second->Release();
    // }
}


void __thiscall Frontend::SetFrontend(
    _in std::string & strServerIP, 
    _in Word wPort,
    _in std::string & strVMID,
    _in std::string & strEmail,
    _in std::string & strPassword
    )
{
    if(m_strEOSB.empty())
    {
        m_strEOSB = Login(strEmail, strPassword);
        std::cout<<"Login done"<<std::endl;
        std::cout<<"EOSB:"<<m_strEOSB<<std::endl;
    }
    
    StructuredBuffer oBuffer;
    oBuffer.PutByte("RequestType", (Byte)EngineRequest::eConnectVirtualMachine);
    oBuffer.PutString("Eosb", m_strEOSB);
    oBuffer.PutString("Username", strEmail);

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
            StructuredBuffer oResponse(stlResponse);
            strVMID = oResponse.GetString("VirtualMachineUuid");

            StructuredBuffer oDataset = oResponse.GetStructuredBuffer("Dataset");
            std::vector<std::string> stlGuidList = oDataset.GetStructuredBuffer("Tables").GetNamesOfElements();
            m_stlDataTableMap.emplace(strVMID, stlGuidList);

            std::shared_ptr<TlsNode> stlSocket(poSocket);
            m_stlConnectionMap.emplace(strVMID, stlSocket);
        }
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    std::thread stlListener(&Frontend::Listener, this, m_stlConnectionMap[strVMID].get());
    stlListener.detach();
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
    StructuredBuffer oBuffer;
    oBuffer.PutString("EndPoint", "JobEngine");
    oBuffer.PutByte("RequestType", (Byte)EngineRequest::eSubmitJob);
    oBuffer.PutString("SafeObjectUuid", strFNID);
    oBuffer.PutString("JobUuid", strJobID);

    try
    {
        //PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer);
        PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer.GetSerializedBuffer());
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
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
//         ::RegisterException(oBaseException, __func__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __LINE__);
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
//         ::RegisterException(oBaseException, __func__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __LINE__);
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
            //PutTlsTransactionAndGetResponse(i.second.get(), oBuffer, 100);
            PutTlsTransaction(i.second.get(), oBuffer.GetSerializedBuffer());
        }
        
        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
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
    for(size_t i=0; i<stlInputIds.size(); i++)
    {
        StructuredBuffer oBuffer;
    
        oBuffer.PutByte("RequestType", (Byte)EngineRequest::ePushdata);
        oBuffer.PutString("EndPoint", "JobEngine");
        oBuffer.PutString("DataId", stlInputIds[i]);
        oBuffer.PutBuffer("Data", stlInputVars[i]);

        try
        {
            //PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer);
            PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer.GetSerializedBuffer());
        }
        
        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
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
            //PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer);
            PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer.GetSerializedBuffer());
        }
        
        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
        }

    }
}

void __thiscall Frontend::HandlePullData(
    _in std::string & strVMID,
    _in std::string & strJobID,
    _in std::string & strFNID
    )
{
    std::vector<std::string> stlOutputIDs = m_stlFNTable[strFNID]->GetOutput();

    for(size_t i=0; i<stlOutputIDs.size(); i++)
    {
        StructuredBuffer oBuffer;
        std::string strOutputFilename = strJobID + "." + stlOutputIDs[i];
        
        oBuffer.PutByte("RequestType", (Byte)EngineRequest::ePullData);
        oBuffer.PutString("EndPoint", "JobEngine");
        oBuffer.PutString("Filename", strOutputFilename);
            
        try
        {
            //std::future<std::vector<Byte>> stlFutureResult = std::async(std::launch::async, PutTlsTransactionAndGetResponse, m_stlConnectionMap[strVMID], oBuffer, 100);
            //PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer);
            //m_stlResultMap[strJobID].push_back(stlFutureResult);
            PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer.GetSerializedBuffer());
        }
            
        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
        }
    }
}

void __thiscall Frontend::QueryResult(
    _in std::string& strJobID,
    _in std::string& strFNID,
    _inout std::vector<std::vector<Byte>>& stlOutput
)
{
    std::vector<std::string> stlOutputID = m_stlFNTable[strFNID]->GetOutput();

    for(size_t i =0; i<stlOutputID.size(); i++)
    {
        std::string strDataID = strJobID + "." + stlOutputID[i];
        std::lock_guard<std::mutex> lock(m_stlResultMapMutex);
        stlOutput.push_back(m_stlResultMap[strDataID]);
    }
}

JobStatusSignals __thiscall Frontend::QueryJobStatus(
    _in std::string& strJobID
)
{
    std::lock_guard<std::mutex> lock(m_stlJobStatusMapMutex);
    return m_stlJobStatusMap[strJobID];
}

std::vector<std::string> __thiscall Frontend::QueryDataset(
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
//         ::RegisterException(oBaseException, __func__, __LINE__);
//     }

//     catch(...)
//     {
//         ::RegisterUnknownException(__func__, __LINE__);
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
        //PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer);
        PutTlsTransaction(m_stlConnectionMap[strVMID].get(), oBuffer.GetSerializedBuffer());
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

void __thiscall Frontend::RegisterSafeObject(
    _in std::string& strFilePath
    )
{
    for (const auto & entry : std::filesystem::directory_iterator(strFilePath))
    {
        std::string strFilename = entry.path().string();
        std::ifstream stlFNFile;
        stlFNFile.open(strFilename, (std::ios::in | std::ios::binary | std::ios::ate));
        unsigned int unFileSizeInBytes = (unsigned int)stlFNFile.tellg();
        std::vector<Byte> stlBuffer;
        stlBuffer.resize(unFileSizeInBytes);
        //stlFNFile.unsetf(std::ios::skipws);
        stlFNFile.seekg(0, std::ios::beg);
        stlFNFile.read((char*)stlBuffer.data(), unFileSizeInBytes);
        //stlBuffer.insert(stlBuffer.begin(),std::istream_iterator<Byte>(stlFNFile), std::istream_iterator<Byte>());
        stlFNFile.close();

        StructuredBuffer oSafeObject(stlBuffer);
        std::unique_ptr<SafeObject> poFN = std::make_unique<SafeObject>(oSafeObject);
        std::string strFNID = poFN->GetSafeObjectID();
        m_stlFNTable.emplace(strFNID, std::move(poFN));
    }
}
