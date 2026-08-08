# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [0.4.0] - 2026-08-08

### Added

- Added GS double buffering so complete frames are uploaded off-screen before
  being presented.
- Added VSync presentation through the current PS2SDK graph API.

### Changed

- Horizontally scale the original 320-pixel menu graphics, text, cursors, and
  translated player preview to the 640-pixel PS2 framebuffer.
- Link the PS2SDK graph library required by synchronized presentation.

### Fixed

- Fixed the main menu and submenus appearing compressed to half width.
- Prevented partially uploaded frames from being displayed, eliminating
  tearing on the validated NTSC path.

### Validation

- Confirmed gameplay and corrected interface rendering in PCSX2.
- Confirmed gameplay and corrected interface rendering on a real PlayStation 2.
- PAL and 480p output remain unvalidated.

## [0.3.0] - 2026-07-29

### Added

- Added game-data discovery beside the ELF through PCSX2 `host:` and USB
  `mass:`, with a clear error when `id1/pak0.pak` cannot be found.
- Embedded the current PS2SDK BDM, FAT, fileXio, keyboard, and mouse modules in
  the ELF so no external `irx` directory is required.
- Added on-screen fatal-error reporting during early startup.

### Changed

- Replaced the obsolete usbhdfsd/fio path with the BDM/fileXio stack used by
  the Quake II PS2 port.
- Resolve the base game, expansions, and `-game` directories relative to the
  detected ELF location instead of hardcoding the root of `mass:`.
- Route the Quake system file layer through PS2SDK's standard POSIX interface,
  allowing the same code to access PCSX2 `host:` and USB `mass:`.
- Corrected negative file-descriptor handling and file creation semantics in
  the PS2 system layer.
- Replaced handwritten cache, GS interrupt-mask, CRTC, and GIF DMA operations
  with supported PS2SDK APIs.
- Disabled unavailable USB keyboard and mouse polling on the PCSX2 `host:`
  path while preserving gamepad input.

### Fixed

- Fixed the GIF transfer size calculation and cache coherency that previously
  produced black or gray screens.
- Fixed a real-hardware hang during GS initialization by initializing only the
  GIF DMA channel instead of resetting the entire DMAC.
- Corrected NTSC display timing so the picture and HUD are no longer shifted
  down and clipped on a CRT.
- Expanded Quake's fixed 320-pixel status bar across the 640-pixel framebuffer
  without reducing the resolution of the 3D scene.
- Replaced the obsolete timer with a stable monotonic runtime clock.

### Validation

- Confirmed boot and gameplay in PCSX2.
- Confirmed USB game-data loading, boot, and gameplay on a real PlayStation 2
  through LaunchELF.

## [0.2.0] - 2026-07-29

### Added

- Credited Nicolas Plourde (nic067), creator of the original 2004 PlayStation 2
  port, in the project documentation.
- Added a cross-platform Python build driver for NTSC, PAL, and VESA targets.
- Added a checksum-pinned PS2DEV bootstrap for Windows and Linux.
- Added automatic embedding of redistributable IRX modules from PS2SDK.
- Added GitHub Actions builds and ELF artifacts for every video target.

### Changed

- Updated the archived source for GCC 15 and the current PS2SDK.
- Corrected the local GS 64-bit register types for the current EE ABI.
- Replaced the removed `padReset()` call with the supported initialization
  sequence.

## [0.1.0] - 2026-07-29

### Added

- Initial version-controlled import of the historical Quake PS2 source.
- Project overview and source layout documentation.
- Development roadmap through the stable 1.0.0 release.
- Semantic version tracking through the `VERSION` file.
- Repository rules excluding Quake game data, user state, prebuilt ELF files,
  binary IOP modules, and generated build products.

[Unreleased]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.4.0...HEAD
[0.4.0]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/alekszverr-jpg/quake1-ps2/releases/tag/v0.1.0
