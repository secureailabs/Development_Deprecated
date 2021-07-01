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
#include "TlsClient.h"

static SailAuthentication * gs_oSailAuthentication = nullptr;

/********************************************************************************************
 *
 * @function CreateRequestPacket
 * @brief Create a Tls request packet to send to the database portal
 * @param[in] c_oRequest StructuredBuffer containing the request parameters
 * @return Serialized request packet
 *
 ********************************************************************************************/

std::vector<Byte> __stdcall CreateRequestPacket(
    _in const StructuredBuffer & c_oRequest
    )
{
    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(uint32_t) + c_oRequest.GetSerializedBufferRawDataSizeInBytes() + sizeof(Dword);

    std::vector<Byte> stlSerializedBuffer(unSerializedBufferSizeInBytes);
    Byte * pbSerializedBuffer = (Byte *) stlSerializedBuffer.data();

    // The format of the request data is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x436f6e74                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfRestRequestStructuredBuffer                                |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfRestRequestStructuredBuffer] RestRequestStructuredBuffer             |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x656e6420                                                                 |
    // +------------------------------------------------------------------------------------+

    *((Dword *) pbSerializedBuffer) = 0x436f6e74;
    pbSerializedBuffer += sizeof(Dword);
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) c_oRequest.GetSerializedBufferRawDataSizeInBytes();
    pbSerializedBuffer += sizeof(uint32_t);
    ::memcpy((void *) pbSerializedBuffer, (const void *) c_oRequest.GetSerializedBufferRawDataPtr(), c_oRequest.GetSerializedBufferRawDataSizeInBytes());
    pbSerializedBuffer += c_oRequest.GetSerializedBufferRawDataSizeInBytes();
    *((Dword *) pbSerializedBuffer) = 0x656e6420;

    return stlSerializedBuffer;
}

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
        gs_oSailAuthentication->TerminateSignalEncountered();
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
    m_fTerminationSignalEncountered = false;
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
 * @function TerminateSignalEncountered
 * @brief Set termination signal
 *
 ********************************************************************************************/

void __thiscall SailAuthentication::TerminateSignalEncountered(void)
{
    __DebugFunction();

    m_fTerminationSignalEncountered = true;
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

    // Add parameters for ShutdownPortal resource in a StructuredBuffer.
    StructuredBuffer oShutdownPortal;
    oShutdownPortal.PutStructuredBuffer("Eosb", oEosb);

    // Verifies user credentials and starts an authenticated session with SAIL SaaS
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AuthenticationManager/User/Login", oLoginParameters, 2);

    // Take in a full EOSB, call Cryptographic plugin and fetches user guid and organization guid
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AuthenticationManager/GetBasicUserInformation", oGetBasicUserInformationParameters, 1);

    // Take in a nonce and send back a certificate and public key
    m_oDictionary.AddDictionaryEntry("GET", "/SAIL/AuthenticationManager/RemoteAttestationCertificate", oGetRemoteAttestationCertificate, 1);

    // Shutdown the portal
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/AuthenticationManager/ShutdownPortal", oShutdownPortal, 0);

    // Reset the database
    m_oDictionary.AddDictionaryEntry("DELETE", "/SAIL/AuthenticationManager/Admin/ResetDatabase", 0);
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
        else if ("/SAIL/AuthenticationManager/ShutdownPortal" == strResource)
        {
            stlResponseBuffer = this->ShutdownPortal(c_oRequestStructuredBuffer);
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
    else if ("DELETE" == strVerb)
    {
       if ("/SAIL/AuthenticationManager/Admin/ResetDatabase" == strResource)
        {
            stlResponseBuffer = this->ResetDatabase(c_oRequestStructuredBuffer);
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

    Dword dwStatus = 404;
    Socket * poIpcAccountManager = nullptr, * poIpcCryptographicManager = nullptr;

    try
    {
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
        StructuredBuffer oCredentials;
        oCredentials.PutDword("TransactionType", 0x00000001);
        oCredentials.PutString("Passphrase", strPassphrase);
        poIpcAccountManager = ::ConnectToUnixDomainSocket("/tmp/{0BE996BF-6966-41EB-B211-2D63C9908289}");
        StructuredBuffer oAccountRecords(::PutIpcTransactionAndGetResponse(poIpcAccountManager, oCredentials));
        poIpcAccountManager->Release();
        poIpcAccountManager = nullptr;
        // Call CryptographicManager plugin to get the Eosb
        if ((0 < oAccountRecords.GetSerializedBufferRawDataSizeInBytes())&&(404 != oAccountRecords.GetDword("Status")) )
        {
            oAccountRecords.PutDword("TransactionType", 0x00000001);
            oAccountRecords.PutString("Passphrase", ::Base64HashOfEmailPassword(strEmail, strPassword));
            poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
            std::vector<Byte> stlEosb = ::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oAccountRecords);
            poIpcCryptographicManager->Release();
            poIpcCryptographicManager = nullptr;
            if (0 < stlEosb.size())
            {
                oResponse.PutBuffer("Eosb", stlEosb);
                dwStatus = 201;
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poIpcAccountManager)
    {
        poIpcAccountManager->Release();
    }
    if (nullptr != poIpcCryptographicManager)
    {
        poIpcCryptographicManager->Release();
    }

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
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

    Dword dwStatus = 401;
    Socket * poIpcCryptographicManager = nullptr;

    try 
    {
        std::vector<Byte> stlEosb = c_oRequest.GetBuffer("Eosb");

        StructuredBuffer oDecryptEosbRequest;
        oDecryptEosbRequest.PutDword("TransactionType", 0x00000007);
        oDecryptEosbRequest.PutBuffer("Eosb", stlEosb);

        // Call CryptographicManager plugin to get the decrypted eosb
        poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
        StructuredBuffer oDecryptedEosb(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oDecryptEosbRequest));
        poIpcCryptographicManager->Release();
        poIpcCryptographicManager = nullptr;
        if ((0 < oDecryptedEosb.GetSerializedBufferRawDataSizeInBytes())&&(201 == oDecryptedEosb.GetDword("Status")))
        {
            StructuredBuffer oEosb(oDecryptedEosb.GetStructuredBuffer("UserInformation").GetStructuredBuffer("Eosb"));
            // Send back the user information
            oResponse.PutGuid("UserGuid", oEosb.GetGuid("UserId"));
            oResponse.PutGuid("OrganizationGuid", oEosb.GetGuid("OrganizationGuid"));
            oResponse.PutString("OrganizationName", oEosb.GetString("OrganizationName"));
            // TODO: get user access rights, Username, Title, and Email from the confidential record, for now it can't be decrypted
            oResponse.PutQword("AccessRights", oEosb.GetQword("UserAccessRights"));
            oResponse.PutString("Username", oEosb.GetString("Username"));
            oResponse.PutString("Title", oEosb.GetString("Title"));
            oResponse.PutString("Email", oEosb.GetString("Email"));
            // Send back the updated Eosb
            oResponse.PutBuffer("Eosb", oDecryptedEosb.GetBuffer("UpdatedEosb"));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poIpcCryptographicManager)
    {
        poIpcCryptographicManager->Release();
    }

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);
    
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
    Socket * poIpcCryptographicManager = nullptr;

    try 
    {
        // Get nonce
        std::vector<Byte> stlNonce = c_oRequest.GetBuffer("Nonce");

        StructuredBuffer oRemoteAttestationCertificate;
        oRemoteAttestationCertificate.PutDword("TransactionType", 0x00000004);
        oRemoteAttestationCertificate.PutBuffer("MessageDigest", stlNonce);

        // Call CryptographicManager plugin to get the digital signature blob
        poIpcCryptographicManager = ::ConnectToUnixDomainSocket("/tmp/{AA933684-D398-4D49-82D4-6D87C12F33C6}");
        StructuredBuffer oPluginResponse(::PutIpcTransactionAndGetResponse(poIpcCryptographicManager, oRemoteAttestationCertificate));
        poIpcCryptographicManager->Release();
        poIpcCryptographicManager = nullptr;
        if ((0 < oPluginResponse.GetSerializedBufferRawDataSizeInBytes())&&(200 == oPluginResponse.GetDword("Status")))
        {
            // Add digital signature and public key to the response
            StructuredBuffer oDigitalSignature(oPluginResponse.GetStructuredBuffer("DSIG"));
            oResponse.PutBuffer("RemoteAttestationCertificatePem", oDigitalSignature.GetBuffer("DigitalSignature"));
            oResponse.PutString("PublicKeyCertificate", oDigitalSignature.GetString("PublicKeyPEM"));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poIpcCryptographicManager)
    {
        poIpcCryptographicManager->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function ShutdownPortal
 * @brief Shuts down the portal
 * @param[in] c_oRequest contains the request body
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::ShutdownPortal(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    // TODO: check if the user is a Sail admin or think of some other way to shut down the portal

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Shutdown the DatabaseGateway
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "POST");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/ShutdownPortal");
        std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
        // Send request packet
        poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

        // Read header and body of the response
        std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
        _ThrowBaseExceptionIf((0 != stlRestResponseLength.size()), "Database Gateway could not be shut down.", nullptr);
        // Make sure to release the poTlsNode
        poTlsNode->Release();
        poTlsNode = nullptr;
            
        // Send termination signal
        oResponse.PutBoolean("TerminateSignalEncountered", true);
        dwStatus = 200;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class SailAuthentication
 * @function ResetDatabase
 * @brief Reset the database
 * @param[in] c_oRequest contains the request body
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall SailAuthentication::ResetDatabase(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    // TODO: disable this function in production

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    TlsNode * poTlsNode = nullptr;

    try 
    {
        // Make a Tls connection with the database portal
        poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6500);
        // Reset the database
        StructuredBuffer oRequest;
        oRequest.PutString("PluginName", "DatabaseManager");
        oRequest.PutString("Verb", "DELETE");
        oRequest.PutString("Resource", "/SAIL/DatabaseManager/ResetDatabase");
        std::vector<Byte> stlRequest = ::CreateRequestPacket(oRequest);
        // Send request packet
        poTlsNode->Write(stlRequest.data(), (stlRequest.size()));

        // Read header and body of the response
        std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
        _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
        unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
        std::vector<Byte> stlResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
        _ThrowBaseExceptionIf((0 == stlResponse.size()), "Dead Packet.", nullptr);
        // Make sure to release the poTlsNode
        poTlsNode->Release();
            
        StructuredBuffer oDatabaseResponse(stlResponse);
        if (404 != oDatabaseResponse.GetDword("Status"))
        {
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
        // Add status if it was a dead packet
        if (strcmp("Dead Packet.",oException.GetExceptionMessage()) == 0)
        {
            dwStatus = 408;
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    if (nullptr != poTlsNode)
    {
        poTlsNode->Release();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}