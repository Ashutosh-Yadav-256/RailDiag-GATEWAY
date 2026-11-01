# IEC 61375 / TRDP integration boundary

## Current status

**Not implemented in this starter.** The running gateway currently accepts the RailDiag `RDG1` TCP simulation protocol. That format is not TRDP and must not be represented as IEC 61375 support.

## Intended integration

Use the TCNOpen TRDP Light stack as the protocol implementation, behind a transport interface. The selected train interface control document must define the applicable IEC 61375 profile, ComIDs, dataset layout, source/destination addressing, cycle times, timeouts and communication supervision.

Potential public source reference: [TCNOpen mirror](https://github.com/HtoTheB/TCNOpen-Mirror). It is an unofficial mirror; verify its provenance, licensing, revision and suitability with the project owner before adopting it. The TRDP stack is C; a narrow C ABI wrapper can be consumed by the C++ gateway.

## Adapter responsibilities

1. Initialize and close the TRDP stack/session according to the selected TCNOpen release documentation.
2. Subscribe to the configured process-data ComID and validate dataset length and configured schema.
3. Convert received process-data samples into the gateway's internal `Telemetry` model.
4. Publish diagnostic events only where the approved interface specification requires them.
5. Detect missing cycles, duplicate/out-of-order data, invalid datasets and session/network errors.
6. Keep the simulation transport available for deterministic unit/integration tests.

## Required verification before calling it TRDP-capable

- Build against a pinned TCNOpen release and record its commit/hash and license notices.
- Interoperate with an independent TRDP endpoint or a verified reference simulator.
- Test PD publish/subscribe, MD if required, dataset encoding, ComID configuration, timeout handling and network recovery.
- Capture packets and verify the trace against the applicable licensed standard and interface specification.
- Have railway communications specialists review the integration.

Do not claim IEC 61375 conformance based only on linking a TRDP library.
