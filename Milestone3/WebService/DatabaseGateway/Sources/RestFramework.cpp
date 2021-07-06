/*********************************************************************************************
 *
 * @file RestFramework.cpp
 * @author Shabana Akhtar Baig
 * @date 20 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "RestFramework.h"
#include "ExceptionRegister.h"

/********************************************************************************************/

static RestFramework * gs_poRestFramework = nullptr;

/********************************************************************************************
 *
 * @function RegisterPlugin
 * @brief Calls RestFramework::RegisterPlugin() to register a plugin's dictionary with the framework
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

extern "C" bool __thiscall RegisterPlugin(
    _in const char * c_szPluginName,
    _in const char * c_szIdentifier,
    _in Qword qwPluginVersion,
    _in SubmitRequestFn fnSubmitRequest,
    _in GetResponseFn fnGetResponse,
    _in const Byte * c_pbSerializedDictionary,
    _in unsigned int unSerializedDictionarySizeInBytes
    )
{
    bool fReturnValue = false;

    if (nullptr != gs_poRestFramework)
    {
        fReturnValue = gs_poRestFramework->RegisterPlugin(c_szPluginName, c_szIdentifier, qwPluginVersion, fnSubmitRequest, fnGetResponse, c_pbSerializedDictionary, unSerializedDictionarySizeInBytes);
    }

    return fReturnValue;
};

/********************************************************************************************
 *
 * @class RestFramework
 * @function RestFramework
 * @brief Constructor
 * @param[in] wPortNumber Listening port
 * @param[in] c_strPluginFolderPath Path to shared libraries
 *
 ********************************************************************************************/

RestFramework::RestFramework(
    _in const Word wPortNumber,
    // _in CryptographicTrustStore & oCryptographicTrustStore,
    _in const std::string & c_strPluginFolderPath
    )
{
    __DebugFunction();
    __DebugAssert(0 < c_strPluginFolderPath.length());

    gs_poRestFramework = this;
    if (nullptr == gs_poRestFramework)
    {
      std::cout << "Null pointer" << std::endl;
    }

    m_poDictionaryManager = new PluginDictionaryManager();
    this->LoadPlugins(c_strPluginFolderPath);
    m_poTlsServer = new TlsServer(wPortNumber);
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function RestFramework
 * @brief Constructor
 * @param[in] c_szUnixSocketAddress Name of the Unix socket
 * @param[in] c_strPluginFolderPath Path to shared libraries
 *
 ********************************************************************************************/

RestFramework::RestFramework(
    _in const char* c_szUnixSocketAddress,
    // _in CryptographicTrustStore & oCryptographicTrustStore,
    _in const std::string & c_strPluginFolderPath
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != c_szUnixSocketAddress);
    __DebugAssert(0 < c_strPluginFolderPath.length());

    gs_poRestFramework = this;

    m_poDictionaryManager = new PluginDictionaryManager();
    this->LoadPlugins(c_strPluginFolderPath);
    m_poTlsServer = new TlsServer(c_szUnixSocketAddress);
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function RestFramework
 * @brief Copy Constructor
 * @param[in] c_oRestFramework Another instance of the class
 * @note
 *      This constructor triggers an assertion failure if called.
 *
 ********************************************************************************************/

RestFramework::RestFramework(
    _in const RestFramework & c_oRestFramework
    )
{
    __DebugFunction();
    __DebugAssert(false);
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function ~RestFramework
 * @brief Destructor
 *
 ********************************************************************************************/

RestFramework::~RestFramework(void)
{
    __DebugFunction();

    m_poDictionaryManager->Release();
    m_poTlsServer->Release();
    m_stlPluginHandles.clear();
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function RegisterPlugin
 * @brief Calls PluginDictionaryManager::RegisterPlugin() to register a plugin's dictionary with the framework
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

bool __thiscall RestFramework::RegisterPlugin(
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
    __DebugAssert(nullptr != c_szPluginName);
    __DebugAssert(nullptr != c_szIdentifier);
    __DebugAssert(nullptr != c_pbSerializedDictionary);
    __DebugAssert(0 < unSerializedDictionarySizeInBytes);

    bool fSuccess = false;

    fSuccess = m_poDictionaryManager->RegisterPlugin(c_szPluginName, c_szIdentifier, qwPluginVersion, fnSubmitRequest, fnGetResponse, c_pbSerializedDictionary, unSerializedDictionarySizeInBytes);

    return fSuccess;
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function RunServer
 * @brief Wait for connections and process incoming connections if resources are available
 *
 ********************************************************************************************/

void __thiscall RestFramework::RunServer(void)
{
    __DebugFunction();

    RestFrameworkRuntimeData * poRestFrameworkRuntimeData = new RestFrameworkRuntimeData(m_poDictionaryManager);
    unsigned int unNumberOfResources = std::thread::hardware_concurrency();
    while (false == poRestFrameworkRuntimeData->IsTerminationSignalEncountered())
    {
        try
        {
            if (true == m_poTlsServer->WaitForConnection(100))  // check if a connection and the resources are available
            {
                // Make sure never to create more threads than the number of available logical cores
                if (unNumberOfResources > poRestFrameworkRuntimeData->GetNumberOfActiveConnections())
                {
                    poRestFrameworkRuntimeData->HandleConnection(m_poTlsServer->Accept());
                }
                else
                {
                    // Put this thread to sleep since we have maxed out our threading resources
                    // and need to wait for a running thread to exit
                    std::cout << "Resources maxed out. Going to sleep\n";
                    ::sleep(10);
                    std::cout << "Waking up. Checking resources...\n";
                }
            }
        }
        catch (BaseException oException)
        {
            ::RegisterException(oException, __func__, __LINE__);
        }
        catch (...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
        }
    }

    poRestFrameworkRuntimeData->Release();
    // Shutdown plugins and Close all plugin handles
    for (void * pPluginHandle : m_stlPluginHandles)
    {
        ShutdownPluginFn fnShutdownPlugin = (ShutdownPluginFn) ::dlsym(pPluginHandle, "ShutdownPlugin");
        fnShutdownPlugin();
        ::dlclose(pPluginHandle);
    }
}

/********************************************************************************************
 *
 * @class RestFramework
 * @function LoadPlugins
 * @brief Load .so files from the given path anc call their InitializePlugin function
 * @param[in] c_strPluginFolderPath Path to shared libraries
 * @note
 *      A pointer to C function ::RegisterPlugin is passed to InitializePlugin as a Callback
 ********************************************************************************************/

void __thiscall RestFramework::LoadPlugins(
    _in const std::string & c_strPluginFolderPath
    )
{
    __DebugFunction();

    void * pPluginHandle;
    char * szErrorMessage = nullptr;

    try 
    {
        for (const auto & itrFileEntry : std::experimental::filesystem::directory_iterator(c_strPluginFolderPath))
        {
            if (".so" == itrFileEntry.path().extension())
            {
                pPluginHandle = ::dlopen(itrFileEntry.path().string().c_str(), RTLD_NOW);

                if (!pPluginHandle)
                {
                    __DebugError("%s", "Error: Library file could not be open.");
                }
                else
                {
                    InitializePluginFn fnInitializePlugin = (InitializePluginFn) ::dlsym(pPluginHandle, "InitializePlugin");
                    _ThrowBaseExceptionIf((nullptr != (szErrorMessage = dlerror())), szErrorMessage, nullptr);
                    fnInitializePlugin(::RegisterPlugin);
                    m_stlPluginHandles.push_back(pPluginHandle);
                }
            }
        }
    }
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
        // Close all plugin handles
        for (void * pPluginHandle : m_stlPluginHandles)
        {
            ::dlclose(pPluginHandle);
        }
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        // Close all plugin handles
        for (void * pPluginHandle : m_stlPluginHandles)
        {
            ::dlclose(pPluginHandle);
        }
    }
}
