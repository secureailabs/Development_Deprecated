/*********************************************************************************************
 *
 * @file researchVM.h
 * @author Jingwei Zhang
 * @date 29 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "StructuredBuffer.h"
#include "SocketServer.h"
#include "RootOfTrustNode.h"
#include "Socket.h"
#include "engine.h"
#include "job.h"
#include "utils.h"
#include "function_node.h"

#define HEADERLENGTH 2

/********************************************************************************************/

class ComputationVM{
    private:
        JobEngine m_oEngine;
        SocketServer m_oSocketServer;
        std::map<std::string, std::unique_ptr<FunctionNode>> m_stlFNMap;
        std::string m_strVMID;
        std::string m_strEOSB;
        bool m_fStop;
        RootOfTrustNode m_oRootOfTrustNode;

        void __thiscall HandleConnection
        (
            _in Socket* poSocket
        );
        void __thiscall SocketListen(void);
        void __thiscall HandleConnect
        (
            _in StructuredBuffer& oContent,
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleQuit
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleRun
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall Run
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleInspect
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleCheck
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleGetTable
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandlePushFN
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall SaveFN
        (
            std::string& strFNID,
            std::string& strFNScript
        );
        void  __thiscall SaveOutput
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandleDeleteData
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall HandlePullData
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall SaveBuffer
        (
            std::string& strJobID,
            std::vector<std::string>& stlVarIDs,
            std::vector<std::vector<Byte>>& stlVars 
        );
        void __thiscall HandlePushData
        (
            _in StructuredBuffer& oContent, 
            _in StructuredBuffer& oResponse
        );
        void __thiscall LoadDataToBuffer
        (
            _in std::string& strJobID,
            _in std::vector<std::string>& stlVarIDs,
            _in std::vector<std::vector<Byte>>& stlVars
        );
        std::string __thiscall RetrieveDatasets
        (
            void
        );
        void __thiscall Halt(void);
        void __thiscall SetEOSB(std::string& strEOSB);

    public:
        ComputationVM
        (
            _in Word wPortIdentifier, 
            _in size_t nMaxProcess,
            _in RootOfTrustNode& oRootOfTrustNode
        );
        void __thiscall InitializeVM(void);
};
