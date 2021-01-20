/*********************************************************************************************
 *
 * @file frontendCLI.h
 * @author Jingwei Zhang
 * @date 29 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "engine.h"
#include "StructuredBuffer.h"
#include "Socket.h"
#include <string>
#include <stack>
#include <map>
#include <memory>

#define RESPONSEHEADERLENGTH 2

/********************************************************************************************/

class EngineCLI{

    private:
        const std::string m_strUsername;
        std::unique_ptr<Socket> m_stlSocket;
        std::stack<std::string> m_stlCommandStack;
        bool m_fStop;

        void __thiscall CLIparse
        (
            _in std::string& command
        );
        std::map<std::string, std::string> __thiscall DecomposeString
        (
            _in std::string& command
        );
        void __thiscall HandleRun
        (
            _in StructuredBuffer&, 
            _in std::map<std::string, std::string>&
        );
        void __thiscall HandleCheck
        (
            _in StructuredBuffer&, 
            _in std::map<std::string, std::string>&
        );
        void __thiscall HandleQuit
        (
            _in StructuredBuffer&
        );
        void __thiscall HandleInspect
        (
            _in StructuredBuffer&,
            _in std::map<std::string, std::string>&
        );
        void __thiscall PrintWelcome(void);
        void __thiscall PrintHelp(void);

    public:
        EngineCLI
        (
            _in std::string strUsername, 
            _in std::string strServerIP, 
            _in Word wPort
        );
        void __thiscall CLImain(void);
};
