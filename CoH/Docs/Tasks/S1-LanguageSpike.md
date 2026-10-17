# S1 Spike: a modern language for CoH code

**Branch** `coh/spike-languages` (never merged; only the findings are) · **Starts** once 04 Accuracy or 09 Squads is merged, whichever is first · **Time box** one evening

A secondary, future feature: a spike to try, not a commitment.

## Question

Would writing CoH code in a modern language, transpiled or compiled into the engine, make the coding easier without costing determinism, debugging or build simplicity?
How does each candidate compare with plain modern C++20, which the build already compiles?

## Test subject

Port one real, pure piece: **Sectors' capture rule**, `CoH/Features/Sectors/Domain/Capture.h/.cpp`.
If Accuracy has merged, you may use its hit-chance rule instead, which tests floats and synced randomness harder.
- It has unit tests (`Tests/CaptureTest.cpp`) to check each port against.
- It runs every logic frame, so its speed counts.
- It is synced logic, so a wrong float or a different iteration order shows up as a replay mismatch.

For each candidate:
1. Write the capture rule in that language.
2. Swap it in behind the same C++ interface; only the Domain implementation changes, and the Engine code still calls `Capture.h`.
3. Run the existing `CaptureTest` cases through it.
4. Play back a recorded Sectors replay (`CoH\Scripts\replaycheck.ps1 -Replays 00000000.rep`, or a new one recorded with the flag on).
   It must play back with no mismatch; this is the determinism test.

## Candidates, in this order

1. **Baseline: idiomatic C++20.** The same rule rewritten using `std::span`, ranges, `std::optional` and designated initializers. This is the bar the others must beat.
2. **cppfront (cpp2).** Herb Sutter's new syntax, transpiled to C++. This is the most literal form of "transpile a modern language to C++".
3. **Nim with the C++ backend** (`nim cpp`), compiled by the same MSVC.
4. **Rust through the `cxx` bridge**, built as a static library for `i686-pc-windows-msvc` and linked into the 32-bit exe. It is compiled, not transpiled; included as the strongest modern-language option.
5. **Lua 5.4 embedded**, if time allows. This is the scripting alternative: the rule as a script driven from C++. Note how floats and table order affect determinism.

Skip a candidate after 45 minutes if its toolchain won't build into the 32-bit MSVC CMake build, and record why.

## Measure, for each

- **Build:** steps added to CMake and `install.ps1`, extra tools to install, clean-build time added.
- **Code:** lines and readability against the C++ version; how much binding code it needs.
- **Correctness:** the `CaptureTest` cases pass.
- **Determinism:** the Sectors replay plays back clean.
- **Speed:** time for a million capture steps, against the C++ baseline.
- **Debugging:** can you set a breakpoint and read a call stack in Visual Studio? Does Tracy see it?
- **Writing it:** how many tries it took to get it compiling and correct. The code is mostly AI-written, so this matters.
- **Engine access:** how hard it would be for this language to touch engine types (`Object`, `Player`, INI) and not only pure Domain code.

## Deliverable

- `CoH/Docs/Spikes/S1-Languages.md`: a table of the measures for each candidate, what broke, and one recommendation:
  - stay with C++20
  - adopt a candidate for `Domain/` code only
  - adopt Lua for a scripting layer
- The code stays on `coh/spike-languages`. Only the findings doc is merged into `coh/main`, and only with the user's OK.

## Rules

- Nothing from the spike changes `coh/main` code or any other slice. The Sectors flag must keep its original C++ capture rule on `coh/main`.
- Read `CLAUDE.md` and `CoH/Docs/CONTEXT.md` first, and follow the budget rules: toolchain output goes to logs.
- Never close the user's running game.
