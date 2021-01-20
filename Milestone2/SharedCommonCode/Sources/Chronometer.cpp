/*********************************************************************************************
 *
 * @file Chronometer.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 * @brief Implementation of a multiple precision chronometer
 *
 ********************************************************************************************/

#include "Chronometer.h"
#include "DebugLibrary.h"
#include <stdio.h>

/********************************************************************************************/

Chronometer::Chronometer(void) throw()
{
    __DebugFunction();
    
    m_un64StartTime = 0xFFFFFFFFFFFFFFFF;
    m_un64ElapsedTime = 0;
}

/********************************************************************************************/

Chronometer::~Chronometer(void) throw()
{
    __DebugFunction();
    
    m_un64StartTime = 0xFFFFFFFFFFFFFFFF;
    m_un64ElapsedTime = 0;
}

/********************************************************************************************/

void __thiscall Chronometer::Start(void) throw()
{
    __DebugFunction();
    
    struct timespec sCurrentTime;

    if ((0xFFFFFFFFFFFFFFFF == m_un64StartTime)&&(0 == m_un64ElapsedTime)&&(0 == ::clock_gettime(CLOCK_MONOTONIC_RAW, &sCurrentTime)))
    {
        m_un64StartTime = (sCurrentTime.tv_sec * 1000000000) + sCurrentTime.tv_nsec;
    }
}

/********************************************************************************************/

void __thiscall Chronometer::Stop(void) throw()
{
    __DebugFunction();
    
    if (0xFFFFFFFFFFFFFFFF != m_un64StartTime)
    {
        __DebugAssert(0 == m_un64ElapsedTime);
        
        struct timespec sCurrentTime;

        if (0 == ::clock_gettime(CLOCK_MONOTONIC_RAW, &sCurrentTime))
        {
            m_un64ElapsedTime = ((sCurrentTime.tv_sec * 1000000000) + sCurrentTime.tv_nsec) - m_un64StartTime;
            m_un64StartTime = 0xFFFFFFFFFFFFFFFF;
        }
    }
}

/********************************************************************************************/

void __thiscall Chronometer::Reset(void) throw()
{
    __DebugFunction();
    
    m_un64StartTime = 0xFFFFFFFFFFFFFFFF;
    m_un64ElapsedTime = 0;
}

/********************************************************************************************/

double __thiscall Chronometer::GetElapsedTimeWithPrecision(
    _in ChronometerPrecision eChronometerPrecision
    ) const throw()
{
    __DebugFunction();
    
    double dbElapsedTimeWithPrecision = (double) 0xFFFFFFFFFFFFFFFF;
    unsigned long un64ElapsedTime = (unsigned long) 0xFFFFFFFFFFFFFFFF;

    // It is possible for this method to be called when the chronometer is stopped or when
    // it is running. If it is running, we must get a temporary reading and use this
    // to return the most accurate amount of elapsed time
    if (0xFFFFFFFFFFFFFFFF == m_un64StartTime)
    {
        un64ElapsedTime = m_un64ElapsedTime;
    }
    else
    {
        struct timespec sCurrentTime;

        if (0 == ::clock_gettime(CLOCK_MONOTONIC_RAW, &sCurrentTime))
        {
            un64ElapsedTime = ((sCurrentTime.tv_sec * 1000000000) + sCurrentTime.tv_nsec) - m_un64StartTime;
        }
    }

    switch(eChronometerPrecision)
    {
        case Year
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 31546000000000000.0;
            break;
        case Day
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 86400000000000.0;
            break;
        case Hour
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 3600000000000.0;
            break;
        case Minute
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 600000000000.0;
            break;
        case Second
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 1000000000.0;
            break;
        case Millisecond
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 1000000.0;
            break;
        case Microsecond
        :   dbElapsedTimeWithPrecision = ((double) un64ElapsedTime) / 1000.0;
            break;
        case Nanosecond
        :    dbElapsedTimeWithPrecision = ((double) un64ElapsedTime);
            break;
    }

    return dbElapsedTimeWithPrecision;
}
