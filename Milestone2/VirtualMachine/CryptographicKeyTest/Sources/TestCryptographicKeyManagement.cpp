/*********************************************************************************************
 *
 * @file TestCryptographicKeyManagement.cpp
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <string.h>

#include "TlsClient.h"
#include "Exceptions.h"
#include "StructuredBuffer.h"
#include "HardCodedCryptographicKeys.h"
#include "DebugLibrary.h"

/********************************************************************************************/

std::vector<Byte> __stdcall CreateRequestPacket(
    _in const StructuredBuffer & c_oRequestPacket
    )
{
    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(uint32_t) + c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes() + sizeof(Dword);

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
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes();
    pbSerializedBuffer += sizeof(uint32_t);
    ::memcpy((void *) pbSerializedBuffer, (const void *) c_oRequestPacket.GetSerializedBufferRawDataPtr(), c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes());
    pbSerializedBuffer += c_oRequestPacket.GetSerializedBufferRawDataSizeInBytes();
    *((Dword *) pbSerializedBuffer) = 0x656e6420;

    return stlSerializedBuffer;
}

/********************************************************************************************/

StructuredBuffer GetRemoteAttestationProofAndCertificate(
    _in const std::vector<Byte> c_stlChallengeNonceBuffer
    )
{
    __DebugFunction();

    StructuredBuffer oRemoteAttestationProofAndCertificate;

    StructuredBuffer oChallengeNonce(c_stlChallengeNonceBuffer);
    uint64_t un64BitIdentifier = oChallengeNonce.GetQword("ChallengeNonceId");
    std::vector<Byte> stlChallengeNonce = oChallengeNonce.GetBuffer("Nonce");

    oRemoteAttestationProofAndCertificate.PutQword("ChallengeNonceId", un64BitIdentifier);
    oRemoteAttestationProofAndCertificate.PutBuffer("Proof", stlChallengeNonce);
    oRemoteAttestationProofAndCertificate.PutBuffer("Certificate", gc_abRemoteAttestationPublicKeyCertificate, gc_unRemoteAttestationPublicKeyCertificateSizeInBytes);

    return oRemoteAttestationProofAndCertificate;
}

/********************************************************************************************/

std::vector<Byte> TestAuthenticateIntitializer(void)
{
    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create rest request
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "CryptographicKeyManagementPlugin");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Initializer/ChallengeNonce");

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    std::cout << "Response from Rest Framework:  " << std::endl;
    std::cout << std::string(stlRestResponse.begin(), stlRestResponse.end()) << std::endl;

    return stlRestResponse;
}

/********************************************************************************************/

void TestSignPublicKeyCertificate(
    _in std::vector<Byte> stlChallengeNonceBuffer
    )
{
    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    StructuredBuffer oRemoteAttestationProofAndCertificate = ::GetRemoteAttestationProofAndCertificate(stlChallengeNonceBuffer);

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "CryptographicKeyManagementPlugin");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Initializer/KeyPairAndCertificate");
    oRestRequestStructuredBuffer.PutStructuredBuffer("RequiredParameters", oRemoteAttestationProofAndCertificate);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet.", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet.", nullptr);

    std::cout << "Response from Rest Framework:  " << std::endl;
    std::cout << std::string(stlRestResponse.begin(), stlRestResponse.end()) << std::endl;

}

/********************************************************************************************/

void TestSignDatasetHash(void)
{
    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create Request StructuredBuffer
    StructuredBuffer oStructuredBufferSignHashRequest;
    oStructuredBufferSignHashRequest.PutString("EOSB", "abcd");
    oStructuredBufferSignHashRequest.PutString("base64SHA2Hash", "U0hBMjU2KHRleHQpPSAzYjA5YWViNmY1ZjUzMzZiZWIyMDVkN2Y3MjAzNzFiYzkyN2NkNDZjMjE5MjJlMzM0ZDQ3YmEyNjRhY2I1YmE0Cg==");

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "CryptographicKeyManagementPlugin");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Initializer/SignDatasetHash");
    oRestRequestStructuredBuffer.PutStructuredBuffer("RequiredParameters", oStructuredBufferSignHashRequest);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet", nullptr);

    std::cout << "Response from Rest Framework:  " << std::endl;
    std::cout << std::string(stlRestResponse.begin(), stlRestResponse.end()) << std::endl;

}

/********************************************************************************************/
void TestGenerateKey(void)
{
    std::vector<Byte> stlRestResponse;
    TlsNode * poTlsNode = nullptr;
    poTlsNode = ::TlsConnectToNetworkSocket("127.0.0.1", 6200);

    // Create Request StructuredBuffer
    StructuredBuffer oSBKeyGenParams;
    oSBKeyGenParams.PutString("KeyType", "RSA2048");

    // Create rest packet
    StructuredBuffer oRestRequestStructuredBuffer;
    oRestRequestStructuredBuffer.PutString("PluginName", "CryptographicKeyManagementPlugin");
    oRestRequestStructuredBuffer.PutString("Verb", "POST");
    oRestRequestStructuredBuffer.PutString("Resource", "SecureAiLabs/Initializer/GenerateKey");
    oRestRequestStructuredBuffer.PutStructuredBuffer("RequiredParameters", oSBKeyGenParams);

    std::vector<Byte> stlRestRequest = ::CreateRequestPacket(oRestRequestStructuredBuffer);

    // Send request packet
    poTlsNode->Write(stlRestRequest.data(), (stlRestRequest.size()));

    // Read header and body of the response
    std::vector<Byte> stlRestResponseLength = poTlsNode->Read(sizeof(uint32_t), 100);
    _ThrowBaseExceptionIf((0 == stlRestResponseLength.size()), "Dead Packet", nullptr);
    unsigned int unResponseDataSizeInBytes = *((uint32_t *) stlRestResponseLength.data());
    stlRestResponse = poTlsNode->Read(unResponseDataSizeInBytes, 100);
    _ThrowBaseExceptionIf((0 == stlRestResponse.size()), "Dead Packet", nullptr);

    StructuredBuffer oResponse = StructuredBuffer(stlRestResponse);
    std::cout << "Response from Rest Framework:  " << std::endl;
    std::cout << oResponse.GetGuid("KeyGuid").ToString(eHyphensAndCurlyBraces) << std::endl;
}

/********************************************************************************************/

int foo_main()
{
    std::cout << "----------TEST CLIENT----------" << std::endl;

    try
    {
        std::cout << "\n--------TESTING GENERATEKEY--------\n" << std::endl;
        ::TestGenerateKey();
        std::cout << "\n--------TESTING AUTHENTICATEINITIALIZER--------\n" << std::endl;
        std::vector<Byte> stlChallengeNonceBuffer = ::TestAuthenticateIntitializer();
        std::cout << "\n--------TESTING SIGNPUBLICKEYCERTIFICATE--------\n" << std::endl;
        ::TestSignPublicKeyCertificate(stlChallengeNonceBuffer);
        std::cout << "\n--------TESTING SIGNDATASETHASH--------\n" << std::endl;
        ::TestSignDatasetHash();
    }
    catch(BaseException oBaseException)
    {
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
        std::cout << oBaseException.GetFunctionName() << std::endl;
        std::cout << oBaseException.GetFilename() << std::endl;
    }
    return 0;
}
