/*********************************************************************************************
 *
 * @file CryptoUtils.h
 * @date 27 Jan 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

 #pragma once

 #include "CoreTypes.h"
 #include "StructuredBuffer.h"

/********************************************************************************************/

void __stdcall PrintBytesBufferAsHexOnStdout(
     _in const std::vector<Byte> & stlByteBuffer
    );

/********************************************************************************************/

extern "C" std::string __cdecl Base64HashOfEmailPassword(
     _in const std::string & strEmail,
     _in const std::string & strPassword
    );

/********************************************************************************************/

StructuredBuffer __stdcall EncryptUsingSailSecretKey(
     _in const std::vector<Byte> & stlPlainText
    );

/********************************************************************************************/

std::vector<Byte> __stdcall EncryptUsingPasswordKey(
    _in const std::vector<Byte> & stlPlainText,
    _in const std::string & strBase64HashOfEmailPassword
    );

/********************************************************************************************/

std::vector<Byte> __stdcall EncryptUsingPasswordKey(
    _in const std::vector<Byte> & stlPlainText,
    _in const std::string & strEmail,
    _in const std::string & strPassword
    );

/********************************************************************************************/

std::vector<Byte> __stdcall DecryptUsingPasswordKey(
    _in const std::vector<Byte> & stlCipherText,
    _in const std::string & strBase64HashOfEmailPassword
    );

/********************************************************************************************/

std::vector<Byte> __stdcall DecryptUsingPasswordKey(
    _in const std::vector<Byte> & stlCipherText,
    _in const std::string & strEmail,
    _in const std::string & strPassword
    );

/********************************************************************************************/

std::vector<Byte> __stdcall DecryptUsingSailSecretKey(
    _in const std::vector<Byte> & stlCipherText
    );

/********************************************************************************************/

std::vector<Byte> __stdcall GenerateAccountKey(void);

/********************************************************************************************/

void AddUserAccountsToDatabase(
    const StructuredBuffer & c_oBasicUser,
    const StructuredBuffer & c_oConfidentialUser
    );
