# Roadmap

This roadmap tracks the path from the recovered archive to a stable,
hardware-validated PlayStation 2 release. Milestone scope may be refined as
hardware testing exposes platform-specific constraints.

## 0.1.0 — Archive Baseline

- [x] Preserve the 2004 and 2009 source snapshots.
- [x] Document the platform architecture and known risks.
- [x] Establish Semantic Versioning and a changelog.
- [x] Exclude game data and prebuilt binaries from version control.

## 0.2.0 — Reproducible Build

- [x] Port the build to the current PS2DEV/PS2SDK toolchain.
- [x] Add a pinned container or equivalent reproducible environment.
- [x] Embed only redistributable IOP modules from the active SDK.
- [x] Produce an unpacked development ELF and a packed release ELF.
- [x] Add automated compile checks.

## 0.3.0 — Platform Foundation

- [ ] Replace obsolete PS2SDK APIs and handwritten system calls.
- [ ] Correct file-handle error handling and path construction.
- [ ] Implement monotonic high-resolution timing and deterministic frame deltas.
- [ ] Make IOP reset and module loading reliable across launchers.
- [ ] Add serial/host diagnostic logging and useful fatal-error output.

## 0.4.0 — Video and Frame Pacing

- [ ] Correct GS DMA transfer sizes and buffer bounds.
- [ ] Introduce VSync and double buffering.
- [ ] Render at an efficient internal resolution and scale through the GS.
- [ ] Support NTSC, PAL, and 480p with correct aspect ratios.
- [ ] Eliminate tearing and validate 60/50 Hz frame pacing.

## 0.5.0 — Input

- [ ] Implement stateful DualShock 2 button handling.
- [ ] Add analog movement, analog look, dead zones, and sensitivity controls.
- [ ] Support simultaneous button combinations.
- [ ] Preserve optional USB keyboard and mouse support.
- [ ] Add configurable bindings with sensible PS2 defaults.

## 0.6.0 — Audio

- [ ] Replace or validate the archived SDL audio backend.
- [ ] Stabilize DMA mixing, latency, and shutdown behavior.
- [ ] Add volume controls and underrun diagnostics.
- [ ] Decide and document the supported music playback path.

## 0.7.0 — Storage and Game Content

- [ ] Load game data reliably from USB mass storage.
- [ ] Add memory-card save support with clear failure messages.
- [ ] Validate shareware and registered data layouts.
- [ ] Support command-line mods without hard-coded device paths.

## 0.8.0 — Performance and Compatibility

- [ ] Profile representative maps on real hardware.
- [ ] Optimize palette conversion and framebuffer uploads.
- [ ] Reduce renderer stalls and unnecessary full-frame work.
- [ ] Run long-session, save/load, demo, and map-transition tests.

## 0.9.0 — Release Candidate

- [ ] Test on multiple PS2 revisions and common launch methods.
- [ ] Complete NTSC, PAL, and 480p compatibility matrices.
- [ ] Resolve release-blocking crashes, corruption, audio, and input defects.
- [ ] Write user-facing installation and troubleshooting documentation.

## 1.0.0 — Stable

- [ ] Publish a reproducible, hardware-validated release.
- [ ] Maintain stable save compatibility within the 1.x series.
- [ ] Document known limitations and supported configurations.

## Definition of “stable”

Version 1.0.0 requires successful real-hardware testing, correct video timing,
working sound and controls, reliable save/load behavior, and completion of the
original Quake campaign without a release-blocking defect.
