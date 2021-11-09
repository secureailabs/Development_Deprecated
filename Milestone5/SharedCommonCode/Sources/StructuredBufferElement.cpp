/*********************************************************************************************
 *
 * @file StructuredBufferElement.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "StructuredBuffer.h"
#include "64BitHashes.h"

#include <string.h>

#include <iostream>

/********************************************************************************************/

StructuredBufferElement::StructuredBufferElement(
    _in const char * szName,
    _in Byte bElementType,
    _in const Byte * c_pbElementData,
    _in unsigned int unElementDataSizeInBytes
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != szName);
    
    m_strElementName = szName;
    m_qwElementName64BitHash = ::Get64BitHashOfNullTerminatedString(szName, false);
    m_bElementType = bElementType;
    if ((nullptr != c_pbElementData)&&(0 < unElementDataSizeInBytes))
    {
        m_stlElementData.assign(c_pbElementData, c_pbElementData + unElementDataSizeInBytes);
    }
    
    this->Serialize();
    
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 < m_stlSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBufferElement::StructuredBufferElement(
    _in const Byte * c_pbSerializedRawData,
    _in unsigned int unSerializedRawDataSizeInBytes
    )
    : m_bElementType(0xFF), m_qwElementName64BitHash(0)
{
    __DebugFunction();

    this->DeSerialize(c_pbSerializedRawData, unSerializedRawDataSizeInBytes);
        
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 < m_stlSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBufferElement::StructuredBufferElement(
    _in const std::vector<Byte> c_stlSerializedRawData
    )
    : m_bElementType(0xFF), m_qwElementName64BitHash(0)
{
    __DebugFunction();

    this->DeSerialize((const Byte *) c_stlSerializedRawData.data(), (unsigned int) c_stlSerializedRawData.size());
    
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 < m_stlSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBufferElement::StructuredBufferElement(
    _in StructuredBufferElement & oStructuredBufferElement
    )
{
    __DebugFunction();
    __DebugAssert(0 < oStructuredBufferElement.m_strElementName.length());
    __DebugAssert(0 != oStructuredBufferElement.m_qwElementName64BitHash);
    __DebugAssert(0xFF != oStructuredBufferElement.m_bElementType);
    __DebugAssert(0 < oStructuredBufferElement.m_stlSerializedBuffer.size());
    
    m_strElementName = oStructuredBufferElement.m_strElementName;
    m_qwElementName64BitHash = oStructuredBufferElement.m_qwElementName64BitHash;
    m_bElementType = oStructuredBufferElement.m_bElementType;
    m_stlElementData = oStructuredBufferElement.m_stlElementData;
    m_stlSerializedBuffer = oStructuredBufferElement.m_stlSerializedBuffer;
    
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 < m_stlSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBufferElement::StructuredBufferElement(
    _in const StructuredBufferElement & c_oStructuredBufferElement
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_oStructuredBufferElement.m_strElementName.length());
    __DebugAssert(0 != c_oStructuredBufferElement.m_qwElementName64BitHash);
    __DebugAssert(0xFF != c_oStructuredBufferElement.m_bElementType);
    __DebugAssert(0 < c_oStructuredBufferElement.m_stlSerializedBuffer.size());
    
    m_strElementName = c_oStructuredBufferElement.m_strElementName;
    m_qwElementName64BitHash = c_oStructuredBufferElement.m_qwElementName64BitHash;
    m_bElementType = c_oStructuredBufferElement.m_bElementType;
    m_stlElementData = c_oStructuredBufferElement.m_stlElementData;
    m_stlSerializedBuffer = c_oStructuredBufferElement.m_stlSerializedBuffer;
    
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 < m_stlSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBufferElement::~StructuredBufferElement(void)
{
    __DebugFunction();

    m_strElementName.clear();
    m_bElementType = 0xFF;
    m_qwElementName64BitHash = 0;
    m_stlElementData.clear();
    m_stlSerializedBuffer.clear();
}

/********************************************************************************************/

std::string __thiscall StructuredBufferElement::GetElementName(void) const throw()
{
    __DebugFunction();
    //__DebugAssert(0 < m_strElementName.length());
    
    return m_strElementName;
}

/********************************************************************************************/

Qword __thiscall StructuredBufferElement::GetElementName64BitHash(void) const throw()
{
    __DebugFunction();
    //__DebugAssert(0 != m_qwElementName64BitHash);
    
    return m_qwElementName64BitHash;
}

/********************************************************************************************/

Byte __thiscall StructuredBufferElement::GetElementType(void) const throw()
{
    __DebugFunction();
    
    return m_bElementType;
}

/********************************************************************************************/

const Byte * __thiscall StructuredBufferElement::GetRawDataPtr(void) const throw()
{
    __DebugFunction();
    
    return (const Byte *) m_stlElementData.data();
}

/********************************************************************************************/

unsigned int __thiscall StructuredBufferElement::GetRawDataSizeInBytes(void) const throw()
{
    __DebugFunction();
    
    return (unsigned int) m_stlElementData.size();
}

/********************************************************************************************/

const Byte * __thiscall StructuredBufferElement::GetSerializedBufferRawDataPtr(void) const throw()
{
    __DebugFunction();
    //__DebugAssert(0 < m_stlSerializedBuffer.size());
    
    return (const Byte *) m_stlSerializedBuffer.data();
}

/********************************************************************************************/

unsigned int __thiscall StructuredBufferElement::GetSerializedBufferRawDataSizeInBytes(void) const throw()
{
    __DebugFunction();
    //__DebugAssert(0 < m_stlSerializedBuffer.size());
    
    return (unsigned int) m_stlSerializedBuffer.size();
}
        
/********************************************************************************************/

std::vector<Byte>  __thiscall StructuredBufferElement::GetSerializedBuffer(void) const throw()
{
    __DebugFunction();
    //__DebugAssert(0 < m_stlSerializedBuffer.size());
    
    return m_stlSerializedBuffer;
}

/********************************************************************************************/

void __thiscall StructuredBufferElement::Serialize(void) const throw()
{
    __DebugFunction();
    __DebugAssert(0 < m_strElementName.length());
    __DebugAssert(0 != m_qwElementName64BitHash);
    __DebugAssert(0xFF != m_bElementType);
    __DebugAssert(0 == m_stlSerializedBuffer.size());
    
    // The format of the serialized data is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x54FB0134                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Byte] Type                                                                        |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfName including null terminating character                  |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInByteOfElementData                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x010F020D                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfName] Name String                                                    |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x030C040B                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [SizeInByteOfElementData] Element Data                                             |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x3410BF45                                                                 |
    // +------------------------------------------------------------------------------------+
    
    unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(Byte) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(Dword) + m_strElementName.length() + 1 + sizeof(Dword) + m_stlElementData.size() + sizeof(Dword);
    // First we need to get a sorted vector of elements
    m_stlSerializedBuffer.resize(unSerializedBufferSizeInBytes);
    Byte * pbSerializedBuffer = (Byte *) m_stlSerializedBuffer.data();
    
    *((Dword *) pbSerializedBuffer) = 0x54FB0134;
    pbSerializedBuffer += sizeof(Dword);
    *((Byte *) pbSerializedBuffer) = m_bElementType;
    pbSerializedBuffer += sizeof(Byte);
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) m_strElementName.length() + 1;
    pbSerializedBuffer += sizeof(uint32_t);
    *((uint32_t *) pbSerializedBuffer) = (uint32_t) m_stlElementData.size();
    pbSerializedBuffer += sizeof(uint32_t);
    *((Dword *) pbSerializedBuffer) = 0x010F020D;
    pbSerializedBuffer += sizeof(Dword);
    ::memcpy((void *) pbSerializedBuffer, (const void *) m_strElementName.c_str(), m_strElementName.length());
    pbSerializedBuffer += m_strElementName.length();
    *((Byte *) pbSerializedBuffer) = 0; // Ensure the string is in fact null terminated no matter what
    pbSerializedBuffer += 1;
    *((Dword *) pbSerializedBuffer) = 0x030C040B;
    pbSerializedBuffer += sizeof(Dword);
    if (0 < m_stlElementData.size())
    {
        ::memcpy((void *) pbSerializedBuffer, (const void *) m_stlElementData.data(), m_stlElementData.size());
        pbSerializedBuffer += m_stlElementData.size();
    }
    *((Dword *) pbSerializedBuffer) = 0x3410BF45;
}

/********************************************************************************************/

void __thiscall StructuredBufferElement::DeSerialize(
    _in const Byte * c_pbSerializedData,
    _in unsigned int unSerializedDataSizeInBytes
    )
{
    __DebugFunction();
    __DebugAssert(0 == m_strElementName.length());
    __DebugAssert(0xFF == m_bElementType);
    __DebugAssert(0  == m_qwElementName64BitHash);
    __DebugAssert(0 == m_stlElementData.size());
    __DebugAssert(0 == m_stlSerializedBuffer.size());
    
    _ThrowBaseExceptionIf(((nullptr == c_pbSerializedData)||(0 == unSerializedDataSizeInBytes)), "Invalid parameters passed in", nullptr);
    
    // The format of the serialized data is:
    //
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x54FB0134                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Byte] Type                                                                        |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInBytesOfName including null terminating character                  |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] SizeInByteOfElementData                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x010F020D                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [SizeInBytesOfName] Name String                                                    |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x030C040B                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [SizeInByteOfElementData] Element Data                                             |
    // +------------------------------------------------------------------------------------+
    // | [Dword] 0x3410BF45                                                                 |
    // +------------------------------------------------------------------------------------+

    const Byte * pbCurrentByte = c_pbSerializedData;
    _ThrowBaseExceptionIf((0x54FB0134 != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [HEADER] = 0x54FB0134 but got 0x%08X", *((Dword *) pbCurrentByte));
    pbCurrentByte += sizeof(Dword);
    m_bElementType = *((Byte *) pbCurrentByte);
    pbCurrentByte += sizeof(Byte);
    unsigned int unElementNameSizeInBytesIncludingNull = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    unsigned int unElementDataSizeInBytes = *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    _ThrowBaseExceptionIf((0x010F020D != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [SEPARATPE] = 0x010F020D but got 0x%08X", *((Dword *) pbCurrentByte));
    pbCurrentByte += sizeof(Dword);
    m_strElementName = (const char *) pbCurrentByte;
    pbCurrentByte += unElementNameSizeInBytesIncludingNull;
    _ThrowBaseExceptionIf((0x030C040B != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [SEPARATPE] = 0x030C040B but got 0x%08X", *((Dword *) pbCurrentByte));
    pbCurrentByte += sizeof(Dword);
    if (0 < unElementDataSizeInBytes)
    {
        m_stlElementData.resize(unElementDataSizeInBytes);
        ::memcpy((void *) m_stlElementData.data(), (const void *) pbCurrentByte, unElementDataSizeInBytes);
        pbCurrentByte += unElementDataSizeInBytes;
    }
    _ThrowBaseExceptionIf((0x3410BF45 != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [FOOTER] = 0x3410BF45 but got 0x%08X", *((Dword *) pbCurrentByte));
    
    m_qwElementName64BitHash = ::Get64BitHashOfNullTerminatedString(m_strElementName.c_str(), false);
    m_stlSerializedBuffer.assign(c_pbSerializedData, c_pbSerializedData + unSerializedDataSizeInBytes);
}
    
