# Validation — 2026-10-14

Target: Beat Saber Quest 1.40.8_7379, Scotland2, Techniqolor 0.1.

Passed on this computer:

- QPM offline restore from the existing local cache. The port has its own copied extern directory.
- Android NDK r27d / Clang 18, arm64-v8a, API 24, C++20 RelWithDebInfo build and link.
- Windows/MSVC color contract executable: 25 checks; CTest 1/1 passed.
  Tests cover the original .NET Random(400) sequence, warm/cold interpolation, HSV wrap,
  PC defaults/Desync behavior, independent left overrides, HDR/fade/off intensity and config validation.
- QMOD identity, package version, dependency declarations, MIT license, ELF64 AArch64 architecture,
  and byte-exact SHA-256 agreement of packaged and built libtechniqolor.so.
- ELF exported entry points: setup and late_load. DT_NEEDED contains the declared runtime dependencies
  and Scotland2/system libraries, without PC/.NET libraries.

Native library SHA-256:
`759DAE90AA5115402927BBB6616C9EDA6844F8A1F8679995DC652F6C4A7BCB58`

Evidence: ../build.log, ../tests.log, ../package.log, ../build/host-tests/Testing/Temporary/LastTest.log.
Matching symbols: ../build/debug/libtechniqolor.so.

Not verified:

- Device installation, loader behavior, game startup, menus, in-headset visuals, performance,
  gameplay transitions, pause/restart/practice and interaction with other mods.
- Stock/custom environments, particular v3/v4 maps, custom saber/note/bomb models and multiplayer.

The earlier ADB check on 2026-10-13 returned an empty device list. No headset test or installation is claimed.
See HEADSET_CHECKLIST.md. This is an experimental build, not a completed headset-validated port.
