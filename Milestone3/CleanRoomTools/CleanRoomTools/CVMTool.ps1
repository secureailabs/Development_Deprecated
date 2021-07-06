<#PSScriptInfo

.VERSION 1.0

.GUID e73ad37c-b405-48c0-8cff-001d1983aa28

.AUTHOR cvmps@microsoft.com
#>

<# 

.DESCRIPTION 
 Script to provision CVM compatible VHDs 

#> 

# Do Get-Help .\CVMTool.ps1 to see the ways you can run this script

[CmdletBinding()]
    param(
        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM', HelpMessage="SshKey that you want to provision in CVM (required only for Linux)")]
        [Alias('S')]
        [string] $SshKey,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM', HelpMessage="OS type of the CVM that you are provisoning")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM', HelpMessage="OS type of the CVM that you are provisoning")]
        [ValidateSet ("Linux", "Windows")]
        [string] $OsType,

        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM', HelpMessage="Password that you want to provision in CVM (required only for Windows)")]
        [Alias('P')]
        [string] $Password,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM', HelpMessage="The VHD that you want to provision for CVM")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM', HelpMessage="The VHD that you want to provision for CVM")]
        [Alias('O')]
        [ValidateScript({
            $global:ProvisioningVHDName = "CVMProvisioningVM20.04.vhdx"

            $osVHDExists = test-path $_ -pathtype leaf
            if ($osVHDExists  -eq $false)
            {
                throw "Supplied osVhdPath argument $_ does not exist."
            }

            try {
                $osVhdBaseName = Split-Path ([string]$_) -leaf

            } catch {
                throw "Incorrect OS VHD Path."
            }
                              
            if ($osVhdBaseName -eq $global:ProvisioningVHDName)
            {
                throw "It seems you are using incorrect VHD $_, the provisioning VHD is used internally, please supply an OS disk."
            }
            $true
        })]
        [string] $OsVhdPath,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM', HelpMessage="Username that you want to provision in CVM")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM', HelpMessage="Username that you want to provision in CVM")]
        [Alias('U')]
        [string] $UserName,
                
        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Hostname that you want to provision in CVM (required only for Linux)")]       
        [Alias('H')]
        [string] $HostName="testhost",

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Flag to determine if the Azure Linux Agent (waagent) should be disabled (required only for Linux)")]       
        [Alias('D')]
        [switch] $DisableWaAgent,
    
        
        #[Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Flag to determine if the Azure Linux Agent (waagent) should be disabled (required only for Linux)")]       
        #[switch] $NoInitramfsPatching,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM',  HelpMessage="VMGSEncryptionKey KEY ID URI")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM',  HelpMessage="VMGSEncryptionKey KEY ID URI")]
        [Alias('V')]
        [ValidateScript({
            try {
                  
                if (([uri]$_).Segments.Length -ne 4)
                {
                    throw "VMGSEncryptionKey should have 4 segments (ie with version) eg. https://cvmakvmhsm.managedhsm.azure.net/keys/myrsakey-withskr/17c116ee6528089a0be5c5adf3acc837."
                }
                $true
            } catch {
                throw "VMGSEncryptionKey is not a valid URI. eg. https://cvmakvmhsm.managedhsm.azure.net/keys/myrsakey-withskr/17c116ee6528089a0be5c5adf3acc837 "
            }
        })]
        [uri] $VMGSEncryptionKey,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Region specific configurations, example eastus, westus")]
        [Parameter(Mandatory=$false, ParameterSetName = 'WindowsCVM',  HelpMessage="Region specific configurations, example eastus, westus")]
        [Alias('M')]
        [string] $Region,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM',  HelpMessage="Tenant id")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM',  HelpMessage="Tenant id")]
        [ValidateScript({
            try {
                [System.Guid]::Parse($_) | Out-Null
                $true
            } catch {
                throw "Tenant id should be a guid"
            }
        })]
        [string] $TenantId,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Subscription id")]
        [Parameter(Mandatory=$false, ParameterSetName = 'WindowsCVM',  HelpMessage="Subscription id")]
        [ValidateScript({
            try {
                [System.Guid]::Parse($_) | Out-Null
                $true
            } catch {
                throw "Subscription id should be a guid"
            }
        })]
        [string] $Subscription,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="PublicKeyFile")]
        [Parameter(Mandatory=$false, ParameterSetName = 'WindowsCVM',  HelpMessage="PublicKeyFile")]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string] $PublicKeyFile,

        [Parameter(Mandatory=$true, ParameterSetName = 'LinuxCVM',  HelpMessage="MAA Endpoint URI")]
        [Parameter(Mandatory=$true, ParameterSetName = 'WindowsCVM',  HelpMessage="MAA Endpoint URI")]
        [uri] $MaaEndpointUri,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM',  HelpMessage="Test")]
        [Parameter(Mandatory=$false, ParameterSetName = 'WindowsCVM',  HelpMessage="Test")]
        [Alias('T')]
        [switch] $Test,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM', HelpMessage="Secure boot setting of the CVM")]
        [switch] $SBoff,

        [Parameter(Mandatory=$false, ParameterSetName = 'LinuxCVM', HelpMessage="Whether to encrypt the disk or not")]
        [ValidateSet ("true", "false")]
        [string] $Encryption = "true"
    )

    $NoInitramfsPatching=$false

#region Internal Functions

<# 
    Checks if a given file is present
#>
function CheckIfFileIsPresent
{
    param(
        [Parameter(Mandatory = $true)]
        $FilePath
    )

    if ([string]::IsNullOrEmpty($FilePath) -or !(Test-Path $FilePath -PathType Any))
    {
        Write-Error "No file found at path '$FilePath'"
    }
}

<# 
    Checks if a given command succeeded.
#>
function Verify-Succeeded
{
    [CmdletBinding()]
    PARAM(
    [Parameter(Position=1, Mandatory=$true)]
    [ScriptBlock] $Command,
    [Parameter(Position=1, Mandatory=$false)]
    [Switch] $IgnoreWarning
    )
    Process
    {
        $TemporaryScriptBlock = " -ErrorAction Stop"
        $Command = [ScriptBlock]::Create($Command.ToString() + $TemporaryScriptBlock.ToString() )
        $old_ErrorActionPreference = $ErrorActionPreference
        $ErrorActionPreference = "Stop"
            
        try
        {
                Invoke-command -ScriptBlock $Command -WarningVariable  WVariable

                if(!$IgnoreWarning)
                {
                  if(![String]::IsNullOrEmpty($WVariable))
                  {
                     throw [System.Exception] "Warning!!!!! $WVariable"
                  }
                }
                else
                {
                  if(![String]::IsNullOrEmpty($WVariable))
                  {
                    // Add warning log 
                  }
                }                      
            }
            catch
            {
                if(($_.Exception.Message -ne $null) -and (!$IgnoreWarning))
                {
                    $FullyQualifiedErrorId = $_.FullyQualifiedErrorId
                    if(![String]::IsNullOrEmpty($WVariable))
                    {
                        // Add error log
                    }
                   throw [System.Exception] "Command Execution Failed $_.Exception.Message $FullyQualifiedErrorId"
                }                
            }
            $ErrorActionPreference = $old_ErrorActionPreference
      }
}

<# 
    Generate a random key
#>
function Generate-RandomKey
{
    param(
        [Parameter(Mandatory=$true)]
        [int]$Length,        
        [Parameter(Mandatory=$false)]
        [switch]$Strong=$false
    )

    switch ($Strong)
    {
        $true
        {
            [string]$allowed_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 !@#$%^&*()_+{}|[]\:;'<>?,./`~"
        }
        $false
        {
            [string]$allowed_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
        }
    }

    $bytes = New-Object Byte[] $Length
    [Security.Cryptography.RNGCryptoServiceProvider]::Create().GetBytes($bytes)

    $passphrase = ""

    if ($NoInitramfsPatching)
    {
        # Use well known passphrase helps debugging, this is temporary.
        Write-Host ("This is unsupported path for testing only")
        $passphrase = "passphrase"
    }
    else
    {
        for( $i=0; $i -lt $Length; $i++)
        {
            $passphrase += $allowed_characters[$bytes[$i] % $allowed_characters.Length]	
        }
    }

    return $passphrase
}

<# 
    Converts from bytes to base64url
#>
function Base64UrlEncode {
    param (
        [Parameter(Mandatory=$true)]
        [byte[]] $Argument
    )
    $base64 = [System.Convert]::ToBase64String($Argument) 
    $base64url = $base64.Split('=')[0]
    $base64url = $base64url.Replace('+', '-')
    $base64url = $base64url.Replace('/', '_')
   
    return $base64url
}

<# 
    Converts from base64url to base64
#>
function Base64UrlDecode 
{
    param (
        [Parameter(Mandatory)]
        [string] $Argument
    )
    
    $output = $Argument
    $output = $output.Replace('-', '+')
    $output = $output.Replace('_', '/')

    switch($output.Length % 4) {
        0 {break}
        2 {$output += '=='; break}
        3 {$output += '='; break}
        DEFAULT {Write-Warning 'Illegal base64 string!'}
    }

    return $output
}

<# 
    Get encryption keys
#>
function GetKeysObject
{
    $bootKey = Generate-RandomKey -Length 16
    $rootKey = Generate-RandomKey -Length 16 ### Increase size before release (easy to enter passphrase if needed)
    return $bootKey, $rootKey
}

<# 
    Validates if all file paths in the list are present 
#>
function ValidatePresent
{
    param(
        [parameter(Mandatory=$true)]
        [array] $FileList
        )

    $FileList | Where-Object { $_} | ForEach-Object {
         if (-not (Test-Path $_ -PathType Leaf))
         {
            throw "The required tools/files are not present. Could not find $($_), exiting."
         }
    } 
}

<# 
    Creates sealed keys and vtpm blob 
#>
function CreateSealedKeys
{
    param(
        [parameter(Mandatory=$true)]
        [string]$BootKey,
        [parameter(Mandatory=$true)]
        [string]$RootKey,
        [parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$PcrConfig,
        [parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$VTPMFile
    )

    $PcrConfigSanitized = $PcrConfig | SanitizePath
    $bootKeyFile = Join-Path $global:outputFolder "bootKey"
    $rootKeyFile = Join-Path $global:outputFolder "rootKey"
    $serializeKeyFile = Join-Path $global:outputFolder "serializeKey"
    $sealedKeys = Join-Path $global:outputFolder "sealedKeys"

    $BootKey | Out-File -Encoding ascii $BootKeyFile -NoNewLine
    
    if ($NoInitramfsPatching)
    {
        # We corrupt the rootkey on purpose before sealing so that the initramfs does not get patched.
        $RootKey+"a"| Out-File -Encoding ascii $RootKeyFile -NoNewLine
    }
    else
    {
        $RootKey | Out-File -Encoding ascii $RootKeyFile -NoNewLine
    }

    $ret = Start-Process $global:SerializeTool -NoNewWindow -ArgumentList "$($bootKeyFile) $($rootKeyFile) $($serializeKeyFile)" -PassThru -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:SerializeTool) failed with error $($ret.ExitCode), Arguments were $($bootKeyFile) $($rootKeyFile) $($serializeKeyFile)"
    }

    $ret = Start-Process $global:CvmUtil -NoNewWindow -ArgumentList "-seal $($VTPMFile) $($PcrConfigSanitized) $($serializeKeyFile) $($sealedKeys)" -PassThru -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode)
    {
        throw " Error calling $($global:CvmUtil) failed with error $($ret.ExitCode ), Arguments were -seal $($VTPMFile) $($PcrConfig) $($serializeKeyFile) $($sealedKeys)"
    }


    Remove-Item -Path $serializeKeyFile | Out-Null

    return $sealedKeys

}

<# 
    Creates encrypted os vhd with the guest configuration enabled using the commandlet for provisioning. 
#>
function CreateEncryptedOsVHDLinux
{
    param(
        [parameter(Mandatory=$true)]
        [string]$ProvisioningVHD,
        [parameter(Mandatory=$true)]
        [string]$OsVHD,
        [parameter(Mandatory=$true)]
        [string]$BootKey,
        [parameter(Mandatory=$true)]
        [string]$RootKey,
        [parameter(Mandatory=$true)]
        [string]$Username,
        [parameter(Mandatory=$true)]
        [string]$SshPublicKey,
        [parameter(Mandatory=$true)]
        [string]$SealedKeysFile,
        [parameter(Mandatory=$true)]
        [string]$LogsFolder,
        [parameter(Mandatory=$true)]
        [string]$Encryption,
        [parameter(Mandatory=$true)]
        [bool]$DisableWaAgent,
        [parameter(Mandatory=$true)]
        [bool]$InitramfsPatching

    )
    $OsVhdPath = $OsVhdPath | SanitizePath
    $ProvisioningVHD = $ProvisioningVHD | SanitizePath
    $module = Import-Module $global:NewCVMCmdLet -passthru
    if(!$module)
    { 
        throw "PSModuleInfo returned from import-module $(Join-Path $global:root $global:NewCVMCmdLet) was null"
    }
    Verify-Succeeded -Command {New-VHDForCVM -ProvisioningVHD $ProvisioningVHD -OsVHD $OsVhdPath -Username $userName -Hostname $HostName -SshPublicKey $SshPublicKey -OutputFolder $LogsFolder -BootKey $BootKey -RootKey $RootKey -SealedKeysFile $SealedKeysFile -Encryption $Encryption -DisableWaAgent $DisableWaAgent.ToString() -InitramfsPatching $InitramfsPatching.ToString() }
}

<# 
    Wraps a key using MHSM 
#>
function WrapKeyMHSMRestAPI
{
    param(
    [parameter(Mandatory=$true)]
    [string]$Token,
    [parameter(Mandatory=$true)]
    [string]$KeyUri,
    [parameter(Mandatory=$true)]
    [string]$Payload,
    [parameter(Mandatory=$false)]
    [string]$Algorithm="RSA-OAEP-256"
    )
  
    $headers = @{ 'Authorization' = "Bearer $($Token)"; "Content-Type" = "application/json" }

    $Body = @{
            "alg" = "$($Algorithm)"
            "value" = "$($Payload)"
            }
    $wrapUri = $KeyUri.TrimEnd("/") + "/wrapkey?api-version=7.1"
    
    $Parameters = @{
        Method = "POST"
        Uri =  "$($wrapUri)"
        Body = ($Body | ConvertTo-Json) 
        Headers =  $headers 
    }
    $response = Invoke-RestMethod @Parameters #response in base64
    If (-not $response) 
    {
        throw "Can't recieve an answer from $wrapUri"
    }

    return $response
}
<# 
    UnWraps a key using .NET API 
#>
function WrapKeyNetAPI
{
    param(
        [parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$PublicKeyFileInput,
        [parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$PayloadFile,
        [parameter(Mandatory=$true)]
        [string]$OutputFile
        )
    $ret = Start-Process $global:CVMWrapKey -NoNewWindow -ArgumentList "$($PublicKeyFileInput) $($PayloadFile) $($OutputFile)" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
       
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:CVMWrapKey) failed with error $($PublicKeyFileInput) $($PayloadFile) $($OutputFile)" 
    }
    
    return
}
<# 
    UnWraps a key using MHSM 
#>
function UnWrapKeyMHSMRestAPI
{
    param(
    [parameter(Mandatory=$true)]
    [string]$Token,
    [parameter(Mandatory=$true)]
    [string]$KeyUri,
    [parameter(Mandatory=$true)]
    [string]$Payload,
    [parameter(Mandatory=$false)]
    [string]$Algorithm="RSA-OAEP-256"
    )
  
    $headers = @{'Authorization' = "Bearer $($Token)"; "Content-Type" = "application/json" }

    $Body = @{
            "alg" = "$($Algorithm)"
            "value" = "$($Payload)"
            }
    $unwrapUri = $KeyUri.TrimEnd("/") + "/unwrapkey?api-version=7.1"
    $Parameters = @{
        Method = "POST"
        Uri =  "$($unwrapUri)"
        Body = ($Body | ConvertTo-Json) 
        Headers =  $headers 
    }

    $response = Invoke-RestMethod @Parameters #response in base64
    If (-not $response) 
    {
        throw "Can't recieve an answer from $unwrapUri"
    }

    return $response
}

<# 
    THis test reads back the data at id 6(encryption key) from VMGS and unwraps it and compares it 
    with the expected key.
#>
function TestVMGSAESKey
{
    param(
        [parameter(Mandatory=$true)]
        [string]$VMGSFile,
        [parameter(Mandatory=$true)]
        [string]$Token,
        [parameter(Mandatory=$true)]
        [string]$KeyUri,
        [parameter(Mandatory=$true)]
        [byte[]]$ExpectedKey
    )
    if ($Test -eq $true)
    {
        $wrappedKeyTestFile = Join-Path $global:outputFolder "wrappedkeytestfile"
        $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-r -f $($VMGSFile) -i=6 -d $wrappedKeyTestFile" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
        if (0 -ne $ret.ExitCode) 
        {
            throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were -r -f $($VMGSFile) -i=6 -d $wrappedKeyTestFile"
        }

        $AESKeyWrapped = [io.file]::ReadAllBytes($wrappedKeyTestFile)        
        $AESKeyWrappedBase64Url = Base64UrlEncode -Argument $AESKeyWrapped
        $mhsmunwrapResponse = UnWrapKeyMHSMRestAPI -Token $Token -KeyUri $KeyUri -PayLoad $AESKeyWrappedBase64Url #$mhsmwrapResponse.value.ToString()
        $UnwrapKeyBase64 = Base64UrlDecode -Argument $mhsmunwrapResponse.value
        $UnwrapKeyBytes = [System.Convert]::FromBase64String($UnwrapKeyBase64)

        if ($UnwrapKeyBytes.length -ne $ExpectedKey.length)
        {
            throw "The key that was wrapped and put into the VMGS did not match the length of the expected key (checked via unwrap)"
        }

        $i=0
        foreach ($value in $UnwrapKeyBytes)
        {
            if ($UnwrapKeyBytes[$i] -ne $ExpectedKey[$i])
            {
                throw "The key that was wrapped and put into the VMGS did not match the expected key (checked via unwrap)"
            }
            $i++;
        }

        Write-Host "TestVMGSAESKey Test Passed " -ForegroundColor Green
        Remove-Item -Path $wrappedKeyTestFile | Out-Null
     }
}

<# 
    Creates CVM compatible VMGS with the key reference and VTPM blob. 
#>
function CreateKeyReference
{
    param(
        [Parameter(Mandatory=$true)] 
        [uri]$KeyUri,
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$KeyReferenceTemplate,
        [Parameter(Mandatory=$true)]
        [string]$KeyReference
        )
        $templateObject = $null
        try
        {
            # Key Uri will be like https://cvmakvmhsm.managedhsm.azure.net/keys/myrsakey-withskr/17c116ee...
            $keyName = $KeyUri.Segments[2].Trim("/") 
            $hostName  = "https://" + $KeyUri.Host
            $keyVersion = $KeyUri.Segments[3].Trim("/") 
            $templateObject = Get-Content $KeyReferenceTemplate | ConvertFrom-Json
            $templateObject.key_info.key_name = $keyName
            $templateObject.key_info.key_version = $keyVersion
            $templateObject.key_info.host = $hostName
            $templateObject.key_info.tenant_id = $TenantId
        }
        catch
        {
            throw "Unable to parse key Uri and generate security profile $KeyReference Key $KeyUri $($PSItem.ToString())"
        }

        # Add maa endpoint informaton

        try 
        {
            
            # MaaEndpoint Uri is provided as input, add it to security profile
            $templateObject.attestation_info.host = $MaaEndpointUri               
            ConvertTo-Json $templateObject -Depth 100 | Out-File $KeyReference -Encoding ascii
        }
        catch {
            throw "Unable to populate MAA endpoint $($PSItem.ToString())"
        }

        if ($Test -eq $true)
        {
            Write-Host "Successfully added $keyName : Output $KeyReference" -ForegroundColor Green 
        }
}

<# 
    Creates encrypted CVM compatible VMGS with the security profile and VTPM blob. 
#>
function CreateVMGS
{
    param(
        [parameter(Mandatory=$true)]
        [string]$VMGSFile,
        [parameter(Mandatory=$true)]
        [string]$VTPMFile,
        [parameter(Mandatory=$false)]
        [string]$Token,
        [parameter(Mandatory=$true)]
        [string]$KeyUri,
        [parameter(Mandatory=$true)]
        [string]$OsType

    )
    $AESKey = New-Object Byte[] 32
    [Security.Cryptography.RNGCryptoServiceProvider]::Create().GetBytes($AESKey)
    $AES_key_in_base_64_url = Base64UrlEncode -Argument $AESKey
    [io.file]::WriteAllBytes($global:KeyFile, $AESKey)

    CheckIfFileIsPresent -FilePath $global:KeyFile 
        
    # 2) Initialize vmgs with encryption enabled
    $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-c -f $($VMGSFile) -k $($global:KeyFile) -EncryptionAlgorithm AES_GCM" -PassThru -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were -c -f $($VMGSFile) -k $($global:KeyFile) -EncryptionAlgorithm AES_GCM"
    }

    # 3) Write PCR template
    $pcrTeamplateFile = $global:LinuxPcrTemplate
    if ($OsType -eq 'Windows')
    {
        $pcrTeamplateFile = $global:WindowsPcrTemplate
    }
    $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-w -f $($VMGSFile) -d $($pcrTeamplateFile) -i=9" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode)
    {
        throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were -w -f $($VMGSFile) -d $($pcrTeamplateFile) -i=9"
    }

    # 4) Write the vtpm blob encrypted.
    $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-w -f $($VMGSFile) -d $($VTPMFile) -k $($global:KeyFile) -i=3" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were  -w -f $($VMGSFile) -d $($VTPMFile) -k $($global:KeyFile) -i=3" 
    }
    $wrappedKeyFile =  Join-Path $global:outputFolder "wrappedkeyfile"
    
    # 5) Wrap the keyfile using OAEP-256 using key in MHSM and then put it into the VMGS.
    # If Public Key is provided use that instead.
    if ($PublicKeyFile.Length -ne 0)
    {
        WrapKeyNetAPI -PublicKeyFileInput $PublicKeyFile -PayloadFile $global:KeyFile -OutputFile $wrappedKeyFile
    }
    else 
    {
        $mhsmwrapResponse = WrapKeyMHSMRestAPI -Token $Token -KeyUri $KeyUri -PayLoad $AES_key_in_base_64_url
        if (!$mhsmwrapResponse)
        {
            throw " Error wrapping key -Token $Token -KeyUri $KeyUri -PayLoad $AES_key_in_base_64_url"
        }

        # 4) Write the wrapped bytes into the vmgs.
        $wrappedKeyBase64 = Base64UrlDecode -Argument $mhsmwrapResponse.value.ToString()
        $wrappedKey = [System.Convert]::FromBase64String($wrappedKeyBase64)
       
        [io.file]::WriteAllBytes($wrappedKeyFile, $wrappedKey)
    }
    $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-w -f $($VMGSFile) -d $($wrappedKeyFile) -i=6" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were -w -f $($VMGSFile) -d $($wrappedKeyFile) -i=6"
    }

    # 6) Write the security profile.
    CreateKeyReference -KeyUri $KeyUri -KeyReferenceTemplate $global:KeyReferenceTemplatePath -KeyReference $global:KeyReferencePath
    $ret = Start-Process $global:VMGSTool -NoNewWindow -ArgumentList "-w -f $($VMGSFile) -i=5 -d $KeyReferencePath" -PassThru  -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode) 
    {
        throw " Error calling $($global:VMGSTool) failed with error $($ret.ExitCode), Arguments were -f $($VMGSFile) -i=5 -d $KeyReferencePath"
    }

    if ($Test -eq $false)
    {
        Remove-Item -Path $wrappedKeyFile | Out-Null
        Remove-Item -Path $global:KeyReferencePath | Out-Null
        Remove-Item -Path $global:KeyFile | Out-Null
    }
}

function TestHyperV
{
    if ((Get-Service -Name vmms).Status -ne "Running") 
    {
        $error = "Hyper-V (VMMS) is not running/enabled in this host. Please make sure Hyper-V is installed.`n" +
                 "You can enable the Hyper-V feature by running:`n" +
                 "Enable-WindowsOptionalFeature -Online -All -FeatureName 'Microsoft-Hyper-V', 'Microsoft-Hyper-V-Management-Powershell'`n" +
                 "And then rebooting your host."
        throw $error
    }
    
    try 
    {
        Get-VM | Out-Null
    }
    catch 
    {
        $error = "Hyper-V cmdlets are not available in this host. Please make sure they can be used.`n" +
        "You can enable the Hyper-V management powershell feature by running:`n" +
        "Enable-WindowsOptionalFeature -Online -All -FeatureName 'Microsoft-Hyper-V-Management-Powershell'`n" +
        "And then rebooting your host (not strictly necessary, but a good idea nonetheless)."
        throw $error
   }
}

#end-region

$Stopwatch =  [system.diagnostics.stopwatch]::StartNew()
$global:outputFolder = [System.IO.Path]::Combine($env:TEMP, "CVMOutput", [GUID]::NewGuid().ToString())

#Create output folder if needed
if((Test-Path $global:outputFolder) -eq $false)
{
    Verify-Succeeded -Command {New-Item -Path $global:outputFolder -ItemType Directory | Out-Null}
}

$global:logsFolder = Join-Path $global:outputFolder "logs"
#Create logs folder if needed

if((Test-Path $global:logsFolder) -eq $false)
{
    Verify-Succeeded -Command {New-Item -Path $global:logsFolder -ItemType Directory | Out-Null}
}

# Script initializations, define all file names here to avoid confusions.
$global:root = Split-Path -Parent $MyInvocation.MyCommand.Path
$global:script = split-path -Leaf $MyInvocation.MyCommand
$global:previousPath = $env:PATH;
$env:PATH += ";" + $root;

# Tools and templates
$global:SerializeTool = Join-Path $global:root "SerializeKey.exe"
$global:CvmUtil = Join-Path $global:root "CVMUtil.exe"
$global:VMGSTool = Join-Path $global:root "VMGSTool.exe"
$global:UnattendXmlCreatorTool = Join-Path $global:root "UnattendXmlCreator.exe" 
$global:CVMWrapKey = Join-Path $global:root "CVMWrapKey.exe"
$global:NewCVMCmdLet = Join-Path $global:root "AzProvisionVHD.dll"
$global:KeyReferenceTemplatePath = Join-Path $global:root "keyreference.json"
$global:RegionConfigurationJsonFile = Join-Path $global:root "regionConfiguration-map.json"
$global:PcrValuesLinuxFilePath = Join-Path $global:root "pcrvalueslinux.json"
$global:PcrValuesLinuxFilePathSbOff = Join-Path $global:root "pcrvalueslinuxSbOff.json"
$global:PcrValuesWindowsFilePath = Join-Path $global:root "pcrvalueswindows.json"
$global:CvmWindowsSetupPsm1 = Join-Path $global:root "CVMWindowsSetup.psm1"
Import-Module $global:CvmWindowsSetupPsm1 -Force
$global:WindowsPcrTemplate = Join-Path $global:root "MicrosoftWindows_Template.json" | SanitizePath
$global:LinuxPcrTemplate = Join-Path $global:root "OpenSourceShieldedVM_Template.json" | SanitizePath

$global:ProvisioningVHD = Join-Path $global:root $global:ProvisioningVHDName # hard coded for testing for now.

# Tools called by this script.
$global:Tools = @($global:SerializeTool, $global:CvmUtil, $global:VMGSTool, $global:NewCVMCmdLet, $global:UnattendXmlCreatorTool, $global:PcrValuesWindowsFilePath, $global:PcrValuesLinuxFilePath, $global:RegionConfigurationJsonFile, 'TpmEngFWInit.dll', 'tmcoredll.dll', 'TpmEngUM138.dll', 'keyreference.json')

                  # Make all paths absolute once we have the packages.

#Output files
$global:KeyFile = Join-Path $global:outputFolder "keyfile"
$global:VmgsFile =  Join-Path $global:outputFolder "vmgs.vhd"
$global:KeyReferencePath = Join-Path $global:outputFolder "keyreference.json"
$global:outLog = Join-Path $global:logsFolder "cvmtool.out.log"
$global:errLog = Join-Path $global:logsFolder "cvmtool.err.log"
$global:vTPMFile = Join-Path $global:outputFolder "vtpm"

[environment]::CurrentDirectory=$pwd

if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) 
{
    Write-Warning "You are not running this as local administrator. Run it again in an elevated prompt." ; 
    break
}

if ($PSCmdlet.ParameterSetName -eq 'LinuxCVM')
{
    $global:Tools += $global:ProvisioningVHD
}

Push-Location $root
try {
    $accessTokenInfo = @{
        "accessToken" = ""
    }    
    if(($MaaEndpointUri.Length -eq 0) -or ($PublicKeyFile.Length -eq 0)){
        $userInfo = (az ad signed-in-user show) | Out-String | ConvertFrom-Json
        if ($lastexitcode -ne 0)
        {
            $loginInfo = az login
        }

        try
        {
            $accessTokenInfo = (az account get-access-token --resource https://managedhsm.azure.net) | Out-String  | ConvertFrom-Json
        }
        catch
        {
            throw "Got an exception in getting token for managed HSM, please check if you have access to https://managedhsm.azure.net. If you have access, please try passing the sub id explicitly using -subscription param" 
        }

        if ($lastexitcode -ne 0)
        {
            throw "Got an error in getting token for managed HSM please check if you have access to https://managedhsm.azure.net "
        }
    }

    if ([string]::IsNullOrEmpty($Subscription) -eq $false)
    {
        Write-Host "Setting subscription id - $($Subscription)"
        az account set --subscription $Subscription
        if ($lastexitcode -ne 0)
        {
            throw "Got an exception in setting subscription id"
        }
    }

    ValidatePresent $global:Tools
    TestHyperV
    $OsVhdPath = Resolve-Path $OsVhdPath
    Write-Host "This tool performs in place encryption, encrypting disk $($OsVhdPath) " -ForegroundColor Yellow

    $ret = Start-Process $global:CvmUtil -NoNewWindow -ArgumentList "-createvtpmblob $($global:vTPMFile)" -PassThru -Wait -RedirectStandardOutput $global:outLog -RedirectStandardError $global:errLog
    if (0 -ne $ret.ExitCode)
    {
        throw " Error calling $($global:CvmUtil) failed with error $($ret.ExitCode ), Arguments were -createvtpmblob $($global:vTPMFile)"
    }

    CreateVMGS -VMGSFile $global:VmgsFile -VTPMFile $vtpmFile -Token $accessTokenInfo.accessToken.ToString() -KeyUri $VMGSEncryptionKey -OsType $OsType
    Write-Host "Created CVM compatible vmgs file $($vmgsFile) which has the guest state." -ForegroundColor Green
    Write-Output $vmgsFile > vgms_filename.temp 

    switch ($PSCmdlet.ParameterSetName) {

    'LinuxCVM' {
        
        if ($OsType -ne "Linux")
        {
            throw "Parameter types do not match the OsType"
        }
        $bootKey, $rootKey = GetKeysObject
        if($SBoff)
        {
            $sealedKeys = CreateSealedKeys -BootKey $bootKey.ToString() -RootKey $rootKey.ToString() -PcrConfig $global:PcrValuesLinuxFilePathSbOff -VTPMFile $vtpmFile
        }
        else 
        {
            $sealedKeys = CreateSealedKeys -BootKey $bootKey.ToString() -RootKey $rootKey.ToString() -PcrConfig $global:PcrValuesLinuxFilePath -VTPMFile $vtpmFile
        }
        [bool] $initramfsPatching = !$NoInitramfsPatching
        CreateEncryptedOsVHDLinux -ProvisioningVHD $global:ProvisioningVHD -OsVHD $OsVhdPath -Username $UserName -SshPublicKey $SshKey -BootKey $bootKey.ToString() -RootKey $rootKey.ToString() -SealedKeysFile $sealedKeys -LogsFolder $global:logsFolder -Encryption $Encryption -DisableWaAgent $DisableWaAgent -InitramfsPatching $initramfsPatching
        Write-Host "Created CVM compatible disk $($OsVhdPath) which is encrypted and has the user configuration provisioned." -ForegroundColor Green  
        if ($Test -ne $true)
        {
            Remove-Item -Path $sealedKeys | Out-Null
            Remove-Item -Path $vtpmFile | Out-Null
        }
    }

    'WindowsCVM' 
    {
        if ($OsType -ne "Windows")
        {
            throw "Parameter types do not match the OsType. Do .GetHelp CVMTool.ps1 and use correct commandline."
        }

        if ($Test)
        {
            PrepareAndEncryptWindowsVhdForCvm -VhdPath $($OsVhdPath) -PcrJsonPath $($global:PcrValuesWindowsFilePath) -VtpmBlobPath $($vtpmFile) -LocalAccountUsername $($userName) -LocalAccountPassword $($Password) -CvmUtil $($global:CvmUtil) -UnattendXmlCreatorTool $($global:UnattendXmlCreatorTool) -RecoveryKeyFilePathToSave $global:outputFolder -DebugMode
        }
        else
        {
            PrepareAndEncryptWindowsVhdForCvm -VhdPath $($OsVhdPath) -PcrJsonPath $($global:PcrValuesWindowsFilePath) -VtpmBlobPath $($vtpmFile) -LocalAccountUsername $($userName) -LocalAccountPassword $($Password) -CvmUtil $($global:CvmUtil) -UnattendXmlCreatorTool $($global:UnattendXmlCreatorTool) -RecoveryKeyFilePathToSave $global:outputFolder
        }
        
        Write-Host "Succesfully encrypted $($OsVhdPath)." -ForegroundColor Green
    }
   }
 
}
finally {
    Pop-Location
    [environment]::CurrentDirectory = $pwd
}
Write-Host "Time taken : $($Stopwatch.Elapsed)"