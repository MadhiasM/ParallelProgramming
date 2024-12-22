/**
 * Implementation of a helper class responsible for embedding images generated
 * during the performance analysis in the notebook file.
 */

#ifndef LAZY_IMAGE_H
#define LAZY_IMAGE_H

#include "base64.h"
#include "nlohmann/detail/input/position_t.hpp" // for nlohmann
#include "nlohmann/json.hpp"                    // for basic_json<>::object_t
#include "nlohmann/json_fwd.hpp"                // for json
#include <fstream>                              // for ifstream, stringstream
#include <memory>                               // for allocator, unique_ptr
#include <sstream>                              // for basic_stringstream
#include <stddef.h>                             // for size_t
#include <string>                               // for string, basic_string
#include <sys/stat.h>                           // for stat, st_mtime

namespace nl = nlohmann;

namespace display {
class lazy_image {
public:
  inline lazy_image(const std::string &filename, const std::string &info)
      : filename(filename), info(info), old_time(0) {}

  inline lazy_image(const std::string &filename)
      : filename(filename), info(""), old_time(0) {}

  inline lazy_image() : filename(""), info(""), old_time(0) {}

  inline void set_path(const std::string &name) {
    filename.assign(name);
    old_time = 0;
  }

  std::string get_filename() { return filename; }

  std::string get_info() { return info; }

  bool check_new() {
    struct stat result;
    if (stat(filename.c_str(), &result) == 0) {
      long new_time = result.st_mtime;
      if (new_time > old_time) {
        old_time = new_time;

        return true;
      }
    }
    return false;
  }

  void read() {
    fin = std::make_unique<std::ifstream>(filename, std::ios::binary);
    m_buffer = std::make_unique<std::stringstream>();
    *m_buffer << fin->rdbuf();
  }

  std::unique_ptr<std::stringstream> m_buffer;

private:
  std::string filename;
  std::unique_ptr<std::ifstream> fin;
  std::string info;
  long old_time;
};

nl::json mime_bundle_repr(lazy_image &i) {
  if (i.check_new()) {
    i.read();
  }

  auto bundle = nl::json::object();
  if (i.m_buffer != nullptr && i.m_buffer->str() != "") {
    std::string filename = i.get_filename();
    size_t last_dot_position = filename.find_last_of('.');
    std::string suffix = filename.substr(last_dot_position);

    if (suffix == ".html") {
      bundle["text/html"] = i.m_buffer->str();
    } else if (suffix == ".png") {
      bundle["image/png"] = base64_encode(
          reinterpret_cast<const unsigned char *>(i.m_buffer->str().c_str()),
          i.m_buffer->str().size());
    }
  } else {
    std::string path = __FILE__;
    size_t last_separator = path.find_last_of("/\\");
    std::string name_of_this_file = path.substr(last_separator + 1);
    bundle["text/html"] = "<h3>Error in C++ file " + name_of_this_file +
                          ": Can't find file\n\"" + i.get_filename() +
                          "\"</h3>";
  }
  return bundle;
}
} // namespace display

#endif
