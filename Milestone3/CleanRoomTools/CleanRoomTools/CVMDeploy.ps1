<#PSScriptInfo

.VERSION 1.0

.GUID e73ad37c-b405-48c0-8cff-001d1983aa28

.AUTHOR cvmps@microsoft.com
#>

<#

.DESCRIPTION
 Script to provision CVM from VHD and VMGS

#> 

# Do Get-Help .\CVMDeploy.ps1 to see the ways you can run this script

[CmdletBinding()]
    param(
        [Parameter(Mandatory=$true, HelpMessage="Azure resource group that you want to provision CVM to")]
        [Alias('R')]
        [string] $ResourceGroup,

        [Parameter(Mandatory=$true, HelpMessage="Region specific configurations, example eastus")]
        [Alias('M')]
        [string] $Region,

        [Parameter(Mandatory=$true, HelpMessage="Private preview access token from welcome mail")]
        [Alias('T')]
        [string] $SASToken,

        [Parameter(Mandatory=$true, HelpMessage="Virtual machine size")]
        [Alias('S')]
        [ValidateSet ("Standard_DC1as_v4", 
                      "Standard_DC2as_v4",
                      "Standard_DC8as_v4",
                      "Standard_DC16as_v4",
                      "Standard_DC32as_v4",
                      "Standard_DC48as_v4",
                      "Standard_DC96as_v4")]
        [string] $VMSize,

        [Parameter(Mandatory=$true, HelpMessage="Virtual machine name")]
        [Alias('N')]
        [string] $VMName,

        [Parameter(Mandatory=$true, HelpMessage="VHD file path")]
        [ValidateScript({
            $osVHDExists = test-path $_ -pathtype leaf
            if ($osVHDExists -eq $false)
            {
                throw "Supplied OsVhdPath argument $_ does not exist."
            }

            $true
        })]
        [string] $OsVhdPath,

        [Parameter(Mandatory=$true, HelpMessage="VMGS file path")]
        [ValidateScript({
            $osVMGSExists = test-path $_ -pathtype leaf
            if ($osVMGSExists -eq $false)
            {
                throw "Supplied VMGSPath argument $_ does not exist."
            }

            $true
        })]
        [string] $VMGSPath,

        [Parameter(Mandatory=$true, HelpMessage="OS type of the CVM that you are provisoning")]
        [ValidateSet ("Linux", "Windows")]
        [string] $OSType,

        [Parameter(Mandatory=$false, HelpMessage="Storage account SKU of the guest image files that you are uploading")]
        [ValidateSet ("Premium_LRS", "Standard_LRS")]
        [string] $DiskSku = "Premium_LRS",

        [Parameter(Mandatory=$false, HelpMessage="Storage account type of the guest image files that you are uploading")]
        [ValidateSet("BlobStorage", "Storage", "StorageV2")]
        [string] $StorageKind = "StorageV2",

        [Parameter(Mandatory=$false, HelpMessage="Hyper-V generation")]
        [Alias('G')]
        [ValidateSet ("V1", "V2")]
        [string] $HyperVGen = "V2",

        [Parameter(Mandatory=$false, HelpMessage="Secure boot setting of the CVM")]
        [ValidateSet ("true", "false")]
        [string] $SecureBootEnabled = "true",

        [Parameter(Mandatory=$false, HelpMessage="vTPM setting of the CVM")]
        [ValidateSet ("true", "false")]
        [string] $VTPMEnabled = "true",

        [Parameter(Mandatory=$false, HelpMessage="Subscription id")]
        [ValidateScript({
            try {
                [System.Guid]::Parse($_) | Out-Null
                $true
            } catch {
                throw "Subscription id should be a guid"
            }
        })]
        [string] $Subscription
    )


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
        return $false
    }
    return $true
}

<#
    Delete storage account from resource group
#>
function DeleteStorageAccount
{
    param(
        [Parameter(Mandatory = $true)]
        $storageAccountName
    )

    Write-Host "Deleting storage account $($storageAccountName)..."
    az storage account delete -n $storageAccountName -g $ResourceGroup -y
}

#end-region


$Stopwatch =  [system.diagnostics.stopwatch]::StartNew()

# Tools and templates
$global:root = Split-Path -Parent $MyInvocation.MyCommand.Path
$global:AzCopy = Join-Path $global:root "AzCopy.exe"
[environment]::CurrentDirectory=$pwd

$containerName = "vhds"
$cvmDeployTemplate = "https://cvmprivatepreviewsa.blob.core.windows.net/cvmprivatepreviewcontainer/deploymentTemplate/deployCVM.json"

if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) 
{
    Write-Warning "You are not running this as local administrator. Run it again in an elevated prompt." ;
    break
}


Push-Location $root
try {
    # $userInfo = (az ad signed-in-user show) | Out-String | ConvertFrom-Json
    # if ($lastexitcode -ne 0)
    # {
        # $loginInfo = az login
    # }

    # if ([string]::IsNullOrEmpty($Subscription) -eq $false)
    # {
        # Write-Host "Setting subscription id - $($Subscription)"
        # az account set --subscription $Subscription
        # if ($lastexitcode -ne 0)
        # {
            # throw "Got an exception in setting subscription id"
        # }
    # }

    # Check tools
    if ((CheckIfFileIsPresent $global:AzCopy) -ne $true)
    {
        throw  "Missing required tool 'AzCopy': '$global:AzCopy'"
    }

    $vhdName = Split-Path -Path $OsVhdPath -Leaf -Resolve
    $vmgsName = Split-Path -Path $VMGSPath -Leaf -Resolve

    # Create resource group
    Write-Host "Creating resource group $($ResourceGroup)..." -ForegroundColor Yellow
    az group create -l $Region -n $ResourceGroup
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in creating resource group"
    }
    az configure --defaults group=$ResourceGroup
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in setting default resource group"
    }

    # Check for duplicate VM names before deployment
    Write-Host "Verifying VM Name for duplicates" -ForegroundColor Yellow
    $VMList = (az vm list -g $ResourceGroup) | ConvertFrom-Json
    foreach ($element in $VMList) {
        if($element.name -eq $VMName) {
            throw "Virtual Machine with name $($VMName) already exists in resource group"
        }
    }

    # Create storage account with a unique name
    $rgBase = ($ResourceGroup.ToLower() -replace '[^a-z0-9]')
    if ($rgBase.Length -gt 6) {
        $rgBase = $rgBase.Substring(0, 6)
    }
    if ($rgBase.Length -lt 1)
    {
        $rgBase = "vhdstr"
    }
    do {
        $storageAccountName = $rgBase + (Get-Random -Maximum 1000000)
        Write-Host "Checking storage account name $($storageAccountName)..." -ForegroundColor Yellow
    } until ((az storage account check-name --name $storageAccountName  | Out-String | ConvertFrom-Json).nameAvailable -eq $true)

    Write-Host "Creating storage account $($storageAccountName)..." -ForegroundColor Yellow
    $storageAccount = (az storage account create -n $storageAccountName -l $Region --sku $DiskSku --kind $StorageKind) | Out-String | ConvertFrom-Json
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in creating storage account"
    }
    $storageAccountId=$storageAccount.id
    Write-Host "Storage account created (Id: $($storageAccountId))" -ForegroundColor Yellow

    $deploymentSuccess = $false

    # Storage account key
    $storageAccountKeys=(az storage account keys list --account-name $StorageAccountName) | Out-String | ConvertFrom-Json
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in getting storage account keys"
    }
    $storageAccountKey0=$storageAccountKeys.value[0]

    # Create container
    Write-Host "Creating storage account container $($containerName)..." -ForegroundColor Yellow
    az storage container create -n $containerName --account-name  $storageAccountName --account-key $storageAccountKey0
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in creating container"
    }

    # Create SAS token
    Write-Host "Creating ad-hoc container SAS token..." -ForegroundColor Yellow
    $end = (([datetime]::UtcNow).AddHours(8)).ToString("yyyy-MM-ddTHH:mmZ")
    $containerSASToken = az storage container generate-sas -n $containerName --account-name $storageAccountName --account-key $storageAccountKey0 --https-only --permissions dlrw --expiry $end -o tsv
    if ($lastexitcode -ne 0)
    {
        throw "Got an exception in creating container SAS token"
    }
    if (-not $containerSASToken.StartsWith('?'))
    {
        $containerSASToken = "?" + $containerSASToken
    }

    # Upload blobs
    $vmgsUrl = "https://$($storageAccountName).blob.core.windows.net/$($containerName)/$($vmgsName)"
    $vmgsUrlSAS = $vmgsUrl + $containerSASToken
    Write-Host "Uploading VMGS file to $($vmgsUrl)..." -ForegroundColor Yellow
    $ret = Start-Process $global:AzCopy -NoNewWindow -ArgumentList "copy $($VMGSPath) $($vmgsUrlSAS) --blob-type PageBlob" -PassThru -Wait
    if (0 -ne $ret.ExitCode)
    {
        throw "Got an exception in uploading $($VMGSPath) failed with error $($ret.ExitCode)"
    }

    $vhdUrl = "https://$($storageAccountName).blob.core.windows.net/$($containerName)/$($vhdName)"
    $vhdUrlSAS = $vhdUrl + $containerSASToken
    Write-Host "Uploading OS VHD File to $($vhdUrl)..." -ForegroundColor Yellow
    $ret = Start-Process $global:AzCopy -NoNewWindow -ArgumentList "copy $($OsVhdPath) $($vhdUrlSAS) --blob-type PageBlob" -PassThru -Wait
    if (0 -ne $ret.ExitCode)
    {
        throw "Got an exception in uploading $($OsVhdPath) failed with error $($ret.ExitCode)"
    }

    # Deploy VM
    if (-not $SASToken.StartsWith('?'))
    {
        $SASToken = "?" + $SASToken
    }
    $cvmDeployTemplateSAS = """" + $cvmDeployTemplate + $SASToken + """"

    Write-Host "Deploying CVM..." -ForegroundColor Yellow
    az deployment group create `
      --template-uri $cvmDeployTemplateSAS `
      -p osType=$OSType `
         osDiskURL=$vhdUrl `
         osDiskVmgsURL=$vmgsUrl `
         osDiskStorageAccountID=$storageAccountId `
         hyperVGeneration=$HyperVGen `
         vmName=$VMName `
         vmSize=$VMSize `
         secureBootEnabled=$SecureBootEnabled `
         vTpmEnabled=$VTPMEnabled

    if ($lastexitcode -eq 0)
    {
        Write-Host "Successfully deployed $($VMName)." -ForegroundColor Green
        $deploymentSuccess = $true
    }
}
finally {
    # Delete storage account upon deployment failure
    if($deploymentSuccess -eq $false)
    {
        Write-Host "Deployment failed" -ForegroundColor Red
        DeleteStorageAccount($storageAccountName)
    }
    Pop-Location
    [environment]::CurrentDirectory = $pwd
}
Write-Host "Time taken : $($Stopwatch.Elapsed)"