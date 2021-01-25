/*********************************************************************************************
 *
 * @file Dataset.cpp
 * @author Shabana Akhtar Baig
 * @date 18 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Dataset.h"

/********************************************************************************************
 *
 * @class Dataset
 * @function Dataset
 * @brief Constructor
 * @param[in] c_strDatasetUuid Dataset uuid
 * @param[in] c_strDatasetName Dataset name
 * @param[in] c_strDatasetOrganization Dataset organization
 * @param[in] c_strDatasetSubmittedBy User who submitted the dataset
 * @param[in] qwDatasetSubmissionDate Dataset submission date
 *
 ********************************************************************************************/

Dataset::Dataset(
    _in const std::string & c_strDatasetUuid,
    _in const std::string & c_strDatasetName,
    _in const std::string & c_strDatasetOrganization,
    _in const std::string & c_strDatasetSubmittedBy,
    _in Qword qwDatasetSubmissionDate
    )
{
    __DebugFunction();
    __DebugAssert(0 != c_strDatasetUuid.length());
    __DebugAssert(0 != c_strDatasetName.length());
    __DebugAssert(0 != c_strDatasetOrganization.length());
    __DebugAssert(0 != c_strDatasetSubmittedBy.length());

    m_strDatasetUuid = c_strDatasetUuid;
    m_strDatasetName = c_strDatasetName;
    m_strDatasetOrganization = c_strDatasetOrganization;
    m_strDatasetSubmittedBy = c_strDatasetSubmittedBy;
    m_qwDatasetSubmissionDate = qwDatasetSubmissionDate;
}

/********************************************************************************************
 *
 * @class Dataset
 * @function ~Dataset
 * @brief Destructor
 *
 ********************************************************************************************/

Dataset::~Dataset(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class Dataset
 * @function GetDatasetUuid
 * @brief Fetch dataset uuid
 * @return Dataset uuid
 *
 ********************************************************************************************/

std::string __thiscall Dataset::GetDatasetUuid(void) const throw()
{
    __DebugFunction();

    return m_strDatasetUuid;
}

/********************************************************************************************
 *
 * @class Dataset
 * @function GetDatasetName
 * @brief Fetch dataset name
 * @return Dataset name
 *
 ********************************************************************************************/

std::string __thiscall Dataset::GetDatasetName(void) const throw()
{
    __DebugFunction();

    return m_strDatasetName;
}

/********************************************************************************************
 *
 * @class Dataset
 * @function GetDatasetOrganization
 * @brief Fetch dataset organization
 * @return Dataset organization
 *
 ********************************************************************************************/

std::string __thiscall Dataset::GetDatasetOrganization(void) const throw()
{
    __DebugFunction();

    return m_strDatasetOrganization;
}

/********************************************************************************************
 *
 * @class Dataset
 * @function GetDatasetSubmittedBy
 * @brief Fetch user who submitted the dataset
 * @return User who submitted the dataset
 *
 ********************************************************************************************/

std::string __thiscall Dataset::GetDatasetSubmittedBy(void) const throw()
{
    __DebugFunction();

    return m_strDatasetSubmittedBy;
}

/********************************************************************************************
 *
 * @class Dataset
 * @function GetDatasetSubmissionDate
 * @brief Fetch submission date
 * @return Submission date
 *
 ********************************************************************************************/

Qword __thiscall Dataset::GetDatasetSubmissionDate(void) const throw()
{
    __DebugFunction();

    return m_qwDatasetSubmissionDate;
}
