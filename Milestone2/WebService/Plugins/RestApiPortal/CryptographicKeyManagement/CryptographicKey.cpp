/*********************************************************************************************
 *
 * @file CryptographicKey.cpp
 * @author Prawal Gangwar
 * @date 11 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "Object.h"
#include "CryptographicEngine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "Base64Encoder.h"

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

#include <vector>
#include <iostream>
#include <string>
#include <fstream>

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function CryptographicKey
 * @brief Constructor with no initial key
 *
 ********************************************************************************************/

CryptographicKey::CryptographicKey(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function CryptographicKey
 * @brief Copy Constructor
 *
 ********************************************************************************************/

CryptographicKey::CryptographicKey(
    _in const CryptographicKey & c_oCryptographicKey
    )
{
    __DebugFunction();

    m_oKeyGuid = c_oCryptographicKey.m_oKeyGuid;
    m_KeySpec = c_oCryptographicKey.m_KeySpec;
    m_poPrivateKey = c_oCryptographicKey.m_poPrivateKey;
    m_poPublicKey = c_oCryptographicKey.m_poPublicKey;
    m_stlSymmetricKey = c_oCryptographicKey.m_stlSymmetricKey;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function ~CryptographicKey
 * @brief Destructor
 *
 ********************************************************************************************/

CryptographicKey::~CryptographicKey(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function CryptographicKey
 * @brief Constructor which creates new keys based on input Key Spec Algo
 * @param[in] KeySpec Enum to specify the key type
 * @throw BaseException on an internal error occurs
 *
 ********************************************************************************************/

CryptographicKey::CryptographicKey(
    _in KeySpec eKeyAlgo
    )
{
    __DebugFunction();

    m_KeySpec = eKeyAlgo;

    switch (eKeyAlgo)
    {
        case KeySpec::eRSA2048
        :   this->GenerateRsaKeyPair(2048);
            break;
        case KeySpec::eRSA3076
        :   this->GenerateRsaKeyPair(3076);
            break;
        case KeySpec::eRSA4096
        :   this->GenerateRsaKeyPair(4096);
            break;
        case KeySpec::eECC384
        :   this->GenerateEccKeyPair("secp384r1");
            break;
        case KeySpec::eAES256
        :   this->GenerateAesKey(256);
            break;
        case KeySpec::eAES128
        :   this->GenerateAesKey(128);
            break;
        default
        :   _ThrowBaseException("Invalid Key Spec", nullptr);
            break;
    }
    m_oKeyGuid = Guid();
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function CryptographicKey
 * @brief Constructor to create AES-256 password derived keys
 * @param[in] KeySpec Enum to specify the key type
 * @param[in] eHashAlgorithm Hash Funciton to use for ket derivation
 * @param[in] c_szPassword Password/Passphrase used to derive key
 * @throw BaseException on an internal error occurs
 *
 ********************************************************************************************/

CryptographicKey::CryptographicKey(
    _in KeySpec eKeySpec,
    _in HashAlgorithm eHashAlgorithm,
    _in const std::string & c_strPassword
    )
{
    __DebugFunction();
    __DebugAssert(KeySpec::ePDKDF2 == eKeySpec);

    const EVP_MD * c_poEvpMd = ::GetEVP_MDForHashAlgorithm(eHashAlgorithm);

    // The generated keys are always 256 bits or 32 bytes + 16 bytes for the deterministic IV
    // Only AES CFB is used with password generated keys for now.
    m_stlSymmetricKey.resize(32 + AES_CFB_IV_LENGTH);

    // Taking 12 extra bytes where the IV for the AES is generated so that it does not need
    // to stay with the cipher text and can be generated deterministically everytime
    int nKeyGenStatus = ::PKCS5_PBKDF2_HMAC(c_strPassword.c_str(), c_strPassword.length(), nullptr, 0, 1024, c_poEvpMd, m_stlSymmetricKey.size(), m_stlSymmetricKey.data());
    _ThrowBaseExceptionIf((1 != nKeyGenStatus), "Failed to derive symmetric key from password.", nullptr);

    m_KeySpec = eKeySpec;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function CryptographicKey
 * @brief Constructor which creates new keys based on input Key Spec Algo
 * @param[in] oGuid Guid of the key to initialize with
 * @throw BaseException on an internal error occurs
 *
 ********************************************************************************************/

CryptographicKey::CryptographicKey(
    _in Guid oGuid
)
{
    __DebugFunction();

    std::ifstream oKeyFile(oGuid.ToString(eRaw)+".key", std::ios::binary | std::ios::ate);
    std::streamsize size = oKeyFile.tellg();
    _ThrowBaseExceptionIf((0 >= size), "No stored key found", nullptr);
    m_oKeyGuid = oGuid;
    oKeyFile.seekg(0, std::ios::beg);

    std::vector<char> stlFileData(size);

    if (oKeyFile.read(stlFileData.data(), size))
    {
        StructuredBuffer oStructuredBufferKey((unsigned char *)stlFileData.data(), size);
        std::string strKeySpec = oStructuredBufferKey.GetString("KeySpec");
        m_KeySpec = ::GetKeySpecFromString(strKeySpec);

        if ((m_KeySpec == KeySpec::eAES128) || (m_KeySpec == KeySpec::eAES256) || (m_KeySpec == KeySpec::ePDKDF2))
        {
            m_stlSymmetricKey = oStructuredBufferKey.GetBuffer("Key");
        }
        else if ((m_KeySpec == KeySpec::eRSA2048) || (m_KeySpec == KeySpec::eRSA3076) || (m_KeySpec == KeySpec::eRSA4096) || (m_KeySpec == KeySpec::eECC384))
        {
            std::string strPemPublicKey = oStructuredBufferKey.GetString("PublicKey");
            BioUniquePtr poBio(::BIO_new(BIO_s_mem()), ::BIO_free);
            _ThrowOutOfMemoryExceptionIfNull(poBio);

            int nOpenSslStatus = ::BIO_write(poBio.get(), strPemPublicKey.c_str(), strPemPublicKey.length());
            _ThrowBaseExceptionIf((strPemPublicKey.length() != nOpenSslStatus), "Could not write to BIO", nullptr);

            m_poPublicKey.reset(::PEM_read_bio_PUBKEY(poBio.get(), nullptr, nullptr, nullptr), ::EVP_PKEY_free);
            _ThrowIfNull(m_poPublicKey, "Couldn't extract public key from PEM string", nullptr);

            std::string strPemPrivateKey = oStructuredBufferKey.GetString("PrivateKey");
            nOpenSslStatus = ::BIO_write(poBio.get(), strPemPrivateKey.c_str(), strPemPrivateKey.length());
            _ThrowBaseExceptionIf((strPemPrivateKey.length() != nOpenSslStatus), "Could not write to BIO", nullptr);

            m_poPrivateKey.reset(::PEM_read_bio_PrivateKey(poBio.get(), nullptr, nullptr, nullptr), ::EVP_PKEY_free);
            _ThrowIfNull(m_poPrivateKey, "Couldn't extract public key from PEM string", nullptr);
        }
        else
        {
            _ThrowBaseException("Invalid KeySpec in key file", nullptr);
        }
    }
    else
    {
        _ThrowBaseException("No stored key found.", nullptr);
    }
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GenerateRsaKeyPair
 * @brief Generate RSA public/private key pair
 * @param[in] nKeySizeInBits Size of the RSA Key
 * @throw BaseException on failed key generation
 *
 ********************************************************************************************/

void __thiscall CryptographicKey::GenerateRsaKeyPair(
    _in int nKeySizeInBits
    )
{
    __DebugFunction();

    std::unique_ptr<BIGNUM, decltype(&::BN_free)> poBignum(::BN_new(), BN_free);
    _ThrowIfNull(poBignum, "failed to create BIGNUM", nullptr);
    int nOpenSslStatus = ::BN_set_word(poBignum.get(), RSA_F4);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create BIGNUM for RSA key Pair", nullptr);

    std::unique_ptr<RSA, decltype(&::RSA_free)> poRSA(::RSA_new(), RSA_free);
    _ThrowIfNull(poRSA, "failed to create RSA object", nullptr);
    nOpenSslStatus = ::RSA_generate_key_ex(poRSA.get(), nKeySizeInBits, poBignum.get(), nullptr);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create RSA key Pair", nullptr);

    // This RSA key object is later referred in the EVP_PKEY object.
    // When the parent EVP_PKEY object is freed this is freed up that so this does not require it
    // to be freed explicitly.
    RSA * publicKey = ::RSAPublicKey_dup(poRSA.get());
    _ThrowIfNull(publicKey, "RSA Public Key Failed", nullptr);
    RSA * privateKey = ::RSAPrivateKey_dup(poRSA.get());
    _ThrowIfNull(privateKey, "RSA Private key failed", nullptr);

    m_poPrivateKey.reset(::EVP_PKEY_new(), ::EVP_PKEY_free);
    _ThrowIfNull(m_poPrivateKey, "Private key failed", nullptr);

    m_poPublicKey.reset(::EVP_PKEY_new(), ::EVP_PKEY_free);
    _ThrowIfNull(m_poPublicKey, "RSA Private key failed", nullptr);

    nOpenSslStatus = ::EVP_PKEY_assign_RSA(m_poPrivateKey.get(), privateKey);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create RSA Private Key", nullptr);
    nOpenSslStatus = ::EVP_PKEY_assign_RSA(m_poPublicKey.get(), publicKey);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create RSA Public Key", nullptr);
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GenerateEccKeyPair
 * @brief Generate ECC public/private key pair
 * @param[in] strCurveName Cure to use for the EC Key
 * @throw BaseException on failed key generation
 *
 ********************************************************************************************/

void __thiscall CryptographicKey::GenerateEccKeyPair(
    _in const std::string & strCurveName
    )
{
    __DebugFunction();

    int nEccGroup = ::OBJ_txt2nid(strCurveName.c_str());
    _ThrowBaseExceptionIf((NID_undef == nEccGroup), "Invalid ECC curve", nullptr);

    std::unique_ptr<EC_KEY, decltype(&::EC_KEY_free)> poEcKey(::EC_KEY_new_by_curve_name(nEccGroup), ::EC_KEY_free);
    _ThrowIfNull(poEcKey, "Failed to create EC key", nullptr);

    int nOpenSslStatus = ::EC_KEY_generate_key(poEcKey.get());
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create ECC key Pair", nullptr);

    // This EC key is later added to the EVP_PKEY and hence does not require to be freed here.
    // It will be freed with the EVP_PKEY_free.
    EC_KEY * poPublicKey = ::EC_KEY_new_by_curve_name(nEccGroup);
    _ThrowIfNull(poPublicKey, "Failed to create EC key", nullptr);
    EC_KEY * poPrivateKey = ::EC_KEY_new_by_curve_name(nEccGroup);
    _ThrowIfNull(poPublicKey, "Failed to create EC key", nullptr);

    // The EC_KEY_set_public_key will copy the key from the poEcKey, not refer it.
    // So poEcKey should be freed to prevent leak
    nOpenSslStatus = ::EC_KEY_set_public_key(poPublicKey, ::EC_KEY_get0_public_key(poEcKey.get()));
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create ECC key Pair", nullptr);

    // The EC private key consists of both the public and private parts of the key.
    nOpenSslStatus = ::EC_KEY_set_public_key(poPrivateKey, ::EC_KEY_get0_public_key(poEcKey.get()));
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create ECC key Pair", nullptr);
    nOpenSslStatus = ::EC_KEY_set_private_key(poPrivateKey, ::EC_KEY_get0_private_key(poEcKey.get()));
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create ECC key Pair", nullptr);

    m_poPrivateKey.reset(::EVP_PKEY_new(), ::EVP_PKEY_free);
    _ThrowIfNull(m_poPrivateKey, "Private key failed", nullptr);

    m_poPublicKey.reset(::EVP_PKEY_new(), ::EVP_PKEY_free);
    _ThrowIfNull(m_poPublicKey, "RSA Private key failed", nullptr);

    nOpenSslStatus = ::EVP_PKEY_assign_EC_KEY(m_poPrivateKey.get(), poPrivateKey);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Assigning EC Private Key to EVP_PKEY failed", nullptr);
    nOpenSslStatus = ::EVP_PKEY_assign_EC_KEY(m_poPublicKey.get(), poPublicKey);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Assigning EC Public Key to EVP_PKEY failed", nullptr);

    ::EC_KEY_free(poPublicKey);
    ::EC_KEY_free(poPrivateKey);
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GenerateAesKey
 * @brief Generate an AES key of the given size in bits
 * @param[in] nKeySizeInBits the size of the key in bits
 * @throw BaseException on invalid key or if key already exists
 *
 ********************************************************************************************/

void __thiscall CryptographicKey::GenerateAesKey(
    _in int nKeySizeInBits
    )
{
    __DebugFunction();

    m_stlSymmetricKey.resize(nKeySizeInBits / 8);
    int nOpensslStatus = ::RAND_bytes(m_stlSymmetricKey.data(), m_stlSymmetricKey.size());
    _ThrowBaseExceptionIf((1 != nOpensslStatus), "Failed to create AES Key", nullptr);
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function IsOperationSupported
 * @brief Check if the requested Cryptographic Operation can be performed by the key
 * @param[in] eOperation Cryptographic Operation to be checked
 * @return true if supported, else false
 *
 ********************************************************************************************/

bool __thiscall CryptographicKey::IsOperationSupported(
    _in CryptographicOperation eOperation
    )
{
    __DebugFunction();

    bool fResponseSupport = false;

    // For now this just performs the primary check so that wrong keys cannot be used for
    // operations they are not supposed to perform. Later versions will be more specific
    // on the purpose of each key which will be a subset of currently defined operations
    if ((KeySpec::eRSA2048 == m_KeySpec) || (KeySpec::eRSA3076 == m_KeySpec) || (KeySpec::eRSA4096 == m_KeySpec))
    {
        if ((CryptographicOperation::eDecrypt == eOperation) || (CryptographicOperation::eEncrypt == eOperation) || (CryptographicOperation::eDigitalSignature == eOperation) || (CryptographicOperation::eDigitalSignatureVerify == eOperation))
        {
            fResponseSupport = true;
        }
    }
    else if (KeySpec::eECC384 == m_KeySpec)
    {
        if ((CryptographicOperation::eDigitalSignature == eOperation) || (CryptographicOperation::eDigitalSignatureVerify == eOperation))
        {
            fResponseSupport = true;
        }
    }
    else if((KeySpec::eAES128 == m_KeySpec) || (KeySpec::eAES256 == m_KeySpec) || (KeySpec::ePDKDF2 == m_KeySpec))
    {
        if ((CryptographicOperation::eEncrypt == eOperation) || (CryptographicOperation::eDecrypt == eOperation))
        {
            fResponseSupport = true;
        }
    }

    return fResponseSupport;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetPublicKey
 * @brief Get the Public Key class memeber's shared pointer
 * @return Shared Pointed to the public key, nullptr on failure
 * @throw BaseException on failure
 * @note The lifetime of this EVP_PKEY is same as that of the object.
 *  In case it needs to be used beyond that, do a deep copy using
 ********************************************************************************************/

EVP_PKEY * __thiscall CryptographicKey::GetPublicKey()
{
    __DebugFunction();

    return m_poPublicKey.get();
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetPrivateKey
 * @brief Get the Private Key class memeber's shared pointer
 * @return Shared Pointed to the private key, nullptr on failure
 * @throw BaseException on failure
 *
 ********************************************************************************************/

EVP_PKEY * __thiscall CryptographicKey::GetPrivateKey()
{
    __DebugFunction();

    return m_poPrivateKey.get();
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetSymmetricKey
 * @brief Get the symmetric key class member of the object
 * @return Byte vector containing the key
 * @throw BaseException if no symmetric key exists
 *
 ********************************************************************************************/

std::vector<Byte> & __thiscall CryptographicKey::GetSymmetricKey()
{
    __DebugFunction();

    _ThrowBaseExceptionIf((0 >= m_stlSymmetricKey.size()), "No symmetric key set for this key object", nullptr);
    return m_stlSymmetricKey;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetKeySpec
 * @brief Get the type of Key
 * @return KeySpec enum type
 *
 ********************************************************************************************/

KeySpec __thiscall CryptographicKey::GetKeySpec()
{
    __DebugFunction();

    return m_KeySpec;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetKeyGuid
 * @brief Get the Guid of the Key
 * @return Guid object
 *
 ********************************************************************************************/

Guid __thiscall CryptographicKey::GetKeyGuid()
{
    __DebugFunction();

    return m_oKeyGuid;
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function StoreKey
 * @brief Write the key data to the File System
 *
 ********************************************************************************************/

// TODO: encrypt the key and then wite it to the filesystem
void __thiscall CryptographicKey::StoreKey()
{
    __DebugFunction();

    std::ofstream oKeyFile(m_oKeyGuid.ToString(eRaw)+".key", std::ios::binary);
    StructuredBuffer oStructuredBufferKey;
    oStructuredBufferKey.PutString("KeySpec", ::GetStringForKeySpec(m_KeySpec));

    if ((m_KeySpec == KeySpec::eAES128) || (m_KeySpec == KeySpec::eAES256) || (m_KeySpec == KeySpec::ePDKDF2))
    {
        oStructuredBufferKey.PutBuffer("Key", m_stlSymmetricKey);
    }
    else if ((m_KeySpec == KeySpec::eRSA2048) || (m_KeySpec == KeySpec::eRSA3076) || (m_KeySpec == KeySpec::eRSA4096) || (m_KeySpec == KeySpec::eECC384))
    {
        BioUniquePtr poBIO(::BIO_new(::BIO_s_mem()), ::BIO_free);
        _ThrowOutOfMemoryExceptionIfNull(poBIO);

        std::string strPemPrivateKey;
        if (1 == ::PEM_write_bio_PrivateKey(poBIO.get(), m_poPrivateKey.get(), nullptr, nullptr, 0, nullptr, nullptr))
        {
            char * pTempBufferLoc;
            int nBytesInBio = ::BIO_get_mem_data(poBIO.get(), &pTempBufferLoc);
            _ThrowBaseExceptionIf((0 >= nBytesInBio), "Nothing to read from BIO", nullptr);
            strPemPrivateKey.resize(nBytesInBio);
            int nBytesRead = ::BIO_read(poBIO.get(), &(strPemPrivateKey[0]), strPemPrivateKey.size());
            _ThrowBaseExceptionIf((nBytesInBio != nBytesRead), "Failed to read from BIO", nullptr);
        }
        std::string strPemPublicKey;
        if (1 == ::PEM_write_bio_PUBKEY(poBIO.get(), m_poPrivateKey.get()))
        {
            char * pTempBufferLoc;
            int nBytesInBio = ::BIO_get_mem_data(poBIO.get(), &pTempBufferLoc);
            _ThrowBaseExceptionIf((0 >= nBytesInBio), "Nothing to read from BIO", nullptr);
            strPemPublicKey.resize(nBytesInBio);
            int nBytesRead = ::BIO_read(poBIO.get(), &(strPemPublicKey[0]), strPemPublicKey.size());
            _ThrowBaseExceptionIf((nBytesInBio != nBytesRead), "Failed to read from BIO", nullptr);
        }
        oStructuredBufferKey.PutString("PublicKey", strPemPublicKey);
        oStructuredBufferKey.PutString("PrivateKey", strPemPrivateKey);
    }
    else
    {
        _ThrowBaseException("Invalid KeySpec in file", nullptr);
    }

    oKeyFile.write((const char *)oStructuredBufferKey.GetSerializedBufferRawDataPtr(), oStructuredBufferKey.GetSerializedBufferRawDataSizeInBytes());
    oKeyFile.close();
}

/********************************************************************************************
 *
 * @class CryptographicKey
 * @function GetPublicKeyPEM
 * @brief Get x509 public key certificate (PEM) of the public key used to verify this signature
 * @param[in] c_oKeyGuid Signature key
 * @return string representing the PEM used
 *
 ********************************************************************************************/

std::string __thiscall CryptographicKey::GetPublicKeyPEM(void)
{
    __DebugFunction();

    std::string strPemPublicKey;

    std::ifstream oKeyFile(m_oKeyGuid.ToString(eRaw)+".key", std::ios::binary | std::ios::ate);
    std::streamsize size = oKeyFile.tellg();
    _ThrowBaseExceptionIf((0 >= size), "No stored key found", nullptr);
    oKeyFile.seekg(0, std::ios::beg);

    std::vector<char> stlFileData(size);

    if (oKeyFile.read(stlFileData.data(), size))
    {
        StructuredBuffer oStructuredBufferKey((unsigned char *)stlFileData.data(), size);
        std::string strKeySpec = oStructuredBufferKey.GetString("KeySpec");
        KeySpec eKeySpec = ::GetKeySpecFromString(strKeySpec);

        if ((eKeySpec == KeySpec::eAES128) || (eKeySpec == KeySpec::eAES256) || (eKeySpec == KeySpec::ePDKDF2))
        {
            _ThrowBaseException("Invalid key spec used.", nullptr);
        }
        else if ((eKeySpec == KeySpec::eRSA2048) || (eKeySpec == KeySpec::eRSA3076) || (eKeySpec == KeySpec::eRSA4096) || (eKeySpec == KeySpec::eECC384))
        {
            strPemPublicKey = oStructuredBufferKey.GetString("PublicKey");
        }
        else
        {
            _ThrowBaseException("Invalid KeySpec in key file", nullptr);
        }
    }
    else
    {
        _ThrowBaseException("No stored key found.", nullptr);
    }

    return strPemPublicKey;
}
