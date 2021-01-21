/*********************************************************************************************
 *
 * @file Dataset.h
 * @author Shabana Akhtar Baig
 * @date 18 Nov 2020
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

class Dataset : public Object
{
    public:

        // Constructor and destructor
        Dataset(
            _in const std::string & c_strDatasetUuid,
            _in const std::string & c_strDatasetName,
            _in const std::string & c_strDatasetOrganization,
            _in const std::string & c_strDatasetSubmittedBy,
            _in Qword qwDatasetSubmissionDate
            );
        virtual ~Dataset(void);

        // Accessor methods
        std::string __thiscall GetDatasetUuid(void) const throw();
        std::string __thiscall GetDatasetName(void) const throw();
        std::string __thiscall GetDatasetOrganization(void) const throw();
        std::string __thiscall GetDatasetSubmittedBy(void) const throw();
        Qword __thiscall GetDatasetSubmissionDate(void) const throw();

    private:

        // Private data members
        std::string m_strDatasetUuid;
        std::string m_strDatasetName;
        std::string m_strDatasetOrganization;
        std::string m_strDatasetSubmittedBy;
        Qword m_qwDatasetSubmissionDate;
};
