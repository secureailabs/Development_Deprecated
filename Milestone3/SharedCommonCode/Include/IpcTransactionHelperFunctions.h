/*********************************************************************************************
 *
 * @file IpcHelperFunctions.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/
 
#include "CoreTypes.h"
#include "Socket.h"
#include "StructuredBuffer.h"

extern std::vector<Byte> __stdcall GetIpcTransaction(
    _in Socket * poSocket
    );
extern bool __stdcall PutIpcTransaction(
    _in Socket * poSocket,
    _in const std::vector<Byte> c_stlSerializedTransaction
    ) throw();
extern bool __stdcall PutIpcTransaction(
    _in Socket * poSocket,
    _in const StructuredBuffer & c_oTransaction
    ) throw();
extern std::vector<Byte> __stdcall PutIpcTransactionAndGetResponse(
    _in Socket * poSocket,
    _in const StructuredBuffer & c_oTransaction
    );