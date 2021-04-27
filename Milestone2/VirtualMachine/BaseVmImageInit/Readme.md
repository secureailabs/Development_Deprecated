# Virtual Machine Initialization Tool Usage Guid

## Introduction
The BaseVmImageInit tool is a process that runs on the newly provisioned Azure Virtual Machine and waits for a connection from the client to upload files and start the execution of the entrypoint mentioned in the StructuredBuffer payload it receives.

## Template Virtual Machine Creation

1. Create an Ubuntu 20.04 running virtual machine using AzurePortal. Ensure the following at the creation time.
    * SSH is enabled with either SSH keys or at least a username and password.
    * Configure the Virtual Machine's Network Security Group's `Inbound Security Rules` to accept connection on port 9090 for this process. At the same time also allow other ports which will be used by the applications running on VM.
2. Copy the IP of the Virtual Machine and ssh into the Virtual Machine `ssh <username>@<VM Public IP>`
3. Set a crontab job to run BaseVmImageInit on every reboot by:
    * Run `crontab -e`
    * Add the line `@reboot sudo ./BaseVmImageInit > ImageInit.log`. This will run the BaseVmImageInit on every reboot and put all the logs in the `ImageInit.log` file.
    * Install the following:
        ```
        sudo apt-get update
        sudo apt-get install -y libssl-dev libuuid1 python3-dev python3-pip
        sudo pip3 install numpy scipy
        sudo pip3 install pandas xgboost matplotlib shap imbalanced-learn
        sudo pip3 install -U scikit-learn
        sudo pip3 install ipython
        sudo pip3 install torch --no-cache-dir
        ```
4. Copy the BaseVmImageInit using the following instructions to the virtual machine.

## Build Instructions

`make all -j`

will build the BaseVmImageInit tool but it needs to be manually copied to the template Virtual Machine later. This can be done easily using Secure Copy scp:

`scp <Path to BaseVmImageInit> <username>@<Virtual Machine Public IP>:<path on the VM>`

Example:

`scp BaseVmImageInit azureuser@52.179.155.217:/home/azureuser`

Make sure ssh is running on the newly created Virtual Machine and port 22 is open to public.

## Create a Virtual Machine Image from the Virtual Machine

Once the Virtual Machine is in a state that can be rebooted and can download all the binaries and run them. Wen can create an image out of it and provision new virtual machines which will be copies of it.
1. Goto Azure console on the portal or install Azure CLI on Powershell.
2. Run `az vm deallocate --resource-group <ResourceGroupName> --name <Virtual Machine Name>` to stop and deallocate the Virtual Machine
3. Run `az vm generalize --resource-group <ResourceGroupName> --name <Virtual Machine Name>`
4. Create an image using the generalized Virtual Machine `az image create --resource-group <ResourceGroupName> --name <New Image Name> --source <Virtual Machine Name>`
5. Make sure to update the relevant global constants `EndPointTools/InitializerTool/InitializerData.h` for these changes to get used while creating a new Virtual Machine using the Initializer tool.

