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
    _in const char * c_szResource
    )
{
    __DebugFunction();

    StructuredBuffer oDictionaryEntry;
    oDictionaryEntry.PutString("Verb", c_szVerb);

    m_oDictionaryStructuredBuffer.PutStructuredBuffer(c_szResource, oDictionaryEntry);
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function AddDictionaryEntry
 * @brief Insert a dictionary entry in the plugin dictionary
 * @param[in] c_szVerb Verb of a plugin
 * @param[in] c_szResource Resource of a plugin
 * @param[in] oRequiredParameters StructuredBuffer containing the required parameters for the
 *            c_szResource 
 *
 ********************************************************************************************/

void __thiscall PluginDictionary::AddDictionaryEntry(
    _in const char * c_szVerb,
    _in const char * c_szResource,
    _in const StructuredBuffer & oRequiredParameters
    )
{
    __DebugFunction();

    StructuredBuffer oDictionaryEntry;
    oDictionaryEntry.PutString("Verb", c_szVerb);
    oDictionaryEntry.PutStructuredBuffer("RequiredParameters", oRequiredParameters);

    m_oDictionaryStructuredBuffer.PutStructuredBuffer(c_szResource, oDictionaryEntry);
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function AddDictionaryEntry
 * @brief Insert a dictionary entry in the plugin dictionary
 * @param[in] c_szVerb Verb of a plugin
 * @param[in] c_szResource Resource of a plugin
 * @param[in] oRequiredParameters StructuredBuffer containing the required parameters for the
 *            c_szResource 
 * @params[in] oOptionalParameters StructuredBuffer containing the optional parameters for the
 *            c_szResource 
 *
 ********************************************************************************************/

void __thiscall PluginDictionary::AddDictionaryEntry(
    _in const char * c_szVerb,
    _in const char * c_szResource,
    _in const StructuredBuffer & oRequiredParameters,
    _in const StructuredBuffer & oOptionalParameters
    )
{
    __DebugFunction();

    StructuredBuffer oDictionaryEntry;
    oDictionaryEntry.PutString("Verb", c_szVerb);
    oDictionaryEntry.PutStructuredBuffer("RequiredParameters", oRequiredParameters);
    oDictionaryEntry.PutStructuredBuffer("OptionaleRequirements", oOptionalParameters);

    m_oDictionaryStructuredBuffer.PutStructuredBuffer(c_szResource, oDictionaryEntry);
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

    return m_oDictionaryStructuredBuffer.GetSerializedBuffer();
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

  return (const Byte *) m_oDictionaryStructuredBuffer.GetSerializedBuffer().data();
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

    std::vector<Byte> stlDictionaryBuffer = m_oDictionaryStructuredBuffer.GetSerializedBuffer();

    return (unsigned int) stlDictionaryBuffer.size();
}

/********************************************************************************************
 *
 * @class PluginDictionary
 * @function GetPluginDictionaryStructuredBuffer
 * @brief Get the PluginDictionary
 * @return StructuredBuffer representing a plugin's dictionary
 *
 ********************************************************************************************/

StructuredBuffer __thiscall PluginDictionary::GetPluginDictionaryStructuredBuffer(void) const throw()
{
    __DebugFunction();

    return m_oDictionaryStructuredBuffer;
}
