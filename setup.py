from skbuild import setup

setup(
    name="libcasm-global",
    version="2.2.0",
    packages=["libcasm", "libcasm.casmglobal", "libcasm.counter"],
    package_dir={"": "python"},
    cmake_install_dir="python/libcasm",
    include_package_data=False,
)
