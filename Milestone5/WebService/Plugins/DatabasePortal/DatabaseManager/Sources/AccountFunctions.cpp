/*********************************************************************************************
 *
 * @file AccountFunctions.cpp
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
 * @function GetOrganizationInformation
 * @brief Fetch organization information associated with organization guid from the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing organization information
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetOrganizationInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get organization confidential document
        bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                            << "OrganizationOrUserUuid" << strOrganizationGuid 
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oConfidentialDocument)
        {
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                bsoncxx::document::element stlEncryptedSsb = oConfidentialDocument->view()["EncryptedSsb"];
                if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
                {
                    StructuredBuffer oConfidentialOrganization(stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
                    // TODO: Once the organization confidential record is encrypted in RegisterOrganization, Decrypt record, and then fetch it
                    StructuredBuffer oOrganizationInformation;
                    oOrganizationInformation.PutString("OrganizationName", oConfidentialOrganization.GetString("OrganizationName"));
                    oOrganizationInformation.PutString("OrganizationAddress", oConfidentialOrganization.GetString("OrganizationAddress"));
                    oOrganizationInformation.PutString("PrimaryContactName", oConfidentialOrganization.GetString("PrimaryContactName"));
                    oOrganizationInformation.PutString("PrimaryContactTitle", oConfidentialOrganization.GetString("PrimaryContactTitle"));
                    oOrganizationInformation.PutString("PrimaryContactEmail", oConfidentialOrganization.GetString("PrimaryContactEmail"));
                    oOrganizationInformation.PutString("PrimaryContactPhoneNumber", oConfidentialOrganization.GetString("PrimaryContactPhoneNumber"));
                    oOrganizationInformation.PutString("SecondaryContactName", oConfidentialOrganization.GetString("SecondaryContactName"));
                    oOrganizationInformation.PutString("SecondaryContactTitle", oConfidentialOrganization.GetString("SecondaryContactTitle"));
                    oOrganizationInformation.PutString("SecondaryContactEmail", oConfidentialOrganization.GetString("SecondaryContactEmail"));
                    oOrganizationInformation.PutString("SecondaryContactPhoneNumber", oConfidentialOrganization.GetString("SecondaryContactPhoneNumber"));
                    // Send back the organization information
                    oResponse.PutStructuredBuffer("OrganizationInformation", oOrganizationInformation);
                }
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
 * @function GetBasicOrganizationRecord
 * @brief Fetch basic organization record associated with organization name from the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing basic organization record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetBasicOrganizationRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strOrganizationName = c_oRequest.GetString("OrganizationName");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access BasicOrganization collection
        mongocxx::collection oBasicOrganizationCollection = oSailDatabase["BasicOrganization"];
        // Fetch Basic Organization record associated with the strOrganizationName
        bool fFound = false;
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicOrganization = oBasicOrganizationCollection.find_one(document{} << "OrganizationName" << strOrganizationName << finalize);
        if (bsoncxx::stdx::nullopt != oBasicOrganization)
        {
            fFound = true;
        }

        if (true == fFound)
        {
            // Get values from the document and generate the BasicOrganization StructuredBuffer
            bsoncxx::document::view oBasicOrganizationView = oBasicOrganization->view();
            bsoncxx::document::element strOrganizationGuid = oBasicOrganizationView["OrganizationUuid"];
            bsoncxx::document::element strOrganizationName = oBasicOrganizationView["OrganizationName"];

            StructuredBuffer oBasicOrganizationRecord;

            if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
            {
                oBasicOrganizationRecord.PutGuid("OrganizationGuid", Guid(strOrganizationGuid.get_utf8().value.to_string().c_str()));
            }
            if (strOrganizationName && strOrganizationName.type() == type::k_utf8)
            {
                oBasicOrganizationRecord.PutString("OrganizationName", strOrganizationName.get_utf8().value.to_string());
            }
            oResponse.PutStructuredBuffer("BasicOrganizationRecord", oBasicOrganizationRecord);

            dwStatus = 200;
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
 * @function GetOrganizationName
 * @brief Fetch basic organization name associated with organization guid from the database
 * @param[in] c_oRequest contains the dataset guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing organization name
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetOrganizationName(
    _in const std::string & c_strOrganizationGuid
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
        // Access BasicOrganization collection
        mongocxx::collection oBasicOrganizationCollection = oSailDatabase["BasicOrganization"];
        // Fetch Basic Organization record associated with the strOrganizationName
        bool fFound = false;
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicOrganization = oBasicOrganizationCollection.find_one(document{} << "OrganizationUuid" << c_strOrganizationGuid << finalize);
        if (bsoncxx::stdx::nullopt != oBasicOrganization)
        {
            fFound = true;
        }

        if (true == fFound)
        {
            // Get values from the document and generate the BasicOrganization StructuredBuffer
            bsoncxx::document::view oBasicOrganizationView = oBasicOrganization->view();
            bsoncxx::document::element strOrganizationName = oBasicOrganizationView["OrganizationName"];

            if (strOrganizationName && strOrganizationName.type() == type::k_utf8)
            {
                oResponse.PutString("OrganizationName", strOrganizationName.get_utf8().value.to_string());
                dwStatus = 200;
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
 * @function GetBasicUserRecord
 * @brief Fetch basic user record associated with the 64bithash from the database
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing basic user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetBasicUserRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try
    {
        Qword qw64BitHash = c_oRequest.GetQword("Passphrase");
        // Check if its for login transaction
        // If yes, then check if the DatabaseGateway will check if the account is eNew or eOpen
        // Otherwise, it will send back an error code
        bool fIsLoginTransaction = false;
        if (true == c_oRequest.IsElementPresent("IsLoginTransaction", BOOLEAN_VALUE_TYPE))
        {
            fIsLoginTransaction = c_oRequest.GetBoolean("IsLoginTransaction");
        }

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access BasicUser collection
        mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
        // Fetch basic user record associated with the qw64BitHash
        bool fFound = false;
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUser = bsoncxx::stdx::nullopt;
        if (true == fIsLoginTransaction) 
        {
            oBasicUser = oBasicUserCollection.find_one(document{} 
                                                    << "64BitHash" << (double)qw64BitHash 
                                                    << "AccountStatus" <<  open_document
                                                    << "$lte" << AccountStatus::eOpen << close_document
                                                    << finalize);
        }
        else
        {
            oBasicUser = oBasicUserCollection.find_one(document{} << "64BitHash" << (double)qw64BitHash << finalize);
        }

        if (bsoncxx::stdx::nullopt != oBasicUser)
        {
            fFound = true;
        }

        if (true == fFound)
        {
            // Get values from the document and generate the BasicUser StructuredBuffer
            bsoncxx::document::view oBasicUserView = oBasicUser->view();
            bsoncxx::document::element strUserGuid = oBasicUserView["UserUuid"];
            bsoncxx::document::element strOrganizationGuid = oBasicUserView["OrganizationUuid"];
            bsoncxx::document::element dwAccountStatus = oBasicUserView["AccountStatus"];
            bsoncxx::document::element stlAccountKeyWrappedWithPDK = oBasicUserView["WrappedAccountKey"];

            StructuredBuffer oBasicUserRecord;

            if (strUserGuid && strUserGuid.type() == type::k_utf8)
            {
                oBasicUserRecord.PutGuid("UserGuid", Guid(strUserGuid.get_utf8().value.to_string().c_str()));
            }
            if (strOrganizationGuid && strOrganizationGuid.type() == type::k_utf8)
            {
                std::string strOrganizationIdentifier = strOrganizationGuid.get_utf8().value.to_string().c_str();
                oBasicUserRecord.PutGuid("OrganizationGuid", Guid(strOrganizationIdentifier.c_str()));
                // Get organization name
                StructuredBuffer oOrganizationName = this->GetOrganizationName(strOrganizationIdentifier);
                if (200 == oOrganizationName.GetDword("Status")) 
                {
                    oBasicUserRecord.PutString("OrganizationName", oOrganizationName.GetString("OrganizationName"));
                }
            }
            if (dwAccountStatus && dwAccountStatus.type() == type::k_double)
            {
                oBasicUserRecord.PutDword("AccountStatus", (Dword) dwAccountStatus.get_double().value);
            }
            if (stlAccountKeyWrappedWithPDK && stlAccountKeyWrappedWithPDK.type() == type::k_binary)
            {
                oBasicUserRecord.PutBuffer("WrappedAccountKey", stlAccountKeyWrappedWithPDK.get_binary().bytes, stlAccountKeyWrappedWithPDK.get_binary().size);
            }

            oResponse.PutStructuredBuffer("BasicUserRecord", oBasicUserRecord);
            dwStatus = 200;
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
 * @function GetConfidentialUserRecord
 * @brief Fetch confidential user record from the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing confidential user record
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::GetConfidentialUserRecord(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strUserUuid = c_oRequest.GetString("UserUuid");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access ConfidentialOrganizationOrUser collection
        mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
        // Fetch confidential user record associated with the strUserUuid
        bool fFound = false;
        bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialUser = oConfidentialUserCollection.find_one(document{} << "OrganizationOrUserUuid" << strUserUuid << finalize);
        if (bsoncxx::stdx::nullopt != oConfidentialUser)
        {
            fFound = true;
        }

        if (true == fFound)
        {
            // Get values from the document and generate the oConfidentialUser StructuredBuffer
            bsoncxx::document::view oConfidentialUserView = oConfidentialUser->view();
            bsoncxx::document::element stlEncryptedSsb = oConfidentialUserView["EncryptedSsb"];

            StructuredBuffer oConfidentialUserRecord;

            oConfidentialUserRecord.PutGuid("OrganizationOrUserGuid", Guid(strUserUuid.c_str()));

            if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
            {
                oConfidentialUserRecord.PutBuffer("EncryptedSsb", stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
            }
            oResponse.PutStructuredBuffer("ConfidentialOrganizationOrUserRecord", oConfidentialUserRecord);
            dwStatus = 200;
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

    // Send back the transaction status
    oResponse.PutDword("Status", dwStatus);

    return oResponse.GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class DatabaseManager
 * @function RegisterOrganization
 * @brief Add a new organization to the database and call AddSuperUser() to add the organization's super user
 * @param[in] c_oRequest contains the organization information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource is called when a new user tries registering themselves. Their organziation 
 *  is registered first and the user is registered as the organization's super admin, if the organization
 *  is registered successfully.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterOrganization(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Check if a record with the OrganizationName exists in the database
        StructuredBuffer oRequest(c_oRequest.GetStructuredBuffer("Request"));
        StructuredBuffer oBasicOrganizationRecord(this->GetBasicOrganizationRecord(oRequest));
        if (404 == oBasicOrganizationRecord.GetDword("Status"))
        {
            // Create guids for the documents
            Guid oBasicOrganizationGuid, oConfidentialOrganizationGuid;
            // Create guid for organziation and organziation root key
            // Todo: get organziation root key guid
            std::string strOrganizationGuid = Guid(eOrganization).ToString(eHyphensAndCurlyBraces);
            std::string strOrganziationRootKeyGuid = Guid().ToString(eHyphensAndCurlyBraces);
            // Create guid for the super user
            std::string strUserGuid = Guid(eUser).ToString(eHyphensAndCurlyBraces);
            // Get organization name
            std::string strOrganizationName = oRequest.GetString("OrganizationName");

            // Create basic organziation document
            // TODO: add public root key certificate and wrapped account encryption keys
            bsoncxx::document::value oBasicOrganizationDocumentValue = bsoncxx::builder::stream::document{}
                << "OrganizationUuid" << strOrganizationGuid
                << "OrganizationName" << strOrganizationName
                << finalize;

            // Create confidential organziation document
            StructuredBuffer oConfidentialOrganziation;
            oConfidentialOrganziation.PutString("OrganziationUuid", strOrganizationGuid);
            oConfidentialOrganziation.PutString("OrganizationRootKeyUuid", strOrganziationRootKeyGuid);
            // TODO: get account wrapping key guid
            oConfidentialOrganziation.PutString("AccountWrappingKeyUuid", Guid().ToString(eHyphensAndCurlyBraces));
            oConfidentialOrganziation.PutString("OrganizationName", strOrganizationName);
            oConfidentialOrganziation.PutString("OrganizationAddress", oRequest.GetString("OrganizationAddress"));
            oConfidentialOrganziation.PutString("PrimaryContactName", oRequest.GetString("PrimaryContactName"));
            oConfidentialOrganziation.PutString("PrimaryContactTitle", oRequest.GetString("PrimaryContactTitle"));
            oConfidentialOrganziation.PutString("PrimaryContactEmail", oRequest.GetString("PrimaryContactEmail"));
            oConfidentialOrganziation.PutString("PrimaryContactPhoneNumber", oRequest.GetString("PrimaryContactPhoneNumber"));
            oConfidentialOrganziation.PutString("SecondaryContactName", oRequest.GetString("SecondaryContactName"));
            oConfidentialOrganziation.PutString("SecondaryContactTitle", oRequest.GetString("SecondaryContactTitle"));
            oConfidentialOrganziation.PutString("SecondaryContactEmail", oRequest.GetString("SecondaryContactEmail"));
            oConfidentialOrganziation.PutString("SecondaryContactPhoneNumber", oRequest.GetString("SecondaryContactPhoneNumber"));
            oConfidentialOrganziation.PutString("AccountCreatorUuid", strUserGuid);
            oConfidentialOrganziation.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
            // TODO: encrypt oConfidentialOrganization
            bsoncxx::types::b_binary oEncryptedSsb
            {
                bsoncxx::binary_sub_type::k_binary,
                uint32_t(oConfidentialOrganziation.GetSerializedBufferRawDataSizeInBytes()),
                oConfidentialOrganziation.GetSerializedBufferRawDataPtr()
            };
            bsoncxx::document::value oConfidentialOrganziationDocumentValue = bsoncxx::builder::stream::document{}
            << "OrganizationOrUserUuid" << strOrganizationGuid
            << "EncryptedSsb" << oEncryptedSsb
            << finalize;

            // Each client and transaction can only be used in a single thread
            mongocxx::pool::entry oClient = m_poMongoPool->acquire();
            // Access SailDatabase
            mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
            // Access BasicOrganization collection
            mongocxx::collection oBasicOrganziationCollection = oSailDatabase["BasicOrganization"];
            // Access ConfidentialOrganizationOrUser collection
            mongocxx::collection oConfidentialOrganizationCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
            // Create a transaction callback
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                // Insert document in the BasicOrganization collection
                auto oResult = oBasicOrganziationCollection.insert_one(*poSession, oBasicOrganizationDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    // Insert document in the ConfidentialOrganizationOrUser collection
                    oResult = oConfidentialOrganizationCollection.insert_one(*poSession, oConfidentialOrganziationDocumentValue.view());
                    if (!oResult) {
                        std::cout << "Error while writing to the database." << std::endl;
                    }
                    else
                    {
                        oRequest.PutString("OrganizationUuid", strOrganizationGuid);
                        oRequest.PutString("UserUuid", strUserGuid);
                        oRequest.PutQword("AccessRights", eAdmin);
                        // Add new user as the super user of the organization
                        dwStatus = StructuredBuffer(this->AddSuperUser(oRequest, oClient, &poSession)).GetDword("Status");
                        _ThrowBaseExceptionIf((201 != dwStatus), "Error creating super user", nullptr);
                        // Send back organization guid to register a root event
                        oResponse.PutString("OrganizationGuid", strOrganizationGuid);
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
        else 
        {
            oResponse.PutString("ErrorMessage", "Organization already exists!");
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
 * @function AddSuperUser
 * @brief Add organziation's super user to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource registers a new user as the super user of a newly registered organziation.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::AddSuperUser(
    _in const StructuredBuffer & c_oRequest,
    _in mongocxx::pool::entry & oClient,
    _in mongocxx::v_noabi::client_session ** poSession
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        // Create guids for the documents
        Guid oBasicUserGuid, oConfidentialUserGuid;
        // Create guids for user and user root key
        // Todo: get user root key guid
        std::string strUserGuid = c_oRequest.GetString("UserUuid");
        Guid oUserRootKeyGuid;

        std::string strEmail = c_oRequest.GetString("Email");
        std::string strPassword = c_oRequest.GetString("Password");
        // Get 64 bit hash of "strEmail/strPassword" string
        Qword qw64BitHashPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strPassword).c_str(), false);
        // Generate "strEmail/strPassword" derived key
        std::string strDerivedKey = ::Base64HashOfEmailPassword(strEmail, strPassword);
        // Generate account encryption key
        // This key will be generated once for every organization when a super admin is registering
        // This key will be used to encrypt other users records withing the organization
        std::vector<Byte> stlAccountEncryptionKey = ::GenerateAccountKey();
        // Wrap account encryption key with strDerivedKey
        std::vector<Byte> stlWrappedAccountEncryptionKey = ::EncryptUsingPasswordKey(stlAccountEncryptionKey, strDerivedKey);

        // Create basic user document
        bsoncxx::types::b_binary oWrappedAccountEncryptionKey {bsoncxx::binary_sub_type::k_binary,
                                    uint32_t(stlWrappedAccountEncryptionKey.size()),
                                    stlWrappedAccountEncryptionKey.data()};
        bsoncxx::document::value oBasicUserDocumentValue = bsoncxx::builder::stream::document{}
            << "64BitHash" << (double) qw64BitHashPassphrase
            << "OrganizationUuid" << c_oRequest.GetString("OrganizationUuid")
            << "UserUuid" << strUserGuid
            << "AccountStatus" << (double) AccountStatus::eNew
            << "WrappedAccountKey" << oWrappedAccountEncryptionKey
            << finalize;

        // Create confidential user document
        StructuredBuffer oConfidentialUser;
        oConfidentialUser.PutGuid("UserGuid", Guid(strUserGuid.c_str()));
        oConfidentialUser.PutGuid("UserRootKeyGuid", oUserRootKeyGuid);
        oConfidentialUser.PutQword("AccessRights", c_oRequest.GetQword("AccessRights"));
        oConfidentialUser.PutString("Username", c_oRequest.GetString("Name"));
        oConfidentialUser.PutString("Title", c_oRequest.GetString("Title"));
        oConfidentialUser.PutString("Email", strEmail);
        oConfidentialUser.PutString("PhoneNumber", c_oRequest.GetString("PhoneNumber"));
        oConfidentialUser.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
        // Encrypt oConfidentialUser StructuredBuffer with account encryption key
        std::vector<Byte> stlEncryptedWithPasswordDerivedKey = ::EncryptUsingPasswordKey(oConfidentialUser.GetSerializedBuffer(), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end()));
        // Encrypt stlEncryptedWithPasswordDerivedKey with sail secret key
        StructuredBuffer oEncrypedConfidentialUser = ::EncryptUsingSailSecretKey(stlEncryptedWithPasswordDerivedKey);
        bsoncxx::types::b_binary oEncryptedSsb
        {
            bsoncxx::binary_sub_type::k_binary,
            uint32_t(oEncrypedConfidentialUser.GetSerializedBufferRawDataSizeInBytes()),
            oEncrypedConfidentialUser.GetSerializedBufferRawDataPtr()
        };
        bsoncxx::document::value oConfidentialUserDocumentValue = bsoncxx::builder::stream::document{}
        << "OrganizationOrUserUuid" << strUserGuid
        << "EncryptedSsb" << oEncryptedSsb
        << finalize;

        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Access BasicUser collection
        mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
        // Access ConfidentialOrganizationOrUser collection
        mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
        // Create a transaction callback
        // Insert document in the BasicUser collection
        auto oResult = oBasicUserCollection.insert_one(**poSession, oBasicUserDocumentValue.view());
        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
        else
        {
            // Insert document in the ConfidentialOrganizationOrUser collection
            oResult = oConfidentialUserCollection.insert_one(**poSession, oConfidentialUserDocumentValue.view());
            if (!oResult) {
                std::cout << "Error while writing to the database." << std::endl;
            }
            else
            {
                    dwStatus = 201;
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
 * @function RegisterUser
 * @brief Add a new user to the database
 * @param[in] c_oRequest contains the user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing request status
 * @note
 *  This resource is called when an Admin user of an organization tries to register a new user.
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RegisterUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        StructuredBuffer oRequest(c_oRequest.GetStructuredBuffer("Request"));
        std::string strEmail = oRequest.GetString("Email");
        std::string strPassword = oRequest.GetString("Password");
        // Get 64 bit hash of "strEmail/strPassword" string
        Qword qw64BitHashPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strPassword).c_str(), false);
        // Check if a record with the qw64BitHashPassphrase exists in the database
        StructuredBuffer oPassphrase;
        oPassphrase.PutQword("Passphrase", qw64BitHashPassphrase);
        StructuredBuffer oBasicUserRecord(this->GetBasicUserRecord(oPassphrase));

        if (404 == oBasicUserRecord.GetDword("Status"))
        {
            // Create guids for the documents
            Guid oBasicUserGuid, oConfidentialUserGuid;
            // Create guids for user and user root key
            // Todo: get user root key guid
            std::string strUserGuid = Guid(eUser).ToString(eHyphensAndCurlyBraces);
            Guid oUserRootKeyGuid;
            // Generate "strEmail/strPassword" derived key
            std::string strDerivedKey = ::Base64HashOfEmailPassword(strEmail, strPassword);
            // Get the organization account encryption key
            std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");
            // Wrap account encryption key with strDerivedKey
            std::vector<Byte> stlWrappedAccountEncryptionKey = ::EncryptUsingPasswordKey(stlAccountEncryptionKey, strDerivedKey);

            // Create basic user document
            bsoncxx::types::b_binary oWrappedAccountEncryptionKey {bsoncxx::binary_sub_type::k_binary,
                                        uint32_t(stlWrappedAccountEncryptionKey.size()),
                                        stlWrappedAccountEncryptionKey.data()};
            bsoncxx::document::value oBasicUserDocumentValue = bsoncxx::builder::stream::document{}
                << "64BitHash" << (double) qw64BitHashPassphrase
                << "OrganizationUuid" << c_oRequest.GetString("OrganizationGuid")
                << "UserUuid" << strUserGuid
                << "AccountStatus" << (double) AccountStatus::eNew
                << "WrappedAccountKey" << oWrappedAccountEncryptionKey
                << finalize;

            // Create confidential user document
            StructuredBuffer oConfidentialUser;
            oConfidentialUser.PutGuid("UserGuid", Guid(strUserGuid.c_str()));
            oConfidentialUser.PutGuid("UserRootKeyGuid", oUserRootKeyGuid);
            oConfidentialUser.PutQword("AccessRights", oRequest.GetQword("AccessRights"));
            oConfidentialUser.PutString("Username", oRequest.GetString("Name"));
            oConfidentialUser.PutString("Title", oRequest.GetString("Title"));
            oConfidentialUser.PutString("Email", strEmail);
            oConfidentialUser.PutString("PhoneNumber", oRequest.GetString("PhoneNumber"));
            oConfidentialUser.PutUnsignedInt64("TimeOfAccountCreation", ::GetEpochTimeInMilliseconds());
            // Encrypt oConfidentialUser StructuredBuffer with the account encryption key
            std::vector<Byte> stlEncryptedWithPasswordDerivedKey = ::EncryptUsingPasswordKey(oConfidentialUser.GetSerializedBuffer(), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end()));
            // Encrypt stlEncryptedWithPasswordDerivedKey with sail secret key
            StructuredBuffer oEncrypedConfidentialUser = ::EncryptUsingSailSecretKey(stlEncryptedWithPasswordDerivedKey);
            bsoncxx::types::b_binary oEncryptedSsb
            {
                bsoncxx::binary_sub_type::k_binary,
                uint32_t(oEncrypedConfidentialUser.GetSerializedBufferRawDataSizeInBytes()),
                oEncrypedConfidentialUser.GetSerializedBufferRawDataPtr()
            };
            bsoncxx::document::value oConfidentialUserDocumentValue = bsoncxx::builder::stream::document{}
            << "OrganizationOrUserUuid" << strUserGuid
            << "EncryptedSsb" << oEncryptedSsb
            << finalize;

            // Each client and transaction can only be used in a single thread
            mongocxx::pool::entry oClient = m_poMongoPool->acquire();
            // Access SailDatabase
            mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
            // Access BasicUser collection
            mongocxx::collection oBasicUserCollection = oSailDatabase["BasicUser"];
            // Create a transaction callback
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                // Insert document in the BasicUser collection
                auto oResult = oBasicUserCollection.insert_one(*poSession, oBasicUserDocumentValue.view());
                if (!oResult) {
                    std::cout << "Error while writing to the database." << std::endl;
                }
                else
                {
                    // Access Object collection
                    mongocxx::collection oConfidentialUserCollection = oSailDatabase["ConfidentialOrganizationOrUser"];
                    // Insert document in the ConfidentialOrganizationOrUser collection
                    oResult = oConfidentialUserCollection.insert_one(*poSession, oConfidentialUserDocumentValue.view());
                    if (!oResult) {
                        std::cout << "Error while writing to the database." << std::endl;
                    }
                    else
                    {
                        dwStatus = 201;
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
 * @function UpdateUserRights
 * @brief Update user's access rights
 * @param[in] c_oRequest contains user guid, and new rights bitmap
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateUserRights(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        Qword qwNewAccessRights = c_oRequest.GetQword("AccessRights");
        // Get the account encryption key to encrypt and decrypt confidential user record
        std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get user confidential document
        bsoncxx::stdx::optional<bsoncxx::document::value> oUserConfidentialDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                            << "OrganizationOrUserUuid" << strUserGuid 
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oUserConfidentialDocument)
        {
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                bsoncxx::document::element stlEncryptedSsb = oUserConfidentialDocument->view()["EncryptedSsb"];
                if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
                {
                    StructuredBuffer oConfidentialRecord(stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
                    // Decrypt record with the account encryption key
                    StructuredBuffer oDecryptedUserRecord(::DecryptUsingPasswordKey(::DecryptUsingSailSecretKey(oConfidentialRecord.GetSerializedBuffer()), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end())));
                    // Update user access rights in the decrypted record
                    oDecryptedUserRecord.PutQword("AccessRights", qwNewAccessRights);
                    // Encrypt updated user record
                    std::vector<Byte> stlNewEncryptedSsb = ::EncryptUsingSailSecretKey(::EncryptUsingPasswordKey(oDecryptedUserRecord.GetSerializedBuffer(), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end()))).GetSerializedBuffer();
                    // Create blob
                    bsoncxx::types::b_binary oNewEncryptedSsb
                    {
                        bsoncxx::binary_sub_type::k_binary,
                        uint32_t(stlNewEncryptedSsb.size()),
                        stlNewEncryptedSsb.data()
                    };
                    oSailDatabase["ConfidentialOrganizationOrUser"].update_one(*poSession, document{} << "OrganizationOrUserUuid" << strUserGuid << finalize,
                                                        document{} << "$set" << open_document <<
                                                        "EncryptedSsb" << oNewEncryptedSsb << close_document << finalize);
                }
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
 * @function UpdateOrganizationInformation
 * @brief Update organization information
 * @param[in] c_oRequest contains organization guid, and new information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateOrganizationInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        StructuredBuffer oOrganizationInformation = c_oRequest.GetStructuredBuffer("OrganizationInformation");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get organization confidential document
        bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                            << "OrganizationOrUserUuid" << strOrganizationGuid 
                                                                                                            << finalize);
        // Get organization basic document
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicDocument = oSailDatabase["BasicOrganization"].find_one(document{} 
                                                                                                            << "OrganizationUuid" << strOrganizationGuid 
                                                                                                            << finalize);
        if ((bsoncxx::stdx::nullopt != oConfidentialDocument) && (bsoncxx::stdx::nullopt != oBasicDocument))
        {
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                bsoncxx::document::element stlEncryptedSsb = oConfidentialDocument->view()["EncryptedSsb"];
                if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
                {
                    StructuredBuffer oConfidentialOrganization(stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
                    // TODO: Once the organization confidential record is encrypted in RegisterOrganization, Decrypt record, and then update it
                    // Update organization information
                    oConfidentialOrganization.PutString("OrganizationName", oOrganizationInformation.GetString("OrganizationName"));
                    oConfidentialOrganization.PutString("OrganizationAddress", oOrganizationInformation.GetString("OrganizationAddress"));
                    oConfidentialOrganization.PutString("PrimaryContactName", oOrganizationInformation.GetString("PrimaryContactName"));
                    oConfidentialOrganization.PutString("PrimaryContactTitle", oOrganizationInformation.GetString("PrimaryContactTitle"));
                    oConfidentialOrganization.PutString("PrimaryContactEmail", oOrganizationInformation.GetString("PrimaryContactEmail"));
                    oConfidentialOrganization.PutString("PrimaryContactPhoneNumber", oOrganizationInformation.GetString("PrimaryContactPhoneNumber"));
                    oConfidentialOrganization.PutString("SecondaryContactName", oOrganizationInformation.GetString("SecondaryContactName"));
                    oConfidentialOrganization.PutString("SecondaryContactTitle", oOrganizationInformation.GetString("SecondaryContactTitle"));
                    oConfidentialOrganization.PutString("SecondaryContactEmail", oOrganizationInformation.GetString("SecondaryContactEmail"));
                    oConfidentialOrganization.PutString("SecondaryContactPhoneNumber", oOrganizationInformation.GetString("SecondaryContactPhoneNumber"));
                    // Create blob
                    bsoncxx::types::b_binary oNewEncryptedSsb
                    {
                        bsoncxx::binary_sub_type::k_binary,
                        uint32_t(oConfidentialOrganization.GetSerializedBufferRawDataSizeInBytes()),
                        oConfidentialOrganization.GetSerializedBufferRawDataPtr()
                    };
                    // Update the confidential record
                    oSailDatabase["ConfidentialOrganizationOrUser"].update_one(*poSession, document{} << "OrganizationOrUserUuid" << strOrganizationGuid << finalize,
                                                        document{} << "$set" << open_document <<
                                                        "EncryptedSsb" << oNewEncryptedSsb << close_document << finalize);
                    // Update the basic record
                    oSailDatabase["BasicOrganization"].update_one(*poSession, document{} << "OrganizationUuid" << strOrganizationGuid << finalize,
                                                        document{} << "$set" << open_document <<
                                                        "OrganizationName" << oOrganizationInformation.GetString("OrganizationName") << close_document << finalize);
                }
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
 * @function UpdateUserInformation
 * @brief Update user information, excluding access rights
 * @param[in] c_oRequest user guid, and new user information
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdateUserInformation(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 204;

    try 
    {
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        StructuredBuffer oUserInformation = c_oRequest.GetStructuredBuffer("UserInformation");
        // Get the account encryption key to encrypt and decrypt confidential user record
        std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get user confidential document
        bsoncxx::stdx::optional<bsoncxx::document::value> oUserConfidentialDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                            << "OrganizationOrUserUuid" << strUserGuid 
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oUserConfidentialDocument)
        {
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                bsoncxx::document::element stlEncryptedSsb = oUserConfidentialDocument->view()["EncryptedSsb"];
                if (stlEncryptedSsb && stlEncryptedSsb.type() == type::k_binary)
                {
                    StructuredBuffer oConfidentialRecord(stlEncryptedSsb.get_binary().bytes, stlEncryptedSsb.get_binary().size);
                    // Decrypt record
                    StructuredBuffer oDecryptedUserRecord(::DecryptUsingPasswordKey(::DecryptUsingSailSecretKey(oConfidentialRecord.GetSerializedBuffer()), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end())));
                    // Update user information, excluding access rights
                    oDecryptedUserRecord.PutString("Username", oUserInformation.GetString("Name"));
                    oDecryptedUserRecord.PutString("Title", oUserInformation.GetString("Title"));
                    oDecryptedUserRecord.PutString("PhoneNumber", oUserInformation.GetString("PhoneNumber"));
                    // Encrypt updated user record
                    std::vector<Byte> stlNewEncryptedSsb = ::EncryptUsingSailSecretKey(::EncryptUsingPasswordKey(oDecryptedUserRecord.GetSerializedBuffer(), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end()))).GetSerializedBuffer();
                    // Create blob
                    bsoncxx::types::b_binary oNewEncryptedSsb
                    {
                        bsoncxx::binary_sub_type::k_binary,
                        uint32_t(stlNewEncryptedSsb.size()),
                        stlNewEncryptedSsb.data()
                    };
                    oSailDatabase["ConfidentialOrganizationOrUser"].update_one(*poSession, document{} << "OrganizationOrUserUuid" << strUserGuid << finalize,
                                                        document{} << "$set" << open_document <<
                                                        "EncryptedSsb" << oNewEncryptedSsb << close_document << finalize);
                }
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
 * @function UpdatePassword
 * @brief Update user password
 * @param[in] c_oRequest email and new password
 * @throw BaseException Error StructuredBuffer element not found
 * @returns status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::UpdatePassword(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        std::string strEmail = c_oRequest.GetString("Email");
        std::string strCurrentPassword = c_oRequest.GetString("CurrentPassword");
        std::string strNewPassword = c_oRequest.GetString("NewPassword");
        // Get the account encryption key to decrypt the confidential user record
        std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");

        // Get 64 bit hash of "strEmail/strCurrentPassword" string
        Qword qw64BitHashCurrentPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strCurrentPassword).c_str(), false);
        // Get 64 bit hash of "strEmail/strNewPassword" string
        Qword qw64BitHashNewPassphrase = ::Get64BitHashOfNullTerminatedString((strEmail + "/" + strNewPassword).c_str(), false);
        // Generate "strEmail/strNewPassword" derived key
        std::string strDerivedKey = ::Base64HashOfEmailPassword(strEmail, strNewPassword);
        // Wrap account encryption key with strDerivedKey
        std::vector<Byte> stlWrappedAccountEncryptionKey = ::EncryptUsingPasswordKey(stlAccountEncryptionKey, strDerivedKey);
        bsoncxx::types::b_binary oWrappedAccountEncryptionKey {bsoncxx::binary_sub_type::k_binary,
                                        uint32_t(stlWrappedAccountEncryptionKey.size()),
                                        stlWrappedAccountEncryptionKey.data()};

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Get basic user document
        // Checks if the user exists and if its the same as the logged in user
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUserDocument = oSailDatabase["BasicUser"].find_one(document{} 
                                                                                                            << "64BitHash" << (double) qw64BitHashCurrentPassphrase
                                                                                                            << "UserUuid" << strUserGuid
                                                                                                            << finalize);
        if (bsoncxx::stdx::nullopt != oBasicUserDocument)
        {
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                oSailDatabase["BasicUser"].update_one(*poSession, document{} << "64BitHash" << (double) qw64BitHashCurrentPassphrase << finalize,
                                                    document{} << "$set" << open_document <<
                                                    "64BitHash" << (double) qw64BitHashNewPassphrase << 
                                                    "WrappedAccountKey" << oWrappedAccountEncryptionKey << close_document 
                                                    << finalize);
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
 * @function ListOrganizations
 * @brief Return a list of organizations
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing list of all organizations
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListOrganizations(
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
        // Fetch all basic organization records
        mongocxx::cursor oBasicRecordsCursor = oSailDatabase["BasicOrganization"].find({});
        // Loop through returned documents and add information to the list
        StructuredBuffer oListOfOrganizations;
        for (auto&& oDocumentView : oBasicRecordsCursor)
        {
            bsoncxx::document::element oOrganizationUuid = oDocumentView["OrganizationUuid"];
            if (oOrganizationUuid && oOrganizationUuid.type() == type::k_utf8)
            {
                std::string strOrganizationGuid = oOrganizationUuid.get_utf8().value.to_string();
                bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialRecord = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} << "OrganizationOrUserUuid" << strOrganizationGuid << finalize);
                if (bsoncxx::stdx::nullopt != oConfidentialRecord)
                {
                    bsoncxx::document::element oEncryptedSsb = oConfidentialRecord->view()["EncryptedSsb"];
                    if (oEncryptedSsb && oEncryptedSsb.type() == type::k_binary)
                    {
                        // TODO: Once the organization confidential record is encrypted in RegisterOrganization, Decrypt record, and then add it to the list 
                        oListOfOrganizations.PutStructuredBuffer(strOrganizationGuid.c_str(), StructuredBuffer(oEncryptedSsb.get_binary().bytes, oEncryptedSsb.get_binary().size));
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("Organizations", oListOfOrganizations);
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
 * @function ListUsers
 * @brief Return a list of all users
 * @param[in] c_oRequest contains the request body
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing list of all users
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListUsers(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Get the account encryption key to encrypt and decrypt confidential user record
        std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch all basic user records
        mongocxx::cursor oBasicRecordsCursor = oSailDatabase["BasicUser"].find({});
        // Loop through returned documents and get list of all users
        StructuredBuffer oListOfUsers;
        for (auto&& oDocumentView : oBasicRecordsCursor)
        {
            bsoncxx::document::element oUserUuid = oDocumentView["UserUuid"];
            bsoncxx::document::element oAccountStatus = oDocumentView["AccountStatus"];
            if ((oUserUuid && oUserUuid.type() == type::k_utf8) && (oAccountStatus && oAccountStatus.type() == type::k_double))
            {
                std::string strUserGuid = oUserUuid.get_utf8().value.to_string();
                Dword dwAccountStatus = (Dword) oAccountStatus.get_double().value;
                // Get confidential record associated with the user guid
                bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialUserDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                                    << "OrganizationOrUserUuid" << strUserGuid 
                                                                                                                    << finalize);
                if (bsoncxx::stdx::nullopt != oConfidentialUserDocument)
                {
                    bsoncxx::document::element oEncryptedSsb = oConfidentialUserDocument->view()["EncryptedSsb"];
                    if (oEncryptedSsb && oEncryptedSsb.type() == type::k_binary)
                    {
                        StructuredBuffer oConfidentialRecord(oEncryptedSsb.get_binary().bytes, oEncryptedSsb.get_binary().size);
                        // Decrypt record
                        StructuredBuffer oDecryptedUserRecord(::DecryptUsingPasswordKey(::DecryptUsingSailSecretKey(oConfidentialRecord.GetSerializedBuffer()), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end())));
                        // Add account status to the response
                        oDecryptedUserRecord.PutDword("AccountStatus", dwAccountStatus);
                        // Remove confidential or unnecessary fields
                        oDecryptedUserRecord.RemoveElement("UserRootKeyGuid");
                        oListOfUsers.PutStructuredBuffer(strUserGuid.c_str(), oDecryptedUserRecord);
                    }
                }
            }
        }

        oResponse.PutStructuredBuffer("Users", oListOfUsers);
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
 * @function ListOrganizationUsers
 * @brief Return a list of users for an organization
 * @param[in] c_oRequest contains organization guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Serialized StructuredBuffer containing list of users for an organization
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::ListOrganizationUsers(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Get the account encryption key to encrypt and decrypt confidential user record
        std::vector<Byte> stlAccountEncryptionKey = c_oRequest.GetBuffer("AccountEncryptionKey");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Fetch all basic user records for the organization
        mongocxx::cursor oBasicRecordsCursor = oSailDatabase["BasicUser"].find(document{} 
                                                                                << "OrganizationUuid" << strOrganizationGuid
                                                                                << finalize);
        // Loop through returned documents and get list of all users in the organization
        StructuredBuffer oListOfUsers;
        for (auto&& oDocumentView : oBasicRecordsCursor)
        {
            bsoncxx::document::element oUserUuid = oDocumentView["UserUuid"];
            bsoncxx::document::element oAccountStatus = oDocumentView["AccountStatus"];
            if ((oUserUuid && oUserUuid.type() == type::k_utf8) && (oAccountStatus && oAccountStatus.type() == type::k_double))
            {
                std::string strUserGuid = oUserUuid.get_utf8().value.to_string();
                Dword dwAccountStatus = (Dword) oAccountStatus.get_double().value;
                // Get confidential record associated with the user guid
                bsoncxx::stdx::optional<bsoncxx::document::value> oConfidentialUserDocument = oSailDatabase["ConfidentialOrganizationOrUser"].find_one(document{} 
                                                                                                                    << "OrganizationOrUserUuid" << strUserGuid 
                                                                                                                    << finalize);
                if (bsoncxx::stdx::nullopt != oConfidentialUserDocument)
                {
                    bsoncxx::document::element oEncryptedSsb = oConfidentialUserDocument->view()["EncryptedSsb"];
                    if (oEncryptedSsb && oEncryptedSsb.type() == type::k_binary)
                    {
                        StructuredBuffer oConfidentialRecord(oEncryptedSsb.get_binary().bytes, oEncryptedSsb.get_binary().size);
                        // Decrypt record
                        StructuredBuffer oDecryptedUserRecord(::DecryptUsingPasswordKey(::DecryptUsingSailSecretKey(oConfidentialRecord.GetSerializedBuffer()), std::string(stlAccountEncryptionKey.begin(), stlAccountEncryptionKey.end())));
                        // Add account status to the response
                        oDecryptedUserRecord.PutDword("AccountStatus", dwAccountStatus);
                        // Remove confidential or unnecessary fields
                        oDecryptedUserRecord.RemoveElement("UserRootKeyGuid");
                        oListOfUsers.PutStructuredBuffer(strUserGuid.c_str(), oDecryptedUserRecord);
                    }
                }
            }
        }
        oResponse.PutStructuredBuffer("OrganizationUsers", oListOfUsers);
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
 * @function RecoverUser
 * @brief Recover a deleted user in the database
 * @param[in] c_oRequest contains user guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::RecoverUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Get user guid of the user to be recovered
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        // Get organization guid of admin to compare it with the user's organization
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        // Get basic user document
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUserDocument = oSailDatabase["BasicUser"].find_one(document{} 
                                                                                                            << "UserUuid" << strUserGuid 
                                                                                                            << "OrganizationUuid" << strOrganizationGuid
                                                                                                            << finalize);

        if (bsoncxx::stdx::nullopt != oBasicUserDocument)
        {
            // Update the account status
            mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
            {
                oSailDatabase["BasicUser"].update_one(*poSession, document{} << "UserUuid" << strUserGuid << finalize,
                                                        document{} << "$set" << open_document <<
                                                        "AccountStatus" << (double) AccountStatus::eOpen << close_document << finalize);
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
 * @function DeleteUser
 * @brief Delete a user from the database
 * @param[in] c_oRequest contains user guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::DeleteUser(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        // Get user guid of the user to be deleted
        std::string strUserGuid = c_oRequest.GetString("UserGuid");
        // Get organization guid of admin to compare it with the user's organization
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        bool fIsHardDelete = false;
        if (true == c_oRequest.IsElementPresent("IsHardDelete", BOOLEAN_VALUE_TYPE))
        {
            fIsHardDelete = c_oRequest.GetBoolean("IsHardDelete");
        }

        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];

        // Get basic user document
        bsoncxx::stdx::optional<bsoncxx::document::value> oBasicUserDocument = oSailDatabase["BasicUser"].find_one(document{} 
                                                                                                            << "UserUuid" << strUserGuid 
                                                                                                            << "OrganizationUuid" << strOrganizationGuid
                                                                                                            << finalize);

        if (bsoncxx::stdx::nullopt != oBasicUserDocument)
        {
            if (true == fIsHardDelete)
            {
                // Delete all documents from BasicUser and ConfidentialOrganizationOrUser collections associated with UserGuid
                mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
                {
                    oSailDatabase["BasicUser"].delete_one(*poSession, document{} << "UserUuid" << strUserGuid << finalize);
                    oSailDatabase["ConfidentialOrganizationOrUser"].delete_one(*poSession, document{} << "OrganizationOrUserUuid" << strUserGuid << finalize);
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
            else
            {
                // Update the account status
                mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession) 
                {
                    oSailDatabase["BasicUser"].update_one(*poSession, document{} << "UserUuid" << strUserGuid << finalize,
                                                            document{} << "$set" << open_document <<
                                                            "AccountStatus" << (double) AccountStatus::eClosed << close_document << finalize);
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
 * @function DeleteOrganization
 * @brief Delete an organization and its users from the database
 * @param[in] c_oRequest contains user guid
 * @throw BaseException Error StructuredBuffer element not found
 * @returns Status of the transaction
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall DatabaseManager::DeleteOrganization(
    _in const StructuredBuffer & c_oRequest
    )
{
    __DebugFunction();

    // TODO: Add a check for IsHardDelete and implement soft delete if IsHardDelete is false

    StructuredBuffer oResponse;

    Dword dwStatus = 404;

    try 
    {
        std::string strOrganizationGuid = c_oRequest.GetString("OrganizationGuid");
        bool fIsHardDelete = c_oRequest.GetBoolean("IsHardDelete");
        // Each client and transaction can only be used in a single thread
        mongocxx::pool::entry oClient = m_poMongoPool->acquire();
        // Access SailDatabase
        mongocxx::database oSailDatabase = (*oClient)["SailDatabase"];
        // Delete all documents from collections associated with organization guid
        mongocxx::client_session::with_transaction_cb oCallback = [&](mongocxx::client_session * poSession)
        {
            oSailDatabase["BasicOrganization"].delete_one(*poSession, document{} << "OrganizationUuid" << strOrganizationGuid << finalize);
            oSailDatabase["ConfidentialOrganizationOrUser"].delete_one(*poSession, document{} << "OrganizationOrUserUuid" << strOrganizationGuid << finalize);
            // Delete all user records for the organization
            mongocxx::cursor oBasicRecordsCursor = oSailDatabase["BasicUser"].find(document{} 
                                                                                    << "OrganizationUuid" << strOrganizationGuid
                                                                                    << finalize);
            for (auto&& oDocumentView : oBasicRecordsCursor)
            {
                bsoncxx::document::element oUserUuid = oDocumentView["UserUuid"];
                if (oUserUuid && oUserUuid.type() == type::k_utf8)
                {
                    std::string strUserGuid = oUserUuid.get_utf8().value.to_string();
                    // Delete records associated with strUserGuid
                    oSailDatabase["ConfidentialOrganizationOrUser"].delete_one(*poSession, document{} << "OrganizationOrUserUuid" << strUserGuid << finalize);
                    oSailDatabase["BasicUser"].delete_one(*poSession, document{} << "UserUuid" << strUserGuid << finalize);
                }
            }
            // Delete all audit events associated with the organization
            mongocxx::cursor oAuditRecordsCursor = oSailDatabase["AuditLog"].find(document{} 
                                                                                    << "OrganizationGuid" << strOrganizationGuid
                                                                                    << finalize);
            for (auto&& oDocumentView : oAuditRecordsCursor)
            {
                bsoncxx::document::element oPlainTextObjectBlobGuid = oDocumentView["PlainTextObjectBlobGuid"];
                if (oPlainTextObjectBlobGuid && oPlainTextObjectBlobGuid.type() == type::k_utf8)
                {
                    std::string strPlainTextObjectBlobGuid = oPlainTextObjectBlobGuid.get_utf8().value.to_string();
                    // Get PlainTextObjectBlob record associated with strPlainTextObjectBlobGuid
                    bsoncxx::stdx::optional<bsoncxx::document::value> oPlainTextObjectDocument = oSailDatabase["PlainTextObjectBlob"].find_one(document{} 
                                                                                                                        << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid 
                                                                                                                        << finalize);
                    if (bsoncxx::stdx::nullopt != oPlainTextObjectDocument)
                    {
                        bsoncxx::document::element oObjectGuid = oPlainTextObjectDocument->view()["ObjectGuid"];
                        if (oObjectGuid && oObjectGuid.type() == type::k_utf8)
                        {
                            std::string strObjectGuid = oObjectGuid.get_utf8().value.to_string();
                            // Delete record associated with strObjectGuid
                            oSailDatabase["Object"].delete_one(*poSession, document{} << "ObjectGuid" << strObjectGuid << finalize);
                            // Delete records associated with strPlainTextObjectBlobGuid
                            oSailDatabase["PlainTextObjectBlob"].delete_one(*poSession, document{} << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid << finalize);
                            oSailDatabase["AuditLog"].delete_one(*poSession, document{} << "PlainTextObjectBlobGuid" << strPlainTextObjectBlobGuid << finalize);
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