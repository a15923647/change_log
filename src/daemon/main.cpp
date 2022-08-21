#include "inotify_watcher.h"
#include "vector_storage.h"
#include "config.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <filesystem>

using namespace std;

void qthread() {
  while (1) {
    puts("querying...");

    time_t now; time(&now);
    struct tm * timeinfo = localtime (&now);
    
    string dump_name = to_string(timeinfo->tm_year) + "_" + (timeinfo->tm_mon+1) + "_" + to_string(timeinfo->tm_mday);
    fs::path dump_path = fs::path(config::temp_dir) / fs::path(dump_name);
    ofstream fout(dump_path.u8string());
    string q("1 day");
    std::vector<Node> *res = (std::vector<Node> *)VectorStorage::query(q);
    fout << res->size() << endl;
    for (Node node : *res) {
      fout << node << endl;
    }
    delete res;

    timeinfo->tm_hour = timeinfo->tm_min = timeinfo->tm_sec = 0;
    timeinfo->tm_mday++;
    double seconds = difftime(mktime(timeinfo), time(NULL));
    cout << "sleep " << seconds << " seconds\n";
    sleep(seconds);
    //sleep(10);
  }
}

int main(void) {
  config::load_from_json("config.json");
  VectorStorage *vstorage_p = new VectorStorage();
  thread t(qthread);
  t.detach();
  InotifyWatcher watchd(vstorage_p);
  watchd.start();
  return 0;
}
