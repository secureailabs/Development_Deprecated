/*********************************************************************************************
 *
 * @file UserAccount.cpp
 * @author Shabana Akhtar Baig
 * @date 18 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "UserAccount.h"

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
 * @function UserAccount
 * @brief Constructor
 * @param[in] c_strUserUuid User uuid
 * @param[in] c_strUserName Username
 * @param[in] c_strFullName Full name
 * @param[in] c_strEmail User email
 * @param[in] c_strOrganization User organization
 * @param[in] c_strContactInformation User contact information
 * @param[in] qwLastLogon User last logon time
 * @param[in] wTypeOfUser Type of user
 * @note
 *      TypeOfUser is a bit field since a user can have several roles (i.e. Organization Admin,
 *      Dataset Admin, and Researcher).
 ********************************************************************************************/

UserAccount::UserAccount(
    _in const std::string & c_strUserUuid,
    _in const std::string & c_strUserName,
    _in const std::string & c_strFullName,
    _in const std::string & c_strEmail,
    _in const std::string & c_strOrganization,
    _in const std::string & c_strContactInformation,
    _in Qword qwLastLogon,
    _in Word wTypeOfUser
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strUserUuid.length());
    __DebugAssert(0 != c_strUserName.length());
    __DebugAssert(0 != c_strFullName.length());
    __DebugAssert(0 != c_strEmail.length());
    __DebugAssert(0 != c_strOrganization.length());
    __DebugAssert(0 != c_strContactInformation.length());

    m_strUserUuid = c_strUserUuid;
    m_strUserName = c_strUserName;
    m_strFullName = c_strFullName;
    m_strEmail = c_strEmail;
    m_strOrganization = c_strOrganization;
    m_strContactInformation = c_strContactInformation;
    m_qwLastLogon = qwLastLogon;
    m_wTypeOfUser = wTypeOfUser;
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

    if ((m_strUserName == c_oUserAccount.m_strUserName) && (m_strPassword == c_oUserAccount.m_strPassword) && (m_strOrganization == c_oUserAccount.m_strOrganization))
    {
        fEqual = true;
    }

    return fEqual;
}

/********************************************************************************************
*
* @class UserAccount
* @function IsOrganizationAdmin
* @brief Check if user is OrganizationAdmin
* @return true if user is OrganizationAdmin
* @return false otherwise
*
********************************************************************************************/
bool __thiscall UserAccount::IsOrganizationAdmin(void) const
{
    __DebugFunction();

    bool fIsOrganizationAdmin = false;

    if (4 == (m_wTypeOfUser & 0x04))
    {
        fIsOrganizationAdmin = true;
    }

    return fIsOrganizationAdmin;
}

/********************************************************************************************
*
* @class UserAccount
* @function IsDatasetAdmin
* @brief Check if user is DatasetAdmin
* @return true if user is DatasetAdmin
* @return false otherwise
*
********************************************************************************************/

bool __thiscall UserAccount::IsDatasetAdmin(void) const
{
    __DebugFunction();

    bool fIsDatasetAdmin = false;

    if (2 == (m_wTypeOfUser & 0x02))
    {
        fIsDatasetAdmin = true;
    }

    return fIsDatasetAdmin;
}

/********************************************************************************************
*
* @class UserAccount
* @function IsResearcher
* @brief Check if user is Researcher
* @return true if user is Researcher
* @return false otherwise
*
********************************************************************************************/

bool __thiscall UserAccount::IsResearcher(void) const
{
    __DebugFunction();

    bool fIsResearcher = false;

    if (1 == (m_wTypeOfUser & 0x01))
    {
        fIsResearcher = true;
    }

    return fIsResearcher;
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

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetUserName
 * @brief Fetch username
 * @return Username
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetUserName(void) const throw()
{
    __DebugFunction();

    return m_strUserName;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetOrganization
 * @brief Fetch user organization
 * @return User organization
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetOrganization(void) const throw()
{
    __DebugFunction();

    return m_strOrganization;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetFullName
 * @brief Fetch user full name
 * @return User full name
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetFullName(void) const throw()
{
    __DebugFunction();

    return m_strFullName;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetEmail
 * @brief Fetch user email
 * @return User email
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetEmail(void) const throw()
{
    __DebugFunction();

    return m_strEmail;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetContactInformation
 * @brief Fetch user contact information
 * @return User contact information
 *
 ********************************************************************************************/

std::string __thiscall UserAccount::GetContactInformation(void) const throw()
{
    __DebugFunction();

    return m_strContactInformation;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetlastLogon
 * @brief Fetch last logon time
 * @return User last logon time
 *
 ********************************************************************************************/

Qword __thiscall UserAccount::GetLastLogon(void) const throw()
{
    __DebugFunction();

    return m_qwLastLogon;
}

/********************************************************************************************
 *
 * @class UserAccount
 * @function GetTypeOfUser
 * @brief Fetch type of user
 * @return type of User
 *
 ********************************************************************************************/

Word __thiscall UserAccount::GetTypeOfUser(void) const throw()
{
    __DebugFunction();

    return m_wTypeOfUser;
}
