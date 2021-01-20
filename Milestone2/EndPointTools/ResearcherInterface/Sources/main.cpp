#include "frontendCLI.h"

#include <Python.h>
#include <stdio.h>

void test(){
    wchar_t* argv[1];
    const char* arg = "sail_logistic_regression.logistic_regression(0, [0,1,2,3], 4)";

    argv[0] = Py_DecodeLocale(arg, NULL);

    FILE* fp = fopen("/usr/local/lib/python3.8/site-packages/scripts/main.py", "r+");
    Py_Initialize();

    PySys_SetArgv(1, argv);
    PyRun_SimpleFile(fp, "main.py");

    Py_Finalize();
}

int main(){

    //test();
    EngineCLI oEngineCLI("test", "127.0.0.1", 7000);
    oEngineCLI.CLImain();
}
