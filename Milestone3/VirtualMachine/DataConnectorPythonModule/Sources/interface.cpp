//#include "function.h"
#include <Python.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "SocketClient.h"
#include "StructuredBuffer.h"
#include "DataConnector.h"
#include "IpcTransactionHelperFunctions.h"

static PyObject* ReadBuffer(PyObject* self, PyObject* args)
{
    int tableID;

    if(!PyArg_ParseTuple(args, "i", &tableID))
    {
        return NULL;
    }

    Socket * poSocket =  ConnectToUnixDomainSocket("/tmp/{0bd8a254-49e4-4b86-b1b8-f353c18013c5}");
    StructuredBuffer oRequest;

    oRequest.PutInt8("RequestType", eGetTable);
    oRequest.PutUnsignedInt32("TableID", tableID);

    std::string response;

    std::vector<Byte> stlResponse = ::PutIpcTransactionAndGetResponse(poSocket, oRequest);
    // Release poSocket
    poSocket->Release();
    if (0 < stlResponse.size())
    {
        StructuredBuffer oResponse(stlResponse);
        response = oResponse.GetString("ResponseString");
    }
    else
    {
        std::cout << "Failed to read response" << std::endl;
    }

    return Py_BuildValue("s", response.c_str());
}

static PyMethodDef DataConnectorMethods [] =
{
    {"ReadBuffer", (PyCFunction)ReadBuffer, METH_VARARGS, PyDoc_STR("read file from data connector into buffer")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef DataConnectorModule =
{
    PyModuleDef_HEAD_INIT,
    "_DataConnector",
    NULL,
    -1,
    DataConnectorMethods
};

PyMODINIT_FUNC PyInit__DataConnector(void){
    return PyModule_Create(&DataConnectorModule);
}
