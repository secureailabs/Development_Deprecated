/*********************************************************************************************
 *
 * @file BaseException.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Root class for all exceptions
 *
 * All exceptions within our code should be derived from the BaseException class or should use
 * instances of the BaseException class.
 *
 ********************************************************************************************/

#include "Exceptions.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/********************************************************************************************
 *
 * @class BaseException
 * @function BaseException
 * @param[in] c_szFilename Name of the file containing the function which threw the exception
 * @param[in] c_szFunctionName Name of the function which threw the exception
 * @param[in] unLineNumber Location, as a line number within the @p c_szFilename, of the statement which threw the exception
 * @param[in] c_szExceptionFormat Format of the variadic string to be constructed
 * @param[in] ... Variadic parameters that will be used by @p c_szExceptionFormat
 * @brief BaseException class constructor
 *
 ********************************************************************************************/
#include <iostream>

BaseException::BaseException(
	_in const char * c_szFilename,
	_in const char * c_szFunctionName,
	_in unsigned int unLineNumber,
	_in const char * c_szExceptionFormat,
	_in ...
	)
{
	m_szFilename = c_szFilename;
	m_szFunctionName = c_szFunctionName;
	m_unLineNumber = unLineNumber;

	va_list pListOfArguments;
	va_start( pListOfArguments, c_szExceptionFormat );
	unsigned int unSizeInCharactersIncludingNull = ::vsnprintf( nullptr, 0, c_szExceptionFormat, pListOfArguments ) + 1;
	va_end(pListOfArguments);

	// Unlike most other components who will use the MemoryAllocation library, this component calls
	// malloc directly. This is to ensure that if the actual MemoryAllocation components throws
	// an exception, this doesn't lead to an infinite loop.
	va_start( pListOfArguments, c_szExceptionFormat );
	m_szExceptionMessage = (char *)::malloc(unSizeInCharactersIncludingNull * sizeof(char));
	if (nullptr != m_szExceptionMessage)
	{
		(void)::vsnprintf( m_szExceptionMessage, unSizeInCharactersIncludingNull, c_szExceptionFormat, pListOfArguments );
	}
	va_end(pListOfArguments);
}

/********************************************************************************************
 *
 * class BaseException
 * @function BaseException
 * @param[in] BaseException Instance of BaseException to copy from
 * @brief BaseException class copy constructor
 *
 ********************************************************************************************/

BaseException::BaseException(
	_in const BaseException & c_oBaseException
	)
{
	m_szFilename = c_oBaseException.m_szFilename;
	m_szFunctionName = c_oBaseException.m_szFunctionName;
	m_unLineNumber = c_oBaseException.m_unLineNumber;

	unsigned int unSizeInCharactersIncludingNull = (unsigned int)::strnlen(c_oBaseException.m_szExceptionMessage, 1024) + 1;
	// Unlike most other components who will use the MemoryAllocation library, this component calls
	// malloc directly. This is to ensure that if the actual MemoryAllocation components throws
	// an exception, this doesn't lead to an infinite loop. This is one of the few places in
    // the code where the function malloc() should be used instead of using the SmartMemoryAllocator
	m_szExceptionMessage = (char *)::malloc(unSizeInCharactersIncludingNull * sizeof(char));
	if (nullptr != m_szExceptionMessage)
	{
		::strncpy(m_szExceptionMessage, c_oBaseException.m_szExceptionMessage, unSizeInCharactersIncludingNull);
	}
}

/********************************************************************************************
 *
 * @class BaseException
 * @function ~BaseException
 * @brief BaseException class destructor
 *
 ********************************************************************************************/

BaseException::~BaseException(void)
{
	m_szFilename = nullptr;
	m_szFunctionName = nullptr;
	m_unLineNumber = 0;

	if (nullptr != m_szExceptionMessage)
	{
		::free((void *)m_szExceptionMessage);
		m_szExceptionMessage = nullptr;
	}
}

/********************************************************************************************
 *
 * @class BaseException
 * @function GetFilename
 * @brief Returns the name of the filename which contains the function which threw the exception
 * @return Name (string) of the file containing the function that threw the exception
 *
 ********************************************************************************************/

const char * BaseException::GetFilename(void) const
{
	return m_szFilename;
}

/********************************************************************************************
 *
 * @class BaseException
 * @function GetFunctionName
 * @brief Returns the name of the function which threw the exception
 * @return Name (string) of the function that threw the exception
 *
 ********************************************************************************************/

const char * BaseException::GetFunctionName(void) const
{
	return m_szFunctionName;
}

/********************************************************************************************
 *
 * @class BaseException
 * @function GetLineNumber
 * @brief Returns the line number of the statement which threw the exception
 * @return Line number within the file that contains the function that threw the exception
 *
 ********************************************************************************************/

unsigned int BaseException::GetLineNumber(void) const
{
	return m_unLineNumber;
}

/********************************************************************************************
 *
 * @class BaseException
 * @function GetExceptionMessage
 * @brief Returns the fully formatted exception message containing the specificity of the exception
 * @return String containing the fully formatted exception message
 *
 ********************************************************************************************/

const char * BaseException::GetExceptionMessage(void) const
{
	return m_szExceptionMessage;
}