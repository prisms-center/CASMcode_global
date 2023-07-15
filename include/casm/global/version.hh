#ifndef CASM_global_version
#define CASM_global_version

#include <string>

namespace CASM {

const std::string &version();  // Returns the version defined by the TXT_VERSION
                               // macro at compile time

const std::string &libcasm_global_version();

}  // namespace CASM

extern "C" {

/// \brief Return the libcasm_global version number
inline const char *casm_global_version() { return CASM::version().c_str(); }
}

#endif
