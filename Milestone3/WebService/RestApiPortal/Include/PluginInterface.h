/*********************************************************************************************
 *
 * @file PluginInterface.h
 * @author Shabana Akhtar Baig
 * @date 21 Oct 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 * All REST API Plugins should be derived from PluginInterface class.
 ********************************************************************************************/

#pragma once

#include "RestFramework.h"

class PluginInterface : public Object
{
    public:

        virtual void InitializePlugin(void) = 0;

        virtual uint64_t SubmitRequest(
            _in const StructuredBuffer & c_oRequestStructuredBuffer,
            _out unsigned int * punSerializedResponseSizeInBytes
            ) = 0;

        virtual bool GetResponse(
            _in u_int64_t un64Identifier,
            _out Byte * pbSerializedResponseBuffer,
            _in unsigned int unSerializedResponseBufferSizeInBytes
            ) = 0;
};
