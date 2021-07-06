/*********************************************************************************************
 *
 * @file ExceptionRegister.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementations of C functions used to generate quick 64 bit hashes.
 *
 * In some circumstances, it is really useful to use 64-bit hashes as opposed to larger
 * hashes like MD-5 or SHA-1 and SHA-2. The biggest use of such hashes are for indexing
 * element using STL objects, like std::map. Since 64 bit is a native size to the CPU, it
 * indexes a lot quicker within STL than if you used a larger buffer hosting a more
 * traditional hash.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Exceptions.h"

#include <string>

/********************************************************************************************/

extern void __cdecl RegisterException(
    _in const BaseException & c_oBaseException,
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    ) throw();
extern void __cdecl RegisterUnknownException(
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    ) throw();
extern unsigned int __cdecl GetRegisteredExceptionCount(void) throw();
extern std::string __cdecl GetNextRegisteredException(void) throw();