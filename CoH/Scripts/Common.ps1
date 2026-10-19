# CoH scripts: what install.ps1, package.ps1, skirmish.ps1 and replaycheck.ps1 share. Dot-source it.

$CoHRepo = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$CoHDefaultGameDir = 'C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour'
$CoHUserDataDir = Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Command and Conquer Generals Zero Hour Data'

$CoHBuildDir = Join-Path $CoHRepo 'build\win32-profile'
$CoHExe = Join-Path $CoHBuildDir 'GeneralsMD\Release\generalszh.exe'
$CoHPdb = Join-Path $CoHBuildDir 'GeneralsMD\Release\generalszh.pdb'

function Use-VsDevEnvironment {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) { return }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $vsDir = if (Test-Path $vswhere) { & $vswhere -latest -property installationPath } else { $null }
    if (-not $vsDir) { $vsDir = 'C:\Program Files\Microsoft Visual Studio\18\Community' }
    & (Join-Path $vsDir 'Common7\Tools\Launch-VsDevShell.ps1') -Arch x86 -HostArch amd64 -SkipAutomaticLocation | Out-Null
}

# Builds the win32-profile preset with --config Release: without it the multi-config build makes
# Debug and the Release exe silently stays stale.
function Invoke-CoHBuild {
    Use-VsDevEnvironment
    Push-Location $CoHRepo
    try {
        if (-not (Test-Path (Join-Path $CoHBuildDir 'CMakeCache.txt'))) {
            cmake --preset win32-profile
            if ($LASTEXITCODE) { throw "cmake configure failed ($LASTEXITCODE)" }
        }
        cmake --build $CoHBuildDir --config Release --target z_generals
        if ($LASTEXITCODE) { throw "build failed ($LASTEXITCODE)" }
    }
    finally { Pop-Location }
}

# A build that didn't really happen leaves an exe older than the code it should contain.
# Files under a Tests folder are never built into the exe, so they don't count.
function Assert-CoHExeIsCurrent {
    if (-not (Test-Path $CoHExe)) { throw "No exe at $CoHExe" }
    $sourceDirs = 'CoH', 'Core', 'GeneralsMD', 'Dependencies' | ForEach-Object { Join-Path $CoHRepo $_ }
    $newest = Get-ChildItem $sourceDirs -Recurse -File -Include *.cpp, *.h, *.inl, *.c, *.in |
        Where-Object { $_.FullName.Substring("$CoHRepo".Length) -notmatch '\\Tests\\' } |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    $built = (Get-Item $CoHExe).LastWriteTime
    if ($newest -and $newest.LastWriteTime -gt $built) {
        throw "The exe ($built) is older than $($newest.FullName) ($($newest.LastWriteTime)): the build is stale."
    }
    Write-Host "generalszh.exe built $built"
}

# The features the build has, in the order their packs load: the coh_add_feature lines in CoH\CMakeLists.txt.
function Get-CoHFeatureNames {
    Get-Content (Join-Path $CoHRepo 'CoH\CMakeLists.txt') |
        Select-String -Pattern '^\s*coh_add_feature\((\w+)\)' |
        ForEach-Object { $_.Matches[0].Groups[1].Value }
}

# Fills <GameDir>\CoH\Packs with the Data\INI pack of every feature the build has, replacing what was there.
function Copy-CoHPacks([string]$GameDir) {
    $packsDir = Join-Path $GameDir 'CoH\Packs'
    if (Test-Path $packsDir) { Remove-Item $packsDir -Recurse -Force }
    New-Item -ItemType Directory -Force $packsDir | Out-Null

    foreach ($name in Get-CoHFeatureNames) {
        $source = Join-Path $CoHRepo "CoH\Features\$name\Data\INI"
        if (-not (Test-Path $source)) { continue }
        Copy-Item $source (Join-Path $packsDir $name) -Recurse
        Write-Host "pack $name"
    }
}

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
