{
    "AppId": "876d4e71-3dfb-473f-9c64-df64b5aaa572",
    "Secret": "d9UVy1n~C370T86PAtKFsYnknbWDah4q~-",
    "TenantId": "7bd9a74e-e73c-4c78-8d5d-a5c5d64b437a",
    "KeyURL": "https://newsailhsm.managedhsm.azure.net/keys/cvmrsakey/55896035b5fe4d3f31a87bdd38b4de03",
    "SubscriptionId": "20c11edd-abb4-4bc0-a6d5-c44d6d2524be",
    "Region": "eastus",
    "RSAKey": "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQCvWKok/DXhB7j0Wr2KgE5ZlN37B1BEn5r5cual/gEY8N60RrzIR6FBKZQFQZm02+5oqb9pHsfiSxx+IcguixjHG4anM9ZVbe78ab3sSfPHLIxZn42PS7Kk3yo9HyhSHEYbw4DpJWNM4ZHqHxVnn0+fXcwy3OleIu2/xBu03+4So0CB+560SflMeY3GT69tPxi/Qv4Pk65+1WqM3wMwc+Bj04aAn4oCXTArsmZ48sZXJofcr/881IdhyC3yOuZ2f7S6njGar+7M291/7mInq6iLoqyrEY55BrJvjIAo2cHX11+0wI8Nyiq5cXpQK8inBAkkBLBaSuABcEbYPOoAIBATpsle7SKi6SJ6DQK7olEFxMVV/0VT9SwUjaYgeKF6vZtBmHKp+VeGYAHQbSNvagPReLHJZgmChW85rEB7oUTZ+xYj0NC0+vFlWIesoDwhK+9uhDvIOJysATjP9YY+drLBC/3R0/1tN9dLbuqyxE5B3sVmGe9eDlKEZLVEijcURU9Hp+m7uey5kMcwAWFfFsSorN4x+bV5wAMAgl7Z4vW5JlEQ8hFuy52/AcLD3Pb5TGYxtZxtLsbKoFzRZZKXmnrF7jIXTkRrtsz/0e46FT7/t5zzaSWwvPZuVeEMI0En0cV+Tgwcy2s2O9mT6WMYCxk8M5GTgrU27pnqwKKO0vGMKw== administrator@CLEAN-ROOM-SERVER",
    "ResourceGroup": "Warehouse",
    "SASToken": "?sv=2019-12-12&si=cvmprivatepreviewsas&sr=c&sig=Bdteq9KEOosJ9nsQS1faq2ng6It02HDncxTVvtJ47ZQ%3D",
    "VmSize": "Standard_DC1as_v4'
}

Invoke-WebRequest https://azcopyvnext.azureedge.net/release20210527/azcopy_windows_amd64_10.10.0.zip -OutFile azcopyv10.zip ; Expand-Archive -LiteralPath .\azcopyv10.zip -DestinationPath .\ ; cp .\azcopy_windows_amd64_10.10.0\azcopy.exe .\azcopy.exe

az keyvault key create --id https://newsailhsm.managedhsm.azure.net/keys/cleanroomkey --ops wrapKey unwrapkey encrypt decrypt --kty RSA-HSM --size 3072 --exportable --policy "@.\skr-policy.json"

az keyvault role assignment create --hsm-name newsailhsm --assignee d72d1121-04b0-4d19-a9c9-fb0589bc7a61 --role "Managed HSM Crypto User" --scope /keys/cleanroomkey

$keyId='https://newsailhsm.managedhsm.azure.net/keys/cleanroomkey/e31846043e4b0b51343344acc5b38874'
az keyvault key download --id $keyId -f cleanroomkey.pem
.\CVMTool.ps1 -OsType Windows -UserName saildeveloper -Password FLT8w%GYd4mCj6Jp8VY -OsVhdPath .\win-server.vhd -VMGSEncryptionKey $keyId -MaaEndpointUri 'https://sharedeus.eus.attest.azure.net/' -PublicKeyFile .\cleanroomkey.pem -TenantId $TenantId

$EncryptedVhd='C:\Users\CleanRoomAdmin\Final_hopefully\win-server.vhd'
$Vgms='C:\Users\CLEANR~1\AppData\Local\Temp\2\CVMOutput\6fdede63-b26d-4952-9303-60cc775c7bbc\vmgs.vhd'
.\CVMDeploy.ps1 -ResourceGroup Warehouse -Region eastus -SASToken $sasToken -VMSize Standard_DC2as_v4 -VMName CleanRoom -OsVhdPath $EncryptedVhd -VMGSPath $Vgms -OsType Windows


WARNING: ACTIONS REQUIRED:

1. Save this numerical recovery password in a secure location away from your computer:

275473-225445-503877-027423-282271-479512-325963-029678

To prevent data loss, save this password immediately. This password helps ensure that you can unlock the encrypted volume.