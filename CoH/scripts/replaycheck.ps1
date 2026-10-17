<#
.SYNOPSIS
Plays replays headless with the installed exe and reports any mismatch.

.DESCRIPTION
The replays are looked for under the user data folder's Replays folder. A replay plays with the flags
it was recorded with, so a replay recorded with every flag off checks that "off means original".

Use replays recorded with a main build of this preset, such as Replays\CoH\kernel-flags-off.rep.
Retail replays (GeneralsReplays) need a VC6 build (TESTING.md): the win32-profile build of main
mismatches all of them at frame 110 too.

.EXAMPLE
CoH\scripts\replaycheck.ps1
CoH\scripts\replaycheck.ps1 -Replays '00000000.rep'
#>
param(
    [string]$Replays = 'CoH/*.rep',
    [string]$GameDir
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Common.ps1')
if (-not $GameDir) { $GameDir = $CoHDefaultGameDir }

$exe = Join-Path $GameDir 'generalszh.exe'
$log = Join-Path ([IO.Path]::GetTempPath()) 'coh-replaycheck.log'

$process = Start-Process -FilePath $exe -ArgumentList "-headless -replay `"$Replays`"" -WorkingDirectory $GameDir `
    -RedirectStandardOutput $log -PassThru
if (-not $process.WaitForExit(20 * 60 * 1000)) {
    Write-Host 'The replay check is still running after 20 minutes; it is left running.'
    exit 1
}

Get-Content $log
exit $process.ExitCode
