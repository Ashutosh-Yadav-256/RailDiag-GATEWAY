# RailDiag Gateway

**A Linux-based onboard vehicle diagnostics gateway demonstrator**

RailDiag is an engineering demonstrator inspired by train onboard diagnostics systems. It combines a C++17 Linux gateway, a Python telemetry simulator, an IEC 61131-3 Structured Text example, a transport boundary for future TRDP integration, automated tests, and railway-style lifecycle documentation.

> **Safety and standards notice:** This is an educational demonstrator, not railway equipment and not suitable for use in a real train or safety-related application. It is not certified or assessed against EN 50128, EN 50716, IEC 61131-3, or IEC 61375. The Structured Text file is an IEC 61131-3-style example; the current transport is a local TCP simulator, not a conformant TRDP stack. See [`docs/standards-and-limitations.md`](docs/standards-and-limitations.md).

## What it demonstrates

| Engineering Theme | Demonstrator Evidence |
|---|---|
| C++ Systems Development | C++17 modular gateway, typed telemetry model, non-blocking polling, worker thread reclamation, $O(1)$ LRU sequence tracking, and multi-event diagnostic rules |
| Linux Runtime | POSIX sockets, non-blocking `poll()`, async-signal-safe termination, and CMake build automation |
| Python Simulation & Integration | Deterministic sensor simulator and integration-test harness with automated shutdown verification |
| IEC 61131-3 | Structured Text diagnostic-function example in `plc/` synchronized with gateway logic |
| IEC 61375 / TCN | TRDP integration boundary and integration plan in `adapters/trdp/` |
| Safety Lifecycle / V-Model | Requirements, architecture, hazard log, verification plan, and traceability matrix in `docs/` |
| Multi-tier Testing | 24 native C++ GoogleTests via CTest and 12 Python integration tests |

## Architecture

```text
       Subsystem ECUs / Simulator
      (Normal, Warning, Fault, Compound)
                     │
              TCP Port 9100
             ASCII Protocol (RDG1)
                     │
         ┌───────────────────────┐
         │  raildiag-gateway     │
         │───────────────────────│
         │ • Frame Parser        │ ◄── Zero-copy string_view, finite float checks
         │ • Sequence Guard      │ ◄── O(1) LRU capacity cache (max 1024 devices)
         │ • Diagnostic Engine   │ ◄── Multi-event rule evaluation
         │ • Audit Logger        │ ◄── ISO 8601 UTC + steady monotonic µs
         │ • TCP Server          │ ◄── Non-blocking poll() + worker thread reaper
         └───────────────────────┘
                     │
             JSON Diagnostics
                     │
         Test Client / Downstream Bus
```

See [`docs/architecture.md`](docs/architecture.md) for detailed class specifications, sequence diagrams, and concurrency design.

## Quick start (Ubuntu / Debian / WSL)

Prerequisites: GCC or Clang with C++17 support, CMake 3.16+, `libgtest-dev`, Python 3.10+.

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgtest-dev python3
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run the gateway in one terminal:

```bash
./build/raildiag-gateway --bind 127.0.0.1 --port 9100 --log diagnostics.csv
```

Run a sensor in another terminal:

```bash
python3 -m pip install -e .
raildiag-sim --scenario normal --count 5
raildiag-sim --scenario overtemp --count 3
raildiag-sim --scenario low-voltage --count 3
raildiag-sim --scenario device-fault --count 2
raildiag-sim --scenario compound --count 2
```

Stop the gateway with `Ctrl+C`. The CSV log is written to `--log`.

## Run the tests

### 1. Native C++ GoogleTest Unit Tests (CTest)

```bash
ctest --test-dir build --output-on-failure
```

### 2. Python Integration Tests

```bash
PYTHONPATH=python RAILDIAG_BINARY="$PWD/build/raildiag-gateway" python3 -m unittest discover -s tests -v
```

### 3. Run Everything via Makefile

```bash
make test
```

## Telemetry protocol (simulation only)

One sample per newline:

```text
RDG1|<device_id>|<sequence>|<temperature_c>|<voltage_v>|<current_a>|<health>
```

Example:

```text
RDG1|brake_ecu|1|62.5|24.1|1.8|OK
```

`device_id` is restricted to letters, digits, `_` and `-` (max 48 characters). Sequence numbers must increase per device. The gateway rejects malformed, non-finite, out-of-range, and replayed/out-of-order samples.

Diagnostic thresholds in this demonstrator:

- `FAULT`: device health is `FAULT`, temperature is at least 90 °C, or voltage is below 18 V.
- `WARN`: device health is `WARN`, temperature is at least 75 °C, or voltage is below 21 V.
- `OK`: otherwise.

All active event labels are captured in the JSON `events` array without suppression, enabling rich multi-symptom diagnostics.

## Project layout

```text
include/raildiag/        Modular C++ headers (config, telemetry, diagnostic, guard, logger, server)
src/                    C++17 implementations
python/rail_diag/        Python sensor simulator
plc/                    IEC 61131-3 Structured Text example
adapters/trdp/           TRDP integration boundary and design notes
tests/                  Native GoogleTest C++ unit tests & Python integration tests
docs/                   Requirements, architecture, hazards, traceability, test plan
.github/workflows/       Linux CI workflow
```

## Documentation

| Document | Purpose |
|---|---|
| [**Architecture & Design (SAD)**](docs/architecture.md) | Component architecture, class diagrams, wire protocol, sequence flows, and concurrency |
| [**Requirements (SRS)**](docs/requirements.md) | Functional and interface requirements with verification mappings |
| [**Traceability Matrix**](docs/traceability.csv) | Traceability linking requirements to test verification IDs |
| [**Hazard Log**](docs/hazard-log.md) | Preliminary hazard analysis and software mitigations |
| [**Standards & Limitations**](docs/standards-and-limitations.md) | Mapping against EN 50128/EN 50716, IEC 61375, and IEC 61131-3 |
| [**Test Plan**](docs/test-plan.md) | Test strategy for native unit tests and Python integration tests |
| [**Configuration Management**](docs/configuration-management.md) | Versioning and change management procedures |
