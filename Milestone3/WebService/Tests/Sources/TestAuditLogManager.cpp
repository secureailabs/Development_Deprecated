/*********************************************************************************************
 *
 * @file TestAuditLogManager.cpp
 * @author Shabana Akhtar Baig
 * @date 12 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

 #include "Tests.h"
 #include "Base64Encoder.h"

/********************************************************************************************/

void TestRegisterLeafEvent(
    _in const std::string & c_strEncodedEosb,
    _in const std::string & c_strParentGuid
    )
{
    __DebugFunction();

    bool fSuccess = false;

    StructuredBuffer oLeafEvents;
    // Add leaf event
    StructuredBuffer oEvent;
    oEvent.PutString("EventGuid", Guid(eAuditEventPlainTextLeafNode).ToString(eHyphensAndCurlyBraces));
    oEvent.PutQword("EventType", 6);
    oEvent.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    StructuredBuffer oEncryptedEventData;
    oEncryptedEventData.PutString("EventName", "VMAdded");
    oEncryptedEventData.PutByte("EventType", 1);
    StructuredBuffer oEventData;
    oEventData.PutUnsignedInt64("VersionNumber", 0x0000000100000001);
    oEventData.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    oEncryptedEventData.PutStructuredBuffer("EventData", oEventData);
    oEvent.PutString("EncryptedEventData", ::Base64Encode(oEncryptedEventData.GetSerializedBufferRawDataPtr(), oEncryptedEventData.GetSerializedBufferRawDataSizeInBytes()));
    oLeafEvents.PutStructuredBuffer("0", oEvent);

    fSuccess = ::RegisterLeafEvents(c_strEncodedEosb, c_strParentGuid, oLeafEvents);

    // Check fSuccess
    if (true == fSuccess)
    {
        std::cout << "Test register leaf events passed." << std::endl;
    }
    else
    {
        std::cout << "Test register leaf events failed." << std::endl;
    }
}