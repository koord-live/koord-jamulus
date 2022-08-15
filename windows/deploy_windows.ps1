param (
    # Replace default path with system Qt installation folder if necessary
    [string] $QtPath = "C:\Qt",
    [string] $QtInstallPath = "C:\Qt\6.3.0",

    [string] $QtInstallPath32 = "C:\Qt\6.3.0",
    [string] $QtInstallPath64 = "C:\Qt\6.3.0",
    [string] $QtCompile32 = "msvc2019",
    [string] $QtCompile64 = "msvc2019_64",
    # Important:
    # - Do not update ASIO SDK without checking for license-related changes.
    # - Do not copy (parts of) the ASIO SDK into the Jamulus source tree without
    #   further consideration as it would make the license situation more complicated.
    [string] $AsioSDKName = "asiosdk_2.3.3_2019-06-14",
    [string] $AsioSDKUrl = "https://download.steinberg.net/sdk_downloads/asiosdk_2.3.3_2019-06-14.zip",
    # [string] $InnoSetupIsccPath = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    [string] $MSIXPkgToolUrl = "https://download.microsoft.com/download/6/f/e/6fec9d4c-f570-4826-995a-5feba065fa8b/MSIXPackagingTool_1.2022.110.0.msixbundle",
    [string] $MsixPkgToolPath = "C:\Program Files\WindowsApps\Microsoft.MSIXPackagingTool_1.2022.330.0_x64__8wekyb3d8bbwe\MsixPackagingToolCLI.exe",
    # [string] $VsDistFile64Redist = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Redist\",
    # [string] $VsDistFile64Redist = "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Redist\",
    [string] $VsDistFile64Path = "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Redist\MSVC\14.32.31326\x64\Microsoft.VC143.CRT",
    [string] $BuildOption = ""
)

# Fail early on all errors
$ErrorActionPreference = "Stop"

# change directory to the directory above (if needed)
Set-Location -Path "$PSScriptRoot\..\"

# Global constants
$RootPath = "$PWD"
$BuildPath = "$RootPath\build"
$DeployPath = "$RootPath\deploy"
$WindowsPath ="$RootPath\windows"
$AppName = "Koord-RT"

# Stop at all errors
$ErrorActionPreference = "Stop"

# Execute native command with errorlevel handling
Function Invoke-Native-Command {
    param(
        [string] $Command,
        [string[]] $Arguments
    )

    & "$Command" @Arguments

    if ($LastExitCode -Ne 0)
    {
        Throw "Native command $Command returned with exit code $LastExitCode"
    }
}

# Cleanup existing build folders
Function Clean-Build-Environment
{
    if (Test-Path -Path $BuildPath) { Remove-Item -Path $BuildPath -Recurse -Force }
    if (Test-Path -Path $DeployPath) { Remove-Item -Path $DeployPath -Recurse -Force }

    New-Item -Path $BuildPath -ItemType Directory
    New-Item -Path $DeployPath -ItemType Directory
}

# For sourceforge links we need to get the correct mirror (especially NISIS) Thanks: https://www.powershellmagazine.com/2013/01/29/pstip-retrieve-a-redirected-url/
Function Get-RedirectedUrl {

    param(
        [Parameter(Mandatory=$true)]
        [String]$URL
    )

    $request = [System.Net.WebRequest]::Create($url)
    $request.AllowAutoRedirect=$true
    $response=$request.GetResponse()
    $response.ResponseUri.AbsoluteUri
    $response.Close()
}

function Initialize-Module-Here ($m) { # see https://stackoverflow.com/a/51692402

    # If module is imported say that and do nothing
    if (Get-Module | Where-Object {$_.Name -eq $m}) {
        Write-Output "Module $m is already imported."
    }
    else {

        # If module is not imported, but available on disk then import
        if (Get-Module -ListAvailable | Where-Object {$_.Name -eq $m}) {
            Import-Module $m
        }
        else {

            # If module is not imported, not available on disk, but is in online gallery then install and import
            if (Find-Module -Name $m | Where-Object {$_.Name -eq $m}) {
                Install-Module -Name $m -Force -Verbose -Scope CurrentUser
                Import-Module $m
            }
            else {

                # If module is not imported, not available and not in online gallery then abort
                Write-Output "Module $m not imported, not available and not in online gallery, exiting."
                EXIT 1
            }
        }
    }
}

# Download and uncompress dependency in ZIP format
Function Install-Dependency
{
    param(
        [Parameter(Mandatory=$true)]
        [string] $Uri,
        [Parameter(Mandatory=$true)]
        [string] $Name,
        [Parameter(Mandatory=$true)]
        [string] $Destination
    )

    if (Test-Path -Path "$WindowsPath\$Destination") { return }

    $TempFileName = [System.IO.Path]::GetTempFileName() + ".zip"
    $TempDir = [System.IO.Path]::GetTempPath()

    if ($Uri -Match "downloads.sourceforge.net")
    {
      $Uri = Get-RedirectedUrl -URL $Uri
    }

    Invoke-WebRequest -Uri $Uri -OutFile $TempFileName
    echo $TempFileName
    Expand-Archive -Path $TempFileName -DestinationPath $TempDir -Force
    echo $WindowsPath\$Destination
    Move-Item -Path "$TempDir\$Name" -Destination "$WindowsPath\$Destination" -Force
    Remove-Item -Path $TempFileName -Force
}

# Install VSSetup (Visual Studio detection), ASIO SDK and InnoSetup
Function Install-Dependencies
{
    if (-not (Get-PackageProvider -Name nuget).Name -eq "nuget") {
      Install-PackageProvider -Name "Nuget" -Scope CurrentUser -Force
    }
    Initialize-Module-Here -m "VSSetup"
    Install-Dependency -Uri $AsioSDKUrl `
        -Name $AsioSDKName -Destination "ASIOSDK2"

    # install MSIX Packaging Tool
    # Install  bundle
    
}

# Setup environment variables and build tool paths
Function Initialize-Build-Environment
{
    param(
        [Parameter(Mandatory=$true)]
        [string] $BuildArch
    )

    # Look for Visual Studio/Build Tools 2017 or later (version 15.0 or above)
    $VsInstallPath = Get-VSSetupInstance | `
        Select-VSSetupInstance -Product "*" -Version "17.0" -Latest | `
        Select-Object -ExpandProperty "InstallationPath"

    if ($VsInstallPath -Eq "") { $VsInstallPath = "<N/A>" }

    if ($BuildArch -Eq "x86_64")
    {
        $VcVarsBin = "$VsInstallPath\VC\Auxiliary\build\vcvars64.bat"
    }
    else
    {
        $VcVarsBin = "$VsInstallPath\VC\Auxiliary\build\vcvars32.bat"
    }

    # # Setup Qt executables paths for later calls
    # Set-Item Env:QtQmakePath "$QtMsvcSpecPath\qmake.exe"
    # Set-Item Env:QtCmakePath  "C:\Qt\Tools\CMake_64\bin\cmake.exe" #FIXME should use $Env:QtPath
    # Set-Item Env:QtWinDeployPath "$QtMsvcSpecPath\windeployqt.exe"

    ""
    "**********************************************************************"
    "Using Visual Studio/Build Tools environment settings located at"
    $VcVarsBin
    "**********************************************************************"
    ""

    if (-Not (Test-Path -Path $VcVarsBin))
    {
        Throw "Microsoft Visual Studio ($BuildArch variant) is not installed. " + `
            "Please install Visual Studio 2017 or above it before running this script."
    }

    # Import environment variables set by vcvarsXX.bat into current scope
    $EnvDump = [System.IO.Path]::GetTempFileName()
    Invoke-Native-Command -Command "cmd" `
        -Arguments ("/c", "`"$VcVarsBin`" && set > `"$EnvDump`"")

    foreach ($_ in Get-Content -Path $EnvDump)
    {
        if ($_ -Match "^([^=]+)=(.*)$")
        {
            Set-Item "Env:$($Matches[1])" $Matches[2]
        }
    }

    Remove-Item -Path $EnvDump -Force
}

# Setup Qt environment variables and build tool paths
Function Initialize-Qt-Build-Environment
{
    param(
        [Parameter(Mandatory=$true)]
        [string] $QtInstallPath,
        [Parameter(Mandatory=$true)]
        [string] $QtCompile
    )

    $QtMsvcSpecPath = "$QtInstallPath\$QtCompile\bin"

    # Setup Qt executables paths for later calls
    Set-Item Env:QtQmakePath "$QtMsvcSpecPath\qmake.exe"
    Set-Item Env:QtCmakePath  "C:\Qt\Tools\CMake_64\bin\cmake.exe" #FIXME should use $Env:QtPath
    Set-Item Env:QtWinDeployPath "$QtMsvcSpecPath\windeployqt.exe"

    "**********************************************************************"
    "Using Qt binaries for Visual C++ located at"
    $QtMsvcSpecPath
    "**********************************************************************"
    ""

    if (-Not (Test-Path -Path $Env:QtQmakePath))
    {
        Throw "The Qt binaries for Microsoft Visual C++ 2017 or above could not be located at $QtMsvcSpecPath. " + `
            "Please install Qt with support for MSVC 2017 or above before running this script," + `
            "then call this script with the Qt install location, for example C:\Qt\6.3.0"
    }
}

# Build Jamulus x86_64 and x86
Function Build-App
{
    param(
        [Parameter(Mandatory=$true)]
        [string] $BuildConfig,
        [Parameter(Mandatory=$true)]
        [string] $BuildArch
    )

    # Build kdasioconfig Qt project with CMake / nmake
    # # Build FlexASIO dlls with CMake / nmake
    Invoke-Native-Command -Command "$Env:QtCmakePath" `
        -Arguments ("-DCMAKE_PREFIX_PATH='$QtInstallPath\$QtCompile64\lib\cmake'", `
            "-DCMAKE_BUILD_TYPE=Release", `
            "-S", "$RootPath\KoordASIO\src\kdasioconfig", `
            "-B", "$BuildPath\$BuildConfig\kdasioconfig", `
            "-G", "NMake Makefiles")
    Set-Location -Path "$BuildPath\$BuildConfig\kdasioconfig"
    Invoke-Native-Command -Command "nmake" #FIXME necessary??
    
    Set-Location -Path "$RootPath"

    # Ninja! 
    Invoke-Native-Command -Command "$Env:QtCmakePath" `
        -Arguments ("-S", "$RootPath\KoordASIO\src", `
            "-B", "$BuildPath\$BuildConfig\flexasio", `
            "-G", "Ninja", `
            "-DCMAKE_BUILD_TYPE=Release")

    # Build!
    Invoke-Native-Command -Command "$Env:QtCmakePath" `
        -Arguments ("--build", "$BuildPath\$BuildConfig\flexasio")

    # Now build rest of Koord-Realtime
    Invoke-Native-Command -Command "$Env:QtQmakePath" `
        -Arguments ("$RootPath\$AppName.pro", "CONFIG+=$BuildConfig $BuildArch $BuildOption", `
        "-o", "$BuildPath\Makefile")

    # Compile
    Set-Location -Path $BuildPath
    if (Get-Command "jom.exe" -ErrorAction SilentlyContinue)
    {
        echo "Building with jom /J ${Env:NUMBER_OF_PROCESSORS}"
        Invoke-Native-Command -Command "jom" -Arguments ("/J", "${Env:NUMBER_OF_PROCESSORS}", "$BuildConfig")
    }
    else
    {
        echo "Building with nmake (install Qt jom if you want parallel builds)"
        Invoke-Native-Command -Command "nmake" -Arguments ("$BuildConfig")
    }
    Invoke-Native-Command -Command "$Env:QtWinDeployPath" `
        -Arguments ("--$BuildConfig", "--no-compiler-runtime", "--dir=$DeployPath\$BuildArch", `
        "--no-system-d3d-compiler",  "--no-opengl-sw", `
        "$BuildPath\$BuildConfig\kdasioconfig\KoordASIOControl_builtin.exe")
    # collect for Koord-RT.exe
    Invoke-Native-Command -Command "$Env:QtWinDeployPath" `
        -Arguments ("--$BuildConfig", "--no-compiler-runtime", "--dir=$DeployPath\$BuildArch", `
        "--no-system-d3d-compiler", "--qmldir=$RootPath\src", `
        "-webenginecore", "-webenginewidgets", "-webview", "-qml", "-quick", `
        "$BuildPath\$BuildConfig\$AppName.exe")

    Move-Item -Path "$BuildPath\$BuildConfig\$AppName.exe" -Destination "$DeployPath\$BuildArch" -Force

    # get visibility on deployed files
    Tree "$DeployPath\$BuildArch" /f /a

    # Manually copy in webengine exe
    Copy-Item -Path "$QtInstallPath64/$QtCompile64/bin/QtWebEngineProcess.exe" -Destination "$DeployPath\$BuildArch"

    # Transfer VS dist DLLs for x64
    Copy-Item -Path "$VsDistFile64Path\*" -Destination "$DeployPath\$BuildArch"
        # Also add KoordASIO build files:
            # kdasioconfig files inc qt dlls now in 
                # D:/a/KoordASIO/KoordASIO/deploy/x86_64/
                    # - KoordASIOControl_builtin.exe
                    # all qt dlls etc ...
            # flexasio files in:
                # D:\a\KoordASIO\KoordASIO\src\out\install\x64-Release\bin
                    # - KoordASIO.dll
                    # - portaudio.dll 
    # Move KoordASIOControl_builtin.exe to deploy dir
    Move-Item -Path "$BuildPath\$BuildConfig\kdasioconfig\KoordASIOControl_builtin.exe" -Destination "$DeployPath\$BuildArch" -Force
    # Move all KoordASIO dlls and exes to deploy dir
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\ASIOTest.dll" -Destination "$DeployPath\$BuildArch" -Force
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\FlexASIOTest.exe" -Destination "$DeployPath\$BuildArch" -Force
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\KoordASIO.dll" -Destination "$DeployPath\$BuildArch" -Force
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\portaudio.dll" -Destination "$DeployPath\$BuildArch" -Force
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\PortAudioDevices.exe" -Destination "$DeployPath\$BuildArch" -Force
    Move-Item -Path "$RootPath\KoordASIO\src\out\install\x64-Release\bin\sndfile.dll" -Destination "$DeployPath\$BuildArch" -Force

    # move InnoSetup script to deploy dir
    Move-Item -Path "$WindowsPath\kdinstaller.iss" -Destination "$RootPath" -Force

    # clean up
    Invoke-Native-Command -Command "nmake" -Arguments ("clean")
    Set-Location -Path $RootPath
}

# Build and deploy Koord-RT 64bit and 32bit variants
function Build-App-Variants
{
    # foreach ($_ in ("x86_64", "x86"))
    foreach ($_ in ("x86_64"))
    {
        $OriginalEnv = Get-ChildItem Env:
        if ($_ -eq "x86")
        {
            Initialize-Build-Environment -BuildArch $_
            Initialize-Qt-Build-Environment -QtInstallPath $QtInstallPath32 -QtCompile $QtCompile32
        }
        else
        {
            Initialize-Build-Environment -BuildArch $_
            Initialize-Qt-Build-Environment -QtInstallPath $QtInstallPath64 -QtCompile $QtCompile64
        }
        Build-App -BuildConfig "release" -BuildArch $_
        $OriginalEnv | % { Set-Item "Env:$($_.Name)" $_.Value }
    }
}

# Build Windows installer
Function Build-Installer
{
    # unused for now
    param(
        [string] $BuildOption
    )

    foreach ($_ in Get-Content -Path "$RootPath\$AppName.pro")
    {
        if ($_ -Match "^VERSION *= *(.*)$")
        {
            $AppVersion = $Matches[1]
            break
        }
    }

    #FIXME for 64bit build only
    Set-Location -Path "$RootPath"
    # /Program Files (x86)/Inno Setup 6/ISCC.exe
    # Invoke-Native-Command -Command "${InnoSetupIsccPath}" `
    Invoke-Native-Command -Command "iscc" `
        -Arguments ("$RootPath\kdinstaller.iss", `
         "/FKoord-RT-${AppVersion}")
}

# Build MSIX Package
#FIXME this is all totally heinous
# Function Build-MSIX-Package
# {
#     # set elevated / admin privileges??
#     #FIXME does this even work?
#     Set-ExecutionPolicy Bypass -Scope LocalMachine -Force

#     #debug permissions
#     whoami /groups

#     # Install MSIXPackagingTool
#     # Using download tool and pattern from https://flexxible.com/automating-msix-packaging-with-powershell/
#     echo ">>> Downloading MsixPackagingTool installer ..."

#     & "$WindowsPath\Get_Store_Downloads.ps1" -packageFamilyName Microsoft.MsixPackagingTool_8wekyb3d8bbwe `
#         -downloadFolder C:\store-apps -excludeRegex '_arm__|_x86__|_arm64__'

#     # enable Windows Update service to allow driver installation to succeed!
#     # echo ">>> Enabling Windows Update service ..."
#     # Start-Service wuauserv

#     echo ">>> Installing MsixPackagingTool ..."
#     #FIXME - the version of tool and therefore path WILL change - need to get dynamically
#     # Check previous output for actual output path of download
#     Add-AppxPackage -Path 'C:\store-apps\Microsoft.MSIXPackagingTool_2022.330.739.0_neutral_~_8wekyb3d8bbwe.msixbundle' `
#         -Confirm:$false -ForceUpdateFromAnyVersion -InstallAllResources

#     echo ">>> Outputting MSIX Package Driver version..."
#     dism /online /Get-Capabilities | Select-String -pattern '(\bmsix\.PackagingTool\.Driver\b.*$)'

#     echo ">>> installing MSIX Package Driver....."
#     dism /online /Get-Capabilities | Select-String -pattern '(\bmsix\.PackagingTool\.Driver\b.*$)' `
#         |Select-Object -ExpandProperty Matches|Select-Object -ExpandProperty Value| `
#         ForEach-Object { dism /online /add-capability /capabilityname:$_ }

#     echo ">>> Outputting all installed AppxPackage ....."
#     Get-AppxPackage -all | Where PackageFamilyName -match '_8wekyb3d8bbwe' | sort Name -Unique | select Name,PackageFamilyName

#     echo ">>> Outputting details of installed tool: Microsoft.MSIXPackagingTool"
#     Get-AppxPackage -Name Microsoft.MSIXPackagingTool

#     # debug - list output of tool installation directory
#     dir "C:\Program Files\WindowsApps\Microsoft.MSIXPackagingTool_1.2022.330.0_x64__8wekyb3d8bbwe"

#     echo ">>> Invoking MsixPackagingTool ...."    
#     Invoke-Native-Command -Command "$MsixPkgToolPath" `
#         -Arguments ("create-package", "--template", "$WindowsPath\appXmanifest.xml")
#     # & "$MsixPkgToolPath" create-package --template "$WindowsPath\msix_template.xml"
# }

Clean-Build-Environment
Install-Dependencies
Build-App-Variants
Build-Installer -BuildOption $BuildOption
# Build-MSIX-Package