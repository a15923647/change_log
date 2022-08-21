#ifndef INOTIFY_WATCHER_INCLUDED
#define INOTIFY_WATCHER_INCLUDED
#include "watcher.h"
#include "storage.h"
#include <string>
class InotifyWatcher : public Watcher {
  public:
    InotifyWatcher(Storage* storage_p);
    void start() override;
    void init() override;
  private:
    bool match(std::string& filename);
    void get_file_path_loop(const std::vector<std::string> watch_paths);
    Storage* storage_p;
};
#endif
