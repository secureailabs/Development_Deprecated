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
 * @function ListOfVmsAssociatedWithDc
 * @brief Fetch the virtual machines associated with a digital contract
 * @param[in] c_oRequest contains the digital contract guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the virtual machines' information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListOfVmsAssociatedWithDc(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    StructuredBuffer oListOfVMs;

    try 
    {
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the virtual machine records
        mongocxx::cursor oVmCursor = oSailDatabase["VirtualMachine"].find(document{}
                                                                    << "DigitalContractGuid" << strDcGuid
                                                                    << finalize);
        for (auto&& oVmDocumentView : oVmCursor)
        {                                                                                                           
            bsoncxx::document::element oVmGuid = oVmDocumentView["VirtualMachineGuid"];
            if (oVmGuid && oVmGuid.type() == type::k_utf8)
            {
                std::string strVmGuid = oVmGuid.get_utf8().value.to_string();
                bsoncxx::document::element oPlainTextObjectBlobGuid = oVmDocumentView["PlainTextObjectBlobGuid"];
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
                            // Fetch the virtual machine from the Object collection associated with the object guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);
                                    oListOfVMs.PutStructuredBuffer(strVmGuid.c_str(), oObject);
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("VirtualMachines", oListOfVMs);
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
 * @function ListOfVmIpAddressesAssociatedWithDc
 * @brief Fetch the virtual machines' ip addresses associated with a digital contract
 * @param[in] c_oRequest contains the digital contract guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the virtual machines' ip addresses
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListOfVmIpAddressesAssociatedWithDc(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    StructuredBuffer oListOfVMs;

    try 
    {
        std::string strDcGuid = c_oRequest.GetString("DigitalContractGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch the virtual machine records
        mongocxx::cursor oVmCursor = oSailDatabase["VirtualMachine"].find(document{}
                                                                    << "DigitalContractGuid" << strDcGuid
                                                                    << finalize);
        for (auto&& oVmDocumentView : oVmCursor)
        {                                                                                                           
            bsoncxx::document::element oVmGuid = oVmDocumentView["VirtualMachineGuid"];
            if (oVmGuid && oVmGuid.type() == type::k_utf8)
            {
                std::string strVmGuid = oVmGuid.get_utf8().value.to_string();
                bsoncxx::document::element oPlainTextObjectBlobGuid = oVmDocumentView["PlainTextObjectBlobGuid"];
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
                            // Fetch the virtual machine from the Object collection associated with the object guid
                            bsoncxx::stdx::optional<bsoncxx::document::value> oObjectDocument = oSailDatabase["Object"].find_one(document{} << "ObjectGuid" << strObjectGuid << finalize);
                            if (bsoncxx::stdx::nullopt != oObjectDocument)
                            {
                                bsoncxx::document::element oObjectBlob = oObjectDocument->view()["ObjectBlob"];
                                if (oObjectBlob && oObjectBlob.type() == type::k_binary)
                                {
                                    StructuredBuffer oObject(oObjectBlob.get_binary().bytes, oObjectBlob.get_binary().size);

                                    if (true == oObject.IsElementPresent("State", DWORD_VALUE_TYPE))
                                    {
                                        if (VirtualMachineState::eReadyForComputation == oObject.GetDword("State"))
                                        {
                                            oListOfVMs.PutString(strVmGuid.c_str(), oObject.GetString("IPAddress"));
                                        }
                                    }
                                    dwStatus = 200;
                                }
                            }
                        }
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("VirtualMachines", oListOfVMs);
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
 * @function GetVmsWaitingForData
 * @brief Fetch list of VM ipaddressese that are waiting for the remote data connector's dataset(s)
 * @param[in] c_oRequest contains the dataset guids 
 * @throw BaseException Error StructuredBuffer element not found
 * @returns StructuredBuffer containing the virtual machines' ip addresses
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetVmsWaitingForData(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;
    StructuredBuffer oListOfVMs;

    try 
    {
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Create the query
        StructuredBuffer oDatasets = c_oRequest.GetStructuredBuffer("Datasets");
        auto stlDatasetGuids = bsoncxx::builder::basic::array{};
        for (std::string strDset : oDatasets.GetNamesOfElements())
        {
            stlDatasetGuids.append(oDatasets.GetStructuredBuffer(strDset.c_str()).GetString("DatasetUuid"));
        }
        // Fetch the virtual machine ip addresses
        mongocxx::cursor oVmCursor = oSailDatabase["VirtualMachinesWaitingForData"].find(document{}
                                                                    << "DatasetGuid"
                                                                    << open_document
                                                                    << "$in" << stlDatasetGuids
                                                                    << close_document
                                                                    << finalize);
        for (auto&& oVmDocumentView : oVmCursor)
        {                                                                                                           
            bsoncxx::document::element oVmGuid = oVmDocumentView["VirtualMachineGuid"];
            bsoncxx::document::element oIpAddress = oVmDocumentView["IPAddress"];
            bsoncxx::document::element oDatasetGuid = oVmDocumentView["DatasetGuid"];
            if ((oVmGuid && oVmGuid.type() == type::k_utf8) && (oIpAddress && oIpAddress.type() == type::k_utf8) && (oDatasetGuid && oDatasetGuid.type() == type::k_utf8))
            {
                std::string strVmGuid = oVmGuid.get_utf8().value.to_string();
                std::string strIpAddress = oIpAddress.get_utf8().value.to_string();
                std::string strDatasetGuid = oDatasetGuid.get_utf8().value.to_string();
                StructuredBuffer oVirtualMachineInformation;
                oVirtualMachineInformation.PutString("IPAddress", strIpAddress);
                oVirtualMachineInformation.PutString("DatasetGuid", strDatasetGuid);
                oListOfVMs.PutStructuredBuffer(strVmGuid.c_str(), oVirtualMachineInformation);
            }
        }
        dwStatus = 200;
        oResponse.PutStructuredBuffer("VirtualMachines", oListOfVMs);
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
        oObject.PutString("DigitalContractTitle", c_oRequest.GetString("DigitalContractTitle"));
        oObject.PutString("Note", c_oRequest.GetString("Note"));
        oObject.PutString("VirtualMachineGuid", c_oRequest.GetString("VirtualMachineGuid"));
        oObject.PutUnsignedInt64("RegistrationTime", c_oRequest.GetUnsignedInt64("RegistrationTime"));
        oObject.PutUnsignedInt64("HeartbeatBroadcastTime", c_oRequest.GetUnsignedInt64("HeartbeatBroadcastTime"));
        oObject.PutString("IPAddress", c_oRequest.GetString("IPAddress"));
        oObject.PutUnsignedInt64("NumberOfVCPU", c_oRequest.GetUnsignedInt64("NumberOfVCPU"));
        oObject.PutString("HostRegion", c_oRequest.GetString("HostRegion"));
        oObject.PutUnsignedInt64("StartTime", c_oRequest.GetUnsignedInt64("StartTime"));
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
        << "ObjectType" << GuidOfObjectType::eVirtualMachine
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
 * @function UpdateVirtualMachine
 * @brief Update metadata of virtual machine in the database
 * @param[in] c_oRequest contains the virtual machine's information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateVirtualMachine(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        std::string strVmGuid = c_oRequest.GetString("VirtualMachineGuid");
        StructuredBuffer oVmInformation = c_oRequest.GetStructuredBuffer("VirtualMachineData");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get virtual machine document
        bsoncxx::stdx::optional<bsoncxx::document::value> oVmDocument = oSailDatabase["VirtualMachine"].find_one(document{} 
                                                                                                            << "VirtualMachineGuid" << strVmGuid 
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oVmDocument)
        {
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
                        // Update the virtual machine in the Object collection associated with the VM guid
                        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
                        {
                            // Create object blob
                            bsoncxx::types::b_binary oUpdatedObjectBlob
                            {
                                bsoncxx::binary_sub_type::k_binary,
                                uint32_t(oVmInformation.GetSerializedBufferRawDataSizeInBytes()),
                                oVmInformation.GetSerializedBufferRawDataPtr()
                            };
                            oSailDatabase["Object"].update_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize,
                                                                document{} << "$set" << open_document <<
                                                                "ObjectBlob" << oUpdatedObjectBlob << close_document << finalize);
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

    // Send back the status of the transaction
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterVmAsWaitingForData
 * @brief Add metadata of a virtual machine that is waiting for data to the database
 * @param[in] c_oRequest contains the virtual machine's information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterVmAsWaitingForData(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Create an VirtualMachinesWaitingForData document
        bsoncxx::document::value oDocumentValue = bsoncxx::builder::stream::document{}
        << "VirtualMachineGuid" << c_oRequest.GetString("VirtualMachineGuid")
        << "IPAddress" << c_oRequest.GetString("IPAddress")
        << "DatasetGuid" << c_oRequest.GetString("DatasetGuid")
        << finalize;

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access VirtualMachinesWaitingForData collection
        mongocxx::collection oVirtualMachinesWaitingForDataCollection = oSailDatabase["VirtualMachinesWaitingForData"];
        // Create a transaction callback
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
        {
            // Insert document in the VirtualMachinesWaitingForData collection
            auto oResult = oVirtualMachinesWaitingForDataCollection.insert_one(*poSession, oDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                dwStatus = 201;
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
 * @function RemoveVmAsWaitingForData
 * @brief Remove VM information as waiting for data
 * @param[in] c_oRequest contains virtual machine guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RemoveVmAsWaitingForData(
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
        // Delete document from VirtualMachinesWaitingForData collection associated with VirtualMachineGuid
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
        {
            oSailDatabase["VirtualMachinesWaitingForData"].delete_one(*poSession, document{} << "VirtualMachineGuid" << strVmGuid << finalize);
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