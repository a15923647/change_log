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

//bool InotifyWatcher::match(std::string& filename) {
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
  vector<string> watch_paths(1, config::root_dir);
  get_file_path_loop(watch_paths);
}

void InotifyWatcher::init() {
  const auto copyOptions = fs::copy_options::update_existing;
  fs::copy(config::root_dir, config::temp_dir, copyOptions);
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

  for (string path : watch_paths) {
    wd2path[inotify_add_watch(fd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE)] = fs::path(path);
  }
  unordered_map<int, EventType> evt_trans;
  evt_trans[IN_MODIFY] = EventType::MODIFY;
  evt_trans[IN_CREATE] = EventType::CREATE;
  evt_trans[IN_DELETE] = EventType::DELETE;

  unordered_map<int, string> evt_name;
  evt_name[IN_MODIFY] = "modify";
  evt_name[IN_CREATE] = "create";
  evt_name[IN_DELETE] = "delete";
  
  puts("starts watching...");
  while (1) {
    length = read(fd, buffer, BUF_LEN);
    if (length < 0)
      perror("read");
    for (i = 0; i < length;) {
      struct inotify_event *event = (struct inotify_event *) &buffer[i];

      if (event->len) {
        fs::path dir = wd2path[event->wd];
        fs::path filename(event->name);
        string filename_str(event->name);
        EventType ev_t;
        if (!match(filename_str)) {
          cout << filename_str << " not match tracking pattern\n";
        } else if (event->mask & IN_CREATE | IN_DELETE | IN_MODIFY) {
          string full_path = (dir / filename).u8string();
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
