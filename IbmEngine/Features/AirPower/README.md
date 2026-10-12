# AirPower

Players build no jets: airfields build helicopters only, and air strikes come from general powers. Feature flag: `AirPower`.

- **USA airfields** (and the Air Force, Superweapon and Laser generals' versions) lose the Raptor, Aurora and Stealth Fighter, and the Bunker Busters upgrade (a Stealth Fighter upgrade). Comanches and their upgrades stay.
- **China airfields** (and the Nuke, Infantry and Tank generals' versions) lose the MiG. Helixes, aircraft armor and mines stay.
- **GLA** has no airfield and gets no helicopters.
- **The skirmish AI**: a team that asks for a jet builds a helicopter instead (Raptor, Aurora and Stealth Fighter → Comanche, MiG → Helix), in the same general's version when there is one.
- The airfield model stays for now; it is meant to be remodelled as a helipad later.

Planned next: new general powers for air strikes (an anti-tank strike on a marked spot), and GLA ballistic-missile powers (Scud strikes, mine-laying rockets).

## Files

| File | What it does |
|---|---|
| `AirPower.cpp`, `AirPower.h` | The AI's jet-to-helicopter swap and the `IbmAirPower` INI block |
| `Data/INI/Features/AirPower/10_CommandSet.ini` | The airfields' button sets without jets (`None` empties a slot) |
| `Data/INI/Features/AirPower/20_AirPower.ini` | `AIReplace = <jet> <helicopter>` lines |

Hooks: `TeamTemplateInfo` (Team.cpp) swaps the AI's jets; `CommandSet::parseCommandButton` (ControlBar.cpp) accepts `None` to empty a slot; INI.cpp registers the `IbmAirPower` block.
