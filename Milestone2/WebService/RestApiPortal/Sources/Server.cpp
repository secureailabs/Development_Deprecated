
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

int main()
{
    std::cout << "----------REST API PORTAL----------" << std::endl;
    Word wPort = 6200;
    RestFramework oRestFramework(wPort, "../WebService/SharedLibraries/RestApiPortal");
    oRestFramework.RunServer();
    return 0;
}
