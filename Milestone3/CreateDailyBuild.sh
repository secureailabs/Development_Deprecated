#! /bin/bash
# set -e
set -x

#Build and put stuff in the Binary folder
cd WebService/Plugins/RestApiPortal
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ../../..

cd WebService/Plugins/DatabasePortal
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ../../..

cd VirtualMachine
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ..

cd WebService
for d in $(echo ./*/); do (cd $d; make all -j; cd ..); done
cd ..

cd InternalTools/PackageWebServiceAndComputeVm/
make all -j
cd ../..

cd InternalTools/DatabaseTools/
make all -j
cd ../..

set -e

cd Binary
./PackageWebServiceAndComputeVm
cd ..
