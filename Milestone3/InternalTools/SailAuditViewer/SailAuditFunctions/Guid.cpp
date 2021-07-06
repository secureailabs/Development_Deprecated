/*********************************************************************************************
 *
 * @file Guid.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the Guid class that handles 128 bit (16 bytes) GUID/UUID values.
 *
 ********************************************************************************************/

#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "Guid.h"

#include <stdio.h>
#include <string.h>

#pragma comment(lib, "rpcrt4.lib")
#include <windows.h>
#include <iostream>
#include <guiddef.h>
#include <rpcdce.h>

#define uuid_t UUID

// 0         1         2         3
// 01234567890123456789012345678901234567
// 6E574DA3068843FD9690B5E15DE11402 --------> raw
// 6E574DA3-0688-43FD-9690-B5E15DE11402 ----> with hyphens
// {6E574DA3-0688-43FD-9690-B5E15DE11402} --> with hyphens and braces
static const unsigned int gsc_aunStringCharacterIndexes[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 14, 15, 16, 17, 19, 20, 21, 22, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36
};

// Static lookup array used for ultra fast heaxadecimal translation
static const Byte gsc_abFastHexadecimalTranslation[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

/********************************************************************************************
 *
 * @class Guid
 * @function Guid
 * @brief Default constructor
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID buffer
 * @note
 *    All new Guid objects automatically get a new GUID generated on instantiation.
 *
 ********************************************************************************************/

Guid::Guid(void)
{
    __DebugFunction();

    UUID oUniqueIdentifier;
    UuidCreate(&oUniqueIdentifier);
    m_stlRawData.resize(16);
    ::memcpy((void *) m_stlRawData.data(), (void *) &oUniqueIdentifier, 16);
}

/********************************************************************************************
 *
 * @class Guid
 * @function Guid
 * @brief Constructor which takes input from a incoming byte representing an object type and replaces
 * 4 MSBs with it
 * @param[in] bObjectType Byte representation of an object type
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 * @par
 *    The Guid() constructor accepts the following object types:
 *    (1) 0000 --> Organizatiom
 *    (2) 0001 --> User
 *    (3) 0010 --> Cryptographic Key
 *    (4) 0011 --> Digital Contract
 *    (5) 0100 --> Dataset
 *    (6) 0101 --> Function Node
 *    (7) 0110 --> Virtual Machine
 *    (8) 0111 --> Audit Event (Branch Node)
 *    (9) 1000 --> Audit Event (Encrypted Leaf Node)
 *    (10) 10001 --> Audit Evet (Plain-text Leaf Node)
 *    (2) 1111 --> Others
 *
 ********************************************************************************************/

Guid::Guid(
    _in GuidOfObjectType eObjectType
    )
{
    __DebugFunction();

    uuid_t oUniqueIdentifier;
    UuidCreate(&oUniqueIdentifier);
    m_stlRawData.resize(16);
    ::memcpy((void *) m_stlRawData.data(), (void *) &oUniqueIdentifier, 16);
    // Change Most significant 4 bits to eObjectType
    m_stlRawData[0] &= ~(eObjectType << 4);
    m_stlRawData[0] |= (eObjectType << 4);
    m_eObjectType = eObjectType;
}

/********************************************************************************************
 *
 * @class Guid
 * @function Guid
 * @brief Constructor which takes input from a incoming string representation of a GUID/UUID
 * @param[in] c_szGuid String representation of a GUID/UUID
 * @throw BaseException Invalid or improperly formatted @p c_szGuid parameter value
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 * @note
 *    This constructor will parse the incoming string and initialize itself to that value.
 * @par
 *    The Guid() constructor accepts three kinds of string formats for guid:
 *    (1) {6E574DA3-0688-43FD-9690-B5E15DE11402} --> hyphens and braces
 *    (2) 6E574DA3-0688-43FD-9690-B5E15DE11402 ----> hyphens
 *    (3) 6E574DA3068843FD9690B5E15DE11402 --------> raw
 *    Any other format will cause the constructor to throw an exception
 *
 ********************************************************************************************/

Guid::Guid(
    _in const char * c_szGuid
    )
{
    __DebugFunction();

    this->InitializeFromString(c_szGuid);
}

/********************************************************************************************
 *
 * @class Guid
 * @function Guid
 * @brief Constructor which takes input from a incoming 16 byte buffer containing a raw GUID/UUID
 * @param[in] c_pbBinaryBuffer 16 byte buffer containing a raw GUID/UUID
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 * @note This instance of the Guid object will be initialized with the GUID/UUID value contained
 * in the first 16 bytes within the buffer pointed to by @p c_pbBinaryBuffer
 *
 ********************************************************************************************/

Guid::Guid(
    _in const Byte * c_pbBinaryBuffer
    )
{
    __DebugFunction();

    m_stlRawData.resize(16);
    if (nullptr != c_pbBinaryBuffer)
    {
        ::memcpy((void *) m_stlRawData.data(), (void *) c_pbBinaryBuffer, 16);
    }
    else
    {
        ::memset((void *) m_stlRawData.data(), 0, 16);
    }
}

/********************************************************************************************
 *
 * @class Guid
 * @function Guid
 * @brief Default copy constructor
 * @param[in] c_oGuid Instance of a Guid object to copy from
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 *
 ********************************************************************************************/

Guid::Guid(
    _in const Guid & c_oGuid
    )
{
    __DebugFunction();
    __DebugAssert(16 == c_oGuid.m_stlRawData.size());

    m_stlRawData = c_oGuid.m_stlRawData;
}

/********************************************************************************************
 *
 * @class Guid
 * @function ~Guid
 * @brief Default destructor
 *
 ********************************************************************************************/

Guid::~Guid(void) throw()
{
    __DebugFunction();

    m_stlRawData.clear();
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator =
 * @brief Assignment operator = override
 * @param[in] c_szGuid String representation of a GUID/UUID
 * @throw BaseException Invalid or improperly formatted @p c_szGuid parameter value
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 * @return Guid object
 *
 ********************************************************************************************/

Guid & __thiscall Guid::operator = (
    _in const char * c_szGuid
    )
{
    __DebugFunction();

    this->InitializeFromString(c_szGuid);

    return *this;
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator =
 * @brief Assignment operator = override
 * @param[in] c_oGuid Instance of a Guid object to copy from
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 * @return Guid object
 *
 ********************************************************************************************/

Guid & __thiscall Guid::operator = (
    _in const Guid & c_oGuid
    )
{
    __DebugFunction();
    __DebugAssert(16 == c_oGuid.m_stlRawData.size());

    m_stlRawData = c_oGuid.m_stlRawData;

    return *this;
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator ==
 * @brief Comparison operator == override
 * @param[in] c_szGuid String representation of a GUID/UUID
 * @return true if equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall Guid::operator == (
    _in const char * c_szGuid
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());

    bool fIsEqual = false;

    try
    {
        Guid oGuidToCompareTo(c_szGuid);

        fIsEqual = (m_stlRawData == oGuidToCompareTo.m_stlRawData);
    }

    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fIsEqual;
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator ==
 * @brief Comparison operator == override
 * @param[in] c_oGuid Instance of a Guid object to compare with
 * @return true if equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall Guid::operator == (
    _in const Guid & c_oGuid
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());
    __DebugAssert(16 == c_oGuid.m_stlRawData.size());

    return (m_stlRawData == c_oGuid.m_stlRawData);
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator !=
 * @brief Comparison operator != override
 * @param[in] c_szGuid String representation of a GUID/UUID
 * @return true if not equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall Guid::operator != (
    _in const char * c_szGuid
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());

    bool fIsDifferent = false;

    try
    {
        Guid oGuidToCompareTo(c_szGuid);

        fIsDifferent = (m_stlRawData != oGuidToCompareTo.m_stlRawData);
    }

    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fIsDifferent;
}

/********************************************************************************************
 *
 * @class Guid
 * @function operator !=
 * @brief Comparison operator != override
 * @param[in] c_oGuid Instance of a Guid object to compare with
 * @return true if not equal
 * @return false otherwise
 *
 ********************************************************************************************/

bool __thiscall Guid::operator != (
    _in const Guid & c_oGuid
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());
    __DebugAssert(16 == c_oGuid.m_stlRawData.size());

    return (m_stlRawData != c_oGuid.m_stlRawData);
}

/********************************************************************************************
 *
 * @class Guid
 * @function GetRawDataPtr
 * @brief Returns a const Byte * to the first byte in the raw data buffer containing the GUID/UUID
 * @return Contant byte pointer to the first byte in the raw data buffer containing the GUID/UUID
 *
 ********************************************************************************************/

const Byte * __thiscall Guid::GetRawDataPtr(void) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());

    return (const Byte *) m_stlRawData.data();
}

/********************************************************************************************
 *
 * @class Guid
 * @function GetRawData
 * @brief Returns a buffer containing the raw bytes representing the GUID/UUID
 * @return A buffer containing the raw bytes representing the GUID/UUID
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall Guid::GetRawData(void) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());

    return m_stlRawData;
}

/********************************************************************************************
 *
 * @class Guid
 * @function ToString
 * @brief Returns a string representation of the GUID/UUID
 * @param[in] eGuidFormat Which GUID/UUID format to use in the string formatting (eHyphensOnly, eHyphensAndCurlyBraces, eRaw)
 * @return A string representation of the GUID/UUID
 * @note
 *    The string formatting can be one of:
 *    1. eRaw --> E574DA3068843FD9690B5E15DE11402
 *    2. eHyphensOnly --> E574DA3-0688-43FD-9690-B5E15DE11402
 *    3. eHyphensAndCurlyBraces --> {6E574DA3-0688-43FD-9690-B5E15DE11402}
 *
 ********************************************************************************************/

std::string __thiscall Guid::ToString(
    _in GuidFormat eGuidFormat
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(16 == m_stlRawData.size());

    char szGuid[39];

    switch(eGuidFormat)
    {
        case eHyphensOnly               // 6E574DA3-0688-43FD-9690-B5E15DE11402 ----> with hyphens
        :   ::snprintf(szGuid, sizeof(szGuid), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", (unsigned int) m_stlRawData[0], (unsigned int) m_stlRawData[1], (unsigned int) m_stlRawData[2], (unsigned int) m_stlRawData[3], (unsigned int) m_stlRawData[4], (unsigned int) m_stlRawData[5], (unsigned int) m_stlRawData[6], (unsigned int) m_stlRawData[7], (unsigned int) m_stlRawData[8], (unsigned int) m_stlRawData[9], (unsigned int) m_stlRawData[10], (unsigned int) m_stlRawData[11], (unsigned int) m_stlRawData[12], (unsigned int) m_stlRawData[13], (unsigned int) m_stlRawData[14], (unsigned int) m_stlRawData[15]);
            break;
        case eHyphensAndCurlyBraces     // {6E574DA3-0688-43FD-9690-B5E15DE11402} --> with hyphens and braces
        :   ::snprintf(szGuid, sizeof(szGuid), "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}", (unsigned int) m_stlRawData[0], (unsigned int) m_stlRawData[1], (unsigned int) m_stlRawData[2], (unsigned int) m_stlRawData[3], (unsigned int) m_stlRawData[4], (unsigned int) m_stlRawData[5], (unsigned int) m_stlRawData[6], (unsigned int) m_stlRawData[7], (unsigned int) m_stlRawData[8], (unsigned int) m_stlRawData[9], (unsigned int) m_stlRawData[10], (unsigned int) m_stlRawData[11], (unsigned int) m_stlRawData[12], (unsigned int) m_stlRawData[13], (unsigned int) m_stlRawData[14], (unsigned int) m_stlRawData[15]);
            break;
        default                         // 6E574DA3068843FD9690B5E15DE11402 --------> raw
        :   ::snprintf(szGuid, sizeof(szGuid), "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", (unsigned int) m_stlRawData[0], (unsigned int) m_stlRawData[1], (unsigned int) m_stlRawData[2], (unsigned int) m_stlRawData[3], (unsigned int) m_stlRawData[4], (unsigned int) m_stlRawData[5], (unsigned int) m_stlRawData[6], (unsigned int) m_stlRawData[7], (unsigned int) m_stlRawData[8], (unsigned int) m_stlRawData[9], (unsigned int) m_stlRawData[10], (unsigned int) m_stlRawData[11], (unsigned int) m_stlRawData[12], (unsigned int) m_stlRawData[13], (unsigned int) m_stlRawData[14], (unsigned int) m_stlRawData[15]);
            break;
    };

    return std::string(szGuid);
}

/********************************************************************************************
 *
 * @class Guid
 * @function GetObjectType
 * @brief Method used to fetch type of object represented by the 4 MSBs of the Guid
 * @return Work representing type of object represented by the 4 MSBs of the Guid
 *
 ********************************************************************************************/

GuidOfObjectType __thiscall Guid::GetObjectType(void) const throw()
{
    __DebugFunction();

    return m_eObjectType;
}

/********************************************************************************************
 *
 * @class Guid
 * @function InitializeFromString
 * @brief Method used to initialize the internal GUID/UUID 16 byte buffer with the value represented with an incoming formatted @p c_szGuid string
 * @param[in] c_szGuid Formatted GUID/UUID string
 * @throw BaseException Invalid or improperly formatted @p c_szGuid parameter value
 * @throw bad_alloc If there isn't enough memory left to allocate the internal GUID/UUID buffer
 *
 ********************************************************************************************/

void __thiscall Guid::InitializeFromString(
    _in const char * c_szGuid
    )
{
    __DebugFunction();

    m_stlRawData.resize(16);
    if (nullptr == c_szGuid)
    {
        // If the incoming string is nullptr, then the GUID/UUID is initialized to all 0's
        ::memset((void *) m_stlRawData.data(), 0, 16);
    }
    else
    {
        unsigned int unStartingIndex = 0;
        unsigned int unSizeInCharactersOfGuidString = (unsigned int) ::strnlen(c_szGuid, 39);

        if ('{' == c_szGuid[0])
        {
            if ((38 == unSizeInCharactersOfGuidString)&&('-' == c_szGuid[9])&&('-' == c_szGuid[14])&&('-' == c_szGuid[19])&&('-' == c_szGuid[24])&&('}' == c_szGuid[37]))
            {
                unStartingIndex = 64;
            }
            else
            {
                _ThrowSimpleException("INVALID FORMAT. Invalid format provided in incoming string.");
            }
        }
        else if ('-' == c_szGuid[8])
        {
            if ((36 == unSizeInCharactersOfGuidString)&&('-' == c_szGuid[13])&&('-' == c_szGuid[18])&&('-' == c_szGuid[23]))
            {
                unStartingIndex = 32;
            }
            else
            {
                _ThrowSimpleException("INVALID FORMAT. Invalid format provided in incoming string.");
            }
        }
        else if (32 != unSizeInCharactersOfGuidString)
        {
            _ThrowSimpleException("INVALID FORMAT. Invalid format provided in incoming string.");
        }

        unsigned int unMovingIndex = 0;
        while (32 > unMovingIndex)
        {
            Byte bHighCharacter = gsc_abFastHexadecimalTranslation[(unsigned int) c_szGuid[gsc_aunStringCharacterIndexes[unStartingIndex + unMovingIndex]]];
            Byte bLowCharacter = gsc_abFastHexadecimalTranslation[(unsigned int) c_szGuid[gsc_aunStringCharacterIndexes[unStartingIndex + unMovingIndex + 1]]];

            _ThrowBaseExceptionIf(((255 == bHighCharacter)||(255 == bLowCharacter)), "Illegal character found in UUID string.", nullptr);

            m_stlRawData[unMovingIndex / 2] = (bHighCharacter << 4) | bLowCharacter;
            unMovingIndex += 2;
        }
    }
}
