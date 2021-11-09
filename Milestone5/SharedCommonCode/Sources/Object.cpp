/*********************************************************************************************
 *
 * @file Object.cpp
 * @author Luis Miguel Huapaya
 * @date 30 Sep 2020
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 * @brief Implementation of the base class Object which all other objects should derive from.
 * The base Object class implements thread safe reference counting semantics that all objects
 * should use.
 *
 ********************************************************************************************/

#include "Object.h"

/********************************************************************************************
 *
 * @class Object
 * @function Object
 * @brief Default constructor
 *
 ********************************************************************************************/

Object::Object(void) throw()
{
    m_nReferenceCount = 1;
}

/********************************************************************************************
 *
 * @class Object
 * @function Object
 * @brief Default copy constructor
 * @param[in] oObject Object to copy from
 * @note Even though we are copying from another object, the starting reference count is still 1
 *
 ********************************************************************************************/

Object::Object(
    _in const Object & /*oObject*/
    ) throw()
{
    m_nReferenceCount = 1;
}

/********************************************************************************************
 *
 * @class Object
 * @function ~Object
 * @brief Default destructor
 *
 ********************************************************************************************/

Object::~Object(void) throw()
{
}

/********************************************************************************************
 *
 * @class Object
 * @function AddRef
 * @brief Adds one reference count to the object
 *
 ********************************************************************************************/

int __thiscall Object::AddRef(void) throw()
{
    return (int) ::__sync_add_and_fetch((int *) &m_nReferenceCount, 1);
}

/********************************************************************************************
 *
 * @class Object
 * @function Release
 * @brief Removes one reference count to the object. If the reference count reaches 0, deletes this object
 *
 ********************************************************************************************/

int __thiscall Object::Release(void) throw()
{
    int nReferenceCount;

    nReferenceCount = (int) ::__sync_sub_and_fetch((int *) &m_nReferenceCount, 1);
    if (0 == nReferenceCount)
    {
        delete this;
    }

    return nReferenceCount;
}

/********************************************************************************************
 *
 * @class Object
 * @function GetRef
 * @brief Returns a pointer to the current object and increments the reference count automatically
 *
 ********************************************************************************************/

Object * __thiscall Object::GetRef(void) throw()
{
    this->AddRef();

    return this;
}

