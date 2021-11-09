#include "CryptoUtils.h"

#include "64BitHashes.h"
#include "Base64Encoder.h"
#include "Exceptions.h"
#include "CryptographicEngine.h"
#include "StructuredBuffer.h"
#include "DebugLibrary.h"
#include "DateAndTime.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/********************************************************************************************/

void __thiscall PrintBytesBufferAsHexOnStdout(
    const std::vector<Byte> & stlByteBuffer
)
{
    __DebugFunction();

    std::cout << "[";
    int i = 0;
    for (i = 0; i < stlByteBuffer.size() - 1; i++)
    {
        printf("0x%02X, ", stlByteBuffer.at(i));
        if(0 == ((i+1) % 16))
        {
            std::cout << std::endl;
        }
    }
    printf("0x%02X]\n", stlByteBuffer.at(i));
}

/********************************************************************************************/

std::string __cdecl Base64HashOfEmailPassword(
    const std::string & strEmail,
    const std::string & strPassword
)
{
    __DebugFunction();

    CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    std::string strConcatEmailPassword = strEmail + "/" + strPassword;

    // Calculate SHA256 Hash of the concatinated string
    EVP_MD_CTX * poEvpMdCtx = ::EVP_MD_CTX_new();
    _ThrowIfNull(poEvpMdCtx, "Failed to create Hash Digest", nullptr);

    int nOpenSslStatus = ::EVP_DigestInit_ex(poEvpMdCtx, ::EVP_sha256(), nullptr);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Hash init function failed", nullptr);

    nOpenSslStatus = ::EVP_DigestUpdate(poEvpMdCtx, strConcatEmailPassword.data(), strConcatEmailPassword.length());
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Hash Digest Fail for Input", nullptr);

    unsigned int nHashLength;
    std::vector<Byte> stlMessageDigest(::EVP_MD_CTX_size(poEvpMdCtx));
    nOpenSslStatus = ::EVP_DigestFinal_ex(poEvpMdCtx, stlMessageDigest.data(), &nHashLength);
    _ThrowBaseExceptionIf((1 != nOpenSslStatus), "Hash Digest finalise fail", nullptr);

    // Previously the buffer was set to the max size the message digest could take
    // but the actual size is nHashLength which was calculated in the EVP_DigestFinal_ex call
    // If not resized, there is no way of knowing the actual size unless returned explicitly
    stlMessageDigest.resize(nHashLength);

    ::EVP_MD_CTX_free(poEvpMdCtx);

    return ::Base64Encode(stlMessageDigest.data(), stlMessageDigest.size());
}

/********************************************************************************************/

StructuredBuffer __thiscall EncryptUsingSailSecretKey(
    const std::vector<Byte> & stlPlainText
)
{
    __DebugFunction();

    StructuredBuffer oEncryptedConfidentialUserRecord;
    Guid oSailKeyGuid = "76A426D93D1F4F82AFA48843140EF603";

    struct stat buffer;
    if (0 != stat("76A426D93D1F4F82AFA48843140EF603.key", &buffer))
    {
        std::vector<Byte> c_stlSailKeyFile = {
            0xCD, 0xAB, 0x9C, 0xE5, 0x3B, 0x93, 0x52, 0x2C, 0xB2, 0x9A, 0x7D, 0xE5, 0x57, 0x55, 0x12, 0x98,
            0xC1, 0x6F, 0x29, 0xE8, 0x02, 0x00, 0x00, 0x00, 0x83, 0x63, 0x74, 0x83, 0x3D, 0x00, 0x00, 0x00,
            0x34, 0x01, 0xFB, 0x54, 0x12, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x0D, 0x02, 0x0F,
            0x01, 0x4B, 0x65, 0x79, 0x00, 0x0B, 0x04, 0x0C, 0x03, 0xE5, 0x26, 0x7B, 0x98, 0xA6, 0x03, 0x84,
            0x1F, 0x6C, 0x5F, 0x40, 0xF4, 0x4D, 0xAE, 0x3D, 0x1C, 0xE8, 0x06, 0x0E, 0xEB, 0xC1, 0xCB, 0x68,
            0xD7, 0x0C, 0x0C, 0x60, 0x28, 0xEF, 0xA2, 0x21, 0x8A, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63, 0x74,
            0x83, 0x28, 0x00, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x08, 0x00, 0x00, 0x00, 0x07, 0x00,
            0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x4B, 0x65, 0x79, 0x53, 0x70, 0x65, 0x63, 0x00, 0x0B, 0x04,
            0x0C, 0x03, 0x41, 0x45, 0x53, 0x32, 0x35, 0x36, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x5E, 0xC9, 0xBA,
            0xDC};

        std::ofstream oKeyFile("76A426D93D1F4F82AFA48843140EF603.key", std::ios::binary);
        oKeyFile.write((const char *)c_stlSailKeyFile.data(), c_stlSailKeyFile.size());
        oKeyFile.close();
    }

    CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    std::vector<Byte> stlEncryptedRecord;
    std::vector<Byte> stlInitializationVector = ::GenerateRandomBytes(AES_GCM_IV_LENGTH);
    StructuredBuffer oEncryptParams;
    oEncryptParams.PutBuffer("IV", stlInitializationVector);
    OperationID oSailKeyEncryptId = oCryptographicEngine.OperationInit(CryptographicOperation::eEncrypt, oSailKeyGuid, &oEncryptParams);
    oCryptographicEngine.OperationUpdate(oSailKeyEncryptId, stlPlainText, stlEncryptedRecord);
    bool fEncryptStatus = oCryptographicEngine.OperationFinish(oSailKeyEncryptId, stlEncryptedRecord);
    _ThrowBaseExceptionIf((true != fEncryptStatus), "Encrypt failed.", nullptr);

    // Extract the 16 byte AES GCM Tag from the encrypted Cipher Text and resize the original buffer
    std::vector<Byte> stlAesGcmTag(AES_TAG_LENGTH);
    ::memcpy(stlAesGcmTag.data(), stlEncryptedRecord.data() + (stlEncryptedRecord.size() - AES_TAG_LENGTH), AES_TAG_LENGTH);
    stlEncryptedRecord.resize(stlEncryptedRecord.size() - AES_TAG_LENGTH);

    oEncryptedConfidentialUserRecord.PutBuffer("IV", stlInitializationVector);
    oEncryptedConfidentialUserRecord.PutBuffer("TAG", stlAesGcmTag);
    oEncryptedConfidentialUserRecord.PutBuffer("SailKeyEncryptedConfidentialUserRecord", stlEncryptedRecord);

    return oEncryptedConfidentialUserRecord;
}

/********************************************************************************************/

std::vector<Byte> __thiscall EncryptUsingPasswordKey(
    const std::vector<Byte> & stlPlainText,
    const std::string & strBase64HashOfEmailPassword
)
{
    __DebugFunction();

    CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    // Generate a password derived Key
    CryptographicKeyUniquePtr oPasswordDerivedKey = std::make_unique<CryptographicKey>(KeySpec::ePDKDF2, HashAlgorithm::eSHA256, strBase64HashOfEmailPassword);

    std::vector<Byte> stlCipherText;
    StructuredBuffer oEncryptParams;
    OperationID oPasswordEncryptId = oCryptographicEngine.OperationInit(CryptographicOperation::eEncrypt, std::move(oPasswordDerivedKey), &oEncryptParams);
    oCryptographicEngine.OperationUpdate(oPasswordEncryptId, stlPlainText, stlCipherText);
    bool fEncryptStatus = oCryptographicEngine.OperationFinish(oPasswordEncryptId, stlCipherText);

    return stlCipherText;
}

/********************************************************************************************/

std::vector<Byte> __thiscall EncryptUsingPasswordKey(
    const std::vector<Byte> & stlPlainText,
    const std::string & strEmail,
    const std::string & strPassword
)
{
    __DebugFunction();

    return ::EncryptUsingPasswordKey(stlPlainText, ::Base64HashOfEmailPassword(strEmail, strPassword));
}

/********************************************************************************************/

std::vector<Byte> __thiscall DecryptUsingPasswordKey(
    const std::vector<Byte> & stlCipherText,
    const std::string & strBase64HashOfEmailPassword
)
{
    __DebugFunction();

    CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    // Generate a password derived Key
    CryptographicKeyUniquePtr oPasswordDerivedKey2 = std::make_unique<CryptographicKey>(KeySpec::ePDKDF2, HashAlgorithm::eSHA256, strBase64HashOfEmailPassword);

    StructuredBuffer oEncryptParams;
    std::vector<Byte> stlDecrypted;
    OperationID oPasswordDecryptId = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, std::move(oPasswordDerivedKey2), &oEncryptParams);
    oCryptographicEngine.OperationUpdate(oPasswordDecryptId, stlCipherText, stlDecrypted);
    bool fEncryptStatus = oCryptographicEngine.OperationFinish(oPasswordDecryptId, stlDecrypted);

    return stlDecrypted;
}

/********************************************************************************************/

std::vector<Byte> __thiscall DecryptUsingPasswordKey(
    const std::vector<Byte> & stlCipherText,
    const std::string & strEmail,
    const std::string & strPassword
)
{
    __DebugFunction();

    return ::DecryptUsingPasswordKey(stlCipherText, ::Base64HashOfEmailPassword(strEmail, strPassword));
}

/********************************************************************************************/


std::vector<Byte> __thiscall DecryptUsingSailSecretKey(
    const std::vector<Byte> & stlCipherText
)
{
    __DebugFunction();

    StructuredBuffer oEncryptedUserRecord(stlCipherText);
    StructuredBuffer oDecryptParams;
    oDecryptParams.PutBuffer("IV", oEncryptedUserRecord.GetBuffer("IV"));
    oDecryptParams.PutBuffer("TAG", oEncryptedUserRecord.GetBuffer("TAG"));

    Guid oSailSecretKey = "76A426D93D1F4F82AFA48843140EF603";
    struct stat buffer;
    if (0 != stat("76A426D93D1F4F82AFA48843140EF603.key", &buffer))
    {
        std::vector<Byte> c_stlSailKeyFile = {
            0xCD, 0xAB, 0x9C, 0xE5, 0x3B, 0x93, 0x52, 0x2C, 0xB2, 0x9A, 0x7D, 0xE5, 0x57, 0x55, 0x12, 0x98,
            0xC1, 0x6F, 0x29, 0xE8, 0x02, 0x00, 0x00, 0x00, 0x83, 0x63, 0x74, 0x83, 0x3D, 0x00, 0x00, 0x00,
            0x34, 0x01, 0xFB, 0x54, 0x12, 0x04, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x0D, 0x02, 0x0F,
            0x01, 0x4B, 0x65, 0x79, 0x00, 0x0B, 0x04, 0x0C, 0x03, 0xE5, 0x26, 0x7B, 0x98, 0xA6, 0x03, 0x84,
            0x1F, 0x6C, 0x5F, 0x40, 0xF4, 0x4D, 0xAE, 0x3D, 0x1C, 0xE8, 0x06, 0x0E, 0xEB, 0xC1, 0xCB, 0x68,
            0xD7, 0x0C, 0x0C, 0x60, 0x28, 0xEF, 0xA2, 0x21, 0x8A, 0x45, 0xBF, 0x10, 0x34, 0x83, 0x63, 0x74,
            0x83, 0x28, 0x00, 0x00, 0x00, 0x34, 0x01, 0xFB, 0x54, 0x03, 0x08, 0x00, 0x00, 0x00, 0x07, 0x00,
            0x00, 0x00, 0x0D, 0x02, 0x0F, 0x01, 0x4B, 0x65, 0x79, 0x53, 0x70, 0x65, 0x63, 0x00, 0x0B, 0x04,
            0x0C, 0x03, 0x41, 0x45, 0x53, 0x32, 0x35, 0x36, 0x00, 0x45, 0xBF, 0x10, 0x34, 0x5E, 0xC9, 0xBA,
            0xDC};

        std::ofstream oKeyFile("76A426D93D1F4F82AFA48843140EF603.key", std::ios::binary);
        oKeyFile.write((const char *)c_stlSailKeyFile.data(), c_stlSailKeyFile.size());
        oKeyFile.close();
    }

    CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

    std::vector<Byte> stlUserKeyEncryptedUserRecord;
    OperationID oDecryptionID = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, oSailSecretKey, &oDecryptParams);
    oCryptographicEngine.OperationUpdate(oDecryptionID, oEncryptedUserRecord.GetBuffer("SailKeyEncryptedConfidentialUserRecord"), stlUserKeyEncryptedUserRecord);
    bool fDecryptStatus = oCryptographicEngine.OperationFinish(oDecryptionID, stlUserKeyEncryptedUserRecord);
    _ThrowBaseExceptionIf((false == fDecryptStatus), "Confidential User Record Decryption using SAIL Key failed.", nullptr);

    return stlUserKeyEncryptedUserRecord;
}

/********************************************************************************************/

std::vector<Byte> __thiscall GenerateAccountKey(void)
{
    __DebugFunction();

    CryptographicKey oCryptographicAccountKey(KeySpec::eAES256);
    return oCryptographicAccountKey.GetSymmetricKey();
}
