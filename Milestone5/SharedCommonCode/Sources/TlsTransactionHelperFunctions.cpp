/*********************************************************************************************
 *
 * @file TlsHelperFunctions.cpp
 * @author Prawal Gangwar
 * @date 18 Feb 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "TlsTransactionHelperFunctions.h"
#include "HttpRequestParser.h"
#include "JsonValue.h"

#include <vector>
#include <iostream>
#include <sstream>

/********************************************************************************************
 *
 * @function GetTlsTransaction
 * @brief Function used to get a Tls data packet over network
 * @param[in] poTlsNode Pointer to the TlsNode object connected to remote node
 * @param[in] unMillisecondTimeout The timeout for the start of the packet. It should be
 *              noted that the timout is not the time it will spend to read, this is the
 *              time spent waiting for the first byte of this transaction.
 *              If this value is 0, the call waits for a packet indefinitely
 * @return Byte vector containing the data read.
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall GetTlsTransaction(
    _in TlsNode * poTlsNode,
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    std::vector<Byte> stlSerializedTransactionBuffer;

    try
    {
        // If the value of timeout is zero the call is blocking and checks for data every
        // 1 second otherwise it will wait for data as per the value passed.
        bool fIsBlocking = false;
        if (0 == unMillisecondTimeout)
        {
            fIsBlocking = true;
            unMillisecondTimeout = 1000;
        }

        bool fIsEndOfHeader = false;
        bool fReadTimedout = false;
        std::vector<Byte> stlHeaderData;
        do
        {
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, unMillisecondTimeout);
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else if (false == fIsBlocking)
            {
                fReadTimedout = true;
            }
        }
        while ((false == fIsEndOfHeader) && (false == fReadTimedout));

        if (false == fReadTimedout)
        {
            // The same function call to GetTlsTransaction will take care of both HTTP and
            // StructuredBuffer packets. In both the cases the body of the message is a byte buffer
            // SSB = 'Serialized Structured Buffer'
            if ("SSB PROTOCOL\r\n\r\n" == std::string(stlHeaderData.begin(), stlHeaderData.end()))
            {
                std::vector<Byte> stlTemporaryBuffer;

                stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), 1000);
                _ThrowBaseExceptionIf((sizeof(Qword) != stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
                Qword qwHeadMarker = *((Qword *) stlTemporaryBuffer.data());
                _ThrowBaseExceptionIf((0xFFEEDDCCBBAA0099 != qwHeadMarker), "Invalid head marker encountered.", nullptr);

                // The timout here is again set to unMillisecondTimeout as the size of the data packet could be huge
                stlTemporaryBuffer = poTlsNode->Read(sizeof(unsigned int), 1000);
                _ThrowBaseExceptionIf((sizeof(unsigned int) != stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);

                unsigned int unSizeInBytesOfSerializedTransactionBuffer = *((unsigned int *) stlTemporaryBuffer.data());
                _ThrowBaseExceptionIf((0 == unSizeInBytesOfSerializedTransactionBuffer), "There is no such thing as a zero(0) byte serialized structured buffer.", nullptr);
                do
                {
                    // Don't worry about reading things in chunks and caching it since the TlsNode
                    // object does it for us!!!
                    stlSerializedTransactionBuffer = poTlsNode->Read(unSizeInBytesOfSerializedTransactionBuffer, unMillisecondTimeout);
                    _ThrowBaseExceptionIf(((0 != stlSerializedTransactionBuffer.size())&&(unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size())), "Failed to read data from the Tls tunnel", nullptr);
                }
                while (unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size());

                stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), 1000);
                _ThrowBaseExceptionIf((0 == stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
                Qword qwTailMarker = *((Qword *) stlTemporaryBuffer.data());
                _ThrowBaseExceptionIf((0x0123456789ABCDEF != qwTailMarker), "Invalid marker encountered.", nullptr);
            }
            else
            {
                // Get the length of the data from the header by reading each line of the header
                std::string strPayLoadHeader(stlHeaderData.begin(), stlHeaderData.end());
                std::istringstream oStringStream(strPayLoadHeader);
                std::string strTempLine;
                std::string strLineWithKey;
                while (std::getline(oStringStream, strTempLine))
                {
                    if (strTempLine.find("Content-Length") != std::string::npos)
                    {
                        strLineWithKey = strTempLine;
                        break;
                    }
                }
                std::string strStartOfValue = strLineWithKey.substr(strLineWithKey.find(": ")+2);
                unsigned int unSizeOfPayload = std::stoi(strStartOfValue.c_str());
                std::cout << "Content length " << unSizeOfPayload << std::endl;

                // Read the data as the content lenght is now known
                std::vector<Byte> stlFileToDownload = poTlsNode->Read(unSizeOfPayload, unMillisecondTimeout);
                _ThrowBaseExceptionIf((unSizeOfPayload != stlFileToDownload.size()), "Read over Tls failed. Timeout", nullptr);
                stlFileToDownload.push_back(0);
                stlSerializedTransactionBuffer = JsonValue::ParseDataToStructuredBuffer((char *)stlFileToDownload.data());
            }
        }
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return stlSerializedTransactionBuffer;
}

/********************************************************************************************
 *
 * @function PutTlsTransaction
 * @brief Function used to send a Tls data packet over network
 * @param[in] poTlsNode Pointer to the TlsNode object connected to remote node
 * @param[in] c_stlSerializedBuffer The Byte vector to send
 * @return true on success, false otherwise
 *
 ********************************************************************************************/

bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const std::vector<Byte> & c_stlSerializedBuffer
) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    bool fSuccess = false;

    try
    {
        // If there is nothing to send, do nothing.
        if (0 < c_stlSerializedBuffer.size())
        {
            // Only send an answer if there is a response buffer that is properly filled in.
            // Otherwise, the query was not legal and we want to just close the socket down.
            Qword qwHeadMarker = 0xFFEEDDCCBBAA0099;
            Qword qwTailMarker = 0x0123456789ABCDEF;
            unsigned int unSizeInBytesOfSerializedBuffer = (unsigned int) c_stlSerializedBuffer.size();
            unsigned int unNumberOfBytesWritten = 0;

            // Send the protocol type first such that it can be differentiated from an HTTP packet
            std::string strStrucutredBufferProtocolHeader = "SSB PROTOCOL\r\n\r\n";
            unNumberOfBytesWritten = (unsigned int)poTlsNode->Write((const Byte *)strStrucutredBufferProtocolHeader.c_str(), strStrucutredBufferProtocolHeader.length());
            _ThrowBaseExceptionIf((strStrucutredBufferProtocolHeader.length() != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);

            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &qwHeadMarker, sizeof(qwHeadMarker));
            _ThrowBaseExceptionIf((sizeof(qwHeadMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &unSizeInBytesOfSerializedBuffer, sizeof(unSizeInBytesOfSerializedBuffer));
            _ThrowBaseExceptionIf((sizeof(unSizeInBytesOfSerializedBuffer) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = poTlsNode->Write((const Byte *) c_stlSerializedBuffer.data(), unSizeInBytesOfSerializedBuffer);
            _ThrowBaseExceptionIf((unSizeInBytesOfSerializedBuffer != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &qwTailMarker, sizeof(qwTailMarker));
            _ThrowBaseExceptionIf((sizeof(qwTailMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            fSuccess = true;
        }
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************
 *
 * @function PutTlsTransactionAndGetResponse
 * @brief Function used to send a Tls data packet over network and receive a response
 * @param[in] poTlsNode Pointer to the TlsNode object connected to remote node
 * @param[in] c_oTransaction StructuredBuffer with the request
 * @param[in] unMillisecondTimeout The timeout for the start of the packet. It should be
 *              noted that the timout is not the time it will spend to read, this is the
 *              time spent waiting for the first byte of this transaction.
 *              If this value is 0, the call waits for a packet indefinitely
 * @return Byte vector containing the data read.
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall PutTlsTransactionAndGetResponse(
    _in TlsNode * poTlsNode,
    _in const StructuredBuffer & c_oTransaction,
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();

    _ThrowBaseExceptionIf((false == ::PutTlsTransaction(poTlsNode, c_oTransaction)), "Failed to send Tls transaction", nullptr);

    return ::GetTlsTransaction(poTlsNode, unMillisecondTimeout);
}

/********************************************************************************************
 *
 * @function PutTlsTransaction
 * @brief Function used to send a Tls data packet over network
 * @param[in] poTlsNode Pointer to the TlsNode object connected to remote node
 * @param[in] c_oTransaction StructuredBuffer with the request
 * @return true on success, otherwise false
 *
 ********************************************************************************************/

bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const StructuredBuffer & c_oTransaction
    ) throw()
{
    __DebugFunction();

    return ::PutTlsTransaction(poTlsNode, c_oTransaction.GetSerializedBuffer());
}

/********************************************************************************************/

/********************************************************************************************
 *
 * @function PutHttpResponse
 * @brief Temporary Function used to send a HTTP type Tls data packet over network
 * @param[in] poTlsNode Pointer to the TlsNode object connected to remote node
 * @param[in] stlPayload JSON type data to send
 * @return true on success, otherwise false
 *
 ********************************************************************************************/

bool PutHttpResponse(
    TlsNode * poTlsNode,
    const std::string & stlPayload
)
{
    __DebugFunction();

    std::string strResponseHeader = "HTTP/1.1 200 OK \r\nContent-Length: " + std::to_string(stlPayload.length()) + "\r\nConnection: close\r\nContent-Type: application/json\r\n\r\n";
    std::string strResponseData(strResponseHeader);
    strResponseData += stlPayload;
    std::cout << "\n\nRest Response:\n\n" << strResponseData << std::endl;

    // Send back response data
    poTlsNode->Write((const Byte *) strResponseData.data(), strResponseData.size());

    return true;
}

/********************************************************************************************/