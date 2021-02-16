/*********************************************************************************************
 *
 * @file Guid.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include <string>
#include <vector>

/********************************************************************************************/

typedef enum name
{
    eRaw = 1,
    eHyphensOnly = 2,
    eHyphensAndCurlyBraces = 3
}
GuidFormat;

/********************************************************************************************/

typedef enum guidtype
{
    eOrganization = 0000,
    eUser = 0001,
    eCryptographicKey = 0010,
    eDigitalContract = 0011,
    eDataset = 0100,
    eFunctionNode = 0101,
    eVirtualMachine = 0110,
    eAuditEventBranchNode = 0x7,
    eAuditEventEncryptedLeafNode = 1000,
    eAuditEventPlainTextLeafNode = 1001,
    eOthers = 1111
}
GuidOfObjectType;

/********************************************************************************************/

class Guid : public Object
{
    public:

        Guid(void);
        Guid(
            _in GuidOfObjectType eObjectType
            );
        Guid(
            _in const char * c_szGuid
            );
        Guid(
            _in const Byte * c_pbBinaryBuffer
            );
        Guid(
            _in const Guid & c_oGuid
            );
        virtual ~Guid(void) throw();

        // Assignment operator overloadings
        Guid & __thiscall operator = (
            _in const char * c_szGuid
            );
        Guid & __thiscall operator = (
            _in const Guid & c_oGuid
            );
        // Comparison operator overloadings
        bool __thiscall operator == (
            _in const char * c_szGuid
            ) const throw();
        bool __thiscall operator == (
            _in const Guid & c_oGuid
            ) const throw();
        bool __thiscall operator != (
            _in const char * c_szGuid
            ) const throw();
        bool __thiscall operator != (
            _in const Guid & c_oGuid
            ) const throw();

        // Conversion methods
        const Byte * __thiscall GetRawDataPtr(void) const throw();
        std::vector<Byte> __thiscall GetRawData(void) const throw();
        std::string __thiscall ToString(
            _in GuidFormat eGuidFormat
            ) const throw();
        
        // Fetch type of Object
        Byte __thiscall GetObjectType(void) const throw();

    private:

        void __thiscall InitializeFromString(
            _in const char * c_szGuid
            );

        std::vector<Byte> m_stlRawData;
};
