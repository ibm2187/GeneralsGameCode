# IBM Engine: starts the game straight into the Demo skirmish (no menus) on the blank IbmDemo map,
# or on the map given.
#
#   .\IbmEngine\scripts\demo.ps1
#   .\IbmEngine\scripts\demo.ps1 -Map "Maps\Tournament Desert\Tournament Desert.map"

param(
    [string]$Map = "",
    [string]$GameDir = "C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour"
)

$gameArgs = @("-quickstart", "-demo")
if ($Map) { $gameArgs += "`"$Map`"" }
Start-Process -FilePath (Join-Path $GameDir "generalszh.exe") -ArgumentList $gameArgs -WorkingDirectory $GameDir
