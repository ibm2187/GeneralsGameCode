# K1 Modern C++: a C++20 wrapper in the Kernel

**Branch** `coh/k1-modern-cpp` · **Needs** 00 · **Package** `CoH/Kernel/` · **Flag** none (nothing calls it yet, so the game is unchanged)

## Goal

New CoH code is written in modern C++20 against a small Kernel wrapper, instead of repeating the engine's C idioms in every slice.
The next feature task is the first one written this way. The user chose this over the S1 language spike: plain C++20, no new toolchain.

**No existing code is converted.** Sectors, GeneralsArsenal, Upkeep, Squads, Accuracy and FireRate stay as they are.

## Why

A survey of the merged slices found the same engine idioms over and over (call sites):

| Idiom | Calls | Wrapped by |
|---|---|---|
| Logic frames and seconds | ~51 | `Kernel/Time.h` (`now`, `everySeconds`, `framesFromSeconds`) |
| INI fields with `offsetof` and a parser per line | ~86 | `Kernel/Ini.h` (`Table`, `field`, `seconds`, `percent`, `custom`) |
| `for (i < getPlayerCount())` with `getNthPlayer` and null checks | ~38 | `Kernel/Players.h` (`players`, `sides`, `activeSides`, `aiSides`) |
| `findObjectByID` then `isEffectivelyDead` | 11 | `Kernel/Objects.h` (`findObject`, `findLiving`) |
| `iterateObjectsInRange` with filter arrays and a manual `deleteInstance` | 12 | `Kernel/Objects.h` (`objectsNear`, `objectsOf`, `allObjects`) |
| `getRelationship(...) == ENEMIES` | 6 | `Kernel/Players.h` (`isEnemy`, `isAlly`) |

Plus synced random rolls, `Kernel/Random.h` (`chance`, `pick`, `between`), over `GameLogicRandomValue`.

## What

- One public header per area in `CoH/Kernel/`: `Time.h`, `Random.h`, `Objects.h`, `Players.h`, `Ini.h`.
  Pure parts in `Kernel/Domain/` (`Frames.h`, `Chance.h`), engine parts in `Kernel/Engine/`.
- Tests in `Kernel/Tests/WrapperTest.cpp` (frames, chance, seeded picks, INI tables).
  `WrapperExampleTest.cpp` compiles the README's example feature, so it can't rot.
- The convention in `PLAN.md` (*Conventions*, 10) and one line in `CLAUDE.md`.
- `CoH/README.md`, *Writing a feature*: the example feature in C++20. `CONTEXT.md` §3 lists the headers.

## Rules for the wrapper

- Small: a helper earns its place by replacing a repeated idiom. Anything else waits for a feature that needs it.
- Deterministic: every order it returns is the engine's, the same on every machine. `chance` always draws one number, so the number of draws never depends on tuning.
- No hook changes: the wrapper only calls the engine. `Kernel/Hooks.h` is untouched.

## Done

- Unit tests pass (`z_googletest`).
- `CoH\Scripts\install.ps1` builds.
- `CoH\Scripts\replaycheck.ps1` passes, since nothing calls the wrapper yet.
- Docs updated.
