#ifndef FILE_INCLUDED
#define FILE_INCLUDED
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>
#include <filesystem>
#include <mutex>
#include <sys/stat.h>//
//namespace fs = std::filesystem;
#define fs std::filesystem
std::string sha256(const std::string str);
void read_whole_file(std::string& path, std::string& dest);
struct File {
  fs::path path;
  fs::path temp_path;
  std::string sha256_hexs;
  std::mutex *processing;
  File() {}
  File(std::string path, std::string root_path, std::string temp_dir, std::string content="") {
    this->path = fs::path(path);
    this->temp_path = fs::path(temp_dir) / fs::relative(this->path, fs::path(root_path));//concat
    //std::string content;
    this->processing = new std::mutex();
    if (content.length() == 0)
      read_whole_file(path, content);
    sha256_hexs = sha256(content);
  }
};
#endif
