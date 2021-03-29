/*********************************************************************************************
 *
 * @file SailAuthentication.cpp
 * @author Shabana Akhtar Baig
 * @date 16 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "CryptoUtils.h"
#include "SailAuthentication.h"
#include "SocketClient.h"
#include "IpcTransactionHelperFunctions.h"

static SailAuthentication * gs_oSailAuthentication = nullptr;

/********************************************************************************************
 *
 * @function GetSailAuthentication
 * @brief Create a singleton object of SailAuthentication class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of SailAuthentication class
 *
 ********************************************************************************************/

SailAuthentication * __stdcall GetSailAuthentication(void)
{
    __DebugFunction();

    if (nullptr == gs_oSailAuthentication)
    {
        gs_oSailAuthentication = new SailAuthentication();
        _ThrowOutOfMemoryExceptionIfNull(gs_oSailAuthentication);
    }

    return gs_oSailAuthentication;
}

/********************************************************************************************
 *
 * @function ShutdownSailAuthentication
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownSailAuthentication(void)
{
    __DebugFunction();

    if (nullptr != gs_oSailAuthentication)
    {
        gs_oSailAuthentication->Release();
        gs_oSailAuthentication = nullptr;
    }
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function SailAuthentication
 * @brief Constructor
 *
 ********************************************************************************************/

SailAuthentication::SailAuthentication(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;

    this->InitializeUserAccounts();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function SailAuthentication
 * @brief Copy Constructor
 * @param[in] c_oSailAuthentication Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

SailAuthentication::SailAuthentication(
    _in const SailAuthentication & c_oSailAuthentication
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function ~SailAuthentication
 * @brief Destructor
 *
 ********************************************************************************************/

SailAuthentication::~SailAuthentication(void)
{
    __DebugFunction();

    for (UserAccount * oUserAccount : m_stlUserAccounts)
    {
        delete oUserAccount;
    }
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall SailAuthentication::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "SailAuthentication";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall SailAuthentication::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{EE35DD63-7EB7-402D-9355-A209A5D0FD80}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall SailAuthentication::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function InitializeUserAccounts
 * @brief Insert user data
 *
 ********************************************************************************************/

void __thiscall SailAuthentication::InitializeUserAccounts(void)
{
    __DebugFunction();

    m_stlUserAccounts.push_back(new UserAccount("{FEB1CAE7-0F10-4185-A1F2-DE71B85DBD25}", "johnsnow", "sailpassword", "HBO"));
    m_stlUserAccounts.push_back(new UserAccount("{C1F45EF0-AB47-4799-9407-CA8A40CAC159}", "aryastark", "sailpassword", "HBO"));
    m_stlUserAccounts.push_back(new UserAccount("{0A83BCF5-2845-4437-AEBE-E02DFB349BAB}", "belle", "sailpassword", "Walt Disney"));
    m_stlUserAccounts.push_back(new UserAccount("{64E4FAC3-63C9-4844-BF82-1581F9C750CE}", "gaston", "sailpassword", "Walt Disney"));
    m_stlUserAccounts.push_back(new UserAccount("{F732CA9C-217E-4E3D-BF25-E2425B480556}", "hermoinegranger", "sailpassword", "Universal Studios"));
    m_stlUserAccounts.push_back(new UserAccount("{F3FBE722-1A42-4052-8815-0ABDDB3F2841}", "harrypotter", "sailpassword", "Universal Studios"));
    m_stlUserAccounts.push_back(new UserAccount("{2B9C3814-79D4-456B-B64A-ED79F69373D3}", "antman", "sailpassword", "Marvel Cinematic Universe"));
    m_stlUserAccounts.push_back(new UserAccount("{B40E1F9C-C100-46B3-BD7F-C80EB1351794}", "spiderman", "sailpassword", "Marvel Cinematic Universe"));
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall SailAuthentication::InitializePlugin(void)
{
    __DebugFunction();

    // Add parameters for AuthenticateUserCredentails resource in a StructuredBuffer.
    // Name, ElementType, and Range (if exists) are used by RestFrameworkRuntimeData::RunThread to vet request parameters.
    // Required parameters are marked by setting IsRequired to true
    // Otherwise the parameter is optional
    StructuredBuffer oLoginParameters;
    StructuredBuffer oEmail;
    oEmail.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oEmail.PutBoolean("IsRequired", true);
    oLoginParameters.PutStructuredBuffer("Email", oEmail);
    StructuredBuffer oPassword;
    oPassword.PutByte("ElementType", ANSI_CHARACTER_STRING_VALUE_TYPE);
    oPassword.PutBoolean("IsRequired", true);
    oLoginParameters.PutStructuredBuffer("Password", oPassword);

    // Add parameters for GetImposterEOSB resource in a StructuredBuffer.
    StructuredBuffer oGetImposterParameters;
    StructuredBuffer oEosb;
    oEosb.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oEosb.PutBoolean("IsRequired", true);
    oGetImposterParameters.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for GetBasicUserInformation resource in a StructuredBuffer.
    StructuredBuffer oGetBasicUserInformationParameters;
    oGetBasicUserInformationParameters.PutStructuredBuffer("Eosb", oEosb);

    // Add parameters for GetRemoteAttestationCertificate resource
    StructuredBuffer oGetRemoteAttestationCertificate;
    StructuredBuffer oNonce;
    oNonce.PutByte("ElementType", BUFFER_VALUE_TYPE);
    oNonce.PutBoolean("IsRequired", true);
    oGetRemoteAttestationCertificate.PutStructuredBuffer("Nonce", oNonce);

    // Verifies user credentials and starts an authenticated session with SAIL SaaS
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AuthenticationManager/User/Login", oLoginParameters);

    // Takes in an EOSB and sends back an imposter EOSB (IEOSB)
    // IEOSB has restricted rights and thus minimizes security risks when initializing and logging onto VM's
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AuthenticationManager/User/IEOSB", oGetImposterParameters);

    // Take in a full EOSB, call Cryptographic plugin and fetches user guid and organization guid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AuthenticationManager/GetBasicUserInformation", oGetBasicUserInformationParameters);

    // Take in a nonce and send back a certificate and public key
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AuthenticationManager/RemoteAttestationCertificate", oGetRemoteAttestationCertificate);

}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall SailAuthentication::SubmitRequest(
    _in const StructuredBuffer & c_oRequestStructuredBuffer,
    _out unsigned int * punSerializedResponseSizeInBytes
    )
{
    __DebugFunction();

    uint64_t un64Identifier = 0xFFFFFFFFFFFFFFFF;
    std::string strVerb = c_oRequestStructuredBuffer.GetString("Verb");
    std::string strResource = c_oRequestStructuredBuffer.GetString("Resource");
    // TODO: As an optimization, we should make sure to convert strings into 64 bit hashes
    // in order to speed up comparison. String comparisons WAY expensive.
    std::vector<Byte> stlResponseBuffer;

    // Route to the requested resource
    if ("POST" == strVerb)
    {
        if ("/SAIL/AuthenticationManager/User/Login" == strResource)
        {
            stlResponseBuffer = this->AuthenticateUserCredentails(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AuthenticationManager/User/IEOSB" == strResource)
        {
            stlResponseBuffer = this->GetImposterEOSB(c_oRequestStructuredBuffer);
        }
    }
    else if ("GET" == strVerb)
    {
        if ("/SAIL/AuthenticationManager/GetBasicUserInformation" == strResource)
        {
            stlResponseBuffer = this->GetBasicUserInformation(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/AuthenticationManager/RemoteAttestationCertificate" == strResource)
        {
            stlResponseBuffer = this->GetRemoteAttestationCertificate(c_oRequestStructuredBuffer);
        }
    }

    // Return size of response buffer
    *punSerializedResponseSizeInBytes = stlResponseBuffer.size();
    __DebugAssert(0 < *punSerializedResponseSizeInBytes);

    // Save the response buffer and increment transaction identifier which will be assigned to the next transaction
    ::pthread_mutex_lock(&m_sMutex);
    if (0xFFFFFFFFFFFFFFFF == m_unNextAvailableIdentifier)
    {
        m_unNextAvailableIdentifier = 0;
    }
    un64Identifier = m_unNextAvailableIdentifier;
    m_unNextAvailableIdentifier++;
    m_stlCachedResponse[un64Identifier] = stlResponseBuffer;
    ::pthread_mutex_unlock(&m_sMutex);

    return un64Identifier;
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
 ********************************************************************************************/

 bool __thiscall SailAuthentication::GetResponse(
     _in uint64_t un64Identifier,
     _out Byte * pbSerializedResponseBuffer,
     _in unsigned int unSerializedResponseBufferSizeInBytes
     )
 {
    __DebugFunction();
    __DebugAssert(0xFFFFFFFFFFFFFFFF != un64Identifier);
    __DebugAssert(nullptr != pbSerializedResponseBuffer);
    __DebugAssert(0 < unSerializedResponseBufferSizeInBytes);

    bool fSuccess = false;

    ::pthread_mutex_lock(&m_sMutex);
    if (m_stlCachedResponse.end() != m_stlCachedResponse.find(un64Identifier))
    {
        __DebugAssert(0 < m_stlCachedResponse[un64Identifier].size());

        ::memcpy((void *) pbSerializedResponseBuffer, (const void *) m_stlCachedResponse[un64Identifier].data(), m_stlCachedResponse[un64Identifier].size());
        m_stlCachedResponse.erase(un64Identifier);
        fSuccess = true;
    }
    ::pthread_mutex_unlock(&m_sMutex);

    return fSuccess;
 }

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function AuthenticateUserCredentails
 * @brief Validate a email/password set of credentials
 * @param[in] c_oRequest contains the request body
 * @returns Generated EOSB
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::AuthenticateUserCredentails(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    // Validate user credentials
    std::string strEmail = c_oRequest.GetString("Email");
    std::string strPassword = c_oRequest.GetString("Password");
    std::string strPassphrase;

    // Trim whitespaces in email and convert all letters to lowercase
    strEmail.erase(std::remove_if(strEmail.begin(), strEmail.end(), ::isspace), strEmail.end());
    std::transform(strEmail.begin(), strEmail.end(), strEmail.begin(), ::tolower);
    // Generate email/password string
    strPassphrase = strEmail + "/" + strPassword;

    // Call AccountManager plugin to fetch BasicUser and ConfidentialUser records from the database
    bool fSuccess = false;
    StructuredBuffer oCredentials;
    oCredentials.PutDword("TransactionType", 0x00000001);
    oCredentials.PutString("Passphrase", strPassphrase);
    Socket * poIpcAccountManager =  ::ConnectToUnixDomainSocket("/tmp/{0BE996BF-6966-41EB-B211-2D63C9908289}");
    StructuredBuffer oAccountRecords(::PutIpcTransactionAndGetResponse(poIpcAccountManager, oCredentials));

    // Call CryptographicManager plugin to get the Eosb
    if ((0 < oAccountRecords.GetSerializedBufferRawDataSizeInBytes())&&(404 != oAccountRecords.GetDword("Status")) )
    {
        oAccountRecords.PutDword("TransactionType", 0x00000001);
        oAccountRecords.PutString("Passphrase", ::Base64HashOfEmailPassword(strEmail, strPassword));
        Socket * poIpcCryptographicManager =  ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
        std::vector<Byte> stlEosb = ::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oAccountRecords);
        if (0 < stlEosb.size())
        {
            fSuccess = true;
            oResponse.PutDword("Status", 201);
            oResponse.PutBuffer("Eosb", stlEosb);
        }
    }
    // Add error code if login was unsuccessful
    if (false == fSuccess)
    {
        oResponse.PutDword("Status", 404);
    }

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetImposterEOSB
 * @brief Take in a full EOSB and mark it as an imposter EOSB
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Marked Imposter EOSB
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::GetImposterEOSB(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oIEosb;

    // TODO: Add a call to CryptographicManager to authenticate Eosb
    std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");
    Byte * pbCurrentByte = (Byte *) stlEosb.data();
    __DebugAssert(nullptr != pbCurrentByte);

    _ThrowBaseExceptionIf((0xE62110021B65A123 != *((Qword *) pbCurrentByte)), "Invalid serialization format: Expected [HEADER] = 0xE62110021B65A123 but got 0x%08X", *((Qword *) pbCurrentByte));
    pbCurrentByte += sizeof(Qword);
    uint32_t unSsbSizeInBytes = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    StructuredBuffer oSsb(pbCurrentByte, unSsbSizeInBytes);

    // Mark the Imposter EOSB
    if (0x4 == oSsb.GetQword("AccessRights"))
    {
        // Replace AccessRights = 0x4 with AccessRights = 0x2, where 0x2 means the EOSB is an Imposter EOSB
        oSsb.PutQword("AccessRights", 0x2);
        ::memcpy((void *) pbCurrentByte, (const void *) oSsb.GetSerializedBufferRawDataPtr(), oSsb.GetSerializedBufferRawDataSizeInBytes());
    }

    pbCurrentByte += unSsbSizeInBytes;
    _ThrowBaseExceptionIf((0x321A56B12991126E != *((Qword *) pbCurrentByte)), "Invalid serialization format: Expected [HEADER] = 0x321A56B12991126E but got 0x%08X", *((Qword *) pbCurrentByte));
    oIEosb.PutBuffer("Eosb", stlEosb);

    return oIEosb.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetBasicUserInformation
 * @brief Take in a full EOSB, call Cryptographic plugin and get user guid and organization guid
 * @param[in] c_oRequest contains the Eosb
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized buffer containing user guid and organization guid
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::GetBasicUserInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");

    StructuredBuffer oDecryptEosbRequest;
    oDecryptEosbRequest.PutDword("TransactionType", 0x00000002);
    oDecryptEosbRequest.PutBuffer("Eosb", stlEosb);

    // Call CryptographicManager plugin to get the decrypted eosb
    bool fSuccess = false;
    Socket * poIpcCryptographicManager =  ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
    StructuredBuffer oDecryptedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest));
    if ((0 < oDecryptedEosb.GetSerializedBufferRawDataSizeInBytes())&&(201 == oDecryptedEosb.GetDword("Status")))
    {
        StructuredBuffer oEosb(oDecryptedEosb.GetStructuredBuffer("Eosb"));
        oResponse.PutDword("Status", 200);
        oResponse.PutGuid("UserGuid", oEosb.GetGuid("UserId"));
        oResponse.PutGuid("OrganizationGuid", oEosb.GetGuid("OrganizationGuid"));
        // TODO: get user access rights from the confidential record, for now it can't be decrypted
        oResponse.PutQword("AccessRights", oEosb.GetQword("UserAccessRights"));
        fSuccess = true;
    }
    // Add error code if transaction was unsuccessful
    if (false == fSuccess)
    {
        oResponse.PutDword("Status", 404);
    }

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function GetRemoteAttestationCertificate
 * @brief Take in a nonce and send back a certificate and public key
 * @param[in] c_oRequest contains the request body
 * @returns remote attestation certificate and public key
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::GetRemoteAttestationCertificate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    // Get nonce
    std::vector<Byte> stlNonce = c_oRequest.GetBuffer("Nonce");

    StructuredBuffer oRemoteAttestationCertificate;
    oRemoteAttestationCertificate.PutDword("TransactionType", 0x00000004);
    oRemoteAttestationCertificate.PutBuffer("MessageDigest", stlNonce);

    // Call CryptographicManager plugin to get the digital signature blob
    Socket * poIpcCryptographicManager =  ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
    StructuredBuffer oPluginResponse(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oRemoteAttestationCertificate));
    if ((0 < oPluginResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oPluginResponse.GetDword("Status")))
    {
        // Add digital signature and public key to the response
        StructuredBuffer oDigitalSignature(oPluginResponse.GetStructuredBuffer("DSIG"));
        oResponse.PutBuffer("RemoteAttestationCertificatePem", oDigitalSignature.GetBuffer("DigitalSignature"));
        oResponse.PutString("PublicKeyCertificate", oDigitalSignature.GetString("PublicKeyPEM"));
        dwStatus = 200;
    }
    else
    {
        _ThrowBaseException("Error getting digital signatures.", nullptr);
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
