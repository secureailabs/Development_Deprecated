/*********************************************************************************************
 *
 * @file DebugLibrary.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include <assert.h>
#include "CoreTypes.h"

#define __DebugFunction()
#define __DebugMessage(Format, ...)
#define __DebugError(Format, ...)
#define __DebugWarning(Format, ...)
#define __DebugAssert(Expression)                           assert(Expression);
#define __DebugCount(Name, Expression)                      Expression
#define __DebugElapsedTime(Name, Expression)                Expression
#define __DebugRecordUInt8(ValueName, Value, DebugOnly)
#define __DebugRecordUInt16(ValueName, Value, DebugOnly)
#define __DebugRecordUInt32(ValueName, Value, DebugOnly)
#define __DebugRecordUInt64(ValueName, Value, DebugOnly)
#define __DebugRecordInt8(ValueName, Value, DebugOnly)
#define __DebugRecordInt16(ValueName, Value, DebugOnly)
#define __DebugRecordInt32(ValueName, Value, DebugOnly)
#define __DebugRecordInt64(ValueName, Value, DebugOnly)
#define __DebugRecordByte(ValueName, Value, DebugOnly)
#define __DebugRecordWord(ValueName, Value, DebugOnly)
#define __DebugRecordDword(ValueName, Value, DebugOnly)
#define __DebugRecordQword(ValueName, Value, DebugOnly)
#define __DebugRecordFloat(ValueName, Value, DebugOnly)
#define __DebugRecordDouble(ValueName, Value, DebugOnly)
#define __DebugRecordBoolean(ValueName, Value, DebugOnly)