/*********************************************************************************************
 *
 * @file Utils.cpp
 * @author Shabana Akhtar Baig
 * @date 16 Dec 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

 #include "Utils.h"
 #include "DebugLibrary.h"
 #include "Exceptions.h"

 #include <algorithm>
 #include <cctype>
 #include <cstring>

/********************************************************************************************
 *
 * @function IsNumber
 * @brief Check if the argument is a number
 * @param[in] c_strValue string value
 * @return true if its a number
 * @return false otherwise
 *
 ********************************************************************************************/

bool __stdcall IsNumber(
    _in const std::string & c_strValue
    )
{
    __DebugFunction();

    bool fIsNumber = !c_strValue.empty() && std::all_of(c_strValue.begin(), c_strValue.end(), ::isdigit);

    return fIsNumber;
}

/********************************************************************************************
 *
 * @function IsBoolean
 * @brief Check if the argument is a boolean
 * @param[in] c_strValue string value
 * @return true if its a boolean
 * @return false otherwise
 *
 ********************************************************************************************/

bool __stdcall IsBoolean(
    _in const std::string & c_strValue
    )
{
    __DebugFunction();

    bool fIsBoolean = false;

    if ((4 == c_strValue.length()) && (0 == strncasecmp(c_strValue.c_str(), "true", 4)))
    {
        fIsBoolean = true;
    }
    else if ((5 == c_strValue.length()) && (0 == strncasecmp(c_strValue.c_str(), "false", 5)))

    return fIsBoolean;
}

/********************************************************************************************
 *
 * @function EscapeJsonString
 * @brief Escape json string
 * @param[in] c_strData json string
 * @return escaped string
 *
 ********************************************************************************************/

std::string __stdcall EscapeJsonString(
    _in const std::string & c_strData
    )
{
    __DebugFunction();

    std::string strStringValue = "\"";
    std::string::const_iterator itrData = c_strData.begin();

    while (c_strData.end() != itrData)
    {
        char szCurrentChar = *itrData;

        switch(szCurrentChar)
        {
            case '"'
            :
                strStringValue += "\\\"";
                break;
            case '\\'
            :
                strStringValue += "\\\\";
                break;
            case '/'
            :
                strStringValue += "\\/";
                break;
            case '\b'
            :
                strStringValue += "\\b";
                break;
            case '\n'
            :
                strStringValue += "\\n";
                break;
            case '\r'
            :
                strStringValue += "\\r";
                break;
            case '\t'
            :
                strStringValue += "\\t";
                break;
            case '\f'
            :
                strStringValue += "\\f";
                break;
            default
            :
                strStringValue += szCurrentChar;
                break;
        }
        ++itrData;
    }
    strStringValue += "\"";

    return strStringValue;
}

/********************************************************************************************
 *
 * @function UnEscapeJsonString
 * @brief Unescape json string
 * @param[in] c_strData json string
 * @return unescaped string
 *
 ********************************************************************************************/

std::string __stdcall UnEscapeJsonString(
    _in const std::string & c_strData
    )
{
    __DebugFunction();

    std::string strStringValue;
    bool fEscaped = false;

    std::string::const_iterator itrData = c_strData.begin();

    while (c_strData.end() != itrData)
    {
        char szCurrentChar = *itrData;

        // Unescape charaters
        if (true == fEscaped)
        {
            switch(szCurrentChar)
            {
                case '"'
                :
                    strStringValue += '"';
                    break;
                case '\\'
                :
                    strStringValue += '\\';
                    break;
                case '/'
                :
                    strStringValue += '/';
                    break;
                case 'b'
                :
                    strStringValue += '\b';
                    break;
                case 'n'
                :
                    strStringValue += '\n';
                    break;
                case 'r'
                :
                    strStringValue += '\r';
                    break;
                case 't'
                :
                    strStringValue += '\t';
                    break;
                case 'f'
                :
                    strStringValue += '\f';
                    break;
                default
                :
                    strStringValue += szCurrentChar;
                    break;
            }

            fEscaped = false;
        }
        else
        {
            if ('\\' == szCurrentChar)
            {
                fEscaped = true;
            }
            else
            {
                strStringValue += szCurrentChar;
            }
        }

        ++itrData;
    }

    return strStringValue;
}

/********************************************************************************************
 *
 * @function Base64Encode
 * @brief Encode string into Base64
 * @param[in] c_strData string to encoded
 * @return Base64 encoded string
 *
 ********************************************************************************************/

std::string Base64Encode(
    _in const std::string & c_strInputString
    )
{
    __DebugFunction();

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

    unsigned int unInputStringLength = c_strInputString.size();
    unsigned int unOutStringLength = 4 * ((unInputStringLength + 2) / 3);
    std::string strEncodedString(unOutStringLength, '\0');
    char * p_szEncodedString = const_cast<char*>(strEncodedString.c_str());

    unsigned int unIndex = 0;
    for (; unIndex < unInputStringLength - 2; unIndex += 3)
    {
      *p_szEncodedString++ = aszEncodingTable[(c_strInputString[unIndex] >> 2) & 0x3F];
      *p_szEncodedString++ = aszEncodingTable[((c_strInputString[unIndex] & 0x3) << 4) | ((int) (c_strInputString[unIndex + 1] & 0xF0) >> 4)];
      *p_szEncodedString++ = aszEncodingTable[((c_strInputString[unIndex + 1] & 0xF) << 2) | ((int) (c_strInputString[unIndex + 2] & 0xC0) >> 6)];
      *p_szEncodedString++ = aszEncodingTable[c_strInputString[unIndex + 2] & 0x3F];
    }
    if (unInputStringLength > unIndex)
    {
      *p_szEncodedString++ = aszEncodingTable[(c_strInputString[unIndex] >> 2) & 0x3F];
      if ((unInputStringLength - 1) == unIndex)
      {
        *p_szEncodedString++ = aszEncodingTable[((c_strInputString[unIndex] & 0x3) << 4)];
        *p_szEncodedString++ = '=';
      }
      else
      {
        *p_szEncodedString++ = aszEncodingTable[((c_strInputString[unIndex] & 0x3) << 4) | ((int) (c_strInputString[unIndex + 1] & 0xF0) >> 4)];
        *p_szEncodedString++ = aszEncodingTable[((c_strInputString[unIndex + 1] & 0xF) << 2)];
      }
      *p_szEncodedString++ = '=';
    }

    return strEncodedString;
}

/********************************************************************************************
 *
 * @function Base64Decode
 * @brief Decode Base64 string
 * @param[in] c_strData Base64 encoded string to decode
 * @return Decoded string
 *
 ********************************************************************************************/

std::string Base64Decode(
    _in const std::string & c_strInputString
    )
{
    __DebugFunction();

    std::string strDecodedString;

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

    unsigned int unInputStringLength = c_strInputString.size();

    _ThrowBaseExceptionIf((0 != (unInputStringLength % 4)), "Error: Invalid encoded string.", nullptr);

    unsigned int unOutputStringLength = (unInputStringLength / 4) * 3;
    if ('=' == c_strInputString[unInputStringLength - 1])
    {
        unOutputStringLength--;
    }
    if ('=' == c_strInputString[unInputStringLength - 2])
    {
        unOutputStringLength--;
    }

    strDecodedString.resize(unOutputStringLength);

    for (unsigned int unCurrentCharacter = 0, unNextCharacter = 0; unCurrentCharacter < unInputStringLength;)
    {
        uint32_t unFirstSextet = c_strInputString[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_strInputString[unCurrentCharacter++])];
        uint32_t unSecondSextet = c_strInputString[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_strInputString[unCurrentCharacter++])];
        uint32_t unThirdSextet = c_strInputString[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_strInputString[unCurrentCharacter++])];
        uint32_t unFourthSextet = c_strInputString[unCurrentCharacter] == '=' ? 0 & unCurrentCharacter++ : aszDecodingTable[static_cast<int>(c_strInputString[unCurrentCharacter++])];

        uint32_t unSextets = (unFirstSextet << 3 * 6) + (unSecondSextet << 2 * 6) + (unThirdSextet << 1 * 6) + (unFourthSextet << 0 * 6);

        if (unNextCharacter < unOutputStringLength)
        {
            strDecodedString[unNextCharacter++] = (unSextets >> 2 * 8) & 0xFF;
        }
        if (unNextCharacter < unOutputStringLength)
        {
            strDecodedString[unNextCharacter++] = (unSextets >> 1 * 8) & 0xFF;
        }
        if (unNextCharacter < unOutputStringLength)
        {
            strDecodedString[unNextCharacter++] = (unSextets >> 0 * 8) & 0xFF;
        }
    }

    return strDecodedString;
}
