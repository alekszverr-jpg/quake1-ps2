#!/usr/bin/env python3
"""Install the pinned PS2DEV toolchain used by this project."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import urllib.request


ROOT = Path(__file__).resolve().parents[1]
LOCK_FILE = ROOT / "tools" / "ps2dev-lock.json"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--cache",
        type=Path,
        default=ROOT / ".cache",
        help="download and installation directory (default: .cache)",
    )
    return parser.parse_args()


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def download(url: str, destination: Path, expected_sha256: str) -> None:
    if destination.is_file() and file_sha256(destination) == expected_sha256:
        print(f"Using cached {destination.name}")
        return

    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_suffix(destination.suffix + ".part")
    if partial.exists():
        partial.unlink()

    print(f"Downloading {url}")
    request = urllib.request.Request(
        url,
        headers={"User-Agent": "quake1-ps2-toolchain-bootstrap/1"},
    )
    digest = hashlib.sha256()
    with urllib.request.urlopen(request) as response, partial.open("wb") as output:
        while chunk := response.read(1024 * 1024):
            output.write(chunk)
            digest.update(chunk)

    actual = digest.hexdigest()
    if actual != expected_sha256:
        partial.unlink(missing_ok=True)
        raise RuntimeError(
            f"SHA-256 mismatch for {destination.name}: "
            f"expected {expected_sha256}, got {actual}"
        )
    partial.replace(destination)


def extract_tar(archive: Path, destination: Path, members: list[str] | None = None) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    command = ["tar", "-xf", str(archive), "-C", str(destination)]
    if members:
        command.extend(members)
    print(f"+ {' '.join(command)}")
    subprocess.run(command, check=True)


def executable(path: Path) -> Path:
    if os.name == "nt":
        return path.with_suffix(".exe")
    return path


def validate_toolchain(ps2dev: Path) -> None:
    compiler = executable(ps2dev / "ee" / "bin" / "mips64r5900el-ps2-elf-gcc")
    bin2c = executable(ps2dev / "ps2sdk" / "bin" / "bin2c")
    packer = executable(ps2dev / "bin" / "ps2-packer")
    for required in (compiler, bin2c, packer):
        if not required.is_file():
            raise RuntimeError(f"toolchain is missing {required}")

    env = os.environ.copy()
    env["PATH"] = os.pathsep.join(
        [
            str(ps2dev / "bin"),
            str(ps2dev / "ee" / "bin"),
            str(ps2dev / "ps2sdk" / "bin"),
            env["PATH"],
        ]
    )
    result = subprocess.run(
        [str(compiler), "--version"],
        env=env,
        check=True,
        text=True,
        capture_output=True,
    )
    print(result.stdout.splitlines()[0])


def main() -> int:
    args = parse_args()
    cache = args.cache.resolve()
    lock = json.loads(LOCK_FILE.read_text(encoding="utf-8"))
    system = platform.system().lower()
    platform_key = "windows" if system == "windows" else "linux"
    if platform_key not in lock["toolchains"]:
        raise RuntimeError(f"unsupported host platform: {platform.system()}")

    toolchain = lock["toolchains"][platform_key]
    archive = cache / toolchain["archive"]
    ps2dev = cache / "ps2dev"
    marker = {
        "schema": lock["schema"],
        "snapshot": lock["snapshot"],
        "platform": platform_key,
        "toolchain_sha256": toolchain["sha256"],
    }
    marker_path = ps2dev / ".quake1-ps2-toolchain.json"
    if marker_path.is_file():
        installed = json.loads(marker_path.read_text(encoding="utf-8"))
        if installed == marker:
            validate_toolchain(ps2dev)
            print(f"Using pinned PS2DEV at {ps2dev}")
            return 0

    download(toolchain["url"], archive, toolchain["sha256"])
    if ps2dev.exists():
        print(f"Replacing outdated PS2DEV at {ps2dev}")
        shutil.rmtree(ps2dev)
    extract_tar(archive, cache)

    if platform_key == "windows":
        runtime_root = cache / "mingw-runtime"
        for package in lock["windows_runtime"]:
            package_archive = cache / package["archive"]
            download(package["url"], package_archive, package["sha256"])
            missing = [
                member
                for member in package["members"]
                if not (runtime_root / member).is_file()
            ]
            if missing:
                extract_tar(package_archive, runtime_root, missing)
            for member in package["members"]:
                source = runtime_root / member
                shutil.copy2(source, ps2dev / "ee" / "bin" / source.name)

    validate_toolchain(ps2dev)
    marker_path.write_text(
        json.dumps(marker, indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"PS2DEV ready at {ps2dev}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
