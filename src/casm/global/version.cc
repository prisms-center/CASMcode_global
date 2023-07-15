#include "casm/global/version.hh"

#ifndef TXT_VERSION
#define TXT_VERSION "unknown"
#endif

namespace CASM {

const std::string &version() {
  static const std::string ver = TXT_VERSION;
  return ver;
}

const std::string &libcasm_global_version() {
  static const std::string ver = TXT_VERSION;
  return ver;
}

}  // namespace CASM
