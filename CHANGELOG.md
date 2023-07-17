# Changelog

All notable changes to `libcasm-global` will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- This module includes parts of CASM v1 that are generically useful, including: casm/casm_io, casm/container, casm/external/Eigen, casm/external/gzstream, casm/external/MersenneTwister, casm/global, casm/misc, and casm/system
- This module enables installing via pip install, using scikit-build, CMake, and pybind11
- Added external/nlohmann JSON implementation
- Added external/pybind11_json
- Added Python package libcasm.casmglobal with CASM global constants
- Added Python package libcasm.container with IntCounter and FloatCounter
- Added GitHub Actions for unit testing
- Added GitHub Action build_wheels.yml for Python wheel building using cibuildwheel
- Added Python documentation

### Removed

- Removed autotools build process
- Removed boost dependencies
- Removed external/json_spirit
- Removed external/fadbad
- Removed external/qhull