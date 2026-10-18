from __future__ import annotations

import argparse
import json
import socket
import sys
import time
from typing import Final, Iterator

DEFAULT_HOST: Final[str] = "127.0.0.1"
DEFAULT_PORT: Final[int] = 9100
DEFAULT_DEVICE: Final[str] = "brake_ecu"


def samples(scenario: str, count: int, device: str) -> Iterator[str]:
    for sequence in range(1, count + 1):
        temperature, voltage, current, health = 62.0, 24.0, 1.5, "OK"
        if scenario == "overtemp":
            temperature = 96.0
        elif scenario == "low-voltage":
            voltage = 16.5
        elif scenario == "device-fault":
            health = "FAULT"
        elif scenario == "warning":
            temperature, voltage, health = 78.0, 20.5, "WARN"
        elif scenario == "compound":
            temperature, voltage, health = 95.0, 20.0, "OK"
        yield f"RDG1|{device}|{sequence}|{temperature:.1f}|{voltage:.1f}|{current:.1f}|{health}"


def run(host: str, port: int, scenario: str, count: int, device: str, period: float) -> int:
    try:
        with socket.create_connection((host, port), timeout=3.0) as connection:
            reader = connection.makefile("r", encoding="utf-8", newline="\n")
            for frame in samples(scenario, count, device):
                connection.sendall((frame + "\n").encode("ascii"))
                response = reader.readline()
                if not response:
                    print("Gateway closed the connection before replying", file=sys.stderr)
                    return 1
                parsed = json.loads(response)
                print(json.dumps(parsed, indent=2))
                if "error" in parsed:
                    return 1
                if period > 0:
                    time.sleep(period)
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        print(f"simulation failed: {exc}", file=sys.stderr)
        return 1
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument(
        "--scenario",
        choices=["normal", "warning", "overtemp", "low-voltage", "device-fault", "compound"],
        default="normal",
    )
    parser.add_argument("--count", type=int, default=5)
    parser.add_argument("--device", default=DEFAULT_DEVICE)
    parser.add_argument("--period", type=float, default=0.25)

    args = parser.parse_args(argv)
    if args.count < 1 or args.period < 0 or not 1 <= args.port <= 65535:
        parser.error("count must be positive, period non-negative, and port in 1..65535")

    return run(args.host, args.port, args.scenario, args.count, args.device, args.period)


if __name__ == "__main__":
    raise SystemExit(main())
