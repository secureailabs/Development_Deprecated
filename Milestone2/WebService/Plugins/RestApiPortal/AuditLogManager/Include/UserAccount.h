/*********************************************************************************************
 *
 * @file UserAccount.h
 * @author Shabana Akhtar Baig
 * @date 16 Nov 2020
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

class UserAccount : public Object
{
    public:

        // Constructors and destructor
        UserAccount(
            _in const std::string & c_strUserName,
            _in const std::string & c_strPassword,
            _in const std::string & c_strOrganization
            );

        UserAccount(
            _in const std::string & c_strUserUuid,
            _in const std::string & c_strUserName,
            _in const std::string & c_strPassword,
            _in const std::string & c_strOrganization
            );
        virtual ~UserAccount(void);

        // Compare values of m_strUserName, m_strPassword, m_strOrganization of the other object with this object
        bool __thiscall IsUserEqual(
            _in const UserAccount & UserAccount
            ) const;

        // Fetch user uuid
        std::string __thiscall GetUserUuid() const throw();

    private:

        // Private data members
        std::string m_strUserUuid;
        std::string m_strUserName;
        std::string m_strPassword;
        // Organization name will be unique
        std::string m_strOrganization;
};
