# Changelog

All notable changes to `libcasm-global` will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.2.0] - 2025-08-14

### Changed

- Set pybind11~=3.0


## [2.1.0] - 2025-08-07

### Changed

- Build Linux wheels using manylinux_2_28 (previously manylinux2014)


## [2.0.6] - 2025-05-02

### Changed

- Build for Python 3.13
- Restrict requires-python to ">=3.9,<3.14"
- Run CI tests using Python 3.13
- Build MacOS arm64 wheels using MacOS 15
- Build Linux wheels using Ubuntu 24.04


## [2.0.5] - 2024-07-11

### Changed

- Wheels compiled with numpy>=2.0.0
- Run github actions on push, pull_request, and weekly
- Use ruff NPY201


## [2.0.4] - 2024-01-26

### Fixed

- Fix index_to_kcombination  (933735c)

### Updated

- Updated docs theme

### Added

- Building wheel for Python3.12


## [2.0.3] - 2023-08-10

### Changed

- Updated project metadata and README

### Removed

- Removed CONTRIBUTE.md. This information is now included on the CASM website.


## [2.0.2] - 2023-08-09

### Changed

- Changed to build x86_64 wheels with Github Actions and aarch64 and arm64 wheels with Cirrus-CI


## [2.0.1] - 2023-08-02

### Added

- Build wheels for Linux aarch64.

### Changed

- Changed libcasm_global install location to site_packages/libcasm/lib for all architectures.
- Updated docs to refer to installation and contribution guidelines on CASMcode_docs page.
- Changed C++ tests to use a separate CMake project and fetch googletest

### Removed
- Removed googletest submodule


## [2.0.0] - 2023-07-18

### Added

- This module includes parts of CASM v1 that are generically useful, including: casm/casm_io, casm/container, casm/external/Eigen, casm/external/gzstream, casm/external/MersenneTwister, casm/global, casm/misc, and casm/system
- This module enables installing via pip install, using scikit-build, CMake, and pybind11
- Added external/nlohmann JSON implementation
- Added external/pybind11_json
- Added Python package libcasm.casmglobal with CASM global constants
- Added Python package libcasm.counter with IntCounter and FloatCounter
- Added GitHub Actions for unit testing
- Added GitHub Action build_wheels.yml for Python wheel building using cibuildwheel
- Added Python documentation

### Changed

- Changed KB and PLANCK to CODATA 2014 suggestions

### Removed

- Removed autotools build process
- Removed boost dependencies
- Removed external/json_spirit
- Removed external/fadbad
- Removed external/qhull
