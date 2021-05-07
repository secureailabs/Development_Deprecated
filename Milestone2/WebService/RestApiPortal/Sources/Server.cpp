
/*********************************************************************************************
 *
 * @file TestRestFramework.cpp
 * @author Shabana Akhtar Baig
 * @date 06 Nov 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief
 ********************************************************************************************/

#include "RestFramework.h"
#include "ThreadManager.h"

int main()
{
    std::cout << "----------REST API PORTAL----------" << std::endl;
    Word wPort = 6200;
    // Create singleton instance of the ThreadManager
    ThreadManager * poThreadManager = ThreadManager::GetInstance();
    // RestFramework will load plugins from "SharedLibraries/RestApiPortal" directory
    RestFramework oRestFramework(wPort, "./SharedLibraries/RestApiPortal");
    // Start the server
    oRestFramework.RunServer();
    // Join all threads created by the ThreadManager
    poThreadManager->JoinAllThreads();
    
    return 0;
}
