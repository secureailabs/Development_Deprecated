/*********************************************************************************************
 *
 * @file RemoteDataConnector.cpp
 * @author Prawal Gangwar
 * @date 31 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "RemoteDataConnector.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "CurlRest.h"
#include "JsonValue.h"
#include "ExceptionRegister.h"
#include "FileUtils.h"
#include "TlsClient.h"
#include "TlsTransactionHelperFunctions.h"
#include "Base64Encoder.h"

#include <iostream>
#include <filesystem>
#include <thread>
#include <mutex>

#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>

/********************************************************************************************
 *
 * @function GetRemoteDataConnector
 * @brief Gets the singleton instance reference of the RemoteDataConnector object
 *
 ********************************************************************************************/

RemoteDataConnector & __stdcall RemoteDataConnector::Get(void)
{
    __DebugFunction();

    return m_oRemoteDataConnector;
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function RemoteDataConnector
 * @brief Constructor to create a RemoteDataConnector object
 *
 ********************************************************************************************/

RemoteDataConnector::RemoteDataConnector(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function ~RemoteDataConnector
 * @brief object Destructor
 *
 ********************************************************************************************/

RemoteDataConnector::~RemoteDataConnector(void)
{
    __DebugFunction();

}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function SetDatasetFolderAndRun
 * @brief Register the Eosb of the Orchestrator user for login
 * @param[in] c_oStructuredBuffer The folder to watch for datasets dynamically
 *
 ********************************************************************************************/
void __thiscall RemoteDataConnector::SetDatasetFolderAndRun(
    _in const std::string & c_strFolderName
)
{
    __DebugFunction();

    std::cout << "FileSystemWatcherThread" << std::endl;

    // Start the HeartBeat thread that will start sending the heartbeat to the backend as
    // soon as RemoteDataConnector is registered
    std::thread(&RemoteDataConnector::SendDataConnectorHeartbeat, this).detach();

    int nINotifyFd = ::inotify_init();
    _ThrowBaseExceptionIf((0 >= nINotifyFd), "Unable to create a inotify object", nullptr);

    // Check for existing datasets in the directory and process them
    for (auto & stlFileInDirectory : std::filesystem::directory_iterator(c_strFolderName))
    {
        std::cout << "Existing file found " << stlFileInDirectory.path() << std::endl;
        if (true == stlFileInDirectory.is_regular_file())
        {
            std::thread(&RemoteDataConnector::NewDatasetFoundCallback, this, stlFileInDirectory.path()).detach();
        }
    }

    // Add the directory we want to watch for the new Datasets
    int nDirectoryToWatchFd = ::inotify_add_watch(nINotifyFd, c_strFolderName.c_str(), IN_CREATE);
    _ThrowBaseExceptionIf((-1 == nDirectoryToWatchFd), "Could not watch : *s\n", c_strFolderName.c_str());

    // Allocate memory space for the filesystem events
    std::vector<Byte> stlNotifyEvent(sizeof(struct inotify_event) + NAME_MAX + 1);

    bool fKeepRunning = true;
    while(true == fKeepRunning)
    {
        // This is a blocking call is only invoked when a file system related event is
        // is reported in the specified directory
        std::cout << "Blocked on reading\n";
        int nLengthOfData = ::read(nINotifyFd, stlNotifyEvent.data(), stlNotifyEvent.size());

        struct inotify_event * poInotifyEvent = (struct inotify_event *)stlNotifyEvent.data();
        if(poInotifyEvent->len && (poInotifyEvent->mask & IN_CREATE))
        {
            // For everyfile created we call a JobEngine callback function which should
            // find the most efficient way to handle such a file.
            std::cout << "FileCreateCallback for " << poInotifyEvent->name << std::endl;
            std::thread(&RemoteDataConnector::NewDatasetFoundCallback, this, c_strFolderName + "/" + poInotifyEvent->name).detach();
        }
    }

    // Cleanup the event listener
    ::inotify_rm_watch( nINotifyFd, nDirectoryToWatchFd);
    ::close(nINotifyFd);
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function NewDatasetFoundCallback
 * @brief Register the Eosb of the Orchestrator user for login
 * @param[in] c_oStructuredBuffer
 *
 ********************************************************************************************/
void __thiscall RemoteDataConnector::NewDatasetFoundCallback(
    _in const std::string & c_strDatasetName
) throw()
{
    __DebugFunction();
    __DebugAssert(0 < m_strRestPortalAddress.length());

    std::cout << "New dataset found " << c_strDatasetName << std::endl;
    try
    {
        // Heartbeats sent only for datasets that have been registered successfully
        bool fIsDatasetRegistered = false;

        // Verify the Dataset
        StructuredBuffer oStructuredBufferDataset = this->VerifyDataset(c_strDatasetName);
        if (0 != oStructuredBufferDataset.GetNamesOfElements().size())
        {
            // The registration needs to happen only once but since this function can be called
            // by multiple threads, a lock is taken on the fIsDatasetRegistered variable
            // so that it is updated by only one thread
            std::lock_guard lock(m_stlMutexRestConnection);

            std::string strDatasetGuid = oStructuredBufferDataset.GetString("DatasetUuid");
            std::cout << "strDatasetGuid " << strDatasetGuid << std::endl;
            m_oCollectionOfDatasets.PutStructuredBuffer(strDatasetGuid.c_str(), oStructuredBufferDataset);
            if (true == this->UpdateDatasets())
            {
                std::cout << "Update datset success for " << c_strDatasetName << std::endl;
                m_fIsDataConnectorRegistered = true;
                fIsDatasetRegistered = true;
                // Add the dataset file to the map of files to datasetguid
                m_stlMapOfFileToDatasetsUuids.insert(std::make_pair(c_strDatasetName, strDatasetGuid));
            }
            else
            {
                std::cout << "Update datset failed for " << c_strDatasetName << std::endl;
                // on failure remove the dataset from the list maintained
                m_oCollectionOfDatasets.RemoveElement(strDatasetGuid.c_str());
            }
        }
    }
    catch (const BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function SendDataConnectorHeartbeat
 * @brief Inform the SAIL WebPortal that the dataset is ready for use
 * @param[in] c_strDatasetId
 * @return The Response StrucutredBuffer
 ********************************************************************************************/
void __thiscall RemoteDataConnector::SendDataConnectorHeartbeat(void) throw()
{
    __DebugFunction();
    __DebugAssert(0 < m_strRestPortalAddress.length());

    try
    {
        std::mutex oMutex;
        std::unique_lock<std::mutex> oLock(oMutex);
        m_stlNotifyRegistration.wait(oLock);

        std::cout << "Registration done starting to send heartbeat" << std::endl;
        // Keep sending the data available pings to the Rest Portal until the file
        // is deleted from by the user from the Dataset folder. That is the only way to
        // stop this thread.
        StructuredBuffer oHeartbeatRequest;
        oHeartbeatRequest.PutString("RemoteDataConnectorGuid", m_oGuidDataConnector.ToString(eHyphensAndCurlyBraces));
        do
        {
            // Send the ping to the REST portal and get a response stating if any
            // Virtual Machine is waiting for this dataset
            std::string strVerb = "PUT";
            std::string strApiUrl = "/SAIL/RemoteDataConnectorManager/HeartBeat?Eosb="+ m_strUserEosb;
            auto oJsonBody = JsonValue::ParseStructuredBufferToJson(oHeartbeatRequest);
            std::string strJsonBody = oJsonBody->ToString();
            oJsonBody->Release();
            std::cout << "Sending a heartbeat to backend" << std::endl;
            std::vector<Byte> stlRestResponse = ::RestApiCall(m_strRestPortalAddress, m_dwRestPortalPort, strVerb, strApiUrl, strJsonBody, true);
            std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
            StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
            if (200 == oResponse.GetFloat64("Status"))
            {
                // Update the Eosb in case it changed
                m_strUserEosb = oResponse.GetString("Eosb");

                // Check if any Virtual Machine is waiting for the dataset, if yes new threads
                // are created to upload data into those Virutal Machines
                StructuredBuffer oVirtualMachinesWaiting = oResponse.GetStructuredBuffer("VirtualMachines");
                for (auto strVirtualMachineUuid : oVirtualMachinesWaiting.GetNamesOfElements())
                {
                    // There is a Virtual Machine waiting for data, a new thread is created
                    // to connect to the Virutal Machine and upload the data
                    StructuredBuffer oVirtualMachineInformation = oVirtualMachinesWaiting.GetStructuredBuffer(strVirtualMachineUuid.c_str());
                    if (m_stlSetOfVirtualMachineUploading.end() == m_stlSetOfVirtualMachineUploading.find(oVirtualMachineInformation.GetString("IPAddress")))
                    {
                        m_stlSetOfVirtualMachineUploading.insert(oVirtualMachineInformation.GetString("IPAddress"));
                        std::thread(&RemoteDataConnector::UploadDataSetToVirtualMachine, this, oVirtualMachineInformation.GetString("IPAddress"), oVirtualMachineInformation.GetString("DatasetGuid")).detach();
                    }
                }
            }
            else
            {
                _ThrowBaseException("Heartbeat failed with error. The response is %s", oResponse.ToString().c_str());
            }

            // Sleep for 30 seconds
            std::this_thread::sleep_for(std::chrono::seconds(30));

            // Wake up and check if all the dataset files still exist on the filesystem
            // If some dataset file has been removed update the backend about it
            std::vector<std::string> stlDatasetsRemoved;
            for (auto strFilesToCheck : m_stlMapOfFileToDatasetsUuids)
            {
                if (false == std::filesystem::exists(strFilesToCheck.first))
                {
                    m_oCollectionOfDatasets.RemoveElement(m_stlMapOfFileToDatasetsUuids.at(strFilesToCheck.second).c_str());
                    stlDatasetsRemoved.push_back(strFilesToCheck.first);
                    // Update this information on the database
                    bool fUpdateStatus = this->UpdateDatasets();
                    _ThrowBaseExceptionIf((false == fUpdateStatus), "Failed to update the Remote DataConnector database while removing dataset %s", strFilesToCheck.first.c_str());
                }
            }
            // When using a iterator for a map, erase while in the loop will cause
            // unexpected memory issues, so this has to be done seperately
            for (auto strDatasetRemoved : stlDatasetsRemoved)
            {
                m_stlMapOfFileToDatasetsUuids.erase(strDatasetRemoved);
            }
        } while(true);
    }
    catch (const BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function UserLogin
 * @brief Login to the SAIL Web Portal
 * @param[in] c_oStructuredBuffer
 *
 ********************************************************************************************/
bool __thiscall RemoteDataConnector::UserLogin(
    _in const std::string c_strRestPortalIpAddress,
    _in const Dword c_dwPort,
    _in const std::string & c_strEmail,
    _in const std::string & c_strPassword
)
{
    __DebugFunction();

    bool fLoginSuccess = false;
    // Will be using the same address when the regular pings will be send to
    // the Rest Portal later
    m_strRestPortalAddress = c_strRestPortalIpAddress;
    m_dwRestPortalPort = c_dwPort;

    // Make the API call and get REST response
    std::string strVerb = "POST";
    std::string strApiUrl = "/SAIL/AuthenticationManager/User/Login?Email="+ c_strEmail +"&Password="+ c_strPassword;
    std::string strJsonBody = "";
    std::vector<Byte> stlRestResponse = ::RestApiCall(c_strRestPortalIpAddress, c_dwPort, strVerb, strApiUrl, strJsonBody, true);
    std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
    StructuredBuffer oResponse = JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str());
    if (201 == oResponse.GetFloat64("Status"))
    {
        fLoginSuccess = true;
        m_strUserEosb = oResponse.GetString("Eosb");
    }

    // Make the API call to get the user information
    strApiUrl = "/SAIL/AuthenticationManager/GetBasicUserInformation?Eosb="+ m_strUserEosb;
    // Make the API call and get REST response
    stlRestResponse = ::RestApiCall(c_strRestPortalIpAddress, c_dwPort, "GET", strApiUrl, strJsonBody, true);
    strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
    oResponse = JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str());
    _ThrowBaseExceptionIf((200 != oResponse.GetFloat64("Status")), "Failed REST Response", nullptr);

    m_strUserUuid = oResponse.GetString("UserGuid");
    m_strUserEosb = oResponse.GetString("Eosb");
    m_strUserOrganizationUuid = oResponse.GetString("OrganizationGuid");

    return fLoginSuccess;
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function UploadDataSetToVirtualMachine
 * @brief Upload the dataset file to the Virtual Machine
 * @param[in] c_strVirtualMachineAddress The IP address of the Virtual Machine waiting for
 *      the dataset file
 * @param[in] c_strDatasetFileName Path of the dataset file
 *
 ********************************************************************************************/
void __thiscall RemoteDataConnector::UploadDataSetToVirtualMachine(
    _in const std::string c_strVirtualMachineAddress,
    _in const std::string c_strDatasetUuid
    ) throw()
{
    __DebugFunction();

    std::string strDatasetFile;

    std::cout << "Uploading " << c_strDatasetUuid << " to Virtual Machine " << c_strVirtualMachineAddress << std::endl;

    try
    {
        for (auto strFileName : m_stlMapOfFileToDatasetsUuids)
        {
            if (c_strDatasetUuid == strFileName.second)
            {
                strDatasetFile = strFileName.first;
            }
        }
        _ThrowBaseExceptionIf((0 == strDatasetFile.length()), "Invalid dataset request %s", c_strDatasetUuid.c_str());
        _ThrowBaseExceptionIf((false == std::filesystem::exists(strDatasetFile)), "Dataset file does not exist request %s", c_strDatasetUuid.c_str());

        std::vector<Byte> stlDatasetFiledata = ::ReadFileAsByteBuffer(strDatasetFile);

        StructuredBuffer oInitializationParameters;
        oInitializationParameters.PutString("SailWebApiPortalIpAddress", m_strRestPortalAddress);
        oInitializationParameters.PutString("Base64EncodedDataset", ::Base64Encode(stlDatasetFiledata.data(), stlDatasetFiledata.size()));
        oInitializationParameters.PutString("DataOwnerAccessToken", m_strUserEosb);
        oInitializationParameters.PutString("DataOwnerUserIdentifier", m_strUserUuid);
        oInitializationParameters.PutString("DataOwnerUserIdentifier", m_strUserUuid);
        oInitializationParameters.PutString("DataOwnerOrganizationIdentifier", m_strUserOrganizationUuid);

        // Establish a connection with the Virtual Machine
        // Wait for connetion to establish for 10 minutes with a new attempt every 10 seconds
        TlsNode * poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(c_strVirtualMachineAddress.c_str(), 6800, 10*60*1000, 10*1000);
        _ThrowIfNull(poTlsNode, "TlsConnectToNetworkSocketWithTimeout failed.", nullptr);
        StructuredBuffer oResponse(::PutTlsTransactionAndGetResponse(poTlsNode, oInitializationParameters, 10*1000));

        if ("Success" == oResponse.GetString("Status"))
        {
            std::cout << "Dataset upload success.\n";
        }
        else
        {
            _ThrowBaseException("Dataset Uplaod failed.", nullptr);
        }
    }
    catch (const BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function UpdateDatasets
 * @brief Update the datasets to the SAIL database
 * @return true or false status of update success
 ********************************************************************************************/
bool __thiscall RemoteDataConnector::UpdateDatasets(void)
{
    __DebugFunction();
    __DebugAssert(0 < m_strRestPortalAddress.length());

    bool fUpdateSuccess = false;

    StructuredBuffer oUpdateDataConnector;
    oUpdateDataConnector.PutString("RemoteDataConnectorGuid", m_oGuidDataConnector.ToString(eHyphensAndCurlyBraces));
    oUpdateDataConnector.PutStructuredBuffer("Datasets", m_oCollectionOfDatasets);
    oUpdateDataConnector.PutString("Version", "0.0.1");

    // On finding a dataset the new RemoteDataConnector is registered, but if it has been already
    // registered on the portal but a new dataset arrives, a call will be made to update the
    // dataset list on the remote database
    std::string strVerb = "";
    std::string strApiUrl = "";
    if (false == m_fIsDataConnectorRegistered)
    {
        // Register the DatasetConnector on finding the first valid dataset
        strVerb = "POST";
        strApiUrl = "/SAIL/RemoteDataConnectorManager/RegisterConnector?Eosb="+ m_strUserEosb;
    }
    else
    {
        // Register the DatasetConnector on finding the first valid dataset
        strVerb = "PUT";
        strApiUrl = "/SAIL/RemoteDataConnectorManager/UpdateConnector?Eosb="+ m_strUserEosb;
    }
    auto oJsonBody = JsonValue::ParseStructuredBufferToJson(oUpdateDataConnector);
    std::string strJsonBody = oJsonBody->ToString();
    oJsonBody->Release();
    std::vector<Byte> stlRestResponse = ::RestApiCall(m_strRestPortalAddress, m_dwRestPortalPort, strVerb, strApiUrl, strJsonBody, true);
    std::string strUnescapedResponse = ::UnEscapeJsonString((const char *) stlRestResponse.data());
    StructuredBuffer oResponse(JsonValue::ParseDataToStructuredBuffer(strUnescapedResponse.c_str()));
    if ((201 == oResponse.GetFloat64("Status")) || 200 == oResponse.GetFloat64("Status"))
    {
        m_strUserEosb = oResponse.GetString("Eosb");
        fUpdateSuccess = true;

        // Start the thread which will keep sending the heartbeat signals to the backend
        if (false == m_fIsDataConnectorRegistered)
        {
            m_fIsDataConnectorRegistered = true;
            // Notify the heartbeat thread that the registration is complete and it can
            // start sending the heartbeat to the
            m_stlNotifyRegistration.notify_all();
        }
    }

    return fUpdateSuccess;
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function VerifyDataset
 * @brief Verify if the datasets files contain correct data
 * @return StructuredBuffer containing dataset info on success, else empty structuredBuffer
 *
 ********************************************************************************************/
StructuredBuffer __thiscall RemoteDataConnector::VerifyDataset(
    _in const std::string & c_strDatasetFileName
    ) const throw()
{
    __DebugFunction();

    StructuredBuffer oDatasetInformation;

    try
    {
        // Read the dataset file
        std::vector<Byte> stlDataset = ::ReadFileAsByteBuffer(c_strDatasetFileName);

        // Set the stream to take input from the data vector;
        std::stringstream stlDatasetFile;
        stlDatasetFile.rdbuf()->pubsetbuf((char *)stlDataset.data(), stlDataset.size());

        // Read and verify the Header Marker
        uint64_t unHeader;
        stlDatasetFile.read((char *)&unHeader, sizeof(uint64_t));
        _ThrowBaseExceptionIf((0xDEADBEEFDEADBEEF != unHeader), "Expected header to be [0xDEADBEEFDEADBEEF] but found [%xul]", unHeader, nullptr);

        // Read the size of the Header StructuredBuffer
        uint32_t unHeaderSize;
        stlDatasetFile.read((char *)&unHeaderSize, sizeof(uint32_t));

        // Read the Structured Buffer
        std::vector<Byte> stlHeaderStructuredBuffer(unHeaderSize);
        stlDatasetFile.read((char *)stlHeaderStructuredBuffer.data(), unHeaderSize);
        StructuredBuffer oHeaderStructuredBuffer(stlHeaderStructuredBuffer);

        // Read the relevant data from the Header Strucuted Buffer
        uint64_t unMetaDataOffset = oHeaderStructuredBuffer.GetUnsignedInt64("MetaDataOffset");
        int32_t m_unMetaDataSizeInBytes = oHeaderStructuredBuffer.GetInt32("MetaDataSize");

        // Read the Dataset MetaData Structured Buffer
        stlDatasetFile.seekg(unMetaDataOffset);
        std::vector<Byte> stlMetaDataStructuredBuffer(m_unMetaDataSizeInBytes);
        stlDatasetFile.read((char *)stlMetaDataStructuredBuffer.data(), m_unMetaDataSizeInBytes);
        StructuredBuffer oDataSetMetaDataStructuredBuffer(stlMetaDataStructuredBuffer);

        std::string strDatasetUuid = oDataSetMetaDataStructuredBuffer.GetString("UUID");

        oDatasetInformation.PutString("DatasetUuid", strDatasetUuid);

        // TODO: Prawal verify the dataset file signature

        // TODO: Prawal Make a call to the rest portal and check if the dataset are registered.

    }
    catch (const BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
        oDatasetInformation.Clear();
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        oDatasetInformation.Clear();
    }

    return oDatasetInformation;
}

/********************************************************************************************
 *
 * @class RemoteDataConnector
 * @function UploadDataSetToVirtualMachine
 * @brief Upload the dataset file to the Virtual Machine
 * @param[in] c_strVirtualMachineAddress The IP address of the Virtual Machine waiting for
 *      the dataset file
 * @param[in] c_strDatasetFileName Path of the dataset file
 *
 ********************************************************************************************/
void __thiscall RemoteDataConnector::ManualUploadDataSetToVirtualMachine(
    _in const std::string c_strVirtualMachineAddress,
    _in const std::string c_strDatasetGuid,
    _in const std::string c_strFileName
    ) throw()
{
    __DebugFunction();

    std::cout << "Uploading " << c_strDatasetGuid << " to Virtual Machine " << c_strVirtualMachineAddress << std::endl;

    try
    {
        std::vector<Byte> stlDatasetFiledata = ::ReadFileAsByteBuffer(c_strFileName);

        StructuredBuffer oInitializationParameters;
        oInitializationParameters.PutString("SailWebApiPortalIpAddress", m_strRestPortalAddress);
        oInitializationParameters.PutString("Base64EncodedDataset", ::Base64Encode(stlDatasetFiledata.data(), stlDatasetFiledata.size()));
        oInitializationParameters.PutString("DataOwnerAccessToken", m_strUserEosb);
        oInitializationParameters.PutString("DataOwnerUserIdentifier", m_strUserUuid);
        oInitializationParameters.PutString("DataOwnerUserIdentifier", m_strUserUuid);
        oInitializationParameters.PutString("DataOwnerOrganizationIdentifier", m_strUserOrganizationUuid);

        // Establish a connection with the Virtual Machine
        // Wait for connetion to establish for 10 minutes with a new attempt every 10 seconds
        TlsNode * poTlsNode = ::TlsConnectToNetworkSocketWithTimeout(c_strVirtualMachineAddress.c_str(), 6800, 10*60*1000, 10*1000);
        _ThrowIfNull(poTlsNode, "TlsConnectToNetworkSocketWithTimeout failed.", nullptr);
        StructuredBuffer oResponse(::PutTlsTransactionAndGetResponse(poTlsNode, oInitializationParameters, 10*1000));

        if ("Success" == oResponse.GetString("Status"))
        {
            std::cout << "Dataset upload success.\n";
        }
        else
        {
            _ThrowBaseException("Dataset Uplaod failed.", nullptr);
        }
    }
    catch (const BaseException & oException)
    {
        ::RegisterException(oException, oException.GetFunctionName(), oException.GetLineNumber());
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
}
