/*********************************************************************************************
 *
 * @file SocketServer.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "Socket.h"

/********************************************************************************************/

class SocketServer : public Object
{
    public:
    
        SocketServer(
            _in Word wPortIdentifier
            ) throw();
        SocketServer(
            _in const char * c_szUnixSocketAddress
            ) throw();
        ~SocketServer(void);

        bool __thiscall WaitForConnection(
            _in unsigned int unMillisecondTimeout
            );
        Socket * __thiscall Accept(void);

    private:
    
        int m_nSocketDescriptor;
        int m_nPollingFileDescriptor;
};