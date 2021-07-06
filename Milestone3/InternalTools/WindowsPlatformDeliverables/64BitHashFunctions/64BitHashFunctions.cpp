/*********************************************************************************************
 *
 * @file SailWebApiFunctions.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Guid class that handles 128 bit (16 bytes) GUID/UUID values.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "64BitHashes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

/// <summary>
/// 
/// </summary>
/// <param name="c_szString"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) uint64_t __cdecl Get64BitHashOfString(
    _in const char * c_szValue,
    _in bool fCaseSensitive
    )
{
    __DebugFunction();

    // [DllImport("64BitHashFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public ulong Get64BitHashOfString(string value, bool caseSensitive);

    uint64_t un64HashValue = 0;

    try
    {
        un64HashValue = ::Get64BitHashOfNullTerminatedString(c_szValue, fCaseSensitive);
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return un64HashValue;
}