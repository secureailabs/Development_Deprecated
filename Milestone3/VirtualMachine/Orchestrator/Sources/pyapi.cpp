#include <Python.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
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

static PyObject* login(PyObject* self, PyObject* args)
{
    char* email;
    char* password;
    int serverPort;
    char* serverIP;
    
    if(!PyArg_ParseTuple(args, "ssis", &email, &password, &serverPort, &serverIP))
    {
        return NULL;
    }
    
    std::string strEmail(email);
    std::string strPassword(password);
    //std::string strServerPort(serverPort);
    std::string strServerIP(serverIP);
    
    std::string strEOSB = getFrontend().Login(strEmail, strPassword, serverPort, strServerIP);
    
    return Py_BuildValue("s", strEOSB.c_str());
}

static PyObject* vmconnect(PyObject* self, PyObject* args)
{
    char* serverIP;
    unsigned int port;
    std::string strVMID;

    if(!PyArg_ParseTuple(args, "sI", &serverIP, &port))
    {
        return NULL;
    }

    std::string strIP(serverIP);
    getFrontend().SetFrontend(strIP, port, strVMID);

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
        std::string strParam = tmpParam;
        std::cout<<"set param: "<<strParam<<std::endl;
        stlParams.push_back(strParam);
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
    int nStatus;
    
    switch(oJobStatus)
    {
        case JobStatusSignals::eJobStart: 
            nStatus = 0;
            break;
        case JobStatusSignals::eJobDone: 
            nStatus = 1;
            break;
        case JobStatusSignals::eJobFail: 
            nStatus = -1;
            break;
        case JobStatusSignals::ePrivacyViolation: 
            nStatus = -2;
            break;
        default:
            break;
    }
    
    return Py_BuildValue("i", nStatus);
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
    std::vector<std::string> stlOutput;
    std::vector<int> stlOutputLabel;

    getFrontend().QueryResult(strJobID, strFNID, stlOutput, stlOutputLabel);
    
    PyObject* output = PyList_New(0);
    
    for(size_t i=0;i<stlOutput.size();i++)
    {
        PyObject* tmp = PyList_New(0);
        PyObject* value = Py_BuildValue("i", stlOutputLabel[i]);
        PyList_Append(tmp, value);
        PyObject* id = Py_BuildValue("s", stlOutput[i].c_str());
        PyList_Append(tmp, id);
        PyList_Append(output, tmp);
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

    std::map<std::string, std::string> stlDataTable = getFrontend().QueryDataset(strVMID);
    
    PyObject* output = PyDict_New();
    
    for(auto const& x : stlDataTable)
    {
        PyObject* value = Py_BuildValue("s", x.second.c_str());
        PyDict_SetItemString(output, x.first.c_str(), value);
    }
    //PyObject* output = PyList_New(stlDataTableIDs.size());

    //for(size_t i=0;i<stlDataTableIDs.size();i++)
    //{
    //    const char* tmpdata = stlDataTableIDs[i].c_str();
    //    PyList_SetItem(output, i, Py_BuildValue("s", tmpdata));
    //}
    return Py_BuildValue("O", output);
}

static PyMethodDef SAILAPIMethods [] =
{
    {"createguid", (PyCFunction)createguid, METH_NOARGS, NULL},
    {"login", (PyCFunction)login, METH_VARARGS, NULL},
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
