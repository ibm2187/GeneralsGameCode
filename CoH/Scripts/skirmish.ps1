<#
.SYNOPSIS
Starts a quick skirmish against one AI with the given flags, straight from startup, for play-testing.

.DESCRIPTION
Runs the installed generalszh.exe (CoH\Scripts\install.ps1) with the Kernel's -cohSkirmish option.
The flags replace Features.ini for this run; with no -Flags every feature is off.
The game records the usual last replay, Replays\00000000.rep.

.EXAMPLE
CoH\Scripts\skirmish.ps1 -Flags Sectors
CoH\Scripts\skirmish.ps1 -Flags Sectors,Accuracy -Map 'Tournament Island' -AI hard -Side FactionGLA -Windowed
CoH\Scripts\skirmish.ps1 -Flags Sectors,Upkeep -Cash 10000
#>
param(
    [string[]]$Flags = @(),
    [string]$Map = 'Tournament Desert',
    [ValidateSet('easy', 'medium', 'hard')][string]$AI = 'medium',
    [string]$Side,
    [switch]$Windowed,
    [int]$Cash,
    [string]$GameDir
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
if (-not $GameDir) { $GameDir = $CoHDefaultGameDir }

$exe = Join-Path $GameDir 'generalszh.exe'
if (-not (Test-Path $exe)) { throw "No generalszh.exe in $GameDir; run CoH\Scripts\install.ps1 first." }

$flagList = if ($Flags.Count) { $Flags -join ',' } else { 'none' }
$arguments = @('-cohSkirmish', "`"$Map`"", '-coh', $flagList, '-cohAI', $AI)
if ($Side) { $arguments += '-cohSide', $Side }
if ($Cash -gt 0) { $arguments += '-cohCash', $Cash }
if ($Windowed) { $arguments += '-win', '-xres', '1600', '-yres', '900' }

Write-Host "generalszh.exe $($arguments -join ' ')"
Start-Process -FilePath $exe -ArgumentList ($arguments -join ' ') -WorkingDirectory $GameDir
