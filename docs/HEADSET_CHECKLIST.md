# Techniqolor 0.1 headset checklist

No items have been verified on a headset yet.

- [ ] The QMOD installs in Beat Saber 1.40.8_7379 with all dependencies satisfied.
- [ ] The game reaches the main menu and logcat contains `Techniqolor 0.1 loaded`.
- [ ] Mod Settings and the Solo tab open; settings persist after restarting the game.
- [ ] On a normal map, lights and walls change smoothly, bombs are recolored, and notes/sabers retain their original colors when Off.
- [ ] Light off events, fades and strobes remain off/fading without permanently lit sources.
- [ ] Legacy basic-event lighting and a modern map with v3/v4 light groups have both been checked.
- [ ] Standard, Isolated Event and Mayhem look different; Mayhem has acceptable performance.
- [ ] Gradient, Warm/Cold, True Random and Off have been checked for each category.
- [ ] Left-hand styles do not affect the right hand; one-saber and left-handed mode work correctly.
- [ ] Chain notes and gradient arcs remain readable and disappear normally.
- [ ] Correctly cutting a recolored note changes its saber's color when the saber style is Off; a wrong-hand cut does not.
- [ ] Saber blade, fake glow, light and trail are recolored; random mode transitions smoothly.
- [ ] Particles change color without increasing their number or breaking off events.
- [ ] Disable Gradient Background affects only the active level's background; lights and other mods' video screens remain intact.
- [ ] Color Boost -1/0/+1 does not turn off-state lights on.
- [ ] Pause/resume, restart, practice seek/speed, failure, results and returning to the menu do not crash.
- [ ] The next level and menu do not inherit effects or object references from the previous level.
- [ ] A selected difficulty declaring Chroma/Noodle suspends the port and logs `Techniqolor suspended`.
- [ ] Disabling the mod restores original colors in the next level.
- [ ] Multiplayer has been checked separately for startup, local player, remote models and session completion.

If the game crashes, capture logcat and the latest tombstone immediately. Use the matching package's debug library for analysis.

```powershell
New-Item -ItemType Directory -Path logs -Force
adb devices -l
adb logcat -d > logs/logcat.txt
```

Include the exact scenario with the logs. A successful build does not establish visual correctness.
