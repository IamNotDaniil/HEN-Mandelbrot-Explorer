#!/usr/bin/env python3
import csv
import json
import pathlib
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: smoke_benchmark.py <binary> <output_dir>", file=sys.stderr)
        return 2

    binary = pathlib.Path(sys.argv[1]).resolve()
    output_dir = pathlib.Path(sys.argv[2]).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    command = [
        str(binary),
        "benchmark",
        "--fractal",
        "julia",
        "--julia-c",
        "-0.4",
        "0.6",
        "--thread-sweep",
        "1,2",
        "--palette",
        "ice",
        str(output_dir),
    ]
    completed = subprocess.run(command, check=True, capture_output=True, text=True)

    stdout = completed.stdout
    if "Running CPU benchmark suite..." not in stdout:
        raise AssertionError("missing benchmark start message")
    if "Benchmark CSV:" not in stdout:
        raise AssertionError("missing benchmark csv message")
    if "Benchmark JSON:" not in stdout:
        raise AssertionError("missing benchmark json message")

    csv_path = output_dir / "benchmark_results.csv"
    json_path = output_dir / "benchmark_results.json"
    if not csv_path.exists():
        raise AssertionError("benchmark csv was not created")
    if not json_path.exists():
        raise AssertionError("benchmark json was not created")

    expected_images = [
        output_dir / "benchmark_720p_julia_t1_preset_1_t1.ppm",
        output_dir / "benchmark_720p_julia_t2_preset_2_t2.ppm",
        output_dir / "benchmark_1080p_julia_t1_preset_3_t1.ppm",
        output_dir / "benchmark_1080p_julia_t2_preset_4_t2.ppm",
        output_dir / "benchmark_zoom_julia_t1_preset_5_t1.ppm",
        output_dir / "benchmark_zoom_julia_t2_preset_6_t2.ppm",
    ]
    for image in expected_images:
        if not image.exists():
            raise AssertionError(f"missing benchmark image: {image}")

    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    if len(rows) != 6:
        raise AssertionError(f"expected 6 benchmark rows, got {len(rows)}")

    required_columns = {
        "preset",
        "fractal",
        "resolution",
        "max_iterations",
        "palette",
        "viewport",
        "julia_c_real",
        "julia_c_imag",
        "threads_requested",
        "threads_used",
        "interior_points_skipped",
        "time_ms",
        "mpx_per_s",
        "output",
    }
    missing = required_columns.difference(rows[0].keys())
    if missing:
        raise AssertionError(f"missing csv columns: {sorted(missing)}")

    seen_threads = set()
    for row in rows:
        if row["fractal"] != "julia":
            raise AssertionError(f"expected overridden fractal 'julia', got {row['fractal']!r}")
        if row["palette"] != "ice":
            raise AssertionError(f"expected overridden palette 'ice', got {row['palette']!r}")
        if row["threads_requested"] not in {"1", "2"}:
            raise AssertionError(f"expected threads_requested in {{1,2}}, got {row['threads_requested']!r}")
        seen_threads.add(row["threads_requested"])

    if seen_threads != {"1", "2"}:
        raise AssertionError(f"expected both sweep thread counts, got {seen_threads!r}")

    payload = json.loads(json_path.read_text(encoding="utf-8"))
    results = payload.get("results")
    if not isinstance(results, list) or len(results) != 6:
        raise AssertionError("benchmark json results payload is invalid")

    seen_json_threads = set()
    for item in results:
        if item.get("fractal") != "julia":
            raise AssertionError(f"expected JSON fractal 'julia', got {item.get('fractal')!r}")
        if item.get("palette") != "ice":
            raise AssertionError(f"expected JSON palette 'ice', got {item.get('palette')!r}")
        if item.get("threads_requested") not in {"1", "2"}:
            raise AssertionError(f"expected JSON threads_requested in {{1,2}}, got {item.get('threads_requested')!r}")
        seen_json_threads.add(item.get("threads_requested"))

    if seen_json_threads != {"1", "2"}:
        raise AssertionError(f"expected both JSON sweep thread counts, got {seen_json_threads!r}")

    print("benchmark smoke test passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
