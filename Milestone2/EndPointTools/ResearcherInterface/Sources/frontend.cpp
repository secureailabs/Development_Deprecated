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
#include "Exceptions.h"
#include "DebugLibrary.h"
#include "IpcTransactionHelperFunctions.h"
#include "HttpRequestParser.h"
#include "JsonValue.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>

#ifndef SERVER_IP_ADDRESS
    #define SERVER_PORT 6200
    #define SERVER_IP_ADDRESS "127.0.0.1"
#endif

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
    _ThrowBaseExceptionIf(("200" != strStatus), "Transaction returned with error code:", strStatus);

    return fSuccess;
}

std::vector<Byte> GetResponseBody(
    _in const std::string & c_strRequestData,
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();

    std::vector<Byte> stlSerializedResponse;

    // Check http code
    bool fSuccess = ParseFirstLine(c_strRequestData);
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
            std::vector<Byte> stlBodyData = poTlsNode->Read(unContentLength, 100);
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

std::string Login(
    _in const std::string & c_strEmail,
    _in const std::string & c_strUserPassword
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strEmail.length());
    __DebugAssert(0 != c_strUserPassword.length());

    std::string strEosb;

    try
    {
        bool fSuccess = false;
        TlsNode * poTlsNode = nullptr;
        poTlsNode = ::TlsConnectToNetworkSocket(SERVER_IP_ADDRESS, SERVER_PORT);

        std::string strHttpLoginRequest = "POST /SAIL/AuthenticationManager/User/Login?Email="+ c_strEmail +"&Password="+ c_strUserPassword +" HTTP/1.1\r\n"
                                        "Accept: */*\r\n"
                                        "Host: localhost:6200\r\n"
                                        "Connection: keep-alive\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";

        // Send request packet
        poTlsNode->Write((Byte *) strHttpLoginRequest.data(), (strHttpLoginRequest.size()));

        // Read Header of the Rest response one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {
            stlHeaderData.push_back(poTlsNode->Read(1, 100).at(0));
            if (4 <= stlHeaderData.size())
            {
                if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        std::vector<Byte> stlSerializedResponse = ::GetResponseBody(strRequestHeader, poTlsNode);
        StructuredBuffer oResponse(stlSerializedResponse);
        _ThrowBaseExceptionIf((201 != oResponse.GetFloat64("Status")), "Error logging in.", nullptr);
        strEosb = oResponse.GetString("Eosb");
    }
    catch(BaseException oBaseException)
    {
        //ShowErrorMessage("Login Failed!");
        std::cout<<"Login Failed"<<std::endl;
    }

    return strEosb;
}

void __thiscall Frontend::SetFrontend
(
    _in std::string& strServerIP, 
    _in Word wPort,
    _in std::string& strVMID
)
{
    Socket* poSocket = ConnectToNetworkSocket(strServerIP, wPort);
    std::shared_ptr<Socket>stlSocket=std::shared_ptr<Socket>(poSocket);

    std::string strEOSB = Login("abc@mail.com","sailpassword");

    StructuredBuffer oBuffer;
    oBuffer.PutInt8("Type", eCONNECT);
    oBuffer.PutString("EOSB", strEOSB);

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
    
    PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
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
    
    PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
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

    PutIpcTransactionAndGetResponse(m_stlConnectionMap[strVMID].get(),oBuffer);
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
