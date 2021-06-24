/*********************************************************************************************
 *
 * @file RestFrameworkRuntimeData.cpp
 * @author Shabana Akhtar Baig
 * @date 29 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "RestFrameworkRuntimeData.h"
#include "ExceptionRegister.h"

/********************************************************************************************
 *
 * @struct ThreadParameters
 * @brief Struct used to pass in parameters to StartThread()
 *
 ********************************************************************************************/

typedef struct
{
    RestFrameworkRuntimeData * m_poRestFrameworkRuntimeData;        /* Pointer to RestFrameworkRunTimeData object */
    TlsNode * m_poTlsNode;                                          /* Pointer to TlsNode */
}
ThreadParameters;

/********************************************************************************************
 *
 * @function StartThread
 * @brief Starts up a connection thread
 * @param[in] poVoidThreadParameters Instance of ThreadParamters
 * @return A null pointer
 *
 ********************************************************************************************/

static void * __stdcall StartThread(
    _in void * poVoidThreadParameters
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poVoidThreadParameters);

    ThreadParameters * poThreadParameters = ((ThreadParameters *) poVoidThreadParameters);
    TlsNode * poTlsNode = poThreadParameters->m_poTlsNode;
    RestFrameworkRuntimeData * poRestFrameworkRuntimeData = poThreadParameters->m_poRestFrameworkRuntimeData;
    poRestFrameworkRuntimeData->RunThread((TlsNode *) poTlsNode);
    // Release the Tls Node
    poTlsNode->Release();
    // Deallocate parameters
    poRestFrameworkRuntimeData->m_oSmartMemoryAllocator.Deallocate(poVoidThreadParameters);
    // Delete the connection
    poRestFrameworkRuntimeData->DeleteConnection();

    ::pthread_exit(nullptr);
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function RestFrameworkRuntimeData
 * @brief Constructor
 * @param[in] poDictionaryManager Pointer to instance of a plugin dictionaries manager
 *
 ********************************************************************************************/

RestFrameworkRuntimeData::RestFrameworkRuntimeData(
    _in PluginDictionaryManager * poDictionaryManager
    )
{
    __DebugFunction();

    m_sMutex = PTHREAD_MUTEX_INITIALIZER;
    m_poDictionaryManager = poDictionaryManager;
    m_fTerminateSignalEncountered = false;
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function ~RestFrameworkRuntimeData
 * @brief Destructor
 *
 ********************************************************************************************/

RestFrameworkRuntimeData::~RestFrameworkRuntimeData(void) throw()
{
    __DebugFunction();

    m_stlConnectionThreads.clear();
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function HandleConnection
 * @brief Create a new thread for the incoming connection and initialize required parameters to start up a thread.
 *        Call RestFrameworkRuntimeData::AddConnection to store thread id.
 * @param[in] poTlsNode TlsNode object pointer
 * @throw BaseException Error creating a thread
 *
 ********************************************************************************************/

void __thiscall RestFrameworkRuntimeData::HandleConnection(
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();
    __DebugAssert(nullptr != poTlsNode);

    ThreadParameters * poThreadParameters = ((ThreadParameters *) m_oSmartMemoryAllocator.Allocate(sizeof(ThreadParameters), true));
    poThreadParameters->m_poRestFrameworkRuntimeData = this;
    poThreadParameters->m_poTlsNode = poTlsNode;

    pthread_t connectionThread;
    int nStatus = ::pthread_create(&connectionThread, nullptr, StartThread, poThreadParameters);
    _ThrowBaseExceptionIf((0 != nStatus), "Error creating a thread with nStatus: %d.", nStatus);
    // Detach the thread as it terminates on its own by calling pthread_exit
    // Detaching the thread will make sure that the system recycles its underlying resources automatically
    ::pthread_detach(connectionThread);

    this->AddConnection(connectionThread);
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function AddConnection
 * @brief Insert thread id in m_stlConnectionThreads
 * @param[in] connectionThread Thread id
 *
 ********************************************************************************************/

void __thiscall RestFrameworkRuntimeData::AddConnection(
    _in pthread_t connectionThread
    )
{
    __DebugFunction();

    ::pthread_mutex_lock(&m_sMutex);
    m_stlConnectionThreads.insert(connectionThread);
    ::pthread_mutex_unlock(&m_sMutex);
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function DeleteConnection
 * @brief Erase id of current thread returned by pthread_self() from m_stlConnectionThreads
 *
 ********************************************************************************************/

void __thiscall RestFrameworkRuntimeData::DeleteConnection(void)
{
    __DebugFunction();

    ::pthread_mutex_lock(&m_sMutex);
    if (m_stlConnectionThreads.end() != m_stlConnectionThreads.find(pthread_self()))
    {
        m_stlConnectionThreads.erase(pthread_self());
    }
    ::pthread_mutex_unlock(&m_sMutex);
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function RunThread
 * @brief Parse request packet and route to requested plugin. Get the response and send it back
 *        to the caller.
 * @param[in] poTlsNode TlsNode object pointer
 *
 ********************************************************************************************/

void __thiscall RestFrameworkRuntimeData::RunThread(
    _in TlsNode * poTlsNode
    )
{
    __DebugFunction();

    try
    {
        SmartMemoryAllocator oLocalSmartMemoryAllocator;
        // The format of the request data is:
        //
        // +------------------------------------------------------------------------------------+
        // | [Dword] 0x436f6e74                                                                 |
        // +------------------------------------------------------------------------------------+
        // | [uint32_t] SizeInBytesOfRestRequestStructuredBuffer                                |
        // +------------------------------------------------------------------------------------+
        // | [SizeInBytesOfRestRequestStructuredBuffer] RestRequestStructuredBuffer             |
        // +------------------------------------------------------------------------------------+
        // | [Dword] 0x656e6420                                                                 |
        // +------------------------------------------------------------------------------------+

        // Read Header of the Rest Request
        // unHeader = MARKER(dword) + SizeInBytesOfRequestData(uint32_t)
        unsigned int unHeaderSize = sizeof(Dword) + sizeof(uint32_t);
        std::vector<Byte> stlHeaderData = poTlsNode->Read(unHeaderSize, 100);
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        // Parse Header of the Rest Request
        const Byte *  pbCurrentHeaderByte = stlHeaderData.data();
        _ThrowBaseExceptionIf((0x436f6e74 != *((Dword *) pbCurrentHeaderByte)), "Invalid request format: Expected [TAG] = 0x436f6e74 but got 0x%08X", *((Dword *) pbCurrentHeaderByte));
        pbCurrentHeaderByte += sizeof(Dword);
        unsigned int unRequestDataSizeInBytes = *((uint32_t *) pbCurrentHeaderByte);
        pbCurrentHeaderByte += sizeof(uint32_t);

        // Use unRequestDataSizeInBytes to read the request body
        unsigned int unBodySize = unRequestDataSizeInBytes + sizeof(Dword);
        std::vector<Byte> stlBodyData = poTlsNode->Read(unBodySize, 100);
        _ThrowBaseExceptionIf((0 == stlBodyData.size()), "Dead Packet.", nullptr);

        // Parse Body of the Rest Request
        const Byte * pbCurrentBodyByte = stlBodyData.data();
        std::vector<Byte> stlSerializedRestRequest;
        if (0 < unRequestDataSizeInBytes)
        {
            stlSerializedRestRequest.resize(unRequestDataSizeInBytes);
            ::memcpy((void *) stlSerializedRestRequest.data(), (const void *) pbCurrentBodyByte, unRequestDataSizeInBytes);
            pbCurrentBodyByte += unRequestDataSizeInBytes;
        }
        _ThrowBaseExceptionIf((0x656e6420 != *((Dword *) pbCurrentBodyByte)), "Invalid request format: Expected [TAG] = 0x656e6420 but got 0x%08X", *((Dword *) pbCurrentBodyByte));
        
        StructuredBuffer oRequestData(stlSerializedRestRequest);

        std::cout << oRequestData.GetString("Verb");
        std::cout << " " << oRequestData.GetString("Resource") <<std::endl;

        // Get the callback functions associated with the 64bithash of the plugin name
        std::string strPluginName = oRequestData.GetString("PluginName");
        SubmitRequestFn fnSubmitRequestFunction = m_poDictionaryManager->GetSubmitRequestFunction(strPluginName.c_str());
        _ThrowBaseExceptionIf((nullptr == fnSubmitRequestFunction), "Error: Invalid SubmitRequestFn.", nullptr);
        GetResponseFn fnGetResponseFunction = m_poDictionaryManager->GetGetResponseFunction(strPluginName.c_str());
        _ThrowBaseExceptionIf((nullptr == fnGetResponseFunction), "Error: Invalid GetResponseFn.", nullptr);

        // Route to the plugin by calling its Callback functions
        // Submit am incoming request by calling plugin's SubmitRequest function
        // The function will send back a 64BitIdentifier and the size of the response
        unsigned int unSerializedResponseSizeInBytes = 0;
        uint64_t un64Identifier = fnSubmitRequestFunction(oRequestData.GetSerializedBufferRawDataPtr(), oRequestData.GetSerializedBufferRawDataSizeInBytes(), &unSerializedResponseSizeInBytes);
        _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == un64Identifier), "Error submitting request.", nullptr);
        __DebugAssert(0 < unSerializedResponseSizeInBytes);


        // Call plugin's GetResponse function. The function will get the reponse associated with the identifier argument
        // and compare the size argument and the saved response's size. If they are equal, it will send back address to the response
        Byte * pbSerializedResponseBuffer = ((Byte *) oLocalSmartMemoryAllocator.Allocate(unSerializedResponseSizeInBytes, true));
        bool fSuccess = fnGetResponseFunction(un64Identifier, pbSerializedResponseBuffer, unSerializedResponseSizeInBytes);
        _ThrowBaseExceptionIf((false == fSuccess), "Error: Getting response failed.", nullptr);

        StructuredBuffer oResponseStructuredBuffer(pbSerializedResponseBuffer, unSerializedResponseSizeInBytes);

        // Check whether the call back function returned termination signal or not
        if (true == oResponseStructuredBuffer.IsElementPresent("TerminateSignalEncountered", BOOLEAN_VALUE_TYPE))
        {
            ::pthread_mutex_lock(&m_sMutex);
            m_fTerminateSignalEncountered = true;
            ::pthread_mutex_unlock(&m_sMutex);
        }
        else
        {
            // Create a response packet
            unsigned int unSerializedBufferSizeInBytes = sizeof(uint32_t) + oResponseStructuredBuffer.GetSerializedBufferRawDataSizeInBytes();
            std::vector<Byte> stlSerializedBuffer(unSerializedBufferSizeInBytes);
            Byte * pbSerializedBuffer = (Byte *) stlSerializedBuffer.data();
            *((uint32_t *) pbSerializedBuffer) = (uint32_t) oResponseStructuredBuffer.GetSerializedBufferRawDataSizeInBytes();
            pbSerializedBuffer += sizeof(uint32_t);
            ::memcpy((void *) pbSerializedBuffer, (const void *) oResponseStructuredBuffer.GetSerializedBufferRawDataPtr(), oResponseStructuredBuffer.GetSerializedBufferRawDataSizeInBytes());
            pbSerializedBuffer += oResponseStructuredBuffer.GetSerializedBufferRawDataSizeInBytes();

            // Send back response data
            poTlsNode->Write((const Byte *)stlSerializedBuffer.data(), stlSerializedBuffer.size());
        }
        // Delete the allocated parameter
        oLocalSmartMemoryAllocator.Deallocate(pbSerializedResponseBuffer);
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
        // send back error message
        unsigned int unErrorResponseSizeInBytes = sizeof(uint32_t) + strlen(oBaseException.GetExceptionMessage());
        std::vector<Byte> stlErrorMessage(unErrorResponseSizeInBytes);
        Byte * pbErrorMessage = (Byte *) stlErrorMessage.data();
        *((uint32_t *) pbErrorMessage) = (uint32_t) strlen(oBaseException.GetExceptionMessage());
        pbErrorMessage += sizeof(uint32_t);
        ::memcpy((void *) pbErrorMessage, (const void *) oBaseException.GetExceptionMessage(), strlen(oBaseException.GetExceptionMessage()));
        poTlsNode->Write((const Byte *) stlErrorMessage.data(), stlErrorMessage.size());
    }

    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
        // send back error message
        Byte bErrorResponse[] = "DatabaseGateway Error: processing the request failed.";
        unsigned int unErrorResponseSizeInBytes = sizeof(uint32_t) + sizeof(bErrorResponse);
        std::vector<Byte> stlErrorMessage(unErrorResponseSizeInBytes);
        Byte * pbErrorMessage = (Byte *) stlErrorMessage.data();
        *((uint32_t *) pbErrorMessage) = (uint32_t) sizeof(bErrorResponse);
        pbErrorMessage += sizeof(uint32_t);
        ::memcpy((void *) pbErrorMessage, (const void *) bErrorResponse, sizeof(bErrorResponse));
        poTlsNode->Write((const Byte *) stlErrorMessage.data(), stlErrorMessage.size());
    }
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function GetNumberOfActiveConnections
 * @brief Fetch number of active connections / threads, as for each connection we create a new thread.
 * @return Number of active connections / threads
 *
 ********************************************************************************************/

unsigned int __thiscall RestFrameworkRuntimeData::GetNumberOfActiveConnections(void) const throw()
{
    __DebugFunction();

    return m_stlConnectionThreads.size();
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function IsTerminationSignalEncountered
 * @brief Fetches boolean representing termination signal
 * @param[in] Termination signal
 * @note
 *      The termination signal is used to stop the listening server.
 *
 ********************************************************************************************/

bool __thiscall RestFrameworkRuntimeData::IsTerminationSignalEncountered(void) const throw()
{
    __DebugFunction();

    return m_fTerminateSignalEncountered;
}
