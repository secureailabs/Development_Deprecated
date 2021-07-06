/*********************************************************************************************
 *
 * @file DataAndTime.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

extern uint64_t __stdcall GetEpochTimeInSeconds(void) throw();

extern uint64_t __stdcall GetEpochTimeInMilliseconds(void) throw();