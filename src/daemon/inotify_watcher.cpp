#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <iostream>
#include <regex>
#include <cxxabi.h>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <string>
#include <filesystem>
#include "inotify_watcher.h"
#include "config.h"
#include "event.h"
//#include "vector_storage.h"
#include "sqlite_storage.h"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))
using namespace std;
//namespace fs = std::filesystem;
#define fs std::filesystem

bool InotifyWatcher::match(std::string& filename) {
   regex e(config::re);
   return regex_match(filename, e);
}

InotifyWatcher::InotifyWatcher() : InotifyWatcher(config::config_path) {}

InotifyWatcher::InotifyWatcher(string config_path) : config_path(config_path) {
  if (config_path.empty()) {// which imply config is not loaded
    std::cerr << "config path is empty on creating InotifyWatcher\nPlease make sure config is loaded\n";
    exit(5);
  }
  init();
}
/*
 * Init
*/
void InotifyWatcher::start() {
  fs::path temp_dirp(config::temp_dir);
  if (!fs::exists(temp_dirp)) 
    fs::create_directory(temp_dirp);
  get_file_path_loop(config::root_dirs);
}

static int watch_dir(int fd, fs::path path, unordered_map<int, fs::path>& wd2path, unordered_set<string>& watching) {
  try {
    for (auto const& dir_entry : fs::directory_iterator{path}) {
      if (fs::is_directory(dir_entry.path()) && !watching.count(dir_entry.path().u8string())) {
        watch_dir(fd, dir_entry.path(), wd2path, watching);
      }
    }
  } catch(std::exception &e) {
    cout << "exception occoured on listing directory, " << e.what() << endl;
  }
  string path_str = path.u8string();
  watching.insert(path_str);
  //On error, -1 is returned
  int cur_wd = inotify_add_watch(fd, path_str.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_OPEN);
  if (cur_wd != -1) {
    wd2path[cur_wd] = path;
    cout << "watching " << path_str << endl;
  }
  return cur_wd;
}

static int watch_config(int fd, string config_path) {
  int conf_wd = inotify_add_watch(fd, config_path.c_str(), IN_MODIFY | IN_CREATE | IN_MOVED_TO);
  if (conf_wd == -1) {
    cerr << "Encounter error on adding config wd\n";
    exit(6);
  } else {
    cout << "watching config file: " + config_path + "\n";
  }
  return conf_wd;
}

void InotifyWatcher::init() {
  if (config::root_dirs.empty()) {
    config::load_from_json(config_path);
  }
  fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }
  config_wd = watch_config(fd, config_path);
}

static void load_watch_paths(int fd, vector<string>& watch_paths, unordered_map<int, fs::path>& wd2path) {
  unordered_set<string> watching;
  for (string path : watch_paths) {
    watch_dir(fd, fs::path(path), wd2path, watching);
  }
  cout << "watching " << watching.size() << " paths\n";
}

void InotifyWatcher::get_file_path_loop(vector<string> watch_paths) {
  char buffer[BUF_LEN];
  int length, i = 0;
  unordered_map<int, fs::path> wd2path;
  
  load_watch_paths(fd, watch_paths, wd2path);

  unordered_map<int, EventType> evt_trans;
  evt_trans[IN_MODIFY] = EventType::MODIFY;
  evt_trans[IN_MOVED_TO] = EventType::MODIFY;
  evt_trans[IN_CREATE] = EventType::CREATE;
  evt_trans[IN_OPEN] = EventType::CREATE;
  evt_trans[IN_DELETE] = EventType::DELETE;

  unordered_map<int, string> evt_name;
  evt_name[IN_MODIFY] = "modify";
  evt_name[IN_MOVED_TO] = "move to";
  evt_name[IN_CREATE] = "create";
  evt_name[IN_OPEN] = "open";
  evt_name[IN_DELETE] = "delete";
  
  puts("starts watching...");
  while (1) {
    length = read(fd, buffer, BUF_LEN);
    if (length < 0)
      perror("read");

    unordered_map<int, fs::path> new_wd2path;
    bool config_event_triggered = false;
    for (i = 0; i < length;) {
      struct inotify_event *event = (struct inotify_event *) &buffer[i];
      
      //cout << "is config path: " << (event->wd == config_wd) << endl;
      //file path is exactly config path
      if (event->wd == config_wd) {
        config_event_triggered = true;
        cout << "detect config is modified\n";
      }
      if (event->len) {
        if (!wd2path.count(event->wd)) continue;
        fs::path dir = wd2path[event->wd];
        fs::path filename(event->name);
        fs::path fpath = dir / filename;
        string filename_str(event->name);
        EventType ev_t;
        if (event->mask & IN_CREATE && fs::is_directory(fpath)) {
          cout << "tracking new dir " << fpath << endl;
          vector<string> new_dirv(1, fpath);
          load_watch_paths(fd, new_dirv, wd2path);
        } else if (!match(filename_str)) {
          cout << filename_str << " not match tracking pattern\n";
        } else if (event->mask & IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO | IN_OPEN) {
          string full_path = fpath.u8string();
          cout << "detect " << evt_name[event->mask] << " event on: " << full_path << endl;
          Event new_ev(evt_trans[event->mask], full_path);
          thread t(SQLiteStorage::update, new_ev);
          t.detach();
          //VectorStorage::update(new_ev);
        }
      }
      i += EVENT_SIZE + event->len;
    }
    if (config_event_triggered) {
      //delete old one
      for (auto &[key, val] : wd2path) {
        //On success, the function returns 0, for failure the function returns -1.
        inotify_rm_watch(fd, key);
      }
      wd2path.clear();
      config_wd = watch_config(fd, config_path);
      if(config::load_from_json(config_path)) {            
        cout << "new config is loaded\n";
      } else {
        cout << "join back old config\n";
      }
      load_watch_paths(fd, config::root_dirs, wd2path);
      cout << "watching " << wd2path.size() << " paths\n";
    }
  }
}
