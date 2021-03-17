#include "64BitHashes.h"
#include "CryptoUtils.h"
#include "DateAndTime.h"
#include "Exceptions.h"
#include "InteractiveClient.h"

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

/********************************************************************************************/

typedef enum rights
{
    eAdmin = 1,
    eAuditor = 2,
    eOrganizationUser = 3,
    eDigitalContractAdmin = 4,
    eDatasetAdmin = 5
}
AccessRights;

typedef struct
{
    std::string m_strEmail;
    std::string m_strName;
    std::string m_strTitle;
    std::string m_strPhoneNumber;
    Qword m_qwAccessRights;
} UserInformation;

typedef struct
{
    std::string m_strOrganizationName;
    std::string m_strOrganizationAddress;
    std::string m_strPrimaryContactName;
    std::string m_strPrimaryContactTitle;
    std::string m_strPrimaryContactEmail;
    std::string m_strPrimaryContactPhoneNumber;
    std::string m_strSecondaryContactName;
    std::string m_strSecondaryContactTitle;
    std::string m_strSecondaryContactEmail;
    std::string m_strSecondaryContactPhoneNumber;
} OrganizationInformation;

mongocxx::instance oInstance{}; // Create only one instance
mongocxx::client * g_oClient = nullptr;

/********************************************************************************************/

void AddUserAccountsToDatabase(
    const StructuredBuffer & c_oBasicUser,
    const StructuredBuffer & c_oConfidentialUser
    )
{
    // Access the SailDatabase
    mongocxx::database oDatabase = (*g_oClient)["SailDatabase"];
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
        << "64BitHash" << (double)c_oBasicUser.GetQword("64BitHash")
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
    // Create the instance of monogd client
    g_oClient = new mongocxx::client   // Use mongocxx client to connect to MongoDB instance
    {
        mongocxx::uri{"mongodb://localhost:27017"}
    };
    _ThrowIfNull(g_oClient, "Cannot connect to mongod server", nullptr);

    try
    {
        std::string strPassword = "sailpassword";
        // Organizations informations
        std::vector<OrganizationInformation> stlOrganizations;
        stlOrganizations.push_back({"Zerg", "Char", "Naruto", "Supply Generator", "overlord@zerg.com", "000-000-0000", "Saitama", "Network Engineer", "nydus@zerg.com", "000-000-0000"});
        stlOrganizations.push_back({"Terran", "Earth", "Eren", "Security Expert", "marine@terran.com", "000-000-0000", "Gon", "Hardware Engineer", "scv@t1erran.com", "000-000-0000"});
        // Super admins information
        std::vector<UserInformation> stlAdmins;
        stlAdmins.push_back({"queen@zerg.com", "Queen of Blades", "Hardware Engineer", "000-000-0000", eAdmin});
        stlAdmins.push_back({"marine@terran.com", "Eren", "Security Expert", "000-000-0000", eAdmin});
        // Users information
        std::vector<UserInformation> stlAccountDb;
        stlAccountDb.push_back({"larva@zerg.com", "Larva", "Security Expert", "000-000-0000", eOrganizationUser});
        stlAccountDb.push_back({"overlord@zerg.com", "Lord", "Supply Generator", "000-000-0000", eOrganizationUser});
        stlAccountDb.push_back({"zergling@zerg.com", "Zergling", "Network Engineer", "000-000-0000", eAuditor});
        stlAccountDb.push_back({"roach@zerg.com", "Roach", "Field Engineer", "000-000-0000", eDigitalContractAdmin});
        stlAccountDb.push_back({"lurker@zerg.com", "Lurker", "Market Discovery Expert", "000-000-0000", eDatasetAdmin});
        stlAccountDb.push_back({"scv@terran.com", "Gon", "Hardware Engineer", "000-000-0000", eOrganizationUser});
        stlAccountDb.push_back({"marauder@terran.com", "Naruto", "Supply Generator", "000-000-0000", eOrganizationUser});
        stlAccountDb.push_back({"reaper@terran.com", "Saitama", "Network Engineer", "000-000-0000", eAuditor});
        stlAccountDb.push_back({"ghost@terran.com", "Alex", "Field Engineer", "000-000-0000", eDigitalContractAdmin});
        stlAccountDb.push_back({"banshee@terran.com", "Yagami", "Market Discovery Expert", "000-000-0000", eDatasetAdmin});
        // Register organizations, super admins and other users
        for (unsigned int unIndex = 0; unIndex < stlOrganizations.size(); ++unIndex)
        {
            StructuredBuffer oOrganizationInformation;
            oOrganizationInformation.PutString("Email", stlAdmins.at(unIndex).m_strEmail);
            oOrganizationInformation.PutString("Password", strPassword);
            oOrganizationInformation.PutString("Name", stlAdmins.at(unIndex).m_strName);
            oOrganizationInformation.PutString("PhoneNumber", stlAdmins.at(unIndex).m_strPhoneNumber);
            oOrganizationInformation.PutString("Title", stlAdmins.at(unIndex).m_strTitle);
            oOrganizationInformation.PutString("OrganizationName", stlOrganizations.at(unIndex).m_strOrganizationName);
            oOrganizationInformation.PutString("OrganizationAddress", stlOrganizations.at(unIndex).m_strOrganizationAddress);
            oOrganizationInformation.PutString("PrimaryContactName", stlOrganizations.at(unIndex).m_strPrimaryContactName);
            oOrganizationInformation.PutString("PrimaryContactTitle", stlOrganizations.at(unIndex).m_strPrimaryContactTitle);
            oOrganizationInformation.PutString("PrimaryContactEmail", stlOrganizations.at(unIndex).m_strPrimaryContactEmail);
            oOrganizationInformation.PutString("PrimaryContactPhoneNumber", stlOrganizations.at(unIndex).m_strPrimaryContactPhoneNumber);
            oOrganizationInformation.PutString("SecondaryContactName", stlOrganizations.at(unIndex).m_strSecondaryContactName);
            oOrganizationInformation.PutString("SecondaryContactTitle", stlOrganizations.at(unIndex).m_strSecondaryContactTitle);
            oOrganizationInformation.PutString("SecondaryContactEmail", stlOrganizations.at(unIndex).m_strSecondaryContactEmail);
            oOrganizationInformation.PutString("SecondaryContactPhoneNumber", stlOrganizations.at(unIndex).m_strSecondaryContactPhoneNumber);
            // Add organizations and their super admins to the database
            bool fSuccess = ::RegisterOrganizationAndSuperUser(oOrganizationInformation);
            _ThrowBaseExceptionIf((false == fSuccess), "Error registering organization and its super admin.", nullptr);
            // Login to the web services
            std::string strEncodedEosb = Login(stlAdmins.at(unIndex).m_strEmail, strPassword);
            _ThrowBaseExceptionIf((0 == strEncodedEosb.size()), "Exiting!", nullptr);
            // Get organization guid from eosb
            StructuredBuffer oUserInformation(::GetBasicUserInformation(strEncodedEosb));
            std::string strOrganizationGuid = oUserInformation.GetString("OrganizationGuid");
            unsigned int unUserIndex = unIndex * 5; // 5 is number of users being added per organization
            for (; unUserIndex < (unIndex * 5 + 5); ++unUserIndex)
            {
                StructuredBuffer oUserInformation;
                oUserInformation.PutString("Email", stlAccountDb.at(unUserIndex).m_strEmail);
                oUserInformation.PutString("Password", strPassword);
                oUserInformation.PutString("Name", stlAccountDb.at(unUserIndex).m_strName);
                oUserInformation.PutString("PhoneNumber", stlAccountDb.at(unUserIndex).m_strPhoneNumber);
                oUserInformation.PutString("Title", stlAccountDb.at(unUserIndex).m_strTitle);
                oUserInformation.PutQword("AccessRights", stlAccountDb.at(unUserIndex).m_qwAccessRights);
                ::RegisterUser(strEncodedEosb, strOrganizationGuid, oUserInformation);
            }
        }
        std::cout << "Accounts added!\n";
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

    delete g_oClient;
    return 0;
}
