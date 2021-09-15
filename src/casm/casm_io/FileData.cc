#include "casm/casm_io/FileData.hh"
#include "casm/global/definitions.hh"
#include "casm/global/filesystem.hh"

namespace CASM {

bool FileData::exists() const { return std::filesystem::exists(path()); }

void FileData::refresh() {
  m_timestamp = std::filesystem::file_time_type();
  if (this->exists()) {
    m_timestamp = std::filesystem::last_write_time(path());
  }
}
}  // namespace CASM
