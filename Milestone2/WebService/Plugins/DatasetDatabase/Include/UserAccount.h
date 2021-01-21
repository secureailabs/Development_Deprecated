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
        UserAccount(
            _in const std::string & c_strUserUuid,
            _in const std::string & c_strUserName,
            _in const std::string & c_strFullName,
            _in const std::string & c_strEmail,
            _in const std::string & c_strOrganization,
            _in const std::string & c_strContactInformation,
            _in Qword qwLastLogon,
            _in Word wTypeOfUser
            );
        virtual ~UserAccount(void);

        // Compare values of m_strUserName, m_strPassword, m_strOrganization of the other object with this object
        bool __thiscall IsUserEqual(
            _in const UserAccount & UserAccount
            ) const;

        // Methods use to check different types of user
        bool __thiscall IsOrganizationAdmin(void) const;
        bool __thiscall IsDatasetAdmin(void) const;
        bool __thiscall IsResearcher(void) const;

        // Accessor methods
        std::string __thiscall GetUserUuid(void) const throw();
        std::string __thiscall GetUserName(void) const throw();
        std::string __thiscall GetOrganization(void) const throw();
        std::string __thiscall GetFullName(void) const throw();
        std::string __thiscall GetEmail(void) const throw();
        std::string __thiscall GetContactInformation(void) const throw();
        Qword __thiscall GetLastLogon(void) const throw();
        Word __thiscall GetTypeOfUser(void) const throw();

    private:

        // Private data members
        std::string m_strUserUuid;
        std::string m_strUserName;
        std::string m_strPassword;
        // Organization name will be unique
        std::string m_strOrganization;
        // Add other data members to store additional user account information
        std::string m_strFullName;
        std::string m_strEmail;
        std::string m_strContactInformation;
        Qword m_qwLastLogon;
        Word m_wTypeOfUser;
};
