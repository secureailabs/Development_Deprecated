/*********************************************************************************************
 *
 * @file DatasetFamilyFunctions.cpp
 * @author David Gascon
 * @date 01 November 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2021 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterDatasetFamily
 * @brief Save a Dataset family in the database
 * @param[in] c_oRequest contains the information for the new dataset
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing the response
 *
 ********************************************************************************************/
// Register a dataset family
std::vector<Byte> __thiscall DatabaseManager::RegisterDatasetFamily(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;
    try
    {
        // Get the object to write to the database
        StructuredBuffer oObject = c_oRequest.GetStructuredBuffer("DatasetFamily");

        Guid oObjectGuid, oPlainTextObjectBlobGuid;

        bsoncxx::document::value oDatasetFamilyDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "DatasetFamilyGuid" << oObject.GetString("DatasetFamilyGuid")
        << "DatasetFamilyOwnerOrganizationGuid" << oObject.GetString("DatasetFamilyOwnerGuid")
        << finalize;

        // Create a blob for the serialized structured buffer
        bsoncxx::types::b_binary oObjectBlob
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
            oObject.GetSerializedBufferRawDataPtr()
        };

        // Create a document that has our object's GUID and its serialized data
        bsoncxx::document::value oObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectBlob" << oObjectBlob
        << finalize;

        // Create a plain text object document - telling us this GUID is a dataset family
        bsoncxx::document::value oPlainTextObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectType" << GuidObjectType::eDatasetFamily
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access Dataset Family collection
        mongocxx::collection oDatasetFamilyCollection = oSailDatabase["DatasetFamily"];

        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the Dataset family collection
            auto oResult = oDatasetFamilyCollection.insert_one(*poSession, oDatasetFamilyDocumentValue.view());
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
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ListDatasetFamilies
 * @brief Get a list of all the dataset families associated with an organization
 * @param[in] c_oRequest contains the information for the query
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing the response
 *
 ********************************************************************************************/
// Fetch list of all dataset families associated with the user's organization
std::vector<Byte> __thiscall DatabaseManager::ListDatasetFamilies(
    _in const StructuredBuffer &c_oRequest
    )
{
    __DebugFunction();

    Dword dwStatus;
    StructuredBuffer oResponse;
    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Fetch all digital contract records associated with a researcher's or data owner's organization
    mongocxx::cursor oDatasetFamilyRecords = oSailDatabase["DatasetFamily"].find({});

    // Loop through returned documents and add information to the list
    StructuredBuffer oListOfDatasetFamilies;
    for (auto&& oDocumentView : oDatasetFamilyRecords)
    {
        bsoncxx::document::element oDatasetFamilyGuid = oDocumentView["DatasetFamilyGuid"];
        bsoncxx::document::element oDatasetFamilyOwnerGuid = oDocumentView["DatasetFamilyOwnerOrganizationGuid"];
        if ((oDatasetFamilyGuid && oDatasetFamilyGuid.type() == type::k_utf8) && (oDatasetFamilyOwnerGuid && oDatasetFamilyOwnerGuid.type() == type::k_utf8) )
        {
            std::string strDatasetFamilyGuid = oDatasetFamilyGuid.get_utf8().value.to_string();
            std::string strDatasetFamilyOwnerGuid = oDatasetFamilyOwnerGuid.get_utf8().value.to_string();

            bsoncxx::document::element oPlainTextObjectBlobGuid = oDocumentView["PlainTextObjectBlobGuid"];
            if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{}
                                                                                                                                               << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid
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
                                StructuredBuffer oDatasetFamilyInformation;
                                StructuredBuffer oOrganizationName = this->GetOrganizationName(oObject.GetString("DatasetFamilyOwnerGuid"));
                                oDatasetFamilyInformation.PutString("DatasetFamilyTitle", oObject.GetString("DatasetFamilyTitle"));
                                oDatasetFamilyInformation.PutString("DatasetFamilyOwnerGuid", oObject.GetString("DatasetFamilyOwnerGuid"));
                                if ( oObject.IsElementPresent("DatasetFamilyTags", ANSI_CHARACTER_STRING_VALUE_TYPE) )
                                {
                                    oDatasetFamilyInformation.PutString("DatasetFamilyTags", oObject.GetString("DatasetFamilyTags"));
                                }
                                oDatasetFamilyInformation.PutBoolean("DatasetFamilyActive", oObject.GetBoolean("DatasetFamilyActive"));
                                if ( 200 == oOrganizationName.GetDword("Status") )
                                {
                                    oDatasetFamilyInformation.PutString("OrganizationName", oOrganizationName.GetString("OrganizationName"));
                                }
                                else
                                {
                                    oDatasetFamilyInformation.PutString("OrganizationName", "");
                                }

                                oListOfDatasetFamilies.PutStructuredBuffer(strDatasetFamilyGuid.c_str(), oDatasetFamilyInformation);
                            }
                        }
                    }
                }
            }
        }
    }
    oResponse.PutStructuredBuffer("DatasetFamilies", oListOfDatasetFamilies);
    dwStatus = 200;
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function UpdateDatasetFamily
 * @brief Update a dataset family's metadata
 * @param[in] c_oRequest contains the information for the query
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing the response
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatabaseManager::UpdateDatasetFamily(
    _in const StructuredBuffer &c_oRequest
    )
{
    __DebugFunction();

    Dword dwStatus{404};
    StructuredBuffer oResponse;

    try
    {
        StructuredBuffer oDatasetFamilyInformation = c_oRequest.GetStructuredBuffer("DatasetFamily");
        std::string strDatasetFamilyGuid = oDatasetFamilyInformation.GetString("DatasetFamilyGuid");
        std::string strOrganizationGuid = oDatasetFamilyInformation.GetString("DatasetFamilyOwnerGuid");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        bsoncxx::stdx::optional<bsoncxx::document::value> oDatasetFamilyDocument = oSailDatabase["DatasetFamily"].find_one(document{}
                                                                                                                << "$and" << open_array << open_document
                                                                                                                << "DatasetFamilyGuid" << strDatasetFamilyGuid
                                                                                                                << close_document << open_document
                                                                                                                << "DatasetFamilyOwnerOrganizationGuid" << strOrganizationGuid
                                                                                                                << close_document << close_array
                                                                                                                << finalize);

        if (bsoncxx::stdx::nullopt != oDatasetFamilyDocument)
        {
            bsoncxx::document::element oPlainTextObjectBlobGuid = oDatasetFamilyDocument->view()["PlainTextObjectBlobGuid"];
            if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{}
                                                                                                                                               << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid
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
                            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session *poSession)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    bsoncxx::types::b_binary oNewObjectBlob{
                                        bsoncxx::binary_sub_type::k_binary,
                                        uint32_t(oDatasetFamilyInformation.GetSerializedBufferRawDataSizeInBytes()),
                                        oDatasetFamilyInformation.GetSerializedBufferRawDataPtr()};

                                    // Update the object blob in the database
                                    oSailDatabase["Object"].update_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize,
                                                                    document{} << "$set" << open_document << "ObjectBlob" << oNewObjectBlob << close_document << finalize);
                                }
                            };

                            mongocxx::client_session oSession = oClient->start_session();
                            try
                            {
                                oSession.with_transaction(oCallback);
                                dwStatus = 200;
                            }
                            catch (mongocxx::exception &e)
                            {
                                std::cout << "Collection transaction exception: " << e.what() << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function UpdateDatasetFamily
 * @brief Update a dataset family's metadata
 * @param[in] c_oRequest contains the information for the query
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing the response
 *
 ********************************************************************************************/
std::vector<Byte> __thiscall DatabaseManager::PullDatasetFamily(
    _in const StructuredBuffer &c_oRequest
    )
{
    __DebugFunction();

    Dword dwStatus{404};
    StructuredBuffer oResponse;

    try
    {
        std::string strDatasetFamilyGuid = c_oRequest.GetString("DatasetFamilyGuid");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        bsoncxx::stdx::optional<bsoncxx::document::value> oDatasetFamilyDocument = oSailDatabase["DatasetFamily"].find_one(document{}
                                                                                                                << "DatasetFamilyGuid" << strDatasetFamilyGuid
                                                                                                                << finalize);

        if (bsoncxx::stdx::nullopt != oDatasetFamilyDocument)
        {
            bsoncxx::document::element oPlainTextObjectBlobGuid = oDatasetFamilyDocument->view()["PlainTextObjectBlobGuid"];
            if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
            {
                std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{}
                                                                                                                                               << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid
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
                            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session *poSession)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oResponse.PutStructuredBuffer("DatasetFamily", oObject);
                                    dwStatus = 200;
                                }
                            };

                            mongocxx::client_session oSession = oClient->start_session();
                            try
                            {
                                oSession.with_transaction(oCallback);
                            }
                            catch (mongocxx::exception &e)
                            {
                                std::cout << "Collection transaction exception: " << e.what() << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

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

std::vector<Byte> __thiscall DatabaseManager::DeleteDatasetFamily(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strDatasetFamilyGuid = c_oRequest.GetString("DatasetFamilyGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
        {
            bsoncxx::stdx::optional<bsoncxx::document::value> oDatasetFamilyDocument = oSailDatabase["DatasetFamily"].find_one(document{}
                                                                                                                << "$and" << open_array << open_document
                                                                                                                << "DatasetFamilyGuid" << strDatasetFamilyGuid
                                                                                                                << close_document << open_document
                                                                                                                << "DatasetFamilyOwnerOrganizationGuid" << strOrganizationGuid
                                                                                                                << close_document << close_array
                                                                                                                << finalize);

            if (bsoncxx::stdx::nullopt != oDatasetFamilyDocument)
            {
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDatasetFamilyDocument->view()["PlainTextObjectBlobGuid"];
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
                            oSailDatabase["DatasetFamily"].delete_one(*poSession, document{} << "DatasetGuid" << strDatasetFamilyGuid << "DatasetFamilyOwnerOrganizationGuid" << strOrganizationGuid << finalize);
                            dwStatus = 200;
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
        }
        catch (mongocxx::exception & e)
        {
            std::cout << "Transaction exception: " << e.what() << std::endl;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        oResponse.Clear();
    }

    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}
