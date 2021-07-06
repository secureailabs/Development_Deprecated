/*********************************************************************************************
 *
 * @file TlsHelperFunctions.h
 * @author Prawal Gangwar
 * @date 18 Feb 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/
 
#include "CoreTypes.h"
#include "TlsNode.h"
#include "StructuredBuffer.h"

extern std::vector<Byte> __stdcall GetPayload(
    TlsNode * poTlsNode,
    unsigned int unMillisecondTimeout
);
extern bool __stdcall PutResponse(
    TlsNode * poTlsNode,
    const std::string & stlPayload
);

extern std::vector<Byte> __stdcall GetTlsTransaction(
    _in TlsNode * poTlsNode,
    _in unsigned int unMillisecondTimeout
    );
extern bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const std::vector<Byte> c_stlSerializedTransaction
    ) throw();
extern bool __stdcall PutTlsTransaction(
    _in TlsNode * poTlsNode,
    _in const StructuredBuffer & c_oTransaction
    ) throw();
extern std::vector<Byte> __stdcall PutTlsTransactionAndGetResponse(
    _in TlsNode * poTlsNode,
    _in const StructuredBuffer & c_oTransaction,
    _in unsigned int unMillisecondTimeout
    );