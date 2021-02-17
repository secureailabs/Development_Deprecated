#include "frontend.h"
#include "Guid.h"
#include <vector>
#include <fstream>
#include <iterator>
#include <string>
#include <Python.h>
#include <stdio.h>

void prepare(std::vector<std::vector<Byte>>& stlVars)
{
	int number = 2;

	for(int i =0; i<number; i++)
	{
	        std::ifstream stlVarFile;
	        stlVarFile.open("/home/jjj/"+std::to_string(i+1)+".pkl", std::ios::in | std::ios::binary);
	        stlVarFile.unsetf(std::ios::skipws);

	        stlVarFile.seekg (0, stlVarFile.end);
	        int length = stlVarFile.tellg();
	        stlVarFile.seekg (0, stlVarFile.beg);

	        std::vector<Byte> stlVec;
	        stlVec.reserve(length);

	        stlVec.insert(stlVec.begin(),std::istream_iterator<Byte>(stlVarFile), std::istream_iterator<Byte>());
	        stlVars.push_back(stlVec);

	        stlVarFile.close();
	}
}

void unpack(std::vector<std::vector<Byte>>& stlVars)
{
	size_t number =1;
	for(size_t i =0; i<number; i++)
	    {
	        std::ofstream stlVarFile;
	        stlVarFile.open("/home/jjj/out.pkl", std::ios::out | std::ios::binary);
	        stlVarFile.write((char*)&stlVars[i][0], stlVars[i].size());
	        stlVarFile.close();
	    }
}

int main(){

    //test();
    Frontend oFrontend;
    std::string strVMID;
    std::string strFNID;
    std::string strIP("127.0.0.1");
    oFrontend.SetFrontend(strIP, 5000, strVMID);

    oFrontend.RegisterFN("/home/jjj/playground/test.py", 2, 1, strFNID);

    oFrontend.HandlePushFN(strVMID, strFNID);

    std::string strJobID = Guid().ToString(eRaw);
    std::vector<std::string> stlInput;
    std::vector<std::string> stlOutput;
    std::vector<std::vector<Byte>> stlInputVars;
    std::vector<std::vector<Byte>> stlOutputVars;
    oFrontend.GetInputVec(strFNID, stlInput);
    oFrontend.GetOutputVec(strFNID, stlOutput);
    prepare(stlInputVars);
    oFrontend.HandlePushData(strVMID, strJobID, stlInput, stlInputVars);

    oFrontend.HandleExecJob(strVMID, strFNID, strJobID);
    oFrontend.HandlePullData(strVMID, strJobID, stlOutput, stlOutputVars);
    unpack(stlOutputVars);
}
