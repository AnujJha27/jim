<#
.SYNOPSIS
    Build script for Jim Text Editor on Windows.
.DESCRIPTION
    This script locates Qt6, configures the build environment, and compiles Jim using MinGW or MSVC.
#>

$ErrorActionPreference = "Stop"

function Extract-PathFromWhere($command) {
    try {
        $paths = where.exe $command 2>$null
        if ($paths) { return $paths[0] }
    } catch { }
    return $null
}

Write-Host "Starting Jim build process..." -ForegroundColor Cyan

# Try to find qmake in PATH first
$qmakePath = Extract-PathFromWhere "qmake"
if ($qmakePath) {
    $QtBinPath = Split-Path $qmakePath -Parent
    Write-Host "Found qmake in PATH at: $QtBinPath" -ForegroundColor Green
} else {
    Write-Host "qmake not found in PATH. Searching common directories..." -ForegroundColor Yellow
    
    # Common Qt6 directories
    $QtPaths = @(
        "C:\Qt\6.*\msvc*\bin",
        "C:\Qt\6.*\mingw*\bin",
        "C:\Qt\Tools\mingw*\bin",
        "C:\Qt\bin"
    )

    $QtBinPath = $null
    foreach ($path in $QtPaths) {
        $resolved = Resolve-Path $path -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Path -First 1
        if ($resolved -and (Test-Path "$resolved\qmake.exe")) {
            $QtBinPath = $resolved
            break
        }
    }

    if (-not $QtBinPath) {
        Write-Error "Could not automatically find Qt6 bin directory. Please add C:\Qt\6.x.x\msvc2019_64\bin to your Windows PATH and run this script again."
        exit 1
    }
    
    Write-Host "Found Qt6 at: $QtBinPath" -ForegroundColor Green
    # Add Qt to PATH for this session
    $env:PATH = "$QtBinPath;$env:PATH"
}

# If using MinGW, we also need the actual compiler in PATH
if ($QtBinPath -like "*mingw*") {
    $MinGwPath = Extract-PathFromWhere "mingw32-make"
    if (-not $MinGwPath) {
        Write-Host "mingw32-make not found in PATH. Searching in C:\Qt\Tools..." -ForegroundColor Yellow
        $ToolsPaths = @("C:\Qt\Tools\mingw*\bin")
        foreach ($tPath in $ToolsPaths) {
            $resolvedTools = Resolve-Path $tPath -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Path -First 1
            if ($resolvedTools -and (Test-Path "$resolvedTools\mingw32-make.exe")) {
                Write-Host "Found MinGW compiler at: $resolvedTools" -ForegroundColor Green
                $env:PATH = "$resolvedTools;$env:PATH"
                break
            }
        }
    }
}

# Check for compiler (MSBuild or MinGW make)
$Compiler = "mingw32-make"
if (Get-Command "msbuild.exe" -ErrorAction SilentlyContinue) {
    if ($QtBinPath -like "*msvc*") {
        $Compiler = "nmake"
    }
} 

if ($Compiler -eq "mingw32-make" -and -not (Get-Command "mingw32-make.exe" -ErrorAction SilentlyContinue)) {
    Write-Error "Neither MSBuild/nmake nor mingw32-make were found. Please ensure your compiler is installed and in PATH."
    exit 1
}

Write-Host "Generating Makefile with qmake..." -ForegroundColor Cyan
# Use existing jim.pro file
if (-not (Test-Path "jim.pro")) {
    Write-Error "jim.pro file not found!"
    exit 1
}

# Run qmake
& qmake -config release jim.pro

Write-Host "Compiling jim..." -ForegroundColor Cyan
if ($Compiler -eq "nmake") {
    & nmake release
} else {
    & mingw32-make release
}

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host "Building Windows audio loopback bridge..." -ForegroundColor Cyan
$bridgeSource = "tools\windows_audio_bridge.cpp"
$bridgePath = ".build\bin\jim-audio-bridge.exe"
if ($Compiler -eq "nmake") {
    & cl.exe /nologo /EHsc /O2 /std:c++17 $bridgeSource /Fe:$bridgePath ole32.lib uuid.lib
    Remove-Item "windows_audio_bridge.obj" -ErrorAction SilentlyContinue
} else {
    & g++.exe -std=c++17 -O2 $bridgeSource -o $bridgePath -lole32 -luuid
}
if ($LASTEXITCODE -ne 0) {
    Write-Error "Windows audio bridge build failed!"
    exit 1
}

Write-Host "Build successful! Executable should be in the .build/bin/ directory." -ForegroundColor Green

# Ask to deploy (commented out for automated testing, uncomment if desired)
# $ans = Read-Host "Do you want to run windeployqt to make this portable? (y/n)"
# if ($ans -eq 'y') {
Write-Host "Deploying Qt dependencies..." -ForegroundColor Cyan
$exePath = ".build\bin\jim.exe"

if (Test-Path $exePath) {
    & windeployqt --no-translations --no-opengl-sw $exePath
    Write-Host "Deployment complete! You can now run $exePath" -ForegroundColor Green
} else {
    Write-Host "Could not find $exePath to deploy." -ForegroundColor Yellow
}
# }
