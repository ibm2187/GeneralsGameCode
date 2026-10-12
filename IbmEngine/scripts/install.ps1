# IBM Engine: installs a build and every feature pack into the Zero Hour game folder.
#
#   .\IbmEngine\scripts\install.ps1                      # profile build -> generalszh.exe
#   .\IbmEngine\scripts\install.ps1 -Preset win32-debug  # debug build   -> generalszh_dbg.exe
#   .\IbmEngine\scripts\install.ps1 -Enable Squads       # also switch features on in Features.ini
#
# Feature packs are copied from IbmEngine\Features\<Name>\Data to <game>\Data.

param(
    [string]$Preset = "win32-profile",
    [string]$GameDir = "C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour",
    [string[]]$Enable = @()
)

$ErrorActionPreference = "Stop"
$repo = Resolve-Path (Join-Path $PSScriptRoot "..\..")

$config = if ($Preset -like "*debug*") { "Debug" } else { "Release" }
$exeName = if ($config -eq "Debug") { "generalszh_dbg" } else { "generalszh" }
$buildDir = Join-Path $repo "build\$Preset\GeneralsMD\$config"

if (-not (Test-Path "$buildDir\generalszh.exe")) {
    throw "No build found at $buildDir. Build the $Preset preset first."
}
if (Get-Process generalszh, generalszh_dbg -ErrorAction SilentlyContinue) {
    throw "The game is running. Close it first, the exe is locked while it runs."
}

Copy-Item "$buildDir\generalszh.exe" "$GameDir\$exeName.exe"
Copy-Item "$buildDir\generalszh.pdb" "$GameDir\$exeName.pdb"
Write-Host "Installed $exeName.exe from $Preset"

# Replace each pack folder whole, so files removed from a pack disappear from the game too.
Get-ChildItem (Join-Path $repo "IbmEngine\Features") -Directory | ForEach-Object {
    $data = Join-Path $_.FullName "Data"
    if (-not (Test-Path $data)) { return }
    Get-ChildItem (Join-Path $data "INI\Features") -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        $target = Join-Path $GameDir "Data\INI\Features\$($_.Name)"
        if (Test-Path $target) { Remove-Item $target -Recurse -Force }
    }
    Copy-Item "$data\*" (Join-Path $GameDir "Data") -Recurse -Force
    Write-Host "Installed feature data of $($_.Name)"
}

if ($Enable.Count -gt 0) {
    $userData = Join-Path ([Environment]::GetFolderPath("MyDocuments")) "Command and Conquer Generals Zero Hour Data"
    $featuresIni = Join-Path $userData "Features.ini"
    $lines = if (Test-Path $featuresIni) { @(Get-Content $featuresIni) } else { @() }
    foreach ($name in $Enable) {
        $lines = @($lines | Where-Object { $_ -notmatch "^\s*$name\s*=" })
        $lines += "$name = yes"
    }
    Set-Content -Path $featuresIni -Value $lines
    Write-Host "Enabled in Features.ini: $($Enable -join ', ')"
}
