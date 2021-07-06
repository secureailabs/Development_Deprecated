/*********************************************************************************************
 *
 * @file frontend.h
 * @author Jingwei Zhang
 * @date 15 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "utils.h"
#include "StructuredBuffer.h"
#include "function_node.h"
#include "Socket.h"
#include <string>
#include <stack>
#include <map>
#include <memory>

#define RESPONSEHEADERLENGTH 2

/********************************************************************************************/

class Frontend{

    private:
        std::map<std::string, std::string> m_stlConnectionMap;
        std::map<std::string, Word> m_stlPortMap;
        std::map<std::string, std::unique_ptr<FunctionNode>> m_stlFNTable;
        bool m_fStop;

    public:
        __thiscall Frontend(void);
        void __thiscall SetFrontend
        (
            _in std::string& strServerIP, 
            _in Word wPort,
            _in std::string& strVMID,
            _in std::string& strEmail,
            _in std::string& strPassword
        );
        void __thiscall HandleExecJob
        (
            _in std::string& strVMID,
            _in std::string& strFNID,
            _in std::string& strJobID
        );
        void __thiscall HandleInspect
        (
            _in std::string& strVMID,
            _inout std::string& strJobs
        );
        void __thiscall HandleGetTable
        (
            _in std::string& strVMID,
            _in std::string& strTables
        );
        void __thiscall HandleQuit
        (
            void
        );
        void __thiscall HandleCheck
        (
            _in std::string& strVMID
        );
        void __thiscall HandlePushData
        (
            _in std::string& strVMID,
            _in std::string& strFNID,
            _in std::string& strJobID,
            _in std::vector<std::string>& stlvarID,
            _in std::vector<std::vector<Byte>>& stlVars,
            _in std::vector<std::string>& stlPassID   
        );
        void __thiscall HandlePullData
        (
            _in std::string& strVMID,
            _in std::string& strJobID,
            _in std::vector<std::string>& stlvarIDs,
            _inout std::vector<std::vector<Byte>>& stlVars   
        );
        void __thiscall HandleDeleteData
        (
            _in std::string& strVMID,
            _in std::vector<std::string>& stlvarID
        );
        void __thiscall HandlePushFN
	    (
            _in std::string& strVMID,
            _in std::string& strFNID
        );
        void __thiscall RegisterFN
        (
            _in std::string strFilePath,
            _in int nInputNumber,
            _in int nOutputNumber,
            _in int nConfidentialInputNumber,
            _in int nConfidentialOutputNumber,
            _inout std::string& strFNID
        );
        void __thiscall GetInputVec
        (
            _in std::string& strFNID,
            _inout std::vector<std::string>& stlVarIDs
        );
        void __thiscall GetOutputVec
        (
            _in std::string& strFNID,
            _inout std::vector<std::string>& stlVarIDs
        );
        void __thiscall GetConfidentialInputVec
        (
            _in std::string& strFNID,
            _inout std::vector<std::string>& stlVarIDs
        );
        void __thiscall GetConfidentialOutputVec
        (
            _in std::string& strFNID,
            _inout std::vector<std::string>& stlVarIDs
        );
};

