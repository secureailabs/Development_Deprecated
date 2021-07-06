/*********************************************************************************************
 *
 * @file DllMain.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Guid class that handles 128 bit (16 bytes) GUID/UUID values.
 *
 ********************************************************************************************/

#include "windows.h"
#include "CoreTypes.h"

/// <summary>
/// 
/// </summary>
/// <param name="hModule"></param>
/// <param name="ul_reason_for_call"></param>
/// <param name="lpReserved"></param>
/// <returns></returns>
bool __stdcall DllMain(
    _in HMODULE hModule,
    _in Dword  dwReasonForCall,
    _in void * lpReserved
    )
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReasonForCall)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}