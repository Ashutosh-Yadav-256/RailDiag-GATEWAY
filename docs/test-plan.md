# Verification and Test Plan

**Document ID:** RDG-VTP-001  
**Version:** 0.1

## Strategy

- Build verification: compile using C++17 with GCC/Clang warning flags.
- Integration verification: launch the gateway and communicate through its TCP loopback interface.
- Positive tests: valid normal telemetry and warning/fault conditions.
- Negative tests: malformed frame, unsupported health value, and replayed sequence.
- Operational checks: gateway startup, configurable listener, and orderly termination.

## Test cases

| Test ID | Input / action | Expected result |
|---|---|---|
| IT-001 | Valid normal telemetry | JSON response with `state=OK` |
| IT-002 | Wrong prefix or field count | `invalid_frame` error |
| IT-003 | Non-finite or out-of-range numeric value | `invalid_frame` error |
| IT-004 | Same or lower sequence for same device | `replayed_or_out_of_order_sequence` |
| IT-005 | High temperature / low voltage / device-reported fault | `WARN` or `FAULT` as specified |
| IT-006 | Accepted sample | CSV record appended |
| IT-007 | Two client connections | Both can receive responses |
| IT-008 | SIGINT / SIGTERM | Process exits after stopping listener and joining workers |

## Test result record

Record the commit, compiler version, OS image, command, test output, failures and reviewer for each verification run. CI status alone is not a safety approval.
