/*********************************************************************************************
 *
 * @file Base64Encoding.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Base64Encoder.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"

#include <cstring>

/********************************************************************************************/
std::string __stdcall Base64Encode(
    _in const Byte * c_pbRawBuffer,
    _in unsigned int unRawBufferSizeInBytes
    ) throw()
{
    __DebugFunction();

    std::vector<char> stlBase64EncodedString;
    if ((nullptr != c_pbRawBuffer)&&(0 < unRawBufferSizeInBytes))
    {
        size_t unSourceOffset{0};
        size_t unDestOffset{0};
        constexpr char aszEncodingTable[] = {
          'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
          'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
          'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
          'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
          'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
          'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
          'w', 'x', 'y', 'z', '0', '1', '2', '3',
          '4', '5', '6', '7', '8', '9', '+', '/'
        };

        // We add one to account for nul byte
        stlBase64EncodedString.resize((4 * ((unRawBufferSizeInBytes + 2) / 3)) + 1, 0);
        char* pbDestPtr{&stlBase64EncodedString[0]};

        // Encode 3 bytes at a time
        while ( unRawBufferSizeInBytes >= 3 )
        {
            pbDestPtr[unDestOffset] = aszEncodingTable[(c_pbRawBuffer[unSourceOffset] & 0xFC) >> 2];
            pbDestPtr[unDestOffset+1] = aszEncodingTable[((c_pbRawBuffer[unSourceOffset] & 0x03) << 4) + ((c_pbRawBuffer[unSourceOffset+1] & 0xF0) >> 4)];
            pbDestPtr[unDestOffset+2] = aszEncodingTable[((c_pbRawBuffer[unSourceOffset+1] & 0x0F) << 2) + ((c_pbRawBuffer[unSourceOffset+2] & 0xC0) >> 6)];
            pbDestPtr[unDestOffset+3] = aszEncodingTable[(c_pbRawBuffer[unSourceOffset+2] & 0x3F)];

            unDestOffset += 4;
            unSourceOffset += 3;
            unRawBufferSizeInBytes -= 3;

        }

        // Handle padding for 1 or 2 bytes
        if ( unRawBufferSizeInBytes == 1 )
        {
            pbDestPtr[unDestOffset] = aszEncodingTable[(c_pbRawBuffer[unSourceOffset] & 0xFC) >> 2];
            pbDestPtr[unDestOffset + 1] = aszEncodingTable[((c_pbRawBuffer[unSourceOffset] & 0x03) << 4)];
            pbDestPtr[unDestOffset + 2] = '=';
            pbDestPtr[unDestOffset + 3] = '=';
            unRawBufferSizeInBytes -= 1;
            unDestOffset += 4;
        }
        else if ( unRawBufferSizeInBytes == 2 )
        {
            pbDestPtr[unDestOffset] = aszEncodingTable[(c_pbRawBuffer[unSourceOffset] & 0xFC) >> 2];
            pbDestPtr[unDestOffset + 1] = aszEncodingTable[((c_pbRawBuffer[unSourceOffset] & 0x03) << 4) + ((c_pbRawBuffer[unSourceOffset + 1] & 0xF0) >> 4)];
            pbDestPtr[unDestOffset + 2] = aszEncodingTable[((c_pbRawBuffer[unSourceOffset + 1] & 0x0F) << 2)];
            pbDestPtr[unDestOffset + 3] = '=';
            unRawBufferSizeInBytes -= 2;
            unDestOffset += 4;
        }
    }

    return stlBase64EncodedString.size() > 0 ? std::string(&stlBase64EncodedString[0]) : "";
}

std::vector<Byte> __stdcall Base64Decode(
    _in const char * c_szBase64String
    )
{
    __DebugFunction();
    
    std::vector<Byte> stlDecodedRawBuffer;
    if (nullptr != c_szBase64String)
    {
        size_t unInputStringLength = strlen(c_szBase64String);
        size_t unOutputStringLength = (unInputStringLength / 4) * 3;

        // Account for padding in the input
        if ('=' == c_szBase64String[unInputStringLength - 1])
        {
            unOutputStringLength--;
        }
        if ('=' == c_szBase64String[unInputStringLength - 2])
        {
            unOutputStringLength--;
        }
        // Allocate the destination buffer
        stlDecodedRawBuffer.resize(unOutputStringLength);

        constexpr unsigned char aszDecodingTable[] = {
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
          52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
          64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
          15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
          64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
          41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
          64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
        };

        for (unsigned int unCurrentCharacter = 0, unNextCharacter = 0; unCurrentCharacter < unInputStringLength;)
        {
            uint32_t unFirstSextet = c_szBase64String[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_szBase64String[unCurrentCharacter++])];
            uint32_t unSecondSextet = c_szBase64String[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_szBase64String[unCurrentCharacter++])];
            uint32_t unThirdSextet = c_szBase64String[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_szBase64String[unCurrentCharacter++])];
            uint32_t unFourthSextet = c_szBase64String[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_szBase64String[unCurrentCharacter++])];

            uint32_t unSextets = (unFirstSextet << 3 * 6) + (unSecondSextet << 2 * 6) + (unThirdSextet << 1 * 6) + (unFourthSextet << 0 * 6);

            if (unNextCharacter < unOutputStringLength)
            {
                stlDecodedRawBuffer[unNextCharacter++] = (unSextets >> 2 * 8) & 0xFF;
            }
            if (unNextCharacter < unOutputStringLength)
            {
                stlDecodedRawBuffer[unNextCharacter++] = (unSextets >> 1 * 8) & 0xFF;
            }
            if (unNextCharacter < unOutputStringLength)
            {
                stlDecodedRawBuffer[unNextCharacter++] = (unSextets >> 0 * 8) & 0xFF;
            }
        }
    }
    return stlDecodedRawBuffer;
}

