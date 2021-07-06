/*********************************************************************************************
 *
 * @file DataAndTime.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DateAndTime.h"
#include "DebugLibrary.h"
#include <time.h>

/********************************************************************************************/

uint64_t __stdcall GetEpochTimeInSeconds(void) throw()
{
	__DebugFunction();
    
    return (uint64_t) ::time(nullptr);
}

/********************************************************************************************/

uint64_t __stdcall GetEpochTimeInMilliseconds(void) throw()
{
	__DebugFunction();
    
    return (uint64_t) ::time(nullptr) * 1000;
}
