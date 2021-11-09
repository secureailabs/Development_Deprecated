/*********************************************************************************************
 *
 * @file DataConnector.h
 * @author Prawal Gangwar
 * @date 05 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the DataConnector class that is used to handle CSV file data
 *     requests
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "RootOfTrustNode.h"
#include "Socket.h"
#include "StructuredBuffer.h"

#include <vector>
#include <string>

#include <fstream>

enum CSVRequest
{
    eGetRowRange,
    eGetColumnRange,
    eGetTable,
    eGetDatasetMetadata,
    eGetTableMetadata,
    eCloseFile,
    eGetUuids
};

/********************************************************************************************/

class DataConnector
{
    public:

        DataConnector(void);
        DataConnector(
            _in const DataConnector & c_oDataConnector
            );
        ~DataConnector(void);

        bool __thiscall LoadAndVerify(
            _in const std::vector<Byte> c_stlDataset,
            _in RootOfTrustNode * poRootOfTrustNode
            );
        void __thiscall HandleRequest(
            _in Socket * poSocket
            ) const throw();

        StructuredBuffer __thiscall GetTableRowRange(
            _in unsigned int unTableID,
            _in unsigned int unStartRowNumber,
            _in unsigned int unEndRowNumber
            ) const;
        StructuredBuffer __thiscall GetTableColumnRange(
            _in unsigned int unTableID,
            _in unsigned int unStartColumnNumber,
            _in unsigned int unEndColumnNumber
            ) const;

    private:

        // Private member methods
        bool __thiscall FilterDataRequest(
            _in StructuredBuffer oDataRequest
            ) const;

        // Private data members
        StructuredBuffer m_oAllDatasetIds;
        RootOfTrustNode * m_poRootOfTrustNode;
        StructuredBuffer * m_poDataSetMetaDataStructuredBuffer;
        std::vector<StructuredBuffer> m_stlTableMetaData;
        std::vector<std::vector<std::vector<std::string>>> m_stlTableData;
        std::map<std::string, int> m_stlMapOfTableNameToId;
};

/********************************************************************************************/

extern DataConnector * __stdcall GetDataConnector(void) throw();
