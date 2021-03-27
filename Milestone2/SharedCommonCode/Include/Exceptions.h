/*********************************************************************************************
 *
 * @file Exceptions.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

/********************************************************************************************/

class BaseException
{
    public:

        BaseException(
            _in const char *c_szFilename,
            _in const char *c_szFunctionName,
            _in unsigned int unLineNumber,
            _in const char *c_szExceptionFormat,
           ... );
        BaseException(
            _in const BaseException & c_oBaseException
            );
        ~BaseException( void );

        const char * GetFilename(void) const;
        const char * GetFunctionName(void) const;
        unsigned int GetLineNumber(void) const;
        const char * GetExceptionMessage(void) const;

    protected:

        const char * m_szFilename;
        const char * m_szFunctionName;
        unsigned int m_unLineNumber;
        char * m_szExceptionMessage;
};

/********************************************************************************************/

#define _ThrowSimpleException(Message) throw BaseException(__FILE__, __func__, __LINE__, Message)
#define _ThrowBaseException(Format, ...) throw BaseException(__FILE__, __func__, __LINE__, Format, __VA_ARGS__)
#define _ThrowBaseExceptionIf(Condition, Format, ...) if (Condition) throw BaseException(__FILE__, __func__, __LINE__, Format, __VA_ARGS__)
#define _ThrowIndexOutOfRangeException() throw BaseException(__FILE__, __func__, __LINE__, "Index out of range!!!")
#define _ThrowInvalidPointerException(Pointer) throw BaseException(__FILE__, __func__, __LINE__, "Invalid pointer (%p)!!!", Pointer)
#define _ThrowOutOfMemoryException() throw 0x8007000E
#define _ThrowOutOfMemoryExceptionIfNull(Value) if (nullptr == Value) throw 0x8007000E
#define _ThrowIfFailed(Function, Message, ...)	if (0 != (errno = Function)) throw BaseException(__FILE__, __func__, __LINE__, Message, __VA_ARGS__)
#define _ThrowIfNull(Value, Message, ...) if (nullptr == Value) throw BaseException(__FILE__, __func__, __LINE__, Message, __VA_ARGS__)