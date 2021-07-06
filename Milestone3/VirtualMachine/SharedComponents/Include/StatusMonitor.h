/*********************************************************************************************
 *
 * @file StatusMonitor.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"
#include "Object.h"

#include <string>

/********************************************************************************************/

class StatusMonitor : public Object
{
    public:
    
        StatusMonitor(
            _in const char * c_szDescription
            );
        virtual ~StatusMonitor(void);
        
        bool __thiscall IsTerminating(void) const throw();
        void __thiscall SignalTermination(
            _in const char * c_szReason
            ) const throw();
        void __thiscall UpdateStatus(
            _in const char * c_szFormat,
            ...
            ) const throw();
            
    private:
    
        Guid m_oStatusMonitorIdentifier;
};