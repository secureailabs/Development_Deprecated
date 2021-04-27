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
#include "Base64Encoder.h"

#include <vector>
#include <iostream>
#include <sstream>

/********************************************************************************************/

bool PutResponse(
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

std::vector<Byte> GetPayload(
    TlsNode * poTlsNode,
    unsigned int unMillisecondTimeout
)
{
    __DebugFunction();

    std::string strPayload;

    // Check whether the read was successful or not
    bool fIsEndOfHeader = false;
    while (false == fIsEndOfHeader)
    {
        std::vector<Byte> stlBuffer = poTlsNode->Read(1, unMillisecondTimeout);
        if (0 < stlBuffer.size())
        {
            strPayload.push_back(stlBuffer.at(0));
            if (4 <= strPayload.size())
            {
                if (("\r\n\r\n" == std::string(strPayload.end() - 4, strPayload.end())) || ("\n\r\n\r" == std::string(strPayload.end() - 4, strPayload.end())))
                {
                    fIsEndOfHeader = true;
                }
            }
        }
    }

    // Get the length of the header
    std::istringstream oStringStream(strPayload);
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
    std::vector<Byte> stlFileToDownload = poTlsNode->Read(unSizeOfPayload, unMillisecondTimeout);
    _ThrowBaseExceptionIf((unSizeOfPayload != stlFileToDownload.size()), "Read over Tls failed. Timeout", nullptr);
    stlFileToDownload.push_back(0);
    std::vector stlResponseDecodedBuffer = ::Base64Decode((char *)stlFileToDownload.data());
    return stlResponseDecodedBuffer;
}

/********************************************************************************************/

std::vector<Byte> __stdcall GetTlsTransaction(
    _in TlsNode * poTlsNode,
    _in unsigned int unMillisecondTimeout
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    std::vector<Byte> stlSerializedTransactionBuffer;

    try
    {
        std::vector<Byte> stlTemporaryBuffer;

        stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), unMillisecondTimeout);
        _ThrowBaseExceptionIf((sizeof(Qword) != stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
        Qword qwHeadMarker = *((Qword *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0xFFEEDDCCBBAA0099 != qwHeadMarker), "Invalid head marker encountered.", nullptr);
        stlTemporaryBuffer = poTlsNode->Read(sizeof(unsigned int), unMillisecondTimeout);
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
        stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), unMillisecondTimeout);
        _ThrowBaseExceptionIf((0 == stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
        Qword qwTailMarker = *((Qword *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0x0123456789ABCDEF != qwTailMarker), "Invalid marker encountered.", nullptr);
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return stlSerializedTransactionBuffer;
}

/********************************************************************************************/

bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const std::vector<Byte> c_stlSerializedTransaction
    ) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    bool fSuccess = false;

    try
    {
        // If there is nothing to send, do nothing.
        if (0 < c_stlSerializedTransaction.size())
        {
            // Only send an answer if there is a response buffer that is properly filled in.
            // Otherwise, the query was not legal and we want to just close the socket down.

            Qword qwHeadMarker = 0xFFEEDDCCBBAA0099;
            Qword qwTailMarker = 0x0123456789ABCDEF;
            unsigned int unSizeInBytesOfSerializedBuffer = (unsigned int) c_stlSerializedTransaction.size();
            unsigned int unNumberOfBytesWritten = 0;

            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &qwHeadMarker, sizeof(qwHeadMarker));
            _ThrowBaseExceptionIf((sizeof(qwHeadMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &unSizeInBytesOfSerializedBuffer, sizeof(unSizeInBytesOfSerializedBuffer));
            _ThrowBaseExceptionIf((sizeof(unSizeInBytesOfSerializedBuffer) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = poTlsNode->Write((const Byte *) c_stlSerializedTransaction.data(), unSizeInBytesOfSerializedBuffer);
            _ThrowBaseExceptionIf((unSizeInBytesOfSerializedBuffer != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            unNumberOfBytesWritten = (unsigned int) poTlsNode->Write((const Byte *) &qwTailMarker, sizeof(qwTailMarker));
            _ThrowBaseExceptionIf((sizeof(qwTailMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Tls tunnel", nullptr);
            fSuccess = true;
        }
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const StructuredBuffer & c_oTransaction
    ) throw()
{
    __DebugFunction();

    return PutTlsTransaction(poTlsNode, c_oTransaction.GetSerializedBuffer());
}

/********************************************************************************************/

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