# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Added a mass-only boot path that waits for `mass:/id1/pak0.pak` and reports
  a clear error when USB game data is unavailable.
- Embedded the current PS2SDK BDM, FAT, fileXio, keyboard, and mouse modules in
  the ELF so no external `irx` directory is required.

### Changed

- Replaced the obsolete usbhdfsd/fio path with the BDM/fileXio stack used by
  the Quake II PS2 port.
- Resolve the base game, expansions, and `-game` directories from `mass:`
  instead of allowing relative paths to fall through to `host:`.
- Corrected negative file-descriptor handling and file creation semantics in
  the PS2 system layer.

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

[Unreleased]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/alekszverr-jpg/quake1-ps2/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/alekszverr-jpg/quake1-ps2/releases/tag/v0.1.0
