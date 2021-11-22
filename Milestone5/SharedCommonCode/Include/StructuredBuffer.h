/*********************************************************************************************
 *
 * @file StructuredBuffer.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"
#include "Object.h"
#include <map>
#include <string>
#include <vector>

#define NULL_VALUE_TYPE                   0x00
#define BOOLEAN_VALUE_TYPE                0x01
#define ANSI_CHARACTER_VALUE_TYPE         0x02
#define ANSI_CHARACTER_STRING_VALUE_TYPE  0x03
#define FLOAT32_VALUE_TYPE                0x04
#define FLOAT64_VALUE_TYPE                0x05
#define INT8_VALUE_TYPE                   0x06
#define INT16_VALUE_TYPE                  0x07
#define INT32_VALUE_TYPE                  0x08
#define INT64_VALUE_TYPE                  0x09
#define UINT8_VALUE_TYPE                  0x0A
#define UINT16_VALUE_TYPE                 0x0B
#define UINT32_VALUE_TYPE                 0x0C
#define UINT64_VALUE_TYPE                 0x0D
#define BYTE_VALUE_TYPE                   0x0E
#define WORD_VALUE_TYPE                   0x0F
#define DWORD_VALUE_TYPE                  0x10
#define QWORD_VALUE_TYPE                  0x11
#define BUFFER_VALUE_TYPE                 0x12
#define INDEXED_BUFFER_VALUE_TYPE         0x13
#define GUID_VALUE_TYPE                   0x14
#define BIG_NUMBER_TYPE                   0x15
#define INVALID_VALUE_TYPE                0xFF
#define ANY_VALUE_TYPE                    0xFF

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

class StructuredBufferElement : public Object
{
    public:

        // Constructors and destructors
        
        StructuredBufferElement(
            _in const char * szElementName,
            _in Byte bElementType,
            _in const Byte * c_pElementData,
            _in unsigned int unElementDataSizeInBytes
            );
        StructuredBufferElement(
            _in const Byte * c_pbSerializedRawData,
            _in unsigned int unSerializedRawDataSizeInBytes
            );
        StructuredBufferElement(
            _in const std::vector<Byte> c_stlSerializedRawData
            );
        StructuredBufferElement(
            _in StructuredBufferElement & oStructuredBufferElement
            );
        StructuredBufferElement(
            _in const StructuredBufferElement & c_oStructuredBufferElement
            );
        virtual ~StructuredBufferElement(void);

        // Public access methods
        
        std::string __thiscall GetElementName(void) const throw();
        Qword __thiscall GetElementName64BitHash(void) const throw();
        Byte __thiscall GetElementType(void) const throw();
        const Byte * __thiscall GetRawDataPtr(void) const throw();
        unsigned int __thiscall GetRawDataSizeInBytes(void) const throw();
        std::vector<Byte>  __thiscall GetRawData(void) const throw();        
        const Byte * __thiscall GetSerializedBufferRawDataPtr(void) const throw();
        unsigned int __thiscall GetSerializedBufferRawDataSizeInBytes(void) const throw();
        std::vector<Byte> __thiscall GetSerializedBuffer(void) const throw();
        
    private:
    
        // Internal Serialization methods
        void __thiscall Serialize(void) const throw();
        void __thiscall DeSerialize(
            _in const Byte * c_pbSerializedData,
            _in unsigned int unSerializedDataSizeInBytes
            );
            
        // Private data members.
        
        std::string m_strElementName;
        Byte m_bElementType;
        Qword m_qwElementName64BitHash;
        std::vector<Byte> m_stlElementData;
        mutable std::vector<Byte> m_stlSerializedBuffer;
};

/********************************************************************************************/

class StructuredBuffer : public Object
{
    public:
        
        // Constructors and destructors
        
        StructuredBuffer(void);
        StructuredBuffer(
            _in const Byte * c_pbSerializedBuffer,
            _in unsigned int unBufferSizeInBytes
            );
        StructuredBuffer(
            _in const std::vector<Byte> & c_oSerializedBuffer
            );
        StructuredBuffer(
            _in const char * c_szBase64SerializedBuffer
            );
        StructuredBuffer(
            _in const StructuredBuffer & c_oStructuredBuffer
            );
        StructuredBuffer& operator=(
            const StructuredBuffer & c_oStructuredBuffer
            );
        virtual ~StructuredBuffer(void) throw();

        // Remove all element
        void __thiscall Clear(void) throw();
        
        // Get a serialized buffer representing this StructuredBuffer. This buffer
        // can be fed into one of the constructors to de-serialize this object. There
        // are two different ways to do it. Either you get a vector<Byte> or you can
        // just ask for a pointer and sizeinbytes
        const Byte * __thiscall GetSerializedBufferRawDataPtr(void) const throw();
        unsigned int __thiscall GetSerializedBufferRawDataSizeInBytes(void) const throw();
        std::vector<Byte> __thiscall GetSerializedBuffer(void) const throw();
        std::string __thiscall GetBase64SerializedBuffer(void) const throw();
        
        // The "quick hashes" as 64 bit hash values that can be used to make quick
        // comparisons. The Get64BitHash() methods returns a hash representing both
        // the element composition and the element values within the StructuredBuffer while
        // the GetComposition64BitHash() represents a hash of the 'structure' of the
        // StructuredBuffer (i.e. two StructuredBuffer with identical element composition but
        // different data within each element would have the same composition hash). The
        // composition hash is a really quick way of checking to make that required
        // elements are all present
        
        Qword __thiscall Get64BitHash(void) const throw();
        Qword __thiscall GetComposition64BitHash(void) const throw();
        
        // Methods used to access element. It should be noted that there can only
        // ever be one element with a specific name (case insensitive) at any one
        // time.
        bool __thiscall IsElementPresent(
            _in const char * c_szElementName,
            _in Byte bElementType
            ) const throw();
        void __thiscall RemoveElement(
            _in const char * c_szElementName
            ) throw();
        std::vector<std::string> __thiscall GetNamesOfElements(void) const throw();
        std::vector<std::string> __thiscall GetDescriptionOfElements(void) const throw();
        
        // Prints out all of the StructuredBuffer metadata and data
        std::string __thiscall ToString(void) const throw();
        
        // Type strong methods to fetch element values
        void __thiscall GetNull(
            _in const char * c_szElementName
            ) const;
        bool __thiscall GetBoolean(
            _in const char * c_szElementName
            ) const;
        char __thiscall GetCharacter(
            _in const char * c_szElementName
            ) const;
        std::string __thiscall GetString(
            _in const char * c_szElementName
            ) const;
        float32_t __thiscall GetFloat32(
            _in const char * c_szElementName
            ) const;
        float64_t __thiscall GetFloat64(
            _in const char * c_szElementName
            ) const;
        int8_t __thiscall GetInt8(
            _in const char * c_szElementName
            ) const;
        int16_t __thiscall GetInt16(
            _in const char * c_szElementName
            ) const;
        int32_t __thiscall GetInt32(
            _in const char * c_szElementName
            ) const;
        int64_t __thiscall GetInt64(
            _in const char * c_szElementName
            ) const;
        uint8_t __thiscall GetUnsignedInt8(
            _in const char * c_szElementName
            ) const;
        uint16_t __thiscall GetUnsignedInt16(
            _in const char * c_szElementName
            ) const;
        uint32_t __thiscall GetUnsignedInt32(
            _in const char * c_szElementName
            ) const;
        uint64_t __thiscall GetUnsignedInt64(
            _in const char * c_szElementName
            ) const;
        Byte __thiscall GetByte(
            _in const char * c_szElementName
            ) const;
        Word __thiscall GetWord(
            _in const char * c_szElementName
            ) const;
        Dword __thiscall GetDword(
            _in const char * c_szElementName
            ) const;
        Qword __thiscall GetQword(
            _in const char * c_szElementName
            ) const;
        Guid __thiscall GetGuid(
            _in const char * c_szElementName
            ) const;
        std::vector<Byte> __thiscall GetBuffer(
            _in const char * c_szElementName
            ) const;
        StructuredBuffer __thiscall GetStructuredBuffer(
            _in const char * c_szElementName
            ) const;
        
        // Type strong methods to put element values
        void __thiscall PutNull(
            _in const char * c_szElementName
            );
        void __thiscall PutBoolean(
            _in const char * c_szElementName,
            _in bool fValue
            );
        void __thiscall PutCharacter(
            _in const char * c_szElementName,
            _in char chValue
            );
        void __thiscall PutString(
            _in const char * c_szElementName,
            _in const char * c_szValue
            );
        void __thiscall PutString(
            _in const char * c_szElementName,
            _in const std::string & c_strValue
            );
        void __thiscall PutFloat32(
            _in const char * c_szElementName,
            _in float32_t fl32Value
            );
        void __thiscall PutFloat64(
            _in const char * c_szElementName,
            _in float64_t fl64Value
            );
        void __thiscall PutInt8(
            _in const char * c_szElementName,
            _in int8_t n8Value
            );
        void __thiscall PutInt16(
            _in const char * c_szElementName,
            _in int16_t n16Value
            );
        void __thiscall PutInt32(
            _in const char * c_szElementName,
            _in int32_t n32Value
            );
        void __thiscall PutInt64(
            _in const char * c_szElementName,
            _in int64_t n64Value
            );
        void __thiscall PutUnsignedInt8(
            _in const char * c_szElementName,
            _in uint8_t un8Value
            );
        void __thiscall PutUnsignedInt16(
            _in const char * c_szElementName,
            _in uint16_t un16Value
            );
        void __thiscall PutUnsignedInt32(
            _in const char * c_szElementName,
            _in uint32_t un32Value
            );
        void __thiscall PutUnsignedInt64(
            _in const char * c_szElementName,
            _in uint64_t un64Value
            );
        void __thiscall PutByte(
            _in const char * c_szElementName,
            _in Byte bValue
            );
        void __thiscall PutWord(
            _in const char * c_szElementName,
            _in Word wValue
            );
        void __thiscall PutDword(
            _in const char * c_szElementName,
            _in Dword dwValue
            );
        void __thiscall PutQword(
            _in const char * c_szElementName,
            _in Qword qwValue
            );
        void __thiscall PutGuid(
            _in const char * c_szElementName,
            _in const Guid & c_oGuid
            );
        void __thiscall PutBuffer(
            _in const char * c_szElementName,
            _in const Byte * c_pbByteArray,
            _in unsigned int unSizeInBytes
            );
        void __thiscall PutBuffer(
            _in const char * c_szElementName,
            _in const std::vector<Byte> c_stlBuffer
            );
        void __thiscall PutStructuredBuffer(
            _in const char * c_szElementName,
            _in const StructuredBuffer & c_oStructuredBuffer
            );
            
    private:
    
        // Quick little utility method used to hash an element name AND check if
        // an element of that type exists. This utility function is used by all the
        // Get...() methods and returns the 64 bit quick hash of the element name.
        // If the element is not found, this method throws and exception
        Qword __thiscall CheckForElement(
            _in const char * c_szElementName,
            _in Byte bElementType
            ) const;
            
        // Add a new typed Element to the StructuredBuffer
        void __thiscall AddElement(
            _in const char * c_szElementName,
            _in Byte bElementType,
            _in const void * c_pElementData,
            _in unsigned int unElementDataSizeInBytes
            );
        void __thiscall RemoveElement(
            _in Qword qwElementName64BitHash
            ) throw();

        // Internal Serialization methods
        void __thiscall Serialize(void) const throw();
        void __thiscall DeSerialize(
            _in const char  * c_szBase64SerializedBuffer
            );
        void __thiscall DeSerialize(
            _in const Byte * c_pbSerializedData,
            _in unsigned int unSerializedDataSizeInBytes
            );
        
        // Method used to managed cached serialized data
        void __thiscall DeleteCachedData(void) throw();
  
        // Inner version of ToString()
        static void __stdcall ToString(
            _inout std::string & strDestination,
            _in const std::string & c_strIndent,
            _in const StructuredBuffer & c_oStructuredBuffer
            );
            
        // Map of StructuredBufferElements
        std::map<Qword, StructuredBufferElement *> m_stlMapOfElements;
        // For optimization purposes, we cache the serialized buffer generated by a call to
        // Serialize() such that subsequent calls to the method can instantly return without
        // having to serialize the object all over again. Calling any methods that modify the
        // StructuredBuffer object will instantly zeroize the cached serialized buffer.
        mutable std::vector<Byte> m_stlSerializedBuffer;
        // For optimization purposes, we track the most recent hash values generated. Methods that
        // return hashes will first check to see if their related cached value is non-zero and if so
        // instantly return the hash value without further processing. However, if the related
        // cached value is zero, then the method will do a full hash computation before caching it
        // and then returning the result. Calling any methods that modify the StructuredBuffer
        // object will instantly zeroize the cached hash values.
        mutable Qword m_qw64BitHash;
        mutable Qword m_qwComposition64BitHash;
};