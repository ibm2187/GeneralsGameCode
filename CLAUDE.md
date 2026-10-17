# GeneralsGameCode: CoH fork

This is a personal fork of the C&C Generals Zero Hour engine. It adds Company of Heroes features under `CoH/`, kept apart from the original code. Nothing goes upstream.

**Before anything else, read [CoH/CONTEXT.md](CoH/CONTEXT.md).** It is the map: what to read for which job, where each engine system lives, and how to keep a session's context small.

Rules that always apply:
- **The original code stays original.** Original files only get `// CoH hook` calls into `CoH/Kernel/Hooks.h`, plus marked `// CoH fix` bug fixes. Every one is listed in the hooks table in `CoH/README.md`.
- **One feature is one slice in `CoH/Features/<Name>/`** with Domain/, Engine/, UI/, Data/INI/, Tests/, a README and a public `<Name>.h`. Slices see each other only through `<Name>.h` or hub events.
- **Every feature has a flag.** With every flag off, the game plays exactly like `main`.
- **Game logic is deterministic.** Use `GameLogicRandomValue` only, and logic frames for time.
- **Build with `--config Release`**, or `generalszh.exe` silently stays stale. `CoH/scripts/install.ps1` does this and checks the exe's timestamp.
- **Never close the user's running game.** Ask first.
- **The user watches test games.** Run them windowed at normal speed. Headless is fine for replay checks.
- **Work on a branch from `coh/main`.** Ask before merging into `coh/main` or pushing.
