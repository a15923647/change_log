#include "config.h"
namespace config {
  std::string temp_dir;
  std::string re;
  std::vector<std::string> root_dirs;
  std::string db_path;
}
void config::load_from_json(std::string path) {
  std::ifstream f(path);
  json data = json::parse(f);
  for (auto& e : data["root_dirs"]) {
    config::root_dirs.push_back(e);
  }
  config::re = data["re"];
  config::temp_dir = data["temp_dir"];
  config::db_path = data["db_path"];
}
