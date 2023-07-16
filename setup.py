from skbuild import setup

setup(
    name="libcasm-global",
    version="2.0.0",
    packages=["libcasm", "libcasm.casmglobal", "libcasm.container"],
    package_dir={"": "python"},
    cmake_install_dir="python/libcasm",
)
