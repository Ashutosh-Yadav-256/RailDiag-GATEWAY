# Configuration and Change Management

## Baseline identification

- Source baseline: Git commit SHA and release tag.
- Build baseline: compiler, CMake version, build flags and dependency lock information.
- Runtime configuration: bind address, port and log path supplied by command-line arguments.
- Interface baseline: `RDG1` simulation protocol and this repository's SRS version.

## Change request workflow

1. Create a change request with an ID, rationale, affected requirement IDs and risk note.
2. Identify affected source, interface, test and documentation artifacts.
3. Implement the smallest reviewable change on a branch.
4. Add/update tests and traceability rows.
5. Run the build and full integration suite.
6. Obtain peer review; record unresolved comments.
7. Merge only after verification succeeds; tag releases and preserve test evidence.

## Defect handling

Each defect record should include observed behavior, expected behavior, reproduction steps, affected version, severity, root cause, correction, regression test and closure review. A defect affecting a safety assumption must be escalated rather than silently reclassified.

## Configuration change control

Changes to protocol fields, thresholds, ports, runtime dependencies or PLC logic require an explicit change request and compatibility note. No synthetic threshold may be changed and then described as a vehicle-derived limit.
