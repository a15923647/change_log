#ifndef INOTIFY_WATCHER_INCLUDED
#define INOTIFY_WATCHER_INCLUDED
#include "watcher.h"
#include "storage.h"
#include "config.h"
#include <string>
class InotifyWatcher : public Watcher {
  public:
    InotifyWatcher(std::string config_path);
    InotifyWatcher();
    void start() override;
    void init() override;
  private:
    bool match(std::string& filename);
    void get_file_path_loop(const std::vector<std::string> watch_paths);
    std::string config_path;
    int config_wd;
    int fd;
};
#endif
