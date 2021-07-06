/*********************************************************************************************
 *
 * @file TestDigitalContractDatabase.cpp
 * @author Shabana Akhtar Baig
 * @date 12 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"

 #include <assert.h>

/********************************************************************************************/

void TestRegisterDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDooGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("Title", "Test Contract");
    oDcInformation.PutUnsignedInt64("SubscriptionDays", 3);
    oDcInformation.PutString("LegalAgreement", "The Parties acknowledge and agree that this Agreement represents the entire agreement between the Parties.");
    oDcInformation.PutString("DOOGuid", c_strDooGuid);
    oDcInformation.PutString("Description", "Creating digital contract for testing.");

    fSuccess = ::RegisterDigitalContract(c_strEncodedEosb, oDcInformation);

    // Check fSuccess
    assert (true == fSuccess);

    std::cout << "Test register digital contract passed." << std::endl;
}

/********************************************************************************************/

std::string TestAcceptDigitalContract(void)
{
    __DebugFunction();

    std::string strDcGuid;

    // Login with dataset admin credentials
    std::string strEosb = ::Login("user5@test.com", "sailpassword");
    // Get list of digital contracts and find the test digital contract
    bool fFound = false;
    StructuredBuffer oListOfContracts = StructuredBuffer(::ListDigitalContracts(strEosb));
    std::vector<std::string> stlGuids = oListOfContracts.GetNamesOfElements();
    for (unsigned int unIndex = 0; (fFound == false && unIndex < stlGuids.size()); ++unIndex)
    {
        if ("Test Contract" == oListOfContracts.GetStructuredBuffer(stlGuids.at(unIndex).c_str()).GetString("Title"))
        {
            strDcGuid = stlGuids.at(unIndex);
            fFound = true;
        }
    }

    assert (true == fFound);

    StructuredBuffer oDcInformation;
    oDcInformation.PutUnsignedInt64("RetentionTime", 16186603);
    oDcInformation.PutString("LegalAgreement", "The Parties acknowledge and agree that this Agreement represents the entire agreement between the Parties.");
    oDcInformation.PutString("DigitalContractGuid", strDcGuid);
    oDcInformation.PutString("Description", "Modified description");

    bool fSuccess = ::AcceptDigitalContract(strEosb, oDcInformation);

    // Check fSuccess
    assert (true == fSuccess);
    
    std::cout << "Test accept digital contract passed." << std::endl;
    
    return strDcGuid;
}

/********************************************************************************************/

void TestActivateDigitalContract(
    _in const std::string & c_strDcGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    // Login with digital contract admin credentials
    std::string strEosb = ::Login("user4@test.com", "sailpassword");

    StructuredBuffer oDcInformation;
    oDcInformation.PutString("DigitalContractGuid", c_strDcGuid);

    fSuccess = ::ActivateDigitalContract(strEosb, oDcInformation);

    // Check fSuccess
    if (true == fSuccess)
    {
        std::cout << "Test activate digital contract passed." << std::endl;
    }
    else
    {
        std::cout << "Test activate digital contract failed." << std::endl;
    }
}

/********************************************************************************************/

void TestPullDigitalContract(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strDcGuid
    )
{
    __DebugFunction();

    StructuredBuffer oDigitalContract(::PullDigitalContract(c_strEncodedEosb, c_strDcGuid));

    // Compare the returned digital contract information with the information used in register digital contract
    std::string strTitle = oDigitalContract.GetString("Title");
    if ("Test Contract" == strTitle)
    {
        std::cout << "Test pull digital contract passed." << std::endl;
    }
    else
    {
        std::cout << "Test pull digital contract failed." << std::endl;
    }
}

/********************************************************************************************/

std::string RunDigitalContractDatabaseTests(
    _in const std::string & c_strEosb,
    _in const std::string & c_strOrganizationGuid
    )
{
    // Call unit tests
    ::TestRegisterDigitalContract(c_strEosb, c_strOrganizationGuid);
    std::string strDcGuid = ::TestAcceptDigitalContract();
    assert (0 < strDcGuid.size());
    ::TestActivateDigitalContract(strDcGuid);
    ::TestPullDigitalContract(c_strEosb, strDcGuid);

    return strDcGuid;
}