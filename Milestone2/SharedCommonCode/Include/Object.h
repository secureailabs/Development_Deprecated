/*********************************************************************************************
 *
 * @file Object.h
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
 ********************************************************************************************/

#pragma once

#include "CoreTypes.h"

/********************************************************************************************/

class Object
{
    public:

        Object(void) throw();
        Object(
            _in const Object & oObject
            ) throw();
        virtual ~Object(void) throw();

        int __thiscall AddRef(void) throw();
        int __thiscall Release(void) throw();
        Object * __thiscall GetRef(void) throw();

    private:

        mutable int m_nReferenceCount;	
};