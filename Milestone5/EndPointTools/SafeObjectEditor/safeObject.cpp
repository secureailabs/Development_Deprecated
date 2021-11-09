#include "Python.h"
#include "Guid.h"
#include "StructuredBuffer.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>

static PyObject* createguid(PyObject* self, PyObject* args)
{
    Guid oGuid;
    std::string strGuid = oGuid.ToString(eRaw);
    return Py_BuildValue("s", strGuid.c_str());
}

static void parameterConvert(StructuredBuffer& oBuffer, PyObject* parameter)
{
    Py_ssize_t listLen = PyList_Size(parameter);

    for(Py_ssize_t i = 0; i<listLen; i++)
    {
        std::cout<<i<<std::endl;
        PyObject* item = PyList_GetItem(parameter, i);
        StructuredBuffer oItem;

        PyObject* paramGuid = PyList_GetItem(item, 0);
        const char* guid = PyUnicode_AsUTF8(paramGuid);
        std::string strGuid(guid);
        oItem.PutString("Uuid", strGuid);
        std::cout<<"guid: "<<strGuid<<std::endl;

        PyObject* paramType = PyList_GetItem(item, 2);
        const char* type = PyUnicode_AsUTF8(paramType);
        std::string strType(type);
        oItem.PutString("Type", strType);
        std::cout<<"type: "<<strType<<std::endl;

        PyObject* paramDes = PyList_GetItem(item, 3);
        const char* description = PyUnicode_AsUTF8(paramDes);
        std::string strDescription(description);
        oItem.PutString("Description", strDescription);
        std::cout<<"Description: "<<strDescription<<std::endl;
        
        PyObject* paramConf = PyList_GetItem(item, 4);
        const char* confidentiality = PyUnicode_AsUTF8(paramConf);
        std::string strConfidentiality(confidentiality);
        oItem.PutString("confidentiality", strConfidentiality);
        std::cout<<"confidentiality: "<<strConfidentiality<<std::endl;

        std::cout<<std::to_string((int)i).c_str()<<std::endl;
        oBuffer.PutStructuredBuffer(std::to_string((int)i).c_str(), oItem);
    }
}

static PyObject* writeSafeObject(PyObject* self, PyObject* args)
{
    StructuredBuffer oResult;
    PyObject* dict;
    if(!PyArg_ParseTuple(args, "O", &dict))
    {
        return NULL;
    }

    PyObject* title = PyDict_GetItemString(dict, "Title");
    const char* sTitle = PyUnicode_AsUTF8(title);
    std::string strTitle(sTitle);
    oResult.PutString("Title", strTitle);

    PyObject* uuid = PyDict_GetItemString(dict, "uuid");
    const char* sUuid = PyUnicode_AsUTF8(uuid);
    std::string strUuid(sUuid);
    oResult.PutString("Uuid", strUuid);
    std::cout<<"uuid"<<strUuid<<std::endl;

    PyObject* description = PyDict_GetItemString(dict, "Description");
    const char* sDes = PyUnicode_AsUTF8(description);
    std::string strDes(sDes);
    oResult.PutString("Description", strDes);
    std::cout<<"description"<<strDes<<std::endl;

    PyObject* inputList = PyDict_GetItemString(dict, "input");
    PyObject* outputList = PyDict_GetItemString(dict, "output");
    StructuredBuffer oInput;
    StructuredBuffer oOutput;
    std::cout<<"no problem here"<<std::endl;
    parameterConvert(oInput, inputList);
    parameterConvert(oOutput, outputList);
    std::cout<<"0:"<<oInput.GetStructuredBuffer("0").GetString("Uuid")<<std::endl;
    oResult.PutStructuredBuffer("InputParameters", oInput);
    oResult.PutStructuredBuffer("OutputParameters", oOutput);
    std::cout<<"no problem here 2"<<std::endl;

    PyObject* body = PyDict_GetItemString(dict, "body");
    const char* sBody = PyUnicode_AsUTF8(body);
    std::string strBody(sBody);
    oResult.PutString("Payload", strBody);
    std::cout<<"body"<<std::endl;

    std::ofstream stlFileStream;
    std::string strFname = strUuid + ".safe";
    std::cout<<strFname<<std::endl;
    std::vector<Byte> stlFileData = oResult.GetSerializedBuffer();
    stlFileStream.open(strFname, std::ios::out | std::ofstream::binary);
    std::copy(stlFileData.begin(), stlFileData.end(), std::ostreambuf_iterator<char>(stlFileStream));
    stlFileStream.close();

    return Py_BuildValue("");
}

static PyMethodDef SafeObjAPIMethods [] =
{
    {"newguid", (PyCFunction)createguid, METH_NOARGS, NULL},
    {"writeSafeObject", (PyCFunction)writeSafeObject, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef SafeObjAPIModule =
{
    PyModuleDef_HEAD_INIT,
    "SafeObjectAPI",
    NULL,
    -1,
    SafeObjAPIMethods
};

PyMODINIT_FUNC PyInit_SafeObjectAPI(void){
    return PyModule_Create(&SafeObjAPIModule);
}
