#!/usr/bin/env python3
import pathlib
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: smoke_render.py <binary> <output_dir>", file=sys.stderr)
        return 2

    binary = pathlib.Path(sys.argv[1]).resolve()
    output_dir = pathlib.Path(sys.argv[2]).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = output_dir / "smoke_render.bmp"

    command = [
        str(binary),
        "render",
        "--fractal",
        "julia",
        "--julia-c",
        "-0.8",
        "0.156",
        "--palette",
        "fire",
        "--threads",
        "2",
        "160",
        "120",
        "120",
        str(output_path),
    ]
    completed = subprocess.run(command, check=True, capture_output=True, text=True)

    stdout = completed.stdout
    if "HEN-ME CPU fractal render complete" not in stdout:
        raise AssertionError("missing render completion message")
    if "Fractal: julia" not in stdout:
        raise AssertionError("missing fractal name in stdout")
    if "Julia c: (-0.80, 0.16)" not in stdout:
        raise AssertionError("missing julia parameter output")
    if "Threads used: 2" not in stdout:
        raise AssertionError("missing thread count in stdout")
    if "Palette: fire" not in stdout:
        raise AssertionError("missing palette name in stdout")

    if not output_path.exists():
        raise AssertionError("render output file was not created")

    with output_path.open("rb") as handle:
        header = handle.read(2)

    if header != b"BM":
        raise AssertionError(f"unexpected bmp header: {header!r}")

    print("render smoke test passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
