/*********************************************************************************************
 *
 * @file DataConnector.cpp
 * @author Prawal Gangwar
 * @date 05 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Base64Encoder.h"
#include "DataConnector.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "StatusMonitor.h"
#include "IpcTransactionHelperFunctions.h"

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>

// Singleton Object
static DataConnector g_oDataConnector;

/********************************************************************************************
 *
 * @function GetDataConnector
 * @brief Gets the singleton instance of the DataConnector object
 *
 ********************************************************************************************/
 
DataConnector * __stdcall GetDataConnector(void) throw()
{
    __DebugFunction();

    return &g_oDataConnector;
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function DataConnector
 * @brief Constructor to create a DataConnector object
 * @param[in] c_szFileName Name of the dataset file(csvp)
 *
 ********************************************************************************************/

DataConnector::DataConnector(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function DataConnector
 * @brief Default copy constructor which is overloaded in order to trap any attempts to copy this singleton object
 *
 ********************************************************************************************/
 
DataConnector::DataConnector(
    _in const DataConnector & c_oDataConnector
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function ~DataConnector
 * @brief object Destructor
 *
 ********************************************************************************************/

DataConnector::~DataConnector(void)
{
    __DebugFunction();

    if (nullptr != m_poDataSetMetaDataStructuredBuffer)
    {
        m_poDataSetMetaDataStructuredBuffer->Release();
        m_poDataSetMetaDataStructuredBuffer = nullptr;
    }
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function LoadAndVerify
 * @brief Loads the csv file by verifying its integerity and storing it in
 *     memory as a 3D vecotr of tables.
 * @return true on successful verification, false otherwise
 *
 ********************************************************************************************/

// TODO: instead of fileName, take the file as a buffer
bool __thiscall DataConnector::LoadAndVerify(
    _in const std::vector<Byte> c_stlDataset,
    _in RootOfTrustNode * poRootOfTrustNode
    )
{
    __DebugFunction();

    m_poRootOfTrustNode = poRootOfTrustNode;

    // Set the stream to take input from the data vector;
    std::stringstream stlDatasetFile;
    stlDatasetFile.rdbuf()->pubsetbuf((char *)c_stlDataset.data(), c_stlDataset.size());

    // Read and verify the Header Marker
    uint64_t unHeader;
    stlDatasetFile.read((char *)&unHeader, sizeof(uint64_t));
    _ThrowBaseExceptionIf((0xDEADBEEFDEADBEEF != unHeader), "Expected header to be [0xDEADBEEFDEADBEEF] but found [%xul]", unHeader, nullptr);

    // Read the size of the Header StructuredBuffer
    uint32_t unHeaderSize;
    stlDatasetFile.read((char *)&unHeaderSize, sizeof(uint32_t));

    // Read the Structured Buffer
    std::vector<Byte> stlHeaderStructuredBuffer(unHeaderSize);
    stlDatasetFile.read((char *)stlHeaderStructuredBuffer.data(), unHeaderSize);
    StructuredBuffer oHeaderStructuredBuffer(stlHeaderStructuredBuffer);

    // Read the relevant data from the Header Strucuted Buffer
    uint64_t unMetaDataOffset = oHeaderStructuredBuffer.GetUnsignedInt64("MetaDataOffset");
    int32_t m_unMetaDataSizeInBytes = oHeaderStructuredBuffer.GetInt32("MetaDataSize");

    // TODO: Verify the signed base64 encoded hash
    std::string m_unSignedHash = oHeaderStructuredBuffer.GetString("SignedHash");

    // Read the Dataset MetaData Structured Buffer
    stlDatasetFile.seekg(unMetaDataOffset);
    std::vector<Byte> stlMetaDataStructuredBuffer(m_unMetaDataSizeInBytes);
    stlDatasetFile.read((char *)stlMetaDataStructuredBuffer.data(), m_unMetaDataSizeInBytes);
    m_poDataSetMetaDataStructuredBuffer = new StructuredBuffer(stlMetaDataStructuredBuffer);
    std::cout << "Dataset Metadata\n" << m_poDataSetMetaDataStructuredBuffer->ToString();
    m_oAllDatasetIds.PutString("DatasetUuid", m_poDataSetMetaDataStructuredBuffer->GetString("UUID"));
    int32_t nNumberOfTables = m_poDataSetMetaDataStructuredBuffer->GetInt32("NumberTables");

    // Convert the metadata offset Byte buffer to uin64_t vector
    std::vector<Byte> stlTableMetadataOffsetBuffer = m_poDataSetMetaDataStructuredBuffer->GetBuffer("OffsetTables");
    std::vector<uint64_t> stlTableMetadataOffsetArray(nNumberOfTables);
    ::memcpy(stlTableMetadataOffsetArray.data(), stlTableMetadataOffsetBuffer.data(), stlTableMetadataOffsetBuffer.size());

    // Convert the metadata size Byte buffer to uin64_t vector
    std::vector<Byte> stlTableMetadataSizeBuffer = m_poDataSetMetaDataStructuredBuffer->GetBuffer("SizeTables");
    std::vector<uint64_t> stlTableMetadataSizeArray(nNumberOfTables);
    ::memcpy(stlTableMetadataSizeArray.data(), stlTableMetadataSizeBuffer.data(), stlTableMetadataSizeBuffer.size());

    StructuredBuffer oTableMetadata;
    // Seek to each tables metadata and store the StructuredBuffer into the class member
    for (unsigned int unTableID = 0; unTableID < nNumberOfTables; unTableID++)
    {
        stlDatasetFile.seekg(stlTableMetadataOffsetArray[unTableID]);
        std::vector<Byte> stlTempTableMetadata(stlTableMetadataSizeArray[unTableID]);
        stlDatasetFile.read((char *)stlTempTableMetadata.data(), stlTableMetadataSizeArray[unTableID]);
        m_stlTableMetaData.push_back(StructuredBuffer(stlTempTableMetadata));
        std::cout << "Table Metadata\n" << StructuredBuffer(stlTempTableMetadata).ToString();
        oTableMetadata.PutString(m_stlTableMetaData[unTableID].GetString("Name").c_str(), m_stlTableMetaData.at(unTableID).GetGuid("Guid").ToString(eRaw));
    }
    m_oAllDatasetIds.PutStructuredBuffer("Tables", oTableMetadata);
    std::cout << "\nDataIds\n" << m_oAllDatasetIds.ToString();

    // Read each table and store it in a 3D vector of tables
    for (unsigned int unTableID = 0; unTableID < nNumberOfTables; unTableID++)
    {
        std::vector<std::vector<std::string>> stlIndividualTable;
        stlDatasetFile.seekg(m_stlTableMetaData[unTableID].GetUnsignedInt64("TableOffset"));
        std::string strLineFromFile;

        for (unsigned int unRowNumber = 0; unRowNumber < m_stlTableMetaData[unTableID].GetInt32("NumberRows"); unRowNumber++)
        {
            std::vector<std::string> stlOneRow;
            std::getline(stlDatasetFile, strLineFromFile);
            std::stringstream oStringStream(strLineFromFile);
            for (unsigned int unColumnNumber = 0; unColumnNumber < m_stlTableMetaData[unTableID].GetInt32("NumberColumns"); unColumnNumber++)
            {
                std::string strCell;
                std::getline(oStringStream, strCell, '\x1f');
                stlOneRow.push_back(strCell);
            }
            stlIndividualTable.push_back(stlOneRow);
        }
        m_stlTableData.push_back(stlIndividualTable);

        // Also add table name to id in the cache map
        m_stlMapOfTableNameToId.insert(std::make_pair(m_stlTableMetaData[unTableID].GetString("Name"), unTableID));
    }

    StructuredBuffer oEventData;
    oEventData.PutBoolean("Success", true);
    oEventData.PutStructuredBuffer("DatasetHeaderData", oHeaderStructuredBuffer);
    oEventData.PutStructuredBuffer("DatasetMetadata", *m_poDataSetMetaDataStructuredBuffer);
    m_poRootOfTrustNode->RecordAuditEvent("LOAD_DATASET", 0x1111, 0x05, oEventData);

    return true;
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function HandleRequest
 * @brief Handle incoming requests for data from the research process
 * @return void
 * @note
 *     The method will stop taking in more requests on request to stop either from the
 *     Root oF Trust or the researcher itself.
 *
 ********************************************************************************************/

// TODO: implement lazy-loading for some requests
void __thiscall DataConnector::HandleRequest(
    _in Socket * poSocket
    ) const throw()
{
    __DebugFunction();
    __DebugAssert(nullptr != poSocket);

    try
    {
        std::vector<Byte> stlRequestBuffer = ::GetIpcTransaction(poSocket, true);
        if (0 < stlRequestBuffer.size())
        {
            // Respose Structured Buffer
            StructuredBuffer oDataResponse;
            StructuredBuffer oResearcherRequest(stlRequestBuffer);

            // Check the digital contract to check if the request is allowed
            bool fAllowed = this->FilterDataRequest(oResearcherRequest);

            // Get the table ID either from the table name provided or the table ID
            uint32_t unTableID;
            if (oResearcherRequest.IsElementPresent("TableID", UINT32_VALUE_TYPE))
            {
                unTableID = oResearcherRequest.GetUnsignedInt32("TableID");
            }
            else
            {
                std::string strTableName = oResearcherRequest.GetString("TableName");
                if (m_stlMapOfTableNameToId.end() != m_stlMapOfTableNameToId.find(strTableName))
                {
                    unTableID = m_stlMapOfTableNameToId.at(strTableName);
                }
                else
                {
                    oDataResponse.PutBoolean("Status", "Fail");
                    oDataResponse.PutString("ResponseString", "Invalid Table Name");
                    fAllowed = false;
                }
            }

            if (true == fAllowed)
            {
                uint8_t requestType = oResearcherRequest.GetInt8("RequestType");

                if (eGetRowRange == requestType)
                {
                    uint32_t unTableRowStart = oResearcherRequest.GetInt32("RowRangeStart");
                    uint32_t unTableRowEnd = oResearcherRequest.GetInt32("RowRangeEnd");
                    StructuredBuffer oTempResponse = GetTableRowRange(unTableID, unTableRowStart, unTableRowEnd);
                    oDataResponse.PutBoolean("Status", oTempResponse.GetBoolean("Status"));
                    oDataResponse.PutString("ResponseString", oTempResponse.GetString("ResponseString"));

                    StructuredBuffer oEventData;
                    oEventData.PutBoolean("Success", oTempResponse.GetBoolean("Status"));
                    oEventData.PutUnsignedInt32("TableIdentifier", unTableID);
                    oEventData.PutInt32("RowRangeStart", unTableRowStart);
                    oEventData.PutInt32("RowRangeStart", unTableRowEnd);
                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_REQUEST_ROW_RANGE", 0x1100, 0x01, oEventData);
                }
                else if (eGetColumnRange == requestType)
                {
                    uint32_t unTableColumnStart = oResearcherRequest.GetInt32("ColumnRangeStart");
                    uint32_t unTableColumnEnd = oResearcherRequest.GetInt32("ColumnRangeEnd");
                    StructuredBuffer oTempResponse = GetTableColumnRange(unTableID, unTableColumnStart, unTableColumnEnd);
                    oDataResponse.PutBoolean("Status", oTempResponse.GetBoolean("Status"));
                    oDataResponse.PutString("ResponseString", oTempResponse.GetString("ResponseString"));

                    StructuredBuffer oEventData;
                    oEventData.PutBoolean("Success", oTempResponse.GetBoolean("Status"));
                    oEventData.PutUnsignedInt32("TableIdentifier", unTableID);
                    oEventData.PutInt32("ColumnRangeStart", unTableColumnStart);
                    oEventData.PutInt32("ColumnRangeEnd", unTableColumnEnd);
                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_REQUEST_COLUMN_RANGE", 0x1100, 0x01, oEventData);
                }
                else if (eGetTable == requestType)
                {
                    StructuredBuffer oTempResponse = GetTableRowRange(unTableID, 0, m_stlTableMetaData[unTableID].GetInt32("NumberRows")-1);
                    oDataResponse.PutBoolean("Status", oTempResponse.GetBoolean("Status"));
                    oDataResponse.PutString("ResponseString", oTempResponse.GetString("ResponseString"));

                    StructuredBuffer oEventData;
                    oEventData.PutBoolean("Success", oTempResponse.GetBoolean("Status"));
                    oEventData.PutUnsignedInt32("TableIdentifier", unTableID);
                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_REQUEST_TABLE", 0x1100, 0x01, oEventData);
                }
                else if (eGetDatasetMetadata == requestType)
                {
                    oDataResponse.PutBoolean("Status", true);
                    oDataResponse.PutStructuredBuffer("ResponseData", *m_poDataSetMetaDataStructuredBuffer);

                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_GET_METADATA", 0x1100, 0x01, oDataResponse);
                }
                else if (eGetTableMetadata == requestType)
                {
                    oDataResponse.PutBoolean("Status", true);
                    oDataResponse.PutStructuredBuffer("ResponseData", m_stlTableMetaData[unTableID]);

                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_TABLE_GET_METADATA", 0x1100, 0x01, oDataResponse);
                }
                else if (eCloseFile == requestType)
                {
                    oDataResponse.PutBoolean("Status", true);

                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_CLOSE", 0x1100, 0x01, oDataResponse);
                }
                else if (eGetUuids == requestType)
                {
                    oDataResponse.PutBoolean("Status", true);
                    oDataResponse.PutStructuredBuffer("ResponseData", m_oAllDatasetIds);

                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_TABLE_GET_GUID", 0x1100, 0x01, oDataResponse);
                }
                else
                {
                    // Do something for invalid rquest
                    oDataResponse.PutBoolean("Status", false);
                    oDataResponse.PutString("ResponseData", "Invalid Request Type");

                    m_poRootOfTrustNode->RecordAuditEvent("DATASET_INVALID_REQUEST", 0x1100, 0x02, oDataResponse);
                }
            }
            else
            {
                // Send a reply of invalid request
                oDataResponse.PutBoolean("Status", false);
                oDataResponse.PutString("ResponseData", "Permission Denied!!");
            }

            _ThrowBaseExceptionIf((false == ::PutIpcTransaction(poSocket, oDataResponse.GetSerializedBuffer())), "Write response failed.", nullptr);
        }
    }

    catch (BaseException oException)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31m%s\033[0m" << oException.GetExceptionMessage() << std::endl
                  << "\033[1;31mThrow from ->|File = \033[0m" << oException.GetFilename() << std::endl
                  << "\033[1;31m             |Function = \033[0m" << oException.GetFunctionName() << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << oException.GetLineNumber() << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }

    catch (...)
    {
        std::cout << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl
                  << "\033[1;31mOH NO, AN UNKNOWN EXCEPTION!!!\033[0m" << std::endl << std::endl
                  << "\033[1;31mCaught in -->|File = \033[0m" << __FILE__ << std::endl
                  << "\033[1;31m             |Function = \033[0m" << __func__ << std::endl
                  << "\033[1;31m             |Line number = \033[0m" << __LINE__ << std::endl
                  << "\r\033[1;31m---------------------------------------------------------------------------------\033[0m" << std::endl;
    }
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function GetTableRowRange
 * @brief Get a range of rows from the table CSV
 * @param[in] unTableID Table ID
 * @param[in] unStartRowNumber Starting Row number to fetch
 * @param[in] unEndRowNumber Last Row number to fetch
 * @return a StructuredBuffer of comma separated row enteries and request status
 *
 ********************************************************************************************/

StructuredBuffer __thiscall DataConnector::GetTableRowRange(
    _in unsigned int unTableID,
    _in unsigned int unStartRowNumber,
    _in unsigned int unEndRowNumber
    ) const
{
    __DebugFunction();

    StructuredBuffer oResponseStructuredBuffer;
    std::string strResponseString;

    if ((m_poDataSetMetaDataStructuredBuffer->GetInt32("NumberTables") <= unTableID) || (m_stlTableMetaData[unTableID].GetInt32("NumberRows") <= unEndRowNumber) || (unStartRowNumber > unEndRowNumber))
    {
        oResponseStructuredBuffer.PutBoolean("Status", false);
        oResponseStructuredBuffer.PutString("ResponseString", "Out of Bounds Request");
    }
    else
    {
        for (unsigned int unRowNumber = unStartRowNumber; unRowNumber <= unEndRowNumber; unRowNumber++)
        {
            for (unsigned int unColumnNumber = 0; unColumnNumber < m_stlTableData[unTableID][unRowNumber].size(); unColumnNumber++)
            {
                if (unColumnNumber == (m_stlTableData[unTableID][unRowNumber].size() -1 ))
                {
                    strResponseString.append(m_stlTableData[unTableID][unRowNumber][unColumnNumber] + "\n");
                }
                else
                {
                    strResponseString.append(m_stlTableData[unTableID][unRowNumber][unColumnNumber] + "\x1f");
                }
            }
        }
        oResponseStructuredBuffer.PutBoolean("Status", true);
        oResponseStructuredBuffer.PutString("ResponseString", strResponseString);
    }

    return oResponseStructuredBuffer;
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function GetTableColumnRange
 * @brief Get a range of columns from the table CSV
 * @param[in] unTableID Table ID
 * @param[in] unStartColumnNumber Starting Column number to fetch
 * @param[in] unEndColumnNumber Last Column number to fetch
 * @return a StructuredBuffer of comma separated column enteries and the request status
 *
 ********************************************************************************************/

StructuredBuffer __thiscall DataConnector::GetTableColumnRange(
    _in unsigned int unTableID,
    _in unsigned int unStartColumnNumber,
    _in unsigned int unEndColumnNumber
    ) const
{
    __DebugFunction();

    StructuredBuffer oResponseStructuredBuffer;
    std::string strResponseString;

    if ((m_poDataSetMetaDataStructuredBuffer->GetInt32("NumberTables") <= unTableID) || (m_stlTableMetaData[unTableID].GetInt32("NumberColumns") <= unEndColumnNumber) || (unStartColumnNumber > unEndColumnNumber))
    {
        oResponseStructuredBuffer.PutBoolean("Status", false);
        oResponseStructuredBuffer.PutString("ResponseString", "Out of Bounds Request");
    }
    else
    {
        for (unsigned int unRowNumber = 0; unRowNumber < m_stlTableData[unTableID].size(); unRowNumber++)
        {
            for (unsigned int unColumnNumber = unStartColumnNumber; unColumnNumber <= unEndColumnNumber; unColumnNumber++)
            {
                if (unColumnNumber == unEndColumnNumber)
                {
                    strResponseString.append(m_stlTableData[unTableID][unRowNumber][unColumnNumber] + "\n");
                }
                else
                {
                    strResponseString.append(m_stlTableData[unTableID][unRowNumber][unColumnNumber] + "\x1f");
                }
            }
        }
        oResponseStructuredBuffer.PutBoolean("Status", true);
        oResponseStructuredBuffer.PutString("ResponseString", strResponseString);
    }

    return oResponseStructuredBuffer;
}

/********************************************************************************************
 *
 * @class DataConnector
 * @function FilterDataRequest
 * @brief Check the request params as per the digital contract and Dataset params
 * @param[in] oDataRequest StructuredBuffer containing the request from research process
 * @return true if values should be returned, else false
 *
 ********************************************************************************************/

bool __thiscall DataConnector::FilterDataRequest(
    _in StructuredBuffer oDataRequest
    ) const
{
    __DebugFunction();

    return true;
}
