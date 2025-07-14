import os

__version__ = "2.2.0"

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

casm_prefix = os.getenv("CASM_PREFIX")
if casm_prefix is None:
    raise Exception("CASM_PREFIX not set")

# Expected installation layout example:
# C++ libraries: <python package prefix>/libcasm/lib/libcasm_<name>.dylib
rpath = os.path.join(casm_prefix, "lib")

# If on macosx, target 10.15
os.environ["MACOSX_DEPLOYMENT_TARGET"] = "10.15"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules_params = {
    "define_macros": [
        ("VERSION_INFO", __version__),
    ],
    "cxx_std": 17,
    "library_dirs": [
        os.path.join(casm_prefix, "lib"),
    ],
    "include_dirs": [
        os.path.join(casm_prefix, "include/casm/external"),
        os.path.join(casm_prefix, "include"),
    ],
    "extra_compile_args": [
        "-D_LIBCPP_DISABLE_AVAILABILITY",
        "--std=c++17",
    ],
    "extra_link_args": [f"-Wl,-rpath,{rpath}", "-lcasm_global"],
}

ext_modules = [
    Pybind11Extension(
        "libcasm.counter._counter", ["src/counter.cpp"], **ext_modules_params
    ),
    Pybind11Extension(
        "libcasm.casmglobal._casmglobal", ["src/casmglobal.cpp"], **ext_modules_params
    ),
]

setup(
    name="libcasm-global",
    version=__version__,
    packages=["libcasm", "libcasm.casmglobal", "libcasm.counter"],
    install_requires=["pybind11"],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
