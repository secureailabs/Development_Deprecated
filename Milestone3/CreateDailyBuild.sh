#! /bin/bash
set -e
set -x

#Build and put stuff in the Binary folder
cd VirtualMachine
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done

cd DataConnectorPythonModule
make clean
make all -j

cd ../..

cd WebService
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ..

cd WebService/Plugins/RestApiPortal
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ../../..

cd WebService/Plugins/DatabasePortal
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ../../..

set -e

cd InternalTools/PackageWebServiceAndComputeVm/
make all -j
cd ../..

cd InternalTools/DatabaseTools/
make all -j
cd ../..

cd Binary
./PackageWebServiceAndComputeVm
cd ..
