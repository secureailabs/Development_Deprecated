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
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function PluginVersion
 * @brief Constructor
 * @param[in] qwVersionNumber Qword representation of a plugin version
 *
 ********************************************************************************************/

PluginVersion::PluginVersion(
    Qword qwVersionNumber
    )
{
    __DebugFunction();

    unsigned int unDelimiterPosition, unVersionStrIndex = 0;
    std::vector<std::string> aVersionNumbers;

    // Get major and minor version numbers from the version Qword
    m_unMinor = qwVersionNumber & 0xFF;
    m_unMajor = (qwVersionNumber >> 8) & 0xFF;
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
 *    majorVersionNumberminorVersionNumber --> 0x00000001.0x00000001
 *
 ********************************************************************************************/

std::string __thiscall PluginVersion::ToString(void) const throw()
{
    __DebugFunction();

    std::string strPluginVersion;

    strPluginVersion += std::to_string(m_unMajor);
    strPluginVersion += '.';
    strPluginVersion += std::to_string(m_unMinor);

    return strPluginVersion;
}

/********************************************************************************************
 *
 * @class PluginVersion
 * @function RoundDown
 * @brief Round down version
 * @param[in] c_oPluginVersion Instance of a PluginVersion object to compare with
 * @return true if less than or equal to
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall PluginVersion::RoundDown(
    PluginVersion const & c_oPluginVersion
    ) const throw()
{
    __DebugFunction();

    if (m_unMajor > c_oPluginVersion.m_unMajor) { return false; }
    if (m_unMajor < c_oPluginVersion.m_unMajor) { return true; }
    if (m_unMajor == c_oPluginVersion.m_unMajor)
    {
        if (m_unMinor <= c_oPluginVersion.m_unMinor)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
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
            return true;
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
            return false;
        }
    }
    return true;
}
