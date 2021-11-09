/*********************************************************************************************
 *
 * @file TlsNode.h
 * @author Prawal Gangwar
 * @date 09 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the TlsNode class that is used to drive TLS Connections
 *
 ********************************************************************************************/

#pragma once

#include <vector>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "CoreTypes.h"
#include "Socket.h"
#include "Object.h"
#include "FifoBuffer.h"

#include "memory"

/********************************************************************************************/

enum SSLMode
{
    eSSLModeServer,
    eSSLModeClient
};

class TlsNode : public Object
{
    public:

        TlsNode(
            _in Socket * poSocket,
            _in enum SSLMode eConnectionMode
            );
        TlsNode(
            _in const TlsNode & c_oTLSNode
            );
        ~TlsNode(void);

        // Callers can specify a unMillisecondTimeout on the read operations. If the methods
        // return 0, this means that the timeout expired before the read operation could be
        // fulfilled. The Read() operations will NEVER return a partial buffer. They will
        // ALWAYS either return 0 or return unNumberOfDesiredBytes, unless there is an
        // error and then the Read() methods return -1
        std::vector<Byte> __thiscall Read(
            _in unsigned int unNumberOfDesiredBytes,
            _in unsigned int unMillisecondTimeout
            ) throw();
        // Unlike the read methods, there is no timeout on Write(). The Write() methods will
        // block until the write operation is completed. The Write() methods will either
        // return the number of bytes or throw exception if it fails.
        int __thiscall Write(
            _in const Byte * c_pbSourceBuffer,
            _in unsigned int unNumberOfBytesToWrite
            ) throw();

    private:

        void __thiscall SSLHandshake(
            _in unsigned int unMillisecondTimeout
            );
        void __thiscall LoadServerCTXKeyAndCertificate(
            _in SSL_CTX * poSSL_CTX
            )const;

        std::unique_ptr<Socket> m_poSocket;
        std::unique_ptr<SSL, decltype(&SSL_free)> m_poSSL;
        // TODO: BIOs are freed in SSL_free() but in case the BIOs are assigned
        // and not attached to SSL object they will not be free and lead to a
        // memory leak. :|
        BIO * m_poWriteBIO;
        BIO * m_poReadBIO;
        FifoBuffer m_oDecryptedBytesCache;
        std::vector<Byte> m_stlTlsHeaderCache;
};
