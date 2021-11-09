/*********************************************************************************************
 *
 * @file CryptographicEngine.cpp
 * @author Prawal Gangwar
 * @date 11 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "Object.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "Base64Encoder.h"
#include "CryptographicEngine.h"
#include "HardCodedCryptographicKeys.h"

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/hmac.h>

#include <vector>
#include <iostream>
#include <string>
#include <optional>

// Singleton Object, can be declared anywhere, but only once
CryptographicEngine CryptographicEngine::m_oCryptographicEngine;

/********************************************************************************************
 *
 * @function CryptographicEngine
 * @brief Gets the singleton instance of the CryptographicEngine object
 *
 ********************************************************************************************/

CryptographicEngine & __thiscall CryptographicEngine::Get(void)
{
    __DebugFunction();

    return m_oCryptographicEngine;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function CryptographicEngine
 * @brief Constructor
 *
 ********************************************************************************************/

CryptographicEngine::CryptographicEngine(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function ~CryptographicEngine
 * @brief Destructor
 *
 ********************************************************************************************/

CryptographicEngine::~CryptographicEngine(void)
{
    __DebugFunction();

}

/********************************************************************************************
 * @class CryptographicEngine
 * @function OperationInit
 * @brief Initializes the cryptographic operation
 * @param[in] eOperation Cryptographic Operation to be performed
 * @param[in] c_oGuidOfkey Cryptographic Key Guid to be used for the operation
 * @param[in] c_oStructuredBufferRequest Parameters for cryptographic operation initialization
 * @return OperationID of the initailized operation, or nullptr if it fails
 * @throw BaseException on failure
 *
 ********************************************************************************************/

OperationID __thiscall CryptographicEngine::OperationInit(
    _in CryptographicOperation eOperation,
    _in const Guid & c_oGuidOfkey,
    _in const StructuredBuffer * c_poStructuredBufferRequest
    ) const
{
    __DebugFunction();

    CryptographicKeyUniquePtr poKey = std::make_unique<CryptographicKey>(c_oGuidOfkey);

    return this->OperationInit(eOperation, std::move(poKey), c_poStructuredBufferRequest);
}

/********************************************************************************************
 * @class CryptographicEngine
 * @function OperationInit
 * @brief Initializes the cryptographic operation
 * @param[in] eOperation Cryptographic Operation to be performed
 * @param[in] poKey Cryptographic Key unique_ptr to be used for operation
 * @param[in] c_oStructuredBufferRequest Parameters for cryptographic operation initialization
 * @return OperationID of the initailized operation, or nullptr if it fails
 * @throw BaseException on failure
 *
 ********************************************************************************************/

OperationID __thiscall CryptographicEngine::OperationInit(
    _in CryptographicOperation eOperation,
    _in CryptographicKeyUniquePtr poKey,
    _in const StructuredBuffer * c_poStructuredBufferRequest
    ) const
{
    __DebugFunction();

    OperationID poOperationID = nullptr;
    bool fOperationPossible = false;

    // Check if the key is allowed to perform the requested operation.
    // It is assumed that the poKey can only be null for hash operations
    if (nullptr != poKey)
    {
        fOperationPossible = poKey->IsOperationSupported(eOperation);
    }
    else if ((CryptographicOperation::eMD5Digest == eOperation) || (CryptographicOperation::eSHA1Digest == eOperation) || (CryptographicOperation::eSHA256Digest == eOperation) || (CryptographicOperation::eSHA384Digest == eOperation) || (CryptographicOperation::eSHA512Digest == eOperation))
    {
        fOperationPossible = true;
    }

    if (true == fOperationPossible)
    {
        switch (eOperation)
        {
        case CryptographicOperation::eEncrypt
        :   poOperationID = this->EncryptInit(std::move(poKey), c_poStructuredBufferRequest);
            break;
        case CryptographicOperation::eDecrypt
        :   poOperationID = this->DecryptInit(std::move(poKey), c_poStructuredBufferRequest);
            break;
        default
        :
            break;
        }
    }

    return poOperationID;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function OperationUpdate
 * @brief Update the operation or operate on the data, can be called multiple times
 * @param[in] poOperationParams ongoing operation identifier
 * @param[in] c_stlInputBuffer Input buffer
 * @param[in] stlOutputBuffer Output Buffer
 * @return true on success, false otherwise
 * @throw BaseException on failure
 *
 ********************************************************************************************/

void __thiscall CryptographicEngine::OperationUpdate(
    _in OperationID poOperationParams,
    _in const std::vector<Byte> & c_stlInputBuffer,
    _inout std::vector<Byte> & stlOutputBuffer
    ) const
{
    __DebugFunction();
    __DebugAssert(nullptr != poOperationParams);

    CryptographicOperation eOperation = poOperationParams->nCryptographicOperation;
    switch (eOperation)
    {
        case CryptographicOperation::eEncrypt
        :   this->EncryptUpdate(poOperationParams, c_stlInputBuffer, stlOutputBuffer);
            break;
        case CryptographicOperation::eDecrypt
        :   this->DecryptUpdate(poOperationParams, c_stlInputBuffer, stlOutputBuffer);
            break;
        default
        :
            break;
    }
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function OperationFinish
 * @brief Finish the operation, should be called once per operation
 * @param[in] poOperationParams ongoing operation identifier
 * @param[in] stlOutputBuffer The buffer with output data
 * @return True on success, false on failure
 * @throw BaseException on failure
 *
 ********************************************************************************************/

bool __thiscall CryptographicEngine::OperationFinish(
    _in OperationID poOperationParams,
    _inout std::vector<Byte> & stlOutputBuffer
    ) const
{
    __DebugFunction();
    __DebugAssert(nullptr != poOperationParams);

    bool fOperationSuccessStatus = false;

    CryptographicOperation eOperation = poOperationParams->nCryptographicOperation;
    switch (eOperation)
    {
        case CryptographicOperation::eEncrypt
        :   fOperationSuccessStatus = this->EncryptFinal(poOperationParams, stlOutputBuffer);
            break;
        case CryptographicOperation::eDecrypt
        :   fOperationSuccessStatus = this->DecryptFinal(poOperationParams, stlOutputBuffer);
            break;
        default
        :
            break;
    }

    return fOperationSuccessStatus;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function GenerateKey
 * @brief Generate a new Cryptographic Key
 * @param[in] eKeySpec Cryptographic Key specification
 * @return SharedPointer to the key object
 * @throw BaseException on failure
 *
 ********************************************************************************************/

Guid __thiscall CryptographicEngine::GenerateKey(
    _in KeySpec eKeySpec
    ) const
{
    __DebugFunction();

    CryptographicKey oCryptographicKey(eKeySpec);
    oCryptographicKey.StoreKey();

    return oCryptographicKey.GetKeyGuid();
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function DeriveKeyFromPassword
 * @brief Derive an AES256 key from a passphrase or password.
 * @param[in] eKeySpec Type of key to be derived from the password
 * @param[in] nKeySizeInBits Size in bits of the key
 * @param[in] eHashAlgorithm hash function to use for derivation
 * @param[in] c_szPassword Password for which key is derived
 * @param[in] c_stlSalt salt to get different keys for same password
 * @return SharedPointer to the key object
 * @throw BaseException on failure
 *
 ********************************************************************************************/

Guid __thiscall CryptographicEngine::DeriveAES256KeyFromPassword(
    _in KeySpec eKeySpec,
    _in HashAlgorithm eHashAlgorithm,
    _in const std::string & c_szPassword
    ) const
{
    __DebugFunction();

    CryptographicKey oCryptographicKey(eKeySpec, eHashAlgorithm, c_szPassword);
    oCryptographicKey.StoreKey();

    return oCryptographicKey.GetKeyGuid();
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function EncryptInit
 * @brief Initialize the encrypt operation and setup the parameters
 * @param[in] poKey Key to use for encryption
 * @param[in] oStructuredBufferRequest Parameters for cryptographic operation
 * @return OperationID of the new task initiated
 * @throw BaseException on failure
 * @note
 *      The StructuredBuffer for RSA encryption would look like
 *        +-----------------------------------------------------------------------------------+
 *        | ["KeyType":String] {Required} "PublicKey" or "PrivateKey" to use for encryption   |
 *        +-----------------------------------------------------------------------------------+
 *
 *       The StructuredBuffer for AES encryption should contain:
 *        +-----------------------------------------------------------------------------------+
 *        | ["IV":Buffer] {Required} Byte Buffer of 12 bytes for AES initialization           |
 *        +-----------------------------------------------------------------------------------+
 *        | ["AAD":Buffer] {Optional} Byte buffer for Addional Authenticated Data             |
 *        +-----------------------------------------------------------------------------------+
 *
 ********************************************************************************************/

OperationID __thiscall CryptographicEngine::EncryptInit(
    _in CryptographicKeyUniquePtr poKey,
    _in const StructuredBuffer * c_poStructuredBufferRequest
    ) const
{
    __DebugFunction();

    OperationID poOperationParams(new OperationParams());
    KeySpec eKeySpec = poKey->GetKeySpec();
    int nOpenSslStatus = 0;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        EVP_PKEY_CTX * poEvpPkeyCtx = nullptr;
        if ("PublicKey" == c_poStructuredBufferRequest->GetString("KeyType"))
        {
            poEvpPkeyCtx = ::EVP_PKEY_CTX_new(poKey->GetPublicKey(), nullptr);
        }
        else if ("PrivateKey" == c_poStructuredBufferRequest->GetString("KeyType"))
        {
            poEvpPkeyCtx = ::EVP_PKEY_CTX_new(poKey->GetPrivateKey(), nullptr);
        }
        _ThrowIfNull(poEvpPkeyCtx ,"Failed to create RSA encrypt context ",nullptr);

        nOpenSslStatus = ::EVP_PKEY_encrypt_init(poEvpPkeyCtx);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to initialize RSA encrypt operation", nullptr);

        // Using the recommended RSA_PKCS1_OAEP_PADDING padding
        nOpenSslStatus = ::EVP_PKEY_CTX_set_rsa_padding(poEvpPkeyCtx, RSA_PKCS1_OAEP_PADDING);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to set RSA padding scheme", nullptr);

        poOperationParams->nCryptographicOperation = CryptographicOperation::eEncrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpPkeyCtx = poEvpPkeyCtx;
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec))
    {
        const EVP_CIPHER * poEvpCipher = ::GetEVP_CIPHERForAesKey(eKeySpec);

        EVP_CIPHER_CTX * poEvpCipherCtx = ::EVP_CIPHER_CTX_new();
        _ThrowIfNull(poEvpCipherCtx, "EVP_CIPHER_CTX_new() failed.", nullptr);

        //  Initialise the encryption operation.
        nOpenSslStatus = ::EVP_EncryptInit_ex(poEvpCipherCtx, poEvpCipher, nullptr, nullptr, nullptr);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to Initialize AES ", nullptr);

        // Set IV length, using the recommended 12 bytes
        nOpenSslStatus = ::EVP_CIPHER_CTX_ctrl(poEvpCipherCtx, EVP_CTRL_AEAD_SET_IVLEN, AES_GCM_IV_LENGTH, nullptr);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to set the IV length for encryption", nullptr);

        // Initialise key and IV
        nOpenSslStatus = ::EVP_EncryptInit_ex(poEvpCipherCtx, nullptr, nullptr, poKey->GetSymmetricKey().data(), c_poStructuredBufferRequest->GetBuffer("IV").data());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create Initialize key and IV", nullptr);

        // Additional Authenticated Data to be sent but not encrypted can be authenticated if passes to the structured buffer
        if (c_poStructuredBufferRequest->IsElementPresent("AAD", BUFFER_VALUE_TYPE))
        {
            std::vector<Byte> stlAdditionalAuthenticatedData = c_poStructuredBufferRequest->GetBuffer("AAD");
            int nCipherTextLength = 0;
            nOpenSslStatus = ::EVP_EncryptUpdate(poEvpCipherCtx, nullptr, &nCipherTextLength, stlAdditionalAuthenticatedData.data(), stlAdditionalAuthenticatedData.size());
            _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);
        }

        poOperationParams->nCryptographicOperation = CryptographicOperation::eEncrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpCipherCtx = poEvpCipherCtx;
    }
    else if ((KeySpec::ePDKDF2 == eKeySpec))
    {
        const EVP_CIPHER * poEvpCipher = ::GetEVP_CIPHERForAesKey(eKeySpec);

        EVP_CIPHER_CTX * poEvpCipherCtx = ::EVP_CIPHER_CTX_new();
        _ThrowIfNull(poEvpCipherCtx, "EVP_CIPHER_CTX_new() failed.", nullptr);

        //  Initialise the encryption operation.
        nOpenSslStatus = ::EVP_EncryptInit_ex(poEvpCipherCtx, poEvpCipher, nullptr, poKey->GetSymmetricKey().data(), (poKey->GetSymmetricKey().data() + poKey->GetSymmetricKey().size() - AES_CFB_IV_LENGTH));
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create Initialize key and IV", nullptr);

        poOperationParams->nCryptographicOperation = CryptographicOperation::eEncrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpCipherCtx = poEvpCipherCtx;
    }

    return poOperationParams;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function EncryptUpdate
 * @brief Initialize the encrypt operation and setup the parameters
 * @param[in] poOperationParams Parameters for cryptographic operation
 * @param[in] stlPlainText Plain text to be encrypted
 * @param[in] stlCipherText Output Cipher Text
 * @throw BaseException on failure
 * @note For symmetric encryption the output buffer is resized everytime, it is just much more efficient to
 * pass as much data as possible in a single go.
 *
 ********************************************************************************************/

void __thiscall CryptographicEngine::EncryptUpdate(
    _in OperationID poOperationParams,
    _in const std::vector<Byte> & c_stlPlainText,
    _in std::vector<Byte> & stlCipherText
    ) const
{
    __DebugFunction();

    KeySpec eKeySpec = poOperationParams->eKeySpec;
    int nOpenSslStatus = 0;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        size_t nCipherTextLength;
        nOpenSslStatus = ::EVP_PKEY_encrypt(poOperationParams->pEvpPkeyCtx , nullptr, &nCipherTextLength, c_stlPlainText.data(), c_stlPlainText.size());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to encrypt the plaintext", nullptr);
        stlCipherText.resize(nCipherTextLength);
        nOpenSslStatus = ::EVP_PKEY_encrypt(poOperationParams->pEvpPkeyCtx, stlCipherText.data(), &nCipherTextLength, c_stlPlainText.data(), c_stlPlainText.size());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to encrypt the plaintext", nullptr);
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec) || (KeySpec::ePDKDF2 == eKeySpec))
    {
        int nCipherTextLength = 0;
        // The Cipher text length in case of AES_GCM is same as that of Plain Text. And since
        // Update can be called multiple times the size of the CipherText vector is
        // increased by that amount.
        size_t unOriginalCipherLength = stlCipherText.size();
        stlCipherText.resize(stlCipherText.size() + c_stlPlainText.size());
        // the output buffer is resized everytime, it is just much more efficient to
        // pass as much data as possible in a single go for symmetric encryption.
        nOpenSslStatus = ::EVP_EncryptUpdate(poOperationParams->pEvpCipherCtx, stlCipherText.data() + unOriginalCipherLength, &nCipherTextLength, c_stlPlainText.data(), c_stlPlainText.size());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);
    }
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function EncryptFinal
 * @brief Finalize the encrypt operation and perform cleanup
 * @param[in] poOperationParams Parameters for cryptographic operation
 * @param[in] stlCipherText Final Cipher Text with TAG(in case of AES_GCM)
 * @return true if successful, otherwise false
 * @throw BaseException on failure
 *
 ********************************************************************************************/

bool __thiscall CryptographicEngine::EncryptFinal(
    _in OperationID poOperationParams,
    _inout std::vector<Byte> & stlCipherText
    ) const
{
    __DebugFunction();

    KeySpec eKeySpec = poOperationParams->eKeySpec;
    int nOpenSslStatus = 0;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        ::EVP_PKEY_CTX_free(poOperationParams->pEvpPkeyCtx);
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec))
    {
        size_t unOriginalCipherLength = stlCipherText.size();
        int nCipherTextLength;

        // Adding additional 16 bytes for the tag.
        stlCipherText.resize(stlCipherText.size() + AES_TAG_LENGTH);

        // Most likely this will not anything to the ciphertext buffer. It internally finalizes the encryption and maybe some cleanup.
        nOpenSslStatus = ::EVP_EncryptFinal_ex(poOperationParams->pEvpCipherCtx, stlCipherText.data() + unOriginalCipherLength, &nCipherTextLength);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);

        // Add the generated TAG at the end of the cipher
        nOpenSslStatus = ::EVP_CIPHER_CTX_ctrl(poOperationParams->pEvpCipherCtx, EVP_CTRL_AEAD_GET_TAG, AES_TAG_LENGTH, stlCipherText.data() + unOriginalCipherLength + nCipherTextLength);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);

        // Free the Cipher context
        // TODO: use unique pointer for this
        ::EVP_CIPHER_CTX_free(poOperationParams->pEvpCipherCtx);
    }
    else if ((KeySpec::ePDKDF2 == eKeySpec))
    {
        size_t unOriginalCipherLength = stlCipherText.size();
        int nCipherTextLength;

        // Most likely this will not anything to the ciphertext buffer. It internally finalizes the encryption and maybe some cleanup.
        nOpenSslStatus = ::EVP_EncryptFinal_ex(poOperationParams->pEvpCipherCtx, stlCipherText.data() + unOriginalCipherLength, &nCipherTextLength);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);

        // Free the Cipher context
        // TODO: use unique pointer for this
        ::EVP_CIPHER_CTX_free(poOperationParams->pEvpCipherCtx);
    }

    return true;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function DecryptInit
 * @brief Initialize the decrypt operation and setup the parameters
 * @param[in] poKey Key to use for decryption
 * @param[in] oStructuredBufferRequest Parameters for cryptographic operation
 * @return OperationID of the new decryption task
 * @throw BaseException on failure
 * @note
 *      The StructuredBuffer for RSA decryption would look like
 *        +-----------------------------------------------------------------------------------+
 *        | ["KeyType":String] {Required} "PublicKey" or "PrivateKey" to use for encryption   |
 *        +-----------------------------------------------------------------------------------+
 *
 *       The StructuredBuffer for AES encryption should contain:
 *        +-----------------------------------------------------------------------------------+
 *        | ["IV":Buffer] {Required} Byte Buffer of 12 bytes which was used for encryption    |
 *        +-----------------------------------------------------------------------------------+
 *        | ["AAD":Buffer] {Optional} Byte buffer for Addional Authenticated Data             |
 *        +-----------------------------------------------------------------------------------+
 *        | ["TAG":Buffer] {Required} 16 byte TAG buffer for authentication of decrypted data |
 *        |                           These are the last 16 bytes of the cipher text          |
 *        +-----------------------------------------------------------------------------------+
 *
 ********************************************************************************************/

OperationID __thiscall CryptographicEngine::DecryptInit(
    _in CryptographicKeyUniquePtr poKey,
    _in const StructuredBuffer * c_poStructuredBufferRequest
    ) const
{
    __DebugFunction();

    OperationID poOperationParams(new OperationParams());
    KeySpec eKeySpec = poKey->GetKeySpec();
    int nOpenSslStatus = 0;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        EVP_PKEY_CTX * poEvpPkeyCtx;
        if ("PublicKey" == c_poStructuredBufferRequest->GetString("KeyType"))
        {
            poEvpPkeyCtx = ::EVP_PKEY_CTX_new(poKey->GetPublicKey(), nullptr);
        }
        else if ("PrivateKey" == c_poStructuredBufferRequest->GetString("KeyType"))
        {
            poEvpPkeyCtx = ::EVP_PKEY_CTX_new(poKey->GetPrivateKey(), nullptr);
        }
        _ThrowIfNull(poEvpPkeyCtx ,"Failed to create RSA decrypt context ", nullptr);

        nOpenSslStatus = ::EVP_PKEY_decrypt_init(poEvpPkeyCtx);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to initialize RSA decrypt operation", nullptr);

        nOpenSslStatus = ::EVP_PKEY_CTX_set_rsa_padding(poEvpPkeyCtx, RSA_PKCS1_OAEP_PADDING);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to set RSA padding scheme", nullptr);

        poOperationParams->nCryptographicOperation = CryptographicOperation::eDecrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpPkeyCtx = poEvpPkeyCtx;
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec))
    {
        const EVP_CIPHER * poEvpCipher = ::GetEVP_CIPHERForAesKey(eKeySpec);

        EVP_CIPHER_CTX * poEvpCipherCtx = ::EVP_CIPHER_CTX_new();
        _ThrowIfNull(poEvpCipherCtx, "EVP_CIPHER_CTX_new() failed.", nullptr);

        //  Initialise the decryption operation.
        nOpenSslStatus = ::EVP_DecryptInit_ex(poEvpCipherCtx, poEvpCipher, nullptr, nullptr, nullptr);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to Initialize AES ", nullptr);

        // Set IV length to 12, as recommended
        nOpenSslStatus = ::EVP_CIPHER_CTX_ctrl(poEvpCipherCtx, EVP_CTRL_AEAD_SET_IVLEN, AES_GCM_IV_LENGTH, nullptr);
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to set the IV length for decryption", nullptr);

        // Initialise key and IV
        nOpenSslStatus = ::EVP_DecryptInit_ex(poEvpCipherCtx, nullptr, nullptr, poKey->GetSymmetricKey().data(), c_poStructuredBufferRequest->GetBuffer("IV").data());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create Initialize key and IV", nullptr);

        // Additional Authenticated Data to be sent but not encrypted can be authenticated if passes to the structured buffer
        if (c_poStructuredBufferRequest->IsElementPresent("AAD", BUFFER_VALUE_TYPE))
        {
            std::vector<Byte> stlAdditionalAuthenticatedData = c_poStructuredBufferRequest->GetBuffer("AAD");
            int nCipherTextLength = 0;
            nOpenSslStatus = ::EVP_DecryptUpdate(poEvpCipherCtx, nullptr, &nCipherTextLength, stlAdditionalAuthenticatedData.data(), stlAdditionalAuthenticatedData.size());
            _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Encryption failed", nullptr);
        }

        // Set the TAG right here which will be used to test if the decrypted data can be trusted or not
        // These are the last 16 bytes of the cipher text
        nOpenSslStatus = ::EVP_CIPHER_CTX_ctrl(poEvpCipherCtx, EVP_CTRL_AEAD_SET_TAG, AES_TAG_LENGTH, c_poStructuredBufferRequest->GetBuffer("TAG").data());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Setting up TAG failed", nullptr);

        poOperationParams->nCryptographicOperation = CryptographicOperation::eDecrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpCipherCtx = poEvpCipherCtx;
    }
    else if ((KeySpec::ePDKDF2 == eKeySpec))
    {
        const EVP_CIPHER * poEvpCipher = ::GetEVP_CIPHERForAesKey(eKeySpec);

        EVP_CIPHER_CTX * poEvpCipherCtx = ::EVP_CIPHER_CTX_new();
        _ThrowIfNull(poEvpCipherCtx, "EVP_CIPHER_CTX_new() failed.", nullptr);

        //  Initialise the decryption operation.
        nOpenSslStatus = ::EVP_DecryptInit_ex(poEvpCipherCtx, poEvpCipher, nullptr, poKey->GetSymmetricKey().data(), (poKey->GetSymmetricKey().data() + poKey->GetSymmetricKey().size() - AES_CFB_IV_LENGTH));
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Failed to create Initialize key and IV", nullptr);

        poOperationParams->nCryptographicOperation = CryptographicOperation::eDecrypt;
        poOperationParams->poKey = std::move(poKey);
        poOperationParams->eKeySpec = eKeySpec;
        poOperationParams->pEvpCipherCtx = poEvpCipherCtx;
    }

    return poOperationParams;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function DecryptUpdate
 * @brief Initialize the decrypt operation and setup the parameters
 * @param[in] poOperationParams Parameters for cryptographic operation
 * @param[in] stlCipherText Input Cipher Text
 * @param[in] stlPlainText Output plain text
 * @throw BaseException on failure
 * @note For symmetric decryption the output buffer is resized everytime update is called,
 * it is just much more efficient to pass as much data as possible in a single go. The encrypted
 * buffer has 16 bytes attached to the end as tag which must be removed when passed as input.
 * A good to do it would be to first copy the tag into the StructuredBuffer and then resize the
 * encrypted buffer to (stlCipherText.size() - AES_TAG_LENGTH).
 *
 ********************************************************************************************/

void __thiscall CryptographicEngine::DecryptUpdate(
    _in OperationID poOperationParams,
    _in const std::vector<Byte> & c_stlCipherText,
    _inout std::vector<Byte> & stlPlainText
    ) const
{
    __DebugFunction();

    KeySpec eKeySpec = poOperationParams->eKeySpec;
    int nOpenSslStatus = 0;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        size_t nPlainTextLength;
        int decryptStatus = ::EVP_PKEY_decrypt(poOperationParams->pEvpPkeyCtx, NULL, &nPlainTextLength, c_stlCipherText.data(), c_stlCipherText.size());
        _ThrowBaseExceptionIf((1 != decryptStatus),"Failed to decrypt the cipher",nullptr);
        stlPlainText.resize(nPlainTextLength);
        decryptStatus = ::EVP_PKEY_decrypt(poOperationParams->pEvpPkeyCtx, stlPlainText.data(), &nPlainTextLength, c_stlCipherText.data(), c_stlCipherText.size());
        _ThrowBaseExceptionIf((1 != decryptStatus),"Failed to decrypt the cipher",nullptr);
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec) || (KeySpec::ePDKDF2 == eKeySpec))
    {
        int nPlainTextLength;
        // The Cipher text length in case of AES_GCM is same as that of Plain Text. And since this Update can be called multiple times
        // the size of the CipherText vector is increased by that amount.
        size_t unOriginalPlainLength = stlPlainText.size();
        stlPlainText.resize(stlPlainText.size() + c_stlCipherText.size());
        // the output buffer is resized everytime, it is just much more efficient to
        // pass as much data as possible in a single go for symmetric encryption.
        nOpenSslStatus = ::EVP_DecryptUpdate(poOperationParams->pEvpCipherCtx, stlPlainText.data() + unOriginalPlainLength, &nPlainTextLength, c_stlCipherText.data(), c_stlCipherText.size());
        _ThrowBaseExceptionIf((1 != nOpenSslStatus), "AES Decryption failed", nullptr);
    }
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function DecryptFinal
 * @brief Finalize the decrypt operation and perform cleanup
 * @param[in] poOperationParams Parameter ID for cryptographic operation
 * @param[in] stlCipherText Final Plain Text
 * @return true if successful, otherwise false.
 * @throw BaseException on failure
 * @note In case false is returned from this function, it would imply that the Decryption or
 *      the authentication of decrypted data failed. Such data or AAD should not be trusted.
 ********************************************************************************************/

bool __thiscall CryptographicEngine::DecryptFinal(
    _in OperationID poOperationParams,
    _inout std::vector<Byte> & stlPlainText
    ) const
{
    __DebugFunction();

    KeySpec eKeySpec = poOperationParams->eKeySpec;
    int nOpenSslStatus = 0;
    int fDecryptSuccessResponse = false;

    if ((KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        EVP_PKEY_CTX_free(poOperationParams->pEvpPkeyCtx);
        fDecryptSuccessResponse = true;
    }
    else if ((KeySpec::eAES256 == eKeySpec) || (KeySpec::eAES128 == eKeySpec) || (KeySpec::ePDKDF2 == eKeySpec))
    {
        size_t unOriginalCipherLength = stlPlainText.size();
        int nCipherTextLength;

        nOpenSslStatus = ::EVP_DecryptFinal_ex(poOperationParams->pEvpCipherCtx, stlPlainText.data() + unOriginalCipherLength, &nCipherTextLength);
        if (0 < nOpenSslStatus)
        {
            fDecryptSuccessResponse = true;
        }
        ::EVP_CIPHER_CTX_free(poOperationParams->pEvpCipherCtx);
    }

    return fDecryptSuccessResponse;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function GenerateSignature
 * @brief Generate the digital signature of the message digest
 * @param[in] poKey Signature key
 * @param[in] c_stlMessageDigest Message digest to sign
 * @return Signed Message
 * @throw BaseException on failure
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicEngine::GenerateSignature(
    _in const Guid & c_oGuidOfkey,
    _in const std::vector<Byte> & c_stlMessageDigest
) const
{
    __DebugFunction();

    std::vector<Byte> stlSignature;

    CryptographicKey oKey(c_oGuidOfkey);
    KeySpec eKeySpec = oKey.GetKeySpec();

    int nOpenSslStatus = 0;

    if ((KeySpec::eECC384 == eKeySpec) || (KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        if (nullptr != oKey.GetPrivateKey())
        {
            EVP_PKEY_CTX * poEvpPkeyCtx = ::EVP_PKEY_CTX_new(oKey.GetPrivateKey(), nullptr);
            _ThrowIfNull(poEvpPkeyCtx, "Fail to Initialize Key Context", nullptr);

            nOpenSslStatus = ::EVP_PKEY_sign_init(poEvpPkeyCtx);
            _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Signature Init failed.", nullptr);

            size_t nSignatureLength;
            int nOpenSslStatus = ::EVP_PKEY_sign(poEvpPkeyCtx, nullptr, &nSignatureLength, c_stlMessageDigest.data(), c_stlMessageDigest.size());
            _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Getting signature max size failed.", nullptr);

            stlSignature.resize(nSignatureLength);
            nOpenSslStatus = ::EVP_PKEY_sign(poEvpPkeyCtx, stlSignature.data(), &nSignatureLength, c_stlMessageDigest.data(), c_stlMessageDigest.size());
            _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Digest Signing failed.", nullptr);

            // The first call to EVP_PKEY_sign gives the max size of the hash but not the exact size.
            // The second EVP_PKEY_sign returns the actual size of the signature in nSignatureLength
            // If not resized properly, it will result in a corrupted signature.
            stlSignature.resize(nSignatureLength);

            // TODO: use a smart pointer to make it thread safe
            ::EVP_PKEY_CTX_free(poEvpPkeyCtx);
        }
    }

    return stlSignature;
}


/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function VerifySignature
 * @brief Verify the digital signature of the message digest
 * @param[in] poKey Signature verification key
 * @param[in] c_stlSignature Signature to verify
 * @param[in] c_stlMessageDigest Message digest to be verified
 * @return True on Successful verification
 * @throw BaseException on failure
 *
 ********************************************************************************************/

bool __thiscall CryptographicEngine::VerifySignature(
    _in const Guid & c_oGuidOfkey,
    _in const std::vector<Byte> & c_stlSignature,
    _in const std::vector<Byte> & c_stlMessageDigest
) const
{
    __DebugFunction();

    bool fSignatureVerifyStatus = false;

    CryptographicKey oKey(c_oGuidOfkey);
    KeySpec eKeySpec = oKey.GetKeySpec();

    if ((KeySpec::eECC384 == eKeySpec) || (KeySpec::eRSA2048 == eKeySpec) || (KeySpec::eRSA3076 == eKeySpec) || (KeySpec::eRSA4096 == eKeySpec))
    {
        if (nullptr != oKey.GetPublicKey())
        {
            EVP_PKEY_CTX * poEvpPkeyCtx = ::EVP_PKEY_CTX_new(oKey.GetPublicKey(), nullptr);
            _ThrowIfNull(poEvpPkeyCtx, "Cannot create EVP_PKEY context for signature verification", nullptr);

            int nOpenSslStatus = ::EVP_PKEY_verify_init(poEvpPkeyCtx);
            _ThrowBaseExceptionIf((1 != nOpenSslStatus),"Signature verification failed", nullptr);

            size_t nSignatureLength = 0;
            nOpenSslStatus = ::EVP_PKEY_verify(poEvpPkeyCtx, c_stlSignature.data(), c_stlSignature.size(), c_stlMessageDigest.data(), c_stlMessageDigest.size());

            if (1 == nOpenSslStatus)
            {
                fSignatureVerifyStatus = true;
            }
            ::EVP_PKEY_CTX_free(poEvpPkeyCtx);
        }
    }

    return fSignatureVerifyStatus;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function GenerateDigitalSignature
 * @brief Generate the digital signature of the message digest. 
 * @param[in] poKey Signature key
 * @param[in] c_stlMessageDigest Message digest to sign
 * @return Signed Message
 * @throw BaseException on failure
 * @note
 *      This function uses EVP_DigestSignInit instead of EVP_PKEY_sign. EVP_PKEY_sign does not
 *      hash data to be signed and EVP_DigestSignInit is used to sign arbitrary messages.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall CryptographicEngine::GenerateDigitalSignature(
    _in const Guid & c_oGuidOfkey,
    _in const std::vector<Byte> & c_stlMessageDigest
) const
{
    __DebugFunction();

    std::vector<Byte> stlSignature;

    CryptographicKey oKey(c_oGuidOfkey);
    if (nullptr != oKey.GetPrivateKey())
    {
        EVP_MD_CTX * poEvpMdCtx = ::EVP_MD_CTX_create();
        _ThrowIfNull(poEvpMdCtx, "Fail to Initialize Key Context", nullptr);

        int nStatus = ::EVP_DigestSignInit(poEvpMdCtx, NULL, EVP_sha256(), NULL, oKey.GetPrivateKey());
        _ThrowBaseExceptionIf((1 != nStatus), "Signature Init failed.", nullptr);

        size_t nSignatureLength;
        std::string strMessage = std::string(c_stlMessageDigest.begin(), c_stlMessageDigest.end());
        nStatus = ::EVP_DigestSignUpdate(poEvpMdCtx, c_stlMessageDigest.data(), c_stlMessageDigest.size());
        _ThrowBaseExceptionIf((1 != nStatus), "Updating with message failed.", nullptr);

        nStatus = ::EVP_DigestSignFinal(poEvpMdCtx, NULL, &nSignatureLength);
        _ThrowBaseExceptionIf((1 != nStatus), "Getting signature length failed.", nullptr);

        stlSignature.resize(nSignatureLength);
        nStatus = ::EVP_DigestSignFinal(poEvpMdCtx, stlSignature.data(), &nSignatureLength);
        _ThrowBaseExceptionIf((1 != nStatus), "Digest Signing failed.", nullptr);

        ::EVP_MD_CTX_destroy(poEvpMdCtx);
    }

    return stlSignature;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function GetKeySpecification
 * @brief Get KeySpec associated with the key guid
 * @param[in] c_oKeyGuid Signature key
 * @return KeySpec used
 *
 ********************************************************************************************/

KeySpec __thiscall CryptographicEngine::GetKeySpecification(
    _in const Guid & c_oKeyGuid
    )
{
    __DebugFunction();

    CryptographicKey oKey(c_oKeyGuid);
    KeySpec eKeySpec = oKey.GetKeySpec();

    return eKeySpec;
}

/********************************************************************************************
 *
 * @class CryptographicEngine
 * @function GetPublicKeyPEM
 * @brief Get x509 public key certificate (PEM) of the public key used to verify this signature
 * @param[in] c_oKeyGuid Signature key
 * @return string representing the PEM used
 *
 ********************************************************************************************/

std::string __thiscall CryptographicEngine::GetPublicKeyPEM(
    _in const Guid & c_oKeyGuid
    )
{
    __DebugFunction();

    CryptographicKey oKey(c_oKeyGuid);
    std::string strPemPublicKey = oKey.GetPublicKeyPEM();

    return strPemPublicKey;
}
