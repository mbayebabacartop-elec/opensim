# ADR 0005: Core data model and `.olsim` persistence

- Status: accepted for the Stage 3 foundation
- Date: 2026-10-30

## Context

The project model must remain independent of Qt and physical solvers. It needs typed component definitions, SI dimensions, deterministic simulation time/events, and versioned persistence without extracting untrusted archive paths to disk.

## Decision

- Implement the model as standard C++20 value types and `Result<T>` errors in `openlab::core`.
- Store SI dimensions as seven signed exponents in base-unit order (m, kg, s, A, K, mol, cd); numeric values are stored in SI.
- Keep definitions, instances, ports and connections separate from simulation engines.
- Store `.olsim` as ZIP with `manifest.json` and `project.json`; v1 rejects unknown versions and has no migration registered yet.
- Use nlohmann/json 3.12.0 (MIT) and miniz 3.1.2 (permissive upstream license), vendored with notices, so builds and runtime need no network.
- Write a sibling temporary file and replace the destination atomically where supported. Load in memory and validate entry names, count, sizes and checksums; never extract archive paths.

## Consequences

- Core types can be used by a native UI, CLI, tests and future plugins without Qt.
- Archive limits and strict entries reduce accidental resource exhaustion and path traversal; this is not a complete hostile-file security audit.
- V1 is intentionally small. Results, assets, schema migrations, canonical definitions and cryptographic provenance are future work.
- Recheck vendored notices and toolchain settings when creating a release/SBOM.
