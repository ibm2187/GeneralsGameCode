<#
.SYNOPSIS
Builds the Zero Hour profile exe and installs it, with the CoH INI packs, into the game folder.

.DESCRIPTION
1. If the game is running, asks you to close it first. It never closes the game itself.
2. Builds the win32-profile preset with --config Release (without it the multi-config build makes
   Debug and the Release exe silently stays stale), then checks that the exe is newer than every source.
3. Copies generalszh.exe and generalszh.pdb into the game folder.
4. Replaces CoH\Packs in the game folder with the Data\INI pack of every feature the build has
   (the coh_add_feature lines in CoH\CMakeLists.txt). A pack only loads in games where its flag is on.

.EXAMPLE
CoH\Scripts\install.ps1
CoH\Scripts\install.ps1 -NoBuild
#>
param(
    [string]$GameDir,
    [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
if (-not $GameDir) { $GameDir = $CoHDefaultGameDir }

$buildDir = Join-Path $CoHRepo 'build\win32-profile'
$exe = Join-Path $buildDir 'GeneralsMD\Release\generalszh.exe'
$pdb = Join-Path $buildDir 'GeneralsMD\Release\generalszh.pdb'

function Use-VsDevEnvironment {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) { return }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $vsDir = if (Test-Path $vswhere) { & $vswhere -latest -property installationPath } else { $null }
    if (-not $vsDir) { $vsDir = 'C:\Program Files\Microsoft Visual Studio\18\Community' }
    & (Join-Path $vsDir 'Common7\Tools\Launch-VsDevShell.ps1') -Arch x86 -HostArch amd64 -SkipAutomaticLocation | Out-Null
}

function Invoke-Build {
    Use-VsDevEnvironment
    Push-Location $CoHRepo
    try {
        if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) {
            cmake --preset win32-profile
            if ($LASTEXITCODE) { throw "cmake configure failed ($LASTEXITCODE)" }
        }
        cmake --build $buildDir --config Release --target z_generals
        if ($LASTEXITCODE) { throw "build failed ($LASTEXITCODE)" }
    }
    finally { Pop-Location }
}

# A build that didn't really happen leaves an exe older than the code it should contain.
# Files under a Tests folder are never built into the exe, so they don't count.
function Assert-ExeIsCurrent {
    if (-not (Test-Path $exe)) { throw "No exe at $exe" }
    $sourceDirs = 'CoH', 'Core', 'GeneralsMD', 'Dependencies' | ForEach-Object { Join-Path $CoHRepo $_ }
    $newest = Get-ChildItem $sourceDirs -Recurse -File -Include *.cpp, *.h, *.inl, *.c, *.in |
        Where-Object { $_.FullName.Substring("$CoHRepo".Length) -notmatch '\\Tests\\' } |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    $built = (Get-Item $exe).LastWriteTime
    if ($newest -and $newest.LastWriteTime -gt $built) {
        throw "The exe ($built) is older than $($newest.FullName) ($($newest.LastWriteTime)): the build is stale."
    }
    Write-Host "generalszh.exe built $built"
}

function Get-FeatureNames {
    Get-Content (Join-Path $CoHRepo 'CoH\CMakeLists.txt') |
        Select-String -Pattern '^\s*coh_add_feature\((\w+)\)' |
        ForEach-Object { $_.Matches[0].Groups[1].Value }
}

function Install-Packs {
    $packsDir = Join-Path $GameDir 'CoH\Packs'
    if (Test-Path $packsDir) { Remove-Item $packsDir -Recurse -Force }
    New-Item -ItemType Directory -Force $packsDir | Out-Null

    foreach ($name in Get-FeatureNames) {
        $source = Join-Path $CoHRepo "CoH\Features\$name\Data\INI"
        if (-not (Test-Path $source)) { continue }
        Copy-Item $source (Join-Path $packsDir $name) -Recurse
        Write-Host "pack $name"
    }
}

if (-not (Wait-CoHGameClosed $GameDir)) { Write-Host 'Stopped; nothing was installed.'; exit 1 }

if (-not $NoBuild) { Invoke-Build }
Assert-ExeIsCurrent

if (-not (Wait-CoHGameClosed $GameDir)) { Write-Host 'Stopped; nothing was installed.'; exit 1 }

Copy-Item $exe, $pdb $GameDir -Force
Write-Host "Installed generalszh.exe and generalszh.pdb in $GameDir"
Install-Packs
