#include "config.h"
namespace config {
  std::string config_path;
  std::string temp_dir;
  std::string re;
  std::vector<std::string> root_dirs;
  std::string db_path;
}
bool config::load_from_json(std::string path) {
  std::string temp_temp_dir, temp_re, temp_db_path;
  std::vector<std::string> temp_root_dirs;
  try {
    std::ifstream f(path);
    json data = json::parse(f);
    for (auto& e : data["root_dirs"]) {
      temp_root_dirs.push_back(e);
    }
    temp_re = data["re"];
    temp_temp_dir = data["temp_dir"];
    temp_db_path = data["db_path"];
    f.close();
  } catch(std::exception &e) {
    std::cerr << "Encounter error on loading config.\n" << e.what() << std::endl;
    return false;
  }
  config::config_path = path;
  config::temp_dir = temp_temp_dir;
  config::re = temp_re;
  config::root_dirs = temp_root_dirs;
  config::db_path = temp_db_path;
  std::cout << "config is loaded from " + config::config_path + "\n";
  return true;
}
