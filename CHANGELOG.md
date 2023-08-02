# Changelog

All notable changes to `libcasm-global` will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
