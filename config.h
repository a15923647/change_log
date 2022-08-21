#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
/*
namespace config {
  const std::string root_dir("/mnt/nas/data/programming/");
  const std::string temp_dir("/mnt/nas/data/fix_tmp/test_change_log");
  const std::string re(".*\\.(cpp|h|py|js|css|html|go)|[mM]akefile");
  const std::vector<std::string> root_dirs({
    "/mnt/nas/data/programming/",
    "/mnt/nas/data/tmp/"
  });
}
*/
namespace config {
  extern std::string temp_dir;
  extern std::string re;
  extern std::vector<std::string> root_dirs;
  void load_from_json(std::string path);
}
#endif
