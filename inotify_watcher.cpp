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
#include "vector_storage.h"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))
using namespace std;
//namespace fs = std::filesystem;
#define fs std::filesystem

bool InotifyWatcher::match(std::string& filename) {
   regex e(config::re);
   return regex_match(filename, e);
}

InotifyWatcher::InotifyWatcher(Storage* storage_p) : storage_p(storage_p) {
  init();
}
/*
 * Init
*/
void InotifyWatcher::start() {
  fs::path temp_dirp(config::temp_dir);
  if (!fs::exists(temp_dirp)) 
    fs::create_directory(temp_dirp);
  //vector<string> watch_paths(1, config::root_dir);
  //get_file_path_loop(watch_paths);
  get_file_path_loop(config::root_dirs);
}

void InotifyWatcher::init() {}

static void watch_dir(int fd, fs::path path, unordered_map<int, fs::path>& wd2path, unordered_set<string>& watching) {
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
  wd2path[inotify_add_watch(fd, path_str.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO)] = path;
  cout << "watching " << path_str << endl;
}

void InotifyWatcher::get_file_path_loop(vector<string> watch_paths) {
  char buffer[BUF_LEN];
  int length, i = 0;
  int fd;
  unordered_map<int, fs::path> wd2path;
  
  fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }
  
  unordered_set<string> watching;
  for (string path : watch_paths) {
    //watch_dir here!
    //wd2path[inotify_add_watch(fd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO)] = fs::path(path);
    watch_dir(fd, fs::path(path), wd2path, watching);
  }
  unordered_map<int, EventType> evt_trans;
  evt_trans[IN_MODIFY] = EventType::MODIFY;
  evt_trans[IN_MOVED_TO] = EventType::MODIFY;
  evt_trans[IN_CREATE] = EventType::CREATE;
  evt_trans[IN_DELETE] = EventType::DELETE;

  unordered_map<int, string> evt_name;
  evt_name[IN_MODIFY] = "modify";
  evt_name[IN_MOVED_TO] = "move to";
  evt_name[IN_CREATE] = "create";
  evt_name[IN_DELETE] = "delete";
  
  puts("starts watching...");
  cout << "watching " << watching.size() << " paths\n";
  while (1) {
    length = read(fd, buffer, BUF_LEN);
    if (length < 0)
      perror("read");
    for (i = 0; i < length;) {
      struct inotify_event *event = (struct inotify_event *) &buffer[i];

      if (event->len) {
        fs::path dir = wd2path[event->wd];
        fs::path filename(event->name);
        fs::path fpath = dir / filename;
        string filename_str(event->name);
        EventType ev_t;
        if (event->mask & IN_CREATE && fs::is_directory(fpath)) {
          cout << "tracking new dir " << fpath << endl;
          watch_dir(fd, fpath, wd2path, watching);
        } else if (!match(filename_str)) {
          cout << filename_str << " not match tracking pattern\n";
        } else if (event->mask & IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO) {
          string full_path = fpath.u8string();
          cout << "detect " << evt_name[event->mask] << " event on: " << full_path << endl;
          Event new_ev(evt_trans[event->mask], full_path);
          thread t(VectorStorage::update, new_ev);
          t.detach();
          //VectorStorage::update(new_ev);
        }
        
      }
      i += EVENT_SIZE + event->len;
    }
  }
}
