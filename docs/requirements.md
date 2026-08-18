# System Requirements Specification (SRS)

**Document ID:** RDG-SRS-001  
**Version:** 0.2  
**Status:** Demonstrator baseline (ECC Rebuilt)  
**Scope:** Educational Linux diagnostics gateway; no real train deployment.

## Functional requirements

| ID | Requirement | Verification |
|---|---|---|
| RDG-FR-001 | The gateway shall accept newline-delimited `RDG1` samples over the local TCP simulation interface. | IT-001 |
| RDG-FR-002 | The gateway shall reject malformed frames and unsupported health values with a structured error response. | IT-002 / UT-001 |
| RDG-FR-003 | The gateway shall validate numeric fields for finiteness and demonstrator bounds. | IT-003 / UT-001 |
| RDG-FR-004 | The gateway shall reject duplicate or out-of-order sequence numbers for each device with bounded memory tracking. | IT-004 / UT-003 |
| RDG-FR-005 | The gateway shall classify telemetry as `OK`, `WARN`, or `FAULT` while preserving all co-occurring event diagnostics. | IT-005 / UT-002 |
| RDG-FR-006 | The gateway shall append accepted diagnostic samples to a CSV log with ISO 8601 UTC and monotonic microsecond timestamps. | IT-006 |
| RDG-FR-007 | The gateway shall handle multiple client connections concurrently and automatically reclaim finished worker threads. | IT-007 |
| RDG-FR-008 | The process shall support orderly termination using SIGINT or SIGTERM via non-blocking listener polling. | IT-008 |

## Interface requirements

| ID | Requirement |
|---|---|
| RDG-IF-001 | The simulation listener shall default to 127.0.0.1:9100 and permit explicit bind/port configuration. |
| RDG-IF-002 | Each accepted sample shall contain exactly seven `|`-delimited fields beginning with `RDG1`. |
| RDG-IF-003 | Each valid sample shall receive one newline-delimited JSON response. |
| RDG-IF-004 | Device identifiers shall be limited to ASCII letters, digits, underscore and hyphen (max 48 chars). |

## Safety-related assumptions and constraints

- RDG is not connected to traction, braking, door, signaling or any train control output.
- Values and thresholds in this demonstrator are synthetic and are not derived from a vehicle specification.
- The software has no allocated Safety Integrity Level (SIL).
- No fail-safe claim is made. A software `FAULT` label is diagnostic output only.
- Any future railway application requires hazard analysis, independent review, system integration evidence and approval under the applicable standards and contract.
