#! /bin/bash
# set -e
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
set -x

#Build and put stuff in the Binary folder
cd $SCRIPT_DIR/WebService/Plugins/RestApiPortal
for d in $(echo ./*/);do (cd $d; make all -j && returncode=$? || returncode=$?; cd ..); done

cd $SCRIPT_DIR/WebService/Plugins/DatabasePortal
for d in $(echo ./*/);do (cd $d; make all -j && returncode=$? || returncode=$?; cd ..); done

cd $SCRIPT_DIR/VirtualMachine
for d in $(echo ./*/);do (cd $d; make all -j && returncode=$? || returncode=$?; cd ..); done

cd $SCRIPT_DIR/WebService
for d in $(echo ./*/);do (cd $d; make all -j && returncode=$? || returncode=$?; cd ..); done

cd $SCRIPT_DIR/InternalTools/PackageWebServiceAndComputeVm/
make all -j && returncode=$? || returncode=$?

cd $SCRIPT_DIR/InternalTools/DatabaseTools/
make all -j && returncode=$? || returncode=$?

set -e

cd $SCRIPT_DIR/Binary
./PackageWebServiceAndComputeVm

cd $SCRIPT_DIR
