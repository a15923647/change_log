#include "vector_storage.h"
#include "lcs.h"
#include "file.h"
#include "encoding.h"
#include "config.h"
void VectorStorage::update(Event& ev) {
  fs::path abspath(fs::path(ev.path));
  /*TODO: add some check(regex) on this spot*/
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

  if (ev.ev_type == EventType::MODIFY) {
    File& f = watch_struct[dir][filename];
    f.processing->lock();
    std::string old_content;
    string temp_path(f.temp_path.u8string());
    read_whole_file(temp_path, old_content);
    //lcs
    LCS lcs(old_content, content);
    //store to container
    record.push_back(Node(ev, lcs.op_list));
    //update file states
    f.sha256_hexs = sha256(content);
    fs::copy(f.path, f.temp_path);
    f.processing->unlock();
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
  for (auto it = record.end()-1; it >= record.begin(); it--) {
    if (difftime(time(NULL), it->trigger_time) > nu) break;
    ret->push_back(*it);
  }
  std::reverse(ret->begin(), ret->end());
  return ret;
}
