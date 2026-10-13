# Feature flags

Switches gameplay features on and off without rebuilding, and loads each feature's INI pack.

## Switching a feature on

- **Features.ini** in `Documents\Command and Conquer Generals Zero Hour Data\` has one line per feature, for example `Squads = yes`. The game adds missing features to it as `0` on startup.
- **Command line** overrides it for one run: `-feature Squads=1` or `-feature Squads=0`.

## Which flags a game uses

Flags are *logic* flags: they change the simulation, so every player in a game must use the same ones.

| Game | Flags used |
|---|---|
| Skirmish, LAN host, campaign | This PC's local flags (skirmish ignores flags saved with its last settings) |
| LAN client | The host's flags, sent in the game options |
| Replay | The flags saved in the replay |
| Main menu background map | None |

The flags travel in the game options string as `FF=<hex mask>`. The key is only written when a flag is on, so games with no flags stay identical to retail. Retail clients can't parse `FF`, so they can't join a game that has a flag on.

Logic code checks `IbmEngine::isFeatureActive(IbmEngine::Feature_Squads)`, never the local flags.

Saved games don't store the flags; loading a save uses the current local flags.

## Flags

| Flag | What it switches on | INI pack |
|---|---|---|
| `Squads` | [Squads](../Squads/README.md) | yes |
| `Demo` | [Demo](../Demo/README.md): a test setup at game start for Squads and Economy (was `SquadsDemo`) | no |
| `Accuracy` | [Accuracy](../Accuracy/README.md) | yes |
| `Economy` | [Economy](../Economy/README.md) | yes |
| `Armor` | [Armor](../Armor/README.md) | yes |
| `AirPower` | [AirPower](../AirPower/README.md) | yes |

## INI packs

When a flag that has a pack is active, `Data\INI\Features\<Name>\*.ini` loads at game start in override mode, the same mode as a map's `map.ini`. A pack can add objects, weapons, buttons and button sets, and change existing ones; the game removes them when the game ends. `map.ini` loads after packs, so a map can still override a pack.

- Files load in name order, so packs number their files (`10_Locomotor.ini`, `20_Weapon.ini`, ...). Define things before they're referenced.
- In override mode, modules must be added with `AddModule`, `RemoveModule` or `ReplaceModule`, even on new objects.
- A `<Name>.str` in the pack folder supplies text, through the same mechanism as `map.str`. It loads before the INI files, because object names (`DisplayName`) are looked up while the INI is parsed. A map with its own `map.str` replaces it, which loses tooltips but keeps names.
- After the packs load, `ControlBar::postProcessCommands()` resolves the images of buttons they added; without it, new buttons show no image. `WeaponStore::postProcessLoad()` runs again too: weapons find their `ProjectileObject` by name in that step, and without it a pack weapon with a projectile fires as if it had none.
- Everything else (special powers, sciences, objects, weapons, OCLs) is looked up by name *as the INI is read*, so a pack must define a thing before the lines that name it.
- New weapons a pack adds stay in memory after the game ends (the engine never removes new weapons); in the next game they're redefined as overrides. Objects, buttons, button sets and locomotors are removed.

## ObjectCopy

`ObjectCopy NewName SourceName` copies a whole object, then applies the fields that follow. `ObjectReskin` can only change the model and size; `ObjectCopy` can change anything.

```ini
ObjectCopy IbmUSASquadRanger AmericaInfantryRanger
  KindOf = +IGNORED_IN_GUI       ; + and - adjust the copied flags
  RemoveModule ModuleTag_05
  AddModule
    Behavior = MobMemberSlavedUpdate IbmSquadTag_Member
      ...
    End
  End
End
```

Rules:
- A `WeaponSet` or `ArmorSet` in the copy replaces all of the source's sets.
- **The copy shares module data with the source.** Fields that write into a module's data change the source too. The known case is `Locomotor = ...`, which writes into the AI module: `ReplaceModule` the AI module first, then set locomotors.

## Testing

- `generalszh.exe -headless -replay <file>` must still pass with flags off.
- The debug build (`install.ps1 -Preset win32-debug`) checks INI strictly. `generalszh_dbg.exe -headless -file "Maps\AllBuildingsAllSidesUnitTest_Save\AllBuildingsAllSidesUnitTest_Save.map" -feature Squads=1` loads a pack and logs to `DebugLogFileD.txt` in the game folder; search it for `IbmEngine` and `ASSERTION`.
