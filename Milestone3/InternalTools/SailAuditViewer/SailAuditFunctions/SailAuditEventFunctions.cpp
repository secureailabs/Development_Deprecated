#include <Windows.h>
#include <wchar.h>
#include <wtypes.h>
#include <time.h>

#include "CurlRest.h"

#include "SmartMemoryAllocator.h"
#include "StructuredBuffer.h"
#include "JsonValue.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "64BitHashes.h"
#include "Base64Encoder.h"

#include <algorithm>
#include <mutex>
#include <queue>

// SAIL Web Api Portal specific global data
static std::string gs_strIpAddressOfSailWebApiPortal;
static unsigned int gs_unPortAddressOfSailWebApiPortal = 0;
static std::string gs_strEosb;
static std::string gs_strOrganizationIdentifier;

// Audit events specific global data
static std::string gs_strAuditEventsParentNodeIdentifier = "{00000000-0000-0000-0000-000000000000}";
static std::map<unsigned int, Qword> gs_stlListOfAuditEventsBySequenceNumber;
static std::map<Qword, std::string> gs_stlListOfLoadedNodes;
static std::map<unsigned int, Qword> gs_stlOrderedListOfLoadedNodeAuditIdentifierHashes;
static std::queue<std::string> gs_stlQueueOfExceptions;
static std::mutex gs_stlMutex;
static unsigned int gs_unTotalEventsLoaded = 0;

/// <summary>
/// Register a BaseException in the internal queue
/// </summary>
/// <param name="c_oBaseException"></param>
/// <param name="c_szFunctionName"></param>
/// <param name="unLineNumber"></param>
/// <returns></returns>
static void __cdecl RegisterException(
    _in const BaseException & c_oBaseException,
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szFunctionName);

    std::string strExceptionMessage;
    
    strExceptionMessage = "Thrown from -> |File = ";
    strExceptionMessage += c_oBaseException.GetFilename();
    strExceptionMessage += "\r\n               |Function = ";
    strExceptionMessage += c_oBaseException.GetFunctionName();
    strExceptionMessage += "\r\n               |Line Number = ";
    strExceptionMessage += std::to_string(c_oBaseException.GetLineNumber());
    strExceptionMessage += "\r\n               |Message = ";
    strExceptionMessage += c_oBaseException.GetExceptionMessage();
    strExceptionMessage += "\r\nCaught in ---->|File = ";
    strExceptionMessage += __FILE__;
    strExceptionMessage += "\r\n               |Function = ";
    strExceptionMessage += c_szFunctionName;
    strExceptionMessage += "\r\n               |Line Number = ";
    strExceptionMessage += std::to_string(unLineNumber);

    gs_stlMutex.lock();
    gs_stlQueueOfExceptions.push(strExceptionMessage);
    gs_stlMutex.unlock();
}

/// <summary>
/// Register an unknown exception in the internal queue
/// </summary>
/// <param name="c_szFunctionName"></param>
/// <param name="unLineNumber"></param>
/// <returns></returns>
static void __cdecl RegisterUnknownException(
    _in const char * c_szFunctionName,
    _in unsigned int unLineNumber
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szFunctionName);

    std::string strExceptionMessage;

    strExceptionMessage = "UNKNOWN EXCEPTION!!!!!";
    strExceptionMessage += "\r\nCaught in ---->|File = ";
    strExceptionMessage += __FILE__;
    strExceptionMessage += "\r\n               |Function = ";
    strExceptionMessage += c_szFunctionName;
    strExceptionMessage += "\r\n               |Line Number = ";
    strExceptionMessage += std::to_string(unLineNumber);

    gs_stlMutex.lock();
    gs_stlQueueOfExceptions.push(strExceptionMessage);
    gs_stlMutex.unlock();
}

/// <summary>
/// We need this special function coupled with the SmartMemoryAllocator to make sure that we
/// are properly freeing the memory relating to the internal wszUnicodeString variable
/// </summary>
/// <param name="c_strAnsiString"></param>
/// <returns></returns>
static BSTR __cdecl ConvertToBSTR(
    _in const std::string & c_strAnsiString
    ) throw()
{
    __DebugFunction();

    BSTR pszString = nullptr;

    try
    {
        SmartMemoryAllocator oSmartMemoryAllocator;

        // Convert the char * string to wchar * string 
        size_t unCharacterCountIncludingNullTerminatingCharacter = (size_t) (c_strAnsiString.size() + 1);
        size_t unConvertedChars = 0;
        wchar_t * wszUnicodeString = (wchar_t*) oSmartMemoryAllocator.Allocate((unsigned int) unCharacterCountIncludingNullTerminatingCharacter * sizeof(wchar_t));
        ::mbstowcs_s(&unConvertedChars, wszUnicodeString, unCharacterCountIncludingNullTerminatingCharacter, c_strAnsiString.c_str(), c_strAnsiString.size());
        pszString = ::SysAllocString(wszUnicodeString);
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return pszString;
}

/// <summary>
/// 
/// </summary>
/// <param name="un64MillisecondEpochGmtTimestamp"></param>
/// <returns></returns>
static std::string __cdecl GetDateStringFromEpochGmtTimestamp(
    _in uint64_t un64MillisecondEpochGmtTimestamp
    )
{
    __DebugFunction();

    // Convert the time in seconds to be usable by _localtime32_s
    time_t sEpochGmtTimeInSeconds = un64MillisecondEpochGmtTimestamp / 1000;
    struct tm sTime;
    ::_localtime32_s(&sTime, (const long*) &sEpochGmtTimeInSeconds);

    // Format the time string
    const char * c_szMonths[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
    const char* c_szDayIth [] = { "oops", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };
    char szTemporaryBuffer[80];
    ::sprintf_s(szTemporaryBuffer, sizeof(szTemporaryBuffer), "%s %02d%s, %04d", c_szMonths[sTime.tm_mon], sTime.tm_mday, c_szDayIth[sTime.tm_mday], (sTime.tm_year + 1900));

    return std::string(szTemporaryBuffer);
}

/// <summary>
/// 
/// </summary>
/// <param name="un64MillisecondEpochGmtTimestamp"></param>
/// <returns></returns>
static std::string __cdecl GetMillisecondEpochGmtTimeStringFromEpochGmtTimestamp(
    _in uint64_t un64MillisecondEpochGmtTimestamp
    )
{
    __DebugFunction();

    // Figure out how many milliseconds
    unsigned int unMilliseconds = un64MillisecondEpochGmtTimestamp % 1000;
    // Convert the time in seconds to be usable by _localtime32_s
    time_t sEpochGmtTimeInSeconds = un64MillisecondEpochGmtTimestamp / 1000;
    struct tm sTime;
    ::_localtime32_s(&sTime, (const long*) &sEpochGmtTimeInSeconds);

    // Format the time string
    char szTemporaryBuffer[80];
    ::sprintf_s(szTemporaryBuffer, sizeof(szTemporaryBuffer), "%02d:%02d:%02d:%04d", sTime.tm_hour, sTime.tm_min, sTime.tm_sec, unMilliseconds);

    return std::string(szTemporaryBuffer);
}

/// <summary>
/// Set the SAIL Web API portal IP address
/// </summary>
/// <param name="c_szIpAddressOfSailWebApiPortal"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl SetSailWebApiPortalIpAddress(
    _in const char * c_szIpAddressOfSailWebApiPortal
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool SetSailWebApiPortalIpAddress(string ipAddressOfSailWebApiPortal);

    bool fSuccess = false;

    try
    {
        if (nullptr != c_szIpAddressOfSailWebApiPortal)
        {
            gs_strIpAddressOfSailWebApiPortal = c_szIpAddressOfSailWebApiPortal;
            gs_unPortAddressOfSailWebApiPortal = 6200;
            fSuccess = true;
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
    }

    return fSuccess;
}

/// <summary>
/// Logs into SAIL web services and gets an EOSB string. This is cached internally for future use
/// </summary>
/// <param name="c_szUsername"></param>
/// <param name="c_szPAssword"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl Login(
    _in const char * c_szUsername,
    _in const char * c_szPassword
    )
{
    __DebugFunction();
    __DebugAssert(0 < gs_strIpAddressOfSailWebApiPortal.size());
    __DebugAssert(0 != gs_unPortAddressOfSailWebApiPortal);

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool Login(string username, string password);

    bool fSuccess = false;

    try
    {
        std::string strVerb = "POST";
        std::string strApiUri = "/SAIL/AuthenticationManager/User/Login?Email=" + std::string(c_szUsername) + "&Password=" + std::string(c_szPassword);
        std::string strJsonBody = "";
        std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        std::string strResponse = ::UnEscapeJsonString((const char*) stlRestResponse.data());
        StructuredBuffer oStructuredBuffer(JsonValue::ParseDataToStructuredBuffer(strResponse.c_str()));
        gs_strEosb = oStructuredBuffer.GetString("Eosb");

        strVerb = "GET";
        strApiUri = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb=" + gs_strEosb;
        stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        strResponse = ::UnEscapeJsonString((const char*) stlRestResponse.data());
        StructuredBuffer oBasicUserInfoStructuredBuffer(JsonValue::ParseDataToStructuredBuffer(strResponse.c_str()));
        gs_strOrganizationIdentifier = oBasicUserInfoStructuredBuffer.GetString("OrganizationGuid");
        fSuccess = true;
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
    }

    return fSuccess;
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szParentIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) void __cdecl ReloadLoadAllAuditEvents(
    _in const char* c_szParentIdentifier
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public void ReloadLoadAllAuditEvents(string parentIdentifier);

    try
    {
        _ThrowBaseExceptionIf((0 == gs_strEosb.size()), "Cannot call LoadAllAuditEvents() without proper authentication", nullptr);
        __DebugAssert(0 < gs_strOrganizationIdentifier.size());

        if (nullptr != c_szParentIdentifier)
        {
            // Formulate the API request string
            std::string strVerb = "GET";
            std::string strApiUri = "/SAIL/AuditLogManager/GetListOfEvents";
            std::string strContent = "{\n    \"Eosb\": \"" + gs_strEosb + "\","
                "\n    \"ParentGuid\": \"" + std::string(c_szParentIdentifier) + "\","
                "\n    \"OrganizationGuid\": \"" + gs_strOrganizationIdentifier + "\","
                "\n    \"Filters\":"
                "\n    {"
                "\n    }"
                "\n}";
            // Execute the API call and get the response
              std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strContent, true);
            // Parse the response into a StructuredBuffer
            std::string strResponse = ::UnEscapeJsonString((const char*) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strResponse.c_str()));
            // Fetch the list of audit events from the response
            StructuredBuffer oListOfEvents(oResponse.GetStructuredBuffer("ListOfEvents"));
            // Loop through each element within the list of audit events and record each individual audit event
            for (std::string strSequenceNumber : oListOfEvents.GetNamesOfElements())
            {
                unsigned int unAuditEventSequenceNumber = std::stoi(strSequenceNumber);

                try
                {
                    gs_unTotalEventsLoaded++;
                    StructuredBuffer oReconstructedAuditEvent;
                    StructuredBuffer oCurrentAuditEvent(oListOfEvents.GetStructuredBuffer(strSequenceNumber.c_str()));
                    // Record the organizational identifier
                    oReconstructedAuditEvent.PutString("OrganizationalIdentifier", oCurrentAuditEvent.GetString("OrganizationGuid"));
                    // Record the event identifier
                    std::string strAuditEventIdentifier = oCurrentAuditEvent.GetString("EventGuid");
                    oReconstructedAuditEvent.PutString("AuditEventIdentifier", strAuditEventIdentifier);
                    // Record the event sequence number
                    oReconstructedAuditEvent.PutUnsignedInt32("SequenceNumber", std::stoul(strSequenceNumber));
                    // Figure out the 64 bit hash of the audit event identifier
                    Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(strAuditEventIdentifier.c_str(), false);
                    boolean fIsLeaf = oCurrentAuditEvent.GetBoolean("isLeaf");
                    oReconstructedAuditEvent.PutBoolean("IsBranchAuditEventNode", !fIsLeaf);
                    StructuredBuffer oObjectBlob(oCurrentAuditEvent.GetStructuredBuffer("ObjectBlob"));
                    oReconstructedAuditEvent.PutString("AuditEventParentIdentifier", oObjectBlob.GetString("ParentGuid"));
                    oReconstructedAuditEvent.PutUnsignedInt64("Timestamp", (uint64_t) oObjectBlob.GetFloat64("Timestamp"));
                    if (false == fIsLeaf)
                    {
                        if (0 == ::strncmp("{00000000-0000-0000-0000-000000000000}", c_szParentIdentifier, 39))
                        {
                            __DebugAssert(0 == unAuditEventSequenceNumber);
                            oReconstructedAuditEvent.PutString("AuditEventType", "ROOT_BRANCH_NODE");
                            oReconstructedAuditEvent.PutString("Comment", "There can only be one!!!");
                        }
                        else
                        {
                            StructuredBuffer oPlainTextEventData = oObjectBlob.GetStructuredBuffer("PlainTextEventData");
                            switch ((Dword) oPlainTextEventData.GetFloat64("BranchType"))
                            {
                                case 1
                                :   oReconstructedAuditEvent.PutString("AuditEventType", "DIGITAL_CONTRACT_BRANCH_NODE");
                                    oReconstructedAuditEvent.PutString("DigitalContractIdentifier", oPlainTextEventData.GetString("GuidOfDcOrVm"));
                                    break;
                                case 2
                                :   oReconstructedAuditEvent.PutString("AuditEventType", "VIRTUAL_MACHINE_BRANCH_NODE");
                                    oReconstructedAuditEvent.PutString("VirtualMachineIdentifier", oPlainTextEventData.GetString("GuidOfDcOrVm"));
                                    break;
                                default
                                :   oReconstructedAuditEvent.PutString("AuditEventType", "STRANGE_BRANCH_NODE");
                                    break;
                            }
                        }

                        // Recursively call LoadAllAuditEvents() with the identifier of the current branch node
                        ::ReloadLoadAllAuditEvents(strAuditEventIdentifier.c_str());
                    }
                    else
                    {
                        oReconstructedAuditEvent.PutBoolean("IsLeafNode", true);
                        std::string strBase64EncryptedAuditEventData = oObjectBlob.GetString("EncryptedEventData");
                        StructuredBuffer oEncryptedAuditEventData(strBase64EncryptedAuditEventData.c_str());
                        oReconstructedAuditEvent.PutString("AuditEventType", oEncryptedAuditEventData.GetString("EventName"));
                        oReconstructedAuditEvent.PutString("AdditionalEventData", strBase64EncryptedAuditEventData);
                    }

                    // Now we check to make sure ALL of the required elements are within the oReconstructedAuditEvent
                    // StructuredBuffer
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("OrganizationalIdentifier", ANSI_CHARACTER_STRING_VALUE_TYPE)), "Invalid audit event: missing OrganizationalIdentifier", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("AuditEventParentIdentifier", ANSI_CHARACTER_STRING_VALUE_TYPE)), "Invalid audit event: missing AuditEventParentIdentifier", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("AuditEventIdentifier", ANSI_CHARACTER_STRING_VALUE_TYPE)), "Invalid audit event: missing AuditEventIdentifier", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("IsBranchAuditEventNode", BOOLEAN_VALUE_TYPE)), "Invalid audit event: missing IsBranchAuditEventNode", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("SequenceNumber", UINT32_VALUE_TYPE)), "Invalid audit event: missing SequenceNumber", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("Timestamp", UINT64_VALUE_TYPE)), "Invalid audit event: missing Timestamp", nullptr);
                    _ThrowBaseExceptionIf((false == oReconstructedAuditEvent.IsElementPresent("AuditEventType", ANSI_CHARACTER_STRING_VALUE_TYPE)), "Invalid audit event: missing AuditEventType", nullptr);

                    // Only add the new event if it doesn't already exist within the gs_stlListOfLoadedNodes
                    if (gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier))
                    {
                        gs_stlOrderedListOfLoadedNodeAuditIdentifierHashes[(unsigned int) gs_stlListOfLoadedNodes.size()] = qwHashOfAuditEventIdentifier;
                        gs_stlListOfLoadedNodes[qwHashOfAuditEventIdentifier] = oReconstructedAuditEvent.GetBase64SerializedBuffer();
                    }
                }

                catch (BaseException oBaseException)
                {
                    ::RegisterException(oBaseException, __func__, __LINE__);
                }

                catch (...)
                {
                    ::RegisterUnknownException(__func__, __LINE__);
                }
            }
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szParentIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl DesignateAuditEventsUsingParentIdentifier(
    _in const char * c_szParentIdentifier
    )
{
    __DebugFunction();
    __DebugAssert(0 < gs_strIpAddressOfSailWebApiPortal.size());
    __DebugAssert(0 != gs_unPortAddressOfSailWebApiPortal);

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool DesignateAuditEventsUsingParentIdentifier(string parentIdentifier);

    bool fSuccess = false;

    try
    {
        if (nullptr != c_szParentIdentifier)
        {
            bool fIsProperBranchNode = false;
            // First we make sure that the parent identifier is a proper branch node
            if (0 == ::strncmp("{00000000-0000-0000-0000-000000000000}", c_szParentIdentifier, 39))
            {
                fIsProperBranchNode = true;
            }
            else
            {
                // Find the audit event node with the matching audit event identifier and figure
                // out if it is a branch node
                Qword qwHashOfAuditEventParentIdentifier = ::Get64BitHashOfNullTerminatedString(c_szParentIdentifier, false);
                if (gs_stlListOfLoadedNodes.end() != gs_stlListOfLoadedNodes.find(qwHashOfAuditEventParentIdentifier))
                {
                    StructuredBuffer oParentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventParentIdentifier).c_str());
                    fIsProperBranchNode = oParentAuditEvent.GetBoolean("IsBranchAuditEventNode");
                }
            }
            // We continue with this operation only of the c_szParentIdentifier referred to an audit event
            // which was a proper branch node
            if (true == fIsProperBranchNode)
            {
                // Clear the current list of audit events
                gs_stlListOfAuditEventsBySequenceNumber.clear();
                // Make sure the provided c_szParentIdentifier becomes the current Audit Events parent identifier
                gs_strAuditEventsParentNodeIdentifier = c_szParentIdentifier;
                // Now let's run through all of the current loaded nodes and find out which ones are parented
                // by c_szParentIdentifier
                for (auto strSerializedAuditEvent : gs_stlListOfLoadedNodes)
                {
                    StructuredBuffer oCurrentAuditEvent(strSerializedAuditEvent.second.c_str());
                    std::string strParentNodeIdentifier = oCurrentAuditEvent.GetString("AuditEventParentIdentifier");
                    if (strParentNodeIdentifier == c_szParentIdentifier)
                    {
                        std::string strAuditEventIdentifier = oCurrentAuditEvent.GetString("AuditEventIdentifier");
                        unsigned int unAuditEventSequenceNumber = oCurrentAuditEvent.GetUnsignedInt32("SequenceNumber");
                        // Figure out the 64 bit hash of the audit event identifier
                        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(strAuditEventIdentifier.c_str(), false);
                        gs_stlListOfAuditEventsBySequenceNumber[unAuditEventSequenceNumber] = qwHashOfAuditEventIdentifier;
                    }
                }

                fSuccess = true;
            }
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/// <summary>
/// Returns the identifier of the parent node for all of the current audit event nodes
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport)  BSTR __cdecl GetDesignatedAuditEventsParentIdentifier(void)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetDesignatedAuditEventsParentIdentifier();

    return ::ConvertToBSTR(gs_strAuditEventsParentNodeIdentifier);
}

/// <summary>
/// Return the number of events loaded by the last call to SelectAuditEventsFromParentIdentifier()
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetDesignatedAuditEventsCount(void)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetDesignatedAuditEventsCount();
    
    return (unsigned int) gs_stlListOfAuditEventsBySequenceNumber.size();
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetAllLoadedAuditEventsCount(void)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetAllLoadedAuditEventsCount();

    return (unsigned int) gs_stlListOfLoadedNodes.size();
}

/// <summary>
/// 
/// </summary>
/// <param name="unSequenceNumber"></param>
/// <returns></returns>
extern "C" __declspec(dllexport)  BSTR __cdecl GetDesignatedAuditEventIdentifierBySequenceNumber(
    _in unsigned int unSequenceNumber
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetDesignatedAuditEventIdentifierBySequenceNumber(uint sequenceNumber);

    std::string strAuditEventIdentifier;

    try
    {
        _ThrowBaseExceptionIf((gs_stlListOfAuditEventsBySequenceNumber.end() == gs_stlListOfAuditEventsBySequenceNumber.find(unSequenceNumber)), "Invalid sequence numbder %d provided", unSequenceNumber);
        Qword qwHashOfAuditEventIdentifier = gs_stlListOfAuditEventsBySequenceNumber.at(unSequenceNumber);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Unexpected error. Could not find audit event 0x%08X%08X", HIDWORD(qwHashOfAuditEventIdentifier), LODWORD(qwHashOfAuditEventIdentifier));
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventIdentifier = oCurrentAuditEvent.GetString("AuditEventIdentifier");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventIdentifier);
}

/// <summary>
/// 
/// </summary>
/// <param name="unSequenceNumber"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventIdentifierByIndex(
    _in unsigned int unSequenceNumber
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventIdentifierByIndex(uint sequenceNumber);

    std::string strAuditEventIdentifier;

    try
    {
        _ThrowBaseExceptionIf((gs_stlOrderedListOfLoadedNodeAuditIdentifierHashes.end() == gs_stlOrderedListOfLoadedNodeAuditIdentifierHashes.find(unSequenceNumber)), "Invalid sequence numbder %d provided", unSequenceNumber);
        Qword qwHashOfAuditEventIdentifier = gs_stlOrderedListOfLoadedNodeAuditIdentifierHashes.at(unSequenceNumber);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Unexpected error. Could not find audit event 0x%08X%08X", HIDWORD(qwHashOfAuditEventIdentifier), LODWORD(qwHashOfAuditEventIdentifier));
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventIdentifier = oCurrentAuditEvent.GetString("AuditEventIdentifier");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventIdentifier);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport)  BSTR __cdecl GetAuditEventOrganizationalIdentifier(
    _in const char * c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventOrganizationalIdentifier(string auditEventIdentifier);

    std::string strAuditEventIdentifier;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Unexpected error. Could not find audit event 0x%08X%08X", HIDWORD(qwHashOfAuditEventIdentifier), LODWORD(qwHashOfAuditEventIdentifier));
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventIdentifier = oCurrentAuditEvent.GetString("OrganizationalIdentifier");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventIdentifier);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport)  BSTR __cdecl GetAuditEventParentIdentifier(
    _in const char* c_szAuditEventIdentifier
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventParentIdentifier(string eventIdentifier);

    std::string strAuditEventParentIdentifier;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventParentIdentifier = oCurrentAuditEvent.GetString("AuditEventParentIdentifier");
        if (0 == strAuditEventParentIdentifier.size())
        {
            strAuditEventParentIdentifier = "asasas";
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventParentIdentifier);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl IsBranchNodeEditEvent(
    _in const char * c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool IsBranchNodeEditEvent(string eventIdentifier);

    bool fIsBranchNodeEditEvent = false;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        fIsBranchNodeEditEvent = oCurrentAuditEvent.GetBoolean("IsBranchAuditEventNode");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fIsBranchNodeEditEvent;
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetAuditEventSequenceNumber(
    _in const char * c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetAuditEventSequenceNumber(string eventIdentifier);

    unsigned int unSequenceNumber = 0xFFFFFFFF;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        unSequenceNumber = oCurrentAuditEvent.GetUnsignedInt32("SequenceNumber");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return unSequenceNumber;
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventDateGmt(
    _in const char* c_szAuditEventIdentifier
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventDateGmt(string eventIdentifier);

    std::string strAuditEventEpochGmtDate = "January 1st, 1970";

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventEpochGmtDate = ::GetDateStringFromEpochGmtTimestamp((uint64_t) oCurrentAuditEvent.GetUnsignedInt64("Timestamp"));
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventEpochGmtDate);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventTimeWithMillisecondsGmt(
    _in const char* c_szAuditEventIdentifier
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventTimeWithMillisecondsGmt(string eventIdentifier);

    std::string strAuditEventEpochGmtDate = "00:00:00:0000";

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventEpochGmtDate = ::GetMillisecondEpochGmtTimeStringFromEpochGmtTimestamp(oCurrentAuditEvent.GetUnsignedInt64("Timestamp"));
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventEpochGmtDate);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) uint64_t GetAuditEventGmtEpochTimestampInMilliseconds(
    _in const char * c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public ulong GetAuditEventGmtEpochTimestampInMilliseconds(string eventIdentifier);

    uint64_t un64EpochGmtTimestampInSeconds = 0;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        un64EpochGmtTimestampInSeconds = oCurrentAuditEvent.GetUnsignedInt64("Timestamp");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return un64EpochGmtTimestampInSeconds;
}

/// <summary>
/// 
/// </summary>
/// <param name="c_szAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR GetAuditEventEpochTimestampInMillisecondsGmt(
    _in const char* c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventEpochTimestampInMillisecondsGmt(string eventIdentifier);

    std::string strAuditEventEpochGmtDate = "9999/99/99 @ 00:00:00:0000 GMT";

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        uint64_t un64MillisecondEpochGmtTimestamp = oCurrentAuditEvent.GetUnsignedInt64("Timestamp");
        // Figure out how many milliseconds
        unsigned int unMilliseconds = un64MillisecondEpochGmtTimestamp % 1000;
        // Convert the time in seconds to be usable by _localtime32_s
        time_t sEpochGmtTimeInSeconds = un64MillisecondEpochGmtTimestamp / 1000;
        struct tm sTime;
        ::_localtime32_s(&sTime, (const long*) &sEpochGmtTimeInSeconds);
        // Format the time string
        char szTemporaryBuffer[80];
        ::sprintf_s(szTemporaryBuffer, sizeof(szTemporaryBuffer), "%04d/%02d/%02d @ %02d:%02d:%02d:%04d", sTime.tm_year + 1900, sTime.tm_mon, sTime.tm_mday, sTime.tm_hour, sTime.tm_min, sTime.tm_sec, unMilliseconds);
        strAuditEventEpochGmtDate = szTemporaryBuffer;

    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventEpochGmtDate);


}

/// <summary>
/// Returns the name of the event
/// </summary>
/// <param name="c_szBranchNodeIdentifier"></param>
/// <param name="unSequenceNumber"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventType(
    _in const char* c_szAuditEventIdentifier
)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventType(string eventIdentifier);

    std::string strAuditEventName;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        strAuditEventName = oCurrentAuditEvent.GetString("AuditEventType");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventName);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventGeneralDetails(
    _in const char * c_szAuditEventIdentifier
    )
{
    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventGeneralDetails(string eventIdentifier);

    std::string strAuditEventDescription;

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        JsonValue * poJsonValue = JsonValue::ParseStructuredBufferToJson(oCurrentAuditEvent);
        if (nullptr != poJsonValue)
        {
            strAuditEventDescription = poJsonValue->ToString();
            poJsonValue->Release();
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventDescription);
}

/// <summary>
/// 
/// </summary>
/// <param name="c_strAuditEventIdentifier"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetAuditEventAdditionalDataDetails(
    _in const char * c_szAuditEventIdentifier
    )
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetAuditEventAdditionalDataDetails(string eventIdentifier);

    std::string strAuditEventAdditionalData = "";

    try
    {
        Qword qwHashOfAuditEventIdentifier = ::Get64BitHashOfNullTerminatedString(c_szAuditEventIdentifier, false);
        _ThrowBaseExceptionIf((gs_stlListOfLoadedNodes.end() == gs_stlListOfLoadedNodes.find(qwHashOfAuditEventIdentifier)), "Audit Event Node %s not found.", c_szAuditEventIdentifier);
        StructuredBuffer oCurrentAuditEvent(gs_stlListOfLoadedNodes.at(qwHashOfAuditEventIdentifier).c_str());
        if (true == oCurrentAuditEvent.IsElementPresent("AdditionalEventData", ANSI_CHARACTER_STRING_VALUE_TYPE))
        {
            StructuredBuffer oAdditionalData(oCurrentAuditEvent.GetString("AdditionalEventData").c_str());
            strAuditEventAdditionalData = oAdditionalData.ToString();
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strAuditEventAdditionalData);
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetExceptionCount(void)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetExceptionCount();

    unsigned int unNumberOfExceptions = 0;

    try
    {
        gs_stlMutex.lock();
        unNumberOfExceptions = (unsigned int) gs_stlQueueOfExceptions.size();
        gs_stlMutex.unlock();
    }

    catch (...)
    {

    }

    return unNumberOfExceptions;
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetNextException(void)
{
    __DebugFunction();

    // [DllImport("SailAuditFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetNextException();

    std::string strException = "";

    try
    {
        gs_stlMutex.lock();
        if (false == gs_stlQueueOfExceptions.empty())
        {
            strException = gs_stlQueueOfExceptions.front();
            gs_stlQueueOfExceptions.pop();
        }
        gs_stlMutex.unlock();
    }

    catch (...)
    {

    }

    return ::ConvertToBSTR(strException);
}