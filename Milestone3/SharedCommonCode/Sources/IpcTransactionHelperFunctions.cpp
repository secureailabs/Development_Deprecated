/*********************************************************************************************
 *
 * @file IpcHelperFunctions.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "IpcTransactionHelperFunctions.h"

#include <vector>
#include <iostream>

/********************************************************************************************/

std::vector<Byte> __stdcall GetIpcTransaction(
    _in Socket * poSocket,
    _in bool fIsBlocking
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    std::vector<Byte> stlSerializedTransactionBuffer;

    try
    {
        std::vector<Byte> stlTemporaryBuffer;
        do
        {
            stlTemporaryBuffer = poSocket->Read(sizeof(Qword), 60000);
            if (0 < stlTemporaryBuffer.size())
            {
                fIsBlocking = false;
            }
        }
        while (true == fIsBlocking);
        _ThrowBaseExceptionIf((sizeof(Qword) != stlTemporaryBuffer.size()), "Failed to read data from the Ipc tunnel", nullptr);
        Qword qwHeadMarker = *((Qword *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0xFFEEDDCCBBAA0099 != qwHeadMarker), "Invalid head marker encountered.", nullptr);
        stlTemporaryBuffer = poSocket->Read(sizeof(unsigned int), 10000);
        _ThrowBaseExceptionIf((sizeof(unsigned int) != stlTemporaryBuffer.size()), "Failed to read data from the Ipc tunnel", nullptr);
        unsigned int unSizeInBytesOfSerializedTransactionBuffer = *((unsigned int *) stlTemporaryBuffer.data());
        _ThrowBaseExceptionIf((0 == unSizeInBytesOfSerializedTransactionBuffer), "There is no such thing as a zero(0) byte serialized structured buffer.", nullptr);
        do
        {
            // Don't worry about reading things in chunks and caching it since the TlsNode
            // object does it for us!!!
            stlSerializedTransactionBuffer = poSocket->Read(unSizeInBytesOfSerializedTransactionBuffer, 10000);
            _ThrowBaseExceptionIf(((0 != stlSerializedTransactionBuffer.size())&&(unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size())), "Failed to read data from the Ipc tunnel", nullptr);
        }
        while (unSizeInBytesOfSerializedTransactionBuffer != stlSerializedTransactionBuffer.size());
        stlTemporaryBuffer = poSocket->Read(sizeof(Qword), 10000);
        _ThrowBaseExceptionIf((0 == stlTemporaryBuffer.size()), "Failed to read data from the Ipc tunnel", nullptr);
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

bool __stdcall PutIpcTransaction(
    _in Socket * poSocket,
    _in const std::vector<Byte> c_stlSerializedTransaction
    ) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);
    
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
            unNumberOfBytesWritten = (unsigned int) poSocket->Write((const Byte *) &qwHeadMarker, sizeof(qwHeadMarker));
            _ThrowBaseExceptionIf((sizeof(qwHeadMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Ipc tunnel. errno: %d", errno);
            unNumberOfBytesWritten = (unsigned int) poSocket->Write((const Byte *) &unSizeInBytesOfSerializedBuffer, sizeof(unSizeInBytesOfSerializedBuffer));
            _ThrowBaseExceptionIf((sizeof(unSizeInBytesOfSerializedBuffer) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Ipc tunnelerrno: %d", errno);
            unNumberOfBytesWritten = poSocket->Write((const Byte *) c_stlSerializedTransaction.data(), unSizeInBytesOfSerializedBuffer);
            _ThrowBaseExceptionIf((unSizeInBytesOfSerializedBuffer != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Ipc tunnelerrno: %d", errno);
            unNumberOfBytesWritten = (unsigned int) poSocket->Write((const Byte *) &qwTailMarker, sizeof(qwTailMarker));
            _ThrowBaseExceptionIf((sizeof(qwTailMarker) != unNumberOfBytesWritten), "Failed to write the expected number of bytes into the Ipc tunnelerrno: %d", errno);
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

bool __stdcall PutIpcTransaction(
    _in Socket * poSocket,
    _in const StructuredBuffer & c_oTransaction
    ) throw()
{
    __DebugFunction();

    return PutIpcTransaction(poSocket, c_oTransaction.GetSerializedBuffer());
}

/********************************************************************************************/

std::vector<Byte> __stdcall PutIpcTransactionAndGetResponse(
    _in Socket * poSocket,
    _in const StructuredBuffer & c_oTransaction,
    _in bool fIsBlocking
    )
{
    __DebugFunction();

    _ThrowBaseExceptionIf((false == ::PutIpcTransaction(poSocket, c_oTransaction)), "Failed to send Ipc transaction", nullptr);

    return ::GetIpcTransaction(poSocket, fIsBlocking);
}
