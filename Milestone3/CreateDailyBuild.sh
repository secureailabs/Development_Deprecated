#! /bin/bash
# set -e
declare -a StringArray=("BaseVmImageInit" "CommunicationPortal" "DataDomainProcess" "DatabaseGateway"
"InitializerProcess" "JobEngine" "RestApiPortal" "RootOfTrustProcess" "SignalTerminationProcess"
"Tests")

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
set -x

# Build and put stuff in the Binary folder
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

set +x
# Verify files exist
for val in "${StringArray[@]}"; do
    echo -e "\nsearching for ${val} ..."
    find $SCRIPT_DIR/Binary/ -name $val
    retVal=$?
    if [ $retVal -ne 0 ]; then
        echo "Error ${val} does not exist"
        exit $retVal
    fi
done

set -e

cd $SCRIPT_DIR/Binary
./PackageWebServiceAndComputeVm

cd $SCRIPT_DIR
