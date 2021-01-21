
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
    std::cout << "----------TEST REST API FRAMEWORK----------" << std::endl;
    Word wPort = 6200;
    RestFramework oRestFramework(wPort, "../WebService/SharedLibraries");
    oRestFramework.RunServer();
    return 0;
}
