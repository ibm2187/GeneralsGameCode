# CoH scripts: what install.ps1, skirmish.ps1 and replaycheck.ps1 share. Dot-source it.

$CoHRepo = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$CoHDefaultGameDir = 'C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour'
$CoHUserDataDir = Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Command and Conquer Generals Zero Hour Data'

# The game processes started from the game folder: the CoH build, the debug build, the retail game.
# The folder is compared with a trailing separator, so ...\ZH-FireRate does not match ...\ZH-FireRateAccuracy.
function Get-CoHGameProcesses([string]$GameDir) {
    $fullDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($GameDir)
    $prefix = [IO.Path]::GetFullPath($fullDir).TrimEnd('\', '/') + '\'
    Get-Process | Where-Object {
        $path = $null
        try { $path = $_.Path } catch { }
        $path -and $path.StartsWith($prefix, [StringComparison]::OrdinalIgnoreCase)
    }
}

# Asks the user to close the game while it runs. Never closes it. Returns $false when the user stops.
function Wait-CoHGameClosed([string]$GameDir) {
    while ($running = @(Get-CoHGameProcesses $GameDir)) {
        $names = ($running | ForEach-Object { "$($_.ProcessName) ($($_.Id))" }) -join ', '
        Write-Host "Zero Hour is running: $names. It is never closed for you."
        # A non-interactive shell cannot ask; Read-Host throws there, so stop instead.
        try { $answer = Read-Host 'Close the game, then press Enter to go on, or type q to stop' }
        catch { Write-Host 'This shell cannot ask you to close it.'; return $false }
        if ($answer -eq 'q') { return $false }
    }
    return $true
}
