/*********************************************************************************************
 *
 * @file CryptographyHelperFunctions.cpp
 * @author Prawal Gangwar
 * @date 4 Jan 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "Object.h"
#include "CryptographicEngine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"

#include <openssl/rand.h>

/********************************************************************************************
 *
 * @function GenerateRandomBytes
 * @brief Generates a random byte vector of specified length
 * @param[in] unSizeInBytes Size of the random vector
 * @return Random Bytes vector of size unSizeInBytes
 * @throw BaseException on error
 * @note This call provides an abstraction over the Openssl's secure random function
 *      and can be used to generate the Initializaion Vector and AES Key of specified length
 ********************************************************************************************/

std::vector<Byte> __thiscall GenerateRandomBytes(
    _in unsigned int unSizeInBytes
    )
{
    __DebugFunction();

    std::vector<Byte> stlRandomVector(unSizeInBytes);

    int nOpenSslStatus = ::RAND_bytes(stlRandomVector.data(), stlRandomVector.size());
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Random Generator Failed", nullptr);

    return stlRandomVector;
}

/********************************************************************************************
 *
 * @function GetEVP_MDForHashAlgorithm
 * @brief Returns the EVP_MD function pointer corresponding to the Hashing Algorithm
 * @param[in] eHashAlgorithm Hash Algorithm for which EVP_MD is needed
 * @return Pointer to EVP_MD which is the hash function
 * @throw BaseException on error
 * @note The returned EVP_MD is not an allocation that needs to be freed. It just points to
 *      a relevant hash function.
 *
 ********************************************************************************************/

const EVP_MD * __thiscall GetEVP_MDForHashAlgorithm(
    _in HashAlgorithm eHashAlgorithm
    )
{
    __DebugFunction();

    const EVP_MD * poEvpMd = nullptr;

    switch (eHashAlgorithm)
    {
        case HashAlgorithm::eSHA1
        :   poEvpMd = ::EVP_sha1();
            break;
        case HashAlgorithm::eSHA256
        :   poEvpMd = ::EVP_sha256();
            break;
        case HashAlgorithm::eSHA384
        :   poEvpMd = ::EVP_sha384();
            break;
        case HashAlgorithm::eMD5
        :   poEvpMd = ::EVP_md5();
            break;
        default
        :   _ThrowBaseException("Invalid Hash Function", nullptr);
            break;
    }

    return poEvpMd;
}

/********************************************************************************************
 *
 * @function GetEVP_CIPHERForAesKey
 * @brief Returns the EVP_CIPHER function pointer corresponding to the AES key type
 * @param[in] eKeySpec The AES key type
 * @return Pointer to EVP_CIPHER which is the cipher function
 * @throw BaseException on error
 * @note The returned EVP_CIPHER is not an allocation that needs to be freed. It just points to
 *       a relevant hash function.
 *
 ********************************************************************************************/

const EVP_CIPHER * __thiscall GetEVP_CIPHERForAesKey(
    _in KeySpec eKeySpec
    )
{
    __DebugFunction();

    const EVP_CIPHER * poEvpCipherResponse = nullptr;

    switch (eKeySpec)
    {
        case KeySpec::eAES128
        :   poEvpCipherResponse = ::EVP_aes_128_gcm();
            break;
        case KeySpec::eAES256
        :   poEvpCipherResponse = ::EVP_aes_256_gcm();
            break;
        case KeySpec::ePDKDF2
        :   poEvpCipherResponse = ::EVP_aes_256_cfb128();
            break;
        default
        :   _ThrowBaseException("EVP_CIPHER not available for the config", nullptr);
            break;
    }

    return poEvpCipherResponse;
}

/********************************************************************************************
 *
 * @function GetEVP_CIPHERForAesKey
 * @brief Returns the EVP_CIPHER function pointer corresponding to the AES key type
 * @param[in] eKeySpec The AES key type
 * @return Pointer to EVP_CIPHER which is the cipher function
 * @throw BaseException on error
 * @note The returned EVP_CIPHER is not an allocation that needs to be freed. It just points to
 *       a relevant hash function.
 *
 ********************************************************************************************/

KeySpec __thiscall GetKeySpecFromString(
    _in const std::string & strKeyType
    )
{
    __DebugFunction();

    KeySpec eResponseKeySpec = KeySpec::eInvalidKey;

    if ("RSA2048" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eRSA2048;
    }
    else if ("RSA3072" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eRSA3076;
    }
    else if ("RSA4096" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eRSA4096;
    }
    else if ("ECC384" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eECC384;
    }
    else if ("AES128" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eAES128;
    }
    else if ("AES256" == strKeyType)
    {
        eResponseKeySpec = KeySpec::eAES256;
    }
    else if ("PDKDF2" == strKeyType)
    {
        eResponseKeySpec = KeySpec::ePDKDF2;
    }

    return eResponseKeySpec;
}

/********************************************************************************************
 *
 * @function GetEVP_CIPHERForAesKey
 * @brief Returns the EVP_CIPHER function pointer corresponding to the AES key type
 * @param[in] eKeySpec The AES key type
 * @return Pointer to EVP_CIPHER which is the cipher function
 * @throw BaseException on error
 * @note The returned EVP_CIPHER is not an allocation that needs to be freed. It just points to
 *       a relevant hash function.
 *
 ********************************************************************************************/

std::string __thiscall GetStringForKeySpec(
    _in KeySpec eKeySpec
    )
{
    __DebugFunction();

    std::string strResponseString;

    if (KeySpec::eRSA2048 == eKeySpec)
    {
        strResponseString = "RSA2048";
    }
    else if (KeySpec::eRSA3076 == eKeySpec)
    {
        strResponseString = "RSA3072";
    }
    else if (KeySpec::eRSA4096 == eKeySpec)
    {
        strResponseString = "RSA4096";
    }
    else if (KeySpec::eECC384 == eKeySpec)
    {
        strResponseString = "ECC384";
    }
    else if (KeySpec::eAES128 == eKeySpec)
    {
        strResponseString = "AES128";
    }
    else if (KeySpec::eAES256 == eKeySpec)
    {
        strResponseString = "AES256";
    }
    else if (KeySpec::ePDKDF2 == eKeySpec)
    {
        strResponseString = "PDKDF2";
    }

    return strResponseString;
}

/********************************************************************************************
 *
 * @function GetHashAlgorithmFromString
 * @brief Returns the HashAlgorithm enum corresponding to the Hash Algorithm string
 * @param[in] strHashAlgorithm The Hash Algorithm string
 * @return Hash Algorithm enum
 * @throw BaseException on error
 *
 ********************************************************************************************/

HashAlgorithm __thiscall GetHashAlgorithmFromString(
    _in const std::string & strHashAlgorithm
    )
{
    HashAlgorithm eResponseHashAlgorithm = HashAlgorithm::eInvalidHashAlgorithm;

    if ("SHA1" == strHashAlgorithm)
    {
        eResponseHashAlgorithm = HashAlgorithm::eSHA1;
    }
    else if ("SHA256" == strHashAlgorithm)
    {
        eResponseHashAlgorithm = HashAlgorithm::eSHA256;
    }
    else if ("SHA384" == strHashAlgorithm)
    {
        eResponseHashAlgorithm = HashAlgorithm::eSHA384;
    }
    else if ("MD5" == strHashAlgorithm)
    {
        eResponseHashAlgorithm = HashAlgorithm::eMD5;
    }

    return eResponseHashAlgorithm;
}
