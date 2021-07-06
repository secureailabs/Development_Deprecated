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
    std::string strGuid = oGuid.ToString(eRaw);
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
    char* home;

    if(!PyArg_ParseTuple(args, "ssss", &vmID, &jobID, &fnID, &home))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strJobID(jobID);
    std::string strFNID(fnID);
    std::string strHome(home);
    
    std::vector<std::string> stlVarIDs;
    std::vector<std::vector<Byte>> stlVars;
    
    std::vector<std::string> stlConfidentialOutputIDs;

    getFrontend().GetOutputVec(strFNID, stlVarIDs);
    getFrontend().GetConfidentialOutputVec(strFNID, stlConfidentialOutputIDs);

    getFrontend().HandlePullData(strVMID, strJobID, stlVarIDs, stlVars);

    PyObject* pullable = PyList_New(stlVarIDs.size());
    PyObject* confidential = PyList_New(stlConfidentialOutputIDs.size());
    
    for(size_t i=0; i<stlVarIDs.size(); i++)
    {
        std::ofstream stlVarFile;
        
        //std::ofstream stlOutputLog;
        //stlOutputLog.open(strHome+"/"+strJobID+stlVarIDs[i]+".log" ,std::ios::out);
        
        stlVarFile.open(strHome+"/"+strJobID+stlVarIDs[i], std::ios::out | std::ios::binary);
        //stlVarFile.write((char*)&stlVars[i][0], stlVars[i].size());
        //stlOutputLog<<stlVars[i].size();
        //stlOutputLog.close();
        stlVarFile.write((char*)stlVars[i].data(), stlVars[i].size());
        stlVarFile.close();

        PyList_SetItem(pullable, i, Py_BuildValue("s", stlVarIDs[i].c_str()));
    }
    
    for(size_t i =0; i<stlConfidentialOutputIDs.size(); i++)
    {
        PyList_SetItem(confidential, i, Py_BuildValue("s", std::string(strJobID+stlConfidentialOutputIDs[i]).c_str()));
    }
    
    PyObject* result=Py_BuildValue("[OO]", pullable, confidential);
    
    return result;
}

static PyObject* pushdata(PyObject* self, PyObject* args)
{
    char* vmID;
    char* jobID;
    char* fnID;
    char* home;
    PyObject* confidentialInputList;

    if(!PyArg_ParseTuple(args, "sssOs", &vmID, &jobID, &fnID, &confidentialInputList, &home))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strJobID(jobID);
    std::string strFNID(fnID);
    std::string strHome(home);
    
    std::vector<std::string> stlInputIDs;
    std::vector<std::vector<Byte>> stlInputVars;

    getFrontend().GetInputVec(strFNID, stlInputIDs);
    
    int number = stlInputIDs.size();

    for(int i =0; i<number; i++)
    {
        std::ifstream stlVarFile;
        stlVarFile.open(strHome+"/"+strJobID+"_"+std::to_string(i), std::ios::in | std::ios::binary);
        stlVarFile.unsetf(std::ios::skipws);

        stlVarFile.seekg (0, stlVarFile.end);
        int length = stlVarFile.tellg();
        stlVarFile.seekg (0, stlVarFile.beg);

        std::vector<Byte> stlVec;
        stlVec.reserve(length);

        stlVec.insert(stlVec.begin(),std::istream_iterator<Byte>(stlVarFile), std::istream_iterator<Byte>());
        stlInputVars.push_back(stlVec);
        
        stlVarFile.close();
    }
    

    PyObject *iter = PyObject_GetIter(confidentialInputList);
    std::vector<std::string> stlConfidentialInputIDs;
    
    while (true) 
    {
        PyObject *next = PyIter_Next(iter);
        if (!next) {
            break;
        }

        std::string strID(PyUnicode_AsUTF8(next));
        stlConfidentialInputIDs.push_back(strID);
   }
    

    getFrontend().HandlePushData(strVMID, strFNID, strJobID, stlInputIDs, stlInputVars, stlConfidentialInputIDs);

    return Py_BuildValue("");
}

static PyObject* deletedata(PyObject* self, PyObject* args)
{
    char* vmID;
    PyObject* varArray;

    if(!PyArg_ParseTuple(args, "sO!", &vmID, &PyList_Type, &varArray))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    int number = PyList_Size(varArray);
    
    std::vector<std::string> stlVarArray;
    for(int i =0; i<number; i++)
    {
        PyObject* strObj = PyList_GetItem(varArray, i);
        PyObject * temp_bytes = PyUnicode_AsEncodedString(strObj, "UTF-8", "strict");
        char* varstr = PyBytes_AS_STRING(temp_bytes);
        stlVarArray.push_back(std::string(varstr));
    }

    getFrontend().HandleDeleteData(strVMID, stlVarArray);

    return Py_BuildValue("");
}

static PyObject* pushfn(PyObject* self, PyObject* args)
{
    char* vmID;
    char* fnID;

    if(!PyArg_ParseTuple(args, "ss", &vmID, &fnID))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);
    std::string strFNID(fnID);

    getFrontend().HandlePushFN(strVMID, strFNID);

    return Py_BuildValue("");
}

static PyObject* execjob(PyObject* self, PyObject* args)
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

    getFrontend().HandleExecJob(strVMID, strFNID, strJobID);

    return Py_BuildValue("");
}

static PyObject* gettableID(PyObject* self, PyObject* args)
{
    char* vmID;
    std::string strTableID;

    if(!PyArg_ParseTuple(args, "s", &vmID))
    {
        return NULL;
    }
    
    std::string strVMID(vmID);

    getFrontend().HandleGetTable(strVMID, strTableID);

    return Py_BuildValue("s", strTableID.c_str());
}

static PyObject* registerfn(PyObject* self, PyObject* args)
{
    char* file;
    int inputNumber;
    int outputNumber;
    int confidentialInputNumber;
    int confidentialOutputNumber;

    if(!PyArg_ParseTuple(args, "siiii", &file, &inputNumber, &outputNumber, &confidentialInputNumber, &confidentialOutputNumber))
    {
        return NULL;
    }

    std::string strFNID;
    std::string strFile(file);

    getFrontend().RegisterFN(strFile, inputNumber, outputNumber, confidentialInputNumber, confidentialOutputNumber, strFNID);
    
    std::vector<std::string> stlInputVec;
    std::vector<std::string> stlOutputVec;
    std::vector<std::string> stlConfidentialInputVec;
    std::vector<std::string> stlConfidentialOutputVec;

    getFrontend().GetInputVec(strFNID, stlInputVec);
    getFrontend().GetOutputVec(strFNID, stlOutputVec);
    getFrontend().GetConfidentialInputVec(strFNID, stlConfidentialInputVec);
    getFrontend().GetConfidentialOutputVec(strFNID, stlConfidentialOutputVec);

    PyObject* result = PyList_New(5);
    PyObject* inputList = PyList_New(stlInputVec.size());
    PyObject* outputList = PyList_New(stlOutputVec.size());
    PyObject* confidentialInputList = PyList_New(stlConfidentialInputVec.size());
    PyObject* confidentialOutputList = PyList_New(stlConfidentialOutputVec.size());
    
    for(size_t i=0; i<stlInputVec.size(); i++)
    {
        PyList_SetItem(inputList, i, Py_BuildValue("s", stlInputVec[i].c_str()));
    }
    for(size_t i=0; i<stlOutputVec.size(); i++)
    {
        PyList_SetItem(outputList, i, Py_BuildValue("s", stlOutputVec[i].c_str()));
    }
    for(size_t i=0; i<stlConfidentialInputVec.size(); i++)
    {
        PyList_SetItem(confidentialInputList, i, Py_BuildValue("s", stlConfidentialInputVec[i].c_str()));
    }
    for(size_t i=0; i<stlConfidentialOutputVec.size(); i++)
    {
        PyList_SetItem(confidentialOutputList, i, Py_BuildValue("s", stlConfidentialOutputVec[i].c_str()));
    }
    
    PyList_SetItem(result, 0, Py_BuildValue("s", strFNID.c_str()));
    PyList_SetItem(result, 1, inputList);
    PyList_SetItem(result, 2, outputList);
    PyList_SetItem(result, 3, confidentialInputList);
    PyList_SetItem(result, 4, confidentialOutputList);

    return result;
}

static PyObject* quit(PyObject* self, PyObject* args)
{
    getFrontend().HandleQuit();
    return Py_BuildValue("");
}

static PyMethodDef SAILAPIMethods [] =
{
    {"createguid", (PyCFunction)createguid, METH_NOARGS, NULL},
    {"connect", (PyCFunction)vmconnect, METH_VARARGS, NULL},
    {"pushdata", (PyCFunction)pushdata, METH_VARARGS, NULL},
    {"pulldata", (PyCFunction)pulldata, METH_VARARGS, NULL},
    {"deletedata", (PyCFunction)deletedata, METH_VARARGS, NULL},
    {"pushfn", (PyCFunction)pushfn, METH_VARARGS, NULL},
    {"execjob", (PyCFunction)execjob, METH_VARARGS, NULL},
    {"gettableID", (PyCFunction)gettableID, METH_VARARGS, NULL},
    {"registerfn", (PyCFunction)registerfn, METH_VARARGS, NULL},
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
