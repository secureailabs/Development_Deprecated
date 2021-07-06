/*********************************************************************************************
 *
 * @file PluginDictionaryManager.h
 * @author Shabana Akhtar Baig
 * @date 03 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "64BitHashes.h"
#include "CoreTypes.h"
#include "DebugLibrary.h"
#include "Object.h"
#include "PluginDictionary.h"
#include "RestFrameworkSharedFunctions.h"
#include "StructuredBuffer.h"

#include <iostream>
#include <functional>
#include <map>
#include <vector>

/********************************************************************************************/

class PluginDictionaryManager : public Object
{
    public:

        // Constructor and Destructor
        PluginDictionaryManager(void);
        virtual ~PluginDictionaryManager(void) throw();

        // Stores a plugins information in a vector
        bool __thiscall RegisterPlugin(
            _in const char * c_szPluginName,
            _in const char * c_szIdentifier,
            _in Qword qwPluginVersion,
            _in SubmitRequestFn fnSubmitRequest,
            _in GetResponseFn fnGetResponse,
            _in const Byte * c_pbSerializedDictionary,
            _in unsigned int unSerializedDictionarySizeInBytes
            );

        // Access Methods
        std::string __thiscall GetPluginName(
            _in unsigned int unIndex
            );
        std::string __thiscall GetPluginUuid(
            _in unsigned int unIndex
            );
        Qword __thiscall GetPluginVersion(
            _in unsigned int unIndex
            );
        std::vector<Byte> __thiscall GetPluginSerializedDictionary(
            _in unsigned int unIndex
            );
        unsigned int __thiscall GetPluginStructuredBufferSize(void);

        // Methods to access a plugin's CallBack functions
        SubmitRequestFn __thiscall GetSubmitRequestFunction(
            _in const char * c_szPluginName
            );
        GetResponseFn __thiscall GetGetResponseFunction(
            _in const char * c_szPluginName
            );

    private:

        // Private data members

        // Stores plugin's information including its dictionary
        std::vector<std::vector<Byte>> m_stlPluginStructuredBuffer;
        // Map of CallBackFunctions
        std::map<Qword, SubmitRequestFn> m_stlSubmitRequestFunctions;
        std::map<Qword, GetResponseFn> m_stlGetResponseFunctions;
};

/********************************************************************************************/
