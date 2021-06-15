/*********************************************************************************************
 *
 * @file PluginDictionary.h
 * @author Shabana Akhtar Baig
 * @date 20 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 * A PluginDictionary object is passed to the RestFramework by a plugin during registration.
 * The object will provide information about the plugin's verbs, resources, and parameters.
 *
 ********************************************************************************************/

#pragma once

#include "DebugLibrary.h"
#include "Object.h"
#include "StructuredBuffer.h"

#include <iostream>
#include <vector>

/********************************************************************************************/

class PluginDictionary : public Object
{
    public:

        // Constructor and Destructor

        PluginDictionary(void);
        PluginDictionary(
            _in const std::vector<Byte> & c_stlSerializedDictionary
            );
        virtual ~PluginDictionary(void) throw();

        // Methods to add a dictionary entry
        void __thiscall AddDictionaryEntry(
            _in const char * c_szVerb,
            _in const char * c_szResource,
            _in Qword qwRequiredUnixConnections
            );
        void __thiscall AddDictionaryEntry(
            _in const char * c_szVerb,
            _in const char * c_szResource,
            _in const StructuredBuffer & oParameters,
            _in Qword qwRequiredUnixConnections
            );

        // Methods to access plugin's dictionary
        std::vector<Byte> __thiscall GetSerializedDictionary(void) const throw();
        const Byte * __thiscall GetSerializedDictionaryRawDataPtr(void) const throw();
        unsigned int __thiscall GetSerializedDictionaryRawDataSizeInBytes(void) const throw();
        StructuredBuffer * __thiscall GetPluginDictionaryStructuredBuffer(void) const throw();

    private:

        //Private data members

        StructuredBuffer * m_poDictionaryStructuredBuffer;
};
