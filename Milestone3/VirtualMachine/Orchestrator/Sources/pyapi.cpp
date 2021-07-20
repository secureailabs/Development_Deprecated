#include <Python.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include "Guid.h"
#include "frontend.h"

static Frontend& getFrontend()
{
    
    static Frontend oFrontend = Frontend();
    return oFrontend;
}

static PyObject* createguid(PyObject* self, PyObject* args)
{
    Guid oGuid;
    std::string strGuid = oGuid.ToString(eHyphensAndCurlyBraces);
    return Py_BuildValue("s", strGuid.c_str());
}

static PyObject* vmconnect(PyObject* self, PyObject* args)
{
    char* serverIP;
    char* email;
    char* password;
    unsigned int port;
    std::string strVMID;

    if(!PyArg_ParseTuple(args, "sIss", &serverIP, &port, &email, &password))
    {
        return NULL;
    }

    std::string strIP(serverIP);
    std::string strEmail(email);
    std::string strPassword(password);
    getFrontend().SetFrontend(strIP, port, strVMID, strEmail, strPassword);

    return Py_BuildValue("s", strVMID.c_str());
}

static PyObject* pulldata(PyObject* self, PyObject* args)
{
    char* vmID;
    char* jobID;
    char* fnID;

    if(!PyArg_ParseTuple(args, "sss", &vmID, &jobID, &fnID))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strJobID(jobID);
    std::string strFNID(fnID);
    //std::vector<std::vector<Byte>> stlOutputList;

    getFrontend().HandlePullData(strVMID, strJobID, strFNID);

    // PyObject* oOutput = PyList_New(stlOutputList.size());
    
    // for(size_t i =0; i<stlOutputList.size(); i++)
    // {
    //     void* tmpdata = stlOutputList[i].data();
    //     PyList_SetItem(oOutput, i, Py_BuildValue("y#", tmpdata, stlOutputList[i].size()));
    // }
    
    // return oOutput;
    return Py_BuildValue("");
}

static PyObject* pushdata(PyObject* self, PyObject* args)
{
    char* vmID;
    PyObject* InputId;
    PyObject* InputList;

    if(!PyArg_ParseTuple(args, "sOO", &vmID, &InputId, &InputList))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);


    PyObject *iter = PyObject_GetIter(InputList);
    std::vector<std::vector<Byte>> stlInputs;
    
    while (true) 
    {
        char* tmpInputs;
        long int len;

        PyObject *next = PyIter_Next(iter);
        if (!next) {
            break;
        }

        PyBytes_AsStringAndSize(next, &tmpInputs, &len);

        std::vector<Byte> stlByteElement((Byte*)tmpInputs, (Byte*)tmpInputs+len);
        stlInputs.push_back(stlByteElement);
   }

    iter = PyObject_GetIter(InputId);
    std::vector<std::string> stlInputId;

    while (true) 
    {
        PyObject *next = PyIter_Next(iter);
        if (!next) {
            break;
        }

        const char* tmpParam = PyUnicode_AsUTF8(next);
        stlInputId.push_back(std::string(tmpParam));
    }

    getFrontend().HandlePushData(strVMID, stlInputId, stlInputs);

    return Py_BuildValue("");
}

static PyObject* setparameter(PyObject* self, PyObject* args)
{
    char* vmID;
    char* jobID;
    char* fnID;
    PyObject* ParamsList;

    if(!PyArg_ParseTuple(args, "sssO", &vmID, &jobID, &fnID, &ParamsList))
    {
        return NULL;
    }

    std::string strVMID(vmID);
    std::string strJobID(jobID);
    std::string strFNID(fnID);

    PyObject *iter = PyObject_GetIter(ParamsList);
    std::vector<std::string> stlParams;

    while (true) 
    {
        PyObject *next = PyIter_Next(iter);
        if (!next) {
            break;
        }

        const char* tmpParam = PyUnicode_AsUTF8(next);
        stlParams.push_back(std::string(tmpParam));
    }

    getFrontend().HandleSetParameters(strVMID, strFNID, strJobID, stlParams);
    return Py_BuildValue("");
}

// static PyObject* deletedata(PyObject* self, PyObject* args)
// {
//     char* vmID;
//     PyObject* varArray;

//     if(!PyArg_ParseTuple(args, "sO!", &vmID, &PyList_Type, &varArray))
//     {
//         return NULL;
//     }
    
//     std::string strVMID(vmID);
//     int number = PyList_Size(varArray);
    
//     std::vector<std::string> stlVarArray;
//     for(int i =0; i<number; i++)
//     {
//         PyObject* strObj = PyList_GetItem(varArray, i);
//         PyObject * temp_bytes = PyUnicode_AsEncodedString(strObj, "UTF-8", "strict");
//         char* varstr = PyBytes_AS_STRING(temp_bytes);
//         stlVarArray.push_back(std::string(varstr));
//     }

//     getFrontend().HandleDeleteData(strVMID, stlVarArray);

//     return Py_BuildValue("");
// }

static PyObject* pushsafeobj(PyObject* self, PyObject* args)
{
    char* vmID;
    char* fnID;

    if(!PyArg_ParseTuple(args, "ss", &vmID, &fnID))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strFNID(fnID);

    getFrontend().HandlePushSafeObject(strVMID, strFNID);

    return Py_BuildValue("");
}

static PyObject* submitjob(PyObject* self, PyObject* args)
{
    char* vmID;
    char* fnID;
    char* jobID;

    if(!PyArg_ParseTuple(args, "sss", &vmID, &fnID, &jobID))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strFNID(fnID);
    std::string strJobID(jobID);

    getFrontend().HandleSubmitJob(strVMID, strFNID, strJobID);

    return Py_BuildValue("");
}

// static PyObject* gettableID(PyObject* self, PyObject* args)
// {
//     char* vmID;
//     std::string strTableID;

//     if(!PyArg_ParseTuple(args, "s", &vmID))
//     {
//         return NULL;
//     }
    
//     std::string strVMID(vmID);

//     getFrontend().HandleGetTable(strVMID, strTableID);

//     return Py_BuildValue("s", strTableID.c_str());
// }
static PyObject* queryjobstatus(PyObject* self, PyObject* args)
{
    char* jobid;

    if(!PyArg_ParseTuple(args, "s", &jobid))
    {
        return NULL;
    }

    std::string strJobID(jobid);

    JobStatusSignals oJobStatus = getFrontend().QueryJobStatus(strJobID);
    std::string strStatus;
    
    switch(oJobStatus)
    {
        case JobStatusSignals::eJobStart: 
            strStatus = "Job is running";
            break;
        case JobStatusSignals::eJobDone: 
            strStatus = "Job is done";
            break;
        case JobStatusSignals::eJobFail: 
            strStatus = "Job is failed";
            break;
        default:
            break;
    }
    
    return Py_BuildValue("s", strStatus.c_str());
}

static PyObject* registersafeobj(PyObject* self, PyObject* args)
{
    char* file;

    if(!PyArg_ParseTuple(args, "s", &file))
    {
        return NULL;
    }

    std::string strFile(file);

    getFrontend().RegisterSafeObject(strFile);

    return Py_BuildValue("");
}

static PyObject* quit(PyObject* self, PyObject* args)
{
    getFrontend().HandleQuit();
    return Py_BuildValue("");
}

static PyObject* queryresult(PyObject* self, PyObject* args)
{
    char* jobid;
    char* fnid;

    if(!PyArg_ParseTuple(args, "ss", &jobid, &fnid))
    {
        return NULL;
    }

    std::string strJobID(jobid);
    std::string strFNID(fnid);
    std::vector<std::vector<Byte>> stlOutput;

    getFrontend().QueryResult(strJobID, strFNID, stlOutput);

    PyObject* output = PyList_New(stlOutput.size());

    for(size_t i=0;i<stlOutput.size();i++)
    {
        Byte* tmpdata = stlOutput[i].data();
        PyList_SetItem(output, i, Py_BuildValue("y#", tmpdata, stlOutput[i].size()));
    }

    return Py_BuildValue("O", output);
}

static PyObject* querydata(PyObject* self, PyObject* args)
{
    char* vmid;

    if(!PyArg_ParseTuple(args, "s", &vmid))
    {
        return NULL;
    }

    std::string strVMID(vmid);

    std::vector<std::string> stlDataTableIDs = getFrontend().QueryDataset(strVMID);

    PyObject* output = PyList_New(stlDataTableIDs.size());

    for(size_t i=0;i<stlDataTableIDs.size();i++)
    {
        const char* tmpdata = stlDataTableIDs[i].c_str();
        PyList_SetItem(output, i, Py_BuildValue("s", tmpdata));
    }

    return Py_BuildValue("O", output);
}

static PyMethodDef SAILAPIMethods [] =
{
    {"createguid", (PyCFunction)createguid, METH_NOARGS, NULL},
    {"connect", (PyCFunction)vmconnect, METH_VARARGS, NULL},
    {"pushdata", (PyCFunction)pushdata, METH_VARARGS, NULL},
    {"pulldata", (PyCFunction)pulldata, METH_VARARGS, NULL},
    // {"deletedata", (PyCFunction)deletedata, METH_VARARGS, NULL},
    {"pushsafeobj", (PyCFunction)pushsafeobj, METH_VARARGS, NULL},
    {"submitjob", (PyCFunction)submitjob, METH_VARARGS, NULL},
    // {"gettableID", (PyCFunction)gettableID, METH_VARARGS, NULL},
    {"registersafeobj", (PyCFunction)registersafeobj, METH_VARARGS, NULL},
    {"queryresult", (PyCFunction)queryresult, METH_VARARGS, NULL},
    {"querydata", (PyCFunction)querydata, METH_VARARGS, NULL},
    {"queryjobstatus", (PyCFunction)queryjobstatus, METH_VARARGS, NULL},
    {"setparameter", (PyCFunction)setparameter, METH_VARARGS, NULL},
    {"quit", (PyCFunction)quit, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef SAILPyAPIModule =
{
    PyModuleDef_HEAD_INIT,
    "SAILPyAPI",
    NULL,
    -1,
    SAILAPIMethods
};

PyMODINIT_FUNC PyInit_SAILPyAPI(void){
    return PyModule_Create(&SAILPyAPIModule);
}
