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
#include "Socket.h"
#include "engine.h"
#include "job.h"

#define HEADERLENGTH 2

/********************************************************************************************/

enum RequestType{
    eQUIT = 0,
    eRUN,
    eCHECK,
    eINSPECT
};

/********************************************************************************************/

class ComputationVM{
    private:
        JobEngine m_oEngine;
        SocketServer m_oSocketServer;
        bool m_fStop;

        void __thiscall HandleConnection
        (
            _in Socket* poSocket
        );
        void __thiscall SocketListen(void);
        void __thiscall HandleQuit(void);
        std::string __thiscall HandleRun
        (
            _in std::string strRequest,
            _in std::string strJobName
        );
        std::string __thiscall HandleInspect
        (
            _in std::string strRequest
        );
        std::string __thiscall HandleCheck
        (
            _in std::string strRequest
        );
        void __thiscall Halt(void);
        std::string __thiscall RetrieveDatasets(void);

    public:
        ComputationVM
        (
            _in Word wPortIdentifier, 
            _in size_t nMaxProcess
        );
        void __thiscall InitializeVM(void);
};
