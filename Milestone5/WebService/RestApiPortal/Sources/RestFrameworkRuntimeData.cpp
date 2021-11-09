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
#include "HttpRequestParser.h"
#include "JsonValue.h"
#include "PluginVersion.h"
#include "Utils.h"

#include <utility>

/********************************************************************************************
 *
 * @brief Global map to associate an http code with a string
 *
 ********************************************************************************************/

std::map<Dword, std::string> g_stlHttpCodes = {
    {eOk, "OK"},
    {eCreated, "Created"},
    {eNoContent, "NoContent"},
    {eBadRequest, "BadRequest"},
    {eUnauthorized, "Unauthorized"},
    {eForbidden, "Forbidden"},
    {eNotFound, "NotFound"},
    {eRequestTimeout, "RequestTimeout"},
    {eInternalError, "InternalServerError"}
};

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
    m_qwRequiredNumberOfUnixConnections = 0;
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

    std::string strResponseData;

    Qword qwRequiredNumberOfUnixConnections = 0;

    try
    {
        SmartMemoryAllocator oLocalSmartMemoryAllocator;

        // Read Header of the Rest Request one byte at a time
        bool fIsEndOfHeader = false;
        std::vector<Byte> stlHeaderData;
        while (false == fIsEndOfHeader)
        {   
            std::vector<Byte> stlBuffer = poTlsNode->Read(1, 2000);
            // Check whether the read was successful or not
            if (0 < stlBuffer.size())
            {
                stlHeaderData.push_back(stlBuffer.at(0));
                if (4 <= stlHeaderData.size())
                {
                    if (("\r\n\r\n" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())) || ("\n\r\n\r" == std::string(stlHeaderData.end() - 4, stlHeaderData.end())))
                    {
                        fIsEndOfHeader = true;
                    }
                }
            }
            else 
            {
                fIsEndOfHeader = true;
            }
        }
        _ThrowBaseExceptionIf((0 == stlHeaderData.size()), "Dead Packet.", nullptr);

        // Parse Header of the Rest Request
        HttpRequestParser oParser;
        std::string strRequestHeader = std::string(stlHeaderData.begin(), stlHeaderData.end());
        bool fSuccess = oParser.ParseRequest(strRequestHeader);
        _ThrowBaseExceptionIf((false == fSuccess), "Error: Parsing request failed.", nullptr);
        // Get Verb and Resource from the parsed request
        std::string strVerb = oParser.GetHeaderValue("Verb");
        std::string strResource = oParser.GetHeaderValue("Resource");
        std::cout << "Rest request: " << strVerb << " " << strResource << std::endl;
        _ThrowBaseExceptionIf(((true == strVerb.empty()) || (true == strResource.empty())), "Invalid Request.", nullptr);

        // Parse request body
        // If request specifies plugin version, find the plugin with the requested version
        // If not then find the plugin with the latest version
        StructuredBuffer * poRequestParameters;
        std::vector<Byte> stlSerializedParameters;
        bool fIsJson = false;
        int nMatchingPluginIndex;
        if (true == oParser.HeaderExists("Content-Length"))
        {
            unsigned int unContentLength = std::stoi(oParser.GetHeaderValue("Content-Length"));
            if (0 < unContentLength)
            {
                // Read request content
                std::vector<Byte> stlBodyData = poTlsNode->Read(unContentLength, 2000);
                _ThrowBaseExceptionIf((0 == stlBodyData.size()), "Dead Packet.", nullptr);
                std::string strRequestBody = std::string(stlBodyData.begin(), stlBodyData.end());

                // Check Content-Type
                _ThrowBaseExceptionIf((false == oParser.HeaderExists("Content-Type")), "Invalid request format.", nullptr);
                std::string strContentType = oParser.GetHeaderValue("Content-Type");
                if ("application/json" == strContentType)
                {
                    // Parse Json
                    std::string strUnEscapseJsonString = ::UnEscapeJsonString(strRequestBody);
                    stlSerializedParameters = JsonValue::ParseDataToStructuredBuffer(strUnEscapseJsonString.c_str());
                    fIsJson = true;
                }
                else if ("application/x-www-form-urlencoded" == strContentType)
                {
                    oParser.ParseUrlEncodedString(strRequestBody);
                }
                else
                {
                    _ThrowBaseException("Content-Type not supported.", nullptr);
                }
            }
        }

        if (true == fIsJson)
        {
            poRequestParameters = new StructuredBuffer(stlSerializedParameters);
        }
        else
        {
            poRequestParameters = new StructuredBuffer();
        }

        nMatchingPluginIndex = this->ParseRequestContent(oParser, poRequestParameters);
        _ThrowBaseExceptionIf((-1 == nMatchingPluginIndex), "Resource not found.", nullptr);
        // Validate request data
        StructuredBuffer oRequestData;
        std::vector<Byte> stlSerializedRestRequest = poRequestParameters->GetSerializedBuffer();
        bool fValid = this->ValidateRequestData(stlSerializedRestRequest, nMatchingPluginIndex, strResource, &oRequestData);
        _ThrowBaseExceptionIf((false == fValid), "Invalid request data.", nullptr);
        // Add verb and resource to the parsed and validated structured buffer
        oRequestData.PutString("Verb", strVerb);
        oRequestData.PutString("Resource", strResource);

        // Add required number of unix connections to connections count
        // This count is used by the RestFramework to determine if a new connection can be accepted as new connection means creating a new thread
        qwRequiredNumberOfUnixConnections = oRequestData.GetQword("NumberOfUnixConnections");
        m_qwRequiredNumberOfUnixConnections += qwRequiredNumberOfUnixConnections;

        // Get the plugin name associated with the best matching plugin index
        std::string strPluginName(m_poDictionaryManager->GetPluginName(nMatchingPluginIndex));
        // Get the callback functions associated with the 64bithash of the plugin name
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
        _ThrowBaseExceptionIf((0 >= unSerializedResponseSizeInBytes), "Error processing request", nullptr);

        // Call plugin's GetResponse function. The function will get the reponse associated with the identifier argument
        // and compare the size argument and the saved response's size. If they are equal, it will send back address to the response
        Byte * pbSerializedResponseBuffer = ((Byte *) oLocalSmartMemoryAllocator.Allocate(unSerializedResponseSizeInBytes, true));
        fSuccess = fnGetResponseFunction(un64Identifier, pbSerializedResponseBuffer, unSerializedResponseSizeInBytes);
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
            JsonValue * poResponseJson = JsonValue::ParseStructuredBufferToJson(oResponseStructuredBuffer);
            std::string strResponseString = poResponseJson->ToString();
            Dword dwStatus = oResponseStructuredBuffer.GetDword("Status");
            std::string strResponseHeader = "HTTP/1.1 "+ std::to_string(dwStatus) +" "+ g_stlHttpCodes[dwStatus] +" \r\nContent-Length: " + std::to_string(strResponseString.size()) + "\r\nConnection: close\r\nContent-Type: application/json\r\n\r\n";
            strResponseData = strResponseHeader;
            strResponseData += strResponseString;
            // Delete the JsonValue pointer
            poResponseJson->Release();
        }
        // Delete the allocated parameters
        poRequestParameters->Release();
        oLocalSmartMemoryAllocator.Deallocate(pbSerializedResponseBuffer);
    }
    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
        // create error message
        if (strcmp("Resource not found.",oBaseException.GetExceptionMessage()) == 0)
        {
            strResponseData = "HTTP/1.1 404 NotFound\r\nConnection: close\r\n\r\n";
        }
        else
        {
            strResponseData = "HTTP/1.1 400 BadRequest\r\nConnection: close\r\n\r\n";
        }
        std::cout << "\n\nRest Response:\n\n" << strResponseData << std::endl;
    }
    catch(...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
        // create error message
        strResponseData = "HTTP/1.1 500 InternalServerError\r\nConnection: close\r\n\r\n";
        std::cout << "\n\nRest Response:\n\n" << strResponseData << std::endl;
    }

    try
    {
        // Send back the response data
        poTlsNode->Write((const Byte *) strResponseData.data(), strResponseData.size());
    }
    catch (BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __FILE__, __LINE__);
    }
    catch (...)
    {
        ::RegisterUnknownException(__func__, __FILE__, __LINE__);
    }

    // Decrement the number of required unix connections by the plugin once the transaction is complete
    m_qwRequiredNumberOfUnixConnections -= qwRequiredNumberOfUnixConnections;
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

    return (m_stlConnectionThreads.size() + m_qwRequiredNumberOfUnixConnections);
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function IsTerminationSignalEncountered
 * @brief Fetches boolean representing termination signal
 * @note
 *      The termination signal is used to stop the listening server.
 *
 ********************************************************************************************/

bool __thiscall RestFrameworkRuntimeData::IsTerminationSignalEncountered(void) const throw()
{
    __DebugFunction();

    return m_fTerminateSignalEncountered;
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function FindPlugin
 * @brief Wrapper that calls other functions to find the plugin with the best matching API and validates parameters
 * @param[in] c_oParser Reference to HttpRequestParser object
 * @param[in] poRequestParameters Pointer to structured buffer containing request data
 * @param[out] poValidatedRequestData Pointer to the parsed and validated request structured buffer
 * @return Index of the best matching plugin
 *
 ********************************************************************************************/
int __thiscall RestFrameworkRuntimeData::ParseRequestContent(
    _in const HttpRequestParser & c_oParser,
    _in StructuredBuffer * poRequestParameters
    )
{
    __DebugFunction();

    int nMatchingPluginIndex;
    Qword qwRequestedPluginVersion = 0xFFFFFFFFFFFFFFFF;

    // Loop through query parameters and add them to poRequestParameters after checking their type
    std::vector<std::string> stlParameterKeys = c_oParser.GetParameterKeys();
    for (std::string strKey : stlParameterKeys)
    {
        if (true == ::IsBoolean(c_oParser.GetParameterValue(strKey)))
        {
            poRequestParameters->PutBoolean(strKey.c_str(), std::atoi(c_oParser.GetParameterValue(strKey).c_str()));
        }
        else
        {
            poRequestParameters->PutString(strKey.c_str(), c_oParser.GetParameterValue(strKey));
        }
    }

    if (true == poRequestParameters->IsElementPresent("PluginVersion", QWORD_VALUE_TYPE))
    {
        qwRequestedPluginVersion = poRequestParameters->GetQword("PluginVersion");
    }

    nMatchingPluginIndex = this->FindPlugin(c_oParser, qwRequestedPluginVersion);

    return nMatchingPluginIndex;
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function FindPlugin
 * @brief Finds the plugin with the best matching API
 * @param[in] c_oParser Reference to HttpRequestParser object
 * @param[in] qwRequestedPluginVersion Requested plugin version if specified, 0xFFFFFFFFFFFFFFFF otherwise
 * @return Index of the plugin
 *
 ********************************************************************************************/

int __thiscall RestFrameworkRuntimeData::FindPlugin(
    _in const HttpRequestParser & c_oParser,
    _in Qword qwRequestedPluginVersion
    )
{
    __DebugFunction();

    int nMatchingPluginIndex = -1;

    std::map<int, Qword> stlMatchingPlugins;
    std::string strRequestedResource = c_oParser.GetHeaderValue("Resource");
    std::string strRequestedVerb = c_oParser.GetHeaderValue("Verb");
    bool fFindSpecifiedVersion = false;

    // Check if request specifies a plugin version
    if (0xFFFFFFFFFFFFFFFF != qwRequestedPluginVersion)
    {
        fFindSpecifiedVersion = true;
    }

    PluginVersion oRequestedVersion(qwRequestedPluginVersion);

    // Find the best matching plugin(s); there can be different versions of the same plugin.
    // In that case, add all matching plugins to a map and sort them based on their plugin version.
    // The best matching plugin will be the one with the latest version
    for(unsigned int unPluginBufferIndex = 0; unPluginBufferIndex < m_poDictionaryManager->GetPluginStructuredBufferSize(); ++unPluginBufferIndex)
    {
        StructuredBuffer oPluginDictionary(m_poDictionaryManager->GetPluginSerializedDictionary(unPluginBufferIndex));

        // Check if Plugin has the requested resource
        if (true == oPluginDictionary.IsElementPresent(strRequestedResource.c_str(), INDEXED_BUFFER_VALUE_TYPE))
        {
            StructuredBuffer oPluginParameters(oPluginDictionary.GetStructuredBuffer(strRequestedResource.c_str()));
            std::string strVerb = oPluginParameters.GetString("Verb");
            // Check if the verb is the same as the request verb
            if (strRequestedVerb == strVerb)
            {
                Qword qwPluginVersion = m_poDictionaryManager->GetPluginVersion(unPluginBufferIndex);
                PluginVersion oPluginVersion(qwPluginVersion);
                // If plugin version is specified, check if the Plugin version can be rounded down to requested version
                if (true == fFindSpecifiedVersion)
                {
                    if (true == oPluginVersion.RoundDown(oRequestedVersion))
                    {
                        stlMatchingPlugins[unPluginBufferIndex] = qwPluginVersion;
                    }
                }
                else
                {
                    stlMatchingPlugins[unPluginBufferIndex] = qwPluginVersion;
                }
            }
        }
    }

    // Save the best matching plugins indexes and versions in a vector and sort by the version
    if (false == stlMatchingPlugins.empty())
    {
        std::vector<std::pair<int, Qword>> stlSortedMatchingPlugins;
        for (std::map<int, Qword>::iterator itrMatchingPluginsIterator = stlMatchingPlugins.begin(); itrMatchingPluginsIterator != stlMatchingPlugins.end(); ++itrMatchingPluginsIterator)
        {
            stlSortedMatchingPlugins.push_back(*itrMatchingPluginsIterator);
        }

        sort(stlSortedMatchingPlugins.begin(), stlSortedMatchingPlugins.end(), [=](std::pair<int, Qword>& a, std::pair<int, Qword>& b)
        {
            return PluginVersion(a.second) < PluginVersion(b.second);
        }
        );

        // Get the plugin index with the latest version
        nMatchingPluginIndex = stlSortedMatchingPlugins.at(stlMatchingPlugins.size() - 1).first;
    }

    return nMatchingPluginIndex;
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function ValidateRequestData
 * @brief Validate the requested parameters by comparing their values against the plugin defined range
 * @param[in] c_oSerializedRequest Reference to serialized request buffer
 * @param[in] unMatchingPluginIndex Index of best API matching plugin in m_poDictionaryManager
 * @param[in] c_strResourceString String representing the requested resource
 * @param[out] poRequestData stores the validated parsed request data
 * @return Boolean representing result of the validation
 *
 ********************************************************************************************/

bool __thiscall RestFrameworkRuntimeData::ValidateRequestData(
    _in const std::vector<Byte> & c_stlSerializedRequest,
    _in unsigned int unMatchingPluginIndex,
    _in const std::string & c_strResourceString,
    _out StructuredBuffer * poRequestData
    )
{
    __DebugFunction();

    bool fValid = true;

    StructuredBuffer oSerializedRequest(c_stlSerializedRequest);
    StructuredBuffer oPluginDictionary(m_poDictionaryManager->GetPluginSerializedDictionary(unMatchingPluginIndex));
    StructuredBuffer oPluginParameters(oPluginDictionary.GetStructuredBuffer(c_strResourceString.c_str()));

    // Validate required and optional parameters
    if (true == oPluginParameters.IsElementPresent("Parameters", INDEXED_BUFFER_VALUE_TYPE))
    {
        StructuredBuffer oParameters = oPluginParameters.GetStructuredBuffer("Parameters");
        std::vector<std::string> stlNamesOfParameters = oParameters.GetNamesOfElements();

        for (unsigned int unParameterIndex = 0; ((true == fValid) && (unParameterIndex < stlNamesOfParameters.size())); ++unParameterIndex)
        {
            const char * c_szParameterName = stlNamesOfParameters.at(unParameterIndex).c_str();
            if ((0 != strcmp( c_szParameterName, "ElementType")) && (0 != strcmp( c_szParameterName, "IsRequired")) && (0 != strcmp( c_szParameterName, "Range")) && (0 != strcmp( c_szParameterName, "RangeType")))
            {
                StructuredBuffer oPluginParameter = oParameters.GetStructuredBuffer(c_szParameterName);
                // Call ValidateParameter to validate c_szParameterName in the request data
                fValid = this->ValidateParameter(c_szParameterName, oSerializedRequest, oPluginParameter, poRequestData);
            }
        }
    }

    // Add required number of unix connections to connections count
    // This count is used by the RestFramework to determine if a new connection can be accepted as new connection means creating a new thread
    if (true == fValid)
    {
        poRequestData->PutQword("NumberOfUnixConnections", oPluginParameters.GetQword("NumberOfUnixConnections"));
    }

    return fValid;
}

/********************************************************************************************
 *
 * @class RestFrameworkRuntimeData
 * @function ValidateParameter
 * @brief Validates of the requested parameters by comparing their values against the plugin defined range
 * @param[in] c_szRequireParameterName Parameter to be validated
 * @param[in] c_oSerializedRequest Reference to serialized request buffer
 * @param[in] c_oPluginParameter Reference to structured buffer containing metadata of a plugin parameter
 * @param[in out] poRequestStructuredBuffer Pointer to the parsed and validated request structured buffer
 * @return Boolean representing result of the validation
 *
 ********************************************************************************************/

bool __thiscall RestFrameworkRuntimeData::ValidateParameter(
    _in const char * c_szRequireParameterName,
    _in const StructuredBuffer & c_oSerializedRequest,
    _in const StructuredBuffer & c_oPluginParameter,
    _inout StructuredBuffer * poRequestStructuredBuffer
    )
{
    __DebugFunction();

    bool fValid = true, fExists = false;

    // Get type of the parameter
    Byte bElementType =  c_oPluginParameter.GetByte("ElementType");
    bool fIsRequiredParam = c_oPluginParameter.GetBoolean("IsRequired");
    // Check if range specified
    bool fRangeSpecified = c_oPluginParameter.IsElementPresent("Range", ANSI_CHARACTER_STRING_VALUE_TYPE);
    if (true == fRangeSpecified)
    {
        _ThrowBaseExceptionIf((false == c_oPluginParameter.IsElementPresent("RangeType", BYTE_VALUE_TYPE)), "Error: Range Specified but RangeType not specified.", nullptr);
    }

    // Compare request parameter type with plugin parameter type
    // For number types, cast the request parameter to the parameter type.
    // Because all number types were added as float64 when parsing from json to structured buffer.
    // Get parameter value and validate against plugin range
    if (NULL_VALUE_TYPE == bElementType)
    {
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, NULL_VALUE_TYPE);
        if (true == fExists)
        {
            poRequestStructuredBuffer->PutNull(c_szRequireParameterName);
        }
    }
    else if (BOOLEAN_VALUE_TYPE == bElementType)
    {
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, BOOLEAN_VALUE_TYPE);
        if (true == fExists)
        {
            poRequestStructuredBuffer->PutBoolean(c_szRequireParameterName, (bool) c_oSerializedRequest.GetBoolean(c_szRequireParameterName));
        }
    }
    else if ((ANSI_CHARACTER_VALUE_TYPE == bElementType) || (ANSI_CHARACTER_STRING_VALUE_TYPE == bElementType))
    {
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, ANSI_CHARACTER_STRING_VALUE_TYPE);
        if (true == fExists)
        {
            std::string strParameterValue = c_oSerializedRequest.GetString(c_szRequireParameterName);
            // validate if the request string has allowable characters
            if (true == fRangeSpecified)
            {
                std::string strAllowedCharacters = c_oPluginParameter.GetString("Range");
                Byte bRangeType = c_oPluginParameter.GetByte("RangeType");
                if (ANSI_CHARACTER_VALUE_TYPE == bRangeType)
                {
                    for (unsigned int unStringIndex = 0; unStringIndex < strParameterValue.length(); ++unStringIndex)
                    {
                        if (std::string::npos == strAllowedCharacters.find(strParameterValue[unStringIndex]))
                        {
                            fValid = false;
                        }
                    }
                }
            }
            if (true == fValid)
            {
                poRequestStructuredBuffer->PutString(c_szRequireParameterName, c_oSerializedRequest.GetString(c_szRequireParameterName));
            }
        }
    }
    else if ((0x04 <= bElementType) && (0x11 >= bElementType))
    {
        float64_t fl64ParameterValue;
        // Check if request has the parameter as a number or as a string
        // If it has the parameter as the number then try casting to float64
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, FLOAT64_VALUE_TYPE);
        if (true == fExists)
        {
            fl64ParameterValue = c_oSerializedRequest.GetFloat64(c_szRequireParameterName);
        }
        else if (false == fExists)
        {
            fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, ANSI_CHARACTER_STRING_VALUE_TYPE);
            if (true == fExists)
            {
                fl64ParameterValue = std::stod(c_oSerializedRequest.GetString(c_szRequireParameterName), 0); 
            }
        }
        // Validate the parameter, if valid then add it to the StructuredBuffer w.r.t the plugin parameter type
        if (true == fExists)
        {
            if ((0x0A <= bElementType) && (0x11 >= bElementType))
            {
                if (0 > (Qword) fl64ParameterValue)
                {
                    fValid = false;
                }
            }
            if (true == fRangeSpecified)
            {
                std::string strRange = c_oPluginParameter.GetString("Range");
                Byte bRangeType = c_oPluginParameter.GetByte("RangeType");
                // Check if the number is allowed and if it is in the specified range or not
                if ((0x0A <= bElementType) && (0x11 >= bElementType))
                {
                    fValid = ::ValidateUnsignedNumber((Qword) fl64ParameterValue, strRange, bRangeType);
                }
                else 
                {
                    fValid = ::ValidateSignedNumber(fl64ParameterValue, strRange, bRangeType);
                }
            }
            // Parse the number type and cast it to plugin parameter type
            if (true == fValid)
            {
                ::PutJsonNumberToStructuredBuffer(c_szRequireParameterName, bElementType, fl64ParameterValue, poRequestStructuredBuffer);
            }
        }
    }
    else if (GUID_VALUE_TYPE == bElementType)
    {
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, ANSI_CHARACTER_STRING_VALUE_TYPE);
        if (true == fExists)
        {
            try
            {
                Guid oGuid(c_oSerializedRequest.GetString(c_szRequireParameterName).c_str());
                poRequestStructuredBuffer->PutGuid(c_szRequireParameterName, oGuid);
            }
            catch(const std::exception& e)
            {
                fValid = false;
            }
        }
    }
    else if (BUFFER_VALUE_TYPE == bElementType)
    {
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, ANSI_CHARACTER_STRING_VALUE_TYPE);
        if (true == fExists)
        {
            try
            {
                std::string strValue = c_oSerializedRequest.GetString(c_szRequireParameterName);
                std::string strDecodedValue = ::Base64Decode(strValue);
                std::vector<Byte> stlValue(strDecodedValue.begin(), strDecodedValue.end());
                poRequestStructuredBuffer->PutBuffer(c_szRequireParameterName, stlValue);
            }
            catch(const std::exception& e)
            {
                fValid = false;
            }
        }
    }
    else if (INDEXED_BUFFER_VALUE_TYPE == bElementType)
    {
        std::vector<std::string> stlNamesOfParameters = c_oPluginParameter.GetNamesOfElements();
        fExists = c_oSerializedRequest.IsElementPresent(c_szRequireParameterName, INDEXED_BUFFER_VALUE_TYPE);

        if (true == fExists)
        {
            StructuredBuffer oRequestStructuredBufferParameter = c_oSerializedRequest.GetStructuredBuffer(c_szRequireParameterName);
            for (unsigned int unParameterIndex = 0; ((true == fValid) && (unParameterIndex < stlNamesOfParameters.size())); ++unParameterIndex)
            {
                const char * c_szName = stlNamesOfParameters.at(unParameterIndex).c_str();
                if ((0 != strcmp( c_szName, "ElementType")) && (0 != strcmp( c_szName, "IsRequired")) && (0 != strcmp( c_szName, "Range")) && (0 != strcmp( c_szName, "RangeType")))
                {
                    fValid = this->ValidateParameter(c_szName, oRequestStructuredBufferParameter, c_oPluginParameter.GetStructuredBuffer(c_szName), &oRequestStructuredBufferParameter);
                }
            }
            if (true == fValid)
            {
                poRequestStructuredBuffer->PutStructuredBuffer(c_szRequireParameterName, oRequestStructuredBufferParameter);
            }
        }
    }
    else
    {
        fExists = true;
        fValid = false;
    }

    // If the paramter is an optional parameter and it does not exist in the request, then the request is still valid
    if (false == fExists)
    {
        if (true == fIsRequiredParam)
        {
            fValid = false;
        }
        else
        {
            fValid = true;
        }
    }

    return fValid;
}
