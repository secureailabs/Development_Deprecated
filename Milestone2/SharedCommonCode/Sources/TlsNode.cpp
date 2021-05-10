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

#include "Chronometer.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "Socket.h"
#include "TlsNode.h"
#include "HardCodedCryptographicKeys.h"

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <memory>

/********************************************************************************************
 *
 * @class TlsNode
 * @function TlsNode
 * @brief Constructor that uses a socket object value to initialize TLS Node
 * @param[in] poSocket Socket object pointer of the socket to use
 * @param[in] connectionMode SSLMode enum to distinguish between Client and Server mode
 * @throw BaseException When an internal error occurs
 *
 ********************************************************************************************/

TlsNode::TlsNode(
    _in Socket * poSocket,
    _in enum SSLMode connectionMode
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    SSL_CTX * poSSL_CTX = nullptr;

    try
    {

        m_poSocket = poSocket;

        // Loading and initializing the needed SSL libraries
        ::SSL_library_init();
        ::OpenSSL_add_all_algorithms();
        ::SSL_load_error_strings();
        ::ERR_load_BIO_strings();
        ::ERR_load_crypto_strings();

        // Initalizing the SSL Context structure
        poSSL_CTX = ::SSL_CTX_new(TLS_method());
        if (nullptr == poSSL_CTX)
        {
            ::SSL_CTX_free(poSSL_CTX);
            _ThrowBaseException("SSL_CTX_new() failed.", nullptr);
        }

        // TODO temporary resort until the key pair can be provided by the planned TrustStore
        if (eSSLModeServer == connectionMode)
        {
            /* Load certificate and private key files, and check consistency */
            this->LoadServerCTXKeyAndCertificate(poSSL_CTX);
        }

        // Options Set: To support just TLS1.2 for now
        // SSL_OP_ALL: Bug workarounds
        // SSL_OP_NO_SSLv2: Do not use the SSLv2 protocol
        // SSL_OP_NO_SSLv3: Do not use the SSLv3 protocol
        // SSL_OP_NO_TLSv1_1: Do not use the TLSv1.1 protocol
        // SSL_OP_NO_TLSv1_3: Do not use the TLSv1.3 protocol.
        // SSL_OP_NO_TLSv1: Do not use the TLSv1 protocol.
        ::SSL_CTX_set_options(poSSL_CTX, SSL_OP_ALL|SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3|SSL_OP_NO_TLSv1_1|SSL_OP_NO_TLSv1_3|SSL_OP_NO_TLSv1);

        m_poReadBIO = ::BIO_new(BIO_s_mem());
        m_poWriteBIO = ::BIO_new(BIO_s_mem());
        m_poSSL = ::SSL_new(poSSL_CTX);

        // Free the pointer
        ::SSL_CTX_free(poSSL_CTX);

        // Set the SSL Context based on the connection mode
        if (eSSLModeServer == connectionMode)
        {
            ::SSL_set_accept_state(m_poSSL);
        }
        else if (eSSLModeClient == connectionMode)
        {
            ::SSL_set_connect_state(m_poSSL);
        }

        ::SSL_set_bio(m_poSSL, m_poReadBIO, m_poWriteBIO);
    
        //Perform the TLS Handhshake with a default timeout of 15 second
        this->SSLHandshake(15000);
    }
    catch (BaseException oException)
    {
        // Release the pointer then throw an exception
        if (nullptr != poSSL_CTX)
        {
            ::SSL_CTX_free(poSSL_CTX);
        }
        _ThrowBaseException(oException.GetExceptionMessage(), nullptr);
    }
    catch(...)
    {
        if (nullptr != poSSL_CTX)
        {
            ::SSL_CTX_free(poSSL_CTX);
        }
        _ThrowBaseException("Unknown Exception Caught!", nullptr);
    }
}

/********************************************************************************************
 *
 * @class TlsNode
 * @function ~TlsNode
 * @brief Destructor
 * @throw BaseException When an internal error occurs
 *
 ********************************************************************************************/

TlsNode::~TlsNode(void)
{
    __DebugFunction();

    // The socket object is released
    m_poSocket->Release();
    // Release other data members
    m_stlTlsHeaderCache.clear();
    //SSL_free will also free the read/writes BIOs attached to it
    ::SSL_free(m_poSSL);
}

/********************************************************************************************
 *
 * @class TlsNode
 * @function Read
 * @brief Read @p unNumberOfDesiredBytes of plaintext data from the managed TlsNode
 * @param[in] unNumberOfDesiredBytes Number of bytes to read
 * @param[in] unMillisecondTimeout Timeout in milliseconds
 * @throw BaseException When an internal error occurs
 * @return Empty buffer if the timeout expired before @p unNumberOfDesiredBytes could be read
 * @return Buffer with @p unNumberOfDesiredBytes bytes of data
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall TlsNode::Read(
    _in unsigned int unNumberOfDesiredBytes,
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    std::vector<Byte> stlDestinationBuffer;

    // Try to read the unNumberOfDesiredBytes from the cache. It would return the
    // buffer of unNumberOfDesiredBytes size only if desired bytes are available
    stlDestinationBuffer = m_oDecryptedBytesCache.Read(unNumberOfDesiredBytes);

    // In case there was no sufficient data in the cache, attempt is made to read from socket
    if ((0 == stlDestinationBuffer.size()) && (0 < unMillisecondTimeout))
    {
        int nBytesRead = 0;
        Chronometer oChronometer;
        oChronometer.Start();

        while ((nBytesRead < unNumberOfDesiredBytes) && ((unMillisecondTimeout > oChronometer.GetElapsedTimeWithPrecision(Millisecond))))
        {
            // The SSL header is always cached to prevent the data loss in case the data read
            // times out and the socket FIFO buffer is only left with the data without header.
            // In case the backed up header is empty, a fresh header is read from the socket.
            if (5 != m_stlTlsHeaderCache.size())
            {
                // First 5 bytes are read, which is the size of the SSL layer header
                m_stlTlsHeaderCache = m_poSocket->Read(5, (unMillisecondTimeout - oChronometer.GetElapsedTimeWithPrecision(Millisecond)));
            }
            else if ((5 == m_stlTlsHeaderCache.size()) && ((unMillisecondTimeout > oChronometer.GetElapsedTimeWithPrecision(Millisecond))))
            {
                // the length of SSL record data is extracted from the 4th and 5th byte of
                // the SSL header. The subsequent read will read the required bytes
                int nEncryptedDataSize = m_stlTlsHeaderCache.at(3)*256 + m_stlTlsHeaderCache.at(4);
                std::vector<Byte> stlEncryptedDataReadBuffer = m_poSocket->Read(nEncryptedDataSize, unMillisecondTimeout - oChronometer.GetElapsedTimeWithPrecision(Millisecond));
                _ThrowBaseExceptionIf((stlEncryptedDataReadBuffer.size() != nEncryptedDataSize), "TLS Read failed: Read Timeout", nullptr);

                // Write the TLS header to the read BIO for the SSL_read to use
                _ThrowBaseExceptionIf((5 != ::BIO_write(m_poReadBIO, m_stlTlsHeaderCache.data(), 5)), "TLS Read failed: Writing to readBIO failed", nullptr);

                // Since the Header has been consumed and written to the readBIO,
                // it should be cleaned.
                m_stlTlsHeaderCache.clear();

                // Write the actual read data to the readBIO. This call only fails when the
                // process runs out of memory to write more data.
                _ThrowBaseExceptionIf((nEncryptedDataSize != ::BIO_write(m_poReadBIO, stlEncryptedDataReadBuffer.data(), nEncryptedDataSize)), "TLS Read failed: Writing to readBIO failed", nullptr);

                // Allocating a FIFO buffer reserve for the next SSL_read of dencrypted data.
                // It is not possible to figure out the amount of data SSL_read will
                // write beforehand. Assuming that the decrypted data length will not be
                // larger than the encrypted data length, nEncryptedDataSize is used.
                Byte * pbCircularBufferDestination = m_oDecryptedBytesCache.WriteLock(nEncryptedDataSize);
                if (nullptr != pbCircularBufferDestination)
                {
                    // SSL_read will process and decrypt the record and read the bytes
                    int nBytesActualRead = ::SSL_read(m_poSSL, pbCircularBufferDestination, nEncryptedDataSize);
                    _ThrowBaseExceptionIf((0 > nBytesActualRead), "TLS Read failed: Reading from readBIO failed", nullptr);
                    nBytesRead += nBytesActualRead;
                    // WriteUnlock will put the data into the FIFO buffer.
                    m_oDecryptedBytesCache.WriteUnlock(nBytesActualRead);
                }
            }
        }
        
        // A new attempt is made to read the unNumberOfDesiredBytes from the FIFO
        // In case the unNumberOfDesiredBytes are not available it will return
        // an empty buffer but whatever was cached is not lost
        stlDestinationBuffer = m_oDecryptedBytesCache.Read(unNumberOfDesiredBytes);
    }

    return stlDestinationBuffer;
}

/********************************************************************************************
 *
 * @class TlsNode
 * @function Write
 * @brief Write @p unNumberOfBytesToWrite of data to the TLS
 * @param[in] c_pbSourceBuffer Source buffer that contains plain text data to send
 * @param[in] unNumberOfBytesToWrite Number of bytes to send
 * @return -1 if the operation failed to write the data bytes to the network
 * @return Positive value representing the number of bytes written
 *
 ********************************************************************************************/

int __thiscall TlsNode::Write(
    _in const Byte * c_pbSourceBuffer,
    _in unsigned int unNumberOfBytesToWrite
    )
{
    __DebugFunction();

    // Write the data to the BIO and throw exception in case it fails
    // c_pbSourceBuffer is the unencrypted data, the data written to the writeBIO is encrypted
    int nUnencryptedBytesWrittenToBIO = ::SSL_write(m_poSSL, c_pbSourceBuffer, unNumberOfBytesToWrite);
    _ThrowBaseExceptionIf((nUnencryptedBytesWrittenToBIO != unNumberOfBytesToWrite), "TLS Write failed: Failed to write data to writeBIO", nullptr);

    // Get the pointer(pDataInWriteBIO) to the data in the write BIO and
    // write it to the socket descriptor connected to the other side of the TLS connection
    Byte * pDataInWriteBIO = nullptr;
    size_t nDataInWriteBIO = ::BIO_get_mem_data(m_poWriteBIO, &pDataInWriteBIO);
    if ((0 < nDataInWriteBIO) && (nullptr != pDataInWriteBIO))
    {
        int nActualBytesWritten = m_poSocket->Write(pDataInWriteBIO, nDataInWriteBIO);
        _ThrowBaseExceptionIf((nActualBytesWritten != nDataInWriteBIO), "TLS Write failed: Failed to write data to socket", nullptr);

        // Reset the buffer to the original no-data state as all of it has been read
        // and sent to the socket
        _ThrowBaseExceptionIf((1 != ::BIO_ctrl(m_poWriteBIO, BIO_CTRL_RESET, 0, nullptr)), "TLS failed: Write BIO reset failed", nullptr);
    }

    return nUnencryptedBytesWrittenToBIO;
}

/********************************************************************************************
 *
 * @class TlsNode
 * @function SSLHandshake
 * @brief Wait for the TLS Handshake to take place.
 * @param[in] unMillisecondTimeout Timeout in millisecond
 * @throw BaseException If the SSL Handshake fails
 * @return 0 on TLS handshake completion, -1 otherwise
 * @note
 *     If connection is in client mode, the handshake will be initiated.
 *     This is a non-blocking handshake which will timeout in not completed in
 *     unMillisecondTimeout milliseconds
 *
 ********************************************************************************************/

void __thiscall TlsNode::SSLHandshake(
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    // Start the timeout counter
    Chronometer oChronometer;
    oChronometer.Start();

    do
    {
        int nSSLError = SSL_ERROR_NONE;
        // Client mode handshake will send the 'Client Hello' to server and wait for respnse
        // Server mode handhsake will be in listening mode and wait for 'Client Hello'
        int nSSLHandhsakeStatus = ::SSL_do_handshake(m_poSSL);
        if (0 > nSSLHandhsakeStatus)
        {
            // Get the state at which the handshake was not completed i.e. if it failed in
            // writing to or reading a response from the underlying BIO
            nSSLError = ::SSL_get_error(m_poSSL, nSSLHandhsakeStatus);
        }

        // Get the pointer(pDataInWriteBIO) to the data in the write BIO and
        // write it to the socket descriptor connected to the other side of the TLS connection
        Byte * pDataInWriteBIO = nullptr;
        size_t nDataInWriteBIO = ::BIO_get_mem_data(m_poWriteBIO, &pDataInWriteBIO);
        if ((0 < nDataInWriteBIO) && (nullptr != pDataInWriteBIO))
        {
            int nActualBytesWritten = m_poSocket->Write(pDataInWriteBIO, nDataInWriteBIO);
            _ThrowBaseExceptionIf((nActualBytesWritten != nDataInWriteBIO), "TLS Handshake failed: Failed to write data to socket", nullptr);

            // Reset the buffer to the original no-data state as all of it has been read
            // and sent over the socket
            _ThrowBaseExceptionIf((1 != ::BIO_ctrl(m_poWriteBIO, BIO_CTRL_RESET, 0, nullptr)), "TLS Handshake failed: Write BIO reset failed", nullptr);
        }

        // SSL_read failed when it tried to read data from readBIO because it is empty.
        // Data needs to be read from the network and written to the read BIO for the next
        // SSL_do_handshake call
        if (SSL_ERROR_WANT_READ == nSSLError)
        {
            // First 5 bytes are read, which is the size of the SSL layer header
            std::vector<Byte> stlHandshakeHeaderBuffer = m_poSocket->Read(5, unMillisecondTimeout - oChronometer.GetElapsedTimeWithPrecision(Millisecond));
            _ThrowBaseExceptionIf((5 != stlHandshakeHeaderBuffer.size()), "TLS Handshake failed: Header Read Timeout", nullptr);

            // the length of SSL record data is extracted from the 4th and 5th byte of
            // the SSL header. The subsequent read will read the required bytes
            int nTLSHandshakeDataLength = stlHandshakeHeaderBuffer.at(3)*256 + stlHandshakeHeaderBuffer.at(4);
            std::vector<Byte> stlHandshakeDataBuffer = m_poSocket->Read(nTLSHandshakeDataLength, unMillisecondTimeout - oChronometer.GetElapsedTimeWithPrecision(Millisecond));
            _ThrowBaseExceptionIf((stlHandshakeDataBuffer.size() != nTLSHandshakeDataLength), "TLS Handshake failed: Read Timeout", nullptr);

            // Write the header and data read from the socket to the readBIO for SSL_read
            _ThrowBaseExceptionIf((5 != ::BIO_write(m_poReadBIO, stlHandshakeHeaderBuffer.data(), 5)), "TLS Handshake failed: Writing to readBIO failed", nullptr);
            _ThrowBaseExceptionIf((nTLSHandshakeDataLength != ::BIO_write(m_poReadBIO, stlHandshakeDataBuffer.data(), nTLSHandshakeDataLength)), "TLS Handshake failed: Writing to readBIO failed", nullptr);
        }
        _ThrowBaseExceptionIf((unMillisecondTimeout < oChronometer.GetElapsedTimeWithPrecision(Millisecond)), "TLS Handshake failed: Handshake Timeout", nullptr);
    }
    while (1 != SSL_is_init_finished(m_poSSL));
}

/********************************************************************************************
 *
 * @class TlsNode
 * @function LoadServerCTXKeyAndCertificate
 * @brief Gets the server key and signed certificate
 * @param[in] poSSL_CTX SSL Context which would use the key and certificate
 * @throw BaseException in case of failure
 * @return void
 * @note
 *      The key and public key certificate are generated using ECC
 *
 ********************************************************************************************/

void __thiscall TlsNode::LoadServerCTXKeyAndCertificate(
    _in SSL_CTX * poSSL_CTX
    ) const
{
    EVP_PKEY * poPrivateKey = nullptr;
    X509 * poX509Certificate = nullptr;

    try 
    {
        // Create a BIO buffer to read the keys and certificates from
        std::unique_ptr<BIO, decltype(&::BIO_free)> poBio(::BIO_new(BIO_s_mem()), ::BIO_free);
        _ThrowBaseExceptionIf((nullptr == poBio), "Creating BIO buffer failed.", nullptr);

        // Convert the private key from the PEM format to EVP_PKEY
        int nBytesWrittenToBio = ::BIO_write(poBio.get(), gc_abInitializerTlsPrivateKey, gc_unInitializerTlsPrivateKeySizeInBytes);
        _ThrowBaseExceptionIf((gc_unInitializerTlsPrivateKeySizeInBytes != nBytesWrittenToBio), "Writing to BIO buffer failed.", nullptr);

        ::PEM_read_bio_PrivateKey(poBio.get(), &poPrivateKey, 0, 0);

        long nBIOctrlStatus = ::BIO_ctrl(poBio.get(), BIO_CTRL_RESET, 0, nullptr);
        _ThrowBaseExceptionIf((1 != nBIOctrlStatus),"TLS failed: Write BIO reset failed", nullptr);

        // Convert the certificate from PEM to X509
        nBytesWrittenToBio = ::BIO_write(poBio.get(), gc_abInitializerTlsPublicKeyCertificate, gc_unInitializerTlsPublicKeyCertificateSizeInBytes);
        _ThrowBaseExceptionIf((gc_unInitializerTlsPublicKeyCertificateSizeInBytes != nBytesWrittenToBio), "Writing to BIO buffer failed.", nullptr);

        ::PEM_read_bio_X509(poBio.get(), &poX509Certificate, 0, 0);

        nBIOctrlStatus = ::BIO_ctrl(poBio.get(), BIO_CTRL_RESET, 0, nullptr);
        _ThrowBaseExceptionIf((1 != nBIOctrlStatus), "TLS failed: Write BIO reset failed", nullptr);

        int nSSLStatus = ::SSL_CTX_use_certificate(poSSL_CTX, poX509Certificate);
        _ThrowBaseExceptionIf((1 != nSSLStatus), "SSL_CTX_use_certificate failed\n", nullptr);

        nSSLStatus = ::SSL_CTX_use_PrivateKey(poSSL_CTX, poPrivateKey);
        _ThrowBaseExceptionIf((1 != nSSLStatus), "SSL_CTX_use_PrivateKey_file failed\n", nullptr);

        /* Make sure the key and certificate file match. */
        nSSLStatus = ::SSL_CTX_check_private_key(poSSL_CTX);
        _ThrowBaseExceptionIf((1 != nSSLStatus), "SSL_CTX_check_private_key failed\n", nullptr);

        // Free the pointers
        ::EVP_PKEY_free(poPrivateKey);
        ::X509_free(poX509Certificate);
    }
    catch (BaseException oException)
    {
        // Release the pointers and then throw the base exception 
        if (nullptr != poPrivateKey)
        {
            ::EVP_PKEY_free(poPrivateKey);
        }
        if (nullptr != poX509Certificate)
        {
            ::X509_free(poX509Certificate);
        }

        _ThrowBaseException(oException.GetExceptionMessage(), nullptr);
    }
    catch (...)
    {
        // Release the pointers and then throw the base exception 
        if (nullptr != poPrivateKey)
        {
            ::EVP_PKEY_free(poPrivateKey);
        }
        if (nullptr != poX509Certificate)
        {
            ::X509_free(poX509Certificate);
        }

        _ThrowBaseException("Unknown exception caught.", nullptr);
    }
}
