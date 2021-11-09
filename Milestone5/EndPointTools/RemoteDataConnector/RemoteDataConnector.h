/*********************************************************************************************
 *
 * @file RemoteDataConnector.h
 * @author Prawal Gangwar
 * @date 31 July 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the RemoteDataConnector class that is used to handle datasets
 *      in DataOwner Environment
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"
#include "Object.h"
#include "StructuredBuffer.h"

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <condition_variable>

/********************************************************************************************/

const std::string gc_strDataConnectorDefaultFolder = "Datasets";

class RemoteDataConnector : public Object
{
    public:

        RemoteDataConnector(
            _in const RemoteDataConnector & c_oRemoteDataConnector
            ) = delete;
        void operator=(
            _in RemoteDataConnector const & c_oRemoteDataConnector
            ) = delete;
        ~RemoteDataConnector(void);
        static RemoteDataConnector & Get(void);

        void __thiscall SetDatasetFolderAndRun(
            _in const std::string & c_strFolderName
            );
        bool __thiscall UserLogin(
            _in const std::string c_strRestPortalApi,
            _in const Dword c_dwPort,
            _in const std::string & c_strEmail,
            _in const std::string & c_strPassword
            );
        void __thiscall ManualUploadDataSetToVirtualMachine(
            _in const std::string c_strVirtualMachineAddress,
            _in const std::string c_strDatasetGuid,
            _in const std::string c_strDatasetFileName
            ) throw();

    private:

        // Private member methods
        RemoteDataConnector(void);
        void __thiscall NewDatasetFoundCallback(
            _in const std::string & c_strDatasetName
            ) throw();
        bool __thiscall UpdateDatasets(void);
        void __thiscall UploadDataSetToVirtualMachine(
            _in const std::string c_strVirtualMachineAddress,
            _in const std::string c_strDatasetGuid
            ) throw();
        StructuredBuffer __thiscall VerifyDataset(
            _in const std::string & c_strDatasetFileName
            ) const throw();
        void __thiscall SendDataConnectorHeartbeat(void) throw();

        // Private data members
        Guid m_oGuidDataConnector;
        bool m_fIsDataConnectorRegistered = false;
        static RemoteDataConnector m_oRemoteDataConnector;
        std::condition_variable m_stlNotifyRegistration;
        std::string m_strUserEosb;
        std::string m_strUserUuid;
        std::string m_strUserOrganizationUuid;
        std::string m_strRestPortalAddress;
        Dword m_dwRestPortalPort;
        std::mutex m_stlMutexRestConnection;
        std::unordered_set<std::string> m_stlSetOfVirtualMachineUploading;
        std::unordered_map<std::string, std::string> m_stlMapOfFileToDatasetsUuids;
        StructuredBuffer m_oCollectionOfDatasets;
};

/********************************************************************************************/
