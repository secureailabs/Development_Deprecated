/*********************************************************************************************
 *
 * @file RootOfTrustNode.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"
#include <string>
#include <vector>

/********************************************************************************************/

class RootOfTrustNode : public Object
{
    public:
    
        RootOfTrustNode(
            _in const char * c_szIpcPathForInitialization
            );
        RootOfTrustNode(
            _in const RootOfTrustNode & c_oRootOfTrust
            );
        virtual ~RootOfTrustNode(void);
        
        Guid __thiscall GetDomainIdentifier(void) const;
        std::vector<Byte> __thiscall GetDataset(void) const;
        
        void __thiscall RecordAuditEvent(
            _in const char * c_szEventName,
            _in Word wTargetChannelsBitMask,
            _in Dword dwEventType,
            _in const StructuredBuffer & c_oEventData
            ) const;
            
    private:
    
        Byte m_bProcessType;
        Guid m_oDomainIdentifier;
        
        std::string m_strRootOfTrustIpcPath;
};
