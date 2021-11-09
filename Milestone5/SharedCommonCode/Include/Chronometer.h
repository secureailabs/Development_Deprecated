/*********************************************************************************************
 *
 * @file Chronometer.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include <time.h>

typedef enum
{
    Year = 0,
    Day = 1,
    Hour = 2,
    Minute = 3,
    Second = 4,
    Millisecond = 5,
    Microsecond = 6,
    Nanosecond = 7

} ChronometerPrecision;

//////////////////////////////////////////////////////////////////////////////////////////

class Chronometer : public Object
{
    public:

        Chronometer(void) throw();
        ~Chronometer(void) throw();

        void __thiscall Start(void) throw();
        void __thiscall Stop(void) throw();
        void __thiscall Reset(void) throw();
        double __thiscall GetElapsedTimeWithPrecision(
            _in ChronometerPrecision eChronometerPrecision
            ) const throw();

    private:

        unsigned long m_un64StartTime;
        unsigned long m_un64ElapsedTime;
};