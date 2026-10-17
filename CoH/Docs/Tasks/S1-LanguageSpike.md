# S1 Spike: a modern language for CoH code

**Starts** after both 04 Accuracy and 09 Squads are merged · **Rides on** the next feature task started after them (for example 05 Cover, 06 Suppression, 10 Reinforce, 11 Retreat or 12 Veterancy)

## Question

Would writing CoH code in a modern language, transpiled or compiled into the engine, make the coding easier without costing determinism, debugging or build simplicity?
How does it compare with plain modern C++20, which the build already compiles?

## How

The spike isn't a side experiment. **The next feature task is built in the modern language**, so the language is tried on real work, from INI to tests to play-testing.
That task's own task file still says *what* to build. This file says *how*.

1. **Pick the language.** Wire a small module into the 32-bit MSVC CMake build for each candidate, called from C++ in the engine, and run one unit test through it:
   - **cppfront (cpp2):** Herb Sutter's new syntax, transpiled to C++. This is the most literal form of "transpile a modern language to C++".
   - **Nim with the C++ backend** (`nim cpp`), compiled by the same MSVC.
   - **Rust through the `cxx` bridge**, a static library for `i686-pc-windows-msvc`. It's compiled, not transpiled; included as the strongest modern-language option.
   - **Lua 5.4 embedded**, the scripting alternative.

   Record the measures below for each. Pick one with the user before writing the feature.
2. **Write the feature's `Domain/` code in that language**, with its unit tests.
3. **Engine glue:** go as far as the language reasonably allows into `Engine/`. The `COH_FEATURE` declaration, the hook subscriptions and the original-code hooks stay C++.
   Record exactly where it stopped being practical and why.
4. **Prove it like any task:** the feature's Done list, plus a replay recorded with the flag on that plays back clean. That is the determinism test.

## Measure

- **Build:** steps added to CMake and `install.ps1`, tools to install, clean-build time added.
- **Code:** lines and readability against what the C++20 version would be (write a C++20 version of one Domain function for comparison); how much binding code is needed.
- **Determinism:** the flag-on replay plays back clean; note any float or iteration-order issues.
- **Speed:** the hot Domain function against its C++20 equivalent.
- **Debugging:** breakpoints and call stacks in Visual Studio; whether Tracy sees it.
- **Writing it:** how many tries it took to compile and be correct. The code is mostly AI-written, so this matters.
- **Engine access:** how far into engine types (`Object`, `Player`, INI) it can go.

## Deliverable

- The feature, working, on its own branch.
- `CoH/Docs/Spikes/S1-Languages.md`: the measures for each candidate, what broke, and a recommendation:
  - stay with C++20
  - keep the language for `Domain/` code
  - use Lua as a scripting layer
- **The user decides** what merges:
  - the feature as written, which adopts the language into `coh/main` with its toolchain documented in `CLAUDE.md` and `CONTEXT.md`
  - or the feature rewritten in C++20, with only the findings merged

## Fallback

If no candidate builds into the 32-bit MSVC build, write the feature in C++20 as usual. The spike's result is then "stay with C++20", with the reasons.

## Rules

- Read `CLAUDE.md` and `CoH/Docs/CONTEXT.md` first. Follow the budget rules; toolchain output goes to logs.
- The other slices and the Kernel stay C++.
- With the flag off, the game is unchanged.
- Never close the user's running game.
