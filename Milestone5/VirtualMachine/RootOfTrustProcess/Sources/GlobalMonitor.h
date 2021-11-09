/*********************************************************************************************
 *
 * @file GlobalMonitor.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Guid.h"
#include "Socket.h"
#include "StructuredBuffer.h"

#include <map>
#include <mutex>

/********************************************************************************************/

class GlobalMonitor
{
    public:
    
        GlobalMonitor(void);
        GlobalMonitor(
            _in const GlobalMonitor & c_oGlobalMonitor
            );
        ~GlobalMonitor(void);
        
        void __thiscall Run(
            _in bool fDisplayStatus
            );
        void __thiscall Run(void);
        void __thiscall HandleTransaction(
            _in Socket * poSocket
            );
        
    private:
    
        // Private methods used when displaying live information on the console
        void __thiscall DisplayStatus(void) throw();
        // Private methods used to handle incoming transactions
        std::vector<Byte> __thiscall TransactRegisterMonitor(
            _in const StructuredBuffer & c_oTransactionParameters
            );
        std::vector<Byte> __thiscall TransactMonitorHeartbeat(
            _in const StructuredBuffer & c_oTransactionParameters
            );
        std::vector<Byte> __thiscall TransactMonitorStatusUpdate(
            _in const StructuredBuffer & c_oTransactionParameters
            );
        std::vector<Byte> __thiscall TransactUnregisterMonitor(
            _in const StructuredBuffer & c_oTransactionParameters
            );
        std::vector<Byte> __thiscall TransactSignalTermination(
            _in const StructuredBuffer & c_oTransactionParameters
            );
            
        // Private data members
        
        std::recursive_mutex m_stlLock;
        bool m_fDisplayStatus;
        bool m_fIsTerminating;
        std::string m_strTerminationReason;
        std::map<Qword, std::string> m_stlListOfProcessNamesByIdentifierHashes;
        std::map<Qword, std::string> m_stlListOfDescriptionsByIdentifierHashes;
        std::map<Qword, int> m_stlListOfProcessIdentifiersByIdentifierHashes;
        std::map<Qword, int> m_stlListOfThreadIdentifiersByIdentifierHashes;
        std::map<Qword, uint64_t> m_stlListOfThreadTimestampsByIdentifierHashes;
        std::map<Qword, unsigned int> m_stlTotalNumberOfEventsByIdentifierHashes;
};

/********************************************************************************************/

// Returns the singleton instance of GlobalMonitor
extern GlobalMonitor * __stdcall GetGlobalMonitor(void) throw();