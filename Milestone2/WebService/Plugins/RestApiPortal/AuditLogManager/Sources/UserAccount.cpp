/*********************************************************************************************
 *
 * @file UserAccount.cpp
 * @author Shabana Akhtar Baig
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "UserAccount.h"

/********************************************************************************************
 *
 * @function ToUpper
 * @brief Returns the input string in lower case
 * @param[in] c_strString Input string
 * @return String in lower case
 *
 ********************************************************************************************/

extern "C" std::string __thiscall ToUpper(
    _in const std::string & c_strInput
    )
{
    __DebugFunction();

    std::string strOuput;
    for (unsigned int unIndex = 0; unIndex < c_strInput.length(); unIndex++)
    {
        strOuput[unIndex] = ::toupper(c_strInput[unIndex]);
    }

    return strOuput;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function UserAccount
 * @brief Constructor
 * @param[in] c_strUserName User name
 * @param[in] c_strPassword User password
 * @param[in] c_strOrganization User organization
 *
 ********************************************************************************************/

UserAccount::UserAccount(
    _in const std::string & c_strUserName,
    _in const std::string & c_strPassword,
    _in const std::string & c_strOrganization
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strUserName.length());
    __DebugAssert(0 != c_strPassword.length());
    __DebugAssert(0 != c_strOrganization.length());

    m_strUserName = c_strUserName;
    m_strPassword = c_strPassword;
    m_strOrganization = c_strOrganization;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function UserAccount
 * @brief Constructor
 * @param[in] c_strUserUuid User uuid
 * @param[in] c_strUserName User name
 * @param[in] c_strPassword User password
 * @param[in] c_strOrganization User organization
 *
 ********************************************************************************************/

UserAccount::UserAccount(
    _in const std::string & c_strUserUuid,
    _in const std::string & c_strUserName,
    _in const std::string & c_strPassword,
    _in const std::string & c_strOrganization
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strUserUuid.length());
    __DebugAssert(0 != c_strUserName.length());
    __DebugAssert(0 != c_strPassword.length());
    __DebugAssert(0 != c_strOrganization.length());

    m_strUserUuid = c_strUserUuid;
    m_strUserName = c_strUserName;
    m_strPassword = c_strPassword;
    m_strOrganization = c_strOrganization;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function ~UserAccount
 * @brief Destructor
 *
 ********************************************************************************************/

UserAccount::~UserAccount(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function IsUserEqual
 * @brief Compare if equal
 * @param[in] c_oUserAccount Instance of a PluginVersion object to compare with
 * @return true if username, password, and organization are the same
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall UserAccount::IsUserEqual(
    _in const UserAccount & c_oUserAccount
    ) const
{
    __DebugFunction();

    bool fEqual = false;

    if ((::ToUpper(m_strUserName) == ::ToUpper(c_oUserAccount.m_strUserName)) && (m_strPassword == c_oUserAccount.m_strPassword) && (::ToUpper(m_strOrganization) == ::ToUpper(c_oUserAccount.m_strOrganization)))
    {
        fEqual = true;
    }

    return fEqual;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetUserUuid
 * @brief Fetch user uuid
 * @return User uuid
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetUserUuid() const throw()
{
    __DebugFunction();

    return m_strUserUuid;
}
