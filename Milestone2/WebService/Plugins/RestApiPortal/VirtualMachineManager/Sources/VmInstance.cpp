/*********************************************************************************************
 *
 * @file VmInstance.cpp
 * @author Shabana Akhtar Baig
 * @date 19 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "VmInstance.h"

/********************************************************************************************
 *
 * @class VmInstance
 * @function VmInstance
 * @brief Constructor
 * @param[in] c_strVmInstanceUuid VmInstance uuid
 * @param[in] c_strVmInstanceIpAddress VmInstance IP address
 * @param[in] wVmInstancePortNumber VmInstance port number
 * @param[in] fVmInstanceStatus Status of VMInstance
 * @param[in] qwVmInstanceRegistrationDate Date when VMInstance was first registered
 * @param[in] qwVmInstanceUpTime How long has VMInstance been running
 *
 ********************************************************************************************/

VmInstance::VmInstance(
    _in const std::string & c_strVmInstanceUuid,
    _in const std::string & c_strVmInstanceIpAddress,
    _in Word wVmInstancePortNumber,
    _in bool fVmInstanceStatus,
    _in Qword qwVmInstanceRegistrationDate,
    _in Qword qwVmInstanceUpTime
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strVmInstanceUuid.length());
    __DebugAssert(0 != c_strVmInstanceIpAddress.length());
    __DebugAssert(0 < wVmInstancePortNumber);

    m_strVmInstanceUuid = c_strVmInstanceUuid;
    m_strVmInstanceIpAddress = c_strVmInstanceIpAddress;
    m_wVmInstancePortNumber = wVmInstancePortNumber;
    m_fVmInstanceIsActive = fVmInstanceStatus;
    m_qwVmInstanceRegistrationDate = qwVmInstanceRegistrationDate;
    m_qwVmInstanceUpTime = qwVmInstanceUpTime;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function ~VmInstance
 * @brief Destructor
 *
 ********************************************************************************************/

VmInstance::~VmInstance(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstanceUuid
 * @brief Fetch VmInstance uuid
 * @return VmInstance uuid
 *
 ********************************************************************************************/

std::string __thiscall VmInstance::GetVmInstanceUuid(void) const throw()
{
    __DebugFunction();

    return m_strVmInstanceUuid;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstanceIpAddress
 * @brief Fetch VmInstance IP address
 * @return VmInstance IP address
 *
 ********************************************************************************************/

std::string __thiscall VmInstance::GetVmInstanceIpAddress(void) const throw()
{
    __DebugFunction();

    return m_strVmInstanceIpAddress;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstancePortNumber
 * @brief Fetch VmInstance port number
 * @return VmInstance port number
 *
 ********************************************************************************************/

Word __thiscall VmInstance::GetVmInstancePortNumber(void) const throw()
{
    __DebugFunction();

    return m_wVmInstancePortNumber;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstanceStatus
 * @brief Fetch status of the VMInstance
 * @return Status of the VMInstance
 *
 ********************************************************************************************/

bool __thiscall VmInstance::GetVmInstanceStatus(void) const throw()
{
    __DebugFunction();

    return m_fVmInstanceIsActive;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstanceRegistrationDate
 * @brief Fetch VMInstance's registaration date
 * @return Registration date
 *
 ********************************************************************************************/

Qword __thiscall VmInstance::GetVmInstanceRegistrationDate(void) const throw()
{
    __DebugFunction();

    return m_qwVmInstanceRegistrationDate;
}

/********************************************************************************************
 *
 * @class VmInstance
 * @function GetVmInstanceUpTime
 * @brief Fetch uptime of the VMINstance
 * @return Uptime of the VMINstance
 *
 ********************************************************************************************/

Qword __thiscall VmInstance::GetVmInstanceUpTime(void) const throw()
{
    __DebugFunction();

    return m_qwVmInstanceUpTime;
}
