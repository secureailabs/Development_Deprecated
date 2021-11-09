$ResourceGroup="VirtualMachineImageStorageRg"
$StorageAccountName="sailcomputationimage9852"
$Location="eastus"

# Create resource group for storage account
az group create --name $ResourceGroup --location $Location

# Create storage account
az storage account create --resource-group $ResourceGroup --name $StorageAccountName --location $Location --sku Standard_LRS --kind StorageV2 --access-tier Hot

# Ubuntu Image
.\packer.exe build -var location=$Location -var storage_resource_group=$ResourceGroup -var storage_account=$StorageAccountName packer-vhd-ubuntu.json

# Generate SAS for the container called /system where unmanaged VHD image was captured
# az storage container generate-sas --account-name $StorageAccountName --name system --permissions rl --start "2020-12-01T00:00:00Z" --expiry "2023-01-31T00:00:00Z"
