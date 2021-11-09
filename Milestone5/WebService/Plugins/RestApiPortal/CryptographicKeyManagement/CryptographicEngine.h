/*********************************************************************************************
 *
 * @file CryptographicEngine.h
 * @author Prawal Gangwar
 * @date 11 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/
#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"
#include "Guid.h"

#include <openssl/evp.h>
#include <openssl/x509.h>

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <optional>
#include <variant>

/********************************************************************************************/
// TODO: Add more curves for ECC, will add on need basis
enum class KeySpec
{
    eInvalidKey,
    eRSA2048,
    eRSA3076,
    eRSA4096,
    eECC384,
    eAES256,
    eAES128,
    ePDKDF2
};

enum class HashAlgorithm : uint32_t
{
    eInvalidHashAlgorithm = 0,
    eSHA1 = 1 << 0,
    eSHA256 = 1 << 1,
    eSHA384 = 1 << 2,
    eMD5 = 1 << 3
};

enum class CryptographicOperation : uint32_t
{
    eInvalidOperation = 0,
    eEncrypt = 1 << 0,
    eDecrypt = 1 << 1,
    eDigitalSignature = 1 << 2,
    eDigitalSignatureVerify = 1 << 3,
    eSHA1Digest = 1 << 4,
    eSHA256Digest = 1 << 5,
    eSHA384Digest = 1 << 6,
    eSHA512Digest = 1 << 7,
    eMD5Digest = 1 << 8,
    eHmac = 1 << 9
};

#define AES_GCM_IV_LENGTH 12
#define AES_CFB_IV_LENGTH 16
#define AES_TAG_LENGTH 16

/********************************************************************************************/

std::vector<Byte> __thiscall GenerateRandomBytes(
    _in unsigned int unSizeInBytes
    );
const EVP_MD * __thiscall GetEVP_MDForHashAlgorithm(
    _in HashAlgorithm eHashAlgorithm
    );
const EVP_CIPHER * __thiscall GetEVP_CIPHERForAesKey(
    _in KeySpec eKeySpec
    );
KeySpec __thiscall GetKeySpecFromString(
    _in const std::string & strKeyType
    );
std::string __thiscall GetStringForKeySpec(
    _in KeySpec eKeySpec
    );
HashAlgorithm __thiscall GetHashAlgorithmFromString(
    _in const std::string & strHashAlgorithm
    );

/********************************************************************************************/

using BioUniquePtr = std::unique_ptr<BIO, decltype(&::BIO_free)>;
using EvpKeyUniquePtr = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
using EvpKeySharedPtr = std::shared_ptr<EVP_PKEY>;

/********************************************************************************************/

class CryptographicKey : public Object
{
    public:

        CryptographicKey(void);
        CryptographicKey(
            _in Guid oGuid
            );
        CryptographicKey(
            _in const KeySpec
            );
        CryptographicKey(
            _in KeySpec eKeySpec,
            _in HashAlgorithm eHashAlgorithm,
            _in const std::string & c_szPassword
            );
        CryptographicKey(
            _in const CryptographicKey & c_oCryptographicKey
            );
        ~CryptographicKey(void);

        bool __thiscall IsOperationSupported(
            _in CryptographicOperation eOperation
            );

        void __thiscall StoreKey(void);
        Guid __thiscall GetKeyGuid(void);
        // While using this, keep in mind that the lifetime of this key is same as the
        // Cryptographic Object. If needed to be really copied, this should be deep copied
        // to a different EVP_PKEY struct during the life of this object
        EVP_PKEY * __thiscall GetPublicKey();
        EVP_PKEY * __thiscall GetPrivateKey();
        std::vector<Byte> & __thiscall GetSymmetricKey();
        KeySpec __thiscall GetKeySpec();
        std::string __thiscall GetPublicKeyPEM(void );

    private:

        KeySpec m_KeySpec = KeySpec::eInvalidKey;
        // The keys are declared as shared pointers because of the copy constructor
        // where it is just a shallow copy of the key. This EVP_PKEY object will stay
        // alive as long as any of the copy of this Cryptographic Object is.
        EvpKeySharedPtr m_poPrivateKey;
        EvpKeySharedPtr m_poPublicKey;
        Guid m_oKeyGuid;
        std::vector<Byte> m_stlSymmetricKey;

        void __thiscall GenerateRsaKeyPair(
            _in int keySize
            );
        void __thiscall GenerateEccKeyPair(
            _in const std::string & strCurveName
            );
        void __thiscall GenerateAesKey(
            _in int keySize
            );
};
using CryptographicKeySharedPtr = std::shared_ptr<CryptographicKey>;
using CryptographicKeyUniquePtr = std::unique_ptr<CryptographicKey>;

/********************************************************************************************/

typedef struct
{
    // TODO: change this to variant or union of unique pointers
    EVP_CIPHER_CTX * pEvpCipherCtx = nullptr;
    EVP_PKEY_CTX * pEvpPkeyCtx = nullptr;
    EVP_MD_CTX * pEvpMdCtx = nullptr;
    HMAC_CTX * pHmacCtx = nullptr;
    CryptographicOperation nCryptographicOperation = CryptographicOperation::eInvalidOperation;
    // The key is not needed in the Update and Final functions but is still stored in the
    // struct to manipulate the lifetime of the key object. It just prevents the deletion of
    // the key object until the end of the operation.
    CryptographicKeyUniquePtr poKey = nullptr;
    KeySpec eKeySpec = KeySpec::eInvalidKey;
    bool fCarryOverData = false;
} OperationParams;

using OperationID = std::shared_ptr<OperationParams> ;

/********************************************************************************************/

class CryptographicEngine : public Object
{
    public:

        ~CryptographicEngine(void);
        // Since only one object is supposed to exist
        // we make the constructors private and delete the copy constructor
        CryptographicEngine(
            _in const CryptographicEngine & c_oCryptographicEngine
            ) = delete;
        // The static function for the class that can get the reference to the
        // CryptographicEngine singleton object
        static CryptographicEngine & Get(void);

        OperationID __thiscall OperationInit(
            _in CryptographicOperation nOperation,
            _in const Guid & c_oGuidOfkey,
            _in const StructuredBuffer * c_poStructuredBufferRequest = nullptr
            ) const;
        OperationID __thiscall OperationInit(
            _in CryptographicOperation nOperation,
            _in CryptographicKeyUniquePtr poKey,
            _in const StructuredBuffer * c_poStructuredBufferRequest = nullptr
            ) const;
        void __thiscall OperationUpdate(
            _in OperationID operationId,
            _in const std::vector<Byte> & c_stlInputBuffer,
            _inout std::vector<Byte> & stlOutputBuffer
            ) const;
        bool __thiscall OperationFinish(
            _in OperationID operationId,
            _inout std::vector<Byte> & stlOutputBuffer
            ) const;
        Guid __thiscall GenerateKey(
            _in KeySpec eKeySpec
            ) const;
        Guid __thiscall DeriveAES256KeyFromPassword(
            _in KeySpec eKeySpec,
            _in HashAlgorithm eHashAlgorithm,
            _in const std::string & c_szPassword
            ) const;
        std::vector<Byte> __thiscall GenerateSignature(
            _in const Guid & c_oGuidOfkey,
            _in const std::vector<Byte> & c_stlMessageDigest
            ) const;
        bool __thiscall VerifySignature(
            _in const Guid & c_oGuidOfkey,
            _in const std::vector<Byte> & c_stlSignature,
            _in const std::vector<Byte> & c_stlMessageDigest
            ) const;
        std::vector<Byte> __thiscall GenerateDigitalSignature(
            _in const Guid & c_oGuidOfkey,
            _in const std::vector<Byte> & c_stlMessageDigest
            ) const;
        KeySpec __thiscall GetKeySpecification(
            _in const Guid & c_oKeyGuid
            );
        std::string __thiscall GetPublicKeyPEM(
            _in const Guid & c_oKeyGuid
            );

    private:

        CryptographicEngine(void);
        static CryptographicEngine m_oCryptographicEngine;

        OperationID __thiscall EncryptInit(
            _in CryptographicKeyUniquePtr poKey,
            _in const StructuredBuffer * c_poStructuredBufferRequest
            ) const;
        void __thiscall EncryptUpdate(
            _in OperationID poOperationParams,
            _in const std::vector<Byte> & c_stlPlainText,
            _inout std::vector<Byte> & stlCipherText
            ) const;
        bool __thiscall EncryptFinal(
            _in OperationID poOperationParams,
            _inout std::vector<Byte> & stlCipherText
            ) const;

        OperationID __thiscall DecryptInit(
            _in CryptographicKeyUniquePtr poKey,
            _in const StructuredBuffer * c_poStructuredBufferRequest
            ) const;
        void __thiscall DecryptUpdate(
            _in OperationID poOperationParams,
            _in const std::vector<Byte> & c_stlCipherText,
            _inout std::vector<Byte> & stlPlainText
            ) const;
        bool __thiscall DecryptFinal(
            _in OperationID poOperationParams,
            _inout std::vector<Byte> & stlPlainText
            ) const;
};
