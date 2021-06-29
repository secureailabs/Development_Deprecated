    /*********************************************************************************************
 *
 * @file SailWebApiFunctions.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Guid class that handles 128 bit (16 bytes) GUID/UUID values.
 *
 ********************************************************************************************/

#include <Windows.h>    // Always include Windows.h first

#include "64BitHashes.h"
#include "Base64Encoder.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "JsonValue.h"
#include "RestApiHelperFunctions.h"
#include "StructuredBuffer.h"
#include "SmartMemoryAllocator.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>

// SAIL Web Api Portal specific global data.
static std::string gs_strIpAddressOfSailWebApiPortal;
static unsigned int gs_unPortAddressOfSailWebApiPortal = 0;
static std::string gs_strEosb;
static std::string gs_strOrganizationIdentifier;
static std::string gs_strAuthenticatedUserIdentifier;
static Qword gs_qwAuthenticatedUserAccessRights;
// Local global variables used to track information that was fetched from the Sail Web Api Portal
static std::map<unsigned int, std::string> gs_strIndexedListOfSerializedBase64DigitalContracts;

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
        // User the SmartMemoryAllocator in order to make sure that we deallocate memory properly
        // even if an exception is thrown
        SmartMemoryAllocator oSmartMemoryAllocator;
        // Convert the char * string to wchar * string 
        size_t unCharacterCountIncludingNullTerminatingCharacter = (size_t) (c_strAnsiString.size() + 1);
        size_t unConvertedChars = 0;
        wchar_t* wszUnicodeString = (wchar_t*) oSmartMemoryAllocator.Allocate((unsigned int) unCharacterCountIncludingNullTerminatingCharacter * sizeof(wchar_t));
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
static std::string __cdecl GetDateStringFromEpochMillisecondTimestamp(
    _in uint64_t un64MillisecondEpochGmtTimestamp
    )
{
    __DebugFunction();

    // Convert the time in seconds to be usable by _localtime32_s
    time_t sEpochGmtTimeInSeconds = un64MillisecondEpochGmtTimestamp / 1000;
    struct tm sTime;
    ::_localtime32_s(&sTime, (const long*) &sEpochGmtTimeInSeconds);

    // Format the time string
    const char* c_szMonths [] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
    const char* c_szDayIth [] = { "oops", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st" };
    char szTemporaryBuffer[80];
    ::sprintf_s(szTemporaryBuffer, sizeof(szTemporaryBuffer), "%s %02d%s, %04d @ %02d:%02d:%02d", c_szMonths[sTime.tm_mon], sTime.tm_mday, c_szDayIth[sTime.tm_mday], (sTime.tm_year + 1900), sTime.tm_hour, sTime.tm_min, sTime.tm_sec);

    return std::string(szTemporaryBuffer);
}

/// <summary>
/// Set the SAIL Web API portal IP address
/// </summary>
/// <param name="c_szIpAddressOfSailWebApiPortal"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl SetIpAddress(
    _in const char * c_szIpAddressOfSailWebApiPortal
    )
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool SetIpAddress(string ipAddressOfSailWebApiPortal);

    bool fSuccess = false;

    try
    {
        if (nullptr != c_szIpAddressOfSailWebApiPortal)
        {
            // Reset the IP settings to the new values
            gs_strIpAddressOfSailWebApiPortal = c_szIpAddressOfSailWebApiPortal;
            gs_unPortAddressOfSailWebApiPortal = 6200;
            // Reseting the IP settings of the Sail Web Api portal will effectively log
            // the user out. So we reset ALL settings
            gs_strEosb = "";
            gs_strOrganizationIdentifier = "";
            gs_strAuthenticatedUserIdentifier = "";
            gs_qwAuthenticatedUserAccessRights = 0;
            // If we get here, no exception was thrown so the operation was a success
            fSuccess = true;
        }
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);

        // Reset global values to starting uninitialized values
        gs_strIpAddressOfSailWebApiPortal = "";
        gs_unPortAddressOfSailWebApiPortal = 0;
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
        gs_strAuthenticatedUserIdentifier = "";
        gs_qwAuthenticatedUserAccessRights = 0;
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);

        // Reset global values to starting uninitialized values
        gs_strIpAddressOfSailWebApiPortal = "";
        gs_unPortAddressOfSailWebApiPortal = 0;
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
        gs_strAuthenticatedUserIdentifier = "";
        gs_qwAuthenticatedUserAccessRights = 0;
    }

    return fSuccess;
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetIpAddress(void)
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetIpAddress();

    std::string strIpAddress = "";

    try
    {
        strIpAddress = gs_strIpAddressOfSailWebApiPortal;
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strIpAddress);
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

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool Login(string username, string password);

    bool fSuccess = false;

    try
    {
        // Make sure we are not currently logged on
        _ThrowBaseExceptionIf((false == gs_strEosb.empty()), "Already logged in. Make sure to logout first.", nullptr);
        __DebugAssert(true == gs_strOrganizationIdentifier.empty());
        __DebugAssert(true == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 == gs_qwAuthenticatedUserAccessRights);

        // Build out the REST API call query
        std::string strVerb = "POST";
        std::string strApiUri = "/SAIL/AuthenticationManager/User/Login?Email=" + std::string(c_szUsername) + "&Password=" + std::string(c_szPassword);
        std::string strJsonBody = "";
        // Send the REST API call to the SAIL Web Api Portal
        std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        stlRestResponse.push_back(0);
        // Parse the returning value.
        StructuredBuffer oLoginResponse = JsonValue::ParseDataToStructuredBuffer((const char*) stlRestResponse.data());
        // Did the transaction succeed?
        _ThrowBaseExceptionIf((201 != oLoginResponse.GetFloat64("Status")), "Failed to log in. Invalid Credentials.", nullptr);
        gs_strEosb = oLoginResponse.GetString("Eosb");

        // By default, we automatically get the basic user information since some of those
        // values are useful in other SAIL Web API Portal transactions
        strVerb = "GET";
        strApiUri = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb=" + gs_strEosb;
        stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        StructuredBuffer oBasicUserInfoStructuredBuffer = JsonValue::ParseDataToStructuredBuffer((const char*) stlRestResponse.data());
        // Did the transaction succeed?
        _ThrowBaseExceptionIf((200 != oBasicUserInfoStructuredBuffer.GetFloat64("Status")), "Sail Web Api Portal Transaction has failed.", nullptr);
        // Extract all values and cache them in global variables
        gs_strOrganizationIdentifier = oBasicUserInfoStructuredBuffer.GetString("OrganizationGuid");
        gs_strAuthenticatedUserIdentifier = oBasicUserInfoStructuredBuffer.GetString("UserGuid");
        gs_qwAuthenticatedUserAccessRights = (Qword) oBasicUserInfoStructuredBuffer.GetFloat64("AccessRights");
        // If we get here, no exceptions were thrown, so the transaction was successfull!!!
        fSuccess = true;
    }

    catch (BaseException oBaseException)
    {
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
        gs_strAuthenticatedUserIdentifier = "";
        gs_qwAuthenticatedUserAccessRights = 0;
    }

    catch (...)
    {
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
        gs_strAuthenticatedUserIdentifier = "";
        gs_qwAuthenticatedUserAccessRights = 0;
    }

    return fSuccess;
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl Logout(void)
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool Logout();

    bool fSuccess = false;

    try
    {
        // Check to make sure we are logged in before trying to logout
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        // Okay, logout now
        gs_strEosb = "";
        gs_strOrganizationIdentifier = "";
        gs_strAuthenticatedUserIdentifier = "";
        gs_qwAuthenticatedUserAccessRights = 0;
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
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetSailWebApiPortalImpostorEosb(void)
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetSailWebApiPortalImpostorEosb();

    std::string strSailWebApiPortalImportorEosb = "";

    try
    {
        // Check to make sure we are logged in before trying to transact
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        // Build out the REST API call query
        std::string strVerb = "GET";
        std::string strApiUri = "/SAIL/CryptographicManager/User/GetIEosb?Eosb=" + gs_strEosb;
        std::string strJsonBody = "";
        // Send the REST API call to the SAIL Web Api Portal
        std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        // Parse the returning value.
        StructuredBuffer oGetSailWebApiPortalImpostorEosbResponse = JsonValue::ParseDataToStructuredBuffer((const char*) stlRestResponse.data());
        // Did the transaction succeed?
        _ThrowBaseExceptionIf((200 != oGetSailWebApiPortalImpostorEosbResponse.GetFloat64("Status")), "Sail Web Api Portal Transaction has failed.", nullptr);
        // Extract the IEOSB from the response
        strSailWebApiPortalImportorEosb = oGetSailWebApiPortalImpostorEosbResponse.GetString("UpdatedEosb");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strSailWebApiPortalImportorEosb);
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl LoadDigitalContracts(void)
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint LoadDigitalContracts();

    unsigned int unDigitalContractsCount = 0;

    try
    {
        // Check to make sure we are logged in before trying to transact
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        // Build out the REST API call query
        std::string strVerb = "GET";
        std::string strApiUri = "/SAIL/DigitalContractManager/DigitalContracts";
        std::string strJsonBody = "{\n    \"Eosb\": \"" + gs_strEosb + "\"\n}";
        // Send the REST API call to the SAIL Web Api Portal
        std::vector<Byte> stlRestResponse = ::RestApiCall(gs_strIpAddressOfSailWebApiPortal, (Word) gs_unPortAddressOfSailWebApiPortal, strVerb, strApiUri, strJsonBody, true);
        // Parse the returning value.
        stlRestResponse.push_back(0);
        StructuredBuffer oGetDigitalContractsResponse = JsonValue::ParseDataToStructuredBuffer((const char*) stlRestResponse.data());
        // Did the transaction succeed?
        _ThrowBaseExceptionIf((200 != oGetDigitalContractsResponse.GetFloat64("Status")), "Sail Web Api Portal Transaction has failed.", nullptr);
        // Extract the Eosb (in this case an importer EOSB) from the transaction response
        StructuredBuffer oDigitalContracts = oGetDigitalContractsResponse.GetStructuredBuffer("DigitalContracts");
        // Clear the currently cached list of Digital Contracts
        gs_strIndexedListOfSerializedBase64DigitalContracts.clear();
        // Now let's extract each individual digital contract and registered them in a way
        // were they can be accessed using an index. The StructuredBuffer is a collection of
        // StructuredBuffers whereas the name of each StructuredBuffer is the identifier
        // of the Digital Contract
        unsigned int unIndex = 0;
        for (std::string strElement : oDigitalContracts.GetNamesOfElements())
        {
            StructuredBuffer oElement(oDigitalContracts.GetStructuredBuffer(strElement.c_str()));
            oElement.PutString("DigitalContractIdentifier", strElement);
            gs_strIndexedListOfSerializedBase64DigitalContracts[unIndex] = oElement.GetBase64SerializedBuffer();
            unIndex++;
        }
        // Return the number of digital contracts downloaded
        __DebugAssert(unIndex == gs_strIndexedListOfSerializedBase64DigitalContracts.size());
        unDigitalContractsCount = (unsigned int) gs_strIndexedListOfSerializedBase64DigitalContracts.size();
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return unDigitalContractsCount;
}

/// <summary>
/// 
/// </summary>
/// <param name="unIndex"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetDigitalContractIdentifierAtIndex(
    _in unsigned unIndex
    )
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetDigitalContractIdentifierAtIndex(uint index);

    std::string strDigitalContractIdentifier;

    try
    {
        // Check to make sure we are logged in before trying to logout
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        // Make sure the index provided is not out of range
        _ThrowBaseExceptionIf((gs_strIndexedListOfSerializedBase64DigitalContracts.end() == gs_strIndexedListOfSerializedBase64DigitalContracts.find(unIndex)), "Index (%d) out of maximum range (%d)", unIndex, (gs_strIndexedListOfSerializedBase64DigitalContracts.size() - 1));
        // Deserialize the Base64 bit serialized StructuredBuffer containing the Digital Contract
        StructuredBuffer oDigitalContract(gs_strIndexedListOfSerializedBase64DigitalContracts.at(unIndex).c_str());
        // As an optimization, we use GimLet to compute the string hashes of different values at author
        // time so that at runtime, we do not have to do a whole bunch of string compares over and over
        // again
        strDigitalContractIdentifier = oDigitalContract.GetString("DigitalContractIdentifier");
    }

    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }

    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return ::ConvertToBSTR(strDigitalContractIdentifier);
}
/// <summary>
/// 
/// </summary>
/// <param name="unIndex"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) BSTR __cdecl GetDigitalContractProperty(
    _in const char * c_szDigitalContractIdentifier,
    _in const char * c_szPropertyName
    )
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetDigitalContractProperty(string digitalContractIdentifier, string propertyName);

    std::string strDigitalContractProperty;

    try
    {
        // Check to make sure we are logged in before trying to logout
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Cannot logout if not logged in...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        for (auto base64SerializedDigitalContract : gs_strIndexedListOfSerializedBase64DigitalContracts)
        {
            StructuredBuffer oDigitalContract(base64SerializedDigitalContract.second.c_str());
            std::string strDigitalContractIdentifier = oDigitalContract.GetString("DigitalContractIdentifier");
            if (0 == ::_strnicmp(c_szDigitalContractIdentifier, strDigitalContractIdentifier.c_str(), strDigitalContractIdentifier.size()))
            {
                // As an optimization, we use GimLet to compute the string hashes of different values at author
                // time so that at runtime, we do not have to do a whole bunch of string compares over and over
                // again
                switch (::Get64BitHashOfNullTerminatedString(c_szPropertyName, false))
                {
                    case 0x75D4B1A0DB8B9719 // "DigitalContractIdentifier"
                    :   strDigitalContractProperty = oDigitalContract.GetString("DigitalContractIdentifier");
                        break;
                    case 0xBAAEC6D09BC73437 // "VersionNumber"
                    :   strDigitalContractProperty = oDigitalContract.GetString("VersionNumber");
                        break;
                    case 0xE300C5F310350276 // "ContractStage"
                    :   strDigitalContractProperty = std::to_string((uint64_t) oDigitalContract.GetFloat64("ContractStage"));
                        break;
                    case 0x7A2F25973EEED0EE // "SubscriptionDays"
                    :   strDigitalContractProperty = std::to_string((uint64_t) oDigitalContract.GetFloat64("SubscriptionDays"));
                        break;
                    case 0xC7A10710CA77FF2A // "DatasetGuid"
                    :   strDigitalContractProperty = oDigitalContract.GetString("DatasetGuid");
                        break;
                    case 0x31829862E822916F // "ActivationTime"
                    :   strDigitalContractProperty = ::GetDateStringFromEpochMillisecondTimestamp((uint64_t) oDigitalContract.GetFloat64("ActivationTime"));
                        break;
                    case 0xCD578C737408CB7E // "ExpirationTime"
                    :   strDigitalContractProperty = ::GetDateStringFromEpochMillisecondTimestamp((uint64_t) oDigitalContract.GetFloat64("ExpirationTime"));
                        break;
                    case 0x8620265CA83C7F6E // "EulaAcceptedByDOOAuthorizedUser"
                    :   strDigitalContractProperty = oDigitalContract.GetString("EulaAcceptedByDOOAuthorizedUser");
                        break;
                    case 0xAACB401BA7F6F09F // "EulaAcceptedByROAuthorizedUser"
                    :   strDigitalContractProperty = oDigitalContract.GetString("EulaAcceptedByROAuthorizedUser");
                        break;
                    case 0x9A55771E1A635A3A // "LegalAgreement"
                    :   strDigitalContractProperty = oDigitalContract.GetString("LegalAgreement");
                        break;
                    default
                    :   _ThrowBaseException("Invalid Digital Contract property %s specified.", c_szPropertyName);
                        break;
                }

                // Break out of the loop
                break;
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

    return ::ConvertToBSTR(strDigitalContractProperty);
}

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl UploadInstallationPackageToVirtualMachine(
    _in const char * c_szIpAddressOfVirtualMachine,
    _in const char * c_szBase64EncodedInstallationPackage
    )
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool UploadInstallationPackageToVirtualMachine(string virtualMachineIpAddress, string base64EncodedInstallationPackage);

    bool fSuccess = false;

    try
    {
        // Check to make sure we are logged in before trying to logout
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        std::vector<std::string> stlHeaders;
        std::vector<Byte> stlResponse;
        unsigned int unLoopCounter = 120;
        do
        {
            stlResponse = ::RestApiCall(c_szIpAddressOfVirtualMachine, 9090, "POST", "/UploadData", c_szBase64EncodedInstallationPackage, true, stlHeaders);
            if (0 == stlResponse.size())
            {
                unLoopCounter -= 1;
                ::Sleep(5000);
            }
        }
        while ((0 <= unLoopCounter) && (0 == stlResponse.size()));

        fSuccess = true;
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
/// 
/// </summary>
/// <param name="c_szNameOfVirtualMachine"></param>
/// <param name="c_szIpAddressOfVirtualMachine"></param>
/// <param name="c_szVirtualMachineIdentifier"></param>
/// <param name="c_szClusterIdentifier"></param>
/// <param name="c_szDigitalContractIdentifier"></param>
/// <param name="c_szDatasetIdentifier"></param>
/// <param name="c_szRootOfTrustDomainIdentifier"></param>
/// <param name="c_szComputationalDomainIdentifier"></param>
/// <param name="c_szDataConnectorDomainIdentifier"></param>
/// <param name="c_szSailWebApiPortalIpAddress"></param>
/// <param name="c_szBase64EncodedDataset"></param>
/// <returns></returns>
extern "C" __declspec(dllexport) bool __cdecl UploadInitializationParametersToVirtualMachine(
    _in const char * c_szNameOfVirtualMachine,
    _in const char * c_szIpAddressOfVirtualMachine,
    _in const char * c_szVirtualMachineIdentifier,
    _in const char * c_szClusterIdentifier,
    _in const char * c_szDigitalContractIdentifier,
    _in const char * c_szDatasetIdentifier,
    _in const char * c_szRootOfTrustDomainIdentifier,
    _in const char * c_szComputationalDomainIdentifier,
    _in const char * c_szDataConnectorDomainIdentifier,
    _in const char * c_szSailWebApiPortalIpAddress,
    _in const char * c_szBase64EncodedDataset
    )
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public bool UploadInitializationParametersToVirtualMachine(string nameOfVirtualMachine, string ipAddressOfVirtualMachine, string virtualMachineIdentifier, string clusterIdentifier, string digitalContractIdentifier, string datasetIdentifier, string rootOfTrustDomainIdentifier, string computationalDomainIdentifier, string dataConnectorDomainIdentifier, string ipAddressOfSailWebApiPortal, string base64EncodedDataset, string importorEosb);
    
    bool fSuccess = false;

    try
    {
        // Check to make sure we are logged in before trying to logout
        _ThrowBaseExceptionIf((true == gs_strEosb.empty()), "Authentication is required...", nullptr);
        __DebugAssert(false == gs_strOrganizationIdentifier.empty());
        __DebugAssert(false == gs_strAuthenticatedUserIdentifier.empty());
        __DebugAssert(0 != gs_qwAuthenticatedUserAccessRights);

        // First we need to build out the huge StructuredBuffer with all of the initialization parameters
        StructuredBuffer oInitializationParameters;
        oInitializationParameters.PutString("NameOfVirtualMachine", c_szNameOfVirtualMachine);
        oInitializationParameters.PutString("IpAddressOfVirtualMachine", c_szIpAddressOfVirtualMachine);
        oInitializationParameters.PutString("VirtualMachineIdentifier", c_szVirtualMachineIdentifier);
        oInitializationParameters.PutString("ClusterIdentifier", c_szClusterIdentifier);
        oInitializationParameters.PutString("DigitalContractIdentifier", c_szDigitalContractIdentifier);
        oInitializationParameters.PutString("DatasetIdentifier", c_szDatasetIdentifier);
        oInitializationParameters.PutString("RootOfTrustDomainIdentifier", c_szRootOfTrustDomainIdentifier);
        oInitializationParameters.PutString("ComputationalDomainIdentifier", c_szComputationalDomainIdentifier);
        oInitializationParameters.PutString("DataConnectorDomainIdentifier", c_szDataConnectorDomainIdentifier);
        oInitializationParameters.PutString("SailWebApiPortalIpAddress", c_szSailWebApiPortalIpAddress);
        oInitializationParameters.PutString("Base64EncodedDataset", c_szBase64EncodedDataset);
        oInitializationParameters.PutString("DataOwnerAccessToken", gs_strEosb);
        oInitializationParameters.PutString("DataOwnerOrganizationIdentifier", gs_strOrganizationIdentifier);
        oInitializationParameters.PutString("DataOwnerUserIdentifier", gs_strAuthenticatedUserIdentifier);
        // Now we blast out the transaction
        std::vector<std::string> stlHeaders;
        std::vector<Byte> stlResponse;

        unsigned int unLoopCounter = 120;
        do
        {
            stlResponse = ::RestApiCall(c_szIpAddressOfVirtualMachine, 6800, "POST", "/SAIL/InitializationParameters", oInitializationParameters.GetBase64SerializedBuffer(), true, stlHeaders);
            if (0 == stlResponse.size())
            {
                unLoopCounter -= 1;
                ::Sleep(5000);
            }
            else
            {
                 // Parse the returning value.
                 StructuredBuffer oGetDigitalContractsResponse = JsonValue::ParseDataToStructuredBuffer((const char*)stlResponse.data());
                 // Did the transaction succeed?
                 _ThrowBaseExceptionIf(("Success" != oGetDigitalContractsResponse.GetString("Status")), "Initialization has failed. %s", (const char*)stlResponse.data(),nullptr);
                 fSuccess = true;
            }
        } while ((0 <= unLoopCounter) && (false == fSuccess));
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
/// 
/// </summary>
/// <param name=""></param>
/// <returns></returns>
extern "C" __declspec(dllexport) unsigned int __cdecl GetExceptionCount(void)
{
    __DebugFunction();

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // static extern public uint GetExceptionCount();

    unsigned int unNumberOfExceptions = 0;

    try
    {
        unNumberOfExceptions = ::GetRegisteredExceptionCount();
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

    // [DllImport("SailWebApiPortalFunctions.dll", CallingConvention = CallingConvention.Cdecl )]
    // [return: MarshalAs(UnmanagedType.BStr)]
    // static extern public string GetNextException();

    std::string strException = "";

    try
    {
        strException = ::GetNextRegisteredException();
    }

    catch (...)
    {

    }

    return ::ConvertToBSTR(strException);
}   