function Validate-BitlockerStatus
{
    $OutputVariable = (Get-WindowsOptionalFeature -Online -FeatureName Bitlocker)
    if(!($OutputVariable.state -eq "Enabled") ) 
    {
        throw "Bitlocker required for this operation. Enable bitlocker and try again. Quitting"
    }
}

function Validate-Tools
{
    Param(
        [parameter(Mandatory=$true)]
        [array] $FileList
        )
    $FileList | Where-Object { $_} | ForEach-Object {
         if (!(Test-Path $_ ))
         {
            throw "The required support files are not present. Could not find $($_), exiting."
         }
    }    
}

function Validate-File
{
    Param(
          [parameter(Mandatory=$true)]
          [string] $FilePath
          )
    if(!(Test-Path $FilePath))
    {
        throw "Could not find file at $($FilePath), please check that file exists"
    }
}          

#ToDo:function to validate account username based on azure VM requirments  of length(1-20), should not contain special characters and should not end with "."
function Username-Validation
{
Param(
    [Parameter(Mandatory=$true)]
    [string]$UserName
    )
    if($UserName.Length -le 5)
    {
        throw "Username should be 6 letters or more"    
    }    
}

#ToDo:function to validate account password based on azure VM requirements of length(12-123) characters and should have one each of lowercase letter, 
#uppercase letter,number and special character
function Password-Validation
{
Param(
    [Parameter(Mandatory=$true)]
    [string]$Password
    )
    if($Password.Length -le 5)
    {
        throw "Password should be 6 letters or more"
    }  
}

function Provision-UserSecrets
{
    Param(
      [Parameter(Mandatory=$true)]
      [string]$DriveLetter,
      [Parameter(Mandatory=$true)]
      [string]$Username,
      [Parameter(Mandatory=$true)]
      [string]$Password
      )
        
    Write-Host "Provisioning user secrets"
    $ret = Start-Process -FilePath $global:UnattendXmlCreatorTool -ArgumentList "-Username $LocalAccountUsername -Password $LocalAccountPassword" -PassThru -Wait
    if (0 -ne $ret.ExitCode)
    {
        Write-Host "Failed to generate unattend xml $global:UnattendXmlCreatorTool -Username $LocalAccountUsername -Password $LocalAccountPassword"    
        throw "Fatal: Failed to generate unattend xml"
    }
    Write-Host "Copying unattend.xml to vhd" 
    $unattendFileName = "unattend.xml"
    $pathForUnattend = ".\" + $unattendFileName
    $destinationFolderForUnattend = $DriveLetter + "\Windows\System32\Sysprep\"
    $fullPathForUnattend = $destinationFolderForUnattend + $unattendFileName
    if(Test-Path $fullPathForUnattend) 
    {
        $Ack = Read-Host "unattend.xml exist at $($destinationFolderForUnattend), should it be overwritten with the new generated ? Y or N  "
        while("y","n" -notcontains $Ack.ToLower())
        {
            $Ack = Read-Host "unattend.xml exist at $($destinationFolderForUnattend), should it be overwritten with the new generated ? Y or N  "
        }
        if("y" -contains $Ack.ToLower()) 
        {
            Write-Host "Moving generated xml after user consent"
            Move-Item -Path $pathForUnattend -Destination $destinationFolderForUnattend -Force -ErrorAction Stop
        }
        if("n" -contains $Ack.ToLower())
        {
            $Ack = Read-Host "Continue with existing unattend.xml ? Enter y to continue, n to quit "
            while("y","n" -notcontains $Ack.ToLower())
            {
                $Ack = Read-Host "Continue with existing unattend.xml ? Enter y to continue, n to quit "
            }
            if("n" -contains $Ack.ToLower())
            {
                Write-Host "Quitting"
                exit 1
            }
        }
    }
    else
    {
        Move-Item -Path $pathForUnattend -Destination $destinationFolderForUnattend -ErrorAction Stop
    }
}

function Encrypt-Volume
{
    Param(
      [Parameter(Mandatory=$true)]
      [string]$MountPoint,
      [Parameter(Mandatory=$true)]
      [Boolean]$EnableFullDiskEncryption
      )
    Write-Host "Enabling BitLocker in suspended mode"    
    if(!$EnableFullDiskEncryption)
    {
        $status=manage-bde -on $MountPoint -usedspaceonly
    }
    else
    {
        $status=manage-bde -on $MountPoint
    }
    for($i=0;$i -lt $status.Length;$i++)
    {
        if($status[$i].Contains("ERROR:"))
        {
            throw "Bitlocker failed to run for mountpoint $($MountPoint)"
        }
    }
    $status = Get-BitLockerVolume -MountPoint $MountPoint
    if ($status -eq $null)
    {
        throw "Failed to mount volume"
    } 
    if ($status.VolumeStatus -eq 'EncryptionInProgress')
    {
        Write-Host "Waiting for encryption to complete..."
        while ($status.VolumeStatus -ne 'FullyEncrypted')
        {
            Write-Progress -Activity "Encryption in Progress" -Status "$($status.EncryptionPercentage)% Complete:" -PercentComplete $status.EncryptionPercentage;           
            Start-Sleep -Milliseconds 300;
            $status = Get-BitLockerVolume -MountPoint $mountPoint;
        }
        Write-Progress -Activity "Encryption in Progress" -Status "$($status.EncryptionPercentage)% Complete:" -PercentComplete 100;
    }
    Write-Host "Volume is encrypted"
}

function Add-TPM-Protectors
{
    Param(
      [Parameter(Mandatory=$true)]
      [string]$DriveLetter,
      [Parameter(Mandatory=$true)]
      [string]$VtpmBlobPath,
      [Parameter(Mandatory=$true)]
      [string]$PcrJsonPath
      )

    $PcrJsonPath = $PcrJsonPath | SanitizePath
    Write-Host "Adding tpm protectors for $($VtpmBlobPath) $($PcrJsonPath) $($DriveLetter)" -Verbose
    $ret = Start-Process $global:CvmEncryptionTool -NoNewWindow "-addTpmProtector $($VtpmBlobPath) $($PcrJsonPath) $($DriveLetter)" -PassThru -Wait
    if (0 -ne $ret.ExitCode)
    {
       throw " Failed to add TPM protectors"
    }
}

function Add-Recovery-Password
{
    Param(
      [Parameter(Mandatory=$true)]
      [string]$MountPoint,  
      [Parameter(Mandatory=$true)]
      [string]$RecoveryKeyFilePathToSave
      )
    
    Write-Host "Adding recovery password"   
    $status = Add-BitLockerKeyProtector -MountPoint $mountPoint -RecoveryPasswordProtector
    if($status -eq $null)
    {
        throw "Failed to add Recovery Password"
    }    
    New-Item -Path $RecoveryKeyFilePathToSave -ItemType "file" -Force -ErrorAction Stop
    Set-Content -Path $RecoveryKeyFilePathToSave -Value $status.KeyProtector.RecoveryPassword
    Write-Host "Recovery Key saved to file : $($RecoveryKeyFilePathToSave)" -ForegroundColor Yellow
       
}

function EnableBitlockerFeature
{
    Param(
        [Parameter(Mandatory=$true)]
        [string]$MountPoint)

    $bitlockerFeature = Get-WindowsOptionalFeature -Path $MountPoint -FeatureName Bitlocker -ErrorAction Stop
    if($null -eq $bitlockerFeature)
    {
        throw "Failed to retrieve bitlocker feature from mount point"
    }

    if ($bitlockerFeature.State -eq  'Disabled')
    {
        Write-Host "Enabling Bitlocker feature"
        Enable-WindowsOptionalFeature -Path $MountPoint -FeatureName Bitlocker -All -ErrorAction Stop
    }
}

function Enable_ExternalDebugIp
{
    Param(
          [parameter(Mandatory=$true)]
          [string] $DriveLetter
          )

    $DrivePath = $DriveLetter + "\windows\system32\config\system"
    $status=reg load HKLM\VHDSYS $DrivePath
    if($lastexitcode -ne 0)
    {
        throw "Failed to load registry parameter"
    }
    $status=reg add "HKLM\VHDSYS\ControlSet001\Services\vmbus\Parameters" /v AllowAllDevicesWhenIsolated /t REG_DWORD /d 1 /f
    if($lastexitcode -ne 0)
    {
        throw "Failed to set registry parameter for showing External IP"
    }
    $status=reg unload HKLM\VHDSYS
    if($lastexitcode -ne 0)
    {
        throw "Failed to unload registry parameter"
    }
}

Function Get-FirstAvailableDriveLetter()
{
    $letters = 65..89 | % { [char]$_ }
    return ($letters | ? { (New-Object System.IO.DriveInfo($_ + ":")).DriveType -eq 'NoRootDirectory'})[0]
}

function Reset-BCD
{
    Param(
        [Parameter(Mandatory=$true)]
        [string]$DriveLetter,
        [Parameter(Mandatory=$true)]
        [string]$VhdFilePath
        )
        
        $disk = (Get-VHD $VhdFilePath | Get-Disk)
        $part = $disk | Get-Partition | ? { $_.Type -eq "System" }
                
        if([string]::IsNullOrEmpty($part.DriveLetter))
        {
            $letter = Get-FirstAvailableDriveLetter
            Diskpart-AssignLetter $part $letter
        }
        else
        {
            $letter = $part.DriveLetter
        }
        
        $storeVolume = $letter +":"
        $storePath = $letter +":\" + "EFI\Microsoft\Boot\BCD"
        bcdedit /store $storePath /delete '{default}'
        if ($LastExitCode)        {
            
            throw "Failed to delete defualt BCD store entry!"
        }

        $windowsPath = $DriveLetter + ":\Windows"
        bcdboot $windowsPath /s $storeVolume /f UEFI
        if ($LastExitCode)
        {            
            throw "Failed to generate new BCD store entry!"
        }        
            
}

Function Diskpart-AssignLetter
{
    Param(
    [Parameter(Mandatory=$true)]
    [object]$partition,
    [Parameter(Mandatory=$true)]
    [char]$volLetter=$true
    )

    $dNum = $partition.DiskNumber
    $pNum = $partition.PartitionNumber

    Set-Partition -DiskNumber $dNum -PartitionNumber $pNum -NewDriveLetter $volLetter
    if ($LastExitCode)
    {
        throw "Failed to assign drive letter!"
    }
}

<#
    Fixes path in case of spaces
#>
function SanitizePath
{
    Param(
        [Parameter(Mandatory=$true, ValueFromPipeline = $true)]
        [string]$Path)
    return '"{0}"' -f $Path
}

function PrepareAndEncryptWindowsVhdForCvm
{
    [CmdletBinding()]
    Param(
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$VhdPath,
        [Parameter(Mandatory=$true)]
        [string]$LocalAccountUsername,
        [Parameter(Mandatory=$true)]
        [string]$LocalAccountPassword,
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$VtpmBlobPath,
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$PcrJsonPath,
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$CvmUtil,
        [Parameter(Mandatory=$true)]
        [ValidateScript({test-path $_ -pathtype leaf})]
        [string]$UnattendXmlCreatorTool,
        [Parameter(Mandatory=$false)]
        [switch] $EnableFullDiskEncryption=$false,
        [Parameter(Mandatory=$true)]
        [string]$RecoveryKeyFilePathToSave,
        [Parameter(Mandatory=$false)]
        [switch] $DebugMode=$false)  
    try
    {
        #Main Script
        $global:UnattendXmlCreatorTool = $UnattendXmlCreatorTool
        $global:CvmEncryptionTool = $CvmUtil
        $global:Tools = @($global:UnattendXmlCreatorTool, $global:CvmEncryptionTool)
        #
        #Validate Bitlocker
        #
        Validate-BitlockerStatus
        #
        #Validate Input parameters
        #
        Username-Validation -UserName $LocalAccountUsername
        Password-Validation -Password $LocalAccountPassword
        #
        #Mount VHD
        #
        $VhdPath = Resolve-Path $VhdPath
        Write-Host "Mounting VHD"
        Mount-DiskImage -ImagePath $VhdPath -ErrorAction Stop
        $volumes = Get-DiskImage -ImagePath $VhdPath | Get-Disk | Get-Partition | Get-Volume
        $volume = $volumes | Sort-Object Size -Descending | Select-Object -First 1
        if (-not($volume)) 
        {
            throw "Failed to find partition."
        }
        $DriveLetter = "$($volume.DriveLetter)".Trim()   
        if($DriveLetter -eq "")
        {
            $DriveLetter = Get-FirstAvailableDriveLetter
            $disk = (Get-VHD $VhdPath | Get-Disk)
            $part = $disk | Get-Partition | ? { $_.Type -eq "Basic" }                
            Diskpart-AssignLetter $part $DriveLetter
        }
        #
        #Check if drive is locked
        #
        $status =  Get-BitlockerVolume -MountPoint $DriveLetter -ErrorAction SilentlyContinue
        if($status.LockStatus -eq "Locked")
        {
            throw "Drive $($DriveLetter) is locked. Try again after unlocking the drive"
        }
        #
        #Check if disk is already encrypted and protections are enabled 
        #
        if($status.ProtectionStatus -eq "On")
        {
            throw "Disk is already encrypted with protection enabled. Quitting"
        }
        #
        #Reset BCD store
        #
        Reset-BCD -DriveLetter $DriveLetter -VhdFilePath $VhdPath
        #
        #Provision User secrets, encrypt disk, enable bitlocker add TPM protectors and Recovery Password
        #
        $DriveLetter = $DriveLetter+":"
        Provision-UserSecrets -DriveLetter $DriveLetter -Username $LocalAccountUsername -Password $LocalAccountPassword
        if($DebugMode)
        {
            Enable_ExternalDebugIp -DriveLetter $DriveLetter
        }
        EnableBitlockerFeature -MountPoint $DriveLetter
        Encrypt-Volume -mountPoint $DriveLetter -EnableFullDiskEncryption $EnableFullDiskEncryption
        Add-TPM-Protectors -DriveLetter $DriveLetter -VtpmBlobPath $VtpmBlobPath -PcrJsonPath $PcrJsonPath
        $RecoveryKeyFilePathToSave = $RecoveryKeyFilePathToSave + "\RecoveryKey.txt"
        Add-Recovery-Password -MountPoint $DriveLetter -RecoveryKeyFilePath $RecoveryKeyFilePathToSave
    }
    finally
    {
        #
        #Unmount VHD
        #
        Write-Host "Dismounting Disk"
        Dismount-DiskImage -ImagePath $VhdPath
    }
}
Export-ModuleMember -Function PrepareAndEncryptWindowsVhdForCvm
Export-ModuleMember -Function SanitizePath