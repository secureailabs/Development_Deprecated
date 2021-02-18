/*********************************************************************************************
 *
 * @file VmInstance.h
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "DebugLibrary.h"

#include <iostream>
#include <string>

/********************************************************************************************/

class VmInstance : public Object
{
    public:

        // Constructor and destructor
        VmInstance(
            _in const std::string & c_strVmInstanceUuid,
            _in const std::string & c_strVmInstanceIpAddress,
            _in Word wVmInstancePortNumber,
            _in bool fVmInstanceStatus,
            _in Qword qwVmInstanceRegistrationDate,
            _in Qword qwVmInstanceUpTime
            );
        virtual ~VmInstance(void);

        // Accessor methods
        std::string __thiscall GetVmInstanceUuid(void) const throw();
        std::string __thiscall GetVmInstanceIpAddress(void) const throw();
        Word __thiscall GetVmInstancePortNumber(void) const throw();
        bool __thiscall GetVmInstanceStatus(void) const throw();
        Qword __thiscall GetVmInstanceRegistrationDate(void) const throw();
        Qword __thiscall GetVmInstanceUpTime(void) const throw();

    private:

        // Private data members
        std::string m_strVmInstanceUuid;
        std::string m_strVmInstanceIpAddress;
        Word m_wVmInstancePortNumber;
        bool m_fVmInstanceIsActive;
        Qword m_qwVmInstanceRegistrationDate;
        Qword m_qwVmInstanceUpTime;
};
