/*********************************************************************************************
 *
 * @file DigitalContract.h
 * @author Shabana Akhtar Baig
 * @date 23 Nov 2020
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
#include <vector>

/********************************************************************************************/

class DigitalContract : public Object
{
    public:

        // Constructor and destructor
        DigitalContract(
            _in const std::string & c_strDigitalContractUuid,
            _in const std::string & c_strDataOwnerOrganization,
            _in const std::string & c_strResearcherOrganization,
            _in const std::string & c_strInvolvedDatasets,
            _in Qword qwCreationDate,
            _in Qword qwExpirationDate
            );
        virtual ~DigitalContract(void);

        // Accessor methods
        std::string __thiscall GetDigitalContractUuid(void) const throw();
        std::string __thiscall GetDataOwnerOrganization(void) const throw();
        std::string __thiscall GetResearcherOrganization(void) const throw();
        std::string __thiscall GetInvolvedDatasets(void) const throw();
        Qword __thiscall GetCreationDate(void) const throw();
        Qword __thiscall GetExpirationDate(void) const throw();
        bool __thiscall IsDigitalContractApproved(void) const throw();

    private:

        // Private data members
        std::string m_strDigitalContractUuid;
        std::string m_strDataOwnerOrganization;
        std::string m_strResearcherOrganization;
        std::string m_strInvolvedDatasets;
        Qword m_qwCreationDate;
        Qword m_qwExpirationDate;
        bool m_fIsApproved;
};
