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

/********************************************************************************************/

std::string __stdcall Base64Encode(
    _in const Byte * c_pbRawBuffer,
    _in unsigned int unRawBufferSizeInBytes
    ) throw()
{
    __DebugFunction();
    
    std::string strBase64EncodedString;

    if ((nullptr != c_pbRawBuffer)&&(0 < unRawBufferSizeInBytes))
    {
        static const char * sc_szLookupTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        int nValue = 0;
        int nValueB = -6;
        for (unsigned int unIndex = 0; unIndex < unRawBufferSizeInBytes; ++unIndex)
        {
            Byte bCurrentByte = c_pbRawBuffer[unIndex];
            nValue = (nValue << 8) + bCurrentByte;
            nValueB += 8;
            while (0 <= nValueB)
            {
                strBase64EncodedString.push_back(sc_szLookupTable[(nValue >> nValueB) & 0x3F]);
                nValueB -= 6;
            }
        }
        
        if (-6 < nValueB)
        {
            strBase64EncodedString.push_back(sc_szLookupTable[((nValue << 8) >> (nValueB + 8)) & 0x3F]);
        }
        
        // Do we need to add padding ('=') at the end of our base64 encoded string
        while (0 != (strBase64EncodedString.size() %4))
        {
            strBase64EncodedString.push_back('=');
        }
    }
    
    return strBase64EncodedString;
}

/********************************************************************************************/
    
std::vector<Byte> __stdcall Base64Decode(
    _in const char * c_szBase64String
    )
{
    __DebugFunction();
    
    std::vector<Byte> stlDecodedRawBuffer;
    
    if (nullptr != c_szBase64String)
    {
        static const int sc_anLookupTable[] = {  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  };

        int nValue = 0;
        int nValueB =-8;
        unsigned int unIndex = 0;
        unsigned char ucCharacter = c_szBase64String[0];
        while ((0 != ucCharacter)&&('=' != ucCharacter))
        {
            _ThrowBaseExceptionIf((-1 == sc_anLookupTable[ucCharacter]), "Invalid character encountered in Base64 string", nullptr);
            
            nValue = (nValue << 6) + sc_anLookupTable[ucCharacter];
            nValueB += 6;
            if (0 <= nValueB)
            {
                stlDecodedRawBuffer.push_back(Byte((nValue >> nValueB) & 0xFF));
                nValueB -= 8;
            }
            
            ++unIndex;
            ucCharacter = c_szBase64String[unIndex];
        }
    }
    
    return stlDecodedRawBuffer;
}
    
