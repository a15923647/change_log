#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace config {
  extern std::string temp_dir;
  extern std::string re;
  extern std::vector<std::string> root_dirs;
  extern std::string db_path;
  void load_from_json(std::string path);
}
#endif
