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

#include "StructuredBuffer.h"
#include "SafeObject.h"
#include "TlsNode.h"
#include "TlsServer.h"
#include <string>
#include <stack>
#include <map>
#include <memory>
#include <future>
#include <unordered_set>

/********************************************************************************************/
enum class EngineRequest
{
    eVmShutdown = 0,
    ePushSafeObject = 1,
    eSubmitJob = 2,
    ePullData = 3,
    ePushdata = 4,
    eSetParameters = 5,
    eHaltAllJobs = 6,
    eJobStatusSignal = 7,
    eConnectVirtualMachine = 8,
    eHeartBeatPong = 9
};

enum class JobStatusSignals
{
    eJobStart = 0,
    eJobDone = 1,
    eJobFail = 2,
    ePostValue = 3,
    eVmShutdown = 4,
    ePrivacyViolation = 5,
    eHeartBeatPing = 6
};

class Frontend : public Object{

    public:

        Frontend(void);
        Frontend(
            _in const Frontend& c_oFrontend
        ) = delete;
        ~Frontend(void);

        void __thiscall SetFrontend
        (
            _in std::string& strServerIP, 
            _in Word wPort,
            _in std::string& strVMID
        );
        void __thiscall Listener(
            _in std::string strVMID
        );
        std::string Login(
            _in const std::string& c_strEmail,
            _in const std::string& c_strUserPassword
        );
        void __thiscall HandleSubmitJob
        (
            _in std::string& strVMID,
            _in std::string& strFNID,
            _in std::string& strJobID
        );
        // void __thiscall HandleInspect
        // (
        //     _in std::string& strVMID,
        //     _inout std::string& strJobs
        // );
        // void __thiscall HandleGetTable
        // (
        //     _in std::string& strVMID,
        //     _in std::string& strTables
        // );
        void __thiscall HandleQuit
        (
            void
        );
        // void __thiscall HandleCheck
        // (
        //     _in std::string& strVMID
        // );
        void __thiscall HandlePushData
        (
            _in std::string& strVMID,
            _in std::vector<std::string>& stlInputIds,
            _in std::vector<std::vector<Byte>> & stlInputVars
        );
        void __thiscall HandleSetParameters
        (
            _in std::string& strVMID,
            _in std::string& strFNID,
            _in std::string& strJobID,
            _in std::vector<std::string>stlParams
        );
        void __thiscall HandlePullData
        (
            _in std::string& strVMID,
            _in std::string& strJobID,
            _in std::string & strFNID
        );
        void __thiscall QueryResult
        (
            _in std::string& strJobID,
            _in std::string& strFNID,
            _inout std::map<std::string, int>& stlOutput
        );
        JobStatusSignals __thiscall QueryJobStatus(
            _in std::string& strJobID
        );
        std::map<std::string, std::string> __thiscall QueryDataset(
            _in std::string& strVMID
        );
        // void __thiscall HandleDeleteData
        // (
        //     _in std::string& strVMID,
        //     _in std::vector<std::string>& stlvarID
        // );
        void __thiscall HandlePushSafeObject
        (
            _in std::string& strVMID,
            _in std::string& strFNID
        );
        void __thiscall RegisterSafeObject
        (
            _in std::string& strFilePath
        );
        void __thiscall SaveBuffer(
            _in std::string& strDataID,
            _in std::vector<Byte>& stlVars
        );
        void __thiscall SendDataToJobEngine(
            _in const std::string& strVMID,
            _in StructuredBuffer & c_oStructuredBuffer
        );

    private:
        std::map<std::string, std::shared_ptr<TlsNode>> m_stlConnectionMap;
        std::map<std::string, std::shared_ptr<std::mutex>> m_stlConnectionMutexMap;
        std::map<std::string, JobStatusSignals> m_stlJobStatusMap;
        std::map<std::string, std::map<std::string, std::string>> m_stlDataTableMap;
        //std::string m_strWebPortalIP;
        //std::string m_strWebPortalPort;
        std::map<std::string, std::unique_ptr<SafeObject>> m_stlFNTable;
        //std::map<std::string, std::vector<Byte>> m_stlResultMap;
        std::unordered_set<std::string> m_stlResultSet;
        std::string m_strUsername;
        std::string m_strEOSB;
        std::mutex m_stlResultMapMutex;
        std::mutex m_stlJobStatusMapMutex;
        std::mutex m_stlFlagMutex;
        bool m_fStop;
};
