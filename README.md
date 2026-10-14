# Techniqolor

**Version 0.1 · Beat Saber Quest 1.40.8_7379 · Scotland2**

Techniqolor is a native C++/IL2CPP Quest port of **Technicolor 1.1.2 by Aeroluna**. It adds rainbow colors to lights, notes, walls, bombs and sabers, with separate styles for the left hand. Made with help of ChatGPT 6.1

## Installation

1. Download `Techniqolor-0.1.qmod` from [Releases](https://github.com/NotGodlikeUwU/Techniqolor/releases).
2. Install it using your QMOD installer for modded **Beat Saber 1.40.8_7379**.
4. Open **Mod Settings → Techniqolor**. Settings take effect when you start the next level.

Defaults: lights and walls use **Gradient**, bombs use **True Random**, notes and sabers are **Off**, lighting grouping is **Mayhem**, and frequency is **0.1**.

## Features

| Feature | Quest implementation |
| --- | --- |
| Styles | Off, Warm/Cold, True Random and Gradient |
| Lights | Legacy basic events and modern v3/v4 light groups through LightWithIdManager |
| Grouping | Standard: shared color; Isolated Event: color per group; Mayhem: color per registered light |
| Frequency | Color-change probability in flat Standard/Isolated modes and Gradient speed; flat Mayhem changes each event, matching the PC behavior |
| Notes | Standard ColorNoteVisuals, including chain-note visual components; independent ColorA style |
| Arcs | Gradient arcs follow the corresponding hand's note style; flat arc styles keep their original colors |
| Walls | Standard StretchableObstacle colors, preserving dimensions and UV offset |
| Bombs | `_Color` and `_SimpleColor` through material property blocks |
| Sabers | Glow, fake glow, light and trail; palette and smooth random modes; independent ColorA style |
| Colors after cuts | When saber style is Off, a correctly cut recolored note changes the corresponding saber's color |
| Particles | Event colors, continuous Gradient and separate Mayhem colors |
| Background | Standard background gradient colors and an option to disable the background component |
| Color Boost | Light alpha multiplier `1 + boost`, from -1 to +2 |
| Map compatibility | Rainbow effects are suspended if the selected difficulty declares Chroma or Noodle Extensions in its requirements or suggestions |

## Building

Install **QPM**, **CMake 3.22+**, **Ninja**, **PowerShell 7** and **Android NDK r27d**. Add QPM, CMake and Ninja to PATH.

```powershell
git clone https://github.com/NotGodlikeUwU/Techniqolor.git
cd Techniqolor
pwsh scripts/setup.ps1 -NdkPath C:/Android/android-ndk-r27d
pwsh scripts/build.ps1
pwsh scripts/test.ps1
pwsh scripts/package.ps1 -SkipBuild
```

Replace the example NDK path with your installation. `setup.ps1` restores dependencies into the project's own `extern` directory. Use `-CachePath <path> -Offline` to restore from an existing QPM cache without downloading packages.

Host tests require a C++20 compiler; the recorded local tests used Visual Studio 2026. `build/debug/libtechniqolor.so` contains debugging symbols, while `build/libtechniqolor.so` is the library packaged in the QMOD.

Run `pwsh scripts/archive-source.ps1` to create `Techniqolor-0.1-source.zip`.

QPM requires the internal semantic package version `0.1.0`; the mod's runtime, QMOD and release version are `0.1`.

Key files:

- `src/main.cpp`: hooks and visual effects.
- `src/settings.cpp`: BSML menus.
- `include/color_engine.hpp`: independent color logic.
- `include/modconfig.hpp`: settings.

## Credits and licenses

Original Technicolor: **Aeroluna**, MIT license retained in [LICENSE](LICENSE).

Build scaffold: quest-mod-template, Unlicense. See [third-party notices](THIRD_PARTY_NOTICES.md).

Configuration and UI use the installed dependency headers and the BSMG [configuration guide](https://bsmg.wiki/modding/quest/config.html) and [Quest modding guide](https://bsmg.wiki/modding/quest/intro.html).

Public Quest Chroma headers and implementations were consulted as references for game visual components. Techniqolor does not include Chroma binaries or require Chroma to be installed.
