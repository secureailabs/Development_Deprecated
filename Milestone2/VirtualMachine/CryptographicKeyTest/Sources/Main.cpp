#include "Base64Encoder.h"
#include "Exceptions.h"
#include "CryptographicEngine.h"
#include "StructuredBuffer.h"

#include <openssl/rand.h>

#include <iostream>
#include <cstdlib>

int main()
{
    std::vector<Byte> yolo = {'A', 'B', 'C', 'D'};

    try
    {
        CryptographicEngine & oCryptographicEngine = CryptographicEngine::Get();

        // // #################### RSA Encrypt/Decrypt ##################################################
        Guid rsakeyId = oCryptographicEngine.GenerateKey(KeySpec::eRSA2048);
        std::cout << "RSA Key Generated: " << rsakeyId.ToString(eRaw) << std::endl;
        // RSA Encrypt rsaRequest
        StructuredBuffer rsaRequest;
        rsaRequest.PutString("KeyType", "PublicKey");
        std::vector<Byte> rsaCipher;
        OperationID newID = oCryptographicEngine.OperationInit(CryptographicOperation::eEncrypt, rsakeyId, &rsaRequest);
        oCryptographicEngine.OperationUpdate(newID, yolo, rsaCipher);
        oCryptographicEngine.OperationFinish(newID, yolo);

        // RSA Decrypt rsaRequest
        std::vector<Byte> rsaDecrypted;
        rsaRequest.PutString("KeyType", "PrivateKey");
        newID = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, rsakeyId, &rsaRequest);
        oCryptographicEngine.OperationUpdate(newID, rsaCipher, rsaDecrypted);
        oCryptographicEngine.OperationFinish(newID, rsaCipher);
        std::cout << "RSA Plain after decryption: " << std::string(rsaDecrypted.data(), rsaDecrypted.data()+yolo.size()) << std::endl;

        // #################### AES #########################################################
        Guid symKey = oCryptographicEngine.GenerateKey(KeySpec::eAES256);
        std::cout << "AES Key Generated: " << symKey.ToString(eRaw) << std::endl;
        std::vector<Byte> stlSymmetricKeyIV = ::GenerateRandomBytes(12);
        StructuredBuffer symRequest;
        symRequest.PutBuffer("IV", stlSymmetricKeyIV);

        std::vector<Byte> aesCipher;
        OperationID newIDAES = oCryptographicEngine.OperationInit(CryptographicOperation::eEncrypt, symKey, &symRequest);
        std::cout << "OperationInit" << newIDAES.get() <<  std::endl;
        oCryptographicEngine.OperationUpdate(newIDAES, yolo, aesCipher);
        oCryptographicEngine.OperationFinish(newIDAES, aesCipher);

        // AES decrypt
        symRequest.PutBuffer("TAG", aesCipher.data() + (aesCipher.size() - 16), 16);
        auto aesDecrypt = oCryptographicEngine.OperationInit(CryptographicOperation::eDecrypt, symKey, &symRequest);
        std::vector<Byte> aesDecrypted;
        oCryptographicEngine.OperationUpdate(aesDecrypt, aesCipher, aesDecrypted);
        oCryptographicEngine.OperationFinish(aesDecrypt, aesDecrypted);
        std::cout << "AES Decrypt " << std::string(aesDecrypted.data(), aesDecrypted.data()+yolo.size())<< std::endl;

        // #################### EC #########################################################
        auto ecKey = oCryptographicEngine.GenerateKey(KeySpec::eECC384);
        std::cout << "EC Key Generated: " << ecKey.ToString(eRaw) << std::endl;

        // #################### SHA256 Hash #########################################################
        std::vector<Byte> hash;
        // TODO: do we reallt need hash in Engine ?
        OperationID shaID = oCryptographicEngine.OperationInit(CryptographicOperation::eSHA256Digest, symKey);
        oCryptographicEngine.OperationUpdate(shaID, yolo, hash);
        oCryptographicEngine.OperationFinish(shaID, hash);
        std::cout << "SHA256 Digest: " << Base64Encode(hash.data(), hash.size()) << std::endl;

        // #################### RSA Sign #########################################################
        std::vector<Byte> RSAsignature;
        OperationID signId = oCryptographicEngine.OperationInit(CryptographicOperation::eDigitalSignature, rsakeyId);
        oCryptographicEngine.OperationUpdate(signId, hash, RSAsignature);
        oCryptographicEngine.OperationFinish(signId, RSAsignature);

        // RSA Sign Verify
        OperationID verifyId = oCryptographicEngine.OperationInit(CryptographicOperation::eDigitalSignatureVerify, rsakeyId);
        oCryptographicEngine.OperationUpdate(verifyId, RSAsignature, hash);
        std::cout << "RSA Sign Verify Result " << oCryptographicEngine.OperationFinish(verifyId, RSAsignature) << std::endl;

        // #################### EC Sign #########################################################
        std::vector<Byte> ECsignature;
        OperationID ecsignId = oCryptographicEngine.OperationInit(CryptographicOperation::eDigitalSignature, ecKey);
        oCryptographicEngine.OperationUpdate(ecsignId, hash, ECsignature);
        oCryptographicEngine.OperationFinish(ecsignId, ECsignature);
        std::cout << "EC Signature: " << Base64Encode(ECsignature.data(), ECsignature.size()) << std::endl;

        OperationID ecVerifyId = oCryptographicEngine.OperationInit(CryptographicOperation::eDigitalSignatureVerify, ecKey);
        oCryptographicEngine.OperationUpdate(ecVerifyId, ECsignature, hash);
        std::cout << "EC Sign Verify Result " << oCryptographicEngine.OperationFinish(ecVerifyId, ECsignature) << std::endl;

        // #################### X509 cert #########################################################
        StructuredBuffer oSubject;
        oSubject.PutString("CN", "www.secureailabs.com");
        oSubject.PutString("C", "US");
        oSubject.PutString("ST", "Ontario");
        oSubject.PutString("L", "Ottawa");
        oSubject.PutString("O", "SAIL");
        oSubject.PutString("OU", "SAIL Root CA");

        StructuredBuffer oRequestCertificate;
        oRequestCertificate.PutUnsignedInt64("Version", 3);
        oRequestCertificate.PutUnsignedInt64("SerialNumber", rand());
        oRequestCertificate.PutStructuredBuffer("Subject", oSubject);
        oRequestCertificate.PutStructuredBuffer("Issuer", oSubject);
        oRequestCertificate.PutString("ValidityNotBefore", "20200302235959Z");
        oRequestCertificate.PutString("ValidityNotAfter", "20210302235959Z");

        X509Certificate oX509Certificate(ecKey, oRequestCertificate);
        Guid oKeyGuid = oX509Certificate.GetPublicKeyFromCertificate();
        std::cout << "Certificate" << oX509Certificate.GetPemCertificate() << std::endl;

        // #################### X509 CSR #########################################################
        CertificateSigningRequest oCertificateSigningRequest(ecKey, oRequestCertificate);
        // Guid oCSRKey = oCertificateSigningRequest.GetPublicKeyFromCertificateRequest();
        std::cout << "CSR " << oCertificateSigningRequest.GetPemCertificateRequest() << std::endl;
        std::string strPemCCR =  oCertificateSigningRequest.GetPemCertificateRequest();
        CertificateSigningRequest oNewCSR(strPemCCR);
        std::cout << "CSR " << oCertificateSigningRequest.GetPemCertificateRequest() << std::endl;
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception caught\n";
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exception\n";
    }

    std::cout << "That's all folks\n";

    return 0;
}
