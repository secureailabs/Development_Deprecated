/*********************************************************************************************
 *
 * @file PluginVersion.cpp
 * @author Shabana Akhtar Baig
 * @date 27 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "PluginVersion.h"

/********************************************************************************************
 *
 * @class PluginVersion
 * @function PluginVersion
 * @brief Constructor
 *
 ********************************************************************************************/

PluginVersion::PluginVersion()
{
    __DebugFunction();

    m_unMajor = 0;
    m_unMinor = 0;
    m_unUpdateNumber = 0;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function PluginVersion
 * @brief Constructor
 * @param[in] c_strVersionNumber String representation of a plugin version
 *
 ********************************************************************************************/

PluginVersion::PluginVersion(
    std::string const & c_strVersionNumber
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strVersionNumber.length());

    unsigned int unDelimiterPosition, unVersionStrIndex = 0;
    std::vector<std::string> aVersionNumbers;

    // Get major, minor, and update version numbers from the version string
    while((unDelimiterPosition = c_strVersionNumber.find('.')) != std::string::npos)
    {
        aVersionNumbers.push_back(c_strVersionNumber.substr(unVersionStrIndex, unDelimiterPosition));
        unVersionStrIndex += unDelimiterPosition + 1;
    }

    _ThrowBaseExceptionIf((3 != aVersionNumbers.size()), "Invalid plugin version: Expected plugin version of the form 0.0.0 but got %s", c_strVersionNumber.c_str());

    m_unMajor = std::atoi(aVersionNumbers.at(0).c_str());
    m_unMinor = std::atoi(aVersionNumbers.at(1).c_str());
    m_unUpdateNumber = std::atoi(aVersionNumbers.at(2).c_str());
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function ~PluginVersion
 * @brief Destructor
 *
 ********************************************************************************************/

PluginVersion::~PluginVersion(void) throw()
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function ToString
 * @brief Returns a string representation of the PluginVersion
 * @return A string representation of the GUID/UUID
 * @note
 *    The string representation will be of the form:
 *    majorVersionNumber.minorVersionNumber.updateVersionNumber --> 1.1.1
 *
 ********************************************************************************************/

std::string __thiscall PluginVersion::ToString(void) const throw()
{
    __DebugFunction();

    std::string strPluginVersion;

    strPluginVersion += std::to_string(m_unMajor);
    strPluginVersion += '.';
    strPluginVersion += std::to_string(m_unMinor);
    strPluginVersion += '.';
    strPluginVersion += std::to_string(m_unUpdateNumber);

    return strPluginVersion;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function operator <
 * @brief Comparison operator < override
 * @param[in] c_oPluginVersion Instance of a PluginVersion object to compare with
 * @return true if less than
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall PluginVersion::operator < (
    PluginVersion const & c_oPluginVersion
    ) const throw()
{
    __DebugFunction();

    if(m_unMajor > c_oPluginVersion.m_unMajor) { return false; }
    if(m_unMajor < c_oPluginVersion.m_unMajor) { return true; }
    if(m_unMinor > c_oPluginVersion.m_unMinor) { return false; }
    if(m_unMinor < c_oPluginVersion.m_unMinor) { return true; }
    if(m_unUpdateNumber > c_oPluginVersion.m_unUpdateNumber) { return false; }
    if(m_unUpdateNumber < c_oPluginVersion.m_unUpdateNumber) { return true; }

    return false;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function operator >
 * @brief Comparison operator > override
 * @param[in] c_oPluginVersion Instance of a PluginVersion object to compare with
 * @return true if greater than
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall PluginVersion::operator > (
    PluginVersion const & c_oPluginVersion
    ) const throw()
{
    __DebugFunction();

    if(m_unMajor < c_oPluginVersion.m_unMajor) { return false; }
    if(m_unMajor > c_oPluginVersion.m_unMajor) { return true; }
    if(m_unMinor < c_oPluginVersion.m_unMinor) { return false; }
    if(m_unMinor > c_oPluginVersion.m_unMinor) { return true; }
    if(m_unUpdateNumber < c_oPluginVersion.m_unUpdateNumber) { return false; }
    if(m_unUpdateNumber > c_oPluginVersion.m_unUpdateNumber) { return true; }

    return false;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function operator ==
 * @brief Comparison operator == override
 * @param[in] c_oPluginVersion Instance of a PluginVersion object to compare with
 * @return true if equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall PluginVersion::operator == (
    PluginVersion const & c_oPluginVersion
    ) const throw()
{
    __DebugFunction();

    if(m_unMajor == c_oPluginVersion.m_unMajor)
    {
        if(m_unMinor == c_oPluginVersion.m_unMinor)
        {
            if(m_unUpdateNumber == c_oPluginVersion.m_unUpdateNumber)
            {
                return true;
            }
        }
    }
    return false;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function operator !=
 * @brief Comparison operator != override
 * @param[in] c_oPluginVersion Instance of a PluginVersion object to compare with
 * @return true if not equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall PluginVersion::operator != (
    PluginVersion const & c_oPluginVersion
    ) const throw()
{
    __DebugFunction();

    if(m_unMajor == c_oPluginVersion.m_unMajor)
    {
        if(m_unMinor == c_oPluginVersion.m_unMinor)
        {
            if(m_unUpdateNumber == c_oPluginVersion.m_unUpdateNumber)
            {
                return false;
            }
        }
    }
    return true;
}
