/*********************************************************************************************
 *
 * @file ExceptionRegister.cpp
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

#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"

#include <iostream>
#include <mutex>
#include <queue>
#include <string>

static std::queue<std::string> gs_stlQueueOfExceptions;
static std::mutex gs_stlMutex;

/// <summary>
/// Register a BaseException in the internal queue
/// </summary>
/// <param name="c_oBaseException"></param>
/// <param name="c_szFunctionName"></param>
/// <param name="unLineNumber"></param>
/// <returns></returns>
void __cdecl RegisterException(
    _in const BaseException & c_oBaseException,
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    ) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szFunctionName);

    try
    {
        std::string strExceptionMessage;

        strExceptionMessage = "Thrown from -> |File = ";
        strExceptionMessage += c_oBaseException.GetFilename();
        strExceptionMessage += "\r\n               |Function = ";
        strExceptionMessage += c_oBaseException.GetFunctionName();
        strExceptionMessage += "\r\n               |Line Number = ";
        strExceptionMessage += std::to_string(c_oBaseException.GetLineNumber());
        strExceptionMessage += "\r\n               |Message = ";
        strExceptionMessage += c_oBaseException.GetExceptionMessage();
        strExceptionMessage += "\r\nCaught in ---->|File = ";
        strExceptionMessage += __FILE__;
        strExceptionMessage += "\r\n               |Function = ";
        strExceptionMessage += c_szFunctionName;
        strExceptionMessage += "\r\n               |Line Number = ";
        strExceptionMessage += std::to_string(unLineNumber);

        // Use a lock_guard to make sure that if gs_stlQueueOfExceptions.push throws and
        // exception, the mutex gets unlocked automatically
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        // Push the new exception event onto the queue of exception events
        gs_stlQueueOfExceptions.push(strExceptionMessage);
        
        std::cout << strExceptionMessage << std::endl;
    }

    catch (...)
    {

    }
}

/// <summary>
/// Register an unknown exception in the internal queue
/// </summary>
/// <param name="c_szFunctionName"></param>
/// <param name="unLineNumber"></param>
/// <returns></returns>
void __cdecl RegisterUnknownException(
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    ) throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szFunctionName);

    try
    {
        std::string strExceptionMessage;

        strExceptionMessage = "UNKNOWN EXCEPTION!!!!!";
        strExceptionMessage += "\r\nCaught in ---->|File = ";
        strExceptionMessage += __FILE__;
        strExceptionMessage += "\r\n               |Function = ";
        strExceptionMessage += c_szFunctionName;
        strExceptionMessage += "\r\n               |Line Number = ";
        strExceptionMessage += std::to_string(unLineNumber);

        // Use a lock_guard to make sure that if gs_stlQueueOfExceptions.push throws and
        // exception, the mutex gets unlocked automatically
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        // Push the new exception event onto the queue of exception events
        gs_stlQueueOfExceptions.push(strExceptionMessage);
        
        std::cout << strExceptionMessage << std::endl;
    }

    catch (...)
    {

    }
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
unsigned int __cdecl GetRegisteredExceptionCount(void) throw()
{
    __DebugFunction();
    
    unsigned int unRegisteredExceptionCount = 0;

    try
    {
        // Use a lock_guard to make sure that if gs_stlQueueOfExceptions.push throws and
        // exception, the mutex gets unlocked automatically
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        unRegisteredExceptionCount = (unsigned int) gs_stlQueueOfExceptions.size();
    }

    catch (...)
    {

    }

    return unRegisteredExceptionCount;
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
std::string __cdecl GetNextRegisteredException(void) throw()
{
    __DebugFunction();

    std::string strNextRegisteredException = "";

    try
    {
        // Use a lock_guard to make sure that if gs_stlQueueOfExceptions.push throws and
        // exception, the mutex gets unlocked automatically
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        if (0 < gs_stlQueueOfExceptions.size())
        {
            strNextRegisteredException = gs_stlQueueOfExceptions.front();
            gs_stlQueueOfExceptions.pop();
        }
    }

    catch (...)
    {

    }

    return strNextRegisteredException;
}