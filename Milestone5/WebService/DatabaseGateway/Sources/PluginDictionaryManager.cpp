/*********************************************************************************************
 *
 * @file PluginDictionaryManager.cpp
 * @author Shabana Akhtar Baig
 * @date 29 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "PluginDictionaryManager.h"

#include <string.h>

/********************************************************************************************
 *
 * @class PluginDictionaryManager
 * @function PluginDictionaryManager
 * @brief Constructor
 *
 ********************************************************************************************/

PluginDictionaryManager::PluginDictionaryManager(void)
{
    __DebugFunction();
}

/********************************************************************************************
 *
 * @class PluginDictionaryManager
 * @function ~PluginDictionaryManager
 * @brief Destructor
 *
 ********************************************************************************************/

PluginDictionaryManager::~PluginDictionaryManager(void) throw()
{
    __DebugFunction();

    m_stlPluginStructuredBuffer.clear();
    m_stlSubmitRequestFunctions.clear();
    m_stlGetResponseFunctions.clear();
}

/********************************************************************************************
 *
 * @class PluginDictionaryManager
 * @function RegisterPlugin
 * @brief Stores a plugin's dictionary associating it to the plugin's 64 bit hashed name
 * @param[in] c_szPluginName Name of the plugin
 * @param[in] c_szIdentifier Identifier of the plugin
 * @param[in] qwPluginVersion Version of the plugin
 * @param[in] fnSubmitRequest Pointer to the SubmitRequest Callback
 * @param[in] fnGetResponse Pointer to the GetResponse Callback
 * @param[in] c_pbSerializedDictionary Pointer to the first byte of the plugin's dictionary
 * @param[in] unSerializedDictionarySizeInBytes Size of the plugin's dictionary
 * @return A boolean representing status of registering the plugin's dictionary
 *
 ********************************************************************************************/

bool __thiscall PluginDictionaryManager::RegisterPlugin(
    _in const char * c_szPluginName,
    _in const char * c_szIdentifier,
    _in Qword qwPluginVersion,
    _in SubmitRequestFn fnSubmitRequest,
    _in GetResponseFn fnGetResponse,
    _in const Byte * c_pbSerializedDictionary,
    _in unsigned int unSerializedDictionarySizeInBytes
    )
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        // Get a Guid object for the plugin's uuid
        Guid oGuid(c_szIdentifier);

        std::vector<Byte> stlSerializedDictionary;
        stlSerializedDictionary.resize(unSerializedDictionarySizeInBytes);
        ::memcpy((void *) stlSerializedDictionary.data(), (const void *) c_pbSerializedDictionary, unSerializedDictionarySizeInBytes);

        StructuredBuffer oPluginStructuredBuffer;
        oPluginStructuredBuffer.PutString("PluginName", c_szPluginName);
        oPluginStructuredBuffer.PutGuid("PluginUniqueIdentifier", oGuid);
        oPluginStructuredBuffer.PutQword("PluginVersion", qwPluginVersion);
        oPluginStructuredBuffer.PutBuffer("PluginDictionarySerializedBuffer", stlSerializedDictionary);

        if (0 >= oPluginStructuredBuffer.GetSerializedBufferRawDataSizeInBytes())
        {
          __DebugError("%s", "Error: Invalid plugin dictionary.");
        }

        // Save the plugin structured buffer
        m_stlPluginStructuredBuffer.push_back(oPluginStructuredBuffer);

        // Store callback function associated with the 64bithash of the plugin's name
        Qword qwPluginName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szPluginName, false);
        m_stlSubmitRequestFunctions[qwPluginName64BitHash] = fnSubmitRequest;
        m_stlGetResponseFunctions[qwPluginName64BitHash] = fnGetResponse;

        fSuccess = true;
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************
 *
 * @class PluginDictionaryManager
 * @function GetSubmitRequestFunction
 * @brief Fetched a pointer to the SubmitRequest Callback associated with c_szPluginName
 * @param[in] c_szPluginName Name of the plugin
 * @throw BaseException if Callback function fro the plugin not found
 * @return Pointer to the SubmitRequest Callback associated with c_szPluginName
 *
 ********************************************************************************************/

SubmitRequestFn __thiscall PluginDictionaryManager::GetSubmitRequestFunction(
    _in const char * c_szPluginName
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szPluginName)

    Qword qwPluginName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szPluginName, false);

    SubmitRequestFn fnSubmitRequestFunction;
    if (m_stlSubmitRequestFunctions.end() != m_stlSubmitRequestFunctions.find(qwPluginName64BitHash))
    {
        fnSubmitRequestFunction = m_stlSubmitRequestFunctions[qwPluginName64BitHash];
    }
    else
    {
      _ThrowBaseException("Error: Callback function not found.", nullptr);
    }

    return fnSubmitRequestFunction;
}

/********************************************************************************************
 *
 * @class PluginDictionaryManager
 * @function GetGetResponseFunction
 * @brief Fetched a pointer to the GetResponse Callback associated with c_szPluginName
 * @param[in] c_szPluginName Name of the plugin
 * @throw BaseException if Callback function fro the plugin not found
 * @return Pointer to the GetResponse Callback associated with c_szPluginName
 *
 ********************************************************************************************/

GetResponseFn __thiscall PluginDictionaryManager::GetGetResponseFunction(
    _in const char * c_szPluginName
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szPluginName)

    Qword qwPluginName64BitHash = ::Get64BitHashOfNullTerminatedString(c_szPluginName, false);

    GetResponseFn fnGetResponseFunction;
    if (m_stlGetResponseFunctions.end() != m_stlGetResponseFunctions.find(qwPluginName64BitHash))
    {
        fnGetResponseFunction = m_stlGetResponseFunctions[qwPluginName64BitHash];
    }
    else
    {
      _ThrowBaseException("Error: Callback function not found.", nullptr);
    }

    return fnGetResponseFunction;
}
