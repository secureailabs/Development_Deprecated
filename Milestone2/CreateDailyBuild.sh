#! /bin/bash

#set -e 
set -x 

foldername=$(date +'%Y-%m-%d')

rm -rf $foldername
rm -rf Binary
mkdir $foldername

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

set -e

cd InternalTools/PackageWebServiceAndComputeVm/
make all -j 
cd ../..

# Copy more stuff into the Binary folder
cp -r WebService/SharedLibraries/ Binary/
cp VirtualMachine/DataConnectorPythonModule/libDataConnector.so Binary/

cd Binary
./PackageWebServiceAndComputeVm
cd ..

# Create folder in the daily build for each 
mkdir $foldername/SecureVmDeployment
cp InternalTools/WindowsPlatformDeliverables/Binaries/x64/Debug/* $foldername/SecureVmDeployment/
cp Binary/*.binaries $foldername/SecureVmDeployment/

mkdir $foldername/DataSetSpecificationTool
cp EndPointTools/DataSetSpecification/Release_v0.1/* $foldername/DataSetSpecificationTool 

mkdir $foldername/Orchestrator
cp -r EndPointTools/ResearcherInterface/demo $foldername/Orchestrator/
cp -r EndPointTools/ResearcherInterface/sail $foldername/Orchestrator/

mkdir $foldername/AuditViewer
cp InternalTools/SailAuditViewer/Binaries/x64/Debug/* $foldername/AuditViewer

