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
  const std::string root_dir("/mnt/nas/data/programming/common");
  const std::string temp_dir("/mnt/nas/data/tmp/test_change_log");
  const std::string re(".*\\.(cpp|h|py|js|css|html|go)|[mM]akefile");
}
#endif
