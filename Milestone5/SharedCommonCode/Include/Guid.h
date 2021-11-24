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

/********************************************************************************************

11111100 (0xFC) Bit Mask for MSB in GUID
=======================================================
00000000 (0x00) Undefined
00000100 (0x04) Organization
00001000 (0x08) Group
00001100 (0x0C) User
00010000 (0x10) Cryptographic Key
00010100 (0x14) Digital Contract
00011000 (0x18) Data Federation
00011100 (0x1C) Dataset Family
00100000 (0x20) Dataset
00100100 (0x24) Table
00101000 (0x28) Column
00101100 (0x2C) Cloud (Azure) Template
00110000 (0x30) Platform Services Virtual Machine
00110100 (0x34) Data Services Virtual Machine
00111000 (0x38) WebUI Virtual Machine
00111100 (0x3C) Remote Data Connector Virtual Machine
01000000 (0x40) Secure Computational Virtual Machine
01000100 (0x44) SAFE Function
01001000 (0x48) Job Identifier
01001100 (0x4C) Input Parameter Identifier
01010000 (0x50) Output Parameter Identifier
01010100 (0x54) User Supplied Data
01011000 (0x58) Orchestrator Identifier
01011100 (0x5C) Audit Event (Root Node)
01100000 (0x60) Audit Event (DC Branch Node)
01100100 (0x64) Audit Event (VM Branch Node)
01101000 (0x68) Audit Event (Encrypted Leaf Node)
01101100 (0x6C) Audit Event (Plain-text Leaf Node)

********************************************************************************************/

typedef enum
{
	eUndefined = 0x00,
	eOrganization = 0x04,
	eGroup = 0x08,
	eUser = 0x0C,
	eCryptographicKey = 0x10,
	eDigitalContract = 0x14,
	eDataFederation = 0x18,
	eDatasetFamily = 0x1C,
	eDataset = 0x20,
	eTable = 0x24,
	eColumn = 0x28,
	eAzureTemplate = 0x2C,
	ePlatformServicesVirtualMachine = 0x30,
	eDataServicesVirtualMachine = 0x34,
	eWebUiVirtualMachine = 0x38,
	eRemoteDataConnectorVirtualMachine = 0x3C,
	eSecureComputationalVirtualMachine = 0x40,
	eSafeFunction = 0x44,
	eJobIdentifier = 0x48,
	eInputParameterIdentifier = 0x4C,
	eOutputParameterIdentifier = 0x50,
	eUserSuppliedData = 0x54,
	eOrchestratorIdentifier = 0x58,
	eAuditEvent_RootNode = 0x5C,
	eAuditEvent_DigitalContractBranchNode = 0x60,
	eAuditEvent_VirtualMachineBranchNode = 0x64,
	eAuditEvent_EncryptedLeafNode = 0x68,
	eAuditEvent_PlainTextLeafNode = 0x6C,
	eGuidTypeBitMask = 0xFC,
	eNegativeGuidTypeMask = 0x03
}
GuidObjectType;

/********************************************************************************************/

class Guid : public Object
{
    public:

        Guid(void);
        Guid(
            _in GuidObjectType eObjectType
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
        GuidObjectType __thiscall GetObjectType(void) const throw();

    private:

        void __thiscall InitializeFromString(
            _in const char * c_szGuid
            );

        std::vector<Byte> m_stlRawData;
};
