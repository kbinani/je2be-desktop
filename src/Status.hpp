#pragma once

#include <filesystem>
#include <je2be/status.hpp>

namespace je2be::desktop {

inline Status Error(char const *file, int line, std::string what = {}) {
  namespace fs = std::filesystem;

  Status::Where w(file, line);

  static fs::path const sProjectRoot(fs::path(__FILE__).parent_path());
  std::error_code ec;
  fs::path path(file ? file : "(unknown)");
  fs::path p = fs::relative(path, sProjectRoot, ec);
  if (ec) {
    w.fFile = path.filename().string();
  } else {
    w.fFile = p.string();
  }
  return Status(Status::ErrorData(w, what));
}

} // namespace je2be::desktop
