#include "64BitHashes.h"
#include "CryptoUtils.h"
#include "Exceptions.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

#include <iostream>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

// For testing mongocxx::pool
#include <sstream>
#include <thread>
#include <vector>
#include <mongocxx/pool.hpp>
using bsoncxx::builder::basic::kvp;

void AddUserAccountsToDatabase(
    const StructuredBuffer & c_oBasicUser,
    const StructuredBuffer & c_oConfidentialUser
    )
{
    mongocxx::instance oInstance{}; // Create only one instance
    mongocxx::client * oClient = new mongocxx::client   // Use mongocxx client to connect to MongoDB instance
    {
        mongocxx::uri{"mongodb://localhost:27017"}
    };
    // Access the SailDatabase
    mongocxx::database oDatabase = (*oClient)["SailDatabase"];
    // Access the ConfidentialOrganizationOrUser collection
    mongocxx::collection oConfidentialCollection = oDatabase["ConfidentialOrganizationOrUser"];
    // Create a document
    bsoncxx::types::b_binary oEncryptedSsb {bsoncxx::binary_sub_type::k_binary,
                              uint32_t(c_oConfidentialUser.GetSerializedBufferRawDataSizeInBytes()),
                              c_oConfidentialUser.GetSerializedBufferRawDataPtr()};

    auto oBuilder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value oConfidentialDocumentValue = oBuilder
      << "OrganizationOrUserUuid" << c_oBasicUser.GetString("UserUuid")
      << "EncryptedSsb" << oEncryptedSsb
      << finalize;

    // Insert document in the collection
    auto oResult = oConfidentialCollection.insert_one(oConfidentialDocumentValue.view());

    if (!oResult) {
        std::cout << "Error while writing to the database." << std::endl;
    }
    else
    {
        // Access a collection
        mongocxx::collection oBasicUserCollection = oDatabase["BasicUser"];
        // Create a document
        std::vector stlWrappedAccountKey = c_oBasicUser.GetBuffer("WrappedAccountKey");
        bsoncxx::types::b_binary oWrappedAccountKey {bsoncxx::binary_sub_type::k_binary,
                                  uint32_t(stlWrappedAccountKey.size()),
                                  stlWrappedAccountKey.data()};
        bsoncxx::document::value oBasicUserDocumentValue = oBuilder
        << "64BitHash" << (double)c_oBasicUser.GetQword("64BitHashedPassphrase")
        << "OrganizationUuid" << c_oBasicUser.GetString("OrganizationUuid")
        << "UserUuid" << c_oBasicUser.GetString("UserUuid")
        << "AccountStatus" << (double)c_oBasicUser.GetDword("AccountStatus")
        << "WrappedAccountKey" << oWrappedAccountKey
        << finalize;

        // Insert document in the collection
        oResult = oBasicUserCollection.insert_one(oBasicUserDocumentValue.view());

        if (!oResult) {
            std::cout << "Error while writing to the database." << std::endl;
        }
    }

    delete oClient;
}

// Testing mongocxx::pool
void RunThreadedInserts()
{
    mongocxx::instance oInstance{}; // Create only one instance
    mongocxx::pool oPool{mongocxx::uri{"mongodb://localhost:27017"}};
    std::vector<std::thread> threads{};

    for (auto i : {0, 1, 2, 3, 4, 5}) {
        auto run = [&](std::int64_t j) {
            // Each client and collection can only be used in a single thread.
            auto client = oPool.acquire();
            auto coll = (*client)["TestThreading"]["TestCase"];

            bsoncxx::types::b_int64 index = {j};
            coll.insert_one(bsoncxx::builder::basic::make_document(kvp("x", index)));
        };

        std::thread runner{run, i};

        threads.push_back(std::move(runner));
    }

    for (auto&& runner : threads) {
        runner.join();
    }
}

int main()
{
    try
    {
        std::string strEmail, strPassword = "sailpassword", strPassphrase, strUserName, strTitle;
        std::cout << "Enter email address: ";
        getline(std::cin, strEmail, '\n');
        std::cout << "Enter user name: ";
        getline(std::cin, strUserName, '\n');
        std::cout << "Enter title in the organization: ";
        getline(std::cin, strTitle, '\n');
        strPassphrase = strEmail + "/" + strPassword;
        Qword qw64BitHashedPassphrase = ::Get64BitHashOfNullTerminatedString(strPassphrase.c_str(), false);

        std::string strHashedPassphrase = ::Base64HashOfEmailPassword(strEmail, strPassword);

        // Generate Account Key
        std::vector<Byte> stlAccountKey = ::GenerateAccountKey();
        // Encrypt it using the password derived key
        std::vector<Byte> stlWrappedAccountKey = ::EncryptUsingPasswordKey(stlAccountKey, strHashedPassphrase);
        // Create a Basic User Record as described in the 'Account' subsection of Internal Web Service Constructs
        Guid oUserId, oOrganizationId, oUserRootKeyId;
        StructuredBuffer oBasicUserRecord;
        oBasicUserRecord.PutQword("64BitHashedPassphrase", qw64BitHashedPassphrase);
        oBasicUserRecord.PutString("OrganizationUuid", oOrganizationId.ToString(eHyphensAndCurlyBraces));
        oBasicUserRecord.PutString("UserUuid", oUserId.ToString(eHyphensAndCurlyBraces));
        oBasicUserRecord.PutDword("AccountStatus", 0x1);
        oBasicUserRecord.PutBuffer("WrappedAccountKey", stlWrappedAccountKey);

        // Create a the Confidential User Record
        StructuredBuffer oConfidentialUserRecord;
        oConfidentialUserRecord.PutGuid("UserGuid", oUserId);
        oConfidentialUserRecord.PutGuid("UserRootKeyGuid", oUserRootKeyId);
        oConfidentialUserRecord.PutQword("AccountRights", 0x1);
        oConfidentialUserRecord.PutString("Username", strUserName);
        oConfidentialUserRecord.PutString("Title", strTitle);
        oConfidentialUserRecord.PutString("EmailAddress", strEmail);
        oConfidentialUserRecord.PutString("PhoneNumber", "000-000-0000");

        std::vector<Byte> stlPasswordKeyEncrypted = ::EncryptUsingPasswordKey(oConfidentialUserRecord.GetSerializedBuffer(), strHashedPassphrase);
        StructuredBuffer oEncryptedBuffer = ::EncryptUsingSailSecretKey(stlPasswordKeyEncrypted);

        // Insert documents
        ::AddUserAccountsToDatabase(oBasicUserRecord, oEncryptedBuffer);
        // ::RunThreadedInserts();
    }
    catch(BaseException & oBaseException)
    {
        std::cout << "Exception caught\n";
        std::cout << oBaseException.GetExceptionMessage() << std::endl;
    }
    catch(...)
    {
        std::cout << "Some exception\n";
    }

    std::cout << "That's all folks\n";

    return 0;
}
