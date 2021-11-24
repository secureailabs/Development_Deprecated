/*********************************************************************************************
 * @file pyStructuredBuffer.cpp
 * @author Prawal Gangwar
 * @date 17 Nov 2021
 * @License Private and Confidential. Internal Use Only.
 * @copyright Copyright (C) 2020 Secure AI Labs, Inc. All Rights Reserved.
 *
*********************************************************************************************/

#include "StructuredBuffer.h"
#include "DebugLibrary.h"
#include "FileUtils.h"

#include <iostream>
#include <string>
#include <vector>

#include <Python.h>

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GenerateParamterGuid(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    Guid oGuid;

    return Py_BuildValue("s", oGuid.ToString(eRaw).c_str());
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *StructuredBufferInit(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    StructuredBuffer * poStructuredBuffer = new StructuredBuffer();
    std::cout << "StructuredBuffer init" << std::endl;

    return Py_BuildValue("K", (void *)poStructuredBuffer);
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *PutString(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    char * value;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "Kss", &poStructuredBuffer, &szElementName, &value))
    {
        return nullptr;
    }
    poStructuredBuffer->PutString(szElementName, value);
    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetString(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &szElementName))
    {
        return nullptr;
    }

    return Py_BuildValue("s", poStructuredBuffer->GetString(szElementName).c_str());
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *PutNull(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &szElementName))
    {
        return nullptr;
    }

    poStructuredBuffer->PutNull(szElementName);
    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *PutFloat64(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    double f64Value;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ksd", &poStructuredBuffer, &szElementName, &f64Value))
    {
        return nullptr;
    }
    poStructuredBuffer->PutFloat64(szElementName, f64Value);

    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetFloat64(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &szElementName))
    {
        return nullptr;
    }

    return Py_BuildValue("d", poStructuredBuffer->GetFloat64(szElementName));
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *PutBoolean(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    int fValue;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ksp", &poStructuredBuffer, &szElementName, &fValue))
    {
        return nullptr;
    }
    std::cout << "PutBool" << szElementName << std::endl;
    std::cout << "PutBool" << fValue << std::endl;

    fValue = (fValue != 0);
    poStructuredBuffer->PutBoolean(szElementName, fValue);

    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetBoolean(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &szElementName))
    {
        return nullptr;
    }

    if (poStructuredBuffer->GetBoolean(szElementName))
    {
        return Py_True;
    }
    else
    {
        return Py_False;
    }
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *PutStructuredBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    StructuredBuffer * poStructuredBufferToPut = nullptr;
    if (!PyArg_ParseTuple(poPyObjectArguments, "KsK", &poStructuredBuffer, &szElementName, &poStructuredBufferToPut))
    {
        return nullptr;
    }

    poStructuredBuffer->PutStructuredBuffer(szElementName, *poStructuredBufferToPut);
    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetStructuredBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szElementName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if (!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &szElementName))
    {
        return nullptr;
    }

    StructuredBuffer * poStructuredBufferToGet = new StructuredBuffer(poStructuredBuffer->GetStructuredBuffer(szElementName));
    return Py_BuildValue("K", poStructuredBufferToGet);
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *ToString(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "K", &poStructuredBuffer))
    {
        return nullptr;
    }

    return Py_BuildValue("s", poStructuredBuffer->ToString().c_str());
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *Delete(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "K", &poStructuredBuffer))
    {
        return nullptr;
    }

    if (nullptr != poStructuredBuffer)
    {
        poStructuredBuffer->Release();
    }
    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *DumpStructuredBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * pszFileName;
    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "Ks", &poStructuredBuffer, &pszFileName))
    {
        return nullptr;
    }

    std::vector<Byte> stlSerializedStructuredBuffer = poStructuredBuffer->GetSerializedBuffer();
    ::WriteBytesAsFile(pszFileName, stlSerializedStructuredBuffer);

    return Py_BuildValue("");
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *LoadStructuredBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    char * szFilename;
    if(!PyArg_ParseTuple(poPyObjectArguments, "s", &szFilename))
    {
        return nullptr;
    }

    std::vector<Byte> stlSerializedStructuredBuffer = ::ReadFileAsByteBuffer(szFilename);
    StructuredBuffer * poStructuredBuffer = new StructuredBuffer(stlSerializedStructuredBuffer);

    return Py_BuildValue("K", (void *)poStructuredBuffer);
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetDescriptionOfElements(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "K", &poStructuredBuffer))
    {
        return nullptr;
    }

    std::vector<std::string> stlDescriptionOfElements = poStructuredBuffer->GetDescriptionOfElements();
    PyObject * poPyList = PyList_New(stlDescriptionOfElements.size());
    for(size_t i = 0; i < stlDescriptionOfElements.size(); i++)
    {
        PyList_SetItem(poPyList, i, Py_BuildValue("s", stlDescriptionOfElements[i].c_str()));
    }

    return poPyList;
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetElementTypeStringFromInt(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    Dword dwType = 0;
    if(!PyArg_ParseTuple(poPyObjectArguments, "K", &dwType))
    {
        return nullptr;
    }

    return Py_BuildValue("s", gs_aszTypeNames[dwType]);
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *GetSerializedStructuredBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    StructuredBuffer * poStructuredBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "K", &poStructuredBuffer))
    {
        return nullptr;
    }

    std::vector<Byte> stlSerailizedBuffer = poStructuredBuffer->GetSerializedBuffer();

    return PyBytes_FromStringAndSize((const char *)stlSerailizedBuffer.data(), stlSerailizedBuffer.size());
}

/********************************************************************************************
 *
 * @function GenerateParamterGuid
 * @brief Function to generate a GUID for the parameter
 * @return a guid
 * @throw BaseException
 *
 ********************************************************************************************/
static PyObject *FromSerializedBuffer(
    _in PyObject *poSelfObject,
    _in PyObject *poPyObjectArguments
)
{
    __DebugFunction();

    PyObject * poPyBuffer = nullptr;
    if(!PyArg_ParseTuple(poPyObjectArguments, "O", &poPyBuffer))
    {
        return nullptr;
    }

    Py_buffer pyBuffer;
    if(PyObject_GetBuffer(poPyBuffer, &pyBuffer, PyBUF_SIMPLE) != 0)
    {
        return nullptr;
    }

    std::vector<Byte> stlSerializedBuffer;
    stlSerializedBuffer.resize(pyBuffer.len);
    memcpy(stlSerializedBuffer.data(), pyBuffer.buf, pyBuffer.len);
    PyBuffer_Release(&pyBuffer);

    StructuredBuffer * poStructuredBuffer = new StructuredBuffer(stlSerializedBuffer);
    return Py_BuildValue("K", (void *)poStructuredBuffer);
}

static PyMethodDef PyStructuredBufferMethods [] =
{
    // StructuredBuffer Get/Put Functions
    {"Init", (PyCFunction)StructuredBufferInit, METH_NOARGS, NULL},
    {"PutString", (PyCFunction)PutString, METH_VARARGS, NULL},
    {"GetString", (PyCFunction)GetString, METH_VARARGS, NULL},
    {"PutNull", (PyCFunction)PutNull, METH_VARARGS, NULL},
    {"PutFloat64", (PyCFunction)PutFloat64, METH_VARARGS, NULL},
    {"GetFloat64", (PyCFunction)GetFloat64, METH_VARARGS, NULL},
    {"PutBoolean", (PyCFunction)PutBoolean, METH_VARARGS, NULL},
    {"GetBoolean", (PyCFunction)GetBoolean, METH_VARARGS, NULL},
    {"PutStructuredBuffer", (PyCFunction)PutStructuredBuffer, METH_VARARGS, NULL},
    {"GetStructuredBuffer", (PyCFunction)GetStructuredBuffer, METH_VARARGS, NULL},

    // StructuredBuffer Helper functions
    {"ToString", (PyCFunction)ToString, METH_VARARGS, NULL},
    {"Delete", (PyCFunction)Delete, METH_VARARGS, NULL},
    {"LoadStructuredBuffer", (PyCFunction)LoadStructuredBuffer, METH_VARARGS, NULL},
    {"DumpStructuredBuffer", (PyCFunction)DumpStructuredBuffer, METH_VARARGS, NULL},
    {"GetDescriptionOfElements", (PyCFunction)GetDescriptionOfElements, METH_VARARGS, NULL},
    {"GetElementTypeStringFromInt", (PyCFunction)GetElementTypeStringFromInt, METH_VARARGS, NULL},
    {"GetSerializedStructuredBuffer", (PyCFunction)GetSerializedStructuredBuffer, METH_VARARGS, NULL},
    {"FromSerializedBuffer", (PyCFunction)FromSerializedBuffer, METH_VARARGS, NULL},

    {"GenerateParamterGuid", (PyCFunction)GenerateParamterGuid, METH_NOARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef PyStructuredBufferModule =
{
    PyModuleDef_HEAD_INIT,
    "PyStructuredBuffer",
    NULL,
    -1,
    PyStructuredBufferMethods
};

PyMODINIT_FUNC PyInit_PyStructuredBuffer(void)
{
    return PyModule_Create(&PyStructuredBufferModule);
}