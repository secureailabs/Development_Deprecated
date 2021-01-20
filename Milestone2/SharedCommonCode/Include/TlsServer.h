/*********************************************************************************************
 *
 * @file TlsServer.h
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TlsServer class that is used to create TLS Servers
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "SocketServer.h"
#include "TlsNode.h"

/********************************************************************************************/

class TlsServer : public Object
{
    public:

        TlsServer(
            _in Word wPortIdentifier
            );
        TlsServer(
            _in const char * c_szUnixSocketAddress
            );
        ~TlsServer(void);

        bool __thiscall WaitForConnection(
            _in unsigned int unMillisecondTimeout
            );
        TlsNode * __thiscall Accept(void);

    private:

        SocketServer * m_poSocketServer;
};
