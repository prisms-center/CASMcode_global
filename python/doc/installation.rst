Installation
============

Currently, CASM packages must be built by cloning the github repositories and building from source.


Install from source
===================

Installation requires:

- Python >=3.8
- git
- Compilers:

  - On Ubuntu linux:

    .. code-block::

        sudo apt-get install build-essential

  - On Mac OSX, install Command Line Tools for XCode:

    .. code-block::

        xcode-select --install


Normal installation of `libcasm-global`:

.. code-block::

    git clone https://github.com/prisms-center/CASMcode_global.git
    cd CASMcode_global
    pip install .

Repeat to install additional CASM packages.

To uninstall:

.. code-block::

    pip uninstall libcasm-global
