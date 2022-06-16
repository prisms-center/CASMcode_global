#include "casm/casm_io/FileData.hh"

#include "casm/global/definitions.hh"
#include "casm/global/filesystem.hh"

namespace CASM {

std::time_t _to_time_t(std::filesystem::file_time_type const &ftime) {
  return std::chrono::duration_cast<std::chrono::seconds>(
             ftime.time_since_epoch())
      .count();
}

bool FileData::exists() const { return std::filesystem::exists(path()); }

void FileData::refresh() {
  m_timestamp = _to_time_t(std::filesystem::file_time_type());
  if (this->exists()) {
    m_timestamp = _to_time_t(std::filesystem::last_write_time(path()));
  }
}
}  // namespace CASM
