# Verification

Verified on 2026-10-26 using the installed PlatformIO environment.

## Compilation

| Project | Command from collection root | Result | RAM bytes | Flash bytes |
| --- | --- | --- | --- | --- |
| Homework 1.5 | `pio run -d homework-01-05-button-interrupt-counter` | SUCCESS, exit 0 | 31180 | 292409 |
| Homework 1.6 | `pio run -d homework-01-06-photoresistor-hysteresis` | SUCCESS, exit 0 | 30948 | 297645 |
| Homework 2.1 | `pio run -d homework-02-01-nonblocking-led` | SUCCESS, exit 0 | 30588 | 290313 |

Environment: Espressif32 7.1.3, Arduino ESP32 package 4.20017.260907+sha.dcc1105b, ESP32-S3 toolchain 8.4.0+2021r2-patch5. The configurations retain their original unpinned platform selection; future installs can resolve different versions.

All builds emitted `Warning! An extra UF2 bootloader image is already added!` and still completed successfully. No firmware was uploaded and no hardware tests were performed.

## Preservation and contents

SHA-256 comparisons confirmed that all eight copied source/header/configuration files match their originals. Original authored files were preserved in the prepared collection. After a second hash verification and explicit owner authorization, the old project contents were removed on 2026-10-26. Three empty original directories remained locked by another process: Homework_1.5, ESP32_blink2, and Module_class_2.2. Generated build/editor files and scaffold placeholder READMEs from the old homework folders were discarded.

| Original directory name | Prepared directory |
| --- | --- |
| Homework_1.5 | homework-01-05-button-interrupt-counter |
| Homework_1.6 | homework-01-06-photoresistor-hysteresis |
| Homework_2.1 | homework-02-01-nonblocking-led |

Only owner-confirmed `Homework_*` projects are included. ESP32_blink, ESP32_blink2, Logic_analyzer_1.5, Module_2.1, and Module_class_2.2 are excluded.

`git check-ignore` confirmed generated `.pio` firmware paths are ignored for all three projects. The publication file list contains authored READMEs, this report, `.gitignore`, source/header files, and PlatformIO configurations. Placeholder READMEs and local editor settings were not copied.

A local Git repository was initialized on main. Compilation was completed before publication; publication status is available in Git history and the GitHub repository.


