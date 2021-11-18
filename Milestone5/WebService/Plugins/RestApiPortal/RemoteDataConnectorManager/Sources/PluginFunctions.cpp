/*********************************************************************************************
 *
 * @file PluginFunctions.cpp
 * @author Shabana Akhtar Baig
 * @date 30 Jul 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "RemoteDataConnectorManager.h"

/********************************************************************************************
 *
 * @function SubmitRequest
 * @brief Call RemoteDataConnectorManager's Submit request and return a unique identifier
 *        along with the size of response in bytes.
 * @param[in] c_pbSerializedRequest pointer to the serialized request
 * @param[in] unSerializedRequestSizeInBytes size of the serialized request
 * @param[out] punSerializedResponseSizeInBytes size of the serialized response
 * @return A unique transaction identifier and size in bytes of the response
 *
 ********************************************************************************************/

extern "C" uint64_t SubmitRequest(
    _in const Byte * c_pbSerializedRequest,
    _in unsigned int unSerializedRequestSizeInBytes,
    _out unsigned int * punSerializedResponseSizeInBytes
    )
{
    __DebugFunction();

    uint64_t un64Identifier = 0xFFFFFFFFFFFFFFFF;

    try
    {
        RemoteDataConnectorManager * poRemoteDataConnectorManager = ::GetRemoteDataConnectorManager();
        __DebugAssert(nullptr != poRemoteDataConnectorManager);
        if ((nullptr != c_pbSerializedRequest)&&(0 < unSerializedRequestSizeInBytes)&&(nullptr != punSerializedResponseSizeInBytes))
        {
            StructuredBuffer oRequest(c_pbSerializedRequest, unSerializedRequestSizeInBytes);
            un64Identifier = poRemoteDataConnectorManager->SubmitRequest(oRequest, punSerializedResponseSizeInBytes);
        }
        else if (nullptr != punSerializedResponseSizeInBytes)
        {
            *punSerializedResponseSizeInBytes = 0;
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    return un64Identifier;
}

/********************************************************************************************
 *
 * @function GetResponse
 * @brief Call RemoteDataConnectorManager's GetResponse and return response associated
 *        with the transaction identifier
 * @param[in] un64Identifier transaction identifier returned by ::SubmitRequest
 * @param[out] pbSerializedResponseBuffer Byte * to the first byte of buffer allocated for the
 *             response
 * @param[in] unSerializedResponseBufferSizeInBytes size of response
 * @return Return status of the call and copied response
 *
 ********************************************************************************************/

extern "C" bool GetResponse(
    _in uint64_t un64Identifier,
    _out Byte * pbSerializedResponseBuffer,
    _in unsigned int unSerializedResponseBufferSizeInBytes
    )
{
    __DebugFunction();
    // TODO: Return relevant Http error code representing the cause of a request processing failure

    bool fSuccess = false;

    try
    {
        RemoteDataConnectorManager * poRemoteDataConnectorManager = ::GetRemoteDataConnectorManager();
        __DebugAssert(nullptr != poRemoteDataConnectorManager);
        if ((nullptr != pbSerializedResponseBuffer)&&(0 < unSerializedResponseBufferSizeInBytes))
        {
            fSuccess = poRemoteDataConnectorManager->GetResponse(un64Identifier, pbSerializedResponseBuffer, unSerializedResponseBufferSizeInBytes);
        }
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
 * @function InitializePlugin
 * @brief RestFramework loads all plugins and calls this function and passes RegisterPlugin as a Callback
 *        function. This function creates  singleton object of RemoteDataConnectorManager class and
 *        calls RemoteDataConnectorManager's InitializePlugin, which initializes the plugin's
 *        data members including its dictionary. This function then calls RegisterPluginFn which is used to
 *        register RemoteDataConnectorManager's dictionary with the RestFramework
 * @param[in] fnRegisterPluginFn pointer to flat function which calls RestFramework::RegisterPlugin
 * @return A boolean representing status of plugin's registration with the Rest portal
 *
********************************************************************************************/

extern "C" bool __stdcall InitializePlugin(
    _in RegisterPluginFn fnRegisterPluginFn,
    _in Byte * pbSerializedInitParameters,
    _in uint64_t unSerializedInitParametersBufferSizeInBytes
    )
{
    __DebugFunction();
    _ThrowBaseExceptionIf((nullptr == fnRegisterPluginFn), "Invalid registration function", nullptr);

    bool fSuccess = false;

    try
    {
        StructuredBuffer oInitializationVectorBuffer(pbSerializedInitParameters, unSerializedInitParametersBufferSizeInBytes);
        RemoteDataConnectorManager * poRemoteDataConnectorManager = ::GetRemoteDataConnectorManager();
        __DebugAssert(nullptr != poRemoteDataConnectorManager);
        poRemoteDataConnectorManager->InitializePlugin(oInitializationVectorBuffer);
        std::vector<Byte> stlDictionary = poRemoteDataConnectorManager->GetDictionarySerializedBuffer();
        fSuccess = fnRegisterPluginFn(poRemoteDataConnectorManager->GetName(), poRemoteDataConnectorManager->GetUuid(), poRemoteDataConnectorManager->GetVersion(), SubmitRequest, GetResponse, stlDictionary.data(), stlDictionary.size());
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
 * @function ShutdownPlugin
 * @brief Call RemoteDataConnectorManager to release the object resources and
 *        set global static pointer to NULL
 *
 ********************************************************************************************/

extern "C" void __stdcall ShutdownPlugin(void)
{
    __DebugFunction();

    try
    {
        ::ShutdownRemoteDataConnectorManager();
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __FILE__, __LINE__);;
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }
}
