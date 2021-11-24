/*********************************************************************************************
 *
 * @file RemoteDataConnectorFunctions.cpp
 * @author Shabana Akhtar Baig
 * @date 30 Jul 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ListRemoteDataConnectors
 * @brief Fetch the list of all available remote data connectors for an organization
 * @param[in] c_oRequest contains the organization guid 
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the list of all available remote data connectors
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListRemoteDataConnectors(
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
        // Fetch all remote data connectors for an organization
        mongocxx::cursor oConnectorRecords = oSailDatabase["RemoteDataConnector"].find(document{} 
                                                                                << "OrganizationGuid" << c_oRequest.GetString("OrganizationGuid")
                                                                                << finalize);
        // Loop through returned documents and add them to the list of data connectors
        StructuredBuffer oListOfConnectors;
        for (auto&& oDocumentView : oConnectorRecords)
        {  
            bsoncxx::document::element oConnectorGuid = oDocumentView["RemoteDataConnectorGuid"];
            if (oConnectorGuid && oConnectorGuid.type() == type::k_utf8)
            {                   
                std::string strConnectorGuid = oConnectorGuid.get_utf8().value.to_string();                                                             
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
                            // Fetch the template from the Object collection associated with the template guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oListOfConnectors.PutStructuredBuffer(strConnectorGuid.c_str(), oObject);
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("Connectors", oListOfConnectors);
        dwStatus = 200;
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
 * @function PullRemoteDataConnector
 * @brief Fetch the remote data connector metadata
 * @param[in] c_oRequest contains the connector guid and organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the remote data connector metadata
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::PullRemoteDataConnector(
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
        // Fetch the remote data connector metadata
        bsoncxx::stdx::optional<bsoncxx::document::value> oConnectorDocument = oSailDatabase["RemoteDataConnector"].find_one(document{}
                                                                                                                    << "RemoteDataConnectorGuid" << c_oRequest.GetString("RemoteDataConnectorGuid")
                                                                                                                    << "OrganizationGuid" << c_oRequest.GetString("OrganizationGuid")
                                                                                                                    << finalize);
        if (bsoncxx::stdx::nullopt != oConnectorDocument)
        {                                                                         
            bsoncxx::document::element oPlainTextObjectBlobGuid = oConnectorDocument->view()["PlainTextObjectBlobGuid"];
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
                                oResponse.PutStructuredBuffer("Connector", oObject);
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
 * @function RegisterRemoteDataConnector
 * @brief Store remote data connector's metadata in the database
 * @param[in] c_oRequest contains the remote data connector's metadata
 * @throw BaseException Error StructuredBuffer element not found
 * @returns transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterRemoteDataConnector(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Get the organization guid
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Get the connector's guid
        std::string strConnectorGuid = c_oRequest.GetString("RemoteDataConnectorGuid");
        // Create guids for the documents
        Guid oObjectGuid, oPlainTextObjectBlobGuid;
        
        // Create a connector document
        bsoncxx::document::value oConnectorDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "RemoteDataConnectorGuid" << strConnectorGuid
        << "OrganizationGuid" << strOrganizationGuid
        << finalize;

        // Create an object document
        StructuredBuffer oObject;
        oObject.PutString("RemoteDataConnectorGuid", strConnectorGuid);
        oObject.PutStructuredBuffer("Datasets", c_oRequest.GetStructuredBuffer("Datasets"));
        oObject.PutString("OrganizationGuid", strOrganizationGuid);
        oObject.PutString("UserGuid", c_oRequest.GetString("UserGuid"));
        oObject.PutString("Version", c_oRequest.GetString("Version"));
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
        << "ObjectType" << GuidObjectType::eRemoteDataConnectorVirtualMachine
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access Remote Data Connector collection
        mongocxx::collection oRemoteDataConnectorCollection = oSailDatabase["RemoteDataConnector"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the Remote Data Connector collection
            auto oResult = oRemoteDataConnectorCollection.insert_one(*poSession, oConnectorDocumentValue.view());
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
 * @function UpdateRemoteDataConnector
 * @brief Update the remote data connector in the database
 * @param[in] c_oRequest contains connector guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateRemoteDataConnector(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strConnectorGuid = c_oRequest.GetString("RemoteDataConnectorGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the document in RemoteDataConnector collection associated with ConnectorGuid
        bsoncxx::stdx::optional<bsoncxx::document::value> oTemplateDocument = oSailDatabase["RemoteDataConnector"].find_one(document{} 
                                                                                                                                << "RemoteDataConnectorGuid" << strConnectorGuid 
                                                                                                                                << "OrganizationGuid" << strOrganizationGuid 
                                                                                                                                << finalize);
        if (bsoncxx::stdx::nullopt != oTemplateDocument)
        {
            bsoncxx::document::element oPlainTextObjectBlobGuid = oTemplateDocument->view()["PlainTextObjectBlobGuid"];
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
                        // Update the connector metadata in the Object collection associated with the connector guid
                        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
                        {
                            // Get the current object
                            // Update the object blob
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oObject.PutStructuredBuffer("Datasets", c_oRequest.GetStructuredBuffer("Datasets"));
                                    oObject.PutString("Version", c_oRequest.GetString("Version"));
                                    bsoncxx::types::b_binary oUpdatedTemplateBlob
                                    {
                                        bsoncxx::binary_sub_type::k_binary,
                                        uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
                                        oObject.GetSerializedBufferRawDataPtr()
                                    };
                                    oSailDatabase["Object"].update_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize,
                                                                        document{} << "$set" << open_document <<
                                                                        "ObjectBlob" << oUpdatedTemplateBlob << close_document << finalize);
                                    dwStatus = 200;
                                }
                            }
                            else
                            {
                                dwStatus = 404;
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