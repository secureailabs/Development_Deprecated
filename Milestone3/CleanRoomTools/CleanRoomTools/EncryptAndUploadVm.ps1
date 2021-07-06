$appId="876d4e71-3dfb-473f-9c64-df64b5aaa572"
$password="d9UVy1n~C370T86PAtKFsYnknbWDah4q~-"
$tenantId="7bd9a74e-e73c-4c78-8d5d-a5c5d64b437a"
$subscriptionId="20c11edd-abb4-4bc0-a6d5-c44d6d2524be"
$objectId="d72d1121-04b0-4d19-a9c9-fb0589bc7a61"
# $objectId=$(Get-AzureADServicePrincipal -Filter "AppId eq 'bf7b6499-ff71-4aa2-97a4-f372087be7f0'").ObjectId
$transactionId="aaaaaaaa"
Copy-Item ubuntu20.04.vhd $transactionId"-ubuntu20.04.vhd"
az login --service-principal -u $appId -p $password --tenant $tenantId
az account set --subscription $subscriptionId

# Install-Module AzureAD
# Connect-AzureAD -TenantId $tenantId
# New-AzureADServicePrincipal -AppId bf7b6499-ff71-4aa2-97a4-f372087be7f0 -DisplayName “Confidential VM Guest Agent”
# $mhsmName="NewSailHsm"
# az keyvault role assignment create --hsm-name $mhsmName --assignee $objectId --role "Managed HSM Crypto User" --scope /keys
# az keyvault role assignment create --hsm-name $mhsmName --assignee $objectId --role "Managed HSM Administrator" --scope /

# $keyname="cvmrsakey"
$keyId="https://newsailhsm.managedhsm.azure.net/keys/cvmrsakey"
# az keyvault key create --id $keyId --ops wrapKey unwrapkey encrypt decrypt --kty RSA-HSM --size 3072 --exportable --policy "@.\skr-policy.json"

$keyurl="https://newsailhsm.managedhsm.azure.net/keys/cvmrsakey/55896035b5fe4d3f31a87bdd38b4de03"
az keyvault key download --id $keyurl -f $keyname.pem

$rsaKey="ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQCvWKok/DXhB7j0Wr2KgE5ZlN37B1BEn5r5cual/gEY8N60RrzIR6FBKZQFQZm02+5oqb9pHsfiSxx+IcguixjHG4anM9ZVbe78ab3sSfPHLIxZn42PS7Kk3yo9HyhSHEYbw4DpJWNM4ZHqHxVnn0+fXcwy3OleIu2/xBu03+4So0CB+560SflMeY3GT69tPxi/Qv4Pk65+1WqM3wMwc+Bj04aAn4oCXTArsmZ48sZXJofcr/881IdhyC3yOuZ2f7S6njGar+7M291/7mInq6iLoqyrEY55BrJvjIAo2cHX11+0wI8Nyiq5cXpQK8inBAkkBLBaSuABcEbYPOoAIBATpsle7SKi6SJ6DQK7olEFxMVV/0VT9SwUjaYgeKF6vZtBmHKp+VeGYAHQbSNvagPReLHJZgmChW85rEB7oUTZ+xYj0NC0+vFlWIesoDwhK+9uhDvIOJysATjP9YY+drLBC/3R0/1tN9dLbuqyxE5B3sVmGe9eDlKEZLVEijcURU9Hp+m7uey5kMcwAWFfFsSorN4x+bV5wAMAgl7Z4vW5JlEQ8hFuy52/AcLD3Pb5TGYxtZxtLsbKoFzRZZKXmnrF7jIXTkRrtsz/0e46FT7/t5zzaSWwvPZuVeEMI0En0cV+Tgwcy2s2O9mT6WMYCxk8M5GTgrU27pnqwKKO0vGMKw== administrator@CLEAN-ROOM-SERVER"
./CVMTool.ps1 -OsType Linux -UserName sail -SshKey $rsaKey -OsVhdPath ".\ubuntu20.04.vhd" -VMGSEncryptionKey $keyurl –MaaEndpointUri "https://sharedeus.eus.attest.azure.net/" -PublicKeyFile .\cvmrsakey.pem -TenantId $tenantId

$vgms="C:\Users\ADMINI~1\AppData\Local\Temp\2\CVMOutput\4a2d3625-4db8-445c-ace9-07a9119836f1\vmgs.vhd"
$vhd="C:\Users\Administrator\Final\CleanRoomTools\aa-ubuntu20.04.vhd"

# $appObjId=$(Get-AzureADServicePrincipal -Filter "AppId eq '876d4e71-3dfb-473f-9c64-df64b5aaa572'").ObjectId
# $AgentObjectId=$(Get-AzureADServicePrincipal -Filter "AppId eq 'bf7b6499-ff71-4aa2-97a4-f372087be7f0'").ObjectId

# az keyvault role assignment create --hsm-name $mhsmName --assignee $appObjId --role "Managed HSM Crypto User" --scope /keys/cvmrsakey
# az keyvault role assignment create --hsm-name $mhsmName --assignee $AgentObjectId --role "Managed HSM Crypto User" --scope /keys/cvmrsakey