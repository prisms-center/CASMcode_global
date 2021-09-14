# Create a conda environment ready for CASM development in osx
#
# Installs boost from a conda package available from the prisms-center channel
#
# Requires the following environment variables:
# - CASM_GLOBAL_DIR: directory where this repo is cloned
# - CASM_ENV_NAME: name to give created conda environment
# - CASM_PYTHON_VERSION: Python version to install
# - CASM_BOOST_VERSION: Boost version to install (casm-boost-cpp17 version)
# - CASM_BOOST_XCODE_BUILD_STR: Build string ( (casm-boost-cpp17 build string)
#
# For default version and build string values:
#
#     `source $CASM_GLOBAL_DIR/devel_scripts/versions.sh`
#

if ! [ -n "$CASM_GLOBAL_DIR" ]; then
  echo "CASM_GLOBAL_DIR not set... stopping."
  exit 1
fi

. $CASM_GLOBAL_DIR/devel_scripts/functions.sh

check_var "CASM_ENV_NAME" "Conda environment name"
check_var "CASM_PYTHON_VERSION" "CASM Python version"
check_var "CASM_BOOST_VERSION" "CASM Boost conda package version"
check_var "CASM_BOOST_XCODE_BUILD_STR" "CASM Boost conda package build string"

# install conda packages into CASM_ENV_NAME environment
conda create -c prisms-center -c conda-forge -c defaults -y -n $CASM_ENV_NAME \
  "python =$CASM_PYTHON_VERSION" \
  "casm-boost-cpp17 =$CASM_BOOST_VERSION $CASM_BOOST_XCODE_BUILD_STR" \
  "m4 >=1.4.18" \
  autoconf \
  automake \
  make \
  libtool \
  pkg-config \
  wget \
  bzip2 \
  six \
  git \
  gitpython \
  jq
