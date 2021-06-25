/*********************************************************************************************
 *
 * @file DigitalContractFunctions.cpp
 * @author Shabana Akhtar Baig
 * @date 09 April 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "DatabaseManager.h"

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function DigitalContractBranchExists
 * @brief Given an organization guid fetch digital contract event guid, if exists
 * @param[in] c_oRequest contains the organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing digital contract guid
 *
 ********************************************************************************************/

 std::vector<Byte> __thiscall DatabaseManager::DigitalContractBranchExists(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Get root event guid
        StructuredBuffer oGetRootRequest;
        oGetRootRequest.PutString("ParentGuid", "{00000000-0000-0000-0000-000000000000}");
        oGetRootRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
        StructuredBuffer oFilters;
        oGetRootRequest.PutStructuredBuffer("Filters", oFilters);
        StructuredBuffer oRootEvent = StructuredBuffer(this->GetListOfEvents(oGetRootRequest)).GetStructuredBuffer("ListOfEvents");
        _ThrowBaseExceptionIf((0 == oRootEvent.GetNamesOfElements().size()), "Root event does not exist", nullptr);
        std::string strRootEventGuid = oRootEvent.GetStructuredBuffer(oRootEvent.GetNamesOfElements()[0].c_str()).GetGuid("EventGuid").ToString(eHyphensAndCurlyBraces);

        // Check if DC branch event exists
        StructuredBuffer oGetDCRequest;
        oGetDCRequest.PutString("ParentGuid", strRootEventGuid);
        oGetDCRequest.PutString("OrganizationGuid", c_oRequest.GetString("OrganizationGuid"));
        oGetDCRequest.PutStructuredBuffer("Filters", c_oRequest.GetStructuredBuffer("Filters"));
        StructuredBuffer oBranchEvent(this->GetListOfEvents(oGetDCRequest));
        StructuredBuffer oListOfEvents(oBranchEvent.GetStructuredBuffer("ListOfEvents"));
        if (0 < oListOfEvents.GetNamesOfElements().size())
        {
            StructuredBuffer oEvent(oListOfEvents.GetStructuredBuffer(oListOfEvents.GetNamesOfElements()[0].c_str()));
            oResponse.PutString("DCEventGuid", oEvent.GetGuid("EventGuid").ToString(eHyphensAndCurlyBraces));
            dwStatus = 200;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        oResponse.Clear();
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oResponse.Clear();
    }
    
    // Add transaction status
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function ListDigitalContracts
 * @brief Fetch list of all digital contracts associated with the user's organization
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing list of digital contracts associated with the user's organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListDigitalContracts(
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
        // Fetch all digital contract records associated with a researcher's or data owner's organization
        mongocxx::cursor oDcRecords = oSailDatabase["DigitalContract"].find(document{} 
                                                                            << "$or" << open_array << open_document
                                                                            << "ResearcherOrganization" << c_oRequest.GetString("UserOrganization")
                                                                            << close_document << open_document
                                                                            << "DataOwnerOrganization" << c_oRequest.GetString("UserOrganization") 
                                                                            << close_document << close_array
                                                                            << finalize);
        // Loop through returned documents and add information to the list
        StructuredBuffer oListOfDigitalContracts;
        for (auto&& oDocumentView : oDcRecords)
        {
            bsoncxx::document::element oDcGuid = oDocumentView["DigitalContractGuid"];
            bsoncxx::document::element oRoGuid = oDocumentView["ResearcherOrganization"];
            bsoncxx::document::element oDooGuid = oDocumentView["DataOwnerOrganization"];
            if ((oDcGuid && oDcGuid.type() == type::k_utf8) && (oDooGuid && oDooGuid.type() == type::k_utf8) && (oRoGuid && oRoGuid.type() == type::k_utf8))
            {
                std::string strDcGuid = oDcGuid.get_utf8().value.to_string();
                std::string strDooGuid = oDooGuid.get_utf8().value.to_string();
                std::string strRoGuid = oRoGuid.get_utf8().value.to_string();
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
                            // Fetch digital contract from the Object collection associated with the digital contract guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject;
                                    oObject.PutStructuredBuffer("DigitalContract", StructuredBuffer(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size));
                                    // Get doo and ro organization names and add them to the response
                                    StructuredBuffer oRoName = this->GetOrganizationName(strRoGuid);
                                    if (200 == oRoName.GetDword("Status")) 
                                    {
                                        oObject.PutString("ROName", oRoName.GetString("OrganizationName"));
                                    }
                                    StructuredBuffer oDooName = this->GetOrganizationName(strDooGuid);
                                    if (200 == oDooName.GetDword("Status")) 
                                    {
                                        oObject.PutString("DOOName", oDooName.GetString("OrganizationName"));
                                    }
                                    // Add doo and ro guids
                                    oObject.PutString("ResearcherOrganization", strRoGuid);
                                    oObject.PutString("DataOwnerOrganization", strDooGuid);
                                    oListOfDigitalContracts.PutStructuredBuffer(strDcGuid.c_str(), oObject);
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("DigitalContracts", oListOfDigitalContracts);
        dwStatus = 200;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @function PullDigitalContract
 * @brief Fetch the digital contract information
 * @param[in] c_oRequest contains the digital contract guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the digital contract information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::PullDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the digital contract record
        bsoncxx::stdx::optional<bsoncxx::document::value> oDcDocument = oSailDatabase["DigitalContract"].find_one(document{}
                                                                                                                    << "DigitalContractGuid" << strDcGuid
                                                                                                                    << "$or" << open_array << open_document
                                                                                                                    << "ResearcherOrganization" << c_oRequest.GetString("UserOrganization")
                                                                                                                    << close_document << open_document
                                                                                                                    << "DataOwnerOrganization" << c_oRequest.GetString("UserOrganization") 
                                                                                                                    << close_document << close_array 
                                                                                                                    << finalize);
        if (bsoncxx::stdx::nullopt != oDcDocument)
        {                                                                                                           
            bsoncxx::document::element oDcGuid = oDcDocument->view()["DigitalContractGuid"];
            bsoncxx::document::element oRoGuid = oDcDocument->view()["ResearcherOrganization"];
            bsoncxx::document::element oDooGuid = oDcDocument->view()["DataOwnerOrganization"];
            if ((oDcGuid && oDcGuid.type() == type::k_utf8) && (oDooGuid && oDooGuid.type() == type::k_utf8) && (oRoGuid && oRoGuid.type() == type::k_utf8))
            {
                std::string strDcGuid = oDcGuid.get_utf8().value.to_string();
                std::string strDooGuid = oDooGuid.get_utf8().value.to_string();
                std::string strRoGuid = oRoGuid.get_utf8().value.to_string();
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDcDocument->view()["PlainTextObjectBlobGuid"];
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
                            // Fetch the digital contract from the Object collection associated with the digital contract guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oResponse.PutStructuredBuffer("DigitalContract", oObject);
                                    // Get doo and ro organization names and add them to the response
                                    StructuredBuffer oRoName = this->GetOrganizationName(strRoGuid);
                                    if (200 == oRoName.GetDword("Status")) 
                                    {
                                        oResponse.PutString("ROName", oRoName.GetString("OrganizationName"));
                                    }
                                    StructuredBuffer oDooName = this->GetOrganizationName(strDooGuid);
                                    if (200 == oDooName.GetDword("Status")) 
                                    {
                                        oResponse.PutString("DOOName", oDooName.GetString("OrganizationName"));
                                    }
                                    // Add doo and ro guids
                                    oResponse.PutString("ResearcherOrganization", strRoGuid);
                                    oResponse.PutString("DataOwnerOrganization", strDooGuid);
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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
 * @function RegisterDigitalContract
 * @brief Take in full EOSB and register a digital contract
 * @param[in] c_oRequest contains the digital contract information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns transaction status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Create guids for the documents
        Guid oObjectGuid, oPlainTextObjectBlobGuid;
        
        // Create a digital contract document
        bsoncxx::document::value oDcDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "DigitalContractGuid" << c_oRequest.GetString("DigitalContractGuid")
        << "ResearcherOrganization" << c_oRequest.GetString("ResearcherOrganization")
        << "DataOwnerOrganization" << c_oRequest.GetString("DataOwnerOrganization")
        << finalize;

        // Create an object document
        StructuredBuffer oObject;
        oObject.PutBuffer("DigitalContractBlob", c_oRequest.GetBuffer("DigitalContractBlob"));
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
        << "ObjectType" << eDigitalContract
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access DigitalContract collection
        mongocxx::collection oDigitalContractCollection = oSailDatabase["DigitalContract"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the DigitalContract collection
            auto oResult = oDigitalContractCollection.insert_one(*poSession, oDcDocumentValue.view());
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
        ::RegisterException(oException, __func__, __LINE__);
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
 * @function UpdateDigitalContract
 * @brief Update the digital contract when a data owner accepts the digital contract
 * @param[in] c_oRequest contains the update digital contract blob
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction and instructions of what happens next
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateDigitalContract(
    _in const StructuredBuffer & c_oRequest
    )
{
    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the digital contract record
        bsoncxx::stdx::optional<bsoncxx::document::value> oDcDocument = oSailDatabase["DigitalContract"].find_one(document{}
                                                                                                                    << "DigitalContractGuid" << strDcGuid
                                                                                                                    << finalize);
        if (bsoncxx::stdx::nullopt != oDcDocument)
        {
            bsoncxx::document::element oDcGuid = oDcDocument->view()["DigitalContractGuid"];
            if (oDcGuid && oDcGuid.type() == type::k_utf8)
            {
                std::string strDcGuid = oDcGuid.get_utf8().value.to_string();
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDcDocument->view()["PlainTextObjectBlobGuid"];
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
                            // Update the digital contract in the Object collection associated with the digital contract guid
                            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
                            {
                                // Create object blob
                                StructuredBuffer oObject;
                                oObject.PutBuffer("DigitalContractBlob", c_oRequest.GetBuffer("DigitalContractBlob"));
                                bsoncxx::types::b_binary oUpdatedDcBlob
                                {
                                    bsoncxx::binary_sub_type::k_binary,
                                    uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
                                    oObject.GetSerializedBufferRawDataPtr()
                                };
                                oSailDatabase["Object"].update_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize,
                                                                    document{} << "$set" << open_document <<
                                                                    "ObjectBlob" << oUpdatedDcBlob << close_document << finalize);
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
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
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

