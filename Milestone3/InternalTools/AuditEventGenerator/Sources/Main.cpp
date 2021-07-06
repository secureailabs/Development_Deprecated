/*********************************************************************************************
 *
 * @file Main.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @license Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "ApiCallHelpers.h"
#include "CoreTypes.h"
#include "CommandLine.h"
#include "DateAndTime.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "ExceptionRegister.h"
#include "Guid.h"
#include "StructuredBuffer.h"
#include "ThreadManager.h"

#include <unistd.h>

#include <iostream>
#include <string>
#include <mutex>
#include <queue>

static bool gs_fTerminationSignal = false;
static Guid gs_oVirtualMachineIdentifier;
static Guid gs_oClusterIdentifier;
static Guid gs_oDigitalContractIdentifier;
static Guid gs_oComputationalDomainIdentifier;
static std::string gs_strVirtualMachineIpAddress;
static std::string gs_strDataOwnerAccessToken;
static std::string gs_strResearcherAccessToken;
static std::string gs_strVirtualMachineAccessToken;
static std::string gs_strDataOrganizationAuditEventParentBranchNodeIdentifier;
static std::string gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier;
static unsigned int gs_unDataOwnerAuditEventSequenceNumber = 0;
static unsigned int gs_unResearcherAuditEventSequenceNumber = 0;
static std::mutex gs_stlMutex;
static std::queue<std::string> gs_stlDataOrganizationAuditEventQueue;
static std::queue<std::string> gs_stlResearchOrganizationAuditEventQueue;

/********************************************************************************************/

static void __stdcall RecordAuditEvent(
    _in const char * c_szEventName,
    _in Word wTargetChannelsBitMask,
    _in Dword dwEventType,
    _in const StructuredBuffer & c_oEventData
    )
{
    __DebugFunction();
    
    StructuredBuffer oAuditEvent;
    StructuredBuffer oEventData(c_oEventData.GetBase64SerializedBuffer().c_str());

    Guid oEventIdentifier;
    
    oAuditEvent.PutWord("TargetChannelsBitMask", wTargetChannelsBitMask);
    oAuditEvent.PutString("EventGuid", oEventIdentifier.ToString(eHyphensAndCurlyBraces));
    oAuditEvent.PutQword("EventType", dwEventType);
    oAuditEvent.PutUnsignedInt64("Timestamp", ::GetEpochTimeInMilliseconds());
    oAuditEvent.PutString("EventName", c_szEventName);
    oEventData.PutString("EventName", c_szEventName);
    oAuditEvent.PutString("EncryptedEventData", oEventData.GetBase64SerializedBuffer());
    
    const std::lock_guard<std::mutex> lock(gs_stlMutex);
    //if (0x1000 == (0x1000 & wTargetChannelsBitMask)) // Data Owner
    {
        unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &gs_unDataOwnerAuditEventSequenceNumber, 1);
        oAuditEvent.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
        gs_stlDataOrganizationAuditEventQueue.push(oAuditEvent.GetBase64SerializedBuffer());
    }
    //if (0x0100 == (0x0100 & wTargetChannelsBitMask)) // Researcher
    {
        unsigned int unSequenceNumber = ::__sync_fetch_and_add((int *) &gs_unResearcherAuditEventSequenceNumber, 1);
        oAuditEvent.PutUnsignedInt32("SequenceNumber", unSequenceNumber);
        gs_stlResearchOrganizationAuditEventQueue.push(oAuditEvent.GetBase64SerializedBuffer());
    }
}

/********************************************************************************************/

static void __thiscall AuditEventDispatcher(void)
{
    __DebugFunction();
    
    bool fIsShutdown = false;
    bool fIsShuttingDown = false;
    while (false == fIsShutdown)
    {
        try
        {
            const std::lock_guard<std::mutex> lock(gs_stlMutex);
            
            // Handle lingering events within the data organization audit event queue
            if ((0 < gs_strDataOwnerAccessToken.size())&&(0 < gs_strDataOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                bool fTransmitAuditEvents = false;
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < gs_stlDataOrganizationAuditEventQueue.size())
                {
                    fTransmitAuditEvents = true;
                    StructuredBuffer oNewAuditEvent(gs_stlDataOrganizationAuditEventQueue.front().c_str());
                    if (true == oNewAuditEvent.IsElementPresent("TargetChannelsBitMask", WORD_VALUE_TYPE))
                    {
                        oNewAuditEvent.RemoveElement("TargetChannelsBitMask");
                    }
                    gs_stlDataOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                }
                
                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < gs_strDataOwnerAccessToken.size());
                    __DebugAssert(0 < gs_strDataOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(gs_strDataOwnerAccessToken, gs_strDataOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
            
            if ((0 < gs_strResearcherAccessToken.size())&&(0 < gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size()))
            {
                bool fTransmitAuditEvents = false;
                StructuredBuffer oAuditEventsToTransmit;
                unsigned int unElementIndex = 0;
                while (0 < gs_stlResearchOrganizationAuditEventQueue.size())
                {
                    fTransmitAuditEvents = true;
                    StructuredBuffer oNewAuditEvent(gs_stlResearchOrganizationAuditEventQueue.front().c_str());
                    if (true == oNewAuditEvent.IsElementPresent("TargetChannelsBitMask", WORD_VALUE_TYPE))
                    {
                        oNewAuditEvent.RemoveElement("TargetChannelsBitMask");
                    }
                    gs_stlResearchOrganizationAuditEventQueue.pop();
                    oAuditEventsToTransmit.PutStructuredBuffer(std::to_string(unElementIndex++).c_str(), oNewAuditEvent);
                }

                if (true == fTransmitAuditEvents)
                {    
                    __DebugAssert(0 < gs_strResearcherAccessToken.size());
                    __DebugAssert(0 < gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier.size());
                    
                    ::TransmitAuditEventsToSailWebApiPortal(gs_strResearcherAccessToken, gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier, oAuditEventsToTransmit);
                }
            }
        }
        
        catch (BaseException oException)
        {
            ::RegisterException(oException, __func__, __LINE__);
        }
        
        catch(...)
        {
            ::RegisterUnknownException(__func__, __LINE__);
        }
        
        // This must appear before the check to oStatusMonitor.IsTerminating()
        if (true == fIsShuttingDown)
        {
            fIsShutdown = true;
        }
        
        // Check to see if we are terminating
        if ((false == fIsShuttingDown)&&(true == gs_fTerminationSignal))
        {
            fIsShuttingDown = true;
            StructuredBuffer oEventData;
            oEventData.PutString("IpAddressOfSecureVirtualMachine", gs_strVirtualMachineIpAddress);
            oEventData.PutString("VirtualMachineIdentifier", gs_oClusterIdentifier.ToString(eHyphensAndCurlyBraces));
            oEventData.PutString("ClusterIdentifier", gs_oClusterIdentifier.ToString(eHyphensAndCurlyBraces));
            ::RecordAuditEvent("VM_SHUTDOWN", 0x1100, 0x05, oEventData);
        }
        
        // Put this thread to sleep for 5 seconds
        ::sleep(1);
    }
}

/********************************************************************************************/

static void * __stdcall StartAuditEventDispatcherThread(
    _in void * pParameter
    )
{
    __DebugFunction();
    
    try
    {
        ::AuditEventDispatcher();
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch (...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }
    
    return nullptr;
}

/********************************************************************************************/

static bool __stdcall InitializeVirtualMachine(void)
{
    __DebugFunction();

    bool fSuccess = false;

    try
    {
        // First we generate an audit event which marks the start of the virtual machine
        StructuredBuffer oEventData;
        oEventData.PutString("PythonIntepreterVersion", "v3.8");
        oEventData.PutString("SecureVirtualMachineVersion", "0.1.37");
        oEventData.PutString("IpAddressOfSecureVirtualMachine", gs_strVirtualMachineIpAddress);
        oEventData.PutString("VirtualMachineIdentifier", gs_oVirtualMachineIdentifier.ToString(eHyphensAndCurlyBraces));
        oEventData.PutString("ClusterIdentifier", gs_oClusterIdentifier.ToString(eHyphensAndCurlyBraces));
        ::RecordAuditEvent("VM_INITIALIZE", 0x1100, 0x05, oEventData);
        {
            // Make sure we are thread safe
            const std::lock_guard<std::mutex> lock(gs_stlMutex);
            // Make sure all of the parameters are proper
            if ((0 < gs_strDataOwnerAccessToken.size())&&(0 < gs_strVirtualMachineIpAddress.size()))
            {
                gs_strVirtualMachineAccessToken = ::RegisterVirtualMachineWithSailWebApiPortal(gs_strDataOwnerAccessToken, gs_oVirtualMachineIdentifier.ToString(eHyphensAndCurlyBraces), gs_oDigitalContractIdentifier.ToString(eHyphensAndCurlyBraces), gs_strVirtualMachineIpAddress);
                fSuccess = true;
            }
        }
        // Now, we generate an event to recognize the digital contract initialization
        oEventData.Clear();
        oEventData.PutString("DigitalContractIdentifier", gs_oDigitalContractIdentifier.ToString(eHyphensAndCurlyBraces));
        oEventData.PutString("VirtualMachineIdentifier", gs_oVirtualMachineIdentifier.ToString(eHyphensAndCurlyBraces));
        oEventData.PutString("ClusterIdentifier", gs_oClusterIdentifier.ToString(eHyphensAndCurlyBraces));
        ::RecordAuditEvent("DC_INITIALIZE", 0x1100, 0x05, oEventData);
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

static bool __stdcall RegisterDataOwnerEosb(void)
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    try
    {
        // Make sure we are thread safe
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        // Make sure all of the parameters are proper
        if (0 < gs_strDataOwnerAccessToken.size())
        {
            gs_strDataOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineDataOwner(gs_strDataOwnerAccessToken, gs_oVirtualMachineIdentifier.ToString(eHyphensAndCurlyBraces));
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

static bool __stdcall RegisterResearcherEosb(void)
{
    __DebugFunction();
    
    bool fSuccess = false;
    
    try
    {
        // Make sure we are thread safe
        const std::lock_guard<std::mutex> lock(gs_stlMutex);
        // Make sure all of the parameters are proper
        if (0 < gs_strResearcherAccessToken.size())
        {
            gs_strResearcherOrganizationAuditEventParentBranchNodeIdentifier = ::RegisterVirtualMachineResearcher(gs_strResearcherAccessToken, gs_oVirtualMachineIdentifier.ToString(eHyphensAndCurlyBraces));
            fSuccess = true;
        }
    }
    
    catch (BaseException oException)
    {
        ::RegisterException(oException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    return fSuccess;
}

/********************************************************************************************/

int __cdecl main(
    _in int nNumberOfArguments,
    _in char ** pszCommandLineArguments
    )
{
    __DebugFunction();
    
    try
    {
        // Figure out whether or not the GlobalMonitor will be displaying status information
        // or not.
        StructuredBuffer oCommandLineParameters = ::ParseCommandLineParameters(nNumberOfArguments, (const char **) pszCommandLineArguments);
        if ((true == oCommandLineParameters.IsElementPresent("VirtualMachineIpAddress", ANSI_CHARACTER_STRING_VALUE_TYPE))&&(true == oCommandLineParameters.IsElementPresent("DigitalContractIdentifier", ANSI_CHARACTER_STRING_VALUE_TYPE)))
        {
            std::cout << oCommandLineParameters.ToString() << std::endl;
            // Get the command line parameters
            gs_strVirtualMachineIpAddress = oCommandLineParameters.GetString("VirtualMachineIpAddress");
            gs_oDigitalContractIdentifier = oCommandLineParameters.GetString("DigitalContractIdentifier").c_str();
            // Set the target IP address for the web portal
            ::SetIpAddressOfSailWebApiPortalGateway("137.116.90.145", 6200);
            // Login for both the data owner and the researcher
            std::string strDataOwner = "nadams@mghl.com";
            std::string strResearcher = "lbart@igr.com";
            std::string strPassword = "sailpassword";
            gs_strDataOwnerAccessToken = ::LoginToSailWebApiPortal(strDataOwner, strPassword);
            gs_strResearcherAccessToken = ::LoginToSailWebApiPortal(strResearcher, strPassword);
            // Start the AuditEventDispatcher
            ThreadManager * poThreadManager = ThreadManager::GetInstance();
            _ThrowBaseExceptionIf((0xFFFFFFFFFFFFFFFF == poThreadManager->CreateThread("RootOfTrustCodeGroup", StartAuditEventDispatcherThread, nullptr)), "Failed to start the AuditEventDispatcher Thread", nullptr);
            // Some basic startup tasks
            ::InitializeVirtualMachine();
            ::sleep(2);
            ::RegisterDataOwnerEosb();
            ::sleep(2);
            ::RegisterResearcherEosb();
            ::sleep(2);
            // Now we do a bunch of events
            ////////////////////////////////////////////////////////////////////////////////////////////////
            StructuredBuffer oEventData;
            StructuredBuffer oDatasetMetadata;
            StructuredBuffer oDatasetHeader;
            
            oDatasetMetadata.PutInt32("NumberTables", 1);
            oDatasetMetadata.PutInt64("Timestamp", 1616992108);
            oDatasetMetadata.PutString("Name", "sadvfds");
            oDatasetMetadata.PutString("Tags", "xvfdvd");
            oDatasetMetadata.PutGuid("UUID", Guid("9b1c1178-a85f-4e0e-91e9-b509dd65694b"));
            oDatasetMetadata.PutString("Description", "vfdsv");
            oEventData.PutStructuredBuffer("DatasetMetadata", oDatasetMetadata);
            
            oDatasetHeader.PutUnsignedInt32("Version", 1);
            oDatasetHeader.PutString("SignedHash", "6WYJsY/+iCupNo6upBV2W7rLmVZNkcWOHEVczFJPJuQ/+PTaGFbqQVPSeRkHDT2dvn+0/qoqD/qw7YpiH2qOj0r3bb3VxeCXxT8rf2ArBppvFwedSfiDY1/wzUVXOlJ7");
            oDatasetHeader.PutUnsignedInt64("MetadataOffset", 1173);
            oDatasetHeader.PutUnsignedInt32("MetadataSize", 452);
            oEventData.PutStructuredBuffer("DatasetMetadata", oDatasetMetadata);
            ::RecordAuditEvent("LOAD_DATASET", 0x1111, 0x05, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutGuid("ComputationalDomainIdentifier", gs_oComputationalDomainIdentifier);
            oEventData.PutUnsignedInt32("CPU cores", 16);
            oEventData.PutString("Python Version", "python3.8");
            oEventData.PutString("OS Version", "Ubuntu 20.04");
            ::RecordAuditEvent("COMPUTATIONAL_PROCESS_START", 0x1111, 0x05, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Username", "lbart@igr.com");
            oEventData.PutString("OrchestratorIpAddress", "132.34.4.23");
            oEventData.PutString("OrchestratorVersion", "1.0.0");
            oEventData.PutString("EOSB", gs_strResearcherAccessToken);
            ::RecordAuditEvent("CONNECT_SUCCESS", 0x1111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}");
            oEventData.PutString("FunctionNodeSignature", "e0JFRURGMDc5LTUxRkMtNEZCQS1BMzI1LURENjNGMkY5Njk0Rn17QkVFREYwNzktNTFGQy00RkJBLUEzMjUtREQ2M0YyRjk2OTRGfQ==");
            oEventData.PutString("FunctionNodeHash", "e602261bebc8e5b489a961087dbc30c87d2898eceefa7c4236172f6ea40333a6");
            oEventData.PutString("FunctionNodeVersion", "0.1.0");
            oEventData.PutString("FunctionNodeMetadata", "ZTYwMjI2MWJlYmM4ZTViNDg5YTk2MTA4N2RiYzMwYzg3ZDI4OThlY2VlZmE3YzQyMzYxNzJmNmVhNDAzMzNhNmU2MDIyNjFiZWJjOGU1YjQ4OWE5NjEwODdkYmMzMGM4N2QyODk4ZWNlZWZhN2M0MjM2MTcyZjZlYTQwMzMzYTZlNjAyMjYxYmViYzhlNWI0ODlhOTYxMDg3ZGJjMzBjODdkMjg5OGVjZWVmYTdjNDIzNjE3MmY2ZWE0MDMzM2E2ZTYwMjI2MWJlYmM4ZTViNDg5YTk2MTA4N2RiYzMwYzg3ZDI4OThlY2VlZmE3YzQyMzYxNzJmNmVhNDAzMzNhNmU2MDIyNjFiZWJjOGU1YjQ4OWE5NjEwODdkYmMzMGM4N2QyODk4ZWNlZWZhN2M0MjM2MTcyZjZlYTQwMzMzYTY=");
            oEventData.PutString("FunctionNodeCode", "b0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEib0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEib0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEib0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEib0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEib0V2ZW50RGF0YS5QdXRTdHJpbmcoIkZ1bmN0aW9uTm9kZU1ldGFkYXRhIm9FdmVudERhdGEuUHV0U3RyaW5nKCJGdW5jdGlvbk5vZGVNZXRhZGF0YSJvRXZlbnREYXRhLlB1dFN0cmluZygiRnVuY3Rpb25Ob2RlTWV0YWRhdGEi");
            ::RecordAuditEvent("PUSH_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{C0CC1A89-1554-4984-BAB8-96DF8E0016EB}");
            oEventData.PutString("FunctionNodeSignature", "e0MwQ0MxQTg5LTE1NTQtNDk4NC1CQUI4LTk2REY4RTAwMTZFQn17QzBDQzFBODktMTU1NC00OTg0LUJBQjgtOTZERjhFMDAxNkVCfQ==");
            oEventData.PutString("FunctionNodeHash", "2797f76d2750df23a474ca6aba4121a2d7aa9a6e14e89ef5b285e8b6b230106f");
            oEventData.PutString("FunctionNodeVersion", "0.1.0");
            oEventData.PutString("FunctionNodeMetadata", "Mjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2ZjI3OTdmNzZkMjc1MGRmMjNhNDc0Y2E2YWJhNDEyMWEyZDdhYTlhNmUxNGU4OWVmNWIyODVlOGI2YjIzMDEwNmYyNzk3Zjc2ZDI3NTBkZjIzYTQ3NGNhNmFiYTQxMjFhMmQ3YWE5YTZlMTRlODllZjViMjg1ZThiNmIyMzAxMDZmMjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2ZjI3OTdmNzZkMjc1MGRmMjNhNDc0Y2E2YWJhNDEyMWEyZDdhYTlhNmUxNGU4OWVmNWIyODVlOGI2YjIzMDEwNmYyNzk3Zjc2ZDI3NTBkZjIzYTQ3NGNhNmFiYTQxMjFhMmQ3YWE5YTZlMTRlODllZjViMjg1ZThiNmIyMzAxMDZmMjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2Zg==");
            oEventData.PutString("FunctionNodeCode", "TWpjNU4yWTNObVF5TnpVd1pHWXlNMkUwTnpSallUWmhZbUUwTVRJeFlUSmtOMkZoT1dFMlpURTBaVGc1WldZMVlqSTROV1U0WWpaaU1qTXdNVEEyWmpJM09UZG1OelprTWpjMU1HUm1Nak5oTkRjMFkyRTJZV0poTkRFeU1XRXlaRGRoWVRsaE5tVXhOR1U0T1dWbU5XSXlPRFZsT0dJMllqSXpNREV3Tm1ZeU56azNaamMyWkRJM05UQmtaakl6WVRRM05HTmhObUZpWVRReE1qRmhNbVEzWVdFNVlUWmxNVFJsT0RsbFpqVmlNamcxWlRoaU5tSXlNekF4TURabU1qYzVOMlkzTm1ReU56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09TWpjNU4yWTNObVF5TnpVd1pHWXlNMkUwTnpSallUWmhZbUUwTVRJeFlUSmtOMkZoT1dFMlpURTBaVGc1WldZMVlqSTROV1U0WWpaaU1qTXdNVEEyWmpJM09UZG1OelprTWpjMU1HUm1Nak5oTkRjMFkyRTJZV0poTkRFeU1XRXlaRGRoWVRsaE5tVXhOR1U0T1dWbU5XSXlPRFZsT0dJMllqSXpNREV3Tm1ZeU56azNaamMyWkRJM05UQmtaakl6WVRRM05HTmhObUZpWVRReE1qRmhNbVEzWVdFNVlUWmxNVFJsT0RsbFpqVmlNamcxWlRoaU5tSXlNekF4TURabU1qYzVOMlkzTm1ReU56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09");
            ::RecordAuditEvent("PUSH_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{316320C1-7D4C-4EA6-B644-910077C4FCC9}");
            oEventData.PutString("FunctionNodeSignature", "ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fQ==");
            oEventData.PutString("FunctionNodeHash", "032f34169c202bb380948affe084cb4abd20d1396bc7d7d5f7803cf0e7c9aefe");
            oEventData.PutString("FunctionNodeVersion", "0.1.0");
            oEventData.PutString("FunctionNodeMetadata", "Mjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2ZjI3OTdmNzZkMjc1MGRmMjNhNDc0Y2E2YWJhNDEyMWEyZDdhYTlhNmUxNGU4OWVmNWIyODVlOGI2YjIzMDEwNmYyNzk3Zjc2ZDI3NTBkZjIzYTQ3NGNhNmFiYTQxMjFhMmQ3YWE5YTZlMTRlODllZjViMjg1ZThiNmIyMzAxMDZmMjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2ZjI3OTdmNzZkMjc1MGRmMjNhNDc0Y2E2YWJhNDEyMWEyZDdhYTlhNmUxNGU4OWVmNWIyODVlOGI2YjIzMDEwNmYyNzk3Zjc2ZDI3NTBkZjIzYTQ3NGNhNmFiYTQxMjFhMmQ3YWE5YTZlMTRlODllZjViMjg1ZThiNmIyMzAxMDZmMjc5N2Y3NmQyNzUwZGYyM2E0NzRjYTZhYmE0MTIxYTJkN2FhOWE2ZTE0ZTg5ZWY1YjI4NWU4YjZiMjMwMTA2Zg==");
            oEventData.PutString("FunctionNodeCode", "npSallUWmhZbUUwTVRJeFlUSmtOMkZoT1dFMlpURTBaVGc1WldZMVlqSTROV1U0WWpaaU1qTXdNVEEyWmpJM09UZG1OelprTWpjMU1HUm1Nak5oTkRjMFkyRTJZV0poTkRFeU1XRXlaRGRoWVRsaE5tVXhOR1U0T1dWbU5XSXlPRFZsT0dJMllqSXpNREV3Tm1ZeU56azNaamMyWkRJM05UQmtaakl6WVRRM05HTmhObUZpWVRReE1qRmhNbVEzWVdFNVlUWmxNVFJsT0RsbFpqVmlNamcxWlRoaU5tSXlNekF4TURabU1qYzVOMlkzTm1ReU56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09TWpjNU4yWTNObVF5TnpVd1pHWXlNMkUwTnpSallUWmhZbUUwTVRJeFlUSmtOMkZoT1dFMlpURTBaVGc1WldZMVlqSTROV1U0WWpaaU1qTXdNVEEyWmpJM09UZG1OelprTWpjMU1HUm1Nak5oTkRjMFkyRTJZV0poTkRFeU1XRXlaRGRoWVRsaE5tVXhOR1U0T1dWbU5XSXlPRFZsT0dJMllqSXpNREV3Tm1ZeU56azNaamMyWkRJM05UQmtaakl6WVRRM05HTmhObUZpWVRReE1qRmhNbVEzWVdFNVlUWmxNVFJsT0RsbFpqVmlNamcxWlRoaU5tSXlNekF4TURabU1qYzVOMlkzTm1ReU56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09");
            ::RecordAuditEvent("PUSH_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}.{604F8F85-8098-4AE3-8DEB-05CC4AEAB828}");
            oEventData.PutString("Base64Data", "3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05M56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09TWpjNU4yWTNObVF5TnpVd1pHWXlNMkUwTnpSallUWmhZbUUwTVRJeFlUSmtOMkZoT1dFMlpURTBaVGc1WldZMVlqSTROV1U0WWpaaU1qTXdNVEEyWmpJM09UZG1OelprTWpjMU1HUm1Nak5oTkRjMFkyRTJZV0poTkRFeU1XRXlaRGRoWVRsaE5tVXhOR1U0T1dWbU5XSXlPRFZsT0dJMllqSXpNREV3Tm1ZeU56azNaamMyWkRJM05UQmtaakl6WVRRM05HTmhObUZpWVRReE1qRmhNbVEzWVdFNVlUWmxNVFJsT0RsbFpqVmlNamcxWlRoaU5tSXlNekF4TURabU1qYzVOMlkzTm1ReU56VXdaR1l5TTJFME56UmpZVFpoWW1FME1USXhZVEprTjJGaE9XRTJaVEUwWlRnNVpXWTFZakk0TldVNFlqWmlNak13TVRBMlpqSTNPVGRtTnpaa01qYzFNR1JtTWpOaE5EYzBZMkUyWVdKaE5ERXlNV0V5WkRkaFlUbGhObVV4TkdVNE9XVm1OV0l5T0RWbE9HSTJZakl6TURFd05tWXlOemszWmpjMlpESTNOVEJrWmpJellUUTNOR05oTm1GaVlUUXhNakZoTW1RM1lXRTVZVFpsTVRSbE9EbGxaalZpTWpnMVpUaGlObUl5TXpBeE1EWm1NamM1TjJZM05tUXlOelV3WkdZeU0yRTBOelJqWVRaaFltRTBNVEl4WVRKa04yRmhPV0UyWlRFMFpUZzVaV1kxWWpJNE5XVTRZalppTWpNd01UQTJaZz09");
            ::RecordAuditEvent("PUSH_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}");
            ::RecordAuditEvent("EXEC_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}.{604F8F85-8098-4AE3-8DEB-05CC4AEAB828}");
            oEventData.PutString("Base64Data", "TAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05M");
            ::RecordAuditEvent("PUSH_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{C0CC1A89-1554-4984-BAB8-96DF8E0016EB}");
            ::RecordAuditEvent("EXEC_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{C0CC1A89-1554-4984-BAB8-96DF8E0016EB}.{1ADD711B-5395-4C30-9589-7F800C6FCBF4}");
            oEventData.PutString("Base64Data", "zIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05M");
            ::RecordAuditEvent("PULL_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}.{604F8F85-8098-4AE3-8DEB-05CC4AEAB828}");
            oEventData.PutString("Base64Data", "Q0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05M");
            ::RecordAuditEvent("PUSH_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}.{604F8F85-8098-4AE3-8DEB-05CC4AEAB828}");
            oEventData.PutString("Base64Data", "zE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZ");
            ::RecordAuditEvent("PUSH_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{BEEDF079-51FC-4FBA-A325-DD63F2F9694F}.{604F8F85-8098-4AE3-8DEB-05CC4AEAB828}");
            oEventData.PutString("Base64Data", "NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtNNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN");
            ::RecordAuditEvent("PUSH_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("FunctionNodeIdentifier", "{316320C1-7D4C-4EA6-B644-910077C4FCC9}");
            ::RecordAuditEvent("EXEC_FN", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{316320C1-7D4C-4EA6-B644-910077C4FCC9}.{B45A6E68-519C-4EC4-B005-4667CEA38957}");
            oEventData.PutString("Base64Data", "ezMxNjMyMEMxLTdENGFzZGFDLTRFQTYtQjY0YXNkYWRhNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMWRhZDYzMjBDMS03RDRDLTRFQTYtQmFzZDY0NC1hczkxYXNkYWRhc2RhZDAwNzdDNGRhQ0M5fXszMTZzZGEzc2QyMEMxLTdENEMtYWFzZGFzQjZkNDQtOTEwMDc3QzRGQ0M5fXNkezMxNjMyMEMxLTdENGFzZGFDLTRFQTYtQjY0YXNkYWRhNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMWRhZDYzMjBDMS03RDRDLTRFQTYtQmFzZDY0NC1hczkxYXNkYWRhc2RhZDAwNzdDNGRhQ0M5fXszMTZzZGEzc2QyMEMxLTdENEMtYWFzZGFzQjZkNDQtOTEwMDc3QzRGQ0M5fXNkezMxNjMyMEMxLTdENGFzZGFDLTRFQTYtQjY0YXNkYWRhNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMWRhZDYzMjBDMS03RDRDLTRFQTYtQmFzZDY0NC1hczkxYXNkYWRhc2RhZDAwNzdDNGRhQ0M5fXszMTZzZGEzc2QyMEMxLTdENEMtYWFzZGFzQjZkNDQtOTEwMDc3QzRGQ0M5fXNkezMxNjMyMEMxLTdENGFzZGFDLTRFQTYtQjY0YXNkYWRhNC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMWRhZDYzMjBDMS03RDRDLTRFQTYtQmFzZDY0NC1hczkxYXNkYWRhc2RhZDAwNzdDNGRhQ0M5fXszMTZzZGEzc2QyMEMxLTdENEMtYWFzZGFzQjZkNDQtOTEwMDc3QzRGQ0M5fXNk");
            ::RecordAuditEvent("PULL_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutString("Identifier", "{316320C1-7D4C-4EA6-B644-910077C4FCC9}.{3073F833-9F9E-4C93-929D-E5FFD10B7C15}");
            oEventData.PutString("Base64Data", "ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXNkezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXNkezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXszMTYzMjBDMS03RDRDLTRFQTYtQjY0NC05MTAwNzdDNEZDQzl9ezMxNjMyMEMxLTdENEMtNEVBNi1CNjQ0LTkxMDA3N0M0RkNDOX17MzE2MzIwQzEtN0Q0Qy00RUE2LUI2NDQtOTEwMDc3QzRGQ0M5fXNk");
            ::RecordAuditEvent("PULL_DATA", 0x0111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutBoolean("Success", true);
            oEventData.PutString("Username", "lbart@igr.com");
            ::RecordAuditEvent("LOGOFF", 0x1111, 0x04, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            oEventData.Clear();
            oEventData.PutBoolean("Success", true);
            oEventData.PutGuid("ComputationalDomainIdentifier", gs_oComputationalDomainIdentifier);
            ::RecordAuditEvent("COMPUTATIONAL_PROCESS_STOP", 0x1111, 0x05, oEventData);
            ::sleep(2);
            ////////////////////////////////////////////////////////////////////////////////////////////////
            gs_fTerminationSignal = true;
            poThreadManager->JoinAllThreads();
        }
    }

    catch(BaseException oBaseException)
    {
        ::RegisterException(oBaseException, __func__, __LINE__);
    }
    
    catch(...)
    {
        ::RegisterUnknownException(__func__, __LINE__);
    }

    // All we have to do now is basically wait for ALL other threads to finish
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    poThreadManager->JoinAllThreads();
        
    return 0;
}