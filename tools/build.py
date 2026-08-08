#!/usr/bin/env python3
"""Build Quake for the PlayStation 2 with a current PS2DEV toolchain."""

from __future__ import annotations

import argparse
import concurrent.futures
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DIR = ROOT / "source"

BASE_SOURCES = [
    "chase.c",
    "cl_demo.c",
    "cl_input.c",
    "cl_main.c",
    "cl_parse.c",
    "cl_tent.c",
    "cmd.c",
    "common.c",
    "console.c",
    "crc.c",
    "cvar.c",
    "draw.c",
    "d_edge.c",
    "d_fill.c",
    "d_init.c",
    "d_modech.c",
    "d_part.c",
    "d_polyse.c",
    "d_scan.c",
    "d_sky.c",
    "d_sprite.c",
    "d_surf.c",
    "d_vars.c",
    "d_zpoint.c",
    "host.c",
    "host_cmd.c",
    "keys.c",
    "menu.c",
    "mathlib.c",
    "model.c",
    "nonintel.c",
    "pr_cmds.c",
    "pr_edict.c",
    "pr_exec.c",
    "r_aclip.c",
    "r_alias.c",
    "r_bsp.c",
    "r_light.c",
    "r_draw.c",
    "r_efrag.c",
    "r_edge.c",
    "r_misc.c",
    "r_main.c",
    "r_sky.c",
    "r_sprite.c",
    "r_surf.c",
    "r_part.c",
    "r_vars.c",
    "screen.c",
    "sbar.c",
    "sv_main.c",
    "sv_phys.c",
    "sv_move.c",
    "sv_user.c",
    "zone.c",
    "view.c",
    "wad.c",
    "world.c",
    "cd_null.c",
    "net_vcr.c",
    "net_main.c",
    "net_loop.c",
    "net_none.c",
    "sys_ps2.c",
    "in_ps2.c",
    "ps2_gs.c",
    "vid_ps2.c",
    "ps2.c",
    "pad.c",
]

SOUND_SOURCES = [
    "snd/snd_sdl.c",
    "snd/snd_mix.c",
    "snd/snd_dma.c",
    "snd/snd_mem.c",
]

IRX_MODULES = {
    "iomanx_irx": "iomanX.irx",
    "filexio_irx": "fileXio.irx",
    "bdm_irx": "bdm.irx",
    "bdmfs_fatfs_irx": "bdmfs_fatfs.irx",
    "usbd_irx": "usbd.irx",
    "usbmass_bd_irx": "usbmass_bd.irx",
    "ps2kbd_irx": "ps2kbd.irx",
    "ps2mouse_irx": "ps2mouse.irx",
}

VIDEO_DEFINES = {
    "ntsc": "_NTSC",
    "pal": "_PAL",
    "vesa": "_VESA",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--video",
        choices=sorted(VIDEO_DEFINES),
        default="ntsc",
        help="video mode to compile (default: ntsc)",
    )
    parser.add_argument(
        "--internal-width",
        type=int,
        choices=(320, 640),
        default=640,
        help="software-renderer width before GS scaling (default: 640)",
    )
    parser.add_argument(
        "--sound",
        action="store_true",
        help="enable the archived SDL audio backend",
    )
    parser.add_argument(
        "--metrics",
        action="store_true",
        help="draw PS2 frame-stage timing metrics on screen",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=max(1, os.cpu_count() or 1),
        help="parallel compiler processes",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="remove the selected build directory before compiling",
    )
    return parser.parse_args()


def executable(directory: Path, name: str) -> Path:
    suffix = ".exe" if os.name == "nt" else ""
    candidate = directory / f"{name}{suffix}"
    if not candidate.is_file():
        raise FileNotFoundError(f"required tool not found: {candidate}")
    return candidate


def find_ps2dev() -> Path:
    configured = os.environ.get("PS2DEV")
    candidates = [Path(configured)] if configured else []
    candidates.append(ROOT / ".cache" / "ps2dev")
    for candidate in candidates:
        if (candidate / "ps2sdk").is_dir() and (candidate / "ee").is_dir():
            return candidate.resolve()
    raise FileNotFoundError(
        "PS2DEV was not found. Set PS2DEV or run tools/bootstrap_ps2dev.py."
    )


def run(command: list[str], env: dict[str, str]) -> None:
    display = " ".join(command)
    print(f"+ {display}", flush=True)
    subprocess.run(command, cwd=ROOT, env=env, check=True)


def main() -> int:
    args = parse_args()
    ps2dev = find_ps2dev()
    ps2sdk = ps2dev / "ps2sdk"
    tool_bin = ps2dev / "ee" / "bin"
    sdk_bin = ps2sdk / "bin"

    compiler = executable(tool_bin, "mips64r5900el-ps2-elf-gcc")
    bin2c = executable(sdk_bin, "bin2c")
    packer = executable(ps2dev / "bin", "ps2-packer")

    env = os.environ.copy()
    env["PS2DEV"] = str(ps2dev)
    env["PS2SDK"] = str(ps2sdk)
    env["GSKIT"] = str(ps2dev / "gsKit")
    path_entries = [
        ps2dev / "bin",
        tool_bin,
        ps2dev / "iop" / "bin",
        ps2dev / "dvp" / "bin",
        sdk_bin,
    ]
    env["PATH"] = os.pathsep.join(map(str, path_entries)) + os.pathsep + env["PATH"]

    build_profile = args.video
    if args.internal_width != 640:
        build_profile = f"{args.video}-{args.internal_width}"
    if args.metrics:
        build_profile = f"{build_profile}-metrics"
    build_dir = ROOT / "build" / build_profile
    object_dir = build_dir / "obj"
    generated_dir = build_dir / "generated"
    if args.clean and build_dir.exists():
        shutil.rmtree(build_dir)
    object_dir.mkdir(parents=True, exist_ok=True)
    generated_dir.mkdir(parents=True, exist_ok=True)

    generated_sources: list[Path] = []
    irx_dir = ps2sdk / "iop" / "irx"
    for label, filename in IRX_MODULES.items():
        output = generated_dir / f"{label}.c"
        run([str(bin2c), str(irx_dir / filename), str(output), label], env)
        generated_sources.append(output)

    sources = [SOURCE_DIR / item for item in BASE_SOURCES]
    if args.sound:
        sources.extend(SOURCE_DIR / item for item in SOUND_SOURCES)
    else:
        sources.append(SOURCE_DIR / "snd_null.c")
    sources.extend(generated_sources)

    common_flags = [
        "-std=gnu17",
        "-D_EE",
        "-DNEWLIB_PORT_AWARE",
        f"-D{VIDEO_DEFINES[args.video]}",
        f"-DPS2_INTERNAL_WIDTH={args.internal_width}",
        "-D_IOPRESET",
        "-Dstricmp=strcasecmp",
        "-G0",
        "-O2",
        "-fcommon",
        "-gdwarf-2",
        "-Wall",
        "-Wno-unused-but-set-variable",
        "-Wno-unused-variable",
        "-Wno-unused-function",
        f"-I{SOURCE_DIR}",
        f"-I{ps2sdk / 'ee' / 'include'}",
        f"-I{ps2sdk / 'common' / 'include'}",
    ]
    if args.sound:
        common_flags.extend(
            [
                "-D_SOUND",
                "-DSDL",
                f"-I{ps2sdk / 'ports' / 'include' / 'SDL'}",
            ]
        )
    if args.metrics:
        common_flags.append("-DPS2_DIAGNOSTIC_METRICS")

    compile_jobs: list[tuple[Path, Path]] = []
    for index, source in enumerate(sources):
        relative = source.relative_to(ROOT) if source.is_relative_to(ROOT) else source
        safe_name = "_".join(relative.parts).replace(".c", "")
        target = object_dir / f"{index:03d}_{safe_name}.o"
        compile_jobs.append((source, target))

    def compile_one(job: tuple[Path, Path]) -> None:
        source, target = job
        run(
            [
                str(compiler),
                *common_flags,
                "-c",
                str(source),
                "-o",
                str(target),
            ],
            env,
        )

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = [executor.submit(compile_one, job) for job in compile_jobs]
        for future in concurrent.futures.as_completed(futures):
            future.result()

    unpacked_elf = build_dir / "quake.elf"
    link_command = [
        str(compiler),
        f"-T{ps2sdk / 'ee' / 'startup' / 'linkfile'}",
        "-O2",
        "-o",
        str(unpacked_elf),
        *(str(target) for _, target in compile_jobs),
        f"-L{ps2sdk / 'ee' / 'lib'}",
        f"-L{ps2sdk / 'ports' / 'lib'}",
        "-Wl,-zmax-page-size=128",
        "-lmouse",
        "-lkbd",
        "-lpad",
        "-lfileXio",
        "-lpatches",
        "-ldebug",
        "-lgraph",
        "-ldma",
        "-lm",
        "-lc",
    ]
    if args.sound:
        link_command.extend(["-lsdl", "-lsdlmain"])
    run(link_command, env)

    packed_elf = build_dir / "quake-packed.elf"
    run([str(packer), str(unpacked_elf), str(packed_elf)], env)

    print(f"Built: {unpacked_elf.relative_to(ROOT)}")
    print(f"Built: {packed_elf.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
