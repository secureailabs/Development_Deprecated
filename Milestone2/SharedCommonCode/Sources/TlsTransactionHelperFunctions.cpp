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
#include "TlsTransactionHelperFunctions.h"

#include <vector>
#include <iostream>

/********************************************************************************************/

std::vector<Byte> __stdcall GetTlsTransaction(
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    std::vector<Byte> stlSerializedTransactionBuffer;

    try
    {
        std::vector<Byte> stlTemporaryBuffer;

        stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), 10000);
        _ThrowBaseExceptionIf((sizeof(Qword) != stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
        Qword qwHeadMarker = *((Qword *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0xFFEEDDCCBBAA0099 != qwHeadMarker), "Invalid head marker encountered.", nullptr);
        stlTemporaryBuffer = poTlsNode->Read(sizeof(unsigned int), 10000);
        _ThrowBaseExceptionIf((sizeof(unsigned int) != stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
        unsigned int unSizeInBytesOfSerializedTransactionBuffer = *((unsigned int *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0 == unSizeInBytesOfSerializedTransactionBuffer), "There is no such thing as a zero(0) byte serialized structured buffer.", nullptr);
        do
        {
            // Don't worry about reading things in chunks and caching it since the TlsNode
            // object does it for us!!!
            stlSerializedTransactionBuffer = poTlsNode->Read(unSizeInBytesOfSerializedTransactionBuffer, 10000);
            _ThrowBaseExceptionIf(((0 != stlSerializedTransactionBuffer.size())&&(unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size())), "Failed to read data from the Tls tunnel", nullptr);
        }
        while (unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size());
        stlTemporaryBuffer = poTlsNode->Read(sizeof(Qword), 10000);
        _ThrowBaseExceptionIf((0 == stlTemporaryBuffer.size()), "Failed to read data from the Tls tunnel", nullptr);
        Qword qwTailMarker = *((Qword *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0x0123456789ABCDEF != qwTailMarker), "Invalid marker encountered.", nullptr);
    }

    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
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

    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
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
    _in const StructuredBuffer & c_oTransaction
    )
{
    __DebugFunction();

    _ThrowBaseExceptionIf((false == ::PutTlsTransaction(poTlsNode, c_oTransaction)), "Failed to send Tls transaction", nullptr);

    return ::GetTlsTransaction(poTlsNode);
}