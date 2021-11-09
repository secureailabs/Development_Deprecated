/*********************************************************************************************
 *
 * @file StructuredBuffer.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "64BitHashes.h"
#include "Base64Encoder.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "StructuredBuffer.h"

#include <algorithm>
#include <iostream>
#include <string.h>
#include <vector>

static const char * gs_aszTypeNames[] =
{
    "Null",
    "Bool",
    "Char",
    "String",
    "float32_t",
    "float64_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint8_t",
    "Byte",
    "Word",
    "Dword",
    "Qword",
    "ByteArray",
    "StructuredBuffer",
    "Null",
    "Guid",
    "InvalidType"
};

/********************************************************************************************/

StructuredBuffer::StructuredBuffer(void)
{
    __DebugFunction();
    
    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
}

/********************************************************************************************/

StructuredBuffer::StructuredBuffer(
    _in const Byte * c_pbSerializedBuffer,
    _in unsigned int unBufferSizeInBytes
    )
{
    __DebugFunction();

    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
    this->DeSerialize(c_pbSerializedBuffer, unBufferSizeInBytes);
}

/********************************************************************************************/

StructuredBuffer::StructuredBuffer(
    _in const std::vector<Byte> & c_oSerializedBuffer
    )
{
    __DebugFunction();

    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
    this->DeSerialize(c_oSerializedBuffer.data(), (unsigned int) c_oSerializedBuffer.size());
}

/********************************************************************************************/

StructuredBuffer::StructuredBuffer(
    _in const char * c_szBase64SerializedBuffer
    )
{
    __DebugFunction();
    
    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
    
    this->DeSerialize(c_szBase64SerializedBuffer);
}

/********************************************************************************************/

StructuredBuffer::StructuredBuffer(
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();

    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
    this->DeSerialize(c_oStructuredBuffer.GetSerializedBufferRawDataPtr(), c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
}

/********************************************************************************************/

StructuredBuffer::~StructuredBuffer(void) throw()
{
    __DebugFunction();
    
    this->Clear();
}

/********************************************************************************************/

void __thiscall StructuredBuffer::Clear(void) throw()
{
    __DebugFunction();

    for (auto const & element : m_stlMapOfElements)
    {
        (element.second)->Release();
    }    
    m_stlMapOfElements.clear();
    m_stlSerializedBuffer.clear();
    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
}

/********************************************************************************************/

const Byte * __thiscall StructuredBuffer::GetSerializedBufferRawDataPtr(void) const throw()
{
    __DebugFunction();
    
    this->Serialize();
    
    return (const Byte *) m_stlSerializedBuffer.data();
}

/********************************************************************************************/

unsigned int __thiscall StructuredBuffer::GetSerializedBufferRawDataSizeInBytes(void) const throw()
{
    __DebugFunction();
    
    this->Serialize();
    
    return (unsigned int) m_stlSerializedBuffer.size();
}

/********************************************************************************************/

std::vector<Byte> __thiscall StructuredBuffer::GetSerializedBuffer(void) const throw()
{
    __DebugFunction();

    this->Serialize();
    
    return m_stlSerializedBuffer;
}

/********************************************************************************************/

std::string __thiscall StructuredBuffer::GetBase64SerializedBuffer(void) const throw()
{
    __DebugFunction();

    this->Serialize();
    
    return ::Base64Encode(m_stlSerializedBuffer.data(), (unsigned int) m_stlSerializedBuffer.size());
}

/********************************************************************************************/

Qword __thiscall StructuredBuffer::Get64BitHash(void) const throw()
{
    __DebugFunction();
    
    this->Serialize();
    
    return m_qw64BitHash;
}

/********************************************************************************************/

Qword __thiscall StructuredBuffer::GetComposition64BitHash(void) const throw()
{
    __DebugFunction();
    
    this->Serialize();
    
    return m_qwComposition64BitHash;
}

/********************************************************************************************/
//
// NOTE: There is a big difference between IsElementPresent() and CheckForElement() insofar as
// IsElementPresent() throws no exception if a matching element is not found while
// CheckForElement() specifically throws an exception if no matching element was found.

bool __thiscall StructuredBuffer::IsElementPresent(
    _in const char * c_szElementName,
    _in Byte bExpectedType
    ) const throw()
{
    __DebugFunction();

    bool fFound = false;
    Qword qwElementName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szElementName, false);
    // First we check to see if an element by the same name exists
    if (m_stlMapOfElements.end() != m_stlMapOfElements.find(qwElementName64BitHash))
    {
        // Now we need to check if the element type matches
        if (bExpectedType == m_stlMapOfElements.at(qwElementName64BitHash)->GetElementType())
        {
            fFound = true;
        }
    }
    
    return fFound;
}

/********************************************************************************************/

void __thiscall StructuredBuffer::RemoveElement(
    _in const char * c_szElementName
    ) throw()
{
    __DebugFunction();

    this->RemoveElement(::Get64BitHashOfNullTerminatedString(c_szElementName, false));
}

/********************************************************************************************/

std::vector<std::string> __thiscall StructuredBuffer::GetNamesOfElements(void) const throw()
{
    __DebugFunction();

    std::vector<std::string> stlListOfElements;
    
    for (auto const & element : m_stlMapOfElements)
    {
        stlListOfElements.push_back(element.second->GetElementName());
    }
    
    return stlListOfElements;
}

/********************************************************************************************/

std::vector<std::string> __thiscall StructuredBuffer::GetDescriptionOfElements(void) const throw()
{
    __DebugFunction();

    std::vector<std::string> stlListOfElements;
    
    for (auto const & element : m_stlMapOfElements)
    {
        stlListOfElements.push_back("Name[" + element.second->GetElementName() + "],Type[" + std::to_string((Dword) element.second->GetElementType()) + "],Size[" + std::to_string(element.second->GetRawDataSizeInBytes()) + " Bytes]");
    }
    
    return stlListOfElements;
}

/********************************************************************************************/

std::string __thiscall StructuredBuffer::ToString(void) throw()
{
    __DebugFunction();

    std::string strIndex = "";
    std::string strDestination;

    try
    {
        StructuredBuffer::ToAString(strDestination, strIndex, *this);
    }

    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return strDestination;
}

/********************************************************************************************/

void __thiscall StructuredBuffer::GetNull(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();
    
    this->CheckForElement(c_szElementName, NULL_VALUE_TYPE);
}

/********************************************************************************************/

bool __thiscall StructuredBuffer::GetBoolean(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();
    
    return (bool) *((bool *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, BOOLEAN_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

char __thiscall StructuredBuffer::GetCharacter(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();
    
    return (char) *((char *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, ANSI_CHARACTER_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

std::string __thiscall StructuredBuffer::GetString(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return std::string((const char *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, ANSI_CHARACTER_STRING_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

float32_t __thiscall StructuredBuffer::GetFloat32(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (float32_t) *((float32_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, FLOAT32_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

float64_t __thiscall StructuredBuffer::GetFloat64(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (float64_t) *((float64_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, FLOAT64_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

int8_t __thiscall StructuredBuffer::GetInt8(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (int8_t) *((int8_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, INT8_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

int16_t __thiscall StructuredBuffer::GetInt16(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (int16_t) *((int16_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, INT16_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

int32_t __thiscall StructuredBuffer::GetInt32(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (int32_t) *((int32_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, INT32_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

int64_t __thiscall StructuredBuffer::GetInt64(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (int64_t) *((int64_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, INT64_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

uint8_t __thiscall StructuredBuffer::GetUnsignedInt8(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (uint8_t) *((uint8_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, UINT8_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

uint16_t __thiscall StructuredBuffer::GetUnsignedInt16(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (uint16_t) *((uint16_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, UINT16_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

uint32_t __thiscall StructuredBuffer::GetUnsignedInt32(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (uint32_t) *((uint32_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, UINT32_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

uint64_t __thiscall StructuredBuffer::GetUnsignedInt64(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (uint64_t) *((uint64_t *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, UINT64_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

Byte __thiscall StructuredBuffer::GetByte(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (Byte) *((Byte *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, BYTE_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

Word __thiscall StructuredBuffer::GetWord(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (Word) *((Word *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, WORD_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

Dword __thiscall StructuredBuffer::GetDword(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (Dword) *((Dword *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, DWORD_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

Qword __thiscall StructuredBuffer::GetQword(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return (Qword) *((Qword *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, QWORD_VALUE_TYPE))->GetRawDataPtr());
}

/********************************************************************************************/

Guid __thiscall StructuredBuffer::GetGuid(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    return Guid((const Byte *) m_stlMapOfElements.at(this->CheckForElement(c_szElementName, GUID_VALUE_TYPE))->GetRawDataPtr());
}
    
/********************************************************************************************/

std::vector<Byte> __thiscall StructuredBuffer::GetBuffer(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    std::vector<Byte> stlBuffer;
    const StructuredBufferElement * c_poStructuredBufferElement = m_stlMapOfElements.at(this->CheckForElement(c_szElementName, BUFFER_VALUE_TYPE));
    
    stlBuffer.assign(c_poStructuredBufferElement->GetRawDataPtr(), c_poStructuredBufferElement->GetRawDataPtr() + c_poStructuredBufferElement->GetRawDataSizeInBytes());
    
    return stlBuffer;
}

/********************************************************************************************/

StructuredBuffer __thiscall StructuredBuffer::GetStructuredBuffer(
    _in const char * c_szElementName
    ) const
{
    __DebugFunction();

    const StructuredBufferElement * c_poStructuredBufferElement = m_stlMapOfElements.at(this->CheckForElement(c_szElementName, INDEXED_BUFFER_VALUE_TYPE));
    
    return StructuredBuffer((const Byte *) c_poStructuredBufferElement->GetRawDataPtr(), c_poStructuredBufferElement->GetRawDataSizeInBytes());
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutNull(
    _in const char * c_szValueName
    )
{
    __DebugFunction();

    this->AddElement(c_szValueName, NULL_VALUE_TYPE, (const void *) nullptr, 0);
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutBoolean(
    _in const char * c_szElementName,
    _in bool fElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, BOOLEAN_VALUE_TYPE, (const void *) &fElement, sizeof(fElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutCharacter(
    _in const char * c_szElementName,
    _in char chElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, ANSI_CHARACTER_VALUE_TYPE, (const void *) &chElement, sizeof(chElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutString(
    _in const char * c_szElementName,
    _in const char * c_szElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, ANSI_CHARACTER_STRING_VALUE_TYPE, (const void *) c_szElement, (unsigned int) (::strnlen(c_szElement, MAX_STRING_LENGTH) + 1));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutString(
    _in const char * c_szElementName,
    _in const std::string & c_strElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, ANSI_CHARACTER_STRING_VALUE_TYPE, (const void *) c_strElement.c_str(), (unsigned int) (c_strElement.size() + 1));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutFloat32(
    _in const char * c_szElementName,
    _in float32_t fl32Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, FLOAT32_VALUE_TYPE, (const void *) &fl32Element, sizeof(fl32Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutFloat64(
    _in const char * c_szElementName,
    _in float64_t fl64Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, FLOAT64_VALUE_TYPE, (const void *) &fl64Element, sizeof(fl64Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutInt8(
    _in const char * c_szElementName,
    _in int8_t n8Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, INT8_VALUE_TYPE, (const void *) &n8Element, sizeof(n8Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutInt16(
    _in const char * c_szElementName,
    _in int16_t n16Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, INT16_VALUE_TYPE, (const void *) &n16Element, sizeof(n16Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutInt32(
    _in const char * c_szElementName,
    _in int32_t n32Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, INT32_VALUE_TYPE, (const void *) &n32Element, sizeof(n32Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutInt64(
    _in const char * c_szElementName,
    _in int64_t n64Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, INT64_VALUE_TYPE, (const void *) &n64Element, sizeof(n64Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutUnsignedInt8(
    _in const char * c_szElementName,
    _in uint8_t un8Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, UINT8_VALUE_TYPE, (const void *) &un8Element, sizeof(un8Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutUnsignedInt16(
    _in const char * c_szElementName,
    _in uint16_t un16Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, UINT16_VALUE_TYPE, (const void *) &un16Element, sizeof(un16Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutUnsignedInt32(
    _in const char * c_szElementName,
    _in uint32_t un32Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, UINT32_VALUE_TYPE, (const void *) &un32Element, sizeof(un32Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutUnsignedInt64(
    _in const char * c_szElementName,
    _in uint64_t un64Element
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, UINT64_VALUE_TYPE, (const void *) &un64Element, sizeof(un64Element));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutByte(
    _in const char * c_szElementName,
    _in Byte bElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, BYTE_VALUE_TYPE, (const void *) &bElement, sizeof(bElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutWord(
    _in const char * c_szElementName,
    _in Word wElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, WORD_VALUE_TYPE, (const void *) &wElement, sizeof(wElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutDword(
    _in const char * c_szElementName,
    _in Dword dwElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, DWORD_VALUE_TYPE, (const void *) &dwElement, sizeof(dwElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutQword(
    _in const char * c_szElementName,
    _in Qword qwElement
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, QWORD_VALUE_TYPE, (const void *) &qwElement, sizeof(qwElement));
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutGuid(
    _in const char * c_szElementName,
    _in const Guid & c_oGuid
    )
{
    this->AddElement(c_szElementName, GUID_VALUE_TYPE, (const void *) c_oGuid.GetRawDataPtr(), 16);
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutBuffer(
    _in const char * c_szElementName,
    _in const Byte * c_pbByteArray,
    _in unsigned int unSizeInBytes
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, BUFFER_VALUE_TYPE, (const void *) c_pbByteArray, unSizeInBytes);
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutBuffer(
    _in const char * c_szElementName,
    _in const std::vector<Byte> c_stlBuffer
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, BUFFER_VALUE_TYPE, (const void *) c_stlBuffer.data(), (unsigned int) c_stlBuffer.size());
}

/********************************************************************************************/

void __thiscall StructuredBuffer::PutStructuredBuffer(
    _in const char * c_szElementName,
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();

    this->AddElement(c_szElementName, INDEXED_BUFFER_VALUE_TYPE, (const void *) c_oStructuredBuffer.GetSerializedBufferRawDataPtr(), c_oStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
}

/********************************************************************************************/

Qword __thiscall StructuredBuffer::CheckForElement(
    _in const char * c_szElementName,
    _in Byte bElementType
    ) const
{
    __DebugFunction();
    
    Qword qwElementName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szElementName, false);
    // We throw an exception in two different cases.
    // 1. No element with a matching name was found
    // 2. An element with a matching name was found, but the element type didn't match
    _ThrowBaseExceptionIf((m_stlMapOfElements.end() == m_stlMapOfElements.find(qwElementName64BitHash)), "Element %s of type %02X not found.", c_szElementName, bElementType);
    _ThrowBaseExceptionIf((bElementType != m_stlMapOfElements.at(qwElementName64BitHash)->GetElementType()), "Element %s of type %02X not found.", c_szElementName, bElementType);
    
    return qwElementName64BitHash;
}

/********************************************************************************************/

void __thiscall StructuredBuffer::AddElement(
    _in const char * c_szElementName,
    _in Byte bElementType,
    _in const void * c_pElementData,
    _in unsigned int unElementDataSizeInBytes
    )
{
    __DebugFunction();

    Qword qwElementName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szElementName, false);
    StructuredBufferElement * poNewStructuredBufferElement = new StructuredBufferElement(c_szElementName, bElementType, (const Byte *) c_pElementData, unElementDataSizeInBytes);
    
    // Always make sure to remove an existing element if it exists. There can be only one!!!
    this->RemoveElement(qwElementName64BitHash);
    // Now add the new element knowing it is unique
    m_stlMapOfElements[qwElementName64BitHash] = poNewStructuredBufferElement;
    // The cached data becomes invalid as a result
    this->DeleteCachedData();
}

/********************************************************************************************/

void __thiscall StructuredBuffer::RemoveElement(
    _in Qword qwElementName64BitHash
    ) throw()
{
    __DebugFunction();

    if (m_stlMapOfElements.end() != m_stlMapOfElements.find(qwElementName64BitHash))
    {
        m_stlMapOfElements.at(qwElementName64BitHash)->Release();
        m_stlMapOfElements.erase(qwElementName64BitHash);
        this->DeleteCachedData();
    }
}

/********************************************************************************************/

void __thiscall StructuredBuffer::Serialize(void) const throw()
{
    __DebugFunction();

    try
    {
        // We don't actually need to serialized anything if there is cached data already available
        
        if ((0 == m_qw64BitHash)||(0 == m_qwComposition64BitHash)||(0 == m_stlSerializedBuffer.size()))
        {
            unsigned int unSerializedBufferSizeInBytes = sizeof(Dword) + sizeof(Qword) + sizeof(Qword) + sizeof(uint32_t) + sizeof(Dword);
            std::vector<Qword> stlVectorOfElementName64BitHashes;
            std::map<Qword, std::vector<Byte>> stlSerializedElements;
            
            // First we need to get a sorted vector of elements
            for(auto const & element: m_stlMapOfElements)
            {
                stlVectorOfElementName64BitHashes.push_back(element.first);
            }
            std::sort(stlVectorOfElementName64BitHashes.begin(), stlVectorOfElementName64BitHashes.end());
            
            // Now we build stlSerializedElements. While we are at it, we can also compute the cached hashes.
            for (auto const & vectorElement : stlVectorOfElementName64BitHashes) 
            {
                __DebugAssert(m_stlMapOfElements.end() != m_stlMapOfElements.find(vectorElement));
                
                stlSerializedElements[vectorElement] = m_stlMapOfElements.at(vectorElement)->GetSerializedBuffer();
                
                m_qwComposition64BitHash ^= __rotl64(vectorElement, m_stlMapOfElements.at(vectorElement)->GetElementType());
                m_qw64BitHash ^= ::Get64BitHashOfByteArray((const Byte *) stlSerializedElements[vectorElement].data(), (unsigned int) stlSerializedElements[vectorElement].size());
                unSerializedBufferSizeInBytes += (unsigned int) (sizeof(Dword) + sizeof(uint32_t) + stlSerializedElements[vectorElement].size());
            }
            
            // We cannot computer the m_qw64BitHash until all of the serialized buffers
            // of individual elements have been built. As such, in this next step, we
            // serialize all of the StructuredBufferElements into a vector of vectors. The
            // vector of vectors is ordered according to stlVectorOfElementName64BitHashes
            
            m_stlSerializedBuffer.resize(unSerializedBufferSizeInBytes);
            Byte * pbSerializedBuffer = (Byte *) m_stlSerializedBuffer.data();
            __DebugAssert(nullptr != pbSerializedBuffer);
            
            // The format of the serialized data is:
            //
            // +------------------------------------------------------------------------------------+
            // | [Dword] 0xE59CABCD                                                                 |
            // +------------------------------------------------------------------------------------+
            // | [Qword] qw64BitHash                                                                |
            // +------------------------------------------------------------------------------------+
            // | [Qword] m_qwComposition64BitHash                                                   |
            // +------------------------------------------------------------------------------------+
            // | [uint32_t] Number of Elements                                                      |
            // +-----+------------------------------------------------------------------------------+
            // |     | [Dword] 0x83746383                                                           |
            // |     +------------------------------------------------------------------------------+
            // | ... | [uint32_t] un32SizeInBytesOfSerializedElement                                |
            // |     +------------------------------------------------------------------------------+
            // |     | [...] Serialized Element                                                     |
            // +-----+------------------------------------------------------------------------------+
            // | [Dword] 0xDCBAC95E                                                                 |
            // +------------------------------------------------------------------------------------+
            
            *((Dword *) pbSerializedBuffer) = 0xE59CABCD;
            pbSerializedBuffer += sizeof(Dword);
            *((Qword *) pbSerializedBuffer) = m_qw64BitHash;
            pbSerializedBuffer += sizeof(Qword);
            *((Qword *) pbSerializedBuffer) = m_qwComposition64BitHash;
            pbSerializedBuffer += sizeof(Qword);
            *((uint32_t *) pbSerializedBuffer) = (uint32_t) stlVectorOfElementName64BitHashes.size();
            pbSerializedBuffer += sizeof(uint32_t);
            for (auto vectorElement : stlVectorOfElementName64BitHashes) 
            {
                *((Dword *) pbSerializedBuffer) = 0x83746383;
                pbSerializedBuffer += sizeof(Dword);
                *((uint32_t *) pbSerializedBuffer) = (uint32_t) stlSerializedElements[vectorElement].size();
                pbSerializedBuffer += sizeof(uint32_t);
                ::memcpy((void *) pbSerializedBuffer, (const void *) stlSerializedElements[vectorElement].data(), stlSerializedElements[vectorElement].size());
                pbSerializedBuffer += stlSerializedElements[vectorElement].size();
            }
            *((Dword *) pbSerializedBuffer) = 0xDCBAC95E;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        m_qw64BitHash = 0;
        m_qwComposition64BitHash = 0;
        m_stlSerializedBuffer.clear();
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        m_qw64BitHash = 0;
        m_qwComposition64BitHash = 0;
        m_stlSerializedBuffer.clear();
    }
}

/********************************************************************************************/

void __thiscall StructuredBuffer::DeSerialize(
    _in const char * c_szBase64SerializedBuffer
    )
{
    __DebugFunction();
    _ThrowBaseExceptionIf((nullptr == c_szBase64SerializedBuffer), "Invalid parameters passed in", nullptr);

    // Decode the base64 buffer into a raw buffer
    std::vector<Byte> stlRawBuffer = ::Base64Decode(c_szBase64SerializedBuffer);
    // Deserialize the raw buffer
    this->DeSerialize(stlRawBuffer.data(), (unsigned int) stlRawBuffer.size());
}
            
/********************************************************************************************/

void __thiscall StructuredBuffer::DeSerialize(
    _in const Byte * c_pbSerializedData,
    _in unsigned int unSerializedDataSizeInBytes
    )
{
    __DebugFunction();
    _ThrowBaseExceptionIf(((nullptr == c_pbSerializedData)||(0 == unSerializedDataSizeInBytes)), "Invalid parameters passed in", nullptr);
    
    this->Clear();

    // +------------------------------------------------------------------------------------+
    // | [Dword] 0xE59CABCD                                                                 |
    // +------------------------------------------------------------------------------------+
    // | [Qword] qw64BitHash                                                                |
    // +------------------------------------------------------------------------------------+
    // | [Qword] m_qwComposition64BitHash                                                   |
    // +------------------------------------------------------------------------------------+
    // | [uint32_t] Number of Elements                                                      |
    // +-----+------------------------------------------------------------------------------+
    // |     | [Dword] 0x83746383                                                           |
    // |     +------------------------------------------------------------------------------+
    // | ... | [uint32_t] un32SizeInBytesOfSerializedElement                                |
    // |     +------------------------------------------------------------------------------+
    // |     | [...] Serialized Element                                                     |
    // +-----+------------------------------------------------------------------------------+
    // | [Dword] 0xDCBAC95E                                                                 |
    // +------------------------------------------------------------------------------------+

    const Byte * pbCurrentByte = c_pbSerializedData;
    _ThrowBaseExceptionIf((0xE59CABCD != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [HEADER] = 0xE59CABCD but got 0x%08X", *((Dword *) pbCurrentByte));
    pbCurrentByte += sizeof(Dword);
    m_qw64BitHash = *((Qword *) pbCurrentByte);
    pbCurrentByte += sizeof(Qword);
    m_qwComposition64BitHash = *((Qword *) pbCurrentByte);
    pbCurrentByte += sizeof(Qword);
    unsigned int unNumberOfElements = (unsigned int) *((uint32_t *) pbCurrentByte);
    pbCurrentByte += sizeof(uint32_t);
    for (unsigned int unIndex = 0; unIndex < unNumberOfElements; ++unIndex)
    {
        _ThrowBaseExceptionIf((0x83746383 != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [ELEMENT_HEADER] = 0xE59CABCD but got 0x%08X", *((Dword *) pbCurrentByte));
        pbCurrentByte += sizeof(Dword);
        unsigned int unElementSerializedDataSizeInBytes = (unsigned int) *((uint32_t *) pbCurrentByte);
        _ThrowBaseExceptionIf((0 == unElementSerializedDataSizeInBytes), "Invalid serialization format: Encountered zero sized serialized element at index %d", unIndex);
        pbCurrentByte += sizeof(uint32_t);
        StructuredBufferElement * poStructuredBufferElement = new StructuredBufferElement((const Byte *) pbCurrentByte, unElementSerializedDataSizeInBytes);
        __DebugAssert(m_stlMapOfElements.end() == m_stlMapOfElements.find(poStructuredBufferElement->GetElementName64BitHash()));
        m_stlMapOfElements[poStructuredBufferElement->GetElementName64BitHash()] = poStructuredBufferElement;
        pbCurrentByte += unElementSerializedDataSizeInBytes;
    }
    _ThrowBaseExceptionIf((0xDCBAC95E != *((Dword *) pbCurrentByte)), "Invalid serialization format: Expected [FOOTER] = 0xDCBAC95E but got 0x%08X", *((Dword *) pbCurrentByte));
    
    m_stlSerializedBuffer.assign(c_pbSerializedData, c_pbSerializedData + unSerializedDataSizeInBytes);
}

/********************************************************************************************/

void __thiscall StructuredBuffer::DeleteCachedData(void) throw()
{
    __DebugFunction();
    
    m_stlSerializedBuffer.clear();
    m_qw64BitHash = 0;
    m_qwComposition64BitHash = 0;
}

/********************************************************************************************/

void __stdcall StructuredBuffer::ToAString(
    _inout std::string & strDestination,
    _in const std::string & c_strIndent,
    _in const StructuredBuffer & c_oStructuredBuffer
    )
{
    __DebugFunction();

    char szTemporaryString[40];
    
    for (auto element : c_oStructuredBuffer.m_stlMapOfElements)
    {
        strDestination += c_strIndent;
        strDestination += element.second->GetElementName();
        strDestination += " (";
        switch (element.second->GetElementType())
        {
            case NULL_VALUE_TYPE
            :   strDestination += "NULL)";
                break;
            case BOOLEAN_VALUE_TYPE
            :   if (true == *((bool*) element.second->GetRawDataPtr()))
                {
                    strDestination += "BOOLEAN) = TRUE";
                }
                else
                {
                    strDestination += "BOOLEAN) = FALSE";
                }
                break;
            case ANSI_CHARACTER_VALUE_TYPE
            :   strDestination += "ANSI_CHARACTER) = ";
                strDestination += *((const char *) element.second->GetRawDataPtr());
                break;
            case ANSI_CHARACTER_STRING_VALUE_TYPE
            :   strDestination += "STRING) = ";
                strDestination += (const char *) element.second->GetRawDataPtr();
                break;
            case FLOAT32_VALUE_TYPE
            :   strDestination += "FLOAT32) = ";
                strDestination += std::to_string(*((const float32_t *) element.second->GetRawDataPtr()));
                break;
            case FLOAT64_VALUE_TYPE
            :   strDestination += "FLOAT64) = ";
                strDestination += std::to_string(*((const float64_t *) element.second->GetRawDataPtr()));
                break;
            case INT8_VALUE_TYPE
            :   strDestination += "INT8) = ";
                strDestination += std::to_string(*((const int8_t *) element.second->GetRawDataPtr()));
                break;
            case INT16_VALUE_TYPE
            :   strDestination += "INT16) = ";
                strDestination += std::to_string(*((const int16_t *) element.second->GetRawDataPtr()));
                break;
            case INT32_VALUE_TYPE
            :   strDestination += "INT32) = ";
                strDestination += std::to_string(*((const int32_t *) element.second->GetRawDataPtr()));
                break;
            case INT64_VALUE_TYPE
            :   strDestination += "INT64) = ";
                strDestination += std::to_string(*((const int64_t *) element.second->GetRawDataPtr()));
                break;
            case UINT8_VALUE_TYPE
            :   strDestination += "UINT8) = ";
                strDestination += std::to_string(*((const uint8_t *) element.second->GetRawDataPtr()));
                break;
            case UINT16_VALUE_TYPE
            :   strDestination += "UINT16) = ";
                strDestination += std::to_string(*((const uint16_t *) element.second->GetRawDataPtr()));
                break;
            case UINT32_VALUE_TYPE
            :   strDestination += "UINT32) = ";
                strDestination += std::to_string(*((const uint32_t *) element.second->GetRawDataPtr()));
                break;
            case UINT64_VALUE_TYPE
            :   strDestination += "UINT64) = ";
                strDestination += std::to_string(*((const uint64_t *) element.second->GetRawDataPtr()));
                break;
            case BYTE_VALUE_TYPE
            :   ::sprintf_s(szTemporaryString, sizeof(szTemporaryString), "0x%02X", (unsigned int) *((const Byte*) element.second->GetRawDataPtr()));
                strDestination += "BYTE) = ";
                strDestination += szTemporaryString;
                break;
            case WORD_VALUE_TYPE
            :   ::sprintf_s(szTemporaryString, sizeof(szTemporaryString), "0x%04X", (unsigned int) *((const Word *) element.second->GetRawDataPtr()));
                strDestination += "WORD) = ";
                strDestination += szTemporaryString;
                break;
            case DWORD_VALUE_TYPE
            :   ::sprintf_s(szTemporaryString, sizeof(szTemporaryString), "0x%08X", (unsigned int) *((const Dword *) element.second->GetRawDataPtr()));
                strDestination += "DWORD) = ";
                strDestination += szTemporaryString;
                break;
            case QWORD_VALUE_TYPE
            :   ::sprintf_s(szTemporaryString, sizeof(szTemporaryString), "0x%08X%08X", HIDWORD(*((const Qword *) element.second->GetRawDataPtr())), LODWORD(*((const Qword *) element.second->GetRawDataPtr())));
                strDestination += "QWORD) = ";
                strDestination += szTemporaryString;
                break;
            case BUFFER_VALUE_TYPE
            :   strDestination += "BUFFER) = ";
                strDestination += ::Base64Encode(element.second->GetRawDataPtr(), element.second->GetRawDataSizeInBytes());
                break;
            case INDEXED_BUFFER_VALUE_TYPE
            :   {
                    std::string strNewIndent = c_strIndent + "    ";
                    StructuredBuffer oStructuredBuffer(element.second->GetRawDataPtr(), element.second->GetRawDataSizeInBytes());
                    strDestination += "STRUCTURED_BUFFER) = {\r\n";
                    StructuredBuffer::ToAString(strDestination, strNewIndent, oStructuredBuffer);
                    strDestination += c_strIndent;
                    strDestination += "}";
                }
                break;
            case GUID_VALUE_TYPE
            :   {
                    Guid oGuid(element.second->GetRawDataPtr());
                    strDestination += "GUID) = ";
                    strDestination += oGuid.ToString(eHyphensAndCurlyBraces);
                }
                break;
            default
            :   strDestination += "UNKNOWN)";
                break;
        }
        strDestination += "\r\n";
    }
}