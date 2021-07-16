/*********************************************************************************************
 *
 * @file JobEngine.cpp
 * @author Prawal Gangwar
 * @date 27 May 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#include "JobEngine.h"
#include "DebugLibrary.h"
#include "Exceptions.h"
#include "SocketClient.h"
#include "DataConnector.h"
#include "IpcTransactionHelperFunctions.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>

#include <iostream>

#define cout cout << std::this_thread::get_id() << " "

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Creates a file with the content from the buffer
 *
 ********************************************************************************************/

void * __stdcall FileSystemWatcherThread(void * poThreadParameter)
{
    __DebugFunction();

    try
    {
        std::cout << "FileSystemWatcherThread" << std::endl;

        int nINotifyFd = ::inotify_init();
        _ThrowBaseExceptionIf((0 >= nINotifyFd), "Unable to create a inotify object", nullptr);

        // Add the directory we want to watch
        int nDirectoryToWatchFd = ::inotify_add_watch(nINotifyFd, gc_strSignalFolderName.c_str(), IN_CREATE);
        _ThrowBaseExceptionIf((-1 == nDirectoryToWatchFd), "Could not watch : *s\n", gc_strSignalFolderName.c_str());

        bool fKeepRunning = true;
        // Allocate a buffer for
        std::vector<Byte> stlNotifyEvent(sizeof(struct inotify_event) + NAME_MAX + 1);

        // Get the Job Engine Object which has the callback we'd need to call on
        // every new file creation
        JobEngine & oJobEngine = JobEngine::Get();

        while(true == fKeepRunning)
        {
            // This is a blocking call is only invoked when a file system related event is
            // is reported in the specified directory
            int nLengthOfData = ::read(nINotifyFd, stlNotifyEvent.data(), stlNotifyEvent.size());

            struct inotify_event * poInotifyEvent = (struct inotify_event *)stlNotifyEvent.data();
            if(poInotifyEvent->len && (poInotifyEvent->mask & IN_CREATE))
            {
                // For everyfile created we call a JobEngine callback function which should
                // find the most efficient way to handle such a file.
                std::cout << "FileCreateCallback for " << poInotifyEvent->name << std::endl;

                // TODO: this call is blocking, make it non-blocking.
                oJobEngine.FileCreateCallback(poInotifyEvent->name);

                if (gc_strHaltAllJobsSignalFilename == poInotifyEvent->name)
                {
                    fKeepRunning = false;
                }
            }
        }

        // Cleanup the event listener
        ::inotify_rm_watch( nINotifyFd, nDirectoryToWatchFd);
        ::close(nINotifyFd);
    }
    catch(const BaseException & oException)
    {
        std::cout << oException.GetExceptionMessage() << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return nullptr;
}

/********************************************************************************************
 *
 * @function BytesToFile
 * @brief Creates a file with the content from the buffer
 *
 ********************************************************************************************/

std::string DataConnectorGetTable(
    _in unsigned int unTableId
)
{
    __DebugFunction();

    std::string strResponse;

    Socket * poSocket =  ::ConnectToUnixDomainSocket("/tmp/{0bd8a254-49e4-4b86-b1b8-f353c18013c5}");
    StructuredBuffer oRequest;

    oRequest.PutInt8("RequestType", eGetTable);
    oRequest.PutUnsignedInt32("TableID", unTableId);

    std::vector<Byte> stlResponse = ::PutIpcTransactionAndGetResponse(poSocket, oRequest, false);

    // Release poSocket
    poSocket->Release();
    if (0 < stlResponse.size())
    {
        StructuredBuffer oResponse(stlResponse);
        strResponse = oResponse.GetString("ResponseString");
    }
    else
    {
        std::cout << "Failed to read response" << std::endl;
    }

    return strResponse;
}
