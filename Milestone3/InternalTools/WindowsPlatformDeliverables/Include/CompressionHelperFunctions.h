//////////////////////////////////////////////////////////////////////////////////////////////
//
// @file CompressionHelperFunctions.h
// @author David Gascon
// @date 25 October 2021
// @License Private and Confidential. Internal Use Only.
// @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "CoreTypes.h"
#include "StructuredBuffer.h"

/********************************************************************************************/
extern StructuredBuffer CompressToStructuredBuffer(
    const void* c_pbRawBytes,
    uint64_t unRawBufferSizeInBytes
) throw();

/********************************************************************************************/
std::vector<Byte> DecompressStructuredBuffer(
    const StructuredBuffer& compressedBuffer
) throw();