/*********************************************************************************************
 *
 * @file Tests.h
 * @author Shabana Akhtar Baig
 * @date 18 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #pragma once

 #include "InteractiveClient.h"

 /********************************************************************************************/

extern void RegressionTest(void);

extern void TestRegisterOrganizationAndSuperUser(void);

extern void TestRegisterUser(void);

extern void RunAccountDatabaseTests(void);

extern void TestRegisterLeafEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid
    );

extern void TestRegisterDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDooGuid
    );

extern std::string TestAcceptDigitalContract(void);

extern void TestActivateDigitalContract(
    _in const std::string & c_strDcGuid
    );

extern void TestPullDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid
    );

extern std::string RunDigitalContractDatabaseTests(
    _in const std::string & c_strEosb,
    _in const std::string & c_strOrganizationGuid
    );

extern void TestLoginWithCorrectCredentials(void);

extern void TestLoginWithIncorrectCredentials(void);

extern void TestGetBasicUserInformation(void);

extern void RunSailAuthenticationTests(void);

extern void TestRegisterVirtualMachine(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid,
    _in const std::string & c_strVmGuid
    );

extern std::string TestRegisterVmAfterDataUpload(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    );

extern std::string TestRegisterVmForComputation(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    );

extern void RunVirtualMachineManagerTests(
    _in const std::string & c_strEosb,
    _in const std::string & c_strDcGuid
    );


