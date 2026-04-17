## General notes

- Do not include "Co-Authored-By" Claude in commit messages.

---

## CASM packages

### C++ extension packages

- `CASMcode_global` (libcasm-global)
- `CASMcode_crystallography` (libcasm-xtal)
- `CASMcode_composition` (libcasm-composition)
- `CASMcode_mapping` (libcasm-mapping)
- `CASMcode_clexulator` (libcasm-clexulator)
- `CASMcode_configuration` (libcasm-configuration)
- `CASMcode_monte` (libcasm-monte)
- `CASMcode_clexmonte` (libcasm-clexmonte)

### Pure Python packages

- `CASMcode_bset` (casm-bset)
- `CASMcode_project` (casm-project)
- `CASMcode_tools` (casm-tools)

---

## Package structure

### C++ extension packages

- `.github/` — GitHub workflow files
- `cmake/` — CMake modules
- `doc/` — Doxygen documentation source
- `include/` — C++ headers
- `python/doc/` — Sphinx docs; `python/libcasm/` — Python source; `python/src/` — pybind11 bindings; `python/tests/` — tests
- `src/` — C++ source; `tests/unit/` — C++ unit tests
- Key files: `CMakeLists.txt.in`, `make_CMakeLists.py`, `pyproject.toml`, `setup.py`, `stylize.sh`, `CHANGELOG.md`
- `python/pyproject.toml` and `python/setup.py` are **editable-mode only**

### Pure Python packages

- `.github/` — workflows; `casm/` — Python source; `doc/` — Sphinx docs; `tests/` — Python tests
- Key files: `pyproject.toml`, `setup.py`, `CHANGELOG.md`, `requirements.txt`

---

## Development

Local development is done in a conda environment with dependencies and `CASM_PREFIX` already set.

Each repo's `agents/AGENTS.md` contains repo-specific dev workflow commands.

For versioning procedures, use the `/casm-version` skill.
For release procedures, use the `/casm-release` skill.

Release and workflow scripts live in `CASMcode_global/dev/`:

- `dev/release.py` — full release workflow (run from package root as `python ../CASMcode_global/dev/release.py`)
- `dev/download_release.py` — download build artifacts from GitHub Actions
- `dev/update_workflow_versions.py` — update CASM dependency versions in `.github/workflows/*.yml`
