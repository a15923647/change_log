#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
struct Dir {
  std::vector<std::string> files;
  std::string file_regex;
};

class Config {
  public:
    std::vector<std::string> watching_dirs;
    std::string global_regex;
};
namespace config {
  std::string root_dir("/mnt/nas/data/programming");
  std::string temp_dir("/mnt/nas/data/tmp/");
}
#endif
