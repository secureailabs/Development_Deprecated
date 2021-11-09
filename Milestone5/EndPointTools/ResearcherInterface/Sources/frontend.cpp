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
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>

#ifndef SERVER_IP_ADDRESS
    #define SERVER_PORT 6200
    #define SERVER_IP_ADDRESS "137.116.90.145"
#endif

__thiscall Frontend::Frontend(void):
    m_stlConnectionMap(),
    m_stlPortMap(),
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
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return strEosb;
}


void __thiscall Frontend::SetFrontend(
    _in std::string & strServerIP, 
    _in Word wPort,
    _in std::string & strVMID,
    _in std::string & strEmail,
    _in std::string & strPassword
    )
{
    std::string strEOSB = Login(strEmail, strPassword);
    
    std::cout<<"Login done"<<std::endl;
    std::cout<<"EOSB:"<<strEOSB<<std::endl;

    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eCONNECT);
    oBuffer.PutString("EOSB", strEOSB);

    TlsNode * poSocket = nullptr;

    try
    {
        poSocket = ::TlsConnectToNetworkSocket(strServerIP.c_str(), wPort);
        _ThrowIfNull(poSocket, "Tls connection error for connectVM", nullptr);

        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        if(0==stlResponse.size())
            _ThrowBaseException("No response for connectVM request", nullptr);

        StructuredBuffer oResponse(stlResponse);
        strVMID = oResponse.GetString("VMID");
        // Make sure to release the socket
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
    
    m_stlConnectionMap.emplace(strVMID, strServerIP);
    m_stlPortMap.emplace(strVMID, wPort);
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

void __thiscall Frontend::HandleExecJob(
    _in std::string & strVMID,
    _in std::string & strFNID,
    _in std::string & strJobID
    )
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eRUN);
    oBuffer.PutString("FNID", strFNID);
    oBuffer.PutString("JobID", strJobID);

    TlsNode * poSocket = nullptr;

    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for ExecJob", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
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

void __thiscall Frontend::HandleInspect(
    _in std::string & strVMID,
    _inout std::string & strJobs
    )
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eINSPECT);

    TlsNode * poSocket =nullptr;
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for inspect request", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        if(0==stlResponse.size())
            _ThrowBaseException("No response for inspect request", nullptr);
        
        StructuredBuffer oResponse(stlResponse);
        strJobs = oResponse.GetString("Payload");
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
}

void __thiscall Frontend::HandleGetTable(
    _in std::string & strVMID,
    _in std::string & strTables
    )
{
    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eGETTABLE);
    
    TlsNode * poSocket = nullptr;
    
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for getTable", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        if(0==stlResponse.size())
            _ThrowBaseException("No response for getTable request", nullptr);

        StructuredBuffer oResponse(stlResponse);
        strTables = oResponse.GetString("Payload");
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
}

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

    oBuffer.PutInt8("Type", eQUIT);
    
    for(auto const& i : m_stlConnectionMap)
    {
        TlsNode * poSocket = nullptr;
        
        try
        {
            poSocket = TlsConnectToNetworkSocket(i.second.c_str(), m_stlPortMap[i.first]);
            _ThrowIfNull(poSocket, "Tls connection error for quit", nullptr);
            std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
            // Make sure to release the socket
            poSocket->Release();
            poSocket = nullptr;
        }
        
        catch(BaseException oBaseException)
        {
            ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        }

        catch(...)
        {
            ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        }
        
        if (nullptr != poSocket)
        {
            poSocket->Release();
            poSocket = nullptr;
        }
    }

    m_fStop = true;
    std::cout<<"Quit success"<<std::endl;
}

void __thiscall Frontend::HandlePushData(
    _in std::string & strVMID,
    _in std::string & strFNID,
    _in std::string & strJobID,
    _in std::vector<std::string> & stlInputID,
    _in std::vector<std::vector<Byte>> & stlInputVars,
    _in std::vector<std::string> & stlConfidentialInputIDs
    )
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePUSHDATA);
    oBuffer.PutString("JobID", strJobID);
    oBuffer.PutString("FNID", strFNID);
    
    StructuredBuffer oInputIDs;
    StructuredBuffer oInputVars;
    VecToBuf<std::vector<std::string>>(stlInputID, oInputIDs);
    VecToBuf<std::vector<std::vector<Byte>>>(stlInputVars, oInputVars);
    oBuffer.PutStructuredBuffer("VarIDs", oInputIDs);
    oBuffer.PutStructuredBuffer("Vars", oInputVars);
    
    StructuredBuffer oConfidentialInputIDs;
    VecToBuf<std::vector<std::string>>(stlConfidentialInputIDs, oConfidentialInputIDs);
    oBuffer.PutStructuredBuffer("ConfidentialInputIDs", oConfidentialInputIDs);
    
    TlsNode * poSocket = nullptr;
    
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for pushData", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        // Make sure to release the socket
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
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

void __thiscall Frontend::GetConfidentialInputVec
(
    _in std::string& strFNID,
    _inout std::vector<std::string>& stlVarIDs
)
{
    stlVarIDs = m_stlFNTable[strFNID]->GetConfidentialInput();
}

void __thiscall Frontend::GetConfidentialOutputVec
(
    _in std::string& strFNID,
    _inout std::vector<std::string>& stlVarIDs
)
{
    stlVarIDs = m_stlFNTable[strFNID]->GetConfidentialOutput();
}

void __thiscall Frontend::HandlePullData(
    _in std::string & strVMID,
    _in std::string & strJobID,
    _in std::vector<std::string> & stlvarIDs,
    _inout std::vector<std::vector<Byte>> & stlVars   
    )
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePULLDATA);
    oBuffer.PutString("JobID", strJobID);
    
    StructuredBuffer oVarIDs;
    StructuredBuffer oPassIDs;
    VecToBuf<std::vector<std::string>>(stlvarIDs, oVarIDs);
    oBuffer.PutStructuredBuffer("VarIDs", oVarIDs);
    
    TlsNode * poSocket  = nullptr;
    
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for pullData", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        if(0==stlResponse.size())
            _ThrowBaseException("No response for pullData request", nullptr);
        StructuredBuffer oResponse(stlResponse);
        StructuredBuffer oVars = oResponse.GetStructuredBuffer("Vars");
        BufToVec<std::vector<std::vector<Byte>>>(oVars, stlVars);
        // Make sure to release the socket
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
}

void __thiscall Frontend::HandleDeleteData(
    _in std::string & strVMID,
    _in std::vector<std::string> & stlvarID   
    )
{
    //TODO
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", eDELETEDATA);
    oBuffer.PutString("VMID", strVMID);
    
    StructuredBuffer oVars;
    VecToBuf<std::vector<std::string>>(stlvarID, oVars);
    oBuffer.PutStructuredBuffer("Vars", oVars);
    
    TlsNode * poSocket = nullptr;
    
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for DeleteData", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        // Make sure to release the socket
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
}

void __thiscall Frontend::HandlePushFN(
    _in std::string & strVMID,
    _in std::string & strFNID
    )
{
    StructuredBuffer oBuffer;
    
    oBuffer.PutInt8("Type", ePUSHFN);
    
    oBuffer.PutString("FNID", m_stlFNTable[strFNID]->GetFNID());
    oBuffer.PutString("FNScript", m_stlFNTable[strFNID]->GetScript());
    
    std::vector<std::string> stlInputIDs = m_stlFNTable[strFNID]->GetInput(); 
    std::vector<std::string> stlOutputIDs = m_stlFNTable[strFNID]->GetOutput(); 
    std::vector<std::string> stlConfidentialInputIDs = m_stlFNTable[strFNID]->GetConfidentialInput();
    std::vector<std::string> stlConfidentialOutputIDs = m_stlFNTable[strFNID]->GetConfidentialOutput();

    StructuredBuffer oInputBuffer;
    StructuredBuffer oOutputBuffer;
    StructuredBuffer oConfidentialInputBuffer;
    StructuredBuffer oConfidentialOutputBuffer;

    VecToBuf<std::vector<std::string>>(stlInputIDs, oInputBuffer);
    VecToBuf<std::vector<std::string>>(stlOutputIDs, oOutputBuffer);
    VecToBuf<std::vector<std::string>>(stlConfidentialInputIDs, oConfidentialInputBuffer);
    VecToBuf<std::vector<std::string>>(stlConfidentialOutputIDs, oConfidentialOutputBuffer);
    
    oBuffer.PutStructuredBuffer("Input", oInputBuffer);
    oBuffer.PutStructuredBuffer("Output", oOutputBuffer);
    oBuffer.PutStructuredBuffer("ConfidentialInput", oConfidentialInputBuffer);
    oBuffer.PutStructuredBuffer("ConfidentialOutput", oConfidentialOutputBuffer);
    
    TlsNode* poSocket = nullptr;
    
    try
    {
        poSocket = TlsConnectToNetworkSocket(m_stlConnectionMap[strVMID].c_str(), m_stlPortMap[strVMID]);
        _ThrowIfNull(poSocket, "Tls connection error for pushFN", nullptr);
        std::vector<Byte> stlResponse = PutTlsTransactionAndGetResponse(poSocket,oBuffer,2*60*1000);
        if(0==stlResponse.size())
            _ThrowBaseException("No response for pushFN request", nullptr);
        // Make sure to release the socket
        poSocket->Release();
        poSocket = nullptr;
    }
    
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
    
    if (nullptr != poSocket)
    {
        poSocket->Release();
        poSocket = nullptr;
    }
}

void __thiscall Frontend::RegisterFN(
    _in std::string strFilePath,
    _in int nInputNumber,
    _in int nOutputNumber,
    _in int nConfidentialInputNumber,
    _in int nConfidentialOutputNumber,
    _inout std::string& strFNID
    )
{
    std::unique_ptr stlFNPointer = std::make_unique<FunctionNode>(nInputNumber, nOutputNumber, nConfidentialInputNumber, nConfidentialOutputNumber, strFilePath);
    strFNID = stlFNPointer->GetFNID();
    m_stlFNTable.emplace(strFNID, std::move(stlFNPointer));
}
