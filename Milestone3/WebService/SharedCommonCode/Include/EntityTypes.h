/*********************************************************************************************
 *
 * @file EntityTypes.h
 * @author Shabana Akhtar Baig
 * @date 22 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

#include <map>
#include <string>

/********************************************************************************************/

typedef enum http_code
{
    eOk = 200,
    eCreated = 201,
    eNoContent = 204,
    eBadRequest = 400,
    eUnauthorized = 401,
    eForbidden = 403,
    eNotFound = 404,
    eRequestTimeout = 408,
    eInternalError = 500
}
HttpCode;

/********************************************************************************************/

typedef enum account_status
{
    eNew = 0,
    eOpen = 1,
    eSuspended = 2,
    eClosed = 3
}
AccountStatus;

/********************************************************************************************/

typedef enum user_rights
{
    eAdmin = 1,
    eAuditor = 2,
    eOrganizationUser = 3,
    eDigitalContractAdmin = 4,
    eDatasetAdmin = 5,
    eSailAdmin = 6
}
AccessRights;

/********************************************************************************************/

typedef enum digital_contract_stage
{
    eApplication = 1,
    eApproval = 2,
    eActive = 3
}
ContractStage;

/********************************************************************************************/

typedef enum eosb_rights
{
    eEosb = 1,
    eIEosb = 2,
    eVmEosb = 3
}
EosbAccessRights;

/********************************************************************************************/

enum class DigitalContractProvisiongStatus
{
    eProvisioning = 1,
    eReady = 2,
    eUnprovisioned = 3,
    eProvisioningFailed = 4
};

/********************************************************************************************/

typedef enum virtual_machine_state
{
    eStarting = 1,
    eConfiguring = 2,
    eInitializing = 3,
    eWaitingForData = 4,
    eReadyForComputation = 5,
    eInUse = 6,
    eShuttingDown = 7,
    eDeleted = 8,
    eDeleteFailed = 9,
    eProvisioningFailed = 10
}
VirtualMachineState;

/********************************************************************************************/

enum class AzureTemplateState
{
    eInitializing = 1,
    eCreatingVirtualNetwork = 2,
    eCreatingNetworkSecurityGroup = 3,
    eFailedCreatingVirtualNetwork = 4,
    eFailedCreatingNetworkSecurityGroup = 5,
    eReady = 6,
    eImageDoesNotExist = 7, // Unused
    eInvalidCredentials = 8,
    eCreatingImage = 9,
    eFailedCreatingImage = 10,
    eInternalError = 11
};
