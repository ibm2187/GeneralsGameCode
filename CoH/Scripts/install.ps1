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

if (-not (Wait-CoHGameClosed $GameDir)) { Write-Host 'Stopped; nothing was installed.'; exit 1 }

if (-not $NoBuild) { Invoke-CoHBuild }
Assert-CoHExeIsCurrent

if (-not (Wait-CoHGameClosed $GameDir)) { Write-Host 'Stopped; nothing was installed.'; exit 1 }

Copy-Item $CoHExe, $CoHPdb $GameDir -Force
Write-Host "Installed generalszh.exe and generalszh.pdb in $GameDir"
Copy-CoHPacks $GameDir
