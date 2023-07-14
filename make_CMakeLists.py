import copy
import os

def is_extensionless_Eigen_header(filepath):
    """Returns true if the provided file resides in
    include/casm/external/Eigen, which contains header files
    that don't have an extension like .h

    Parameters
    ----------
    filepath : path to file, relative to git root

    Returns
    -------
    bool

    """
    parent, f = os.path.split(filepath)
    return parent == "include/casm/external/Eigen"


def header_extensions():
    """List of extensions that are considered header files:
    .h, .hh, .hpp
    Returns
    -------
    list of str

    """
    return [".h", ".hh", ".hpp"]


def source_extensions():
    """List of extensions that are considered source files:
    .c, .cc, .cxx, .cpp
    Returns
    -------
    list of str

    """
    return [".c", ".cc", ".cxx", ".cpp", ".C"]


def has_header_extension(filepath):
    """Returns true of the file has an extension such as
    .h, .hh, etc as defined by header_extensions()

    Parameters
    ----------
    filepath : path to file

    Returns
    -------
    bool

    """
    for ext in header_extensions():
        if filepath.endswith(ext):
            return True
    return False


def has_source_extension(filepath):
    """Returns true of the file has an extension such as
    .c, .cc, etc as defined by source_extensions()

    Parameters
    ----------
    filepath : path to file

    Returns
    -------
    bool

    """
    for ext in source_extensions():
        if filepath.endswith(ext):
            return True
    return False


def header_and_source_extensions():
    return header_extensions() + source_extensions()


def git_root(path):
    git_repo = git.Repo(path, search_parent_directories=True)
    git_root = git_repo.git.rev_parse("--show-toplevel")
    return git_root


def all_files_tracked_by_git():
    # return subprocess.check_output(
    #     ["git", "ls-tree", "--full-tree", "-r", "--name-only", "HEAD"], encoding="utf-8"
    # ).splitlines()
    return subprocess.check_output(["git", "ls-files", "--recurse-submodules"],
                                   encoding="utf-8").splitlines()


def all_files_ignored_by_git():
    return subprocess.check_output(["git", "status", "--ignored"],
                                   encoding="utf-8").splitlines()


def relative_filepath_is_tracked_by_git(filename):
    """Checks if the given file, which must exist relative to
    the path of execution, is being tracked by git.

    TODO: I thought caching the root would help because finding the git
    root might be slow with so many calls, but I'm not convinced it makes
    a big difference.

    Parameters
    ----------
    filename : path to file relative to execution directory

    Returns
    -------
    bool

    """
    cwd = os.getcwd()

    if cwd not in relative_filepath_is_tracked_by_git.cached_roots or True:
        relative_filepath_is_tracked_by_git.cached_roots[cwd] = git_root(cwd)

    git_root_path = relative_filepath_is_tracked_by_git.cached_roots[cwd]

    return (os.path.relpath(os.path.realpath(filename),
                            git_root_path) in all_files_tracked_by_git())


def purge_untracked_files(file_list):
    """Return the same list of files, but only include files
    that are currently being tracked by the git repository

    Parameters
    ----------
    file_list : list of path, relative to execution path

    Returns
    -------
    list of path

    """
    return [f for f in file_list if relative_filepath_is_tracked_by_git(f)]


def header_files(search_root):
    files = [
        (dirpath, files)
        for dirpath, dirnames, files in os.walk(search_root, followlinks=True)
    ]
    _header_files = [
        os.path.join(d, f) for d, fs in files for f in fs
        if is_extensionless_Eigen_header(f) or has_header_extension(f)
    ]
    return _header_files

def source_files(search_root):
    files = [
        (dirpath, files)
        for dirpath, dirnames, files in os.walk(search_root, followlinks=True)
    ]
    _source_files = [
        os.path.join(d, f) for d, fs in files for f in fs
        if has_source_extension(f)
    ]
    return _source_files

def libcasm_testing_source_files(search_dir, relative_to):
    files = [
        os.path.relpath(os.path.join(search_dir, file), relative_to)
        for file in os.listdir(search_dir)
        if file != "gtest_main_run_all.cpp" and has_source_extension(file)
    ]
    return files

def unit_test_source_files(search_dir, relative_to, additional):
    files = copy.copy(additional)
    files += [
        os.path.relpath(os.path.join(search_dir, file), relative_to)
        for file in os.listdir(search_dir)
        if has_source_extension(file)
    ]
    return files

def as_cmake_file_strings(files):
    cmake_file_strings = ""
    for file in files:
        cmake_file_strings += "  ${PROJECT_SOURCE_DIR}/" + str(file) + "\n"
    return cmake_file_strings

### make CMakeLists.txt from CMakeLists.txt.in ###

with open("CMakeLists.txt.in", 'r') as f:
    cmakelists = f.read()

files = header_files("include")
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@header_files@", cmake_file_strings)

files = source_files("src")
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@source_files@", cmake_file_strings)

with open("CMakeLists.txt", 'w') as f:
    f.write(cmakelists)


### make tests/CMakeLists.txt from tests/CMakeLists.txt.in ###

with open("tests/CMakeLists.txt.in", 'r') as f:
    cmakelists = f.read()

files = libcasm_testing_source_files("tests/unit", "tests")
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@libcasm_testing_source_files@", cmake_file_strings)


additional = [
    "unit/gtest_main_run_all.cpp"
]
files = unit_test_source_files("tests/unit/casm_io", "tests", additional)
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@casm_unit_casm_io_source_files@", cmake_file_strings)

files = unit_test_source_files("tests/unit/container", "tests", additional)
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@casm_unit_container_source_files@", cmake_file_strings)

files = unit_test_source_files("tests/unit/global", "tests", additional)
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@casm_unit_global_source_files@", cmake_file_strings)

files = unit_test_source_files("tests/unit/misc", "tests", additional)
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@casm_unit_misc_source_files@", cmake_file_strings)

files = unit_test_source_files("tests/unit/system", "tests", additional)
cmake_file_strings = as_cmake_file_strings(files)
cmakelists = cmakelists.replace("@casm_unit_system_source_files@", cmake_file_strings)

with open("tests/CMakeLists.txt", 'w') as f:
    f.write(cmakelists)
