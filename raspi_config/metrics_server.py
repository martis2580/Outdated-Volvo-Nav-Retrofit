#!/usr/bin/env python3
"""Serve the Volvo HUDIY dashboard and lightweight Raspberry Pi metrics."""

from __future__ import annotations

import json
import glob
import os
import re
import subprocess
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parent
HOST = "127.0.0.1"
PORT = 8765
_previous_cpu: tuple[int, int] | None = None


def read_cpu_usage() -> float | None:
    """Return total CPU busy percentage since the previous sample."""
    global _previous_cpu

    try:
        with open("/proc/stat", encoding="ascii") as stat_file:
            fields = stat_file.readline().split()
        if not fields or fields[0] != "cpu":
            return None
        values = [int(value) for value in fields[1:]]
        idle = values[3] + (values[4] if len(values) > 4 else 0)
        total = sum(values)
    except (OSError, ValueError):
        return None

    current = (total, idle)
    previous = _previous_cpu
    _previous_cpu = current
    if previous is None:
        return None

    total_delta = total - previous[0]
    idle_delta = idle - previous[1]
    if total_delta <= 0:
        return None
    return round(max(0.0, min(100.0, 100.0 * (total_delta - idle_delta) / total_delta)), 1)


def read_memory() -> dict[str, int | float] | None:
    """Return total and used memory using Linux's MemAvailable estimate."""
    values: dict[str, int] = {}
    try:
        with open("/proc/meminfo", encoding="ascii") as meminfo:
            for line in meminfo:
                key, raw_value = line.split(":", 1)
                if key in {"MemTotal", "MemAvailable"}:
                    values[key] = int(raw_value.strip().split()[0])
    except (OSError, ValueError, IndexError):
        return None

    total = values.get("MemTotal")
    available = values.get("MemAvailable")
    if not total or available is None:
        return None
    used = max(0, total - available)
    return {
        "used_percent": round(100.0 * used / total, 1),
        "used_mib": round(used / 1024, 0),
        "total_mib": round(total / 1024, 0),
    }


def read_cpu_temperature_c() -> float | None:
    """Read the SoC temperature, preferring Raspberry Pi's vcgencmd reading."""
    try:
        result = subprocess.run(
            ["vcgencmd", "measure_temp"],
            check=True,
            capture_output=True,
            text=True,
            timeout=1,
        )
        match = re.search(r"temp=([0-9]+(?:\.[0-9]+)?)", result.stdout)
        if match:
            return round(float(match.group(1)), 1)
    except (OSError, subprocess.SubprocessError, ValueError):
        pass

    # Linux thermal-zone values are reported in millidegrees Celsius.
    try:
        raw_value = Path("/sys/class/thermal/thermal_zone0/temp").read_text().strip()
        return round(int(raw_value) / 1000, 1)
    except (OSError, ValueError):
        return None


def read_cooler_rpm() -> int | None:
    """Read the Raspberry Pi 5 cooling-fan tachometer from hwmon sysfs."""
    candidates = glob.glob(
        "/sys/devices/platform/cooling_fan/hwmon/hwmon*/fan1_input"
    )

    # Also support the fan if the kernel exposes it only through hwmon class.
    for name_path in glob.glob("/sys/class/hwmon/hwmon*/name"):
        try:
            name = Path(name_path).read_text().strip().lower()
        except OSError:
            continue
        if name in {"pwmfan", "cooling_fan"}:
            candidates.extend(
                glob.glob(str(Path(name_path).parent / "fan*_input"))
            )

    for path in dict.fromkeys(candidates):
        try:
            return max(0, int(Path(path).read_text().strip()))
        except (OSError, ValueError):
            continue
    return None


def metrics() -> dict[str, Any]:
    return {
        "cpu_percent": read_cpu_usage(),
        "memory": read_memory(),
        "cpu_temp_c": read_cpu_temperature_c(),
        "fan_rpm": read_cooler_rpm(),
        "updated_at": int(time.time()),
    }


class DashboardHandler(BaseHTTPRequestHandler):
    def do_GET(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler API
        if self.path == "/api/metrics":
            body = json.dumps(metrics(), separators=(",", ":")).encode("utf-8")
            content_type = "application/json; charset=utf-8"
        elif self.path in {"/", "/index.html"}:
            try:
                body = (ROOT / "rpi_performance.html").read_bytes()
            except OSError:
                self.send_error(500, "Dashboard page is unavailable")
                return
            content_type = "text/html; charset=utf-8"
        else:
            self.send_error(404)
            return

        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, _format: str, *_args: object) -> None:
        # Keep the systemd journal quiet during the dashboard's regular polling.
        return


def main() -> None:
    os.umask(0o077)
    server = ThreadingHTTPServer((HOST, PORT), DashboardHandler)
    server.serve_forever(poll_interval=0.5)


if __name__ == "__main__":
    main()
