/*********************************************************************************************
 *
 * @file AuditLogFunctions.cpp
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
 * @function GetListOfEvents
 * @brief Fetch confidential user record from the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing confidential user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetListOfEvents(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        StructuredBuffer oListOfEvents;
        std::string strParentGuid = c_oRequest.GetString("ParentGuid");
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        StructuredBuffer oFilters(c_oRequest.GetStructuredBuffer("Filters"));

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get root event if parent guid is "{00000000-0000-0000-0000-000000000000}"
        // Otherwise return all the child events for the given parent guid
        if ("{00000000-0000-0000-0000-000000000000}" == strParentGuid)
        {
            bsoncxx::stdx::optional<bsoncxx::document::value> oAuditLogDocument = oSailDatabase["AuditLog"].find_one(document{} 
                                                                                                                << "ParentGuid" << strParentGuid 
                                                                                                                << "OrganizationGuid" << strOrganizationGuid
                                                                                                                << finalize);
            if (bsoncxx::stdx::nullopt != oAuditLogDocument)
            {
                this->GetEventObjectBlob(oSailDatabase, oFilters, oAuditLogDocument->view(), &oListOfEvents);
            }
        }
        else 
        {
            // Fetch events from the AuditLog collection associated with a parent guid
            mongocxx::cursor oAuditLogCursor = oSailDatabase["AuditLog"].find(document{} 
                                                                                << "ParentGuid" << strParentGuid 
                                                                                << "OrganizationGuid" << strOrganizationGuid
                                                                                << finalize);
            // Parse all returned documents, apply filters, and add to the structured buffer containing the list of events
            for (auto&& oDocumentView : oAuditLogCursor)
            {
                this->GetEventObjectBlob(oSailDatabase, oFilters, oDocumentView, &oListOfEvents);
            }
        }
        oResponse.PutStructuredBuffer("ListOfEvents", oListOfEvents);
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

    // Send back status of the transaction and list of events
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetEventObjectBlob
 * @brief Apply filters and add logs to the listofevents
 * @param[in] oDocumentView contains mongo document view
 * @param[out] poListOfEvents contains pointer to the structured buffer containing list of events
 * @throw BaseException Error StructuredBuffer element not found
 *
 ********************************************************************************************/

void __thiscall DatabaseManager::GetEventObjectBlob(
    _in const mongocxx::database & c_oSailDatabase,
    _in const StructuredBuffer & c_oFilters,
    _in const bsoncxx::document::view & c_oDocumentView,
    _out StructuredBuffer * poListOfEvents
    )
{
    __DebugFunction();

    // Get values from the document and generate the event log StructuredBuffer
    StructuredBuffer oEvent;
    bsoncxx::document::element strPlainTextObjectBlobGuid = c_oDocumentView["PlainTextObjectBlobGuid"];
    bsoncxx::document::element strEventGuid = c_oDocumentView["EventGuid"];
    bsoncxx::document::element strOrganizationGuid = c_oDocumentView["OrganizationGuid"];
    bsoncxx::document::element fIsLeaf = c_oDocumentView["IsLeaf"];

    // Get the PlainTextObjectBlob and then get the Object blob
    // Apply filters if any
    // If the object is not filtered out then add the audit log information to the Event structured buffer
    bool fAddToListOfEvents = false;
    uint32_t unObjectSequenceNumber = 0;
    if (strPlainTextObjectBlobGuid && strPlainTextObjectBlobGuid.type() == type::k_utf8)
    {
        Guid oPlainTextObjectBlobGuid(strPlainTextObjectBlobGuid.get_utf8().value.to_string().c_str());
        // Fetch events from the PlainTextObjectBlob collection associated with the event guid
        bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectBlobDocument = c_oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                                        << "PlainTextObjectBlobGuid" <<  oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces) 
                                                                                                                                        << finalize);
        if (bsoncxx::stdx::nullopt != oPlainTextObjectBlobDocument)
        {
            bsoncxx::document::element strObjectGuid = oPlainTextObjectBlobDocument->view()["ObjectGuid"];
            if (strObjectGuid && strObjectGuid.type() == type::k_utf8)
            {
                Guid oObjectGuid(strObjectGuid.get_utf8().value.to_string().c_str());
                oEvent.PutGuid("ObjectGuid", oObjectGuid);
                // Fetch events from the Object collection associated with the object guid
                bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = c_oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces) << finalize);
                if (bsoncxx::stdx::nullopt != oObjectDocument)
                {
                    bsoncxx::document::element stlObjectBlob = oObjectDocument->view()["ObjectBlob"];
                    if (stlObjectBlob && stlObjectBlob.type() == type::k_binary)
                    {
                        StructuredBuffer oObject(stlObjectBlob.get_binary().bytes, stlObjectBlob.get_binary().size);
                        // Get event sequence number to; be used in the filter (if specified) and/or used as event structuredbuffer name (so that the events are sorted sequentially)
                        unObjectSequenceNumber = oObject.GetUnsignedInt32("SequenceNumber");
                        // Apply supplied filters on audit logs
                        std::vector<std::string> stlFilters = c_oFilters.GetNamesOfElements();
                        try 
                        {
                            for (std::string strFilter : stlFilters)
                            {
                                if ("SequenceNumber" == strFilter)
                                {
                                    uint32_t unFilterSequenceNumber = c_oFilters.GetUnsignedInt32("SequenceNumber");
                                    _ThrowBaseExceptionIf((unObjectSequenceNumber < unFilterSequenceNumber), "Object sequence number is less than the specified starting sequence number.", nullptr);
                                }
                                else if ("MinimumDate" == strFilter)
                                {
                                    uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                    uint64_t unFilterMinimumDate = c_oFilters.GetUnsignedInt64("MinimumDate");
                                    _ThrowBaseExceptionIf((unObjectTimestamp < unFilterMinimumDate), "Object timestamp is less than the specified minimum date.", nullptr);
                                }
                                else if ("MaximumDate" == strFilter)
                                {
                                    uint64_t unObjectTimestamp = oObject.GetUnsignedInt64("Timestamp");
                                    uint64_t unFilterMaximumDate = c_oFilters.GetUnsignedInt64("MaximumDate");
                                    _ThrowBaseExceptionIf((unObjectTimestamp > unFilterMaximumDate), "Object timestamp is greater than the specified maximum date.", nullptr);
                                }
                                else if ("TypeOfEvents" == strFilter)
                                {
                                    Qword qwObjectEventType = oObject.GetQword("EventType");
                                    Qword qwFilterEventType = c_oFilters.GetQword("TypeOfEvents");
                                    _ThrowBaseExceptionIf((qwObjectEventType != qwFilterEventType), "Object type is not the same as the specified event type.", nullptr);
                                }
                                else if ("DCGuid" == strFilter)
                                {
                                    // Word wType = Guid(strEventGuid.get_utf8().value.to_string().c_str()).GetObjectType();
                                    // _ThrowBaseExceptionIf((eAuditEventBranchNode != wType), "No DC guid exists for this type of object.", nullptr);
                                    StructuredBuffer oPlainTextMetadata(oObject.GetStructuredBuffer("PlainTextEventData"));
                                    std::string strPlainObjectDCGuid = oPlainTextMetadata.GetString("GuidOfDcOrVm");
                                    std::string strFilterDcGuid = c_oFilters.GetString("DCGuid");
                                    _ThrowBaseExceptionIf((1 != oPlainTextMetadata.GetDword("BranchType")), "The audit log is not for a digital contract", nullptr);
                                    _ThrowBaseExceptionIf((strPlainObjectDCGuid != strFilterDcGuid), "The DC guid does not match the requested dc guid", nullptr);
                                }
                                // TODO: Add VMGuid filters
                                // else if ("VMGuid" == strFilter)
                                // {

                                // }
                            }
                            fAddToListOfEvents = true;
                        }
                        catch (BaseException oException)
                        {
                            fAddToListOfEvents = false;
                        }
                        if (true == fAddToListOfEvents)
                        {
                            // If the audit log object is not filtered out then add it to the Event structured buffer
                            oEvent.PutStructuredBuffer("ObjectBlob", oObject);
                            // Add PlainTextObjectBlobGuid to the Event structured buffer
                            oEvent.PutGuid("PlainTextObjectBlobGuid", oPlainTextObjectBlobGuid);
                        }
                    }
                }
            }
        }
    }
    // If the object is not filtered out then add the audit log information to the Event structured buffer
    if (true == fAddToListOfEvents)
    {
        // Add other information to the event 
        if (strEventGuid && strEventGuid.type() == type::k_utf8)
        {
            oEvent.PutGuid("EventGuid", Guid(strEventGuid.get_utf8().value.to_string().c_str()));

            if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
            {
                oEvent.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
            }
            if (fIsLeaf && fIsLeaf.type() == type::k_bool)
            {
                oEvent.PutBoolean("isLeaf", fIsLeaf.get_bool().value);
            }
            // Add event to the response structured buffer that contains list of audit events 
            poListOfEvents->PutStructuredBuffer(std::to_string(unObjectSequenceNumber).c_str(), oEvent);
        }
    }
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function GetNextSequenceNumber
 * @brief Fetch next sequence number from the parent event and update the parent event's next sequence number
 * @param[in] c_oRequest contains organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Sequence number for a non leaf event
 *
 ********************************************************************************************/

uint32_t __thiscall DatabaseManager::GetNextSequenceNumber(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;
    uint32_t unSequenceNumber = -1;

    std::string strEventGuid = c_oRequest.GetString("EventGuid");
    std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");

    // Each client and transaction can only be used in a single thread
    mongocxx::pool::entry oClient = m_poMongoPool->acquire();
    // Access SailDatabase
    mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
    // Get root event if it exists
    // Otherwise return 0 as the next sequence number
    
    bsoncxx::stdx::optional<bsoncxx::document::value> oAuditLogDocument = oSailDatabase["AuditLog"].find_one(document{} 
                                                                                                        << "EventGuid" << strEventGuid 
                                                                                                        << "OrganizationGuid" << strOrganizationGuid
                                                                                                        << finalize);
    if (bsoncxx::stdx::nullopt != oAuditLogDocument)
    {
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            bsoncxx::document::element unNextSequenceNumber = oAuditLogDocument->view()["NextSequenceNumber"];
            if (unNextSequenceNumber && unNextSequenceNumber.type() == type::k_double)
            {
                unSequenceNumber = (uint32_t) unNextSequenceNumber.get_double().value;
            }
            oSailDatabase["AuditLog"].update_one(*poSession, document{} << "EventGuid" << strEventGuid << finalize,
                                                document{} << "$set" << open_document <<
                                                "NextSequenceNumber" << (double)(unSequenceNumber + 1) << close_document << finalize);
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

    return unSequenceNumber;
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function AddNonLeafEvent
 * @brief Add a non leaf audit log event to the database
 * @param[in] c_oRequest contains the event information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddNonLeafEvent(
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
        uint32_t unSequenceNumber, unNextSequenceNumber = 0;
        StructuredBuffer oNonLeafEvent(c_oRequest.GetStructuredBuffer("NonLeafEvent"));
        // Get parent event's next sequence number and use it to assign a sequence number to the branch event
        StructuredBuffer oGetRoot;
        oGetRoot.PutString("EventGuid", oNonLeafEvent.GetString("ParentGuid"));
        oGetRoot.PutString("OrganizationGuid", oNonLeafEvent.GetString("OrganizationGuid"));
        unSequenceNumber = this->GetNextSequenceNumber(oGetRoot);
        if (-1 == unSequenceNumber)
        {
            unSequenceNumber = 0;
        }

        // Create an audit log event document
        bsoncxx::document::value oEventDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "OrganizationGuid" << oNonLeafEvent.GetString("OrganizationGuid")
        << "ParentGuid" << oNonLeafEvent.GetString("ParentGuid")
        << "EventGuid" << oNonLeafEvent.GetString("EventGuid")
        << "NextSequenceNumber" << (double) unNextSequenceNumber
        << "IsLeaf" << false
        << finalize;

        // Create an object document
        StructuredBuffer oObject;
        oObject.PutString("EventGuid", oNonLeafEvent.GetString("EventGuid"));
        oObject.PutString("ParentGuid", oNonLeafEvent.GetString("ParentGuid"));
        oObject.PutString("OrganizationGuid", oNonLeafEvent.GetString("OrganizationGuid"));
        oObject.PutQword("EventType", oNonLeafEvent.GetQword("EventType"));
        oObject.PutUnsignedInt64("Timestamp", oNonLeafEvent.GetUnsignedInt64("Timestamp"));
        oObject.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
        oObject.PutStructuredBuffer("PlainTextEventData", oNonLeafEvent.GetStructuredBuffer("PlainTextEventData"));
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
        << "ObjectType" << GuidOfObjectType::eAuditEventBranchNode
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access AuditLog collection
        mongocxx::collection oAuditLogCollection = oSailDatabase["AuditLog"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the AuditLog collection
            auto oResult = oAuditLogCollection.insert_one(*poSession, oEventDocumentValue.view());
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
                        dwStatus = 200;
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
 * @function AddLeafEvent
 * @brief Add leaf audit log events to the database
 * @param[in] c_oRequest contains the event information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddLeafEvent(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try
    {
        // Get required parameters from the request
        std::string strIdentifierOfParent = c_oRequest.GetString("ParentGuid");
        StructuredBuffer oLeafEvents(c_oRequest.GetStructuredBuffer("LeafEvents"));
        std::vector<std::string> stlEvents = oLeafEvents.GetNamesOfElements();
        // Loop through the array of events, create document for each event, and add them to the relevant vector:
        // stlEventDocuments: holds audit log documents
        // stlObjectDocuments: holds object documents
        // stlPlainTextObjectDocuments: holds plain text object documents
        std::vector<bsoncxx::document::value> stlEventDocuments, stlObjectDocuments, stlPlainTextObjectDocuments;
        for (unsigned int unIndex = 0; unIndex < stlEvents.size(); ++unIndex)
        {
            Guid oObjectGuid, oPlainTextObjectBlobGuid;

            StructuredBuffer oEvent(oLeafEvents.GetStructuredBuffer(stlEvents[unIndex].c_str()));

            // Create an audit log event document and add it to stlEventDocuments vector
            stlEventDocuments.push_back(bsoncxx::builder::stream::document{}
            << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
            << "OrganizationGuid" << c_oRequest.GetString("OrganizationGuid")
            << "ParentGuid" << strIdentifierOfParent
            << "EventGuid" << oEvent.GetString("EventGuid")
            << "IsLeaf" << true
            << finalize);

            // Create an object document and add it to stlObjectDocuments vector
            // Parse and cast event parameters as json arrays are not parsed by rest portal
            StructuredBuffer oObject;
            oObject.PutString("EventGuid", oEvent.GetString("EventGuid"));
            oObject.PutString("ParentGuid", strIdentifierOfParent);
            // Convert number type parameters to the required data type 
            oObject.PutQword("EventType", (Qword) oEvent.GetFloat64("EventType"));
            oObject.PutUnsignedInt64("Timestamp", (uint64_t) oEvent.GetFloat64("Timestamp"));
            oObject.PutUnsignedInt32("SequenceNumber", (uint32_t) oEvent.GetFloat64("SequenceNumber"));
            oObject.PutString("EncryptedEventData", oEvent.GetString("EncryptedEventData"));
            // Create a binary blob to be inserted in the document
            bsoncxx::types::b_binary oObjectBlob
            {
                bsoncxx::binary_sub_type::k_binary,
                uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
                oObject.GetSerializedBufferRawDataPtr()
            };
            stlObjectDocuments.push_back(bsoncxx::builder::stream::document{}
            << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
            << "ObjectBlob" << oObjectBlob
            << finalize);

            // Create a plain text object document and add it to stlPlainTextObjectDocuments vector
            stlPlainTextObjectDocuments.push_back(bsoncxx::builder::stream::document{}
            << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
            << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
            << "ObjectType" << GuidOfObjectType::eAuditEventPlainTextLeafNode
            << finalize);
        }

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access AuditLog collection
        mongocxx::collection oAuditLogCollection = oSailDatabase["AuditLog"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert audit log documents in the AuditLog collection
            auto oResult = oAuditLogCollection.insert_many(*poSession, stlEventDocuments);
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                // Access Object collection
                mongocxx::collection oObjectCollection = oSailDatabase["Object"];
                // Insert object documents in the Object collection
                oResult = oObjectCollection.insert_many(*poSession, stlObjectDocuments);
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    // Access PlainTextObjectBlob collection
                    mongocxx::collection oPlainTextObjectCollection = oSailDatabase["PlainTextObjectBlob"];
                    // Insert plain text object blob documents in the PlainTextObjectBlob collection
                    oResult = oPlainTextObjectCollection.insert_many(*poSession, stlPlainTextObjectDocuments);
                    if (!oResult) {
                        std::cout << "Error while writing to the database." << std::endl;
                    }
                    else
                    {
                        dwStatus = 200;
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