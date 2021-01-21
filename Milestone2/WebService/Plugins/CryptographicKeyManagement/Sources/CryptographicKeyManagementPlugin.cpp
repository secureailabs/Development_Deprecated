/*********************************************************************************************
 *
 * @file CryptographicKeyManagementPlugin.cpp
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "CryptographicKeyManagementPlugin.h"
#include "HardCodedCryptographicKeys.h"
#include "Base64Encoder.h"
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>

static CryptographicKeyManagementPlugin * gs_oCryptographicKeyManagementPlugin = nullptr;

/********************************************************************************************
 *
 * @function GetCryptographicKeyManagementPlugin
 * @brief Create a singleton object of CryptographicKeyManagementPlugin class
 * @throw OutOfMemoryException If there isn't enough memory left to create a new instance
 * @return Return the singleton object of CryptographicKeyManagementPlugin class
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin * __stdcall GetCryptographicKeyManagementPlugin(void)
{
    __DebugFunction();

    if (nullptr == gs_oCryptographicKeyManagementPlugin)
    {
        gs_oCryptographicKeyManagementPlugin = new CryptographicKeyManagementPlugin();
        _ThrowOutOfMemoryExceptionIfNull(gs_oCryptographicKeyManagementPlugin);
    }

    return gs_oCryptographicKeyManagementPlugin;
}

/********************************************************************************************
 *
 * @function ShutdownCryptographicKeyManagementPlugin
 * @brief Release the object resources and set global static pointer to NULL
 *
 ********************************************************************************************/

void __stdcall ShutdownCryptographicKeyManagementPlugin(void)
{
    __DebugFunction();

    if (nullptr != gs_oCryptographicKeyManagementPlugin)
    {
        gs_oCryptographicKeyManagementPlugin->Release();
        gs_oCryptographicKeyManagementPlugin = nullptr;
    }
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CryptographicKeyManagementPlugin
 * @brief Constructor
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin::CryptographicKeyManagementPlugin(void)
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_unNextAvailableIdentifier = 0;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function CryptographicKeyManagementPlugin
 * @brief Copy Constructor
 * @param[in] c_oCryptographicKeyManagementPlugin Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin::CryptographicKeyManagementPlugin(
    _in const CryptographicKeyManagementPlugin & c_oCryptographicKeyManagementPlugin
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function ~CryptographicKeyManagementPlugin
 * @brief Destructor
 *
 ********************************************************************************************/

CryptographicKeyManagementPlugin::~CryptographicKeyManagementPlugin(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetName
 * @brief Fetch the name of the plugin
 * @return Name of the plugin
 *
 ********************************************************************************************/

const char * __thiscall CryptographicKeyManagementPlugin::GetName(void) const throw()
{
    __DebugFunction();

    static const char * sc_szName = "CryptographicKeyManagementPlugin";

    return sc_szName;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetUuid
 * @brief Fetch the UUID of the plugin
 * @return UUID of the plugin
 *
 ********************************************************************************************/

const char * __thiscall CryptographicKeyManagementPlugin::GetUuid(void) const throw()
{
    __DebugFunction();

    static const char * sc_szUuid = "{30998245-A931-4518-9A9D-FB0F43F1F02D}";

    return sc_szUuid;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetVersion
 * @brief Fetch the current version of the plugin
 * @return Version of the plugin
 *
 ********************************************************************************************/

Qword __thiscall CryptographicKeyManagementPlugin::GetVersion(void) const throw()
{
    __DebugFunction();

    return 0x0000000100000001;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetDictionarySerializedBuffer
 * @brief Fetch the serialized buffer of the plugin's dictionary
 * @return Serialized buffer of the plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetDictionarySerializedBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionary.GetSerializedDictionary();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function InitializePlugin
 * @brief Initializer that initializes the plugin's dictionary
 *
 ********************************************************************************************/

void __thiscall CryptographicKeyManagementPlugin::InitializePlugin(void)
{
    __DebugFunction();

    // Adds the first part of the login sequence which feeds the remote endpoint a nonce
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/Initializer/ChallengeNonce");
    // Takes in the nonce, verify it and then return an ephemeral key pair and certificate
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/Initializer/KeyPairAndCertificate");
    // Takes in the EOSB, verify it and then return the digitally signed hash and Public Key Certificate
    m_oDictionary.AddDictionaryEntry("POST", "/SAIL/Initializer/SignDatasetHash");
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function SubmitRequest
 * @brief Method called by flat function SubmitRequest when a client requests for the plugin's resource
 * @param[in] c_oRequestStructuredBuffer points to the request body
 * @param[out] punSerializedResponseSizeInBytes stores the size of the response
 * @throw BaseException Element not found
 * @throw BaseException Error generating challenge nonce
 * @returns a 64 bit unique transaction identifier
 *
 ********************************************************************************************/

uint64_t __thiscall CryptographicKeyManagementPlugin::SubmitRequest(
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

    ::pthread_mutex_lock(&m_sMutex);
    if (0xFFFFFFFFFFFFFFFF == m_unNextAvailableIdentifier)
    {
        m_unNextAvailableIdentifier = 0;
    }
    un64Identifier = m_unNextAvailableIdentifier;
    m_unNextAvailableIdentifier++;
    ::pthread_mutex_unlock(&m_sMutex);

    // Route to the requested resource
    if ("POST" == strVerb)
    {
        if ("/SAIL/Initializer/ChallengeNonce" == strResource)
        {
            stlResponseBuffer = this->GetChallengeNonceForInitializer(c_oRequestStructuredBuffer, un64Identifier);
        }
        else if ("/SAIL/Initializer/KeyPairAndCertificate" == strResource)
        {
            stlResponseBuffer = this->GetKeyPairAndCertificateForInitializer(c_oRequestStructuredBuffer);
        }
        else if ("/SAIL/Initializer/SignDatasetHash" == strResource)
        {
            stlResponseBuffer = this->GetSignedHashAndCertificate(c_oRequestStructuredBuffer);
        }
    }

    // Return size of response buffer
    *punSerializedResponseSizeInBytes = stlResponseBuffer.size();
    __DebugAssert(0 < *punSerializedResponseSizeInBytes);

    // Save the response buffer and increment transaction identifier which will be assigned to the next transaction
    ::pthread_mutex_lock(&m_sMutex);
    m_stlCachedResponse[un64Identifier] = stlResponseBuffer;
    ::pthread_mutex_unlock(&m_sMutex);

    return un64Identifier;
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetResponse
 * @brief Method called by flat function GetResponse to get plugin's response
 * @param[in] un64Identifier is the transaction identifier
 * @param[out] c_pbSerializedResponseBuffer points to the GetResponse
 * @params[in] unSerializedResponseBufferSizeInBytes is used to verify the request
 * @returns a boolean that represents status of the request
 *
 ********************************************************************************************/

 bool __thiscall CryptographicKeyManagementPlugin::GetResponse(
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
 * @class CryptographicKeyManagementPlugin
 * @function GetChallengeNonceForInitializer
 * @brief Generated Nonce to authenticate Initializer
 * @param[in] c_oRequest contains the request body
 * @param[in] un64BitIdentifier contains the nonce index
 * @throw BaseException Error generating challenge nonce
 * @returns generated nonce
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetChallengeNonceForInitializer(
    _in const StructuredBuffer & c_oRequest,
    _in uint64_t un64BitIdentifier
    )
{
    __DebugFunction();

    StructuredBuffer oChallengeNonce;

    // Generate a Nonce using OpenSSL
    Byte abNonce[256];
    int nStatus = ::RAND_bytes(abNonce, sizeof(abNonce));
    _ThrowBaseExceptionIf((1 != nStatus), "Error generating challenge nonce, nStatus: %d.", nStatus);

    // Store the newly generated Nonce in a vector and insert it in m_stlChallengeNonce map
    std::vector<Byte> stlChallengeNonce;
    stlChallengeNonce.resize(sizeof(abNonce));
    ::memcpy((void *) stlChallengeNonce.data(), abNonce, sizeof(abNonce));

    // Insert the vector containing the challenge nonce in m_stlChallengeNonce associating it with the un64BitIdentifier
    ::pthread_mutex_lock(&m_sMutex);
    m_stlChallengeNonce[un64BitIdentifier] = stlChallengeNonce;
    ::pthread_mutex_unlock(&m_sMutex);

    // Send back the Challenge Nonce and the associated un64BitIdentifier
    // The un64BitIdentifier will be used by GetKeyPairAndCertificateForInitializer
    // to get the ChallengeNonce from the map and compare it with the request's ChallengeNonce
    oChallengeNonce.PutQword("ChallengeNonceId", un64BitIdentifier);
    oChallengeNonce.PutBuffer("Nonce", stlChallengeNonce.data(), stlChallengeNonce.size());

    return oChallengeNonce.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetKeyPairAndCertificateForInitializer
 * @brief Compare the Proof with the generated Nonce and send back signed certificate
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error verifying remote attestation proof and certificate
 * @returns signed certificate
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetKeyPairAndCertificateForInitializer(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oInitializerTlsPublicKeyCertificate;
    // Get required parameters
    StructuredBuffer oRemoteAttestationProofAndCertificate = c_oRequest.GetStructuredBuffer("RequiredParameters");
    uint64_t un64BitIdentifier = oRemoteAttestationProofAndCertificate.GetQword("ChallengeNonceId");
    std::vector<Byte> stlProof = oRemoteAttestationProofAndCertificate.GetBuffer("Proof");
    std::vector<Byte> stlCertificate = oRemoteAttestationProofAndCertificate.GetBuffer("Certificate");

    ::pthread_mutex_lock(&m_sMutex);
    // Verify that the ChallengeNonce is the same
    if ((m_stlChallengeNonce.end() != m_stlChallengeNonce.find(un64BitIdentifier)) && (stlProof == m_stlChallengeNonce[un64BitIdentifier]))
    {
        oInitializerTlsPublicKeyCertificate.PutBuffer("Certificate", gc_abRootPublicKeyCertificate, gc_unRootPublicKeyCertificateSizeInBytes);
        m_stlChallengeNonce.erase(un64BitIdentifier);
    }
    else
    {
      ::pthread_mutex_unlock(&m_sMutex);
      _ThrowBaseException("Error verifying remote attestation proof and certificate", nullptr);
    }

    ::pthread_mutex_unlock(&m_sMutex);

    return oInitializerTlsPublicKeyCertificate.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class CryptographicKeyManagementPlugin
 * @function GetSignedHashAndCertificate
 * @brief Generate a signed hash and public key certificate for the requested Hash
 * @param[in] c_oRequest contains the request body

 * @throw BaseException On signing failure
 * @returns Serialized structuredBuffer containing signed hash and certificate
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicKeyManagementPlugin::GetSignedHashAndCertificate(
    _in const StructuredBuffer & c_oRequest
    ) throw()
{
    __DebugFunction();

    StructuredBuffer oStructuredBufferResponse;

    // Get required parameters
    StructuredBuffer oStructuredBufferESOBAndHash = c_oRequest.GetStructuredBuffer("RequiredParameters");
    std::string strBase64Hash = oStructuredBufferESOBAndHash.GetString("base64SHA2Hash");

    // Decode the hash form base63 encoding
    std::vector<Byte> stlSHA256Hash = Base64Decode(strBase64Hash.data());

    // TODO: Use the ESOB to get the key and verify if the UserUuid is a valid Data Owner
    StructuredBuffer oEosb(oStructuredBufferESOBAndHash.GetBuffer("Eosb"));
    std::string strUserUuid = oEosb.GetString("UserUuid");
    bool fIsImposter = oEosb.GetBoolean("IsImposter");
    _ThrowBaseExceptionIf((true == fIsImposter), "Imposter EOSB cannot be used to sign the dataset.", nullptr);

    // Create a BIO buffer to read the keys and certificates from
    BIO * poBIO = ::BIO_new(BIO_s_mem());
    _ThrowIfNull(poBIO, "Creating BIO buffer failed.", nullptr);

    // Convert the data owner private key from the PEM format to the EC_KEY strucutre
    int nBytesWirttenToBio = ::BIO_write(poBIO, gc_abDataOwnerPrivateKey, gc_unDataOwnerPrivateKeySizeInBytes);
    _ThrowBaseExceptionIf((gc_unDataOwnerPrivateKeySizeInBytes != nBytesWirttenToBio), "Writing to BIO buffer failed.", nullptr);

    EVP_PKEY * poDataOwnerPrivateKey = nullptr;
    ::PEM_read_bio_PrivateKey(poBIO, &poDataOwnerPrivateKey, 0, 0);
    EC_KEY * oECDSAPrivateKey = ::EVP_PKEY_get1_EC_KEY(poDataOwnerPrivateKey);
    _ThrowIfNull(oECDSAPrivateKey, "Fetch Signing Key failed.", nullptr);

    long nSslStatus = ::BIO_ctrl(poBIO, BIO_CTRL_RESET, 0, nullptr);
    _ThrowBaseExceptionIf((1 != nSslStatus), "TLS failed: Write BIO reset failed", nullptr);

    // Convert the Root Of Trust private key from the PEM format to the EC_KEY strucutre
    nBytesWirttenToBio = ::BIO_write(poBIO, gc_abRootPrivateKey, gc_unRootPrivateKeySizeInBytes);
    _ThrowBaseExceptionIf((gc_unRootPrivateKeySizeInBytes != nBytesWirttenToBio), "Writing to BIO buffer failed.", nullptr);

    EVP_PKEY * poRootOfTrustPrivateKey = nullptr;
    ::PEM_read_bio_PrivateKey(poBIO, &poRootOfTrustPrivateKey, 0, 0);
    EC_KEY * poECDSARootOfTrustPrivateKey = ::EVP_PKEY_get1_EC_KEY(poRootOfTrustPrivateKey);
    _ThrowIfNull(poECDSARootOfTrustPrivateKey, "Fetch Root Signing Key failed.", nullptr);

    nSslStatus = ::BIO_ctrl(poBIO, BIO_CTRL_RESET, 0, nullptr);
    _ThrowBaseExceptionIf((1 != nSslStatus), "TLS failed: Write BIO reset failed", nullptr);

    // Convert the PEM certiificate to X509 structure
    nBytesWirttenToBio = ::BIO_write(poBIO, gc_abDataOwnerPublicKeyCertificate, gc_unDataOwnerPublicKeyCertificateSizeInBytes);
    _ThrowBaseExceptionIf((gc_unDataOwnerPublicKeyCertificateSizeInBytes != nBytesWirttenToBio), "Writing Public Key Cert to BIO failed", nullptr);

    X509 * poX509DataOwnerPublicKeyCertificate = nullptr;
    ::PEM_read_bio_X509(poBIO, &poX509DataOwnerPublicKeyCertificate, 0, 0);
    nSslStatus = ::BIO_ctrl(poBIO, BIO_CTRL_RESET, 0, nullptr);
    _ThrowBaseExceptionIf((1 != nSslStatus), "TLS failed: Write BIO reset failed", nullptr);

    // Sign the SHA256 hash
    ECDSA_SIG * poECDSASignature = ::ECDSA_do_sign(stlSHA256Hash.data(), stlSHA256Hash.size(), oECDSAPrivateKey);
    _ThrowIfNull(poECDSASignature, "Signing of Hash failed.", nullptr);

    Byte * pDerEncodedSignature = nullptr;
    int nLengthOfDerEncodedSignature = ::i2d_ECDSA_SIG(poECDSASignature, &pDerEncodedSignature);
    _ThrowBaseExceptionIf((0 > nLengthOfDerEncodedSignature), "Conversion SIGNATURE to DER format failed.", nullptr);

    oStructuredBufferResponse.PutString("SignedHash", ::Base64Encode(pDerEncodedSignature, nLengthOfDerEncodedSignature));

    // Sign the certificate
    int nSizeOfSignature = ::X509_sign(poX509DataOwnerPublicKeyCertificate, poRootOfTrustPrivateKey, ::EVP_sha256());
    _ThrowBaseExceptionIf((0 >= nSizeOfSignature), "Signing the DataOwner Public Key Certificate failed", nullptr);

    ::PEM_write_bio_X509(poBIO, poX509DataOwnerPublicKeyCertificate);
    Byte * pDataInBIO = nullptr;
    size_t nDataInBIOSize = ::BIO_get_mem_data(poBIO, &pDataInBIO);
    _ThrowBaseExceptionIf((0 >= nDataInBIOSize), "Reading Signed Public Key Certificate from buffer failed", nullptr);

    // Put the signed certificate in the Response Structured Buffer
    oStructuredBufferResponse.PutString("SignedCertificate", std::string((char *)pDataInBIO));

    // Free the SSL buffers
    ::BIO_free(poBIO);
    ::EVP_PKEY_free(poRootOfTrustPrivateKey);
    ::X509_free(poX509DataOwnerPublicKeyCertificate);
    ::ECDSA_SIG_free(poECDSASignature);
    ::EC_KEY_free(poECDSARootOfTrustPrivateKey);

    return oStructuredBufferResponse.GetSerializedBuffer();
}
