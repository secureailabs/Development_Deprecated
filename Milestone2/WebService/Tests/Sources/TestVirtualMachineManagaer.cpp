/*********************************************************************************************
 *
 * @file TestDigitalContractDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 17 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"

/********************************************************************************************/

void TestRegisterVirtualMachine(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    StructuredBuffer oVmInformation;
    oVmInformation.PutString("DigitalContractGuid", c_strDcGuid);
    oVmInformation.PutString("IPAddress", "127.0.0.1");

    std::string strVmEosb = ::RegisterVirtualMachine(c_strEncodedEosb, c_strVmGuid, oVmInformation);

    // Check fSuccess
    if (strVmEosb.size() > 0)
    {
        std::cout << "Test register virtual machine passed." << std::endl;
    }
    else
    {
        std::cout << "Test register virtual machine failed." << std::endl;
    }
}

/********************************************************************************************/

std::string TestRegisterVmAfterDataUpload(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    std::string strVmEventGuid = ::RegisterVmAfterDataUpload(c_strEncodedEosb, c_strVmGuid);

    // Check fSuccess
    if (strVmEventGuid.size() > 0)
    {
        std::cout << "Test register virtual machine after data upload passed." << std::endl;
    }
    else
    {
        std::cout << "Test register virtual machine after data upload failed." << std::endl;
    }

    return strVmEventGuid;
}

/********************************************************************************************/

std::string TestRegisterVmForComputation(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strVmGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    std::string strVmEventGuid = ::RegisterVmForComputation(c_strEncodedEosb, c_strVmGuid);

    // Check fSuccess
    if (strVmEventGuid.size() > 0)
    {
        std::cout << "Test register virtual machine for computation passed." << std::endl;
    }
    else
    {
        std::cout << "Test register virtual machine for computation failed." << std::endl;
    }

    return strVmEventGuid;
}

/********************************************************************************************/

void RunVirtualMachineManagerTests(
    _in const std::string & c_strEosb,
    _in const std::string & c_strDcGuid
    )
{
    // Create a virtual machine guid that will be used to register a new virtual machine and testing other
    // functions
    std::string strVmGuid = Guid(eVirtualMachine).ToString(eHyphensAndCurlyBraces);

    // Call unit tests for VirtualMachineManager and AuditLogManager
    ::TestRegisterVirtualMachine(c_strEosb, c_strDcGuid, strVmGuid);
    std::string strDooVmEventGuid = ::TestRegisterVmAfterDataUpload(c_strEosb, strVmGuid);
    // Test register leaf event with VM event guid for the data owner organization
    ::TestRegisterLeafEvent(c_strEosb, strDooVmEventGuid);
    std::string strRoVmEventGuid = ::TestRegisterVmForComputation(c_strEosb, strVmGuid);
    // Test register leaf event with VM event guid for the researcher organization
    ::TestRegisterLeafEvent(c_strEosb, strRoVmEventGuid);
}