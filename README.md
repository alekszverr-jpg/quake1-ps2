# Quake for PlayStation 2

An effort to restore, modernize, and stabilize the historical Quake port for
the Sony PlayStation 2.

Current version: **0.1.0 — Archive Baseline**

The repository currently preserves the original 2004 port and its expanded
2009 source snapshot. The code has not yet been migrated to the current
PS2DEV toolchain and should be considered an archival development baseline,
not a production-ready release.

## Goals

- Reproducible builds with the current PS2DEV toolchain.
- Stable NTSC, PAL, and 480p video output.
- Correct frame pacing, VSync, and DMA transfers.
- Full DualShock 2 support, with optional USB keyboard and mouse.
- Reliable sound, storage, and save-game support.
- Validation in PCSX2 and on real PlayStation 2 hardware.

See [ROADMAP.md](ROADMAP.md) for the planned milestones and
[CHANGELOG.md](CHANGELOG.md) for version history.

## Source layout

- `source/` — the most complete source snapshot, dated 2009.
- `source/sys_ps2.c` — PS2 entry point, timing, and system services.
- `source/vid_ps2.c` and `source/ps2_gs.c` — software framebuffer and GS DMA.
- `source/in_ps2.c` and `source/pad.c` — keyboard, mouse, and gamepad input.
- `source/ps2.c` — IOP reset and module loading.
- Root-level PS2 files — the earlier 2004 implementation and patch.
- `readme.txt` — the original author's release notes.

## Versioning

The project follows [Semantic Versioning](https://semver.org/):

- `0.x` versions are development milestones and may contain breaking changes.
- `1.0.0` will mark the first hardware-validated stable release.
- The canonical version is stored in the `VERSION` file.

## Building

The archived Makefiles target an obsolete PS2SDK layout and are not expected
to build with a current toolchain. A reproducible modern build environment is
planned for version 0.2.0.

## Game data

Quake game data is not part of this repository. Provide your own legally
obtained `id1` directory when testing the port. Do not commit PAK files,
music, save games, or configuration files.

## License

The Quake source code is distributed under the GNU General Public License
version 2; see [gnu.txt](gnu.txt). Game assets are not covered by that license.
Third-party PS2 modules and libraries retain their respective licenses.

