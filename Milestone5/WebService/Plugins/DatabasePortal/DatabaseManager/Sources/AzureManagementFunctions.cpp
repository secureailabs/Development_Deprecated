/*********************************************************************************************
 *
 * @file AzureManagementFunctions.cpp
 * @author Shabana Akhtar Baig
 * @date 21 Jul 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ListAzureTemplates
 * @brief Fetch the list of all available azure settings templates of an organization
 * @param[in] c_oRequest contains the organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the list of all available azure settings templates of an organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListAzureTemplates(
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
        // Fetch all azure templates of an organization
        mongocxx::cursor oTemplateRecords = oSailDatabase["AzureSettingsTemplates"].find(document{} 
                                                                                << "OrganizationGuid" << c_oRequest.GetString("OrganizationGuid")
                                                                                << finalize);
        // Loop through returned documents and add them to the list of azure templates
        StructuredBuffer oListOfAzureTemplates;
        for (auto&& oDocumentView : oTemplateRecords)
        {  
            bsoncxx::document::element oTemplateGuid = oDocumentView["TemplateGuid"];
            if (oTemplateGuid && oTemplateGuid.type() == type::k_utf8)
            {                   
                std::string strTemplateGuid = oTemplateGuid.get_utf8().value.to_string();                                                             
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
                                    oObject.RemoveElement("Secret");
                                    oListOfAzureTemplates.PutStructuredBuffer(strTemplateGuid.c_str(), oObject);
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("Templates", oListOfAzureTemplates);
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
 * @function PullAzureSettingsTemplate
 * @brief Fetch the azure settings template
 * @param[in] c_oRequest contains the template guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the azure settings template
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::PullAzureTemplate(
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
        // Fetch the azure settings template of an organization
        bsoncxx::stdx::optional<bsoncxx::document::value> oTemplateDocument = oSailDatabase["AzureSettingsTemplates"].find_one(document{}
                                                                                                                    << "TemplateGuid" << c_oRequest.GetString("TemplateGuid")
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
                        bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                        if (bsoncxx::stdx::nullopt != oObjectDocument)
                        {
                            bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                            if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                            {
                                StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                oResponse.PutStructuredBuffer("Template", oObject);
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
 * @function RegisterAzureSettingsTemplate
 * @brief Store azure settings template in the database
 * @param[in] c_oRequest contains the azure settings template
 * @throw BaseException Error StructuredBuffer element not found
 * @returns transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterAzureTemplate(
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
        // Ge the Tempalte Guid
        std::string strTemplateGuid = c_oRequest.GetString("TemplateGuid");

        // Create guids for the documents
        Guid oObjectGuid, oPlainTextObjectBlobGuid;

        // Create an Azure Template document
        bsoncxx::document::value oTemplateDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "TemplateGuid" << strTemplateGuid
        << "OrganizationGuid" << strOrganizationGuid
        << finalize;

        // Create an object document
        StructuredBuffer oTemplateData = c_oRequest.GetStructuredBuffer("TemplateData");
        StructuredBuffer oObject;
        oObject.PutString("TemplateGuid", strTemplateGuid);
        oObject.PutString("OrganizationGuid", strOrganizationGuid);
        oObject.PutString("Name", oTemplateData.GetString("Name"));
        oObject.PutString("Description", oTemplateData.GetString("Description"));
        oObject.PutString("SubscriptionID", oTemplateData.GetString("SubscriptionID"));
        oObject.PutString("Secret", oTemplateData.GetString("Secret"));
        oObject.PutString("TenantID", oTemplateData.GetString("TenantID"));
        oObject.PutString("ApplicationID", oTemplateData.GetString("ApplicationID"));
        oObject.PutString("ResourceGroup", oTemplateData.GetString("ResourceGroup"));
        oObject.PutString("VirtualNetwork", oTemplateData.GetString("VirtualNetwork"));
        oObject.PutString("NetworkSecurityGroup", oTemplateData.GetString("NetworkSecurityGroup"));
        oObject.PutString("VirtualMachineImage", oTemplateData.GetString("VirtualMachineImage"));
        oObject.PutString("HostRegion", oTemplateData.GetString("HostRegion"));
        oObject.PutDword("State", oTemplateData.GetDword("State"));
        oObject.PutString("Note", oTemplateData.GetString("Note"));
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
        << "ObjectType" << GuidOfObjectType::eAzureSettingsTemplate
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access Azure Settings Template collection
        mongocxx::collection oAzureTemplateCollection = oSailDatabase["AzureSettingsTemplates"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the Azure Settings Template collection
            auto oResult = oAzureTemplateCollection.insert_one(*poSession, oTemplateDocumentValue.view());
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
 * @function UpdateAzureSettingsTemplate
 * @brief Update the azure setting template in the database
 * @param[in] c_oRequest contains template guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateAzureTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strTemplateGuid = c_oRequest.GetString("TemplateGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the document in AzureSettingsTemplates collection associated with TemplateGuid
        bsoncxx::stdx::optional<bsoncxx::document::value> oTemplateDocument = oSailDatabase["AzureSettingsTemplates"].find_one(document{} 
                                                                                                                                << "TemplateGuid" << strTemplateGuid 
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
                        // Update the azure template in the Object collection associated with the template guid
                        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
                        {
                            // Create object blob
                            StructuredBuffer oTemplateData = c_oRequest.GetStructuredBuffer("TemplateData");
                            StructuredBuffer oUpdatedObject;
                            oUpdatedObject.PutString("TemplateGuid", strTemplateGuid);
                            oUpdatedObject.PutString("OrganizationGuid", strOrganizationGuid);
                            oUpdatedObject.PutString("Name", oTemplateData.GetString("Name"));
                            oUpdatedObject.PutString("Description", oTemplateData.GetString("Description"));
                            oUpdatedObject.PutString("SubscriptionID", oTemplateData.GetString("SubscriptionID"));
                            if (true == oTemplateData.IsElementPresent("Secret", ANSI_CHARACTER_STRING_VALUE_TYPE))
                            {
                                oUpdatedObject.PutString("Secret", oTemplateData.GetString("Secret"));
                            }
                            else
                            {
                                // Get the secret from the current object
                                // Fetch the template object from the Object collection associated with the object guid
                                bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                                if (bsoncxx::stdx::nullopt != oObjectDocument)
                                {
                                    bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                    if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                    {
                                        StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                        oUpdatedObject.PutString("Secret", oObject.GetString("Secret"));
                                    }
                                }
                            }
                            oUpdatedObject.PutString("TenantID", oTemplateData.GetString("TenantID"));
                            oUpdatedObject.PutString("ApplicationID", oTemplateData.GetString("ApplicationID"));
                            oUpdatedObject.PutString("ResourceGroup", oTemplateData.GetString("ResourceGroup"));
                            oUpdatedObject.PutString("VirtualNetwork", oTemplateData.GetString("VirtualNetwork"));
                            oUpdatedObject.PutString("NetworkSecurityGroup", oTemplateData.GetString("NetworkSecurityGroup"));
                            oUpdatedObject.PutString("VirtualMachineImage", oTemplateData.GetString("VirtualMachineImage"));
                            oUpdatedObject.PutString("HostRegion", oTemplateData.GetString("HostRegion"));
                            oUpdatedObject.PutDword("State", oTemplateData.GetDword("State"));
                            oUpdatedObject.PutString("Note", oTemplateData.GetString("Note"));
                            bsoncxx::types::b_binary oUpdatedTemplateBlob
                            {
                                bsoncxx::binary_sub_type::k_binary,
                                uint32_t(oUpdatedObject.GetSerializedBufferRawDataSizeInBytes()),
                                oUpdatedObject.GetSerializedBufferRawDataPtr()
                            };
                            oSailDatabase["Object"].update_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize,
                                                                document{} << "$set" << open_document <<
                                                                "ObjectBlob" << oUpdatedTemplateBlob << close_document << finalize);
                        };
                        // Create a session and start the transaction
                        mongocxx::client_session oSession = oClient->start_session();
                        try 
                        {
                            oSession.with_transaction(oCallback);
                            dwStatus = 200;
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
 * @function DeleteAzureTemplate
 * @brief Delete the Azure template from the database
 * @param[in] c_oRequest contains template guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::DeleteAzureTemplate(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strTemplateGuid = c_oRequest.GetString("TemplateGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
        {
            bsoncxx::stdx::optional<bsoncxx::document::value> oTemplateDocument = oSailDatabase["AzureSettingsTemplates"].find_one(document{}
                                                                                                                    << "TemplateGuid" << strTemplateGuid
                                                                                                                    << "OrganizationGuid" << strOrganizationGuid 
                                                                                                                    << finalize);
            // Get PlainTextObjectBlobGuid and ObjectGuid
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
                            // Delete record associated with strObjectGuid
                            oSailDatabase["Object"].delete_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize);
                            // Delete records associated with strPlainTextObjectBlobGuid
                            oSailDatabase["PlainTextObjectBlob"].delete_one(*poSession, document{} << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid << finalize);
                            // Delete document from AzureSettingsTemplates collection associated with TemplateGuid
                            oSailDatabase["AzureSettingsTemplates"].delete_one(*poSession, document{} 
                                                                                << "TemplateGuid" << strTemplateGuid 
                                                                                << "OrganizationGuid" << strOrganizationGuid 
                                                                                << finalize);
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