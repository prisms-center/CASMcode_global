# CASMcode_global (libcasm-global)

## Development workflow

**Before testing new code, always install the package:**

    pip install -v --no-build-isolation .

This is required because the package has C++ extensions (pybind11 bindings). Without it, `pytest` will fail with `AttributeError` or `ImportError` on the compiled modules.

**Run all Python tests:**

    pytest -rsap -x python/tests

**Format Python files:**

    black python
    ruff check --fix python

**Format C++ files (staged only):**

    ./stylize.sh

**After adding new source files, update CMakeLists.txt:**

    python make_CMakeLists.py

## C++ tests

```bash
# Build C++ libraries only
mkdir build_cxx_only && cd build_cxx_only
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu) VERBOSE=1

# Build and run C++ tests
mkdir build_tests && cd build_tests
cmake -DCMAKE_BUILD_TYPE=Release ../tests && make -j$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu) VERBOSE=1 && make test
# Log: build_tests/Testing/Temporary/LastTest.log
```

## Documentation

```bash
sphinx-build -b html python/doc $LIBCASM_PYDOCS/global/2.0
```
