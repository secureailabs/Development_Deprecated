#! /bin/bash
# set -e
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
set -x

#Build and put stuff in the Binary folder
cd $SCRIPT_DIR/WebService/Plugins/RestApiPortal
echo ./*/
for d in ./*/; do (cd $d; make all -j; retVal=$?); if [ ${retVal} -ne 0 ]; then echo "Error"; exit $retVal; fi; (cd ..) done

cd $SCRIPT_DIR/WebService/Plugins/DatabasePortal
for d in ./*/; do (cd $d; make all -j; retVal=$?); if [ ${retVal} -ne 0 ]; then echo "Error"; exit $retVal; fi; (cd ..) done

cd $SCRIPT_DIR/VirtualMachine
for d in ./*/; do (cd $d; make all -j; retVal=$?); if [ ${retVal} -ne 0 ]; then echo "Error"; exit $retVal; fi; (cd ..) done

cd $SCRIPT_DIR/WebService
# for d in ./*/; do (cd $d; make all -j; retVal=$?; echo 'exit code' && echo ${retVal}); if [ ${retVal} -ne 0 ]; then echo "Error"; exit $retVal; fi; (cd ..) done
for d in ./*/; do (cd $d; make all -j; retVal=$?); if [ ${retVal} -ne 0 ]; then echo "Error"; exit $retVal; fi; (cd ..) done
cd $SCRIPT_DIR/InternalTools/PackageWebServiceAndComputeVm/
make all -j

cd $SCRIPT_DIR/InternalTools/DatabaseTools/
make all -j

set -e

cd $SCRIPT_DIR/Binary
./PackageWebServiceAndComputeVm

cd $SCRIPT_DIR
