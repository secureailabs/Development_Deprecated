/*********************************************************************************************
 *
 * @file DigitalContract.cpp
 * @author Shabana Akhtar Baig
 * @date 23 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DigitalContract.h"

/********************************************************************************************
 *
 * @class DigitalContract
 * @function DigitalContract
 * @brief Constructor
 * @param[in] c_strDigitalContractUuid DigitalContract uuid
 * @param[in] c_strDataOwnerOrganization Data owner's organization name
 * @param[in] c_strResearcherOrganization Researcher's organiztion name
 * @param[in] c_strInvolvedDatasets string of null terminated names of datasets in the digital contract
 * @param[in] qwCreationDate DigitalContract creation date
 * @param[in] qwExpirationDate DigitalContract expiration date
 *
 ********************************************************************************************/

DigitalContract::DigitalContract(
    _in const std::string & c_strDigitalContractUuid,
    _in const std::string & c_strDataOwnerOrganization,
    _in const std::string & c_strResearcherOrganization,
    _in const std::string & c_strInvolvedDatasets,
    _in Qword qwCreationDate,
    _in Qword qwExpirationDate
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strDigitalContractUuid.length());
    __DebugAssert(0 != c_strDataOwnerOrganization.length());
    __DebugAssert(0 != c_strResearcherOrganization.length());
    __DebugAssert(0 != c_strInvolvedDatasets.length());

    m_strDigitalContractUuid = c_strDigitalContractUuid;
    m_strDataOwnerOrganization = c_strDataOwnerOrganization;
    m_strResearcherOrganization = c_strResearcherOrganization;
    m_strInvolvedDatasets = c_strInvolvedDatasets;
    m_qwCreationDate = qwCreationDate;
    m_qwExpirationDate = qwExpirationDate;
    m_fIsApproved = false;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function ~DigitalContract
 * @brief Destructor
 *
 ********************************************************************************************/

DigitalContract::~DigitalContract(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetDigitalContractUuid
 * @brief Fetch DigitalContract uuid
 * @return DigitalContract uuid
 *
 ********************************************************************************************/

std::string __thiscall DigitalContract::GetDigitalContractUuid(void) const throw()
{
    __DebugFunction();

    return m_strDigitalContractUuid;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetDataOwnerOrganization
 * @brief Fetch Data owner's organization name
 * @return Data owner's organization name
 *
 ********************************************************************************************/

std::string __thiscall DigitalContract::GetDataOwnerOrganization(void) const throw()
{
    __DebugFunction();

    return m_strDataOwnerOrganization;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetResearcherOrganization
 * @brief Fetch Researcher's organization name
 * @return Researcher's organization name
 *
 ********************************************************************************************/

std::string __thiscall DigitalContract::GetResearcherOrganization(void) const throw()
{
    __DebugFunction();

    return m_strResearcherOrganization;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetInvolvedDatasets
 * @brief Fetch name of dataset(s) involved in the contract
 * @return Name of dataset(s) involved in the contract
 *
 ********************************************************************************************/

std::string __thiscall DigitalContract::GetInvolvedDatasets(void) const throw()
{
    __DebugFunction();

    return m_strInvolvedDatasets;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetCreationDate
 * @brief Fetch digital contract's creation date
 * @return Digital contract's creation date
 *
 ********************************************************************************************/

Qword __thiscall DigitalContract::GetCreationDate(void) const throw()
{
    __DebugFunction();

    return m_qwCreationDate;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function GetExpirationDate
 * @brief Fetch digital contract's expiration date
 * @return Digital contract's expiration date
 *
 ********************************************************************************************/

Qword __thiscall DigitalContract::GetExpirationDate(void) const throw()
{
    __DebugFunction();

    return m_qwExpirationDate;
}

/********************************************************************************************
 *
 * @class DigitalContract
 * @function IsDigitalContractApproved
 * @brief Fetch boolean representing digital contract's approval status
 * @return Boolean representing digital contract's approval status
 *
 ********************************************************************************************/

bool __thiscall DigitalContract::IsDigitalContractApproved(void) const throw()
{
    __DebugFunction();

    return m_fIsApproved;
}
