# V-model development and verification map

This is a learning-oriented V-model mapping for the demonstrator. It is not a completed EN 50128/EN 50716 lifecycle and must not be treated as a compliance declaration.

| Left side: decomposition | Corresponding right side: verification | Evidence in this project |
|---|---|---|
| System context and intended use | System validation against intended use | `requirements.md`, `test-plan.md` |
| System requirements | System integration tests | `requirements.md`, `tests/test_integration.py` |
| Software architecture | Architecture/integration verification | `architecture.md`, CMake build, integration tests |
| Component design | Component tests | parser/evaluator behavior covered through integration tests |
| Implementation | Static checks and build | compiler warnings, GitHub Actions |

## Change rule

Every change should identify affected requirements, design elements, tests, risks and traceability rows. A requirement with no verification evidence is an open item, not a pass.

## Review independence

For a real safety-related project, review independence, competence, tool confidence, SIL allocation and required deliverables must be determined by the applicable standard, system safety plan and contract. This repository does not establish those conditions.
