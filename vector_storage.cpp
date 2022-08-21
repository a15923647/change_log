#include "vector_storage.h"
#include "lcs.h"
#include "file.h"
#include "encoding.h"
#include "config.h"
#include <iostream>

std::vector<Node> record;
std::map<std::string, std::map<std::string, File>> watch_struct;

void VectorStorage::update(Event ev) {
  puts("updating...");
  fs::path abspath(fs::path(ev.path));
  std::string dir = abspath.parent_path().u8string();
  std::string filename = abspath.filename().u8string();
  bool isdir = fs::is_directory(abspath);
  bool isreg = fs::is_regular_file(abspath);
  //only record directory and regular file change
  if (!isdir && !isreg) return;
  string content;
  //utf8 only
  if (isreg) {
    read_whole_file(ev.path, content);
    if (!validUtf8(content))
      return;
  }
  
  fs::path temp_dir = File::root_dir_sub(dir, config::root_dir, config::temp_dir);
  if (!fs::exists(temp_dir)) {
    fs::create_directories(temp_dir);
  }
  const auto copyOptions = fs::copy_options::update_existing;
  if (ev.ev_type == EventType::MODIFY) {
    if (!watch_struct.count(dir)) {
      watch_struct[dir] = std::map<string, File>();
    }
    if (!watch_struct[dir].count(filename)) {
      File not_tracked_f(ev.path, config::root_dir, config::temp_dir, content);
      watch_struct[dir][filename] = not_tracked_f;
      fs::copy(not_tracked_f.path, not_tracked_f.temp_path, copyOptions);
      return;
    }
    File *f = &(watch_struct[dir][filename]);
    f->processing->lock();
    std::string old_content;
    string temp_path(f->temp_path.u8string());
    read_whole_file(temp_path, old_content);
    string new_hash = sha256(content);
    if (new_hash == f->sha256_hexs) {
      cout << "nothing changed (by hashing)\n";
      f->processing->unlock();
      return;
    }
    //lcs
    LCS lcs(old_content, content);
    string lcs_res;
    lcs.lcs(lcs_res);
    std::cout << "old content: " << old_content << std::endl;
    std::cout << "new content: " << content << std::endl;
    std::cout << "diff length: " << lcs.op_list.content_length() << std::endl;
    if (lcs.op_list.content_length() == 0) {
      cout << "nothing changed\n";
      f->processing->unlock();
      return;
    }
    //store to container
    record.push_back(Node(ev, lcs.op_list));
    //update file states
    f->sha256_hexs = new_hash;
    fs::copy(f->path, f->temp_path, copyOptions);
    f->processing->unlock();
  } else if (ev.ev_type == EventType::CREATE) {
    OperationList op_list;
    if(isdir) {
      watch_struct[abspath] = std::map<string, File>();
      record.push_back(Node(ev, op_list));
      return;
    }
    watch_struct[dir][filename] = File(ev.path, config::root_dir, config::temp_dir, content);
    record.push_back(Node(ev, op_list));
  } else if (ev.ev_type == EventType::DELETE) {
    OperationList op_list;
    if(isdir) {
      watch_struct.erase(dir);
      record.push_back(Node(ev, op_list));
      return;
    }
    record.push_back(Node(ev, op_list));
    //pass regular file delete on temp dir
  }
}

void *VectorStorage::query(std::string& query) {
  //query for a period ago
  //nu(1) unit(day hour)
  std::stringstream ss;
  ss << query;
  long long nu;
  ss >> nu;
  std::string unit;
  ss >> unit;
  if (unit == "day")
    nu *= 24;
  nu *= 3600;

  std::vector<Node> *ret = new vector<Node>();
  int rsize = record.size();
  std::cout << "record size: " << rsize << std::endl;
  for (int i = rsize-1; i >= 0; i--) {
    std::cout << i << "/" << rsize << std::endl;
    if (difftime(time(NULL), record[i].trigger_time) > nu) break;
    ret->push_back(record[i]);
  }
  std::reverse(ret->begin(), ret->end());
  return ret;
}
