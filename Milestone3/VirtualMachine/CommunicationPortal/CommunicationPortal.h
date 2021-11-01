/*********************************************************************************************
 *
 * @file CommunicationPortal.h
 * @author Prawal Gangwar
 * @date 08 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Communication Portal is a one-stop gateway to all the communication that happens on
 *      the Virtual Machine
 *
*********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "Socket.h"
#include "TlsNode.h"
#include "StructuredBuffer.h"

#include <string>
#include <unordered_set>
#include <unordered_map>

const std::string gc_strCommunicationPortalAddress = "{3d14320b-c7bd-4496-81b3-90a4a703af93}";

class CommunicationPortal : public Object
{
    public:

        CommunicationPortal(void);
        // Singleton objects cannot be copied, the copy constructor and = operator are deleted
        CommunicationPortal(
            _in const CommunicationPortal & c_oJobEngine
            ) = delete;
        void operator=(
            _in CommunicationPortal const & c_oJobEngine
            ) = delete;
        ~CommunicationPortal(void);

        void __thiscall StartServer(
            _in Word wPortNumber
            );
        void __thiscall WaitForProcessToRegister(void);

        // This function will be used by the process to register themselves
        // with the Communication Portal and get a socket for further communication
        static Socket * const __thiscall RegisterProcess(
            _in const std::string & c_strProcessName
        );

    private:

        // Private member methods
        void __thiscall PersistantConnectionTlsToIpc(
            _in TlsNode * const c_oTlsNode,
            _in const std::string c_strEndpoint,
            _in const StructuredBuffer oStructuredBuffer
            );
            void __thiscall PersistantConnectionIpcToTls(
            _in TlsNode * const c_poTlsNode,
            _in const std::string c_strEndpoint
            );
        void __thiscall OneTimeConnectionHandler(
            _in TlsNode * const c_oTlsNode,
            _in const StructuredBuffer c_oStructuredBuffer
            );
        void __thiscall HandleConnection(
            _in TlsNode * const c_oTlsNode
            );

        // Private data members
        std::unordered_set<std::string> m_oSetOfAllowedProcesses;
        std::unordered_map<std::string, Socket *> m_stlMapOfProcessToIpcSocket;
        // TODO: Prawal take recursive locks for this
        std::unordered_map<std::string, bool> m_stlKillTlsToIpcConnection;
        std::unordered_map<std::string, bool> m_stlIpcToTlsConnectionConnected;
        std::unordered_map<std::string, bool> m_stlTlsToIpcConnectionConnected;
};
