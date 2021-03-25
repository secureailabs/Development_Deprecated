/*********************************************************************************************
 *
 * @file InteractiveClient.h
 * @author Shabana Akhtar Baig
 * @date 15 March 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#pragma once

#include "ConsoleInputHelperFunctions.h"
#include "DateAndTime.h"
#include "Exceptions.h"
#include "PluginDictionary.h"
#include "SmartMemoryAllocator.h"
#include "StructuredBuffer.h"
#include "TlsClient.h"
#include "GuiTerminal.h"
#include "Utils.h"
#include "HttpRequestParser.h"
#include "JsonValue.h"

#include <string.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

extern const char * g_szServerIpAddress;
extern unsigned int g_unPortNumber;

/********************************************************************************************/

extern void AddWebPortalConfiguration(
    _in const char * c_szIpAddress, 
    _in unsigned int unPortNumber
    );

static std::string __stdcall _GetEpochTimeInMilliseconds(void);

extern bool ParseFirstLine(
    _in const std::string & c_strRequestData
    );

extern std::vector<Byte> GetResponseBody(
    _in const std::string & c_strRequestData,
    _in TlsNode * poTlsNode
    );

extern std::string Login(
    _in const std::string & c_strEmail,
    _in const std::string & c_strUserPassword
    );

extern std::vector<Byte> GetBasicUserInformation(
    _in const std::string & c_strEosb
    );

extern std::string GetIEosb(
    _in const std::string & c_strEosb
    );

extern std::string RegisterRootEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid
    );
extern bool RegisterBranchEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const std::string & c_strOrganizationGuid,
    _in const std::string & c_strDcGuid
    );
extern bool RegisterLeafEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid,
    _in const std::string & c_strParentGuid
    );

extern std::string RegisterVirtualMachine(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    );

extern std::string RegisterVmAfterDataUpload(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    );

extern std::string RegisterVmForComputation(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    );

extern bool GetListOfEvents(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid,
    _in const std::string & c_strOrganizationGuid,
    _in unsigned int unIndentDepth
    );

extern bool RegisterOrganizationAndSuperUser(void);

extern bool RegisterOrganizationAndSuperUser(
    _in const StructuredBuffer & c_oOrganizationInformation
    );

extern bool RegisterUser(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid
    );

extern bool RegisterUser(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strOrganizationGuid,
    _in const StructuredBuffer & c_oUserInformation
    );

extern bool UpdateOrganizationInformation(
    _in const std::string & c_strEosb
    );

extern bool ListOrganizations(
    _in const std::string & c_strEosb
    );

extern bool DeleteUser(
    _in const std::string & c_strEosb
    );

extern bool DeleteOrganization(
    _in const std::string & c_strEosb
    );

extern bool RegisterDigitalContract(
    _in const std::string & c_strEncodedEosb
    );

extern bool RegisterDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const StructuredBuffer & c_oDcInformation
    );

extern bool AcceptDigitalContract(
    _in const std::string & c_strEncodedEosb
    );

extern bool ActivateDigitalContract(
    _in const std::string & c_strEncodedEosb
    );

extern bool ListDigitalContracts(
    _in const std::string & c_strEosb
    );

extern bool PullDigitalContract(
    _in const std::string & c_strEosb
    );
