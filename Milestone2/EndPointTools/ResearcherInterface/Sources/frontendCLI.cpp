/*********************************************************************************************
 *
 * @file frontendCLI.cpp
 * @author Jingwei Zhang
 * @date 29 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Class used for single job related issues
 *
 ********************************************************************************************/

#include "frontendCLI.h"
#include "researchVM.h"
#include "engine.h"
#include "StructuredBuffer.h"
#include "SocketClient.h"
#include <iostream>
#include <vector>
#include <cstdlib>

/********************************************************************************************
 *
 * @class EngineCLI
 * @function EngineCLI
 * @param [in] strUsername the userName of the VM
 * @param [in] strServerIP the IP address of the server to connect to
 * @param [in] wPort the port to connect to the server
 * @brief Create a frontendCLI object
 *
 ********************************************************************************************/

EngineCLI::EngineCLI
(
    _in std::string strUsername, 
    _in std::string strServerIP, 
    _in Word wPort
): 
    m_strUsername(strUsername), 
    m_stlSocket(std::move(ConnectToNetworkSocket(strServerIP, wPort))),
    m_fStop(false)
    {}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function CLImain
 * @brief start the CLI interface for the resercher
 *
 ********************************************************************************************/

void __thiscall EngineCLI::CLImain(void)
{
    _in std::string username("test");
    _in std::string command;

    PrintWelcome();

    while(!m_fStop)
    {
        std::cout<<">>> ";
        std::getline(std::cin, command);
        CLIparse(command);
    }
}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function DecomposeString
 * @param [in] command The raw command argument from user input
 * @brief decompose the user input arguments and store the arguments into a map accordint to
 * the option labels.
 *
 ********************************************************************************************/

std::map<std::string, std::string> __thiscall EngineCLI::DecomposeString
(
    _in std::string& command
)
{
    
    std::map<std::string, std::string> argMap;
    size_t posStart = 0;

    while(true){
        posStart = command.find("-", posStart);
        size_t posEnd = command.find("-", posStart+1);

        if(posEnd!=std::string::npos)
        {
            argMap.insert({command.substr(posStart+1, 1), command.substr(posStart+3, posEnd-posStart-4)});
            posStart = posEnd;
        }
        else
        {
            argMap.insert({command.substr(posStart+1, 1), command.substr(posStart+3)});
            break;
        }
    }

    return argMap;
}

/********************************************************************************************
 *
 * @class EngineCLI
 * @function CLIparse
 * @param [in] command full command from user input
 * @brief Parse the user command obtained from frontend
 *
 ********************************************************************************************/

void __thiscall EngineCLI::CLIparse
(
    _in std::string& command
)
{

    std::map<std::string, std::string> argMap;
    StructuredBuffer oRequest;
    std::string strCommandHeader;
    std::string commandArgs;;

    size_t nPosition = command.find(" "); 
    if(nPosition != std::string::npos)
    {
        strCommandHeader = command.substr(0, nPosition);
        commandArgs = command.substr(nPosition+1);
        argMap = DecomposeString(commandArgs);
    }
    else
    {
        strCommandHeader = command;
    }

    if(!strCommandHeader.compare("quit"))
    {
        HandleQuit(oRequest);
    }
    else if(!strCommandHeader.compare("help"))
    {
        PrintHelp();
    }
    else if(!strCommandHeader.compare("run"))
    {
        HandleRun(oRequest, argMap);
    }
    else if(!strCommandHeader.compare("check")){
        HandleCheck(oRequest, argMap);
    }
    else if(!strCommandHeader.compare("inspect"))
    {
        HandleInspect(oRequest, argMap);
    }
    else
    {
        std::cout<<"V1.0 only support run, check and quit operations"<<std::endl;
    }
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

void __thiscall EngineCLI::HandleRun(
    _in StructuredBuffer& oBuffer, 
    _in std::map<std::string, std::string>& argMap
)
{ 
    //-t job type: python, R, etc.
    //-f function node id
    //-i input id
    //-n job name
	std::vector<Byte> stlRequestHeader;

    oBuffer.PutInt8("Type", eRUN);
    oBuffer.PutString("FunctionNodeNumber", argMap["f"]);
    oBuffer.PutString("InputNodeNumber", argMap["i"]);
    oBuffer.PutString("JobName", argMap["n"]);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()/256);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()%256);
    m_stlSocket->Write(stlRequestHeader.data(), stlRequestHeader.size());
    m_stlSocket->Write(oBuffer.GetSerializedBufferRawDataPtr(), oBuffer.GetSerializedBufferRawDataSizeInBytes());

    std::vector<Byte> stlResponseHeader;
    while(0==stlResponseHeader.size()){
        stlResponseHeader = m_stlSocket->Read(RESPONSEHEADERLENGTH, 500);
    }
    size_t nPayloadLength = stlResponseHeader[0]*256 + stlResponseHeader[1];
        
    std::vector<Byte> stlResponse = m_stlSocket->Read(nPayloadLength, 500);
    StructuredBuffer oResponse(stlResponse);
    bool status = oResponse.GetBoolean("Status");
    std::string strJobId = oResponse.GetString("Payload");

    if(status)
    {
        std::cout<<"Job: "<<strJobId<<" initialize successful"<<std::endl;
    }
    else
    {
        std::cout<<"Job initialize failed"<<std::endl;
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

void __thiscall EngineCLI::HandleCheck
(
    _in StructuredBuffer& oBuffer, 
    _in std::map<std::string, std::string>& argMap
)
{
    //-n job name
    //std::string strResult = m_oEngine.GetJobResult(argMap["i"]);
	std::vector<Byte> stlRequestHeader;

    oBuffer.PutInt8("Type", eCHECK);
    oBuffer.PutString("JobName", argMap["n"]);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()/256);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()%256);
    m_stlSocket->Write(stlRequestHeader.data(), stlRequestHeader.size());
    m_stlSocket->Write(oBuffer.GetSerializedBufferRawDataPtr(), oBuffer.GetSerializedBufferRawDataSizeInBytes());

    std::vector<Byte> stlResponseHeader;
    while(0==stlResponseHeader.size()){
        stlResponseHeader = m_stlSocket->Read(RESPONSEHEADERLENGTH, 500);
    }
    size_t nPayloadLength = stlResponseHeader[0]*256 + stlResponseHeader[1];
 
    std::vector<Byte> stlResponse = m_stlSocket->Read(nPayloadLength, 500);
    StructuredBuffer oResponse(stlResponse);
    bool status = oResponse.GetBoolean("Status");

    if(status)
    {
        std::string strResponse = oResponse.GetString("Payload");
        std::cout<<strResponse<<std::endl;
    }
    else
    {
        std::cout<<"Checking result failed"<<std::endl;
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

void __thiscall EngineCLI::HandleQuit
(
    _in StructuredBuffer& oBuffer
)
{
	std::vector<Byte> stlRequestHeader;

    oBuffer.PutInt8("Type", eQUIT);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()/256);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()%256);
    m_stlSocket->Write(stlRequestHeader.data(), stlRequestHeader.size());
    m_stlSocket->Write(oBuffer.GetSerializedBufferRawDataPtr(), oBuffer.GetSerializedBufferRawDataSizeInBytes());

    std::vector<Byte> stlResponseHeader;
    while(0==stlResponseHeader.size()){
        stlResponseHeader = m_stlSocket->Read(RESPONSEHEADERLENGTH, 1000);
    }
    size_t nPayloadLength = stlResponseHeader[0]*256 + stlResponseHeader[1];
 
    std::vector<Byte> stlResponse = m_stlSocket->Read(nPayloadLength, 3000);
    StructuredBuffer oResponse(stlResponse);
    bool status = oResponse.GetBoolean("Status");

    if(status)
    {
        m_fStop = true;
        std::cout<<"Quit success"<<std::endl;
    }
    else
    {
        std::cout<<"Quit failed"<<std::endl;
    }   
}

/********************************************************************************************
 *                                                                                          *
 * @class FrontendCLI                                                                       *
 * @function HandleInspect   
 * @param [in] oBuffer Reference of the structured buffer going to be send                                                               *
 * @brief Handle inspect request                                                            *
 *                                                                                          *
 ********************************************************************************************/
void __thiscall EngineCLI::HandleInspect
(
    _in StructuredBuffer& oBuffer,
    _in std::map<std::string, std::string>& argMap
)
{
    std::vector<Byte> stlRequestHeader;

    oBuffer.PutInt8("Type", eINSPECT);
    oBuffer.PutString("InspectTarget", argMap["t"]);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()/256);
    stlRequestHeader.push_back(oBuffer.GetSerializedBufferRawDataSizeInBytes()%256);
    m_stlSocket->Write(stlRequestHeader.data(), stlRequestHeader.size());
    m_stlSocket->Write(oBuffer.GetSerializedBufferRawDataPtr(), oBuffer.GetSerializedBufferRawDataSizeInBytes());

    std::vector<Byte> stlResponseHeader;
    while(0==stlResponseHeader.size()){
        stlResponseHeader = m_stlSocket->Read(RESPONSEHEADERLENGTH, 500);
    }
    size_t nPayloadLength = stlResponseHeader[0]*256 + stlResponseHeader[1];
 
    std::vector<Byte> stlResponse = m_stlSocket->Read(nPayloadLength, 500);
    StructuredBuffer oResponse(stlResponse);
    bool status = oResponse.GetBoolean("Status");

    if(status)
    {
        std::string strResponse = oResponse.GetString("Payload");
        std::cout<<strResponse<<std::endl;
    }
    else
    {
        std::cout<<"Inspect target failed"<<std::endl;
    }
}


/********************************************************************************************
 *                                                                                          *
 * @class FrontendCLI                                                                       *
 * @function PrintWelcome                                                                   *
 * @brief Print welcome message on front end                                                *
 *                                                                                          *
 ********************************************************************************************/

void __thiscall EngineCLI::PrintWelcome(void)
{
    std::cout<<"********************************************************************************************"<<std::endl;
    std::cout<<"*                                                                                          *"<<std::endl;
    std::cout<<"*  #####     #    ### #                SAIL COMPUTATIONAL VM FRONTEND CLI V 0.1            *"<<std::endl;
    std::cout<<"* #     #   # #    #  #                                                                    *"<<std::endl;
    std::cout<<"* #        #   #   #  #           License Private and Confidential. Internal Use Only      *"<<std::endl;
    std::cout<<"*  #####  #     #  #  #                                                                    *"<<std::endl;
    std::cout<<"*       # #######  #  #          Copyright (C) Secure AI Labs, Inc. All Rights Reserved.   *"<<std::endl;
    std::cout<<"* #     # #     #  #  #                                                                    *"<<std::endl;
    std::cout<<"*  #####  #     # ### #######                      Boston, US, 2020                        *"<<std::endl;
    std::cout<<"*                                                                                          *"<<std::endl;
    std::cout<<"********************************************************************************************"<<std::endl;
    std::cout<<"\n\n\n"<<std::endl;
}

/********************************************************************************************
 *                                                                                          *
 * @class FrontendCLI                                                                       *
 * @function PrintHelp                                                                      *
 * @brief Print help message on the frontend                                                *
 *                                                                                          *
 ********************************************************************************************/

void __thiscall EngineCLI::PrintHelp(void)
{
    std::cout<<"Command Supported in V 0.1"<<std::endl;
    std::cout<<"help: show help messages"<<std::endl;
    std::cout<<"quit: quit the frontend CLI"<<std::endl;
    std::cout<<"run: submit a job to the backend"<<std::endl;
    std::cout<<"    -t: type of the engine which the command will run on (now only support python)"<<std::endl;
    std::cout<<"    -m: module to be used (e.g.: sail_logistic_regression, sail_xgboost)"<<std::endl;
    std::cout<<"    -f: function signature, the function in the module to be called"<<std::endl;
    std::cout<<"    -n: the name of the job"<<std::endl;
    std::cout<<"check: check the result of a submitted job"<<std::endl;
    std::cout<<"    -n: the name of the job"<<std::endl;
    std::cout<<"inspect: inspect running jobs and data"<<std::endl;
    std::cout<<"    -t: inspect target, can be either job or dataset"<<std::endl;
}
