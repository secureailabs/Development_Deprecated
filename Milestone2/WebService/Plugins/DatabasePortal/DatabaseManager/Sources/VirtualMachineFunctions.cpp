/*********************************************************************************************
 *
 * @file VirtualMachineFunctions.cpp
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
 * @function PullVirtualMachine
 * @brief Fetch the virtual machine information
 * @param[in] c_oRequest contains the virtual machine guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the virtual machine information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::PullVirtualMachine(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the virtual machine record
        bsoncxx::stdx::optional<bsoncxx::document::value> oVmDocument = oSailDatabase["VirtualMachine"].find_one(document{}
                                                                                                                    << "VirtualMachineGuid" << strVmGuid
                                                                                                                    << finalize);
        if (bsoncxx::stdx::nullopt != oVmDocument)
        {                                                                                                           
            bsoncxx::document::element oDcGuid = oVmDocument->view()["DigitalContractGuid"];
            if (oDcGuid && oDcGuid.type() == type::k_utf8)
            {
                std::string strDcGuid = oDcGuid.get_utf8().value.to_string();
                bsoncxx::document::element oPlainTextObjectBlobGuid = oVmDocument->view()["PlainTextObjectBlobGuid"];
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
                            // Fetch the virtual machine from the Object collection associated with the virtual machine guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oResponse.PutStructuredBuffer("VirtualMachine", oObject);
                                    oResponse.PutString("DigitalContractGuid", strDcGuid);
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
 * @function RegisterVirtualMachine
 * @brief Add metadata of a new virtual machine to the database
 * @param[in] c_oRequest contains the virtual machine's information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterVirtualMachine(
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

        // Create an virtual machine document
        bsoncxx::document::value oVmDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "DigitalContractGuid" << c_oRequest.GetString("DigitalContractGuid")
        << "VirtualMachineGuid" << c_oRequest.GetString("VirtualMachineGuid")
        << finalize;
        
        // Create a virtual machine object structured buffer
        StructuredBuffer oObject;
        oObject.PutString("DigitalContractGuid", c_oRequest.GetString("DigitalContractGuid"));
        oObject.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
        oObject.PutUnsignedInt64("RegistrationTime", c_oRequest.GetUnsignedInt64("RegistrationTime"));
        oObject.PutUnsignedInt64("HeartbeatBroadcastTime", c_oRequest.GetUnsignedInt64("HeartbeatBroadcastTime"));
        oObject.PutString("IPAddress", c_oRequest.GetString("IPAddress"));
        bsoncxx::types::b_binary oObjectBlob
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oObject.GetSerializedBufferRawDataSizeInBytes()),
            oObject.GetSerializedBufferRawDataPtr()
        };
        // Create an object document
        bsoncxx::document::value oObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectBlob" << oObjectBlob
        << finalize;

        // Create a plain text object document
        bsoncxx::document::value oPlainTextObjectDocumentValue = bsoncxx::builder::stream::document{}
        << "PlainTextObjectBlobGuid" << oPlainTextObjectBlobGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectGuid" << oObjectGuid.ToString(eHyphensAndCurlyBraces)
        << "ObjectType" << eVirtualMachine
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access VirtualMachine collection
        mongocxx::collection oVirtualMachineCollection = oSailDatabase["VirtualMachine"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the VirtualMachine collection
            auto oResult = oVirtualMachineCollection.insert_one(*poSession, oVmDocumentValue.view());
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