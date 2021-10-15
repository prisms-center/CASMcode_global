#include "casm/system/RuntimeLibrary.hh"

#include <filesystem>
#include <vector>

#include "casm/global/filesystem.hh"
#include "casm/misc/string_algorithm.hh"
#include "casm/system/Popen.hh"

namespace CASM {

/// \defgroup System
///
/// \brief Helpers for running subprocesses and runtime linking

/// \class runtime_lib_compile_error
///
/// \brief RuntimeLibrary compilation errors
///
/// \relates RuntimeLibrary

runtime_lib_compile_error::runtime_lib_compile_error(std::string _filename_base,
                                                     std::string _cmd,
                                                     std::string _result,
                                                     std::string _what)
    : std::runtime_error(_what),
      filename_base(_filename_base),
      cmd(_cmd),
      result(_result) {}

void runtime_lib_compile_error::print(std::ostream &sout) const {
  sout << "Error compiling: " << filename_base + ".cc" << std::endl;
  sout << "Attempted: " << cmd << std::endl;
  sout << result << std::endl;
  sout << what() << std::endl;
}

/// \class runtime_lib_shared_error
///
/// \brief RuntimeLibrary shared object compilation errors
///
/// \relates RuntimeLibrary

runtime_lib_shared_error::runtime_lib_shared_error(std::string _filename_base,
                                                   std::string _cmd,
                                                   std::string _result,
                                                   std::string _what)
    : std::runtime_error(_what),
      filename_base(_filename_base),
      cmd(_cmd),
      result(_result) {}

void runtime_lib_shared_error::print(std::ostream &sout) const {
  sout << "Error compiling shared object: " << filename_base + ".so"
       << std::endl;
  sout << "Attempted: " << cmd << std::endl;
  sout << result << std::endl;
  sout << what() << std::endl;
}

/// \class RuntimeLibrary
///
/// \brief Compile, load, and use code at runtime
///
/// RuntimeLibrary encapsulates compiling, loading, and using a library at
/// runtime. CASM generates code to evaluate cluster expansion basis functions,
/// and utilizes RuntimeLibrary to compile and run that code. CASM also uses
/// RuntimeLibrary for user plugins. Internally, RuntimeLibrary uses `dlopen`
/// and `dlclose`.
///
/// ## Using RuntimeLibrary
///
/// Functions with a known name and signature can be obtained from a library
/// using the `RuntimeLibrary::get_function` method and returned as a
/// `std::function`. Example:
/// \code
/// std::shared_ptr<RuntimeLibrary> lib = ...;
///
/// // get the 'int add(int, int)' function
/// std::function<int(int, int)> add = lib->get_function<int(int, int)>("add");
/// \endcode
///
/// To enable runtime symbol lookup, a library should use C-style functions
/// (i.e use `extern "C"` for functions that should be available via
/// `RuntimeLibrary::get_function`).
///
/// The RuntimeLibrary must exist for the entire lifetime of functions obtained
/// from the library. It is best to only open library once. For this reason,
/// it is useful to store RuntimeLibrary in `std::shared_ptr`.
///
/// ## Constructing RuntimeLibrary
///
/// To construct RuntimeLibrary, the path of the source file, the compilation
/// options, and shared object compilation options are provided. If the library
/// is already compiled it is loaded directly, otherwise it is first compiled
/// and then loaded. Example:
/// \code
/// // assumes MyLibrary has a '.cc" extension
/// std::string filename_base = "/path/to/MyLibrary";
///
/// // compilation options
/// std::string compile_options =
///     "g++ -O3 -Wall -fPIC --std=c++17 -I/path/to/include";
///
/// // shared object compilation options
/// std::string so_options = "-shared -L/path/to/lib -lcasm_global";
///
/// std::shared_ptr<RuntimeLibrary> lib = std::make_shared<RuntimeLibrary(
///     filename_base, compile_options, so_options);
/// \endcode
///
/// ## Configuration with environment variables
///
/// It may be useful to configuration the constructor options using environment
/// variables, so RuntimeLibrary has some helper functions for checking
/// environment variables. The following static methods return a pair of
/// strings, the first giving the value found (or a default value) and the
/// second giving the source of the value (i.e. which environment variable was
/// used or the "default"), which can be useful for debugging feedback.
///
/// Configuration methods search priority summary:
/// - `RuntimeLibrary::default_cxx()`:
///   - `$CASM_CXX`,
///   - else `$CXX`,
///   - else default=`"g++"`
/// - `RuntimeLibrary::default_cxxflags()`:
///   - `$CASM_CXXFLAGS`,
///   - else default=`"-O3 -Wall -fPIC --std=c++17"`
/// - `RuntimeLibrary::default_soflags()`:
///   - `$CASM_SOFLAGS`,
///   - else default=`"-shared"`
/// - `RuntimeLibrary::default_casm_includedir()`:
///   - `$CASM_INCLUDEDIR`,
///   - else `$CASM_PREFIX/include`,
///   - else `$prefix/include`
///     - where `$prefix` is found by searching `PATH` for the executable
///       `ccasm` and then relative to that searching checking for `../include/
///       casm`
///   - else default=`"/not/found"`
/// - `RuntimeLibrary::default_casm_libdir()`:
///   - `$CASM_LIBDIR`,
///   - else `$CASM_PREFIX/lib`,
///   - else `$prefix/$librelpath`
///     - where `$prefix` is found by searching PATH for the executable `ccasm`
///       and then `$librelpath` is found by searching for `libcasm_global.so`
///       or `libcasm_global.dylib` in standard locations `lib`, `lib64`, or
///       `lib/x86_64-linux-gnu` relative to `$prefix`,
///   - else default=`"/not/found"`
///
/// Additional helpful methods for constructing compilation options:
/// - `::use_env`
/// - `::find_executable`
/// - `::include_path`
/// - `::link_path`
///
/// \ingroup System

/// \brief Construct a RuntimeLibrary object, with the options to be used for
/// compiling the '.o' file and the '.so' file
///
/// See class documentation for usage.
RuntimeLibrary::RuntimeLibrary(std::string filename_base,
                               std::string compile_options,
                               std::string so_options)
    : m_filename_base(filename_base),
      m_compile_options(compile_options),
      m_so_options(so_options),
      m_handle(nullptr) {
  // If the shared library doesn't exist
  if (!fs::exists(m_filename_base + ".so")) {
    // But the library source code does
    if (fs::exists(m_filename_base + ".cc")) {
      // Compile it
      _compile();

    } else {
      throw std::runtime_error(std::string("Error in RuntimeLibrary\n") +
                               "  Could not find '" + m_filename_base +
                               ".so' or '" + m_filename_base + ".cc'");
    }
  }

  // If the shared library exists
  if (fs::exists(m_filename_base + ".so")) {
    // Load the library with the Clexulator
    _load();

  } else {
    throw std::runtime_error(std::string("Error in Clexulator constructor\n") +
                             "  Did not find '" + m_filename_base + ".so'");
  }
}

RuntimeLibrary::~RuntimeLibrary() {
  if (m_handle != nullptr) {
    _close();
  }
}

/// \brief Compile a shared library
///
/// Compiles source file into ".o" and ".so" files. For example, if this was
/// constructed with `_filename_base = "/path/to/MyLibrary"`, compiles
/// "/path/to/MyLibrary.cc" into "/path/to/MyLibrary.o" and
/// "/path/to/MyLibrary.so".
///
void RuntimeLibrary::_compile() {
  // compile the source code into a dynamic library
  Popen p;
  std::string cmd = m_compile_options + " -o " + m_filename_base + ".o" +
                    " -c " + m_filename_base + ".cc";
  p.popen(cmd);
  if (p.exit_code()) {
    throw runtime_lib_compile_error(
        m_filename_base, cmd, p.gets(),
        "Can not compile " + m_filename_base + ".cc");
  }

  cmd = m_so_options + " -o " + m_filename_base + ".so" + " " +
        m_filename_base + ".o";
  p.popen(cmd);
  if (p.exit_code()) {
    throw runtime_lib_shared_error(
        m_filename_base, cmd, p.gets(),
        "Can not compile " + m_filename_base + ".cc");
  }
}

/// \brief Load a library with a given name
///
/// For `_filename_base == "hello"`, this loads "hello.so"
///
void RuntimeLibrary::_load() {
  m_handle = dlopen((m_filename_base + ".so").c_str(), RTLD_NOW);
  if (!m_handle) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    throw std::runtime_error(std::string("Cannot open library: ") +
                             m_filename_base + ".so");
  }
}

/// \brief Close the current library
void RuntimeLibrary::_close() {
  // close
  if (m_handle != nullptr) {
    dlclose(m_handle);
    m_handle = nullptr;
  }
}

/// \brief Remove the current library and source code
void RuntimeLibrary::rm() {
  _close();
  // rm
  Popen p;
  p.popen(std::string("rm -f ") + m_filename_base + ".cc " + m_filename_base +
          ".o " + m_filename_base + ".so");
}

namespace {

std::vector<std::string> _cxx_env() {
  return std::vector<std::string>{"CASM_CXX", "CXX"};
}

std::vector<std::string> _cxxflags_env() {
  return std::vector<std::string>{"CASM_CXXFLAGS"};
}

std::vector<std::string> _soflags_env() {
  return std::vector<std::string>{"CASM_SOFLAGS"};
}

/// \brief Search for an "include" directory by searching PATH for
/// `executable_name`
///
/// Notes:
/// - Assumes PATH can be split using `:`
fs::path find_include(std::string executable_name, std::string include_name) {
  fs::path loc = find_executable(executable_name);
  if (loc.empty()) {
    return loc;
  }
  fs::path maybe_includedir = loc.parent_path().parent_path() / "include";
  if (fs::exists(maybe_includedir / include_name)) {
    return maybe_includedir / include_name;
  }
  return fs::path();
}

fs::path find_includedir(std::string executable_name,
                         std::string include_name) {
  return find_include(executable_name, include_name).parent_path();
}

fs::path find_lib(std::string executable_name, std::string lib_name) {
  fs::path loc = find_executable(executable_name);
  if (loc.empty()) {
    return loc;
  }
  fs::path maybe_prefix = loc.parent_path().parent_path();

  auto check_dir = [&](fs::path test_libdir) {
    std::vector<std::string> check{"dylib", "so"};
    for (const auto &s : check) {
      auto res = test_libdir / (lib_name + "." + s);
      if (fs::exists(res)) {
        return res;
      }
    }
    return fs::path();
  };

  auto check_names = [&](fs::path test_prefix) {
    std::vector<fs::path> check{"lib", "lib64", "lib/x86_64-linux-gnu"};
    for (const auto &s : check) {
      auto res = check_dir(test_prefix / s);
      if (!res.empty()) {
        return res;
      }
    }
    return fs::path();
  };

  return check_names(maybe_prefix);
}

fs::path find_libdir(std::string executable_name, std::string lib_name) {
  return find_lib(executable_name, lib_name).parent_path();
}

}  // namespace

/// \brief Return default compiler and specifying variable
///
/// \returns {"$CASM_CXX", "CASM_CXX"} if environment variable CASM_CXX exists,
///          {"$CXX", "CXX"} if environment variable CXX exists,
///          otherwise {"g++", "default"}
std::pair<std::string, std::string> RuntimeLibrary::default_cxx() {
  return use_env(_cxx_env(), "g++");
}

/// \brief Default c++ compiler options
///
/// \returns {"$CASM_CXXFLAGS", "CASM_CXXFLAGS"} if environment variable
///          CASM_CXXFLAGS exists,
///          otherwise {"-O3 -Wall -fPIC --std=c++17", "default"}
std::pair<std::string, std::string> RuntimeLibrary::default_cxxflags() {
  return use_env(_cxxflags_env(), "-O3 -Wall -fPIC --std=c++17");
}

/// \brief Default c++ shared library options
///
/// \returns {"$CASM_SOFLAGS", "CASM_SOFLAGS"} if environment variable
///          CASM_SOFLAGS exists,
///          otherwise {"-shared", "default"}
std::pair<std::string, std::string> RuntimeLibrary::default_soflags() {
  return use_env(_soflags_env(), "-shared");
}

/// \brief Return include path option for CASM
///
/// \returns In order of preference, one of:
///     - {$CASM_INCLUDEDIR, "CASM_INCLUDEDIR"}, or
///     - {$CASM_PREFIX/include, "CASM_PREFIX"} or
///     - {$prefix/include, "relpath"}, where using $prefix is found by
///       searching PATH for ccasm, or
///     - {"/not/found", "notfound"}, if previous all fail
///
std::pair<fs::path, std::string> RuntimeLibrary::default_casm_includedir() {
  char *_env;

  // if CASM_INCLUDEDIR exists
  _env = std::getenv("CASM_INCLUDEDIR");
  if (_env != nullptr) {
    return std::make_pair(std::string(_env), "CASM_INCLUDEDIR");
  }

  // if CASM_PREFIX exists
  _env = std::getenv("CASM_PREFIX");
  if (_env != nullptr) {
    return std::make_pair(fs::path(_env) / "include", "CASM_PREFIX");
  }

  // relpath from ccasm
  fs::path _default = find_includedir("ccasm", "casm");
  if (!_default.empty()) {
    return std::make_pair(_default, "relpath");
  }

  // else
  return std::make_pair(fs::path("/not/found"), "notfound");
}

/// \brief Return lib path option for CASM
///
/// \returns In order of preference, one of:
///     - {$CASM_LIBDIR, "CASM_LIBDIR"}, or
///     - {$CASM_PREFIX/lib, "CASM_PREFIX"} or
///     - {$prefix/$librelpath, "relpath"}, where using $prefix is found by
///       searching PATH for ccasm and $librelpath is found by searching for
///       `libcasm_global.so` or `libcasm_global.dylib` in standard locations
///       `lib`, `lib64`, or `lib/x86_64-linux-gnu` relative to $prefix, or
///     - {"/not/found", "notfound"}, if previous all fail
///
std::pair<fs::path, std::string> RuntimeLibrary::default_casm_libdir() {
  char *_env;

  // if CASM_LIBDIR exists
  _env = std::getenv("CASM_LIBDIR");
  if (_env != nullptr) {
    return std::make_pair(std::string(_env), "CASM_LIBDIR");
  }

  // if CASM_PREFIX exists
  _env = std::getenv("CASM_PREFIX");
  if (_env != nullptr) {
    return std::make_pair(fs::path(_env) / "lib", "CASM_PREFIX");
  }

  // relpath from ccasm
  fs::path _default = find_libdir("ccasm", "libcasm_global");
  if (!_default.empty()) {
    return std::make_pair(_default, "relpath");
  }

  // else
  return std::make_pair(fs::path("/not/found"), "notfound");
}

/// \brief Get a value from the environment
///
/// \param var List of environment variables to check, in order of priority
/// \param _default Default value to use if no element of `var` was found
///
/// \returns Pair of {value,source}, where `value` is the value obtained, and
/// `source` is the environment value found, else "default" if no element of
/// `var` was found
///
/// \relates RuntimeLibrary
std::pair<std::string, std::string> use_env(std::vector<std::string> var,
                                            std::string _default) {
  for (const auto &v : var) {
    char *_env = std::getenv(v.c_str());
    if (_env != nullptr) {
      return std::make_pair(std::string(_env), v);
    }
  }
  return std::make_pair(_default, "default");
}

/// \brief Search PATH for `name`
///
/// Notes:
/// - Assumes PATH can be split using `:`
///
/// \relates RuntimeLibrary
fs::path find_executable(std::string name) {
  char *_env = std::getenv("PATH");

  char_separator sep(":");
  tokenizer tok(_env, sep);
  std::vector<std::string> splt(tok.begin(), tok.end());

  for (const auto &p : splt) {
    fs::path test{fs::path(p) / name};
    if (fs::exists(test)) {
      return test;
    }
  }
  return fs::path();
}

/// Return an include path string
///
/// Equivalent to:
/// \code
/// return dir.empty() ? "" : "-I" + dir.string();
/// \endcode
///
/// \relates RuntimeLibrary
std::string include_path(const fs::path &dir) {
  if (!dir.empty()) {
    return "-I" + dir.string();
  }
  return "";
};

/// Return a linking path string
///
/// Equivalent to:
/// \code
/// return dir.empty() ? "" : "-L" + dir.string();
/// \endcode
///
/// \relates RuntimeLibrary
std::string link_path(const fs::path &dir) {
  if (!dir.empty()) {
    return "-L" + dir.string();
  }
  return "";
};

}  // namespace CASM
