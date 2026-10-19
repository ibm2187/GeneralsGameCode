<#
.SYNOPSIS
Builds the Zero Hour profile exe and packs it, with the CoH INI packs, into a drop-in release zip for players.

.DESCRIPTION
1. Builds the win32-profile preset with --config Release (as install.ps1 does) and checks the exe's timestamp.
   -NoBuild packs the last build, after the same check.
2. Writes build\release\CoH-<version>.zip. Its files sit at the top of the zip, so it unzips straight into
   the Zero Hour folder:
     generalszh.exe              the CoH build
     msvcp140.dll and so on      the Visual C++ runtime DLLs the exe imports, so players need no redistributable
     CoH\Packs\<Name>\           the Data\INI pack of every feature in the build
     Features.ini.default        every feature on; players copy it to the user data folder (README-CoH.txt)
     README-CoH.txt              what it is, how to install it, multiplayer rules, turning features off
     LICENSE.txt                 GPL v3, with the source link in the README
     generalszh.pdb              only with -WithSymbols
   It never contains EA data: every file is checked against a list of allowed types.
3. The version is -Version, or else `git describe --tags --always --dirty --match coh-v*` without the
   "coh-v" (a commit hash while there is no coh-v tag).

.EXAMPLE
CoH\Scripts\package.ps1
CoH\Scripts\package.ps1 -NoBuild -Version 0.1 -WithSymbols
#>
param(
    [string]$Version,
    [switch]$NoBuild,
    [switch]$WithSymbols,
    [string]$OutDir
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
if (-not $OutDir) { $OutDir = Join-Path $CoHRepo 'build\release' }

function Get-ReleaseVersion {
    $described = git -C $CoHRepo describe --tags --always --dirty --match 'coh-v*'
    if ($LASTEXITCODE -or -not $described) { throw 'git describe failed; pass -Version.' }
    return $described -replace '^coh-v', ''
}

# The public fork's web address, from the origin remote: https or ssh form, without ".git".
function Get-SourceUrl {
    $url = git -C $CoHRepo remote get-url origin
    if ($LASTEXITCODE -or -not $url) { throw 'The repository has no origin remote to point players to.' }
    $url = $url.Trim() -replace '^git@([^:]+):', 'https://$1/' -replace '\.git$', ''
    return $url
}

# The Visual C++ runtime DLLs the exe imports, from the redistributable folder of the Visual Studio that built it.
# The api-ms-win-crt DLLs are the Universal CRT, part of Windows 10 and later, so they aren't shipped.
function Get-RuntimeDlls {
    $redistDir = $env:VCToolsRedistDir
    if (-not $redistDir) {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
        $vsDir = if (Test-Path $vswhere) { & $vswhere -latest -property installationPath } else { $null }
        if ($vsDir) {
            $redistDir = Get-ChildItem (Join-Path $vsDir 'VC\Redist\MSVC') -Directory -ErrorAction SilentlyContinue |
                Where-Object Name -match '^\d+\.' | Sort-Object { [version]$_.Name } -Descending |
                Select-Object -First 1 -ExpandProperty FullName
        }
    }
    $crtDir = if ($redistDir) { Get-ChildItem (Join-Path $redistDir 'x86') -Directory -Filter 'Microsoft.VC*.CRT' -ErrorAction SilentlyContinue | Select-Object -First 1 }
    if (-not $crtDir) { throw 'No Visual C++ x86 runtime (VC\Redist\MSVC\<version>\x86\Microsoft.VC*.CRT) found.' }

    # An imported DLL's name is in the exe as plain text.
    $imports = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($CoHExe))
    Get-ChildItem $crtDir.FullName -Filter *.dll |
        Where-Object { $imports.IndexOf($_.Name, [StringComparison]::OrdinalIgnoreCase) -ge 0 }
}

# The Features table in CoH\README.md: "| [Name](...) | `Flag` | [task](...) | what it does |".
function Get-FeatureSummaries {
    $summaries = @{}
    foreach ($line in Get-Content (Join-Path $CoHRepo 'CoH\README.md')) {
        if ($line -match '^\| \[(\w+)\]\([^)]*\) \| `(\w+)` \| [^|]* \| (.+) \|$') { $summaries[$Matches[2]] = $Matches[3] }
    }
    return $summaries
}

function Write-FeaturesIni([string]$Path, [string[]]$Features) {
    $lines = @(
        '; CoH features for the games you host (skirmish, campaign, network games you host).'
        '; Copy this file to Documents\Command and Conquer Generals Zero Hour Data\Features.ini'
        '; Set a feature to no to turn it off. Delete Features.ini to play plain Zero Hour.'
        ''
    ) + ($Features | ForEach-Object { "$_ = yes" })
    Set-Content -Path $Path -Value $lines -Encoding ascii
}

function Write-Readme([string]$Path, [string]$ReleaseVersion, [string[]]$Features, [string]$SourceUrl, [string]$Commit) {
    $summaries = Get-FeatureSummaries
    # One feature per entry, wrapped at about 100 columns for Notepad.
    $featureLines = foreach ($name in $Features) {
        $line = '  ' + $name
        $summary = $summaries[$name] -replace '`', ''
        if ($summary) { $line += ' -' }
        foreach ($word in $summary -split ' ' | Where-Object { $_ }) {
            if ($line.Length + 1 + $word.Length -gt 100) { $line; $line = '     ' + $word }
            else { $line += ' ' + $word }
        }
        $line
    }

    $text = @"
CoH for Zero Hour, release $ReleaseVersion
==========================================

Company of Heroes ideas for Command & Conquer Generals Zero Hour: a modified game exe and the INI packs
its features load. Each feature has a flag; with every flag off it plays like plain Zero Hour.

Features in this release:
$($featureLines -join "`r`n")


You need
--------
- Zero Hour 1.04, which you must own (Steam, EA app or The First Decade). This release contains no game
  data: no .big files, maps or art. It uses the files of your own install.
- Windows 10 or later.


Install
-------
1. Find your Zero Hour folder, the one with Game.dat and the .big files. On Steam: right-click the game,
   Manage, Browse local files.
2. Copy everything in this zip into that folder, next to the retail files. Nothing of the retail game is
   replaced: the retail game is still Generals.exe / Game.dat.
3. Copy Features.ini.default to
     Documents\Command and Conquer Generals Zero Hour Data\Features.ini
   (the folder the game keeps Options.ini and replays in). Without it every feature is off in the games
   you host; joining someone else's game works without it.
4. Start generalszh.exe from that folder (a shortcut to it works too). Steam's Play button still starts
   the retail game.

Windows may warn about an unknown publisher, because the exe is not signed: More info, Run anyway.
If the game opens very small, set the resolution in Options.


Multiplayer
-----------
- Every player needs this same release. Players with a different CoH release, the retail game or
  GenTool can't join, and you can't join theirs.
- The host's flags apply to everyone: the game plays with the features in the host's Features.ini, and
  the other players' Features.ini doesn't matter for that game.
- Replays and save games keep the flags they were made with.


Turning features off
--------------------
Edit Documents\Command and Conquer Generals Zero Hour Data\Features.ini and set a feature to no:
    Sectors = yes
    Squads = no
Delete the file to turn every feature off. To try flags for one run without editing the file, start
the game with -coh, for example:  generalszh.exe -coh Sectors,Squads   (-coh none turns all off).
A feature that needs another says so in the list at the top.


Uninstall
---------
Delete generalszh.exe, the msvcp140*.dll and vcruntime140*.dll files, the CoH folder, Features.ini.default,
README-CoH.txt and LICENSE.txt from the Zero Hour folder, and Features.ini from the user data folder.


License and source
------------------
This program is free software under the GNU General Public License version 3 (LICENSE.txt). It is
built from the C&C Generals Zero Hour source code Electronic Arts released under the GPL, through
TheSuperHackers' GeneralsGameCode.
Source code of this release: $SourceUrl/tree/$Commit
Zero Hour's game data is not covered by that license and is not part of this release.

The Visual C++ runtime DLLs (msvcp140*.dll, vcruntime140*.dll) are Microsoft's redistributable files,
shipped so that no runtime has to be installed.
"@
    Set-Content -Path $Path -Value ($text -replace "(?<!`r)`n", "`r`n") -Encoding ascii -NoNewline
}

# Only the build's own files may be in the zip, never EA data (.big, maps, art, audio, strings).
function Assert-NoGameData([string]$Dir) {
    $root = [IO.Path]::GetFullPath($Dir).TrimEnd('\') + '\'
    foreach ($file in Get-ChildItem $Dir -Recurse -File) {
        $relative = $file.FullName.Substring($root.Length)
        $allowed = if ($relative -like 'CoH\Packs\*') { $file.Extension -eq '.ini' }
                   else { $file.Extension -in '.exe', '.pdb', '.dll', '.txt', '.default' }
        if (-not $allowed) { throw "$relative is not a file the release may contain." }
    }
}

if (-not $NoBuild) { Invoke-CoHBuild }
Assert-CoHExeIsCurrent

if (-not $Version) { $Version = Get-ReleaseVersion }
if ($Version -notmatch '^[\w.+-]+$') { throw "The version '$Version' can't be part of a file name." }
$commit = git -C $CoHRepo rev-parse HEAD
$sourceUrl = Get-SourceUrl
$features = @(Get-CoHFeatureNames)

if ($Version -like '*-dirty') { Write-Warning 'The working tree has changes: the source link does not show this build.' }
if (-not (git -C $CoHRepo branch -r --contains $commit)) {
    Write-Warning "Commit $commit is on no remote branch: push it, or the source link in the README is dead."
}

$name = "CoH-$Version"
$stageDir = Join-Path $OutDir $name
$zip = Join-Path $OutDir "$name.zip"
if (Test-Path $stageDir) { Remove-Item $stageDir -Recurse -Force }
New-Item -ItemType Directory -Force $stageDir | Out-Null

Copy-Item $CoHExe $stageDir
if ($WithSymbols) { Copy-Item $CoHPdb $stageDir }
foreach ($dll in Get-RuntimeDlls) { Copy-Item $dll.FullName $stageDir }
Copy-CoHPacks $stageDir
Write-FeaturesIni (Join-Path $stageDir 'Features.ini.default') $features
Write-Readme (Join-Path $stageDir 'README-CoH.txt') $Version $features $sourceUrl $commit
Copy-Item (Join-Path $CoHRepo 'LICENSE.md') (Join-Path $stageDir 'LICENSE.txt')

Assert-NoGameData $stageDir

if (Test-Path $zip) { Remove-Item $zip -Force }
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($stageDir, $zip, [IO.Compression.CompressionLevel]::Optimal, $false)

Write-Host "Wrote $zip"
Get-ChildItem $stageDir -Recurse -File | ForEach-Object {
    '  {0,-45} {1,12:N0}' -f $_.FullName.Substring($stageDir.Length + 1), $_.Length
}
