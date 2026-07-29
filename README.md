# Quake for PlayStation 2

An effort to restore, modernize, and stabilize the historical Quake port for
the Sony PlayStation 2.

Current version: **0.2.0 — Reproducible Build**

The repository currently preserves the original 2004 port and its expanded
2009 source snapshot. The code now builds with a pinned current PS2DEV
toolchain, but it has not yet completed emulator or real-hardware validation
and should not be considered a production-ready release.

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

Python 3.9 or newer and the platform `tar` utility are required. The bootstrap
script downloads the pinned PS2DEV snapshot, verifies every archive by
SHA-256, and installs it under the ignored `.cache` directory:

```sh
python tools/bootstrap_ps2dev.py
python tools/build.py --video ntsc --clean
```

Supported video selections are `ntsc`, `pal`, and `vesa`. Each build produces
an unpacked development ELF and a packed release ELF under `build/<mode>/`.
The archived SDL sound backend is disabled by default and can be compiled with
`--sound`; sound is not considered stable yet.

On systems with GNU Make, `make toolchain` and `make VIDEO=ntsc` provide short
aliases for the same Python commands.

## Game data

Quake game data is not part of this repository. Provide your own legally
obtained `id1` directory when testing the port. Do not commit PAK files,
music, save games, or configuration files.

## Credits

The original PlayStation 2 port was created in 2004 by **Nicolas Plourde**,
also known as **nic067**. This modernization project is built on his pioneering
work bringing Quake to the PS2. His original release notes are preserved in
[readme.txt](readme.txt).

Quake was originally created by id Software. Additional libraries and PS2
modules remain the work of their respective authors and contributors.

## License

The Quake source code is distributed under the GNU General Public License
version 2; see [gnu.txt](gnu.txt). Game assets are not covered by that license.
Third-party PS2 modules and libraries retain their respective licenses.
