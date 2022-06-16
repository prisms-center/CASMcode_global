Build and install
=================

Get the necessary build tools: autotools, git, python=3, gitpython, etc. As a conda environment, this could be:

    conda create -n casm_v2 --override-channels -c conda-forge python=3 m4>=1.4.18 autoconf automake make libtool pkg-config ccache wget bzip2 six git gitpython
    conda activate casm_v2

Clone the repository:

    git clone https://github.com/bpuchala/CASMcode_global
    cd CASMcode_global

Initialize submodules:

    git submodule init
    git submodule update

Generate Makemodule.am files and configure script:

    bash bootstrap.sh

Create a build folder:

    mkdir build && cd build

Configure to generate a Makefile. An example local-configure.sh script for building and installing in the current conda environment:
```
CASM_PREFIX=$CONDA_PREFIX

CASM_CXXFLAGS="-O3 -Wall -DNDEBUG -fcolor-diagnostics"
CASM_CC="ccache cc"
CASM_CXX="ccache c++"
CASM_PYTHON="python"
CASM_CONFIGFLAGS="--prefix=$CASM_PREFIX "

../configure CXXFLAGS="${CASM_CXXFLAGS}" CC="$CASM_CC" CXX="$CASM_CXX" PYTHON="$CASM_PYTHON" ${CASM_CONFIGFLAGS}
```

    bash local-configure.sh

Make, check, install:

    make
    make check
    make install
