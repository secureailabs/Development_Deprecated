/*********************************************************************************************
 *
 * @file PluginDictionary.cpp
 * @author Shabana Akhtar Baig
 * @date 20 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "PluginDictionary.h"

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function PluginDictionary
 * @brief Constructor
 *
 ********************************************************************************************/

PluginDictionary::PluginDictionary(void)
{
    __DebugFunction();

    m_poDictionaryStructuredBuffer = new StructuredBuffer();
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function PluginDictionary
 * @brief Constructor
 * @param[in] c_stlSerializedDictionary Serialized dictionary StructuredBuffer
 *
 ********************************************************************************************/

PluginDictionary::PluginDictionary(
    _in const std::vector<Byte> & c_stlSerializedDictionary
    )
{
    __DebugFunction();

    m_poDictionaryStructuredBuffer = new StructuredBuffer(c_stlSerializedDictionary);
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function ~PluginDictionary
 * @brief Destructor
 *
 ********************************************************************************************/

PluginDictionary::~PluginDictionary(void) throw()
{
    __DebugFunction();

    delete m_poDictionaryStructuredBuffer;
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function AddDictionaryEntry
 * @brief Insert a dictionary entry in the plugin dictionary
 * @param[in] c_szVerb Verb of a plugin
 * @param[in] c_szResource Resource of a plugin
 *
 ********************************************************************************************/

void __thiscall PluginDictionary::AddDictionaryEntry(
    _in const char * c_szVerb,
    _in const char * c_szResource,
    _in Qword qwRequiredUnixConnections
    )
{
    __DebugFunction();

    StructuredBuffer oDictionaryEntry;
    oDictionaryEntry.PutString("Verb", c_szVerb);
    oDictionaryEntry.PutQword("NumberOfUnixConnections", qwRequiredUnixConnections);

    m_poDictionaryStructuredBuffer->PutStructuredBuffer(c_szResource, oDictionaryEntry);
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function AddDictionaryEntry
 * @brief Insert a dictionary entry in the plugin dictionary
 * @param[in] c_szVerb Verb of a plugin
 * @param[in] c_szResource Resource of a plugin
 * @param[in] oParameters StructuredBuffer containing the required and optional parameters for the
 *            c_szResource
 *
 ********************************************************************************************/

void __thiscall PluginDictionary::AddDictionaryEntry(
    _in const char * c_szVerb,
    _in const char * c_szResource,
    _in const StructuredBuffer & oParameters,
    _in Qword qwRequiredUnixConnections
    )
{
    __DebugFunction();

    StructuredBuffer oDictionaryEntry;
    oDictionaryEntry.PutString("Verb", c_szVerb);
    oDictionaryEntry.PutQword("NumberOfUnixConnections", qwRequiredUnixConnections);
    oDictionaryEntry.PutStructuredBuffer("Parameters", oParameters);

    m_poDictionaryStructuredBuffer->PutStructuredBuffer(c_szResource, oDictionaryEntry);
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function GetSerializedDictionary
 * @brief Get the serialized buffer representation of the plugin's dictionary
 * @return A vector containing serialized buffer, representing plugin's dictionary
 *
 ********************************************************************************************/

std::vector<Byte> __thiscall PluginDictionary::GetSerializedDictionary(void) const throw()
{
    __DebugFunction();

    return m_poDictionaryStructuredBuffer->GetSerializedBuffer();
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function GetSerializedDictionaryRawDataPtr
 * @brief Fetch a const Byte * to the serialized buffer of the PluginDictionary
 * @return const Byte * to the serialized buffer of the PluginDictionary
 *
 ********************************************************************************************/

const Byte * __thiscall PluginDictionary::GetSerializedDictionaryRawDataPtr(void) const throw()
{
  __DebugFunction();

  return (const Byte *) m_poDictionaryStructuredBuffer->GetSerializedBuffer().data();
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function GetSerializedDictionaryRawDataSizeInBytes
 * @brief Get the serialized dictionary's size in bytes
 * @return Serialized dictionary's size in bytes
 *
 ********************************************************************************************/

unsigned int __thiscall PluginDictionary::GetSerializedDictionaryRawDataSizeInBytes(void) const throw()
{
    __DebugFunction();

    std::vector<Byte> stlDictionaryBuffer = m_poDictionaryStructuredBuffer->GetSerializedBuffer();

    return (unsigned int) stlDictionaryBuffer.size();
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function GetPluginDictionaryStructuredBuffer
 * @brief Get the PluginDictionary
 * @return Pointer to StructuredBuffer representing a plugin's dictionary
 *
 ********************************************************************************************/

StructuredBuffer * __thiscall PluginDictionary::GetPluginDictionaryStructuredBuffer(void) const throw()
{
    __DebugFunction();

    return m_poDictionaryStructuredBuffer;
}
