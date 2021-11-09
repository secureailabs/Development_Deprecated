/*********************************************************************************************
 *
 * @file DatasetFunctions.cpp
 * @author Shabana Akhtar Baig
 * @date 09 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetDatasetName
 * @brief Fetch dataset name associated with dataset guid from the database
 * @param[in] c_oRequest contains the dataset guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing dataset name
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetDatasetName(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the dataset record
        bsoncxx::stdx::optional<bsoncxx::document::value> oDsetDocument = oSailDatabase["Dataset"].find_one(document{}
                                                                                                            << "DatasetGuid" << c_oRequest.GetString("DatasetGuid")
                                                                                                            << "DataOwnerOrganizationGuid" << c_oRequest.GetString("DataOwnerOrganization")
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oDsetDocument)
        {                                                                         
            bsoncxx::document::element oPlainTextObjectBlobGuid = oDsetDocument->view()["PlainTextObjectBlobGuid"];
            if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                                << "PlainTextObjectBlobGuid" <<  strPlainTextObjectBlobGuid
                                                                                                                                                << finalize);
                if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
                {
                    bsoncxx::document::element oObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
                    if (oObjectGuid && oObjectGuid.type() == type::k_utf8)
                    {
                        std::string strObjectGuid = oObjectGuid.get_utf8().value.to_string();
                        bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                        if (bsoncxx::stdx::nullopt != oObjectDocument)
                        {
                            bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                            if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                            {
                                StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size); 
                                oResponse.PutString("DatasetName", oObject.GetString("DatasetName"));
                                dwStatus = 200;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ListDatasets
 * @brief Fetch the dataset information
 * @param[in] c_oRequest contains the dataset guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the dataset information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListDatasets(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch all dataset records
        mongocxx::cursor oDsetRecords = oSailDatabase["Dataset"].find({});
        // Loop through returned documents and add them to the list of datasets
        StructuredBuffer oListOfDatasets;
        for (auto&& oDocumentView : oDsetRecords)
        {  
            bsoncxx::document::element oDooGuid = oDocumentView["DataOwnerOrganizationGuid"];                          
            bsoncxx::document::element oDsetGuid = oDocumentView["DatasetGuid"];
            if (oDsetGuid && oDsetGuid.type() == type::k_utf8)
            {                   
                std::string strDsetGuid = oDsetGuid.get_utf8().value.to_string();                                                             
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDocumentView["PlainTextObjectBlobGuid"];
                if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
                {
                    std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                    bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                                    << "PlainTextObjectBlobGuid" <<  strPlainTextObjectBlobGuid
                                                                                                                                                    << finalize);
                    if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
                    {
                        bsoncxx::document::element oObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
                        if (oObjectGuid && oObjectGuid.type() == type::k_utf8)
                        {
                            std::string strObjectGuid = oObjectGuid.get_utf8().value.to_string();
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    if (oDooGuid && oDooGuid.type() == type::k_utf8)
                                    {
                                        std::string strOrganizationGuid = oDooGuid.get_utf8().value.to_string();
                                        // Get organization name
                                        StructuredBuffer oDooName = this->GetOrganizationName(strOrganizationGuid);
                                        if (200 == oDooName.GetDword("Status")) 
                                        {
                                            oObject.PutString("OrganizationName", oDooName.GetString("OrganizationName"));
                                        }
                                    } 
                                    oListOfDatasets.PutStructuredBuffer(strDsetGuid.c_str(), oObject);
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("Datasets", oListOfDatasets);
        dwStatus = 200;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function PullDataset
 * @brief Fetch the dataset information
 * @param[in] c_oRequest contains the dataset guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the dataset information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::PullDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strDsetGuid = c_oRequest.GetString("DatasetGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the dataset record
        bsoncxx::stdx::optional<bsoncxx::document::value> oDsetDocument = oSailDatabase["Dataset"].find_one(document{}
                                                                                                                    << "DatasetGuid" << strDsetGuid
                                                                                                                    << finalize);
        if (bsoncxx::stdx::nullopt != oDsetDocument)
        {                                                                         
            bsoncxx::document::element oDooGuid = oDsetDocument->view()["DataOwnerOrganizationGuid"];                                
            bsoncxx::document::element oPlainTextObjectBlobGuid = oDsetDocument->view()["PlainTextObjectBlobGuid"];
            if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                                << "PlainTextObjectBlobGuid" <<  strPlainTextObjectBlobGuid
                                                                                                                                                << finalize);
                if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
                {
                    bsoncxx::document::element oObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
                    if (oObjectGuid && oObjectGuid.type() == type::k_utf8)
                    {
                        std::string strObjectGuid = oObjectGuid.get_utf8().value.to_string();
                        bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                        if (bsoncxx::stdx::nullopt != oObjectDocument)
                        {
                            bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                            if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                            {
                                StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                if (oDooGuid && oDooGuid.type() == type::k_utf8)
                                {
                                    std::string strOrganizationGuid = oDooGuid.get_utf8().value.to_string();
                                    // Get organization name
                                    StructuredBuffer oDooName = this->GetOrganizationName(strOrganizationGuid);
                                    if (200 == oDooName.GetDword("Status")) 
                                    {
                                        oObject.PutString("OrganizationName", oDooName.GetString("OrganizationName"));
                                    }
                                } 
                                oResponse.PutStructuredBuffer("Dataset", oObject);
                                dwStatus = 200;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterDataset
 * @brief Store metadata of the dataset in the database
 * @param[in] c_oRequest contains the dataset information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Get the dataset guid
        std::string strDsetGuid = c_oRequest.GetString("DatasetGuid");
        // Get organization guid
        std::string strDooGuid = c_oRequest.GetString("DataOwnerGuid");
        // Create guids for the documents
        Guid oObjectGuid, oPlainTextObjectBlobGuid;
        
        // Create a dataset document
        bsoncxx::document::value oDsetDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "DatasetGuid" << strDsetGuid
        << "DataOwnerOrganizationGuid" << strDooGuid
        << finalize;

        // Create an object document
        // TODO: Add table and column metadata to oObject
        StructuredBuffer oDatasetMetadata = c_oRequest.GetStructuredBuffer("DatasetData");
        StructuredBuffer oObject;
        oObject.PutString("VersionNumber", oDatasetMetadata.GetString("VersionNumber"));
        oObject.PutString("DataOwnerGuid", strDooGuid);
        oObject.PutString("DatasetGuid", strDsetGuid);
        oObject.PutString("DatasetName", oDatasetMetadata.GetString("DatasetName"));
        oObject.PutString("Description", oDatasetMetadata.GetString("Description"));
        oObject.PutString("Keywords", oDatasetMetadata.GetString("Keywords"));
        oObject.PutUnsignedInt64("PublishDate", oDatasetMetadata.GetUnsignedInt64("PublishDate"));
        oObject.PutByte("PrivacyLevel", oDatasetMetadata.GetByte("PrivacyLevel"));
        oObject.PutString("JurisdictionalLimitations", oDatasetMetadata.GetString("JurisdictionalLimitations"));
        oObject.PutStructuredBuffer("Tables", oDatasetMetadata.GetStructuredBuffer("Tables"));
        bsoncxx::types::b_binary oObjectBlob
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
            oObject.GetSerializedBufferRawDataPtr()
        };
        bsoncxx::document::value oObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectBlob" << oObjectBlob
        << finalize;

        // Create a plain text object document
        bsoncxx::document::value oPlainTextObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectType" << GuidOfObjectType::eDataset
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access Dataset collection
        mongocxx::collection oDatasetCollection = oSailDatabase["Dataset"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the Dataset collection
            auto oResult = oDatasetCollection.insert_one(*poSession, oDsetDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access Object collection
                mongocxx::collection oObjectCollection = oSailDatabase["Object"];
                // Insert document in the Object collection
                oResult = oObjectCollection.insert_one(*poSession, oObjectDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    // Access PlainTextObjectBlob collection
                    mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                    // Insert document in the PlainTextObjectBlob collection
                    oResult = oPlainTextObjectCollection.insert_one(*poSession, oPlainTextObjectDocumentValue.view());
                    if (!oResult) {
                        std::cout << "Error while writing to the database." << std::endl;
                    }
                    else
                    {
                        dwStatus = 201;
                    }
                }
            }
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
        }
        catch (mongocxx::exception& e) 
        {
            std::cout << "Collection transaction exception: " << e.what() << std::endl;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DeleteDataset
 * @brief Delete the dataset from the database
 * @param[in] c_oRequest contains dataset guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::DeleteDataset(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strDsetGuid = c_oRequest.GetString("DatasetGuid");
        std::string strDooGuid = c_oRequest.GetString("DataOwnerGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
        {
            bsoncxx::stdx::optional<bsoncxx::document::value> oDsetDocument = oSailDatabase["Dataset"].find_one(document{}
                                                                                                                    << "DatasetGuid" << strDsetGuid
                                                                                                                    << "DataOwnerOrganizationGuid" << strDooGuid
                                                                                                                    << finalize);
            // Get PlainTextObjectBlobGuid and ObjectGuid
            if (bsoncxx::stdx::nullopt != oDsetDocument)
            {                                                                         
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDsetDocument->view()["PlainTextObjectBlobGuid"];
                if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
                {
                    std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                    bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                                    << "PlainTextObjectBlobGuid" <<  strPlainTextObjectBlobGuid
                                                                                                                                                    << finalize);
                    if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
                    {
                        bsoncxx::document::element oObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
                        if (oObjectGuid && oObjectGuid.type() == type::k_utf8)
                        {
                            std::string strObjectGuid = oObjectGuid.get_utf8().value.to_string();
                            // Delete record associated with strObjectGuid
                            oSailDatabase["Object"].delete_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize);
                            // Delete records associated with strPlainTextObjectBlobGuid
                            oSailDatabase["PlainTextObjectBlob"].delete_one(*poSession, document{} << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid << finalize);
                            // Delete document from Dataset collection associated with DatasetrGuid
                            oSailDatabase["Dataset"].delete_one(*poSession, document{} << "DatasetGuid" << strDsetGuid << "DataOwnerOrganizationGuid" << strDooGuid << finalize);
                        }
                    }
                }
            }
        };
        // Create a session and start the transaction
        mongocxx::client_session oSession = oClient->start_session();
        try 
        {
            oSession.with_transaction(oCallback);
            dwStatus = 200;
        }
        catch (mongocxx::exception & e)
        {
            std::cout << "Transaction exception: " << e.what() << std::endl;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}