<#PSScriptInfo

.VERSION 1.0

.GUID

.AUTHOR prawal@secureailabs.com
#>

<#

.DESCRIPTION
 Script to encrypt CVM from VHD and upload it to Azure

#>

# Do Get-Help .\CVMDeploy.ps1 to see the ways you can run this script

# .\BuildVmImage.ps1 -AppId '876d4e71-3dfb-473f-9c64-df64b5aaa572' -Secret 'd9UVy1n~C370T86PAtKFsYnknbWDah4q~-' -TenantId '7bd9a74e-e73c-4c78-8d5d-a5c5d64b437a' -KeyURL 'https://newsailhsm.managedhsm.azure.net/keys/cvmrsakey/55896035b5fe4d3f31a87bdd38b4de03' -SubscriptionId '20c11edd-abb4-4bc0-a6d5-c44d6d2524be' -TransactionId 'aaaaaaaa' -Region 'eastus' -RSAKey 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQCvWKok/DXhB7j0Wr2KgE5ZlN37B1BEn5r5cual/gEY8N60RrzIR6FBKZQFQZm02+5oqb9pHsfiSxx+IcguixjHG4anM9ZVbe78ab3sSfPHLIxZn42PS7Kk3yo9HyhSHEYbw4DpJWNM4ZHqHxVnn0+fXcwy3OleIu2/xBu03+4So0CB+560SflMeY3GT69tPxi/Qv4Pk65+1WqM3wMwc+Bj04aAn4oCXTArsmZ48sZXJofcr/881IdhyC3yOuZ2f7S6njGar+7M291/7mInq6iLoqyrEY55BrJvjIAo2cHX11+0wI8Nyiq5cXpQK8inBAkkBLBaSuABcEbYPOoAIBATpsle7SKi6SJ6DQK7olEFxMVV/0VT9SwUjaYgeKF6vZtBmHKp+VeGYAHQbSNvagPReLHJZgmChW85rEB7oUTZ+xYj0NC0+vFlWIesoDwhK+9uhDvIOJysATjP9YY+drLBC/3R0/1tN9dLbuqyxE5B3sVmGe9eDlKEZLVEijcURU9Hp+m7uey5kMcwAWFfFsSorN4x+bV5wAMAgl7Z4vW5JlEQ8hFuy52/AcLD3Pb5TGYxtZxtLsbKoFzRZZKXmnrF7jIXTkRrtsz/0e46FT7/t5zzaSWwvPZuVeEMI0En0cV+Tgwcy2s2O9mT6WMYCxk8M5GTgrU27pnqwKKO0vGMKw== administrator@CLEAN-ROOM-SERVER' -ResourceGroup 'Warehouse' -SASToken '?sv=2019-12-12&si=cvmprivatepreviewsas&sr=c&sig=Bdteq9KEOosJ9nsQS1faq2ng6It02HDncxTVvtJ47ZQ%3D' -VmSize 'Standard_DC1as_v4'

[CmdletBinding()]
    param(
        [Parameter(Mandatory=$true, HelpMessage="Login App Id.")]
        [string] $AppId,

        [Parameter(Mandatory=$true, HelpMessage="Login Secret.")]
        [string] $Secret,

        [Parameter(Mandatory=$true, HelpMessage="Tenant Id.")]
        [string] $TenantId,

        [Parameter(Mandatory=$true, HelpMessage="URL of the key.")]
        [string] $KeyURL,

        [Parameter(Mandatory=$true, HelpMessage="Subscription Id.")]
        [string] $SubscriptionId,

        [Parameter(Mandatory=$true, HelpMessage="Transaction Id of the operation")]
        [string] $TransactionId,

        [Parameter(Mandatory=$true, HelpMessage="Azure resource group that you want to upload the Virtual Machine Image to")]
        [string] $ResourceGroup,

        [Parameter(Mandatory=$true, HelpMessage="Region specific configurations, example eastus")]
        [string] $Region,

        [Parameter(Mandatory=$true, HelpMessage="Private preview access token from welcome mail")]
        [string] $SASToken,

        [Parameter(Mandatory=$true, HelpMessage="Private RSA Key to provision the Virtual Machine")]
        [string] $RSAKey,

        [Parameter(Mandatory=$true, HelpMessage="Virtual machine size")]
        [ValidateSet ("Standard_DC1as_v4",
                      "Standard_DC2as_v4",
                      "Standard_DC8as_v4",
                      "Standard_DC16as_v4",
                      "Standard_DC32as_v4",
                      "Standard_DC48as_v4",
                      "Standard_DC96as_v4")]
        [string] $VMSize
    )

Set-PSDebug -Trace 1

# Add azure cli to the path
$env:Path+=';C:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\wbin\'

# Attempt to login to Azure portal with the service principal
az login --service-principal -u $AppId -p $Secret --tenant $TenantId
az account set --subscription $SubscriptionId

# Create a copy of the original azure Virtual Machine to encrypt.
# This is done because the encryption happens in place, we need to keep the original
# copy for next usage
$vmImageName=$TransactionId+'-ubuntu20.04.vhd'
Copy-Item ubuntu20.04.vhd $vmImageName

# Download the public key which will be used to encrypt the image
$publicKeyFile=$TransactionId+'-key.pem'
az keyvault key download --id $KeyURL -f $publicKeyFile

# Provision and Encrypt the Virtual Machine
.\CVMTool.ps1 -OsType 'Linux' -UserName 'saildeveloper' -SshKey $RSAKey -OsVhdPath $vmImageName -VMGSEncryptionKey $keyurl -MaaEndpointUri 'https://sharedeus.eus.attest.azure.net/' -PublicKeyFile $publicKeyFile -TenantId $TenantId

#Upload the Virtual Machine to Azure
# The following file is created in the previous instruction which we will use to get the file name
# of the VGMS file that we need
# Write-Output $vmgsFile > vgms_filename.temp
$vmgs=$(Get-Content vgms_filename.temp)
.\CVMDeploy.ps1 -ResourceGroup $ResourceGroup -Region $Region -SASToken $SASToken -VMSize $VMSize -VMName $TransactionId -OsVhdPath $vmImageName -VMGSPath $vmgs -OsType Linux

# Clean up all the temp files to save space
Remove-Item $vmImageName
Remove-Item $vmgs
Remove-Item $publicKeyFile
