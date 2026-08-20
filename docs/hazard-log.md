# Preliminary hazard / misuse log

**Status:** Brainstorming aid only; not a formal hazard analysis or safety case.

| ID | Hazard / misuse | Possible consequence | Demonstrator mitigation | Residual limitation |
|---|---|---|---|---|
| H-001 | Malformed or non-finite telemetry is accepted | Misleading diagnostic classification | Strict field count, numeric parsing (`std::from_chars`), range checks, and finiteness validation | Does not prove input robustness for all hostile traffic |
| H-002 | Duplicate or out-of-order sample is treated as fresh | Stale state could appear current | Per-device monotonic sequence check with explicit reset support | No cryptographic freshness or distributed time synchronization |
| H-003 | A synthetic threshold is mistaken for a vehicle limit | Incorrect operational decision | Prominent documentation, warning disclaimers, and synthetic data profiles | Human misuse cannot be fully prevented |
| H-004 | Simulation transport is mistaken for TRDP | Incorrect interoperability claim | Explicit protocol naming (`RDG1`) and TRDP limitation notice in `adapters/trdp/` | Requires reviewer discipline |
| H-005 | Gateway is connected to real train control equipment | Unsafe control/diagnostic interaction | No output/control interface; bind defaults to loopback (127.0.0.1) | Not assessed or approved for operational use |
| H-006 | Resource exhaustion / DoS via connection bursts or random device IDs | Gateway crash or unresponsiveness | Automatic worker thread reaping, 512-byte buffer cap, and bounded LRU device tracking (max 1024) | High local traffic volume may still saturate single-host sockets |

Before any real railway use, a qualified team must perform system-level hazard identification, risk assessment, safety requirements allocation, independent verification and validation, and approval under the applicable project standards.
