# Standards mapping and limitations

This project is designed to provide practice material related to the Siemens Train IT vacancy. It is **not compliant, certified, qualified or independently assessed** against any railway standard.

## IEC 61131-3

- The repository contains `plc/VehicleDiagnostic.st`, a Structured Text example of illustrative diagnostic logic.
- It is intended for compilation/experimentation with a separately installed IEC 61131-3 toolchain such as MATIEC.
- The current C++ gateway does not execute this ST program. Runtime integration and toolchain verification remain future work.

## IEC 61375 / TCN / TRDP

- The current running system uses a private TCP simulator protocol named `RDG1`.
- This is **not TRDP**, not TCN, and not an IEC 61375 implementation.
- `adapters/trdp/README.md` documents a planned integration with the C-based TCNOpen TRDP Light stack.
- IEC 61375-2-3 specifies a TCN communication profile for data exchange between consists in trains; actual interoperability depends on the applicable profile, configuration and verified implementation. See the official [IEC publication page](https://webstore.iec.ch/en/publication/22903).

## EN 50128 / EN 50716

- The vacancy references IEC 50128, commonly associated with the railway software standard EN 50128.
- EN 50716:2023 supersedes EN 50128:2011 and EN 50657:2017 in the European standards framework. The applicable national adoption, contract and transition arrangements must be checked for a real project.
- This repository includes learning-oriented requirements, architecture, hazard, verification, V-model and configuration-management artefacts inspired by lifecycle engineering practices.
- It does not reproduce the licensed standard, satisfy all required deliverables, allocate a SIL, establish independence, qualify tools, or demonstrate compliance.

## Official/reference links

- IEC 61375-2-3: https://webstore.iec.ch/en/publication/22903
- EN 50716:2023 overview: https://www.en-standard.eu/bs-en-50716-2023/
- MATIEC overview: https://openplcproject.gitlab.io/matiec/
- TCNOpen mirror (unofficial): https://github.com/HtoTheB/TCNOpen-Mirror

Always use the current licensed standard and project-specific safety plan for actual engineering work.
